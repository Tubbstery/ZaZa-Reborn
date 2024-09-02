#include "includes.h"

Chams g_chams{ };;

void Chams::SetupMaterial(IMaterial* mat, float alpha, Color col, bool z_flag, bool wireframe) {
	if (mat == m_materials[3] || mat == m_materials[4] || mat == m_materials[5]) {
		mat->FindVar(XOR("$envmaptint"), nullptr)->SetVecValue(col.r() / 255.f, col.g() / 255.f, col.b() / 255.f);
	}

	mat->AlphaModulate(alpha);
	mat->ColorModulate(col);

	// mat->SetFlag( MATERIAL_VAR_HALFLAMBERT, flags );
	mat->SetFlag(MATERIAL_VAR_ZNEARER, z_flag);
	mat->SetFlag(MATERIAL_VAR_NOFOG, z_flag);
	mat->SetFlag(MATERIAL_VAR_IGNOREZ, z_flag);

	if (mat && !mat->IsErrorMaterial())
		mat->SetFlag(MATERIAL_VAR_WIREFRAME, wireframe);

	g_csgo.m_studio_render->ForcedMaterialOverride(mat);
}

void Chams::init() {
	if (!init_materials) {
		// create custom materials.
		std::ofstream("csgo\\materials\\sup_metallic.vmt") << R"#("VertexLitGeneric" {
			"$basetexture"	"vgui/white"
			"$envmap"       "env_cubemap"
			"$model"		"1"
			"$flat"			"0"
			"$nocull"		"1"
			"$halflambert"	"1"
			"$nofog"		"1"
			"$ignorez"		"0"
			"$znearer"		"0"
			"$wireframe"	"0"
        })#";

		std::ofstream("csgo/materials/sup_ghost.vmt") << R"#("VertexLitGeneric" {
		    "$basetexture" "models/effects/cube_white"
		    "$additive"                    "1"
		    "$envmap"                    "models/effects/cube_white"
		    "$envmaptint"                "[1.0 1.0. 1.0]"
		    "$envmapfresnel"            "1.0"
		    "$envmapfresnelminmaxexp"    "[0.0 1.0 2.0]"
		    "$alpha"                    "0.99"
	    })#";

		std::ofstream("csgo/materials/texture_scroll.vmt") << R"#("VertexLitGeneric" {
                "$basetexture"                "models/inventory_items/dreamhack_trophies/dreamhack_star_blur"
                "$additive"                   "1"
                "$envmap"                     "env_cubemap"
                "$envmapfresnel"              "1"
		        "$envmapfresnelminmaxexp" "[0 1 2]"
                "$alpha"                      "1"
				"$ignorez"		"1"
				"$wireframe"	"1"

                Proxies
                {
                    TextureScroll
                    {
                        "texturescrollvar"            "$baseTextureTransform"
                        "texturescrollrate"            "0.25"
                        "texturescrollangle"        "270"
                    }
                    Sine
                    {
                        "sineperiod"                "2"
                        "sinemin"                    "0.1"
                        "resultVar"                    "$envmapfresnelminmaxexp[1]"
                    }
                }
          }
        )#";

		init_materials = true;
	}

	// find materials.
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("debug/debugambientcube"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("debug/debugdrawflat"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("sup_metallic"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("dev/glow_armsrace"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("sup_ghost"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("texture_scroll"), XOR("Model textures")));
	m_materials.push_back(g_csgo.m_material_system->FindMaterial(XOR("models/inventory_items/cologne_prediction/cologne_prediction_glass"), XOR("Model textures")));

	// iterate through materials.
	for (int i = 0; i < m_materials.size(); i++) {
		m_materials[i]->IncrementReferenceCount();
	}
}

bool Chams::GetHistoryMatrix(Player* player, matrix3x4_t* out, float& alphamod) {
	LagRecord* current_record;
	AimPlayer* data;

	if (!player)
		return false;

	data = &g_aimbot.m_players[player->index() - 1];
	if (!data || data->m_records.empty())
		return false;

	if (data->m_records.size() < 2)
		return false;

	if (data->m_records.front().m_broke_lc)
		return false;

	// start from begin
	for (auto it = data->m_records.begin(); it != data->m_records.end(); ++it) {
		LagRecord* last_first{ nullptr };
		LagRecord* last_second{ nullptr };

		if (it->valid() && it + 1 != data->m_records.end() && !(it + 1)->valid()) {
			last_first = &*(it + 1);
			last_second = &*it;
		}

		if (!last_first || !last_second)
			continue;

		const auto& FirstInvalid = last_first;
		const auto& LastInvalid = last_second;

		if (!LastInvalid || !FirstInvalid)
			continue;

		if (FirstInvalid->m_lagfixed || LastInvalid->m_lagfixed)
			continue;

		alphamod = (FirstInvalid->m_origin - player->GetAbsOrigin()).length();
		std::clamp(alphamod, 0.f, 30.f);
		alphamod /= 30.f;

		const auto NextOrigin = LastInvalid->m_origin;
		const auto curtime = g_csgo.m_globals->m_curtime;

		auto flDelta = 1.f - (curtime - LastInvalid->m_interp_time) / (LastInvalid->m_sim_time - FirstInvalid->m_sim_time);
		if (flDelta < 0.f || flDelta > 1.f)
			LastInvalid->m_interp_time = curtime;

		flDelta = 1.f - (curtime - LastInvalid->m_interp_time) / (LastInvalid->m_sim_time - FirstInvalid->m_sim_time);

		const auto lerp = math::Interpolate(NextOrigin, FirstInvalid->m_origin, std::clamp(flDelta, 0.f, 1.f));

		matrix3x4_t ret[128];
		std::memcpy(ret, FirstInvalid->m_matrix, sizeof(ret));

		for (size_t i{ }; i < 128; ++i) {
			const auto matrix_delta = FirstInvalid->m_matrix[i].GetOrigin() - FirstInvalid->m_origin;
			ret[i].SetOrigin(matrix_delta + lerp);
		}

		std::memcpy(out, ret, sizeof(ret));
		return true;
	}

	return false;
}

int Chams::FailsSanity(Player* player, const ModelRenderInfo_t& info) {
	if (!g_csgo.m_studio_render)
		return 1;

	if (!init_materials)
		return 2;

	if (!g_csgo.m_engine->IsInGame())
		return 3;

	if (info.m_flags & 0x40000000 || !info.m_renderable/* || !RenderInfo.pRenderable->GetIClientUnknown()*/ || m_in_pse)
		return 4;

	if (player->dormant())
		return 5;

	auto clientclass = player->GetClientClass();
	if (!clientclass)
		return 6;

	// dead and not a ragdoll.
	if (!player->alive() && clientclass->m_ClassID != 37)
		return 7;

	return 0;
}

bool Chams::HandleChams(Player* entity, const ModelRenderInfo_t& info, std::function<void()> oFn) {
	bool bReturn = false;
	matrix3x4_t historymatrix[128];
	float alphamod = 1.f;

	// local.
	if (entity == g_cl.m_local) {
		if (g_menu.main.players.chams_autopeek_enable.get() && g_input.GetKeyState(g_menu.main.movement.autopeek.get()) && g_cl.m_pre_autopeek_bones) {
			m_override_matrix = g_cl.m_pre_autopeek_bones;

			float alpha = g_menu.main.players.chams_autopeek_pulsate_alpha.get()
				? abs(sin(g_csgo.m_globals->m_curtime * 2))
				: g_menu.main.players.chams_autopeek_blend.get() / 100.f;

			SetupMaterial(
				m_materials[g_menu.main.players.chams_autopeek_mat.get()],
				alpha,
				g_menu.main.players.chams_autopeek_col.get(),
				g_menu.main.players.chams_enemy.get(1),
				g_menu.main.players.chams_autopeek_wireframe.get()
			);
			oFn();

			if (g_menu.main.players.chams_autopeek_double_enable.get()) {
				SetupMaterial(
					m_materials[g_menu.main.players.chams_autopeek_double_mat.get() + 3],
					alpha,
					g_menu.main.players.chams_autopeek_double_col.get(),
					g_menu.main.players.chams_enemy.get(1),
					false
				);
				oFn();
			}


			m_override_matrix = nullptr;
		}

		if (g_menu.main.players.chams_local_enable.get()) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_local_mat.get()],
				entity->m_bIsScoped() ? (g_menu.main.players.chams_local_blend.get() / 100.f) / 2 : g_menu.main.players.chams_local_blend.get() / 100.f,
				g_menu.main.players.chams_local_col.get(),
				false,
				g_menu.main.players.chams_local_wireframe.get()
			);
			oFn();

			// local double.
			if (g_menu.main.players.chams_local_double_enable.get()) {
				SetupMaterial(
					m_materials[g_menu.main.players.chams_local_double_mat.get() + 3],
					g_menu.main.players.chams_local_blend.get() / 100.f,
					g_menu.main.players.chams_local_double_col.get(),
					false,
					false
				);
				oFn();
			}

			bReturn = true;
		}

		if (entity->m_bIsScoped() || (g_cl.m_weapon && g_cl.m_weapon_type == WEAPONTYPE_GRENADE)) {
			g_csgo.m_render_view->SetBlend(0.5f);
		}
	}
	else if (entity->enemy(g_cl.m_local) && g_menu.main.players.chams_enemy_enable.get()) {
		// enemy history.
		if (g_menu.main.players.chams_history_enable.get() && GetHistoryMatrix(entity, historymatrix, alphamod)) {
			m_override_matrix = historymatrix;

			SetupMaterial(
				m_materials[g_menu.main.players.chams_history_mat.get()],
				(g_menu.main.players.chams_history_blend.get() / 100.f) * alphamod,
				g_menu.main.players.chams_history_col.get(),
				g_menu.main.players.chams_enemy.get(1),
				g_menu.main.players.chams_history_wireframe.get()
			);
			oFn();

			if (g_menu.main.players.chams_history_double_enable.get()) {
				SetupMaterial(
					m_materials[g_menu.main.players.chams_history_double_mat.get() + 3],
					(g_menu.main.players.chams_history_blend.get() / 100.f) * alphamod,
					g_menu.main.players.chams_history_double_col.get(),
					g_menu.main.players.chams_enemy.get(1),
					false
				);
				oFn();
			}

			m_override_matrix = nullptr;
		}

		// enemy xqz.
		if (g_menu.main.players.chams_enemy.get(1)) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_enemy_mat.get()],
				g_menu.main.players.chams_enemy_blend.get() / 100.f,
				g_menu.main.players.chams_enemy_invis.get(),
				true,
				g_menu.main.players.chams_enemy_wireframe.get()
			);
			oFn();
		}
		// enemy visible.
		if (g_menu.main.players.chams_enemy.get(0)) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_enemy_mat.get()],
				g_menu.main.players.chams_enemy_blend.get() / 100.f,
				g_menu.main.players.chams_enemy_vis.get(),
				false,
				g_menu.main.players.chams_enemy_wireframe.get()
			);
			oFn();
			bReturn = true;
		}

		// enemy double.
		if (g_menu.main.players.chams_enemy_double_enable.get()) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_enemy_double_mat.get() + 3],
				g_menu.main.players.chams_enemy_blend.get() / 100.f,
				g_menu.main.players.chams_enemy_double_col.get(),
				g_menu.main.players.chams_enemy.get(1),
				false
			);
			oFn();
		}
	}
	else if (!entity->enemy(g_cl.m_local) && entity != g_cl.m_local && g_menu.main.players.chams_friendly_enable.get()) {
		// friendly xqz.
		if (g_menu.main.players.chams_friendly.get(1)) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_friendly_mat.get()],
				g_menu.main.players.chams_friendly_blend.get() / 100.f,
				g_menu.main.players.chams_friendly_invis.get(),
				true,
				g_menu.main.players.chams_friendly_wireframe.get()
			);
			oFn();
		}
		// friendly visible.
		if (g_menu.main.players.chams_friendly.get(0)) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_friendly_mat.get()],
				g_menu.main.players.chams_friendly_blend.get() / 100.f,
				g_menu.main.players.chams_friendly_vis.get(),
				false,
				g_menu.main.players.chams_friendly_wireframe.get()
			);
			oFn();
			bReturn = true;
		}

		// friendly double.
		if (g_menu.main.players.chams_friendly_double_enable.get()) {
			SetupMaterial(
				m_materials[g_menu.main.players.chams_friendly_double_mat.get() + 3],
				g_menu.main.players.chams_friendly_blend.get() / 100.f,
				g_menu.main.players.chams_friendly_double_col.get(),
				g_menu.main.players.chams_friendly.get(1),
				false
			);
			oFn();
		}
	}

	g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
	g_csgo.m_render_view->SetColorModulation(colors::white);
	g_csgo.m_render_view->SetBlend(1.f);
	return bReturn;
}

// only used for weapon chams lel.
bool Chams::HandleChamsOther(const ModelRenderInfo_t& info, std::function<void()> oFn) {
	if (!g_menu.main.players.chams_weapon_enable.get())
		return false;

	// friendly xqz.
	SetupMaterial(
		m_materials[g_menu.main.players.chams_weapon_mat.get()],
		g_menu.main.players.chams_weapon_blend.get() / 100.f,
		g_menu.main.players.chams_weapon_vis.get(),
		false,
		g_menu.main.players.chams_weapon_wireframe.get()
	);
	oFn();

	// friendly double.
	if (g_menu.main.players.chams_weapon_double_enable.get()) {
		SetupMaterial(
			m_materials[g_menu.main.players.chams_weapon_double_mat.get() + 3],
			g_menu.main.players.chams_weapon_blend.get() / 100.f,
			g_menu.main.players.chams_weapon_double_col.get(),
			false,
			false
		);
		oFn();
	}

	g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
	g_csgo.m_render_view->SetColorModulation(colors::white);
	g_csgo.m_render_view->SetBlend(1.f);
	return true;
}

void Chams::OnDME(void* ECX, uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone) {
	if (!g_cl.m_local)
		return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, bone);

	auto entity = (Player*)(uintptr_t(info.m_renderable) - 0x4);
	if (!entity)
		return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, bone);

	auto oFn = [&]() {
		if (m_override_matrix) {
			return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, m_override_matrix);
		}

		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, bone);
	};

	int fail_sanity = FailsSanity(entity, info);
	if (fail_sanity) {
		// always fails on 7 for this jawn.
		if (g_cl.m_processing && fail_sanity == 7) {
			auto client_class = entity->GetClientClass();

			static const int CPredictedViewModel = 118;

			if (client_class->m_ClassID == CPredictedViewModel) {
				if (HandleChamsOther(info, oFn)) {
					return;
				}
			}
		}

		return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, bone);
	}

	if (entity->IsPlayer()) {
		if (HandleChams(entity, info, oFn)) {
			return;
		}
	}

	g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(ECX, ctx, state, info, bone);
	g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
	g_csgo.m_render_view->SetColorModulation(colors::white);
	g_csgo.m_render_view->SetBlend(1.f);
}

void Chams::AddHitMatrix(Player* player, matrix3x4_t* bones) {
	if (!g_menu.main.players.chams_shot_enable.get())
		return;

	auto& hit = m_hit_matrices.emplace_back();

	std::memcpy(hit.pBoneToWorld, bones, player->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));

	hit.time = g_csgo.m_globals->m_curtime;

	static int m_nSkin = 0xA1C;
	static int m_nBody = 0xA20;

	hit.info.m_origin = player->GetAbsOrigin();
	hit.info.m_angles = player->GetAbsAngles();

	auto renderable = player->renderable();
	if (!renderable)
		return;

	auto model = player->GetModel();
	if (!model)
		return;

	auto hdr = *(studiohdr_t**)(player->GetModelPtr());
	if (!hdr)
		return;

	hit.state.m_pStudioHdr = hdr;
	hit.state.m_pStudioHWData = g_csgo.m_model_cache->GetHardwareData(model->m_studio);
	hit.state.m_pRenderable = renderable;
	hit.state.m_drawFlags = 0;

	hit.info.m_renderable = renderable;
	hit.info.m_model = model;
	hit.info.m_lighting_offset = nullptr;
	hit.info.m_lighting_origin = nullptr;
	hit.info.m_hitboxset = player->m_nHitboxSet();
	hit.info.m_skin = (int)(uintptr_t(player) + m_nSkin);
	hit.info.m_body = (int)(uintptr_t(player) + m_nBody);
	hit.info.m_index = player->index();
	hit.info.m_instance = util::get_method<ModelInstanceHandle_t(__thiscall*)(void*) >(renderable, 30u)(renderable);
	hit.info.m_flags = 0x1;

	hit.info.m_model_to_world = &hit.model_to_world;
	hit.state.m_pModelToWorld = &hit.model_to_world;

	math::AngleMatrix(hit.info.m_angles, hit.info.m_origin, hit.model_to_world);
}
void AddSkelMatrix(Player* player, matrix3x4_t* bone)
{



}

void Chams::OnPSE() {
	if (!g_menu.main.players.chams_shot_enable.get() || !g_csgo.m_engine->IsInGame()) {
		m_hit_matrices.clear();
		return;
	}

	if (m_hit_matrices.empty() || !g_csgo.m_model_render)
		return;

	auto ctx = g_csgo.m_material_system->GetRenderContext();
	if (!ctx)
		return;

	auto it = m_hit_matrices.begin();

	while (it != m_hit_matrices.end()) {
		if (!it->state.m_pModelToWorld || !it->state.m_pRenderable || !it->state.m_pStudioHdr || !it->state.m_pStudioHWData ||
			!it->info.m_renderable || !it->info.m_model_to_world || !it->info.m_model) {
			++it;
			continue;
		}

		auto delta = g_csgo.m_globals->m_curtime - it->time;
		auto hold_time = g_menu.main.players.chams_shot_time.get();

		if (delta > hold_time) {
			it = m_hit_matrices.erase(it);
			continue;
		}

		auto opacity = (g_menu.main.players.chams_shot_blend.get() / 100.f) * std::min(1.f, hold_time - delta);

		g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_shot_mat.get()], 
			opacity, 
			g_menu.main.players.chams_shot_col.get(), 
			true, 
			g_menu.main.players.chams_shot_wireframe.get()
		);
		g_csgo.m_model_render->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);

		if (g_menu.main.players.chams_shot_double_enable.get()) {
			g_chams.SetupMaterial(g_chams.m_materials[g_menu.main.players.chams_shot_double_mat.get() + 3],
				opacity,
				g_menu.main.players.chams_shot_double_col.get(),
				true,
				false
			);
			g_csgo.m_model_render->DrawModelExecute(ctx, it->state, it->info, it->pBoneToWorld);
		}

		g_csgo.m_studio_render->ForcedMaterialOverride(nullptr);
		g_csgo.m_render_view->SetColorModulation(colors::white);
		g_csgo.m_render_view->SetBlend(1.f);
		++it;
	}
}
#include "includes.h"
#define MIN1(a,b) ((a) < (b) ? (a) : (b))

Visuals g_visuals{ };;

void Visuals::ModulateWorld() {
	std::vector< IMaterial* > world, props;

	if (!g_cl.m_local || !g_csgo.m_engine->IsInGame()) {
		return;
	}

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat || mat->IsErrorMaterial())
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}

	if (g_menu.main.visuals.world.get(0)) {
		// dogshit way of doing.
		switch (g_menu.main.visuals.night_color.get()) {
		case 0:
			for (const auto& w : world) {
				w->ColorModulate(0.17f, 0.16f, 0.18f);
			}
			break;
		case 1:
			for (const auto& w : world) {
				w->ColorModulate(0.09f, 0.09f, 0.12f);
			}
			break;
		case 2:
			float darkness = g_menu.main.visuals.night_darkness.get() / 100.f;
			for (const auto& w : world) {
				w->ColorModulate(darkness, darkness, darkness);
			}
			break;
		}

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		for (const auto& p : props) {
			p->ColorModulate(0.5f, 0.5f, 0.5f);
		}
	}
	// disable night.
	else {
		for (const auto& w : world) {
			w->ColorModulate(1.f, 1.f, 1.f);
		}

		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props) {
			p->ColorModulate(1.f, 1.f, 1.f);
		}
	}

	// transparent props.
	if (g_menu.main.visuals.transparent_props.get()) {

		// IsUsingStaticPropDebugModes my nigga
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(0);
		}

		float alpha = g_menu.main.visuals.transparent_props_amount.get() / 100;
		for (const auto& p : props) {
			p->AlphaModulate(alpha);
		}
	}
	// disable transparent props.
	else {
		// restore r_DrawSpecificStaticProp.
		if (g_csgo.r_DrawSpecificStaticProp->GetInt() != -1) {
			g_csgo.r_DrawSpecificStaticProp->SetValue(-1);
		}

		for (const auto& p : props) {
			p->AlphaModulate(1.0f);
		}
	}

	static const auto mat_ambient_light_r = g_csgo.m_cvar->FindVar(HASH("mat_ambient_light_r"));
	static const auto mat_ambient_light_g = g_csgo.m_cvar->FindVar(HASH("mat_ambient_light_g"));
	static const auto mat_ambient_light_b = g_csgo.m_cvar->FindVar(HASH("mat_ambient_light_b"));

	// ambient lighting.
	if (g_menu.main.visuals.world.get(2)) {
		Color c = g_menu.main.visuals.ambientlight_color.get();
		float ambient_niggr = g_menu.main.visuals.ambientlight_darkness.get();
		mat_ambient_light_r->SetValue(c.r() / ambient_niggr);
		mat_ambient_light_g->SetValue(c.g() / ambient_niggr);
		mat_ambient_light_b->SetValue(c.b() / ambient_niggr);
	}
	else {
		mat_ambient_light_r->SetValue(0.f);
		mat_ambient_light_g->SetValue(0.f);
		mat_ambient_light_b->SetValue(0.f);
	}

	static const auto cl_csm_rot_override = g_csgo.m_cvar->FindVar(HASH("cl_csm_rot_override"));
	static const auto cl_csm_rot_x = g_csgo.m_cvar->FindVar(HASH("cl_csm_rot_x"));
	static const auto cl_csm_rot_y = g_csgo.m_cvar->FindVar(HASH("cl_csm_rot_y"));
	static const auto cl_csm_rot_z = g_csgo.m_cvar->FindVar(HASH("cl_csm_rot_z"));

	if (g_menu.main.visuals.world.get(3)) {
		cl_csm_rot_override->SetValue(1);
		cl_csm_rot_x->SetValue(0);
		cl_csm_rot_y->SetValue(0);
		cl_csm_rot_z->SetValue(0);
	}
	else {
		cl_csm_rot_override->SetValue("0");
		cl_csm_rot_x->SetValue("50");
	}
}

void Visuals::ThirdpersonThink() {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;
	static float                   curfrac;

	// for whatever reason overrideview also gets called from the main menu.
	if (!g_csgo.m_engine->IsInGame())
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive();

	// camera should be in thirdperson.
	if (m_thirdperson && g_cl.m_processing) {
		if (!g_csgo.m_input->CAM_IsThirdPerson()) {
			g_csgo.m_input->CAM_ToThirdPerson();
		}
	}
	// camera should be in firstperson.
	else {
		if (g_csgo.m_input->CAM_IsThirdPerson()) {
			g_csgo.m_input->CAM_ToFirstPerson();
			g_csgo.m_input->m_camera_offset.z = 0.f;
		}
		curfrac = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if (g_csgo.m_input->CAM_IsThirdPerson()) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles(offset);

		// get our viewangle's forward directional vector.
		math::AngleVectors(offset, &forward);

		// cam_idealdist convar.
		offset.z = g_menu.main.visuals.thirdperson_distance.get();

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity(g_cl.m_local);

		g_csgo.m_engine_trace->TraceRay(
			Ray(origin, origin - (forward * offset.z), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f }),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp(tr.m_fraction, 0.f, 1.f);
		curfrac = math::Lerp(VIS_INTERP, curfrac, tr.m_fraction);
		offset.z *= g_menu.main.visuals.thirdperson_interpolate.get() ? curfrac : tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

// meme...
void Visuals::IndicateAngles()
{
	/*
	if (!g_csgo.m_engine->IsInGame() && !g_csgo.m_engine->IsConnected())
		return;

	if (!g_menu.main.antiaim.draw_angles.get())
		return;

	if (!	g_csgo.m_input->CAM_IsThirdPerson())
		return;

	if (!g_cl.m_local || g_cl.m_local->m_iHealth() < 1)
		return;

	const auto &pos = g_cl.m_local->GetRenderOrigin();
	vec2_t tmp;

	if (render::WorldToScreen(pos, tmp))
	{
		vec2_t draw_tmp;
		const vec3_t real_pos(50.f * std::cos(math::deg_to_rad(g_cl.m_radar.y)) + pos.x, 50.f * sin(math::deg_to_rad(g_cl.m_radar.y)) + pos.y, pos.z);

		if (render::WorldToScreen(real_pos, draw_tmp))
		{
			render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 0, 255, 0, 255 });
			render::esp_small.string(draw_tmp.x, draw_tmp.y, { 0, 255, 0, 255 }, "FAKE", render::ALIGN_LEFT);
		}

		if (g_menu.main.antiaim.fake_yaw.get())
		{
			const vec3_t fake_pos(50.f * cos(math::deg_to_rad(g_cl.m_angle.y)) + pos.x, 50.f * sin(math::deg_to_rad(g_cl.m_angle.y)) + pos.y, pos.z);

			if (render::WorldToScreen(fake_pos, draw_tmp))
			{
				render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 });
				render::esp_small.string(draw_tmp.x, draw_tmp.y, { 255, 0, 0, 255 }, "REAL", render::ALIGN_LEFT);
			}
		}

		if (g_menu.main.antiaim.body_fake_stand.get() == 1 || g_menu.main.antiaim.body_fake_stand.get() == 2 || g_menu.main.antiaim.body_fake_stand.get() == 3 || g_menu.main.antiaim.body_fake_stand.get() == 4 || g_menu.main.antiaim.body_fake_stand.get() == 5 || g_menu.main.antiaim.body_fake_stand.get() == 6)
		{
			float lby = g_cl.m_local->m_flLowerBodyYawTarget();
			const vec3_t lby_pos(50.f * cos(math::deg_to_rad(lby)) + pos.x,
				50.f * sin(math::deg_to_rad(lby)) + pos.y, pos.z);

			if (render::WorldToScreen(lby_pos, draw_tmp))
			{
				render::line(tmp.x, tmp.y, draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 });
				render::esp_small.string(draw_tmp.x, draw_tmp.y, { 255, 255, 255, 255 }, "LBY", render::ALIGN_LEFT);
			}
		}
	}*/
}

void Visuals::Hitmarker() {
	static auto cross = g_csgo.m_cvar->FindVar(HASH("weapon_debug_spread_show"));
	cross->SetValue(g_menu.main.visuals.force_xhair.get() && !g_cl.m_local->m_bIsScoped() ? 3 : 0);

	if (g_menu.main.misc.hitmarkers.GetActiveIndices().empty())
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end || m_hit_duration <= 0.f)
		return;

	auto DrawHitmarker = [&](int x, int y, float complete, float alpha) -> void {
		float anim = math::Lerp(1.f - complete, 0.f, 10.f);
		Color color = { 255, 255, 255, int(alpha) };

		// animated.
		if (g_menu.main.misc.hitmarker_style.get() == 0) {
			render::rect_filled(x + anim + 6, y + anim + 6, 1, 1, color);
			render::rect_filled(x + anim + 7, y + anim + 7, 1, 1, color);
			render::rect_filled(x + anim + 8, y + anim + 8, 1, 1, color);
			render::rect_filled(x + anim + 9, y + anim + 9, 1, 1, color);
			render::rect_filled(x + anim + 10, y + anim + 10, 1, 1, color);

			render::rect_filled(x - anim - 6, y - anim - 6, 1, 1, color);
			render::rect_filled(x - anim - 7, y - anim - 7, 1, 1, color);
			render::rect_filled(x - anim - 8, y - anim - 8, 1, 1, color);
			render::rect_filled(x - anim - 9, y - anim - 9, 1, 1, color);
			render::rect_filled(x - anim - 10, y - anim - 10, 1, 1, color);

			render::rect_filled(x - anim - 6, y + anim + 6, 1, 1, color);
			render::rect_filled(x - anim - 7, y + anim + 7, 1, 1, color);
			render::rect_filled(x - anim - 8, y + anim + 8, 1, 1, color);
			render::rect_filled(x - anim - 9, y + anim + 9, 1, 1, color);
			render::rect_filled(x - anim - 10, y + anim + 10, 1, 1, color);

			render::rect_filled(x + anim + 6, y - anim - 6, 1, 1, color);
			render::rect_filled(x + anim + 7, y - anim - 7, 1, 1, color);
			render::rect_filled(x + anim + 8, y - anim - 8, 1, 1, color);
			render::rect_filled(x + anim + 9, y - anim - 9, 1, 1, color);
			render::rect_filled(x + anim + 10, y - anim - 10, 1, 1, color);
		}
		// static.
		else {
			const int line = 6;
			render::line(x - line, y - line, x - (line / 4), y - (line / 4), color);
			render::line(x - line, y + line, x - (line / 4), y + (line / 4), color);
			render::line(x + line, y + line, x + (line / 4), y + (line / 4), color);
			render::line(x + line, y - line, x + (line / 4), y - (line / 4), color);
		}
	};

	int x = g_cl.m_width / 2;
	int y = g_cl.m_height / 2;

	if (g_menu.main.misc.hitmarkers.get(1)) {
		float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
		int alpha = (1.f - complete) * 255;

		DrawHitmarker(x, y, complete, alpha);
	}

	if (g_menu.main.misc.hitmarkers.get(0)) {
		if (m_hitmarkers.size() == 0)
			return;

		for (int i = 0; i < m_hitmarkers.size(); i++) {
			float complete = (g_csgo.m_globals->m_curtime - m_hitmarkers[i].time) / m_hit_duration;
			int alpha = (1.f - complete) * 255;

			if (alpha <= 0.f || m_hitmarkers[i].time > g_csgo.m_globals->m_curtime) {
				m_hitmarkers.erase(m_hitmarkers.begin() + i);
				continue;
			}

			vec2_t pos2D;
			if (!render::WorldToScreen(m_hitmarkers[i].pos, pos2D))
				continue;

			DrawHitmarker(pos2D.x, pos2D.y, complete, alpha);
		}
	}
}


void Visuals::NoSmoke() {
	if (!g_menu.main.visuals.vis_removals.get(1))
		return;
	//colado https://www.unknowncheats.me/forum/counterstrike-global-offensive/262635-epic-wireframe-smoke.html
	std::vector<const char*> vistasmoke_mats =
	{
			"particle/vistasmokev1/vistasmokev1_fire",
			"particle/vistasmokev1/vistasmokev1_smokegrenade",
			"particle/vistasmokev1/vistasmokev1_emods",
			"particle/vistasmokev1/vistasmokev1_emods_impactdust",
	};

	for (auto mat_s : vistasmoke_mats)
	{
		IMaterial* mat = g_csgo.m_material_system->FindMaterial(mat_s, XOR("Other textures"));
		mat->SetFlag(MATERIAL_VAR_NO_DRAW, true);
	}
}

void Visuals::think() {
	// don't run anything if our local player isn't valid.
	if (!g_cl.m_local)
		return;

	if (g_menu.main.aimbot.aimbot_debug.get(2) && !g_aimbot.m_debug_render.empty()) {
		int height = 50;

		for (int i = 0; i < g_aimbot.m_debug_render.size(); i++) {
			render::menu.string(g_cl.m_width - 20, height, Color(255, 255, 255), g_aimbot.m_debug_render[i], render::ALIGN_RIGHT);
			height += render::menu.size(g_aimbot.m_debug_render[i]).m_height + 1;
		}
	}

	if (g_menu.main.visuals.vis_removals.get(4)
		&& g_cl.m_local->alive()
		&& g_cl.m_local->GetActiveWeapon()
		&& g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped()) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt();

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if (size > 1) {
			x -= (size / 2);
			y -= (size / 2);
		}

		// draw our lines.
		render::rect_filled(0, y, w, size, colors::black);
		render::rect_filled(x, 0, size, h, colors::black);
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		draw(ent);
	}

	// draw everything else.
	SpreadCrosshair();
	StatusIndicators();
	Spectators();
	PenetrationCrosshair();
	ManualAntiAim();
	Hitmarker();
	DrawPlantedC4();
	DrawAutopeek();

	//nade prediction stuff
	auto& predicted_nades = g_grenades_pred.get_list();

	static auto last_server_tick = g_csgo.m_cl->m_server_tick;
	if (g_csgo.m_cl->m_server_tick != last_server_tick) {
		predicted_nades.clear();
		last_server_tick = g_csgo.m_cl->m_server_tick;
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		if (ent->dormant())
			continue;

		if (!ent->is(HASH("CMolotovProjectile"))
			&& !ent->is(HASH("CBaseCSGrenadeProjectile")))
			continue;

		if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
			const auto studio_model = ent->GetModel();
			if (!studio_model
				|| std::string_view(studio_model->m_name).find("fraggrenade") == std::string::npos)
				continue;
		}

		const auto handle = reinterpret_cast<Player*>(ent)->GetRefEHandle();

		if (ent->m_fEffects() & EF_NODRAW) {
			predicted_nades.erase(handle);

			continue;
		}

		if (predicted_nades.find(handle) == predicted_nades.end()) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(handle),
				std::forward_as_tuple(
					reinterpret_cast<Player*>(g_csgo.m_entlist->GetClientEntityFromHandle(ent->m_hThrower())),
					ent->is(HASH("CMolotovProjectile")) ? MOLOTOV : HEGRENADE,
					ent->m_vecOrigin(), ent->m_vecVelocity(), ent->m_flSpawnTime_Grenade(),
					game::TIME_TO_TICKS(reinterpret_cast<Player*>(ent)->m_flSimulationTime() - ent->m_flSpawnTime_Grenade())
				)
			);
		}

		if (predicted_nades.at(handle).draw())
			continue;

		predicted_nades.erase(handle);
	}

	g_grenades_pred.get_local_data().draw();
}

void Visuals::Spectators() {
	if (!g_menu.main.visuals.spectators.get())
		return;

	std::vector< std::string > spectators{ XOR("spectators") };
	int h = render::menu_shade.m_size.m_height;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (player->m_bIsLocalPlayer())
			continue;

		if (player->m_lifeState() == LIFE_ALIVE)
			continue;

		if (player->dormant())
			continue;

		if (g_cl.m_processing) {
			if (player->GetObserverTarget() != g_cl.m_local) {
				continue;
			}
		}
		else {
			if (player->GetObserverTarget() != g_cl.m_local->GetObserverTarget()) {
				continue;
			}
		}

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(i, &info))
			continue;

		spectators.push_back(std::string(info.m_name).substr(0, 24));
	}

	if (spectators.size() < 2)
		return;

	size_t total_size = spectators.size() * (h - 1);

	for (size_t i{ }; i < spectators.size(); ++i) {
		const std::string& name = spectators[i];

		render::menu_shade.string(g_cl.m_width - 20, (g_cl.m_height / 2) - (total_size / 2) + (i * (h - 1)), { 255, 255, 255, 179 }, name, render::ALIGN_RIGHT);
	}
}

void Visuals::StatusIndicators() {
	if (!g_cl.m_processing)
		return;

	static std::array< std::string, 6 > indicators{ };
	static std::array< float, 6 > indicators_alpha{};

	static const float goal_alpha = 180.f;
	Color ind_col = g_menu.main.visuals.indicators_color.get();

	// lby.
	{
		indicators[0] = { XOR("LBY") };
		indicators_alpha[0] = std::abs(math::NormalizedAngle(g_cl.m_body_lol - g_cl.m_angle.y)) > 35.f ? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[0], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[0], 0.f));
	}

	// lc.
	{
		indicators[1] = { XOR("LC") };
		indicators_alpha[1] = g_cl.m_lagcomp ? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[1], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[1], 0.f));
	}

	// fake ping.
	{
		indicators[2] = { XOR("PING") };
		indicators_alpha[2] = (g_aimbot.m_fake_latency || g_menu.main.misc.fake_latency_always.get()) ? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[2], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[2], 0.f));
	}

	// damage override.
	{
		indicators[3] = { XOR("DMG") };
		indicators_alpha[3] = (g_menu.main.aimbot.override_damage_mode.get() ? g_input.GetKeyState(g_menu.main.aimbot.override_damage.get()) : g_aimbot.m_damage_override_toggle) 
			? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[3], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[3], 0.f));
	}

	// autopeek.
	{
		indicators[4] = { XOR("PEEK") };
		indicators_alpha[4] = g_input.GetKeyState(g_menu.main.movement.autopeek.get()) ? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[4], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[4], 0.f));
	}

	// force baim.
	{
		indicators[5] = { XOR("BAIM") };
		indicators_alpha[5] = g_input.GetKeyState(g_menu.main.aimbot.baim_key.get()) ? std::ceil(math::Lerp(VIS_INTERP, indicators_alpha[5], goal_alpha)) : std::floor(math::Lerp(VIS_INTERP, indicators_alpha[5], 0.f));
	}

	if (indicators.empty())
		return;

	float font_height = render::esp.m_size.m_height;
	float offset = 10.f;
	vec2_t screen_pos = { g_cl.m_width / 2, g_cl.m_height / 2 };

	for (size_t i{ }; i < indicators.size(); ++i) {
		if (!g_menu.main.visuals.indicators.get(i))
			continue;

		auto& indicator = indicators[i];
		if (!&indicator)
			continue;

		float alpha = indicators_alpha[i];
		float nigga_interp = alpha / goal_alpha;

		ind_col.a() = alpha;

		float nigga_offset = nigga_interp * font_height;

		render::indicator.string(5, screen_pos.y + offset + nigga_offset, ind_col, indicator, render::ALIGN_LEFT);
		offset += nigga_offset;
	}
}

/*void Visuals::SpreadCrosshair() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	if (!g_menu.main.visuals.spread_xhair.get())
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if (!data)
		return;

	// do not do this on: bomb, knife and nades.
	int type = weapon->m_iItemDefinitionIndex();
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;

	// calc radius.
	float radius = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / (std::tan(math::deg_to_rad(g_cl.m_local->GetFOV()) * 0.5f) + FLT_EPSILON);

	// scale by screen size.
	radius *= g_cl.m_height * (1.f / 480.f);

	// get color.
	Color col = g_menu.main.visuals.spread_xhair_col.get();

	// modify alpha channel.
	col.a() = 200 * (g_menu.main.visuals.spread_xhair_blend.get() / 100.f);

	int segements = std::max(16, (int)std::round(radius * 0.75f));
	render::circle(g_cl.m_width / 2, g_cl.m_height / 2, radius, segements, col);
}*/

void Visuals::SpreadCrosshair() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	if (!g_menu.main.visuals.spread_xhair.get())
		return;

	// get active weapon.
	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* data = weapon->GetWpnData();
	if (!data)
		return;

	// do not do this on: bomb, knife and nades.
	int type = weapon->m_iItemDefinitionIndex();
	if (type == WEAPONTYPE_KNIFE || type == WEAPONTYPE_C4 || type == WEAPONTYPE_GRENADE)
		return;

	int w, h;
	g_csgo.m_engine->GetScreenSize(w, h);

	float spreadDist = ((weapon->GetInaccuracy() + weapon->GetSpread()) * 320.f) / std::tan(math::deg_to_rad(g_cl.m_local->GetFOV()) * 0.5f);
	float spreadRadius = (spreadDist * (h / 480.f)) * 50 / 250.f;


	for (float i = 0; i <= spreadRadius; i++)
	{
		Color col = g_menu.main.visuals.spread_xhair_col.get();
		col.a() = (static_cast<int>(i * (255.f / spreadRadius)) * g_menu.main.visuals.spread_xhair_blend.get() / 100.f);
		g_csgo.m_surface->DrawSetColor(col);
		g_csgo.m_surface->DrawOutlinedCircle(w / 2, h / 2, static_cast<int>(i), 240);
	}
}

void RenderPolygonSup(float x, float y, float rotation, Color color) {

	Vertex verts[3];

	//ignore SGHUIT CODE i was lazy affffffffffff idgaf abt i looking good
	float size = 0.8f;

	verts[0] = { x, y };        // 0,  0
	verts[1] = { x - (12.f * size), y + (24.f * size) }; // -1, 1
	verts[2] = { x + (12.f * size), y + (24.f * size) }; // 1,  1


	verts[0] = render::RotateVertex(vec2_t(x, y), verts[0], rotation * 90);
	verts[1] = render::RotateVertex(vec2_t(x, y), verts[1], rotation * 90);
	verts[2] = render::RotateVertex(vec2_t(x, y), verts[2], rotation * 90);


	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawTexturedPolygon(3, verts);

}

void Visuals::ManualAntiAim() {
	if (!g_cl.m_processing || !g_menu.main.antiaim.manual_arrows.get())
		return;

	Color color = g_menu.main.antiaim.manual_arrows_color.get();

	int x = g_cl.m_width / 2;
	int y = g_cl.m_height / 2;

	switch (g_hvh.m_manual_direction) {
	case 1: // forward
		RenderPolygonSup(x, y - 60, 4, color);
		break;
	case 2: // left
		RenderPolygonSup(x - 60, y, 3, color);
		break;
	case 3: // right
		RenderPolygonSup(x, y + 60, 2, color);
		break;
	case 4: // back
		RenderPolygonSup(x + 60, y, 1, color);
		break;
	}
}

void Visuals::PenetrationCrosshair() {
	int   x, y;
	bool  valid_player_hit;
	Color final_color;

	if (!g_menu.main.visuals.pen_crosshair.get() || !g_cl.m_processing)
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;


	valid_player_hit = (g_cl.m_pen_data.m_target && g_cl.m_pen_data.m_target->enemy(g_cl.m_local));
	if (valid_player_hit)
		final_color = colors::light_blue;

	else if (g_cl.m_pen_data.m_pen)
		final_color = colors::transparent_green;

	else
		final_color = colors::transparent_red;

	// todo - dex; use fmt library to get damage string here?
	//             draw damage string?

	// draw small square in center of screen.
	int damage1337 = g_cl.m_pen_data.m_damage;

	if (g_menu.main.visuals.pen_damage.get() && (g_cl.m_pen_data.m_pen || valid_player_hit))
		render::esp_small.string(x + 3, y + 2, { final_color }, std::to_string(damage1337).c_str(), render::ALIGN_LEFT);
	if (g_cl.m_pen_data.m_damage > 1) {
		render::rect_filled(x - 1, y, 1, 1, { final_color });
		render::rect_filled(x, y, 1, 1, { final_color });
		render::rect_filled(x + 1, y, 1, 1, { final_color });
		render::rect_filled(x, y + 1, 1, 1, { final_color });
		render::rect_filled(x, y - 1, 1, 1, { final_color });
		//shadow
		render::rect_filled(x - 2, y, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 1, y - 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 2, y, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x, y + 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x, y - 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 1, y - 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 1, y + 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 2, y + 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 1, y + 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 1, y + 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 2, y + 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 1, y + 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 1, y - 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 1, y - 2, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x - 2, y - 1, 1, 1, { 0,0, 0, 125 });
		render::rect_filled(x + 2, y - 1, 1, 1, { 0,0, 0, 125 });
	}
}

void Visuals::draw(Entity* ent) {
	if (ent && ent->IsPlayer()) {
		Player* player = ent->as< Player* >();
		if (!player || player->m_bIsLocalPlayer())
			return;

		// draw player esp.
		DrawPlayer(player);
	}

	else if (g_menu.main.visuals.items.get() && ent->IsBaseCombatWeapon() && !ent->dormant())
		DrawItem(ent->as< Weapon* >());

	else if (g_menu.main.visuals.proj.get())
		DrawProjectile(ent->as< Weapon* >());
}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	if (!g_cl.m_local)
		return;

	Player* thrower = (Player*)g_csgo.m_entlist->GetClientEntityFromHandle(ent->m_hOwnerEntity());
	if (!thrower)
		return;

	bool safe_grenade_ting = true;
	if (thrower->enemy(g_cl.m_local) || thrower == g_cl.m_local) {
		safe_grenade_ting = false;
	}

	Color col = safe_grenade_ting ? g_menu.main.visuals.proj_safe_color.get() : g_menu.main.visuals.proj_dangerous_color.get();

	if (ent->is(HASH("CHostage"))) {
		std::string distance;
		int dist = (((ent->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
		if (dist < 150) {
			render::esp.string(screen.x, screen.y, colors::white, XOR("hostage"), render::ALIGN_CENTER);
		}
	}

	// find classes.
	if (ent->is(HASH("CInferno"))) {
		// get molotov info.
		const double spawn_time = *(float*)(uintptr_t(ent) + 0x20);
		const double factor = ((spawn_time + 7.031) - g_csgo.m_globals->m_curtime) / 7.031;

		if (factor < 0 || spawn_time < 0.f)
			return;

		float time_since_explosion = g_csgo.m_globals->m_interval * (g_csgo.m_globals->m_tick_count - spawn_time);

		// setup our vectors.
		vec3_t mins, maxs;

		// get molotov bounds (current radius).
		ent->GetRenderBounds(mins, maxs);

		float radius = (maxs - mins).length_2d() * 0.5;
		//radius *= (std::min(time_since_explosion, 0.1f) / 0.1f); molotov radius actually kinda matters to see so maybe dont interpolate it..

		// render the molotov range circle.
		float alpha_factor = factor <= 0.1f ? factor * 10.f : 1.f;
		col.a() = (80.f) * alpha_factor;
		render::world_circle(origin, radius, 0.f, col);
		col.a() = (180.f) * alpha_factor;
		render::WorldCircleOutline(origin, radius, 0.f, col);
	

		// name.
		//col.a() = 255.f;
		render::esp.string(screen.x, screen.y, col, XOR("fire"), render::ALIGN_CENTER);
	}

	else if (ent->is(HASH("CSmokeGrenadeProjectile"))) {
		// get smoke info.
		const float spawn_time = game::TICKS_TO_TIME(ent->m_nSmokeEffectTickBegin());
		const double factor = ((spawn_time + 18.041) - g_csgo.m_globals->m_curtime) / 18.041;

		// smokes are never dangerous lol.
		col = g_menu.main.visuals.proj_safe_color.get();

		// make sure the smoke effect has started
		if (spawn_time >= 0.f && factor >= 0.f) {
			float radius = 144.f;
			auto time_since_explosion = g_csgo.m_globals->m_interval * (g_csgo.m_globals->m_tick_count - ent->m_nSmokeEffectTickBegin());

			radius *= (std::min(time_since_explosion, 0.1f) / 0.1f);

			// render the smoke range circle.
			// idk why lol it was always this much too far ahead... just hard code adjust for it ig.
			float alpha_factor = (factor - 0.031812f) <= 0.1f ? (factor - 0.031812) * 10.f : 1.f;
			col.a() = (80.f) * alpha_factor;
			render::world_circle(origin, radius, 0.f, col);
			col.a() = (180.f) * alpha_factor;
			render::WorldCircleOutline(origin, radius, 0.f, col);
		}

		//col.a() = 255.f;
		render::esp.string(screen.x, screen.y, col, XOR("smoke"), render::ALIGN_CENTER);
	}
	else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
		// get smoke info.
		//const float spawn_time = game::TICKS_TO_TIME(ent->m_nSmokeEffectTickBegin());
		//const double factor = ((spawn_time + 18.041) - g_csgo.m_globals->m_curtime) / 18.041;

		// smokes are never dangerous lol.
		//col = g_menu.main.visuals.proj_safe_color.get();


			// render the smoke range circle.
			// idk why lol it was always this much too far ahead... just hard code adjust for it ig.
			//float alpha_factor = (factor - 0.031812f) <= 0.1f ? (factor - 0.031812) * 10.f : 1.f;
			//col.a() = (80.f) * alpha_factor;
		//	render::world_circle(origin, radius, 0.f, col);
		//	col.a() = (180.f) * alpha_factor;
			//render::WorldCircleOutline(origin, radius, 0.f, col);

		//col.a() = 255.f;
		render::esp.string(screen.x, screen.y, col, XOR("frag"), render::ALIGN_CENTER);
	}
}

void Visuals::DrawItem(Weapon* item) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(item->m_hOwnerEntity());
	if (owner)
		return;

	std::string distance;
	int dist = (((item->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
	//if (dist > 0)
	//distance = tfm::format(XOR("%i FT"), dist);
	if (dist > 0) {
		if (dist > 5) {
			while (!(dist % 5 == 0)) {
				dist = dist - 1;
			}

			if (dist % 5 == 0)
				distance = tfm::format(XOR("%i FT"), dist);
		}
		else
			distance = tfm::format(XOR("%i FT"), dist);
	}

		int closest = 20; //distance at which alpha maxes out
		int maxdist = 50; //distance at which alpha hits 0
		int maxalpha = 180; //maximum alpha value

		//calculate alpha on dist
		int weaponalpha = (maxalpha * (maxdist - dist)) / maxdist;

		//ensure alpha value does not exceed maximum

		weaponalpha = std::min(weaponalpha, maxalpha);
		if (dist > 50) {
			weaponalpha = 0;
		}

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	WeaponInfo* data = item->GetWpnData();
	if (!data)
		return;

	Color col = g_menu.main.visuals.item_color.get();
	col.a() = weaponalpha; // want to be fully invisible >50 feet

	Color col1337 = g_menu.main.visuals.dropammo_color.get();
	col1337.a() = weaponalpha;

	// render bomb in green.
	if (item->is(HASH("CC4")))

		render::esp_small.string(screen.x, screen.y, { 150, 200, 60, 0xb4 }, XOR("BOMB"), render::ALIGN_CENTER);

	// if not bomb
	// normal item, get its name.
	else {
		std::string name{ item->GetLocalizedName() };
		std::transform(name.begin(), name.end(), name.begin(), ::toupper);

		if (g_menu.main.visuals.distance.get())
			render::esp_small.string(screen.x, screen.y - 8, col, distance, render::ALIGN_CENTER);
		render::esp_small.string(screen.x, screen.y, col, name, render::ALIGN_CENTER);
	}

	if (!g_menu.main.visuals.ammo.get())
		return;

	// nades do not have ammo.
	if (data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE)
		return;

	if (item->m_iItemDefinitionIndex() == 0 || item->m_iItemDefinitionIndex() == C4)
		return;

	std::string ammo = tfm::format(XOR("(%i/%i)"), item->m_iClip1(), item->m_iPrimaryReserveAmmoCount());
	//render::esp_small.string( screen.x, screen.y - render::esp_small.m_size.m_height - 1, col, ammo, render::ALIGN_CENTER );

	int current = item->m_iClip1();
	int max = data->m_max_clip1;
	float scale = (float)current / max;
	int bar = (int)std::round((51 - 2) * scale);
	render::rect_filled(screen.x - 25, screen.y + 11, 51, 4, { 0,0,0,col1337.a() });
	render::rect_filled(screen.x - 25 + 1, screen.y + 11 + 1, bar, 2, col1337);

	/*if (g_csgo.m_entlist->GetClientEntity(83)) {
		/*vec2_t screen;
		vec3_t origin = g_csgo.m_entlist->GetClientEntity(83)->GetAbsOrigin();
		if (!render::WorldToScreen(origin, screen))
			return;

		vec2_t screen;
		vec3_t origin = g_csgo.m_entlist->GetClientEntity(83)->GetAbsOrigin();
		if (!render::WorldToScreen(origin, screen))
			return;

		render::esp_small.string(screen.x, screen.y, colors::light_blue, XOR("TEST"), render::ALIGN_CENTER);
	}*/
}

void Visuals::OffScreen(Player* player, int alpha) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[3], verts_outline[3];
	Color  color;

	// todo - dex; move this?
	static auto get_offscreen_data = [](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation) {
		ang_t  view_angles(g_csgo.m_view_render->m_view.m_angles);
		vec3_t fwd, right, up(0.f, 0.f, 1.f);
		float  front, side, yaw_rad, sa, ca;

		// get viewport angles forward directional vector.
		math::AngleVectors(view_angles, &fwd);

		// convert viewangles forward directional vector to a unit vector.
		fwd.z = 0.f;
		fwd.normalize();

		// calculate front / side positions.
		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		// setup offscreen position.
		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		// get the rotation ( yaw, 0 - 360 ).
		out_rotation = math::rad_to_deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::pi);

		// get needed sine / cosine values.
		yaw_rad = math::deg_to_rad(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		// rotate offscreen position around.
		out_offscreen_pos.x = (int)((g_cl.m_width / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((g_cl.m_height / 2.f) - (radius * ca));
	};

	if (!g_menu.main.players.offscreen.get())
		return;

	if (!g_cl.m_processing || !g_cl.m_local->enemy(player))
		return;

	// get the player's center screen position.
	target_pos = player->WorldSpaceCenter();
	is_on_screen = render::WorldToScreen(target_pos, screen_pos);

	// give some extra room for screen position to be off screen.
	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	// origin is not on the screen at all, get offscreen position data and start rendering.
	if (!is_on_screen
		|| screen_pos.x < -leeway_x
		|| screen_pos.x >(g_cl.m_width + leeway_x)
		|| screen_pos.y < -leeway_y
		|| screen_pos.y >(g_cl.m_height + leeway_y)) {

		float size = g_menu.main.config.offscreen_mode.get() / 100.f;
		float pos = g_menu.main.config.offscreen_mode1.get();

		// get viewport origin.
		view_origin = g_csgo.m_view_render->m_view.m_origin;

		// get direction to target.
		delta = (target_pos - view_origin).normalized();

		// note - dex; this is the 'YRES' macro from the source sdk.
		radius = pos * (g_cl.m_height / 480.f);

		// get the data we need for rendering.
		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);

		// bring rotation back into range... before rotating verts, sine and cosine needs this value inverted.
		// note - dex; reference: 
		// https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/src_main/game/client/tf/tf_hud_damageindicator.cpp#L182
		offscreen_rotation = -offscreen_rotation;

		// setup vertices for the triangle.
		verts[0] = { offscreen_pos.x + (1 * size) , offscreen_pos.y + (1 * size) };        // 0,  0
		verts[1] = { offscreen_pos.x - (12.f * size), offscreen_pos.y + (24.f * size) }; // -1, 1
		verts[2] = { offscreen_pos.x + (12.f * size), offscreen_pos.y + (24.f * size) }; // 1,  1

		// setup verts for the triangle's outline.
		verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
		verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
		verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

		// rotate all vertices to point towards our target.
		verts[0] = render::RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = render::RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = render::RotateVertex(offscreen_pos, verts[2], offscreen_rotation);

		// render!
		int alpha1337 = sin(abs(fmod(-math::pi + (g_csgo.m_globals->m_curtime * (2 / .75)), (math::pi * 2)))) * 255;

		if (alpha1337 < 0)
			alpha1337 = alpha1337 * (-1);

		color = g_menu.main.players.offscreen_color.get(); // damage_data.m_color;
		color.a() = (alpha == 255) ? alpha1337 : alpha / 2;

		g_csgo.m_surface->DrawSetColor(color);
		g_csgo.m_surface->DrawTexturedPolygon(3, verts);

	}
}

void Visuals::DrawPlayer(Player* player) {
	constexpr float MAX_DORMANT_TIME = 11.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;

	Rect		  box;
	player_info_t info;
	Color		  color;

	// get player index.
	int index = player->index();

	AimPlayer* data = &g_aimbot.m_players[index - 1];
	if (!data) {
		return;
	}

	// get reference to array variable.
	float& opacity = m_opacities[index - 1];
	float& health_draw = m_health_draw[index - 1];
	esp_interpolation& esp_bruh = m_esp_interpolation[index - 1];

	// opacity should reach 1 in 300 milliseconds.
	constexpr int frequency = 1.f / 0.2f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->enemy(g_cl.m_local);
	bool dormant = player->dormant();

	if (g_menu.main.visuals.enemy_radar.get() && enemy && !dormant)
		player->m_bSpotted() = true;

	// fade out when dead.
	if (!player->alive()) {
		if (g_menu.main.players.esp_extras.get(1)) {
			opacity -= step;
		}
		else {
			opacity = 0.0f;
		}
	}
	else {
		// if non-dormant	-> increment
        // if dormant		-> decrement
		dormant ? opacity -= step : opacity += step;
	}

	// clamp the opacity.
	math::clamp(opacity, 0.f, 1.f);

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && g_menu.main.players.dormant.get();
	if (!opacity && !dormant_esp)
		return;

	// stay for x seconds max.
	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime();
	if (dormant && dt > MAX_DORMANT_TIME)
		return;

	// calculate alpha channels.
	int alpha = (int)(180.f * opacity);
	int low_alpha = (int)(180.f * opacity);

	// get color based on enemy or not.
	color = enemy ? g_menu.main.players.box_enemy.get() : g_menu.main.players.box_friendly.get();

	if (dormant && dormant_esp) {
		alpha = 112;
		low_alpha = 80;

		// fade.
		if (dt > DORMANT_FADE_TIME) {
			// for how long have we been fading?
			float faded = (dt - DORMANT_FADE_TIME);
			float scale = 1.f - (faded / DORMANT_FADE_TIME);

			alpha *= scale;
			low_alpha *= scale;
		}

		// override color.
		color = { 130, 130, 130 };
	}

	// override alpha.
	color.a() = alpha;

	// get player info.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	// run offscreen ESP.
	OffScreen(player, alpha);

	// attempt to get player box.
	if (!GetPlayerBoxRect(player, box)) {
		// OffScreen( player );
		return;
	}

	DebugAimbotPoints( player );

	bool bone_esp = (enemy && g_menu.main.players.skeleton.get(0)) || (!enemy && g_menu.main.players.skeleton.get(1));
	if (bone_esp)
		DrawSkeleton(player, alpha);

	// is box esp enabled for this player.
	bool box_esp = (enemy && g_menu.main.players.box.get(0)) || (!enemy && g_menu.main.players.box.get(1));

	// render box if specified.
	if (box_esp)
		render::rect_outlined(box.x, box.y, box.w, box.h, color, { 10, 10, 10, low_alpha });

	// is name esp enabled for this player.
	bool name_esp = (enemy && g_menu.main.players.name.get(0)) || (!enemy && g_menu.main.players.name.get(1));

	// draw name.
	if (name_esp) {
		std::string nameniggr = info.m_name;
		std::string name = nameniggr.substr(0, 12);

		if (data->m_voice_cheat == CHEAT_GOOFYHOOK) {
			// set the networked player name.
			name = g_cl.GetClientName(data->m_goofy_id);
		}
		else {
			if (name.length() < nameniggr.length()) {
				name += XOR("...");
			}
		}

		Color clr = g_menu.main.players.name_color.get();
		if (dormant) {
			clr.r() = 130;
			clr.g() = 130;
			clr.b() = 130;
		}
		// override alpha.
		clr.a() = low_alpha;

		render::Goofy.string(box.x + box.w / 2, box.y - render::esp.m_size.m_height, clr, name, render::ALIGN_CENTER);
	}

	// is health esp enabled for this player.
	bool health_esp = (enemy && g_menu.main.players.health.get(0)) || (!enemy && g_menu.main.players.health.get(1));

	if (health_esp) {
		int y = box.y + 1;
		int h = box.h - 2;

		// retarded servers that go above 100 hp..
		int hEALTH = std::min(100, player->m_iHealth());
		if (g_menu.main.players.esp_extras.get(1)) {
			health_draw = math::Lerp(step, health_draw, float(hEALTH));
		}
		else {
			health_draw = hEALTH;
		}

		// calculate hp bar color.
		auto firstcol = g_menu.main.players.health_starting_color.get();
		auto secondcol = g_menu.main.players.health_ending_color.get();
		auto hpbarcol = dormant ? Color(130, 130, 130, alpha) : Color(
			math::Lerp(hEALTH * 0.01f, int(secondcol.r()), int(firstcol.r())),
			math::Lerp(hEALTH * 0.01f, int(secondcol.g()), int(firstcol.g())),
			math::Lerp(hEALTH * 0.01f, int(secondcol.b()), int(firstcol.b())),
			alpha
		);

		// get hp bar height.
		int fill = (int)std::round(health_draw * h / 100.f);

		// render glow first.
		if (g_menu.main.players.esp_extras.get(0)) {
			auto colballs1 = hpbarcol;
			auto colballs2 = colballs1;
			colballs1.a() = 95 * (alpha / 255.f);
			colballs2.a() = 0;
			
			render::gradient((box.x - 6) + 4, y - 2, 2, h + 4, colballs1, colballs2, true); // right.
			render::gradient((box.x - 6) - 2, y - 2, 2, h + 4, colballs2, colballs1, true); // left.

			render::gradient(box.x - 7, y + h + 2, 6, 2, colballs1, colballs2, false); // bottom.
			render::gradient(box.x - 7, y - 4, 6, 2, colballs2, colballs1, false); // top.
		}

		// render background.
		render::rect_filled(box.x - 6, y - 2, 4, h + 4, { 10, 10, 10, low_alpha });
		
		// render actual bar.
		render::rect(box.x - 5, y + h - fill - 1, 2, fill + 2, hpbarcol);

		// if hp blasdnoasdasd.
		if (g_menu.main.players.health_number_lethal_only.get() ? hEALTH < 93 : true) {
			if (dormant) {
				render::esp_small.string(box.x - 10, y, { 130, 130, 130, low_alpha }, std::to_string(hEALTH), render::ALIGN_RIGHT);
			}
			else {
				render::esp_small.string(box.x - 10, y, { 255, 255, 255, low_alpha }, std::to_string(hEALTH), render::ALIGN_RIGHT);
			}
		}
	}


	// draw flags.
	{
		std::vector< std::pair< std::string, Color > > flags;

		if (data->m_voice_cheat != CHEAT_UNKNOWN) {
			switch (data->m_voice_cheat) {
			default: // incase i forgot to add it here lol.
				flags.push_back({ XOR("UNDEFINED"), { 255, 255, 255, low_alpha } });
				break;
			case CHEAT_GOOFYHOOK_OLD:
				flags.push_back({ XOR("LOSER"), { 255, 255, 255, low_alpha } });
				break;
			case CHEAT_GOOFYHOOK:
				if (data->m_goofy_whitelist || g_menu.main.aimbot.enable_goofy_whitelist.get()) {
					flags.push_back({ XOR("GOOFY"), { 150, 200, 60, low_alpha } });
				}
				else {
					flags.push_back({ XOR("GOOFY"), { 255, 255, 255, low_alpha } });
				}
				
				break;
			}
		}

		auto items = enemy ? g_menu.main.players.flags_enemy.GetActiveIndices() : g_menu.main.players.flags_friendly.GetActiveIndices();

		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.
		for (auto it = items.begin(); it != items.end(); ++it) {

			// money.
			if (*it == 0)
				if (dormant)
					flags.push_back({ tfm::format(XOR("$%i"), player->m_iAccount()), { 130,130,130, low_alpha } });
				else
					flags.push_back({ tfm::format(XOR("$%i"), player->m_iAccount()), { 150, 200, 60, low_alpha } });

			// armor.
			if (*it == 1) {
				// helmet and kevlar.
				if (player->m_bHasHelmet() && player->m_ArmorValue() > 0)
					if (dormant)
						flags.push_back({ XOR("HK"), { 130,130,130, low_alpha } });
					else
						flags.push_back({ XOR("HK"), { 255, 255, 255, low_alpha } });
				// only helmet.
				else if (player->m_bHasHelmet())
					if (dormant)
						flags.push_back({ XOR("HK"), { 130,130,130, low_alpha } });
					else
						flags.push_back({ XOR("HK"), { 255, 255, 255, low_alpha } });

				// only kevlar.
				else if (player->m_ArmorValue() > 0)
					if (dormant)
						flags.push_back({ XOR("K"), { 130,130,130, low_alpha } });
					else
						flags.push_back({ XOR("K"), { 255, 255, 255, low_alpha } });
			}
			
			// ping.
			if (*it == 2 && g_cl.m_resource) {
				auto& ping_record = g_cl.m_ping_records[index - 1];
				float ping_delta = ping_record.current - ping_record.lowest;

				//g_cl.print(std::to_string(ping_record.lowest) + "\n");

				float ping_lerp = std::clamp(ping_delta, 0.f, 200.f) / 200.f;

				auto ping_color = dormant ? Color(130, 130, 130, low_alpha) : Color(
					255,
					math::Lerp(ping_lerp, 255.f, 0.f),
					math::Lerp(ping_lerp, 255.f, 0.f),
					low_alpha
				);

				int ping_str = round(ping_record.current);

				if (dormant)
					flags.push_back({ std::to_string(ping_str), ping_color});
				else
					flags.push_back({ std::to_string(ping_str), ping_color});
			}

			// scoped.
			if (*it == 3 && player->m_bIsScoped())
				if (dormant)
					flags.push_back({ XOR("SCOPED"), { 130,130,130, low_alpha } });
				else
					flags.push_back({ XOR("SCOPED"), { 60, 180, 225, low_alpha } });

			// flashed.
			if (*it == 4 && player->m_flFlashBangTime() > 0.f)
				if (dormant)
					flags.push_back({ XOR("FLASHED"), { 130,130,130, low_alpha } });
				else
					flags.push_back({ XOR("FLASHED"), { 60, 180, 225, low_alpha } });

			// reload.
			if (*it == 5) {
				// get ptr to layer 1.
				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

				// check if reload animation is going on.
				if (layer1->m_weight != 0.f && player->GetSequenceActivity(layer1->m_sequence) == 967 /* ACT_CSGO_RELOAD */)
					if (dormant)
						flags.push_back({ XOR("RELOAD"), { 130,130,130, low_alpha } });
					else
						flags.push_back({ XOR("RELOAD"), { 60, 180, 225, low_alpha } });
			}

			// bomb.
			if (*it == 6 && player->HasC4()) {
				if (dormant)
					flags.push_back({ XOR("C4"), { 130,130,130, low_alpha } });
				else
					flags.push_back({ XOR("C4"), { 255, 0, 0, low_alpha } });
			}

			if (*it == 7 && enemy) {		
				if (data->m_records.empty())
					continue;

				int res_mode = data->m_records[0].m_resolver_mode;
				if (res_mode == Resolver::Modes::RESOLVE_NONE || res_mode == Resolver::Modes::RESOLVE_WALK)
					continue;

				if (dormant) {
					switch (data->m_unlikely_resolve) {
					case 1:
						flags.push_back({ XOR("UNLIKELY"), { 255, 255, 0, low_alpha } });
						break;
					case 2:
						flags.push_back({ XOR("UNLIKELY"), { 255,0,0, low_alpha } });
						break;
					}

					flags.push_back({ XOR("FAKE"), { 130,130,130, low_alpha } });
				}
				else {
					switch (data->m_unlikely_resolve) {
					case 1:
						flags.push_back({ XOR("UNLIKELY"), { 255, 255, 0, low_alpha } });
						break;
					case 2:
						flags.push_back({ XOR("UNLIKELY"), { 255,0,0, low_alpha } });
						break;
					}

					float resolver_chance = g_resolver.GetDeltaChance(data, data->m_lby_delta);

					if (data->m_cant_hit_lby) {
						// red.
						flags.push_back({ XOR("FAKE"), { 255, 0, 0, low_alpha } });
					}
					else if (res_mode == Resolver::Modes::RESOLVE_BODY) {
						// green.
						flags.push_back({ XOR("FAKE"), { 150, 200, 60, low_alpha } });
					}
					/*else if (resolver_chance != -1.f) {
						// color based on actual hit chance.
						Color color = { math::Lerp(resolver_chance * 0.01f, 255, 150), math::Lerp(resolver_chance * 0.01f, 0, 200), math::Lerp(resolver_chance * 0.01f, 0, 60), low_alpha };
						flags.push_back({ XOR("FAKE"), color });
					}*/
					else if (res_mode == Resolver::Modes::RESOLVE_STAND_FAKE_STATIC) {
						// yellow.
						flags.push_back({ XOR("FAKE"), { 255, 255, 0, low_alpha } });
					}
					else if (res_mode == Resolver::Modes::RESOLVE_STAND_LBY_DELTA_FIRST_TICK || res_mode == Resolver::Modes::RESOLVE_STAND_FAKE_JITTER) {
						// orange
						flags.push_back({ XOR("FAKE"), { 243, 156, 18, low_alpha } });
					}
					else {
						flags.push_back({ XOR("FAKE"), { 255,255,255, low_alpha } });
						//flags.push_back({ XOR("r: %s"), m_record->m_resolver_mode, { 255,255,255, low_alpha } });
						
					}
				}
			}

			if (*it == 8 && g_cl.m_processing && !dormant && data->m_first_record_hitscan_preference == 1) {
				flags.push_back({ XOR("BAIM"), { 150, 200, 60, low_alpha } });
			}

			if (*it == 9 && g_cl.m_processing && !dormant && data->m_first_record_hitscan_preference == 2) {
				flags.push_back({ XOR("DAMAGE"), { 150, 200, 60, low_alpha } });
			}

			if (*it == 10 && g_cl.m_processing && !dormant) {
				if (data->m_peek_data.m_filtered) {
					if (data->m_peek_data.m_localpeek_ticks > data->m_peek_data.m_enemypeek_ticks) {
						flags.push_back({ XOR("HIT"), { 150, 200, 60, low_alpha } });
					}
					else {
						flags.push_back({ XOR("HIT"), { 255, 0, 0, low_alpha } });
					}
				}

				switch (data->m_lagfix_mode) {
				case LagCompensation::LAGCOMP_DONT_PREDICT:
					// green.
					flags.push_back({ XOR("LC"), { 150, 200, 60, low_alpha } });
					break;
				case LagCompensation::LAGCOMP_WAIT:
					// yellow.
					flags.push_back({ XOR("LC"), { 255, 255, 0, low_alpha } });
					break;
				case LagCompensation::LAGCOMP_PREDICT:
					// red.
					flags.push_back({ XOR("LC"), { 255, 0, 0, low_alpha } });
					break;
				}
			}
			if (*it == 11 && !dormant) {
				if (data->m_records.empty())
					continue;
				int res_mode = data->m_records[0].m_resolver_mode;
				flags.push_back({ tfm::format(XOR("R:%i"), res_mode), { 130,255,130, low_alpha } });
				}
		}

		// iterate flags.
		for (size_t i{ }; i < flags.size(); ++i) {
			// get flag job (pair).
			const auto& f = flags[i];

			int offset = i * render::esp_small.m_size.m_height;

			// draw flag.
			render::esp_small.string(box.x + box.w + 2, box.y + offset, f.second, f.first);
		}
	}

	// draw bottom bars.
	{
		float offset1{ 0 };
		float offset3{ 0 };
		float offset{ 0 };
		float distance1337{ 0 };

		// draw lby update bar.
		if (enemy) {
			AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

			// make sure everything is valid.
			if (data && data->m_records.size()) {
				// grab lag record.
				LagRecord* current = &data->m_records.front();

				if (current) {
					bool bruh = !data->m_cant_hit_lby 
						&& current->m_resolver_mode != Resolver::Modes::RESOLVE_NONE 
						&& current->m_resolver_mode != Resolver::Modes::RESOLVE_WALK
						&& (current->m_flags & FL_ONGROUND)
						&& g_menu.main.players.lby_update.get();

					if (g_menu.main.players.esp_extras.get(1)) {
						esp_bruh.lby_timer = (bruh ? std::ceil(math::Lerp(step * 4, esp_bruh.lby_timer * 100, bruh ? 100.f : 0.f))
							: std::floor(math::Lerp(step * 4, esp_bruh.lby_timer * 100, bruh ? 100.f : 0.f))) / 100.f;
					}
					else {
						esp_bruh.lby_timer = bruh ? 1.f : 0.f;
					}

					// calculate box width
					float cycle = 1.f - std::clamp((data->m_lby_timer - current->m_anim_time) * 0.9091f, 0.f, 1.f);
					float width = (box.w * cycle);

					if (width >= 0.f) {
						Color clr = g_menu.main.players.lby_update_color.get();
						if (dormant) {
							clr.r() = 130;
							clr.g() = 130;
							clr.b() = 130;// 180, 60, 120
						}
						clr.a() = alpha * esp_bruh.lby_timer;

						if (g_menu.main.players.esp_extras.get(0)) {
							Color clr2 = clr;
							Color secondcolor = clr;
							clr2.a() = (95 * (alpha / 255.f)) * esp_bruh.lby_timer;
							secondcolor.a() = 0;

							render::gradient(box.x - 1, box.y + box.h + 6 + offset, box.w + 2, 2, clr2, secondcolor, false); // bottom
							render::gradient(box.x - 1, box.y + box.h + offset, box.w + 2, 2, secondcolor, clr2, false); // top

							render::gradient(box.x + box.w + 1, box.y + box.h + 2 + offset, 2, 4, clr2, secondcolor, true); // right.
							render::gradient(box.x - 3, box.y + box.h + 2 + offset, 2, 4, secondcolor, clr2, true); // left.
						}

						// draw.
						render::rect_filled(box.x - 1, box.y + box.h + 2, box.w + 2, 4, { 10, 10, 10, int(alpha * esp_bruh.lby_timer) });

						render::rect(box.x, box.y + box.h + 3, width, 2, clr);

						// move down the offset to make room for the next bar.
						offset += 5.f * esp_bruh.lby_timer;
						offset3 += 1.f * esp_bruh.lby_timer;
					}
				}
			}
		}

		// draw weapon.
		Weapon* weapon = player->GetActiveWeapon();
		if (weapon) {
			WeaponInfo* data = weapon->GetWpnData();
			if (data) {
				int bar;
				float scale;

				// the maxclip1 in the weaponinfo
				int max = data->m_max_clip1;
				int current = weapon->m_iClip1();

				C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

				// set reload state.
				bool reload = (layer1->m_weight != 0.f) && (player->GetSequenceActivity(layer1->m_sequence) == 967);

				bool correct_team = ((enemy && g_menu.main.players.weapon.get(0)) || (!enemy && g_menu.main.players.weapon.get(1)));
				bool bruh = correct_team && max != -1 && g_menu.main.players.ammo.get();

				if (g_menu.main.players.esp_extras.get(1)) {
					esp_bruh.ammo_bar_box = (bruh ? std::ceil(math::Lerp(step * 4, esp_bruh.ammo_bar_box * 100, bruh ? 100.f : 0.f)) 
						: std::floor(math::Lerp(step * 4, esp_bruh.ammo_bar_box * 100, bruh ? 100.f : 0.f))) / 100.f;

					esp_bruh.ammo_bar_text = (correct_team ? std::ceil(math::Lerp(step * 4, esp_bruh.ammo_bar_text * 100, correct_team ? 100.f : 0.f))
						: std::floor(math::Lerp(step * 4, esp_bruh.ammo_bar_text * 100, correct_team ? 100.f : 0.f))) / 100.f;
				}
				else {
					esp_bruh.ammo_bar_box = bruh ? 1.f : 0.f;
					esp_bruh.ammo_bar_text = correct_team ? 1.f : 0.f;
				}

				// check for reload.
				if (reload)
					scale = layer1->m_cycle;

				// not reloading.
				// make the division of 2 ints produce a float instead of another int.
				else
					scale = (float)current / max;

				// relative to bar.
				bar = (int)std::round((box.w - 2) * scale);

				Color clr = g_menu.main.players.ammo_color.get();
				if (dormant) {
					clr.r() = 130;
					clr.g() = 130;
					clr.b() = 130;//95, 174, 227,
				}
				clr.a() = alpha * esp_bruh.ammo_bar_box;

				if (g_menu.main.players.esp_extras.get(0)) {
					Color clr2 = clr;
					Color secondcolor = clr;
					clr2.a() = (95 * (alpha / 255.f)) * esp_bruh.ammo_bar_box;
					secondcolor.a() = 0;

					render::gradient(box.x - 1, box.y + box.h + 6 + offset, box.w + 2, 2, clr2, secondcolor, false); // bottom
					render::gradient(box.x - 1, box.y + box.h + offset, box.w + 2, 2, secondcolor, clr2, false); // top

					render::gradient(box.x + box.w + 1, box.y + box.h + 2 + offset, 2, 4, clr2, secondcolor, true); // right.
					render::gradient(box.x - 3, box.y + box.h + 2 + offset, 2, 4, secondcolor, clr2, true); // left.
				}

				// background.
				render::rect_filled(box.x - 1, box.y + box.h + 2 + offset, box.w + 2, 4, { 10, 10, 10, int(alpha * esp_bruh.ammo_bar_box) });

				// actual bar.
				render::rect(box.x, box.y + box.h + 3 + offset, bar + 2, 2, clr);

				// less then a 5th of the bullets left.
				if (current <= (int)std::round(max / 5) && !reload)
					if (dormant)
						render::esp_small.string(box.x + bar, box.y + box.h + offset, { 130, 130, 130, int(low_alpha * esp_bruh.ammo_bar_box) }, std::to_string(current), render::ALIGN_CENTER);
					else
						render::esp_small.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, int(low_alpha * esp_bruh.ammo_bar_box) }, std::to_string(current), render::ALIGN_CENTER);

				offset += 6.f * esp_bruh.ammo_bar_box;

				// text.
				if (g_menu.main.players.weapon_mode.get(0) && correct_team) {
					offset1 -= 9 * esp_bruh.ammo_bar_text;
					// construct std::string instance of localized weapon name.
					std::string name{ weapon->GetLocalizedName() };

					// small
					//s needs upper case.
					std::transform(name.begin(), name.end(), name.begin(), ::toupper);


					if (dormant)
						render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + distance1337, { 130,130,130, int(low_alpha * esp_bruh.ammo_bar_text) }, name, render::ALIGN_CENTER);
					else
						render::esp_small.string(box.x + box.w / 2, box.y + box.h + offset + distance1337, { 255, 255, 255, int(low_alpha * esp_bruh.ammo_bar_text) }, name, render::ALIGN_CENTER);

				}

				// icons.
				if (g_menu.main.players.weapon_mode.get(1) && correct_team) {
					offset -= 5 * esp_bruh.ammo_bar_text;
					// icons are super fat..
					// move them back up.

					std::string icon = tfm::format(XOR("%c"), m_weapon_icons[weapon->m_iItemDefinitionIndex()]);
					if (dormant)
						render::cs.string(box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 2, { 130,130,130,  int(low_alpha * esp_bruh.ammo_bar_text) }, icon, render::ALIGN_CENTER);
					else
						render::cs.string(box.x + box.w / 2, box.y + box.h + offset - offset1 + distance1337 + 2, { 255, 255, 255,  int(low_alpha * esp_bruh.ammo_bar_text) }, icon, render::ALIGN_CENTER);
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4() {
	bool        mode_2d, mode_3d, is_visible;
	float       explode_time_diff, dist, range_damage;
	vec3_t      dst, to_target;
	int         final_damage;
	std::string time_str, damage_str;
	Color       damage_color;
	vec2_t      screen_pos;

	static auto scale_damage = [](float damage, int armor_value) {
		float new_damage, armor;

		if (armor_value > 0) {
			new_damage = damage * 0.5f;
			armor = (damage - new_damage) * 0.5f;

			if (armor > (float)armor_value) {
				armor = (float)armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max(0, (int)std::floor(damage));
	};

	// store menu vars for later.
	mode_2d = g_menu.main.visuals.planted_c4.get(0);
	mode_3d = g_menu.main.visuals.planted_c4.get(1);
	if (!mode_2d && !mode_3d)
		return;

	// bomb not currently active, do nothing.
	if (!m_c4_planted)
		return;

	// calculate bomb damage.
	// references:
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//     https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//     https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter();
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length();

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp((dist * dist) / ((m_planted_c4_radius_scaled * -2.f) * m_planted_c4_radius_scaled));

		// now finally, scale the damage based on our armor (if we have any).
		final_damage = scale_damage(range_damage, g_cl.m_local->m_ArmorValue());
	}

	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	time_str = tfm::format(XOR("%.2f"), explode_time_diff);
	damage_str = tfm::format(XOR("%i"), final_damage);

	// get damage color.
	damage_color = (final_damage < g_cl.m_local->m_iHealth()) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen(m_planted_c4_explosion_origin, screen_pos);

	std::string bomb = m_last_bombsite.c_str();

	// 'on screen (2D)'.
	if (mode_2d) {
		std::string timer1337 = tfm::format(XOR("%s - %.1fs"), bomb.substr(0, 1), explode_time_diff);
		std::string damage1337 = tfm::format(XOR("%i"), final_damage);

		Color colortimer = { 135, 172, 10, 255 };
		if (explode_time_diff < 10) colortimer = { 200, 200, 110, 255 };
		if (explode_time_diff < 5) colortimer = { 192, 32, 17, 255 };

		if (m_c4_planted && !bombexploded && !bombedefused) {
			if (explode_time_diff > 0.f) {
				render::indicator.string(6, 11, { 0,0, 0, 125 }, timer1337);
				render::indicator.string(5, 10, colortimer, timer1337);
			}
			//Render.StringCustom(5, 0, 0, getSite(c4) + timer + "s", colortimer, 
			//);
			if (g_cl.m_local->m_iHealth() <= final_damage) {
				render::indicator.string(6, 31, { 0,0, 0, 125 }, tfm::format(XOR("FATAL")));
				render::indicator.string(5, 30, { 192, 32, 17, 255 }, tfm::format(XOR("FATAL")));
			}
			else if (final_damage > 1) {
				render::indicator.string(5, 31, { 0,0, 0, 125 }, tfm::format(XOR("- %iHP"), damage1337));
				render::indicator.string(6, 30, { 255, 255, 152, 255 }, tfm::format(XOR("- %iHP"), damage1337));
			}
		}
	}

	// 'on bomb (3D)'.
	if (mode_3d && is_visible) {
		if (explode_time_diff > 0.f)
			render::esp.string(screen_pos.x, screen_pos.y, colors::white, time_str, render::ALIGN_CENTER);

		// only render damage string if we're alive.
		if (g_cl.m_local->alive())
			render::esp.string(screen_pos.x, (int)screen_pos.y + render::esp.m_size.m_height, damage_color, damage_str, render::ALIGN_CENTER);
	}
}

void Visuals::DrawAutopeek() {
	// because of interpolation, if we let go of the key, the circle will still render following the player until the circle fully fades out.
	static vec3_t render_pos;

	if (!g_cl.m_processing)
		return;

	if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
		m_esp_interpolation_individual.m_autopeek = std::ceilf(math::Lerp(VIS_INTERP, m_esp_interpolation_individual.m_autopeek, 255.f));

		if ((render_pos - g_cl.m_pre_autopeek_pos).length_2d() > 10.f) {
			render_pos = g_cl.m_pre_autopeek_pos;
		}
	}
	else {
		m_esp_interpolation_individual.m_autopeek = std::floorf(math::Lerp(VIS_INTERP, m_esp_interpolation_individual.m_autopeek, 0.f));

		if (m_esp_interpolation_individual.m_autopeek < 1.f) {
			render_pos = g_cl.m_pre_autopeek_pos;
		}
	}

	if (!&render_pos || !g_menu.main.movement.autopeek_circle.get())
		return;

	// causes weird screen glitching shit, should fix it...
	if (m_esp_interpolation_individual.m_autopeek < 1.f)
		return;

	Color col = g_menu.main.movement.autopeek_circle_color.get();
	col.a() = 80.f;
	render::world_circle(render_pos, m_esp_interpolation_individual.m_autopeek * 0.1f, 0.f, col);
	col.a() = 180.f;
	render::WorldCircleOutline(render_pos, m_esp_interpolation_individual.m_autopeek * 0.1f, 0.f, col);
}

bool Visuals::GetPlayerBoxRect(Player* player, Rect& box) {
	vec3_t min, max, out_vec;
	float left, bottom, right, top;
	matrix3x4_t& tran_frame = player->m_pCoordFrame();

	// get hitbox bounds.
	min = player->m_vecMins();
	max = player->m_vecMaxs();

	vec2_t screen_boxes[8];

	// transform mins and maxes to points. 
	vec3_t points[] =
	{
		{ min.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z }
	};

	// transform points to 3-dimensional space.
	for (int i = 0; i <= 7; i++) {
		math::VectorTransform(points[i], tran_frame, out_vec);
		if (!render::WorldToScreen(out_vec, screen_boxes[i]))
			return false;
	}

	// generate an array to clamp later.
	vec2_t box_array[] = {
		screen_boxes[3],
		screen_boxes[5],
		screen_boxes[0],
		screen_boxes[4],
		screen_boxes[2],
		screen_boxes[1],
		screen_boxes[6],
		screen_boxes[7]
	};

	// state the position and size of the box.
	left = screen_boxes[3].x,
		bottom = screen_boxes[3].y,
		right = screen_boxes[3].x,
		top = screen_boxes[3].y;

	// clamp the box sizes.
	for (int i = 0; i <= 7; i++) {
		if (left > box_array[i].x)
			left = box_array[i].x;

		if (bottom < box_array[i].y)
			bottom = box_array[i].y;

		if (right < box_array[i].x)
			right = box_array[i].x;

		if (top > box_array[i].y)
			top = box_array[i].y;
	}

	// state the box bounds.
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = (bottom - top);

	return true;
}

bool get_bone(matrix3x4_t* mx, vec3_t& out, int bone = 0) {
	if (bone < 0 || bone >= 128)
		return false;

	matrix3x4_t* bone_matrix = &mx[bone];

	if (!bone_matrix)
		return false;

	out = { bone_matrix->m_flMatVal[0][3], bone_matrix->m_flMatVal[1][3], bone_matrix->m_flMatVal[2][3] };

	return true;
}

void Visuals::DrawHistorySkeleton(Player* player, int opacity) {
	return;
}

void Visuals::DrawSkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	//BoneArray     matrix[ 128 ];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	// get player's model.
	model = player->GetModel();
	if (!model || player->m_BoneCache().m_CachedBoneCount <= 0)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	for (int i{}; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		get_bone(player->m_BoneCache().m_pCachedBones, bone_pos, i);
		get_bone(player->m_BoneCache().m_pCachedBones, parent_pos, parent);

		Color clr = player->enemy(g_cl.m_local) ? g_menu.main.players.skeleton_enemy.get() : g_menu.main.players.skeleton_friendly.get();
		if (player->dormant()) 
			clr = { 112, 112, 112 };

		clr.a() = opacity;

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
	}
}

void Visuals::RenderGlow() {
	Color   color;
	Player* player;

	if (!g_cl.m_local)
		return;

	if (!g_csgo.m_glow->m_object_definitions.Count())
		return;

	float blend = g_menu.main.players.glow_blend.get() / 100.f;

	for (int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];

		// skip non-players.
		if (!obj->m_entity || !obj->m_entity->IsPlayer())
			continue;

		// get player ptr.
		player = obj->m_entity->as< Player* >();

		if (player->m_bIsLocalPlayer())
			continue;

		// get reference to array variable.
		float& opacity = m_opacities[player->index() - 1];

		bool enemy = player->enemy(g_cl.m_local);
		bool full_bloom = false;
		if (enemy && g_menu.main.players.glow_visualize_aimbot.get() && g_cl.m_processing) {
			auto data = &g_aimbot.m_players[player->index() - 1];
			if (!data)
				return;

			full_bloom = data->m_peek_data.m_filtered;
		}

		if (!full_bloom && (enemy && !g_menu.main.players.glow.get(0)))
			continue;

		if (!enemy && !g_menu.main.players.glow.get(1))
			continue;

		// enemy color
		if (enemy) {
			color = g_menu.main.players.glow_enemy.get();
		}
		// friendly color
		else {
			color = g_menu.main.players.glow_friendly.get();
		}

		if (full_bloom) {
			blend = g_menu.main.players.glow_visualize_aimbot_blend.get() / 100.f;
			color = g_menu.main.players.glow_visualize_aimbot_color.get();
		}

		obj->m_render_occluded = true;
		obj->m_render_unoccluded = false;
		obj->m_render_full_bloom = full_bloom;
		obj->m_color = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
		obj->m_alpha = opacity * blend;
	}
}

void Visuals::DrawHitboxMatrix(LagRecord* record, matrix3x4_t* matrix2, int hitbox, Color color) {
	if (!g_menu.main.aimbot.aimbot_debug.get(1))
		return;
	
	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox, *targetbbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel();
	if (!model)
		return;

	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return;

	targetbbox = set->GetHitbox(hitbox);
	if (!targetbbox)
		return;

	for (int i{ }; i < set->m_hitboxes; ++i) {
		bbox = set->GetHitbox(i);
		if (!bbox)
			continue;

		if (bbox->m_radius <= 0.f)
			continue;

		// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

        // create a rotation matrix.
		matrix3x4_t matrix;
		g_csgo.AngleMatrix(bbox->m_angle, matrix);

		// apply the rotation matrix to the entity output space (world).
		math::ConcatTransforms(matrix2[bbox->m_bone], matrix, matrix);

		// get world positions from new matrix.
		math::VectorTransform(bbox->m_mins, matrix, mins);
		math::VectorTransform(bbox->m_maxs, matrix, maxs);

		g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, color.r(), color.g(), color.b(), color.a(), g_csgo.m_cvar->FindVar(HASH("sv_showlagcompensation_duration"))->GetFloat(), 0, 1);
	}
}
//g_visuals.DrawHitboxMatrix(shot->m_record, g_aimbot.m_backup[shot->m_target->index() - 1].m_matrix, shot->m_hitbox, Color(0, 255, 0));
	//}

void Visuals::DrawSkeletonOnHit(LagRecord* record, matrix3x4_t* matrix, int hitbox, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int parent;
	vec3_t bone_pos, parent_pos;
	vec2_t bone_pos_screen, parent_pos_screen;

	// Check if record and matrix are valid
	if (!record) {
	//	std::cout << "Record is null.\n";
		return;
	}

	if (!matrix) {
		//std::cout << "Matrix is null.\n";
		return;
	}

	// Get player's model.
	model = record->m_player->GetModel();
	if (!model || record->m_player->m_BoneCache().m_CachedBoneCount <= 0) {
	//	std::cout << "Model or BoneCache is invalid.\n";
		return;
	}

	// Get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr) {
		//std::cout << "Studio model is null.\n";
		return;
	}

	for (int i = 0; i < hdr->m_num_bones; ++i) {
		// Get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// Get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// Resolve main bone and parent bone positions.
		math::VectorTransform(vec3_t(0, 0, 0), matrix[i], bone_pos); // Get bone position.
		math::VectorTransform(vec3_t(0, 0, 0), matrix[parent], parent_pos); // Get parent position.

		Color clr = record->m_player->enemy(g_cl.m_local) ? g_menu.main.players.skeleton_enemy.get() : g_menu.main.players.skeleton_friendly.get();
		if (record->m_player->dormant())
			clr = { 112, 112, 112 };

		clr.a() = opacity;

		// World to screen both the bone and parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen)) {
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
		}
	//	else {
		//	std::cout << "Failed to convert bone or parent position to screen coordinates for bone index: " << i << ".\n";
	//	}
	}
}

void Visuals::DrawBeams() {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	auto vis_impacts = &g_shots.m_vis_impacts;

	impact_count = vis_impacts->size();
	if (!impact_count)
		return;

	// crash.
	if (!g_cl.m_local)
		return;

	curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	for (size_t i{ impact_count }; i-- > 0; ) {
		auto impact = &vis_impacts->operator[ ](i);
		if (!impact)
			continue;

		// impact is too old, erase it.
		if (curtime - game::TICKS_TO_TIME(impact->m_tickbase) > g_menu.main.visuals.impact_beams_time.get()) {
			vis_impacts->erase(vis_impacts->begin() + i);
			continue;
		}

		// is this the final impact?
		// last impact in the vector, it's the final impact.
		if (i == (impact_count - 1))
			is_final_impact = true;
		// the current impact's tickbase is different than the next, it's the final impact.
		else if ((i + 1) < impact_count && impact->m_tickbase != vis_impacts->operator[ ](i + 1).m_tickbase)
			is_final_impact = true;
		else
			is_final_impact = false;

		// is this the final impact?
		// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

		if (is_final_impact) {
			// calculate start and end position for beam.
			start = impact->m_shoot_pos;

			dir = (impact->m_impact_pos - start).normalized();
			dist = (impact->m_impact_pos - start).length();

			end = start + (dir * dist);

			// setup beam info.
			// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
			beam_info.m_vecStart = start;
			beam_info.m_vecEnd = end;
			beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/purplelaser1.vmt"));
			beam_info.m_pszModelName = XOR("sprites/purplelaser1.vmt");
			beam_info.m_flHaloScale = 0.f;
			beam_info.m_flLife = g_csgo.m_globals->m_frametime + 0.1f;//g_menu.main.visuals.impact_beams_time.get();
			beam_info.m_flWidth = 3.f;
			beam_info.m_flEndWidth = 3.f;
			beam_info.m_flFadeLength = 1.f;
			beam_info.m_flAmplitude = 0.0f;   // beam 'jitter'.

			beam_info.m_flBrightness = 255.f * std::min(1.f, g_menu.main.visuals.impact_beams_time.get() - (curtime - game::TICKS_TO_TIME(impact->m_tickbase)));;

			beam_info.m_flSpeed = 0.2f;  // seems to control how fast the 'scrolling' of beam is... once fully spawned.
			beam_info.m_nStartFrame = 0;
			beam_info.m_flFrameRate = 0.f;
			beam_info.m_nSegments = 2;     // controls how much of the beam is 'split up', usually makes m_flAmplitude and m_flSpeed much more noticeable.
			beam_info.m_bRenderable = true;  // must be true or you won't see the beam.
			beam_info.m_nFlags = 0;

			beam_info.m_flRed = g_menu.main.visuals.impact_beams_color.get().r();
			beam_info.m_flGreen = g_menu.main.visuals.impact_beams_color.get().g();
			beam_info.m_flBlue = g_menu.main.visuals.impact_beams_color.get().b();

			// attempt to render the beam.
			beam = game::CreateGenericBeam(beam_info);
			if (beam) {
				g_csgo.m_beams->DrawBeam(beam);

				// we only want to render a beam for this impact once.
				//impact->m_ignore = true;
			}
		}
	}
}

void Visuals::DebugAimbotPoints(Player* player) {
	if (!g_menu.main.aimbot.aimbot_debug.get(1))
		return;

	std::vector< AimPoint_t > p2{ };
	AimPlayer* data = &g_aimbot.m_players.at(player->index() - 1);
	if (!data || data->m_records.empty())
		return;

	LagRecord* front = &data->m_records.front();
	if (!front || !front->m_matrix)
		return;

	data->SetupHitboxPoints(front, front->m_matrix, g_aimbot.GetMinDamage(player->m_iHealth()), false, p2);

	if (p2.empty())
		return;

	for (auto& p : p2) {
		vec2_t screen;
		if (!p.m_rotated)
			continue;

		if (render::WorldToScreen(p.m_pos, screen)) {
			render::rect_filled(screen.x, screen.y, 3, 3, {255, 255, 255});
		}
	}
}

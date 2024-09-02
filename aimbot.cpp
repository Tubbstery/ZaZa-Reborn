#include "includes.h"

Aimbot g_aimbot{ };;

void AimPlayer::OnRoundStart(Player* player) {
	m_player = player;

	m_records.clear();

	m_cant_hit_lby = true;

	m_voice_cheat = CHEAT_UNKNOWN;
	m_goofy_whitelist = false;

	m_delay_shot_ticks = 0;
	m_peek_data.m_localpeek_ticks = 0;
	m_peek_data.m_enemypeek_ticks = 0;

	g_visuals.m_opacities[player->index() - 1] = 0.f;

	g_aimbot.m_stop = g_aimbot.m_stop_early = false;

	// IMPORTANT: DO NOT CLEAR LAST HIT SHIT.
}

void Aimbot::init() {
	// clear old targets.
	m_targets.clear();
	if (!g_menu.main.aimbot.aimbot_optimizations.get(0)) {
		m_scanned_targets.clear();
	}

	m_target = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_damage = 0.f;
	m_record = nullptr;
	m_stop = false;

	//m_resolver_override_index = -1;
	//m_resolver_override_side = 0;

	m_debug_render.clear();

	UpdateShootpos(0.f);
}

void Aimbot::UpdateShootpos(float pitch) {
	if (!g_cl.m_processing)
		return;

	matrix3x4_t matrix[128];
	float poses[24];
	C_AnimationLayer layers[13];
	const CCSGOPlayerAnimState state = *g_cl.m_local->m_PlayerAnimState();

	std::memcpy(matrix, g_cl.m_local->m_BoneCache().m_pCachedBones, g_cl.m_local->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));
	g_cl.m_local->GetPoseParameters(poses);
	g_cl.m_local->GetAnimLayers(layers);

	g_cl.m_local->m_flPoseParameter()[12] = (pitch + 90.f) / 180.f;
	g_cl.m_local->SetupBones(nullptr, 128, BONE_USED_BY_ANYTHING, g_cl.m_local->m_flSimulationTime());
	g_cl.m_shoot_pos = g_cl.m_local->GetShootPosition();

	*g_cl.m_local->m_PlayerAnimState() = state;
	std::memcpy(g_cl.m_local->m_BoneCache().m_pCachedBones, matrix, g_cl.m_local->m_BoneCache().m_CachedBoneCount * sizeof(matrix3x4_t));
	g_cl.m_local->SetPoseParameters(poses);
	g_cl.m_local->SetAnimLayers(layers);
}

bool Aimbot::AllowHitbox(Player* player, int hitbox, float damage) {
	// can we hit for min damage.
	if (!g_cl.m_weapon_info || penetration::ScaleDamage(player, g_cl.m_weapon_info->m_damage, g_cl.m_weapon_info->m_armor_ratio, HitboxToHitgroup(hitbox)) < damage) {
		return false;
	}

	// are we force baiming.
	if (g_input.GetKeyState(g_menu.main.aimbot.baim_key.get()) || g_cl.m_weapon_id == ZEUS) {
		if (!(hitbox == HITBOX_BODY || hitbox == HITBOX_THORAX || hitbox == HITBOX_CHEST || hitbox == HITBOX_UPPER_CHEST))
			return false;
	}

	// is hitbox selected.
	switch (hitbox) {
	case HITBOX_HEAD:
	case HITBOX_NECK:
		return g_menu.main.aimbot.hitbox.get(0);
		break;
	case HITBOX_PELVIS:
	case HITBOX_BODY:
		return g_menu.main.aimbot.hitbox.get(2);
		break;
	case HITBOX_THORAX:
	case HITBOX_CHEST:
	case HITBOX_UPPER_CHEST:
		return g_menu.main.aimbot.hitbox.get(1);
		break;
	case HITBOX_R_THIGH:
	case HITBOX_L_THIGH:
	case HITBOX_R_CALF:
	case HITBOX_L_CALF:
		return g_menu.main.aimbot.hitbox.get(4);
		break;
	case HITBOX_R_FOOT:
	case HITBOX_L_FOOT:
		return g_menu.main.aimbot.hitbox.get(5);
		break;
	case HITBOX_R_HAND:
	case HITBOX_L_HAND:
	case HITBOX_R_UPPER_ARM:
	case HITBOX_R_FOREARM:
	case HITBOX_L_UPPER_ARM:
	case HITBOX_L_FOREARM:
		return g_menu.main.aimbot.hitbox.get(3);
		break;
	default:
		break;
	}

	return false;
}

float Aimbot::GetMinDamage(float hp) {
	float dmg;

	if (g_cl.m_weapon_id == ZEUS) {
		dmg = hp;
	}
	else {
		bool use_override = g_menu.main.aimbot.override_damage_mode.get() ? g_input.GetKeyState(g_menu.main.aimbot.override_damage.get()) : m_damage_override_toggle;

		if (use_override) {
			dmg = g_menu.main.aimbot.override_damage_amount.get();
		}
		else {
			dmg = g_menu.main.aimbot.minimum_damage.get();
		}

		// scale to player hp.
		dmg *= hp / 100.f;

		if (g_menu.main.aimbot.mimimum_damage_half_pistol.get() && g_cl.m_weapon_type == WEAPONTYPE_PISTOL) {
			dmg *= 0.5f;
		}
	}

	// make sure min damage is at least 1.
	return std::max(dmg, 1.f);
}

// most ghetto extrapolation ever but its chill.
void Aimbot::GetPeekData(AimPlayer* data, LagRecord* record) {
	if (!data || !record || !data->m_player)
		return;

	static const auto pred_ticks = 16;
	static const auto add_ticks = pred_ticks * 2;

	const auto shoot_pos_backup = g_cl.m_shoot_pos;
	const int tick_count = g_csgo.m_globals->m_tick_count;

	g_cl.m_shoot_pos.z += 8; 
	g_cl.m_shoot_pos += g_cl.m_local->m_vecVelocity() * (g_csgo.m_globals->m_interval * pred_ticks);

	data->m_peek_data.m_peek_damage_body = 0.f;

	record->cache();

	if (data->m_peek_data.m_localpeek_ticks <= tick_count) {
		for (int i = 0; i < 2; i++) {
			auto hitbox = i == 0 ? HITBOX_BODY : HITBOX_HEAD;
			if (!AllowHitbox(data->m_player, hitbox, 1.f))
				continue;

			const model_t* model = data->m_player->GetModel();
			if (!model)
				continue;

			studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
			if (!hdr)
				continue;

			mstudiohitboxset_t* set = hdr->GetHitboxSet(data->m_player->m_nHitboxSet());
			if (!set)
				continue;

			mstudiobbox_t* bbox = set->GetHitbox(hitbox);
			if (!bbox)
				continue;

			vec3_t center = vec3_t{ bbox->m_maxs.x, bbox->m_maxs.y - bbox->m_radius, bbox->m_maxs.z };
			math::VectorTransform(center, record->m_matrix[bbox->m_bone], center);

			penetration::PenetrationInput_t in;
			in.m_damage = 1.f;
			in.m_can_pen = true;
			in.m_target = data->m_player;
			in.m_from = g_cl.m_local;
			in.m_pos = center;
			penetration::PenetrationOutput_t out;

			if (g_menu.main.aimbot.aimbot_debug.get(1)) {
				g_csgo.m_debug_overlay->AddLineOverlay(g_cl.m_shoot_pos, center, 255, 0, 0, false, 0.1f);
			}

			if (penetration::run(&in, &out)) {
				data->m_peek_data.m_localpeek_ticks = tick_count + add_ticks;
				if (out.m_hitgroup == HITGROUP_STOMACH) {
					data->m_peek_data.m_peek_damage_body = out.m_damage;
				}
				break;
			}
		}
	}

	g_cl.m_shoot_pos = shoot_pos_backup;

	if (data->m_peek_data.m_enemypeek_ticks <= tick_count) {
		vec3_t enemy_shoot_pos = data->m_player->GetShootPosition();
		enemy_shoot_pos += record->m_velocity * (g_csgo.m_globals->m_interval * pred_ticks);

		for (int i = 0; i < 2; i++) {
			auto hitbox = i == 0 ? HITBOX_BODY : HITBOX_HEAD;
			if (!AllowHitbox(g_cl.m_local, hitbox, 1.f))
				continue;

			const model_t* model = g_cl.m_local->GetModel();
			if (!model)
				continue;

			studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
			if (!hdr)
				continue;

			mstudiohitboxset_t* set = hdr->GetHitboxSet(g_cl.m_local->m_nHitboxSet());
			if (!set)
				continue;

			mstudiobbox_t* bbox = set->GetHitbox(hitbox);
			if (!bbox)
				continue;

			vec3_t center = vec3_t{ bbox->m_maxs.x, bbox->m_maxs.y - bbox->m_radius, bbox->m_maxs.z };
			math::VectorTransform(center, g_cl.m_local->m_BoneCache().m_pCachedBones[bbox->m_bone], center);

			penetration::PenetrationInput_t in;
			in.m_damage = 1.f;
			in.m_can_pen = true;
			in.m_target = g_cl.m_local;
			in.m_from = data->m_player;
			in.m_pos = center;
			penetration::PenetrationOutput_t out;

			if (g_menu.main.aimbot.aimbot_debug.get(1)) {
				g_csgo.m_debug_overlay->AddLineOverlay(enemy_shoot_pos, center, 255, 0, 0, false, 0.1f);
			}

			if (penetration::run(&in, &out)) {
				data->m_peek_data.m_enemypeek_ticks = tick_count + add_ticks;
				break;
			}
		}
	}

	data->m_peek_data.m_filtered = data->m_peek_data.m_enemypeek_ticks > tick_count || data->m_peek_data.m_localpeek_ticks > tick_count;

	if (data->m_peek_data.m_filtered) {
		m_debug_render.push_back(tfm::format(XOR("aimbot -> gpd : %s %s"), data->m_peek_data.m_localpeek_ticks - tick_count, data->m_peek_data.m_enemypeek_ticks - tick_count));
	}
}

bool Aimbot::ShouldDelayShot(AimPlayer* data, LagRecord* record) {
	if (!data)
		return false;

	// we will never recieve an lby update.
	if (!(record->m_flags & FL_ONGROUND) || data->m_cant_hit_lby) {
		return false;
	}

	const auto tick_count = g_csgo.m_globals->m_tick_count;

	// no longer set in da ragebot :D
	if (data->m_delay_shot_ticks > tick_count) {
		m_debug_render.push_back(tfm::format(XOR("aimbot -> sds(r) : %s"), data->m_delay_shot_ticks - tick_count));
		return true;
	}

	const int ticks_until_lby_update = game::TIME_TO_TICKS(data->m_lby_timer - record->m_anim_time);

	// unresolved and incoming lby update.
	if (record->m_resolver_mode <= 4 && ticks_until_lby_update < g_cl.m_max_lag - g_cl.m_lag && g_menu.main.aimbot.delay_shot_modes.get(1)) {
		data->m_delay_shot_ticks = tick_count + ticks_until_lby_update;
		return true;
	}

	return false;
}

void Aimbot::think() {
	// do all startup routines.
	init();

	// sanity.
	if (!g_cl.m_weapon)
		return;

	// no grenades or bomb.
	if ((!g_cl.m_weapon_fire && (g_cl.m_weapon_id == SSG08 || g_cl.m_weapon_id == AWP))
		|| g_cl.m_weapon->m_iClip1() < 1
		|| g_cl.m_weapon_type == WEAPONTYPE_GRENADE
		|| g_cl.m_weapon_type == WEAPONTYPE_C4) {
		return;
	}

	// we have no aimbot enabled.
	if (!g_menu.main.aimbot.enable.get())
		return;

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.
	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	// one tick before being able to shoot.
	if (revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query) {
		*g_cl.m_packet = false;
		return;
	}

	// we can run the main aimbot.
	find();
}

void Aimbot::GatherTargets(int& scans_this_tick) {
	float resolver_override_best_dist = -1.f;

	// gather targets.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		if (scans_this_tick >= 3 && g_menu.main.aimbot.aimbot_optimizations.get(0))
			break;

		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!IsValidTarget(player))
			continue;

		AimPlayer* data = &m_players[i - 1];
		if (!data)
			continue;

		// no point adding them if we would have nothing to aim at.
		if (data->m_records.empty())
			continue;

		if (!g_input.GetKeyState(g_menu.main.aimbot.override_resolver.get())) {
			// aimbot gets run before we modify our viewangles.
			float override_dist = math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - math::CalcAngle(g_cl.m_local->m_vecOrigin(), player->m_vecOrigin()).y);
			if (abs(override_dist) < resolver_override_best_dist || resolver_override_best_dist == -1.f) {
				resolver_override_best_dist = abs(override_dist);
				m_resolver_override_index = i;
				m_resolver_override_side = override_dist > 0 ? 1 : -1;
			}
		}

		// limit targets per tick.
		bool skip_target = false;
		if (g_menu.main.aimbot.aimbot_optimizations.get(0)) {
			for (const int t : m_scanned_targets) {
				if (i - 1 == t) {
					skip_target = true;
					break;
				}
			}
		}

		// limit da target.
		if (skip_target) {
			m_debug_render.push_back(tfm::format(XOR("aimbot -> ltpt : %s"), i));
			continue;
		}

		// more efficient scanning.
		GetPeekData(data, &data->m_records[0]);

		//nigger
		if (ShouldDelayShot(data, &data->m_records[0]))
			continue;

		// increment scanned targets counter.
		++scans_this_tick;

		// store player as potential target this tick.
		m_targets.emplace_back(data);
		m_scanned_targets.emplace_back(i - 1);
	}
}

void Aimbot::find() {
	int targets_scanned_this_tick = 0;

	// gather aimbot targets.
	GatherTargets(targets_scanned_this_tick);

	// we had no1 to scanski.
	if (g_menu.main.aimbot.aimbot_optimizations.get(0) && targets_scanned_this_tick == 0) {
		// clear previously scanned targets.
		m_scanned_targets.clear();

		// restart.
		GatherTargets(targets_scanned_this_tick);
	}

	// we can just exit to knifebot here.
	if (g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS) {
		//knife();
		return;
	}

	// no potential targets.
	if (m_targets.empty())
		return;

	// sort targets into order for more efficient scanning.
	std::sort(m_targets.begin(), m_targets.end(), [](AimPlayer* a, AimPlayer* b) -> bool {
		LagRecord* first = &a->m_records.front();
		LagRecord* second = &a->m_records.front();

		// sort targets by sorting priority.
		switch (g_menu.main.aimbot.target_sorting.get()) {
		case 0: // distance
			return (first->m_origin - g_cl.m_shoot_pos).length() < (second->m_origin - g_cl.m_shoot_pos).length();
			break;
		case 1: // fov.
			return math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, first->m_origin) < math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, second->m_origin);
			break;
		case 2: // lag.
			return first->m_lag < second->m_lag;
			break;
		case 3: // health.
			return a->m_player->m_iHealth() < b->m_player->m_iHealth();
			break;
		case 4: // cycle.
			return false;
			break;
		}
	});

	// data for final target.
	BestTarget_t  best;
	BestTarget_t  temp;

	g_cl.m_fake_ping_delayed = false;

	// scan sorted targets in order.
	for (const auto& t : m_targets) {
		// we are whitelisting.
		if (t->m_goofy_whitelist || (t->m_voice_cheat == CHEAT_GOOFYHOOK && g_menu.main.aimbot.enable_goofy_whitelist.get())) {
			m_debug_render.push_back(tfm::format(XOR("aimbot -> whitelist : %s %s"), t->m_goofy_whitelist, t->m_voice_cheat));
			continue;
		}

		t->m_lagfix_mode = g_lagcomp.StartPrediction(t);

		if (t->m_lagfix_mode == LagCompensation::Lagfix_Modes::LAGCOMP_WAIT) {
			continue;
		}

		if (t->m_records.empty())
			continue;

		std::vector<LagRecord*> final_records;

		if (t->m_lagfix_mode == LagCompensation::Lagfix_Modes::LAGCOMP_PREDICT || t->m_lagfix_mode == LagCompensation::Lagfix_Modes::LAGCOMP_DONT_PREDICT || t->m_lagfix_mode == LagCompensation::Lagfix_Modes::LAGCOMP_WAIT) {
			LagRecord* cur = &t->m_records[0];
			if (!cur || cur->immune()) { // shouldnt have to check lagcomp bounds since theyre breaking lagcomp anyways.
				m_debug_render.push_back(tfm::format(XOR("aimbot -> lagcomp invalid : %s %s"), cur, cur->immune()));
				m_debug_render.push_back(tfm::format(XOR("aimbot -> lagcomp mode: %s"), std::to_string(t->m_lagfix_mode)));
				continue;
			}

			m_debug_render.push_back(t->m_lagfix_mode == LagCompensation::Lagfix_Modes::LAGCOMP_PREDICT ? tfm::format(XOR("aimbot -> lagfix -> extrapolate : %s %s"), t, t->m_lagfix_mode) : tfm::format(XOR("aimbot -> lagfix -> delay : %s %s"), t, std::to_string(t->m_lagfix_mode)));
			m_debug_render.push_back(tfm::format(XOR("aimbot -> lagcomp mode: %s"), std::to_string(t->m_lagfix_mode)));
			final_records.push_back(cur);
		}
		else {
			int last_idx = -1;
			bool last_record = false;

			bool needs_middle_record = false;
			bool was_distance = false;

			int last_resolved_record_idx = -1;

			bool has_oldest_standing_record = false;

			LagRecord* front = nullptr;
			for (int i = 0; i < t->m_records.size(); i++) {
				LagRecord* cur = &t->m_records[i];
				if (!cur || !cur->valid() || cur->immune() || cur->m_lagfixed)
					continue;

				if (i > 0) {
					g_cl.m_fake_ping_delayed = true;
				}

				front = cur;
				break;
			}

			if (!front)
				continue;

			final_records.push_back(front);

			bool front_sideways = g_resolver.IsYawSideways(t->m_player, front->m_eye_angles.y);
			bool front_resolved = front->m_resolver_mode > 4;

			for (int i = t->m_records.size() - 1; i >= 0; i--) {
				LagRecord* cur = &t->m_records[i];
				if (!cur || !cur->valid() || cur->immune() || cur->m_lagfixed)
					continue;

				float dist = (front->m_origin - cur->m_origin).length_2d();

				if (!last_record) {			
					if (dist > 0.1f) {
						final_records.push_back(cur);
						needs_middle_record = true;
						was_distance = true;
					}
					last_idx = i;
					last_record = true;
					continue;
				}

				if (needs_middle_record && i < last_idx / 2) {
					final_records.push_back(cur);
					needs_middle_record = false;
					continue;
				}

				if (was_distance) {
					if ((cur->m_resolver_mode >= front->m_resolver_mode && g_resolver.IsYawSideways(t->m_player, cur->m_eye_angles.y) && !front_sideways)
						|| !front_resolved && cur->m_resolver_mode > 4)
					{
						if (last_resolved_record_idx != -1) {
							final_records[last_resolved_record_idx] = nullptr;
						}

						final_records.push_back(cur);
						last_resolved_record_idx = final_records.size() - 1;
						continue;
					}
				}
				else {
					// the front record was resolved.
					// if we get to this point, we should not have added last or middle, and the front is resolved. so just break out of loop.
					if (front_resolved) {
						break;
					}

					if (!has_oldest_standing_record && abs(math::NormalizedAngle(cur->m_eye_angles.y - front->m_eye_angles.y)) < 0.1f) {
						has_oldest_standing_record = true;
						final_records.push_back(cur);
						continue;
					}
		
					if (cur->m_resolver_mode > 4) {
						final_records.push_back(cur);
						break;
					}
				}
			}
		}

		if (final_records.empty())
			continue;

		bool done_scanning = false;
		bool force_body = g_menu.main.aimbot.force_baim_after_misses.get() && t->m_shot_logs.m_missed_shots > g_menu.main.aimbot.force_baim_miss_count.get();
		bool disable_resolved = g_menu.main.aimbot.force_baim_after_misses_disable_resolved.get();

		for (int i = final_records.size() - 1; i >= 0; i--) {
			LagRecord* cur = final_records[i];
			if (!cur)
				continue;

			cur->cache();

			float player_health = t->m_player->m_iHealth();

			std::vector< AimPoint_t > points;
			t->SetupHitboxPoints(cur, cur->m_matrix, g_aimbot.GetMinDamage(player_health), force_body && !(cur->m_resolver_mode > 4 && disable_resolved), points);

			// needs to be -2 because limb hitbox safety is default to -1;
			float best_hitbox = -2.f;
			int previous_hitbox = -1;

			int override_priority = 0;

			if ((cur->m_resolver_mode <= 4 && t->m_unlikely_resolve > 0 && g_menu.main.aimbot.safety_conditions.get(0))
				|| (!(cur->m_flags & FL_ONGROUND) && g_menu.main.aimbot.safety_conditions.get(1))
				|| (cur->m_resolver_mode == Resolver::Modes::RESOLVE_WALK && cur->m_lag < 2 && g_menu.main.aimbot.safety_conditions.get(2)))
			{
				override_priority = 1;
			}
			else if ((cur->m_resolver_mode == Resolver::Modes::RESOLVE_BODY && g_menu.main.aimbot.damage_conditions.get(0))
				|| (cur->m_resolver_mode == Resolver::Modes::RESOLVE_WALK && g_menu.main.aimbot.damage_conditions.get(1))
				|| (cur->m_lag < 2 && g_menu.main.aimbot.damage_conditions.get(2)))
			{
				override_priority = 2;
			}

			if (i == 0) {
				t->m_first_record_hitscan_preference = override_priority;
			}

			for (int j = 0; j < points.size(); j++) {
				AimPoint_t cur_point = points[j];

				penetration::PenetrationInput_t in;
				in.m_damage = g_aimbot.GetMinDamage(player_health);
				in.m_can_pen = g_menu.main.aimbot.autowall.get();
				in.m_target = cur->m_player;
				in.m_from = g_cl.m_local;
				in.m_pos = cur_point.m_pos;
				penetration::PenetrationOutput_t out;

				if (!penetration::run(&in, &out)) {
					continue;
				}

				g_aimbot.m_stop = true;

				// force this if we could shoot at them.
				t->m_peek_data.m_localpeek_ticks = g_csgo.m_globals->m_tick_count + 32;

				if (cur_point.m_hitbox == previous_hitbox) {
					if (cur_point.m_point_safety > temp.point_safety) {
						temp.player = cur->m_player;
						temp.damage = out.m_damage;
						temp.pos = cur_point.m_pos;
						temp.record = cur;
						temp.hitbox = cur_point.m_hitbox;
						temp.hitgroup = HitboxToHitgroup(cur_point.m_hitbox);
						temp.point_safety = cur_point.m_point_safety;
					}
					continue;
				}

				float preference = g_menu.main.aimbot.hitbox_preference.get();
				switch (override_priority) {
				case 1:
					preference = 100.f;
					break;
				case 2:
					preference = -100.f;
					break;
				}

				float damage_preference = std::max(-preference, 1.f);
				float safety_preference = std::max(preference, 1.f);

				float damage_weight = (std::min(float(player_health), out.m_damage) / player_health) * damage_preference;
				float safety_weight = cur_point.m_hitbox_radius * safety_preference;

				float total_weight = damage_weight + safety_weight;

				if (total_weight > best_hitbox) {
					temp.player = cur->m_player;
					temp.damage = out.m_damage;
					temp.pos = cur_point.m_pos;
					temp.record = cur;
					temp.hitbox = cur_point.m_hitbox;
					temp.hitgroup = HitboxToHitgroup(cur_point.m_hitbox);
					temp.point_safety = cur_point.m_point_safety;

					best_hitbox = total_weight;
					previous_hitbox = cur_point.m_hitbox;
				}
			}

			if (temp.damage > 0.f) {
				if (g_menu.main.aimbot.delay_shot_modes.get(0) && (
					(t->m_peek_data.m_peek_damage_body >= player_health && temp.damage < player_health)
					|| (t->m_peek_data.m_peek_damage_body >= player_health && temp.hitbox == HITBOX_HEAD)
					|| (t->m_peek_data.m_peek_damage_body > temp.damage * 1.5f)
					)) 
				{
					m_debug_render.push_back(tfm::format(XOR("aimbot -> sds(d) : %s %s"), temp.damage, t->m_peek_data.m_peek_damage_body));
					continue; // continue would probably be better than break here, continue to check other records to see if theyre any better.
				}
				best = temp;
				done_scanning = true;
				break;
			}
		}

		if (done_scanning)
			break;
	}

	if (!best.player || !best.record)
		return;

	vec3_t backup_shootpos = g_cl.m_shoot_pos;

	// calculate aim angle.
	math::VectorAngles(best.pos - g_cl.m_shoot_pos, m_angle);

	// update shoot pos with new aim angle.
	UpdateShootpos(m_angle.x);

	// recalculate aim angle.
	math::VectorAngles(best.pos - g_cl.m_shoot_pos, m_angle);

	// our new shootpos is different.
	// check we can still hit the player.
	if ((g_cl.m_shoot_pos - backup_shootpos).length() > 0.1f) {
		penetration::PenetrationInput_t in;
		in.m_damage = GetMinDamage(best.player->m_iHealth());;
		in.m_can_pen = g_menu.main.aimbot.autowall.get();
		in.m_target = best.player;
		in.m_from = g_cl.m_local;
		in.m_pos = best.pos;
		penetration::PenetrationOutput_t out;

		if (!penetration::run(&in, &out)) {
			m_debug_render.push_back(XOR("aimbot -> retrace : failed"));
			return;
		}

		m_debug_render.push_back(XOR("aimbot -> retrace : passed"));
	}

	ang_t _anglebruh;
	math::VectorAngles(best.pos - g_cl.m_shoot_pos, _anglebruh);

	if (!g_aimbot.CheckHitchance(best.player, _anglebruh, best.record, best.hitbox)) {
		m_debug_render.push_back(XOR("aimbot -> hc : failed"));
		return;
	}

	m_debug_render.push_back(XOR("aimbot -> hc : passed"));

	// set member vars.
	m_target = best.player;
	m_aim = best.pos;
	m_damage = best.damage;
	m_record = best.record;
	m_hitbox = best.hitbox;
	m_hitgroup = best.hitgroup;

	// we have a normal weapon or a non cocking revolver
	// choke if its the processing tick.
#ifndef _DEBUG
	if (!g_cl.m_lag) {
		*g_cl.m_packet = false;
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
		g_hvh.SendPacket();
		return;
	}
#endif 

	if (!g_menu.main.aimbot.silent.get()) {
		g_csgo.m_engine->SetViewAngles(m_angle);
	}

	// norecoil.
	m_angle -= g_cl.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

	// shot the shot angle on radar.
	g_cl.m_cmd->m_view_angles = g_cl.m_radar = m_angle;

	if (g_menu.main.aimbot.autofire.get()) {
		g_cl.m_cmd->m_buttons |= IN_ATTACK;
	}

	if (g_cl.m_cmd->m_buttons & IN_ATTACK) {
		*g_cl.m_packet = false;

		g_cl.m_cmd->m_tick = game::TIME_TO_TICKS(m_record->m_sim_time + g_cl.m_lerp);

		g_cl.m_shot = true;

		g_movement.m_autopeek_return = true;

		g_shots.OnShotFire(m_target ? m_target : nullptr, m_target ? m_damage : -1.f, g_cl.m_weapon_info->m_bullets, m_target ? m_record : nullptr, m_hitbox);


		if (g_menu.main.aimbot.aimbot_debug.get(2)) {



			player_info_t info;
			if (g_csgo.m_engine->GetPlayerInfo(m_target->index(), &info)) {
				g_notify.add(tfm::format("fired shot at %s ( hb: %s [%s] | dmg: %s | bt: %s | r: %s | lc: %s | lcfix: %s)\n", 
					info.m_name, getHitboxName(m_hitbox), m_hitgroup, m_damage, game::TIME_TO_TICKS(m_target->m_flSimulationTime() - m_record->m_sim_time), m_record->m_resolver_mode, m_record->m_broke_lc, m_record->m_lagfixed));
			}
		}

		g_chams.AddHitMatrix(m_target, m_record->m_matrix);

		if (g_menu.main.aimbot.aimbot_debug.get(0)) {
			//g_visuals.DrawHitboxMatrix(m_record, m_record->m_matrix, m_hitbox, Color(255, 0, 0));
			//g_visuals.DrawHitboxMatrix(m_record, m_backup[m_target->index() - 1].m_matrix, m_hitbox, Color(0, 0, 255));
			g_visuals.DrawHitboxMatrix(m_record, m_record->m_matrix, m_hitbox, Color(255, 0, 0));
			g_visuals.DrawHitboxMatrix(m_record, m_backup[m_target->index() - 1].m_matrix, m_hitbox, Color(0, 0, 255));

			g_visuals.DrawSkeletonOnHit(m_record, m_record->m_matrix, m_hitbox, 255);
			g_visuals.DrawSkeletonOnHit(m_record, m_backup[m_target->index() - 1].m_matrix, m_hitbox, 255);
		}
		if (!g_menu.main.aimbot.aimbot_debug.get(0)) {
			g_visuals.DrawHitboxMatrix(m_record, m_record->m_matrix, m_hitbox, Color(255, 0, 0));
			g_visuals.DrawHitboxMatrix(m_record, m_backup[m_target->index() - 1].m_matrix, m_hitbox, Color(0, 0, 255));

			g_visuals.DrawSkeletonOnHit(m_record, m_record->m_matrix, m_hitbox, 255);
			g_visuals.DrawSkeletonOnHit(m_record, m_backup[m_target->index() - 1].m_matrix, m_hitbox, 255);
		}
		// so messy. but fukit, this method better because it shows without spread.
		if (g_menu.main.misc.bullet_impacts.get()) {
			CGameTrace trace;
			CTraceFilterSimple filter;
			filter.SetPassEntity(g_cl.m_local);

			g_csgo.m_engine_trace->TraceRay(Ray(g_cl.m_shoot_pos, m_aim), 0x46004003, &filter, &trace);

			g_csgo.m_debug_overlay->AddBoxOverlay(trace.m_endpos, vec3_t(-2.f, -2.f, -2.f), vec3_t(2.f, 2.f, 2.f), ang_t(0.f, 0.f, 0.f), 255, 0, 0, 127, 3.f);

			if ((trace.m_endpos - m_aim).length_2d() > 10.f)
				g_csgo.m_debug_overlay->AddBoxOverlay(m_aim, vec3_t(-2.f, -2.f, -2.f), vec3_t(2.f, 2.f, 2.f), ang_t(0.f, 0.f, 0.f), 255, 0, 0, 127, 3.f);
		}
	}
}

bool Aimbot::CheckHitchance(Player* player, const ang_t& angle, LagRecord* lagrecord, int hitbox) {
	static const int max_seeds = 255;

	if (precomputed_seeds.empty()) {
		for (int i = 0; i <= max_seeds; i++) {
			const auto pi_seed = g_csgo.RandomFloat(0.f, 6.28318531);
			precomputed_seeds.emplace_back(g_csgo.RandomFloat(0.f, 1.f), sin(pi_seed), cos(pi_seed));
		}
		m_debug_render.push_back(XOR("aimbot -> hc : computing seeds"));
	}

	if (!g_cl.m_weapon_fire)
		return false;

	const float min_chance = (g_cl.m_weapon_id == ZEUS ? g_menu.main.aimbot.hitchance_amount_zeus.get() : g_menu.main.aimbot.hitchance_amount.get()) / 100.f;
	if (min_chance < 0.02f)
		return true;

	const auto weapon_inaccuracy = g_cl.m_weapon->GetInaccuracy();
	if (!(g_cl.m_flags & FL_ONGROUND) && g_cl.m_weapon_id == SSG08) {
		m_debug_render.push_back(XOR("aimbot -> hc : jumpscout"));
		return weapon_inaccuracy < 0.009f;
	}

	const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };
	const auto sniper = g_cl.m_weapon->m_iItemDefinitionIndex() == AWP || g_cl.m_weapon->m_iItemDefinitionIndex() == G3SG1 || g_cl.m_weapon->m_iItemDefinitionIndex() == SCAR20 || g_cl.m_weapon->m_iItemDefinitionIndex() == SSG08;
	const auto crouched = g_cl.m_local->m_fFlags() & IN_DUCK;
	const auto info = g_cl.m_weapon_info;

	// no need for hitchance, if we can't increase it anyway.
	if (info) {
		if (crouched) {
			if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->m_inaccuracy_crouch_alt : info->m_inaccuracy_crouch)) {
				return true;
			}
		}
		else {
			if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->m_inaccuracy_stand_alt : info->m_inaccuracy_stand)) {
				return true;
			}
		}
	}

	vec3_t forward, right, up;
	math::AngleVectors(angle, &forward, &right, &up);

	vec3_t total_spread, end;
	float inaccuracy, spread_x, spread_y;
	ang_t spread_angle;

	int hits = 0;
	float mindamage = GetMinDamage(player->m_iHealth());

	std::tuple<float, float, float>* seed;
	for (int i = 0; i <= max_seeds; i++) {
		seed = &precomputed_seeds[i];

		inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
		spread_x = std::get<2>(*seed) * inaccuracy;
		spread_y = std::get<1>(*seed) * inaccuracy;
		total_spread = (forward + right * spread_x + up * spread_y).normalized();

		math::VectorAngles(total_spread, spread_angle);
		math::AngleVectors(spread_angle, end);

		end = g_cl.m_shoot_pos + end.normalized() * g_cl.m_weapon_info->m_range;

		penetration::PenetrationInput_t in;
		in.m_damage = mindamage;
		in.m_can_pen = true;
		in.m_target = player;
		in.m_from = g_cl.m_local;
		in.m_pos = end;
		penetration::PenetrationOutput_t out;

		if (penetration::run(&in, &out)) {
			++hits;
		}

		// abort if hitchance is already sufficent.
		if (static_cast<float>(hits) / static_cast<float>(max_seeds) >= min_chance)
			return true;

		// abort if we can no longer reach hitchance.
		if (static_cast<float>(hits + max_seeds - i) / static_cast<float>(max_seeds) < min_chance)
			return false;
	}

	return static_cast<float>(hits) / static_cast<float>(max_seeds) >= min_chance;
}

void AimPlayer::SetupHitboxPoints(LagRecord* record, matrix3x4_t* record_matrix, float mindmg, bool force_baim_misses, std::vector< AimPoint_t >& points) {
	const model_t* model = m_player->GetModel();
	if (!model)
		return;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(m_player->m_nHitboxSet());
	if (!set)
		return;

	// get hitbox scales.
	const float head_scale = g_menu.main.aimbot.head_pointscale.get() / 100.f;
	const float head_aimheight = g_menu.main.aimbot.head_aimheight.get() / 100.f;
	const float body_scale = g_menu.main.aimbot.body_pointscale.get() / 100.f;

	// more good.
	bool use_debug_color = g_menu.main.aimbot.aimbot_debug.get(1);

	// iterate hitboxes and add aimpoints.
	for (int index = 0; index < HITBOX_MAX; index++) {
		if (!g_aimbot.AllowHitbox(record->m_player, index, mindmg))
			continue;

		if (index == HITBOX_HEAD && force_baim_misses) {
			g_aimbot.m_debug_render.push_back(XOR("multipoint -> disable head"));
			continue;
		}

		mstudiobbox_t* bbox = set->GetHitbox(index);
		if (!bbox)
			continue;

		// compute raw center point.
		vec3_t center = (bbox->m_mins + bbox->m_maxs) / 2.f;

		// dont need accuracy calc anymore.
		auto AddHitboxPoint = [&](vec3_t positionz) -> void {
			AimPoint_t temppoint;
			temppoint.m_pos = positionz;
			temppoint.m_hitbox = index;

			// default to -1.f
			if (bbox->m_radius > 0.f) {
				temppoint.m_hitbox_radius = bbox->m_radius;
				temppoint.m_point_safety = 1.f - ((positionz - center).length() / bbox->m_radius);
			}

			points.push_back(temppoint);
		};

		// these indexes represent boxes.
		if (bbox->m_radius <= 0.f) {
			// FUK DIS SHIT NIGGA
			if (!m_peek_data.m_filtered && g_menu.main.aimbot.aimbot_optimizations.get(1)) {
				continue;
			}

			// references: 
			//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
			//      CBaseAnimating::GetHitboxBonePosition
			//      CBaseAnimating::DrawServerHitboxes

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record_matrix[bbox->m_bone], rot_matrix, matrix);

			// extract origin from matrix.
			vec3_t origin = matrix.GetOrigin();

			// the feet hiboxes have a side, heel and the toe.
			if (index == HITBOX_R_FOOT || index == HITBOX_L_FOOT) {
				float d1 = (bbox->m_mins.z - center.z) * 0.875f;

				// invert.
				if (index == HITBOX_L_FOOT)
					d1 *= -1.f;

				// side is more optimal then center.
				AddHitboxPoint({ center.x, center.y, center.z + d1 });

				if (g_menu.main.aimbot.multipoint.get(3)) {
					// get point offset relative to center point
					// and factor in hitbox scale.
					float d2 = (bbox->m_mins.x - center.x) * body_scale;
					float d3 = (bbox->m_maxs.x - center.x) * body_scale;

					// heel.
					AddHitboxPoint({ center.x + d2, center.y, center.z });

					// toe.
					AddHitboxPoint({ center.x + d3, center.y, center.z });
				}
			}

			// nothing to do here we are done.
			if (points.empty())
				continue;

			// rotate our bbox points by their correct angle
			// and convert our points to world space.
			for (auto& p : points) {
				if (p.m_rotated)
					continue;

				// VectorRotate.
				// rotate point by angle stored in matrix.
				p.m_pos = { p.m_pos.dot(matrix[0]), p.m_pos.dot(matrix[1]), p.m_pos.dot(matrix[2]) };

				// transform point to world space.
				p.m_pos += origin;

				// mark as rotated.
				p.m_rotated = true;
			}
		}

		// these hitboxes are capsules.
		else {
			// factor in the pointscale.
			float bs = bbox->m_radius * body_scale;

			// shawty trifflin.
			if (m_peek_data.m_filtered || !g_menu.main.aimbot.aimbot_optimizations.get(1)) {
				// head has 5 points.
				if (g_menu.main.aimbot.multipoint.get(0) && index == HITBOX_HEAD) {
					// rotation matrix 45 degrees.
					// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
					// std::cos( deg_to_rad( 45.f ) )
					constexpr float rotation = 0.70710678f;

					for (int i = 0; i <= g_menu.main.aimbot.multipoint_intensity.get(); i++) {
						float hs_width = bbox->m_radius * (head_scale * ((3.f - i) / 3.f));
						float hs_height = bbox->m_radius * (head_aimheight * ((3.f - i) / 3.f));

						// left.
						AddHitboxPoint({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - hs_width });

						// right.
						AddHitboxPoint({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + hs_width });

						// top/back/left 45 deg.
						AddHitboxPoint({ bbox->m_maxs.x + (rotation * hs_height), bbox->m_maxs.y + (-rotation * hs_width), bbox->m_maxs.z + (-rotation * hs_width) });

						// top/back/right 45 deg.
						AddHitboxPoint({ bbox->m_maxs.x + (rotation * hs_height), bbox->m_maxs.y + (-rotation * hs_width), bbox->m_maxs.z + (rotation * hs_width) });

						// top.
						AddHitboxPoint({ bbox->m_maxs.x + hs_height, bbox->m_maxs.y, bbox->m_maxs.z });

						// back.
						AddHitboxPoint({ bbox->m_maxs.x, bbox->m_maxs.y - hs_width, bbox->m_maxs.z });

						// top/back 45 deg.
						AddHitboxPoint({ bbox->m_maxs.x + (rotation * hs_height), bbox->m_maxs.y + (-rotation * hs_width), bbox->m_maxs.z });
					}
				}

				// body has 5 points.
				else if (index == HITBOX_BODY || index == HITBOX_PELVIS) {
					if (g_menu.main.aimbot.multipoint.get(2)) {
						// right.
						AddHitboxPoint({ center.x, center.y, center.z - bs });

						// left.
						AddHitboxPoint({ center.x, center.y, center.z + bs });

						// back.
						AddHitboxPoint({ center.x, bbox->m_maxs.y - bs, center.z });
					}
				}

				// other stomach/chest hitboxes have 2 points.
				else if (index == HITBOX_THORAX || index == HITBOX_CHEST || index == HITBOX_UPPER_CHEST) {
					if (g_menu.main.aimbot.multipoint.get(1)) {
						// right.
						AddHitboxPoint({ center.x, center.y, center.z - bs });

						// left.
						AddHitboxPoint({ center.x, center.y, center.z + bs });

						// back.
						AddHitboxPoint({ center.x, bbox->m_maxs.y - bs, center.z });
					}
				}

				else if (index == HITBOX_R_CALF || index == HITBOX_L_CALF) {
					// half bottom.
					if (g_menu.main.aimbot.multipoint.get(3)) {
						AddHitboxPoint({ bbox->m_maxs.x - (bbox->m_radius / 2.f), bbox->m_maxs.y, bbox->m_maxs.z });
					}
				}
			}

			// add center.
			AddHitboxPoint(center);

			// nothing left to do here.
			if (points.empty())
				continue;

			// this can be done outside of the loop below, beacuse this whole thing is in hitbox loop.		
			vec3_t center_transgender = center;
			math::VectorTransform(center_transgender, record_matrix[bbox->m_bone], center_transgender);

			// transform capsule points.
			for (auto& p : points) {
				if (p.m_rotated)
					continue;

				math::VectorTransform(p.m_pos, record_matrix[bbox->m_bone], p.m_pos);

				CGameTrace tr;
				g_csgo.m_engine_trace->ClipRayToEntity(Ray(p.m_pos, center_transgender), MASK_SHOT, record->m_player, &tr);

				if (tr.m_fraction > 0.f && tr.m_entity == record->m_player) {
					p.m_pos = tr.m_endpos;
				}

				// mark as rotated
				p.m_rotated = true;
			}
		}
	}
}

bool Aimbot::CanHit(vec3_t start, vec3_t end, LagRecord* record, int box, bool write_matrix) {
	if (!record || !record->m_player)
		return false;

	const model_t* model = record->m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(box);
	if (!bbox)
		return false;

	vec3_t min, max;
	const auto IsCapsule = bbox->m_radius != -1.f;

	if (IsCapsule) {
		math::VectorTransform(bbox->m_mins, record->m_matrix[bbox->m_bone], min);
		math::VectorTransform(bbox->m_maxs, record->m_matrix[bbox->m_bone], max);
		const auto dist = math::SegmentToSegment(start, end, min, max);

		if (dist < bbox->m_radius) {
			return true;
		}
	}
	else {
		CGameTrace tr;

		// basically will never need this lol.
		if (write_matrix)
			record->cache();

		// setup ray and trace.
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT | CONTENTS_GRATE | CONTENTS_WINDOW, record->m_player, &tr);

		// we dont need to restore matrices, it already does at the end of createmove.

		// check if we hit a valid player / hitgroup on the player and increment total hits.
		if (tr.m_entity == record->m_player && tr.m_hitbox == box) {
			return true;
		}
	}

	return false;
}
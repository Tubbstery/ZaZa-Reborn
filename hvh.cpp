#include "includes.h"

HVH g_hvh{ };;

Player* HVH::GetBestPlayer() {
	// best target.
	Player* bestplayer = nullptr;
	float bestfov = 180.f + 1.f;

	// iterate players.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		// validate player.
		if (!g_aimbot.IsValidTarget(player))
			continue;

		// skip dormant players.
		if (player->dormant())
			continue;

		// get best target based on fov.
		float fov = math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, player->WorldSpaceCenter());

		if (fov < bestfov) {
			bestfov = fov;
			bestplayer = player;
		}
	}

	if (!bestplayer) {
		return nullptr;
	}

	return bestplayer;
}

float HVH::BossStanding() {
	if (!g_menu.main.antiaim.freestand_yaw.get()) {
		return 0.f;
	}

	Player* enemy = GetBestPlayer();
	if (!enemy) {
		return 0.f;
	}

	static const auto extension = 24.0f;
	vec3_t origin = enemy->m_vecOrigin();
	vec3_t eye_pos;
	enemy->GetEyePos(&eye_pos);

	ang_t direction;
	math::VectorAngles(origin - g_cl.m_local->m_vecOrigin(), direction);

	vec3_t forward, right, up;
	math::AngleVectors(direction, &forward, &right, &up);

	CGameTrace tr;
	CTraceFilter filter;
	filter.skip_entity = g_cl.m_local;

	g_csgo.m_engine_trace->TraceRay(Ray(g_cl.m_shoot_pos - right * extension, eye_pos - right * extension), MASK_SHOT, (ITraceFilter*)&filter, &tr);
	float left_frac = tr.m_fraction;

	g_csgo.m_engine_trace->TraceRay(Ray(g_cl.m_shoot_pos + right * extension, eye_pos + right * extension), MASK_SHOT, (ITraceFilter*)&filter, &tr);
	float right_frac = tr.m_fraction;

	bool trace_left = left_frac > right_frac;
	
	if (abs(left_frac - right_frac) < 0.33f) {
		return 0.f;
	}
	else {
		if (trace_left) {
			return 90.f;
		}
		else {
			return -90.f;
		}
	}
}

void HVH::AntiAim() {
	bool attack, attack2;

	if (!g_menu.main.antiaim.enable.get())
		return;

	attack = g_cl.m_cmd->m_buttons & IN_ATTACK;
	attack2 = g_cl.m_cmd->m_buttons & IN_ATTACK2;

	if (g_cl.m_weapon && g_cl.m_weapon_fire) {
		bool knife = g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS;
		bool revolver = g_cl.m_weapon_id == REVOLVER;

		// if we are in attack and can fire, do not anti-aim.
		if (attack || (attack2 && (knife || revolver)))
			return;
	}

	// grenade throwing
	// CBaseCSGrenade::ItemPostFrame()
	// https://github.com/VSES/SourceEngine2007/blob/master/src_main/game/shared/cstrike/weapon_basecsgrenade.cpp#L209
	if (g_cl.m_weapon_type == WEAPONTYPE_GRENADE && 
		(!g_cl.m_weapon->m_bPinPulled() || attack || attack2) 
		&& g_cl.m_weapon->m_fThrowTime() > 0.f 
		&& g_cl.m_weapon->m_fThrowTime() < g_csgo.m_globals->m_curtime) 
	{
		*g_cl.m_packet = true;
		m_desync_walk_active = true;
		return;
	}

	m_mode = AntiAimMode::STAND;
	if ((g_cl.m_buttons & IN_JUMP) || !(g_cl.m_flags & FL_ONGROUND)) {
		m_mode = AntiAimMode::AIR;
	}
	else if (g_cl.m_local->m_vecVelocity().length_2d() > 0.1f && !g_movement.m_fake_walking) {
		m_mode = AntiAimMode::WALK;
	}

	// do not allow 2 consecutive sendpacket true if faking angles.
	if (*g_cl.m_packet && g_cl.m_old_packet) {
		*g_cl.m_packet = false;
	}

	// disable conditions.
	if (g_cl.m_cmd->m_buttons & IN_USE) {
		g_hvh.m_desync_walk = m_desync_walk_active = false;
		return;
	}

	CCSGOPlayerAnimState* animstate = g_cl.m_local->m_PlayerAnimState();
	if (!animstate) {
		return;
	}

	switch (g_menu.main.antiaim.pitch.get()) {
	case 0: // down
		g_cl.m_cmd->m_view_angles.x = 89.f;
		break;
	case 1: // up
		g_cl.m_cmd->m_view_angles.x = -89.f;
		break;
	case 2: // zero
		g_cl.m_cmd->m_view_angles.x = 0.f;
		break;
	case 3: // random
		g_cl.m_cmd->m_view_angles.x = g_csgo.RandomFloat(-89.f, 89.f);
		break;
	}

	static float last_move_lby;
	static bool jitter_switch;
	static int lby_ticks;

	static int desync_walk_ticks;

	static float last_fake_yaw;
	static bool fake_yaw_overlap;

	float worldzero = math::CalcAngle(g_cl.m_local->m_vecOrigin(), vec3_t(0, 0, 0)).y;
	float view_before_mod = g_cl.m_cmd->m_view_angles.y;

	if (g_hvh.m_retardmode) {
		if (jitter_switch) {
			g_cl.m_cmd->m_view_angles.x = g_menu.main.antiaim.retardmode_x1.get();
			g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.retardmode_y1.get();

			g_cl.m_hidden_angles = g_cl.m_view_angles;
		}
		else {
			g_cl.m_cmd->m_view_angles.x = g_menu.main.antiaim.retardmode_x2.get();
			g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.retardmode_y2.get();

			g_cl.m_hide_angles = g_menu.main.antiaim.retardmode_hide_angles.get();
		}

		// run the fake on sendpacket true.
		if (*g_cl.m_packet && *g_cl.m_final_packet) {
			g_cl.m_cmd->m_view_angles.y += 180.f;
			return;
		}
		
		if (g_hvh.m_pre_retardmode) {
			if (!g_hvh.m_pre_retardmode_swap_next_tick) {
				g_cl.m_cmd->m_view_angles.y = view_before_mod - 90.f;
				g_hvh.m_pre_retardmode_swap_next_tick = true;
			}
			else {
				g_cl.m_cmd->m_view_angles.y = view_before_mod + 90.f;
				g_hvh.m_pre_retardmode = false;
			}

			jitter_switch = false;
			return;
		}

		jitter_switch = !jitter_switch;
		return;
	}

	// run the fake on sendpacket true.
	if (*g_cl.m_packet && *g_cl.m_final_packet) {
		switch (g_menu.main.antiaim.fake_yaw_type.get()) {
		case 0: // none
			break;
		case 1: // custom
			// case 0 is local view
			switch (g_menu.main.antiaim.fake_yaw_base.get()) {
			case 1: // lower body
				g_cl.m_cmd->m_view_angles.y = g_cl.m_body_lol;
				break;
			case 2: // zero
				g_cl.m_cmd->m_view_angles.y = 0.f;
				break;
			}

			if (m_mode == STAND && g_menu.main.antiaim.fake_yaw_safety.get() && fake_yaw_overlap) {
				g_cl.m_cmd->m_view_angles.y += g_csgo.RandomFloat(-45.f, 45.f);
			}

			last_fake_yaw = g_cl.m_cmd->m_view_angles.y;
			return;
		case 2: // random
			g_cl.m_cmd->m_view_angles.y = g_csgo.RandomFloat(-180.f, 180.f);
			return;
		}
	}

	// do manual aa.
	if (m_manual_direction > 0) {
		switch (m_manual_direction) {
		case 2:
			g_cl.m_cmd->m_view_angles.y += 90.f;
			break;
		case 3:
			g_cl.m_cmd->m_view_angles.y += 180.f;
			break;
		case 4:
			g_cl.m_cmd->m_view_angles.y -= 90.f;
			break;
		}

		if (m_mode == WALK) {
			last_move_lby = g_cl.m_body_lol;
		}

		g_hvh.m_desync_walk = false;

		return;
	}

	// do desync walk.
	if (desync_walk_ticks > 0) {
		m_desync_walk = false;
	}

	if (m_desync_walk && g_cl.m_pressing_move) {
		++desync_walk_ticks;
		m_desync_walk_active = true;
		g_cl.m_cmd->m_view_angles.y = g_cl.m_cmd->m_view_angles.y - (g_cl.m_body_lol - g_cl.m_abs_yaw);
		return;
	}

	if (abs(g_cl.m_local->m_flPoseParameter()[11] - 0.5f) < 0.1f  || m_mode == AIR) {
		m_desync_walk_active = false;
	}
	desync_walk_ticks = 0;

	// set base yaw.
	switch (g_menu.main.antiaim.base_yaw.get()) {
	case 0:
		g_cl.m_cmd->m_view_angles.y += 180.f;
		break;
	case 1:
		g_cl.m_cmd->m_view_angles.y = worldzero;
		break;
	}

	// apply offset.
	g_cl.m_cmd->m_view_angles.y += g_menu.main.antiaim.base_yaw_offset.get();

	// bruh.
	if (m_desync_walk_active && m_mode != STAND) {
		if (g_menu.main.antiaim.feetyaw_desync_extend.get() && abs(g_cl.m_local->m_flPoseParameter()[11] - 0.5f) < 0.2f) {
			bool side = g_cl.m_local->m_flPoseParameter()[11] > 0.5f;
			float bruhyaw = side ? animstate->m_abs_yaw - 17.5f : animstate->m_abs_yaw + 17.5f;
			g_cl.m_cmd->m_view_angles.y = bruhyaw;
		}

		Player* bestplayer = GetBestPlayer();
		if (bestplayer) {
			float at_target_yaw = math::CalcAngle(bestplayer->m_vecOrigin(), g_cl.m_local->m_vecOrigin()).y;

			if (abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - at_target_yaw)) > 70.f) {
				m_desync_walk_active = false;
			}
		}
	}
	else {
		// add da modifier asdhsadhasdh.
		switch (g_menu.main.antiaim.base_yaw_modifiers.get()) {
		case 1:
			g_cl.m_cmd->m_view_angles.y += jitter_switch ? -g_menu.main.antiaim.modifier_range.get() / 2 : g_menu.main.antiaim.modifier_range.get() / 2;
			break;
		case 2:
			g_cl.m_cmd->m_view_angles.y += cos(g_csgo.m_globals->m_curtime * (g_menu.main.antiaim.modifier_speed.get() / 10.f)) * g_menu.main.antiaim.modifier_range.get();
			break;
		}

		// do freestanding.
		float freestand = BossStanding();
		if (freestand != 0.f) {
			g_cl.m_cmd->m_view_angles.y = view_before_mod + 180.f + freestand;
		}

		switch (g_menu.main.antiaim.lby_modifiers.get()) {
		case 1:
			if (m_mode == STAND) {
				bool side = math::NormalizedAngle(animstate->m_abs_yaw - (view_before_mod + 180.f)) > 0.f;

				float bruhyaw = side ? animstate->m_abs_yaw - 35.f : animstate->m_abs_yaw + 35.f;
				g_cl.m_cmd->m_view_angles.y = bruhyaw;
			}
			// in air yoza.
			else if (m_mode == AIR) {
				g_cl.m_cmd->m_view_angles.y = view_before_mod + 180 + (jitter_switch ? 45.f : -45.f);
			}

			break;
		case 2:
			if (m_mode == STAND) {
				float goal_lby = g_menu.main.antiaim.lby_breaker_lastmoving.get() ? last_move_lby : g_cl.m_cmd->m_view_angles.y + g_menu.main.antiaim.lby_breaker_range.get();

				// lby will update this tick.
				if (!g_cl.m_lag && g_csgo.m_globals->m_curtime >= g_cl.m_body_pred && lby_ticks == 0) {
					g_cl.m_cmd->m_view_angles.y = goal_lby;
					//g_cl.m_hide_angles = true;
					++lby_ticks;
					return;
				}

				if (g_menu.main.antiaim.lby_breaker_lastmoving.get()) {
					bool side = abs(math::NormalizedAngle(view_before_mod + 180.f) - g_cl.m_body_lol + 135.f) < abs(math::NormalizedAngle(view_before_mod + 180.f) - g_cl.m_body_lol - 135.f);

					g_cl.m_cmd->m_view_angles.y = side ? g_cl.m_body_lol + 135.f : g_cl.m_body_lol - 135.f;

					if (abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - animstate->m_abs_yaw)) <= 35.f) {
						g_cl.m_cmd->m_view_angles.y += 180.f;
					}
				}

				lby_ticks = 0;
			}
			break;
		}
	}

	if (m_mode == AIR) {
		while (abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - (view_before_mod + 180.f + BossStanding()))) <= 45.f
			|| abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - last_move_lby)) <= 45.f
			|| abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - g_cl.m_body_lol)) <= 45.f) {
			// just to add some randomization lol.
			if (g_cl.m_local->m_nTickBase() % 2 == 0) {
				++g_cl.m_cmd->m_view_angles.y;
			}
			else {
				--g_cl.m_cmd->m_view_angles.y;
			}
		}
	}

	jitter_switch = !jitter_switch;

	g_cl.m_hidden_angles = g_cl.m_cmd->m_view_angles;
	fake_yaw_overlap = abs(math::NormalizedAngle(g_cl.m_cmd->m_view_angles.y - last_fake_yaw)) <= 35.f;

	if (m_mode == WALK) {
		last_move_lby = g_cl.m_body_lol;
	}
}

void HVH::SendPacket() {
	// if not the last packet this shit wont get sent anyway.
	// fix rest of hack by forcing to false.
	if (!*g_cl.m_final_packet)
		*g_cl.m_packet = false;

	// fake-lag enabled.
	if (g_menu.main.antiaim.lag_enable.get() && !g_csgo.m_gamerules->m_bFreezePeriod() && !(g_cl.m_flags & FL_FROZEN)) {
		// limit of lag.
		int limit = std::min((int)g_menu.main.antiaim.lag_limit.get(), g_cl.m_max_lag);

		// indicates wether to lag or not.
		bool active{ };

		// get current origin.
		vec3_t cur = g_cl.m_local->m_vecOrigin();

		// get prevoius origin.
		vec3_t prev = g_cl.m_net_pos.empty() ? g_cl.m_local->m_vecOrigin() : g_cl.m_net_pos.front().m_pos;

		// delta between the current origin and the last sent origin.
		float delta = (cur - prev).length_sqr();

		auto activation = g_menu.main.antiaim.lag_active.GetActiveIndices();
		for (auto it = activation.begin(); it != activation.end(); it++) {

			// move.
			if (*it == 0 && delta > 0.1f && g_movement.m_speed > 0.1f) {
				active = true;
				break;
			}

			// air.
			else if (*it == 1 && ((g_cl.m_buttons & IN_JUMP) || !(g_cl.m_flags & FL_ONGROUND))) {
				active = true;
				break;
			}

			// crouch.
			else if (*it == 2 && g_cl.m_local->m_bDucking()) {
				active = true;
				break;
			}
		}

		if (active) {
			int mode = g_menu.main.antiaim.lag_mode.get();

			// max.
			if (mode == 0)
				*g_cl.m_packet = false;

			// break.
			else if (mode == 1 && delta <= 4096.f)
				*g_cl.m_packet = false;

			// random.
			else if (mode == 2) {
				// compute new factor.
				if (g_cl.m_lag >= m_random_lag)
					m_random_lag = g_csgo.RandomInt(2, limit);

				// factor not met, keep choking.
				else *g_cl.m_packet = false;
			}

			// break step.
			else if (mode == 3) {
				// normal break.
				if (m_step_switch) {
					if (delta <= 4096.f)
						*g_cl.m_packet = false;
				}

				// max.
				else *g_cl.m_packet = false;
			}

			if (g_cl.m_lag >= limit)
				*g_cl.m_packet = true;
		}
	}

	if (!g_menu.main.antiaim.lag_land.get()) {
		vec3_t                start = g_cl.m_local->m_vecOrigin(), end = start, vel = g_cl.m_local->m_vecVelocity();
		CTraceFilterWorldOnly filter;
		CGameTrace            trace;

		// gravity.
		vel.z -= (g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval);

		// extrapolate.
		end += (vel * g_csgo.m_globals->m_interval);

		// move down.
		end.z -= 2.f;

		g_csgo.m_engine_trace->TraceRay(Ray(start, end), MASK_SOLID, &filter, &trace);

		// check if landed.
		if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z > 0.7f && !(g_cl.m_flags & FL_ONGROUND))
			*g_cl.m_packet = true;
	}

	// force fake-lag to 14 when fakelagging.
	if (g_input.GetKeyState(g_menu.main.antiaim.fakewalk.get())) {
		*g_cl.m_packet = false;
	}

	// do not lag while shooting.
	if (g_cl.m_old_shot)
		*g_cl.m_packet = true;

	// we somehow reached the maximum amount of lag.
	// we cannot lag anymore and we also cannot shoot anymore since we cant silent aim.
	if (g_cl.m_lag >= g_cl.m_max_lag) {
		// set bSendPacket to true.
		*g_cl.m_packet = true;

		// disable firing, since we cannot choke the last packet.
		g_cl.m_weapon_fire = false;
	}
}
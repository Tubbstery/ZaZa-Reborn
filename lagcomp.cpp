#include "includes.h"

LagCompensation g_lagcomp{};;

void AimPlayer::AddRecord(Player* player) {
	// if this happens, delete all the lagrecords.
	if (!g_menu.main.aimbot.enable.get() || !player->alive() || !player->enemy(g_cl.m_local)) {
		m_records.clear();
		return;
	}

	// update player ptr if required.
	// reset player if changed.
	if (m_player != player) {
		m_records.clear();
		m_peek_data.m_filtered = false;
		m_voice_cheat = CHEAT_UNKNOWN;
		m_goofy_whitelist = false;
		m_cant_hit_lby = true;
		resolver_last_origin.clear();
		m_last_ground_yaw = 0.0f;
		m_lby_timer = 0.0f;
		m_move_lby = 0.0f;
		m_move_sim_time = 0.0f;
		m_lby_delta = 0.0f;
		m_og_lby_delta = 0.0f;
		m_calc_lby = true; // actually want it to be true, so we dont calculate some bs.
		m_prev_calc_lby = true;
		m_updates_since_lby_update_tick = 2; // dont want it to be 0 or 1 which will change resolving.
		m_lby_updates_since_moving = 2; // cant be < 2, or will change resolving.
		m_times_lby_changed_since_first_break = 0; // bruh.
		m_static_fake_delta = 180.f;
		m_jitter_fake_delta = 180.f;
		m_shot_logs.m_hit_lby_deltas.clear();
		m_shot_logs.m_missed_lby_deltas.clear();
		m_shot_logs.m_missed_shots = 0;
		m_prev_missed_shots_lol = 0;
		m_fake_type = Resolver::Fakes::FAKE_UNKNOWN;
		m_recalculate_fake_offsets = false;
		m_unlikely_resolve = 0;
		//m_brute_angles.clear();
	}

	// update player ptr.
	m_player = player;

	// no need to store insane amt of data.
	while (m_records.size() > int(1.0f / g_csgo.m_globals->m_interval)) {
		m_records.pop_back();
	}

	// lol new anim system fuk dis.
	if (player->dormant()) {
		if (m_records.size() > 0 && (m_records.front().m_broke_lc)) {
			m_records.clear();
		}
		return;
	}

	AnimationData* animdata = &g_animationsystem.m_animated_players[player->index() - 1];
	if (!animdata)
		return;

	if (animdata->m_AnimationRecord.empty())
		return;

	AnimationRecord* animrecord = &animdata->m_AnimationRecord[0];
	if (!animrecord)
		return;

	// do deadtime bs.
	const auto& deadtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase() + 1) - 1.0f;
	while (m_records.size() >= 1) {
		auto& record = m_records.back();
		if (record.m_sim_time >= deadtime)
			break;

		m_records.pop_back();
	}

	if (m_records.size() > 0) {
		LagRecord* head = &m_records[0];

		// check if player changed simulation time since last time updated
		if (head->m_sim_time >= m_player->m_flSimulationTime())
			return; // don't add new entry for same or older time
	}

	// add new record and get reference to new record.
	LagRecord* newrecord = &m_records.emplace_front();

	// fill new record with relevent data.
	newrecord->m_player = player;
	newrecord->m_bullshit_index = player->index();
	newrecord->m_immune = player->m_fImmuneToGunGameDamageTime();
	newrecord->m_tick = g_csgo.m_cl->m_server_tick;
	newrecord->m_creation_time = g_csgo.m_globals->m_realtime;
	newrecord->m_lag = animrecord->m_lag;
	newrecord->m_sim_time = animrecord->m_sim_time;
	newrecord->m_flags = animrecord->m_flags;
	newrecord->m_origin = animrecord->m_origin;
	newrecord->m_velocity = animrecord->m_velocity;
	newrecord->m_mins = player->m_vecMins();
	newrecord->m_maxs = player->m_vecMaxs();
	newrecord->m_eye_angles = animrecord->m_eye_angles;
	newrecord->m_fake_angles = animrecord->m_fake_angles;
	newrecord->m_bullshit_fake_offset = animrecord->m_bullshit_fake_offset;
	newrecord->m_abs_ang = player->GetAbsAngles();
	newrecord->m_body = animrecord->m_body;
	newrecord->m_duck = animrecord->m_duck;
	newrecord->m_broke_lc = animrecord->m_broke_lc;
	newrecord->m_lagfixed = false;
	newrecord->m_resolver_mode = animrecord->m_resolver_mode;
	newrecord->m_anim_time = animrecord->m_anim_time;
	newrecord->m_resolver_lby_delta = animrecord->m_resolver_lby_delta;
	newrecord->m_interp_time = 0.0f;

	std::memcpy(newrecord->m_layers, animrecord->m_server_layers.data(), sizeof(C_AnimationLayer) * 13);
	player->GetPoseParameters(newrecord->m_poses);

	newrecord->m_bone_count = 0;
	if (animrecord->m_matrix) {
		std::memcpy(newrecord->m_matrix, animrecord->m_matrix, sizeof(matrix3x4_t) * animrecord->m_bone_count);
		newrecord->m_bone_count = animrecord->m_bone_count;
	}
}

void LagCompensation::Update() {
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player || player->m_bIsLocalPlayer())
			continue;

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		data->AddRecord(player);
	}
}

int LagCompensation::StartPrediction(AimPlayer* data) {
	if (data->m_records.size() < 1)
		return LAGCOMP_NOT_BROKEN;

	LagRecord* front = &data->m_records[0];
	if (!front)
		return LAGCOMP_NOT_BROKEN;

	if (!front->m_broke_lc)
		return LAGCOMP_NOT_BROKEN;

	// wait involved credit alpha
	
	//if (g_menu.main.aimbot.fake_lag_correction.get() == 1) {
	
		//g_notify.add(tfm::format("bruhhhh fakelag correction: %s) \n"), g_menu.main.aimbot.fake_lag_correction.get());
		// start at current server tick.
		const int nStartTick = g_csgo.m_cl->m_server_tick + 1;
		
		// try to predict when our shot will be registered.
		int nArrivalTick = nStartTick;

		// account for latency.
		const float flOutgoingLatency = g_csgo.m_net->GetLatency(INetChannel::FLOW_OUTGOING);
		int nServerLatencyTicks = game::TIME_TO_TICKS(flOutgoingLatency);
		int nClientLatencyTicks = game::TIME_TO_TICKS(g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING));
		nArrivalTick += nServerLatencyTicks;

		// get the delta in ticks between the last server net update
		// and the net update on which we created this record.
		int updatedelta = g_csgo.m_cl->m_server_tick - front->m_tick;

		if (!updatedelta) {
			return LAGCOMP_DONT_PREDICT;
		}

		const int receive_tick = std::abs(nArrivalTick - game::TIME_TO_TICKS(front->m_sim_time));
		int nChokedSafe = front->m_lag;
		if (nChokedSafe <= 0)
			nChokedSafe = 1;

		// too much lag to predict, exit and delay shot
		if (receive_tick / nChokedSafe > 19) {
			return LAGCOMP_WAIT;
		}

		const float adjusted_arrive_tick = game::TIME_TO_TICKS(flOutgoingLatency + g_csgo.m_globals->m_realtime - front->m_creation_time);

		// too much lag to predict, exit and delay shot
		if (adjusted_arrive_tick - front->m_lag >= 0) {
			return LAGCOMP_WAIT;
		}

		return LAGCOMP_DONT_PREDICT;
	//}

	int extrapolation_ticks = front->m_lag;
	for (int i = 0; i < std::min(int(data->m_records.size()), 3); i++) {
		LagRecord* cur = &data->m_records[i];
		if (!cur->m_broke_lc)
			break;

		if (cur->m_lag > extrapolation_ticks)
			extrapolation_ticks = cur->m_lag;
	}

	extrapolation_ticks = std::clamp(extrapolation_ticks, 1, 18);

	// we dont sendpacket until the tick after we shoot. remove +1 if i ever add delay fix.
	int local_latency = g_cl.m_latency_ticks + 1;
	int delta = g_cl.m_server_tick - front->m_tick;

	if (front->m_lagfixed) {
		return local_latency < ((extrapolation_ticks * 2) - delta) ? LAGCOMP_PREDICT : LAGCOMP_WAIT;
	}

	if (local_latency < extrapolation_ticks) {
		if (local_latency < extrapolation_ticks - delta) {
			return LAGCOMP_DONT_PREDICT;
		}

		return LAGCOMP_WAIT;
	}
	else {
		int next_update_recieve_tick = front->m_tick + extrapolation_ticks;
		if (local_latency < next_update_recieve_tick - g_cl.m_server_tick) {
			return LAGCOMP_WAIT;
		}
	//	if ((local_latency/0.5) < (next_update_recieve_tick - g_cl.m_server_tick)) {
		//	return LAGCOMP_PREDICT;
		//}

		bool on_ground = false;
		if (data->m_records.size() > 1) {
			on_ground = data->m_records[1].m_flags & FL_ONGROUND;
		}

		for (int i = 0; i < extrapolation_ticks; i++) {
			SimulateMovement(front->m_player, front->m_origin, front->m_velocity, front->m_flags, on_ground);
			front->m_sim_time += g_csgo.m_globals->m_interval;
		}
	}

	AnimationBackup_t backup;
	backup.m_origin = front->m_player->m_vecOrigin();
	backup.m_abs_origin = front->m_player->GetAbsOrigin();
	backup.m_velocity = front->m_player->m_vecVelocity();
	backup.m_abs_velocity = front->m_player->m_vecAbsVelocity();
	backup.m_flags = front->m_player->m_fFlags();
	backup.m_eflags = front->m_player->m_iEFlags();

	front->m_player->m_iEFlags() &= ~0x1000;

	front->m_player->m_vecOrigin() = front->m_origin;
	front->m_player->SetAbsOrigin(front->m_origin);
	front->m_player->m_vecVelocity() = front->m_velocity;
	front->m_player->m_vecAbsVelocity() = front->m_velocity;
	front->m_player->m_fFlags() = front->m_flags;

	front->m_player->SetAnimLayers(front->m_layers);

	front->m_player->SetupBones(front->m_matrix, 128, BONE_USED_BY_HITBOX, front->m_sim_time);
	front->m_bone_count = front->m_player->m_BoneCache().m_CachedBoneCount;

	front->m_player->m_vecOrigin() = backup.m_origin;
	front->m_player->SetAbsOrigin(backup.m_abs_origin);
	front->m_player->m_vecVelocity() = backup.m_velocity;
	front->m_player->m_vecAbsVelocity() = backup.m_abs_velocity;
	front->m_player->m_fFlags() = backup.m_flags;
	front->m_player->m_iEFlags() = backup.m_eflags;

	front->m_lagfixed = true;

	return LAGCOMP_PREDICT;
}

void LagCompensation::AirAccelerate(LagRecord* record, ang_t angle, float fmove, float smove) {
	vec3_t fwd, right, wishvel, wishdir;
	float  maxspeed, wishspd, wishspeed, currentspeed, addspeed, accelspeed;

	// determine movement angles.
	math::AngleVectors(angle, &fwd, &right);

	// zero out z components of movement vectors.
	fwd.z = 0.f;
	right.z = 0.f;

	// normalize remainder of vectors.
	fwd.normalize();
	right.normalize();

	// determine x and y parts of velocity.
	for (int i{}; i < 2; ++i)
		wishvel[i] = (fwd[i] * fmove) + (right[i] * smove);

	// zero out z part of velocity.
	wishvel.z = 0.f;

	// determine maginitude of speed of move.
	wishdir = wishvel;
	wishspeed = wishdir.normalize();

	// get maxspeed.
	// TODO; maybe global this or whatever its 260 anyway always.
	maxspeed = record->m_player->m_flMaxspeed();

	// clamp to server defined max speed.
	if (wishspeed != 0.f && wishspeed > maxspeed)
		wishspeed = maxspeed;

	// make copy to preserve original variable.
	wishspd = wishspeed;

	// cap speed.
	if (wishspd > 30.f)
		wishspd = 30.f;

	// determine veer amount.
	currentspeed = record->m_velocity.dot(wishdir);

	// see how much to add.
	addspeed = wishspd - currentspeed;

	// if not adding any, done.
	if (addspeed <= 0.f)
		return;

	// Determine acceleration speed after acceleration
	accelspeed = g_csgo.sv_airaccelerate->GetFloat() * wishspeed * g_csgo.m_globals->m_interval;

	// cap it.
	if (accelspeed > addspeed)
		accelspeed = addspeed;

	// add accel.
	record->m_velocity += (wishdir * accelspeed);
}

void LagCompensation::SimulateMovement(Player* pEntity, vec3_t& vecOrigin, vec3_t& vecVelocity, int& fFlags, bool bOnGround) {
	if (!(fFlags & FL_ONGROUND))
		vecVelocity.z -= game::TICKS_TO_TIME(g_csgo.sv_gravity->GetFloat());
	else if ((pEntity->m_fFlags() & FL_ONGROUND) && !bOnGround)
		vecVelocity.z = g_csgo.sv_jump_impulse->GetFloat();

	const auto src = vecOrigin;
	auto vecEnd = src + vecVelocity * g_csgo.m_globals->m_interval;

	Ray r(src, vecEnd, pEntity->m_vecMins(), pEntity->m_vecMaxs());

	CGameTrace t{ };
	CTraceFilter filter;
	filter.skip_entity = pEntity;

	g_csgo.m_engine_trace->TraceRay(r, CONTENTS_SOLID, &filter, &t);

	if (t.m_fraction != 1.f) {
		for (auto i = 0; i < 2; i++) {
			vecVelocity -= t.m_plane.m_normal * vecVelocity.dot(t.m_plane.m_normal);

			const auto flDot = vecVelocity.dot(t.m_plane.m_normal);
			if (flDot < 0.f)
				vecVelocity -= vec3_t(flDot * t.m_plane.m_normal.x,
					flDot * t.m_plane.m_normal.y, flDot * t.m_plane.m_normal.z);

			vecEnd = t.m_endpos + vecVelocity * game::TICKS_TO_TIME(1.f - t.m_fraction);

			r = Ray(t.m_endpos, vecEnd, pEntity->m_vecMins(), pEntity->m_vecMaxs());
			g_csgo.m_engine_trace->TraceRay(r, CONTENTS_SOLID, &filter, &t);

			if (t.m_fraction == 1.f)
				break;
		}
	}

	vecOrigin = vecEnd = t.m_endpos;
	vecEnd.z -= 2.f;

	r = Ray(vecOrigin, vecEnd, pEntity->m_vecMins(), pEntity->m_vecMaxs());
	g_csgo.m_engine_trace->TraceRay(r, CONTENTS_SOLID, &filter, &t);

	fFlags &= ~FL_ONGROUND;

	if (t.hit() && t.m_plane.m_normal.z > .7f)
		fFlags |= FL_ONGROUND;
}
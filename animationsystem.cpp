#include "includes.h"

AnimationSystem g_animationsystem{};;

void AnimationSystem::FrameStage() {
	if (!g_csgo.m_engine->IsInGame() || !g_csgo.m_engine->GetNetChannelInfo()) {
		return;
	}

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player || player->m_bIsLocalPlayer()) {
			continue;
		}

		AnimationData* cur_player = &m_animated_players[i - 1];

		// clear anim records.
		if (!player->alive()) {
			cur_player->m_AnimationRecord.clear();
			continue;
		}

		// collect data about the player.
		cur_player->Collect(player);

		// if they need an animation update.
		if (cur_player->m_anim_updated) {
			cur_player->Update();
			cur_player->m_anim_updated = false;
		}
	}
}

void AnimationData::CorrectVelocity(Player* m_player, AnimationRecord* m_record, AnimationRecord* m_previous, AnimationRecord* m_pre_previous) {
	m_record->m_velocity.clear();
	m_record->m_abs_velocity.clear();
	vec3_t vecAnimVelocity = m_player->m_vecVelocity();

	float MAXSPEEDLOL = 250.f;
	auto Weapondickpiss = m_player->GetActiveWeapon();
	if (Weapondickpiss && !Weapondickpiss->IsKnife()) {
		auto weapondatabruv = Weapondickpiss->GetWpnData();
		if (weapondatabruv) {
			MAXSPEEDLOL = m_player->m_bIsScoped() ? weapondatabruv->m_max_player_speed_alt : weapondatabruv->m_max_player_speed;
		}
	}

	if (!m_previous)
	{
		if (m_record->m_server_layers[6].m_playback_rate > 0.0f && vecAnimVelocity.length() > 0.f)
		{
			if (m_record->m_server_layers[6].m_weight > 0.0f) {
				auto v30 = MAXSPEEDLOL;

				if (m_record->m_flags & 6)
					v30 = v30 * 0.34f;
				else if (m_player->m_bIsWalking())
					v30 = v30 * 0.52f;

				auto v35 = m_record->m_server_layers[6].m_weight * v30;
				vecAnimVelocity *= v35 / vecAnimVelocity.length();
			}

			if (m_record->m_flags & 1)
				vecAnimVelocity.z = 0.f;
		}
		else {
			vecAnimVelocity.clear();
		}

		m_record->m_velocity = vecAnimVelocity;
		m_record->m_abs_velocity = vecAnimVelocity;
		return;
	}

	// check if player has been on ground for two consecutive ticks
	bool bIsOnground = m_record->m_flags & FL_ONGROUND && m_previous->m_flags & FL_ONGROUND;

	const vec3_t vecOriginDelta = m_record->m_origin - m_previous->m_origin;

	if (bIsOnground && m_record->m_server_layers[6].m_playback_rate <= 0.0f)
		vecAnimVelocity.clear();

	//
	// entity teleported, reset his velocity (thats what server does)
	// - L3D451R7
	//
	if ((m_player->m_fEffects() & 8) != 0
		|| m_player->m_ubEFNoInterpParity() != m_player->m_ubEFNoInterpParityOld())
	{
		m_record->m_velocity.clear();
		m_record->m_abs_velocity.clear();
		return;
	}

	if (m_record->m_lag <= 1) {
		m_record->m_velocity = vecAnimVelocity;
		m_record->m_abs_velocity = vecAnimVelocity;
		return;
	}

	// fix up velocity 
	auto origin_delta_lenght = vecOriginDelta.length();

	if (m_record->m_lag_time > 0.0f && m_record->m_lag_time < 1.0f && origin_delta_lenght >= 1.f && origin_delta_lenght <= 1000000.0f) {
		vecAnimVelocity = vecOriginDelta / m_record->m_lag_time;

		vecAnimVelocity.validate();

		m_record->m_velocity = vecAnimVelocity;
		// store estimated velocity as main velocity.

		if (!bIsOnground)
		{
			//
			// s/o estk for this correction :^)
			// -L3D451R7
			auto currently_ducking = m_record->m_flags & 2;
			if ((m_previous->m_flags & 2) != currently_ducking)
			{
				float duck_modifier;
				if (currently_ducking)
					duck_modifier = 9.f;
				else
					duck_modifier = -9.f;

				vecAnimVelocity.z += duck_modifier;
			}
		}
	}

	float anim_speed = 0.f;

	// determine if we can calculate animation speed modifier using server anim overlays
	if (bIsOnground
		&& m_record->m_server_layers[11].m_weight > 0.0f
		&& m_record->m_server_layers[11].m_weight < 1.0f
		&& m_record->m_server_layers[11].m_playback_rate == m_record->m_server_layers[11].m_playback_rate)
	{
		// calculate animation speed yielded by anim overlays
		auto flAnimModifier = 0.35f * (1.0f - m_record->m_server_layers[11].m_weight);
		if (flAnimModifier > 0.0f && flAnimModifier < 1.0f)
			anim_speed = MAXSPEEDLOL * (flAnimModifier + 0.55f);
	}

	// this velocity is valid ONLY IN ANIMFIX UPDATE TICK!!!
	// so don't store it to record as m_vecVelocity. i added vecAbsVelocity for that, it acts as a animationVelocity.
	// -L3D451R7
	if (anim_speed > 0.0f) {
		anim_speed /= vecAnimVelocity.length_2d();
		vecAnimVelocity.x *= anim_speed;
		vecAnimVelocity.y *= anim_speed;
	}

	// calculate average player direction when bunny hopping.
	if (m_pre_previous && vecAnimVelocity.length() >= 400.f) {
		auto vecPreviousVelocity = (m_previous->m_origin - m_pre_previous->m_origin) / m_previous->m_lag_time;

		// make sure to only calculate average player direction whenever they're bhopping
		if (vecPreviousVelocity.length() != 0.f && !bIsOnground) {
			auto vecCurrentDirection = math::NormalizedAngle(math::rad_to_deg(atan2(vecAnimVelocity.y, vecAnimVelocity.x)));
			auto vecPreviousDirection = math::NormalizedAngle(math::rad_to_deg(atan2(vecPreviousVelocity.y, vecPreviousVelocity.x)));

			auto vecAvgDirection = vecCurrentDirection - vecPreviousDirection;
			vecAvgDirection = math::deg_to_rad(math::NormalizedAngle(vecCurrentDirection + vecAvgDirection * 0.5f));

			auto vecDirectionCos = cos(vecAvgDirection);
			auto vecDirectionSin = sin(vecAvgDirection);

			// modify velocity accordingly
			vecAnimVelocity.x = vecDirectionCos * vecAnimVelocity.length_2d();
			vecAnimVelocity.y = vecDirectionSin * vecAnimVelocity.length_2d();
		}
	}

	if (!bIsOnground)
	{
		// correct z velocity.
		// https://github.com/perilouswithadollarsign/cstrike15_src/blob/f82112a2388b841d72cb62ca48ab1846dfcc11c8/game/shared/gamemovement.cpp#L1950
		if (!(m_record->m_flags & FL_ONGROUND))
			vecAnimVelocity.z -= g_csgo.sv_gravity->GetFloat() * m_record->m_lag_time * 0.5f;
		else
			vecAnimVelocity.z = 0.0f;
	}
	else
		vecAnimVelocity.z = 0.0f;

	// detect fakewalking players
	if (vecAnimVelocity.length() >= 0.1f) {
		/*if (m_record->m_server_layers[4].m_playback_rate == 0.0f
			&& m_record->m_server_layers[5].m_playback_rate == 0.0f
			&& m_record->m_server_layers[6].m_playback_rate == 0.0f
			&& m_record->m_flags & FL_ONGROUND)
		{
			vecAnimVelocity.clear();
			m_record->m_velocity.clear();
			m_record->m_fake_walk = true;
		}*/

		// better.
		if (m_record->m_server_layers[6].m_weight == 0.0f 
			&& m_record->m_server_layers[12].m_weight == 0.0f 
			&& m_record->m_server_layers[6].m_playback_rate < 0.0001f 
			&& (m_record->m_flags & FL_ONGROUND)) 
		{
			vecAnimVelocity.clear();
			m_record->m_velocity.clear();
			m_record->m_fake_walk = true;
		}
	}

	vecAnimVelocity.validate();

	// assign fixed velocity to record velocity
	m_record->m_abs_velocity = vecAnimVelocity;
}

void AnimationData::Collect(Player* pPlayer) {
	if (!pPlayer->alive())
		m_player = nullptr;

	// reset data
	if (m_player != pPlayer) {
		m_AnimationRecord.clear();
		m_sim_time = 0.0f;
		m_old_sim_time = 0.0f;
		m_was_dormant = false;
		m_resolver_was_dormant = false;
		m_alive = false;
		m_player = pPlayer;
	}

	if (!pPlayer)
		return;

	m_alive = true;
	m_old_sim_time = m_player->m_flOldSimulationTime();
	m_sim_time = m_player->m_flSimulationTime();

	if (m_sim_time == 0.0f || m_player->dormant()) {
		m_was_dormant = true;
		m_resolver_was_dormant = true;
		last_alive_loop_data.m_flCycle = 0;
		last_alive_loop_data.m_flPlaybackRate = 0;
		return;
	}

	// update ping data.
	// when ppl spawn in their ping wont have raised yet from fake ping.
	auto& ping_record = g_cl.m_ping_records[pPlayer->index() - 1];
	if (g_cl.m_resource) {
		float ping = (*g_cl.m_resource)->GetPlayerPing(pPlayer->index());
		if (ping < ping_record.lowest || ping_record.lowest < 1.f) {
			ping_record.lowest = ping;
		}
		ping_record.current = ping;
	}

	if (last_alive_loop_data.m_flCycle == m_player->m_AnimOverlay()[11].m_cycle
		&& last_alive_loop_data.m_flPlaybackRate == m_player->m_AnimOverlay()[11].m_playback_rate)
	{
		// fake update.
		//
		m_player->m_flSimulationTime() = m_old_sim_time; // s/o estk :^))))
		return;
	}

	last_alive_loop_data.m_flCycle = m_player->m_AnimOverlay()[11].m_cycle;
	last_alive_loop_data.m_flPlaybackRate = m_player->m_AnimOverlay()[11].m_playback_rate;

	//
	// no need for this check anymore boi
	// - L3D451R7
	/*if( m_flOldSimulationTime == m_flSimulationTime ) {
		return;
	}*/

	if (m_was_dormant) {
		m_AnimationRecord.clear();
	}

	m_anim_updated = true;
	m_was_dormant = false;

	int nTickRate = int(1.0f / g_csgo.m_globals->m_interval);

	while (m_AnimationRecord.size() > nTickRate) {
		m_AnimationRecord.pop_back();
	}

	auto pWeapon = (Weapon*)(m_player->m_hActiveWeapon().Get());

	AnimationRecord* pPreviousRecord = nullptr;
	AnimationRecord* pPenultimateRecord = nullptr;

	if (m_AnimationRecord.size() > 0) {
		pPreviousRecord = &m_AnimationRecord.front();

		if (m_AnimationRecord.size() > 1) {
			pPenultimateRecord = &m_AnimationRecord.at(1);
		}
	}

	CCSGOPlayerAnimState* animstate = pPlayer->m_PlayerAnimState();
	if (!animstate)
		return;

	// emplace new record
	auto pNewAnimRecord = &m_AnimationRecord.emplace_front();

	// fill up this record with all basic information
	m_player->GetAnimLayers(pNewAnimRecord->m_server_layers.data());
	m_player->GetPoseParameters(pNewAnimRecord->m_poses.data());
	std::memcpy(&pNewAnimRecord->animstate, animstate, sizeof(CCSGOPlayerAnimState));

	pNewAnimRecord->m_body = m_player->m_flLowerBodyYawTarget();
	pNewAnimRecord->m_eye_angles = m_player->m_angEyeAngles();
	pNewAnimRecord->m_fake_angles = pNewAnimRecord->m_eye_angles;
	pNewAnimRecord->m_bullshit_fake_offset = 0.f;
	pNewAnimRecord->m_duck = m_player->m_flDuckAmount();
	pNewAnimRecord->m_sim_time = m_sim_time;
	pNewAnimRecord->m_anim_time = m_old_sim_time + g_csgo.m_globals->m_interval;
	pNewAnimRecord->m_origin = m_player->m_vecOrigin();
	pNewAnimRecord->m_flags = m_player->m_fFlags();
	pNewAnimRecord->m_resolver_mode = Resolver::Modes::RESOLVE_NONE;
	pNewAnimRecord->m_fake_walk = false;
	pNewAnimRecord->m_broke_lc = false;
	pNewAnimRecord->m_entity = m_player;
	pNewAnimRecord->m_resolver_lby_delta = 0.f;

	// calculate choke time and choke ticks, compensating
	// for players coming out of dormancy / newly generated records
	pNewAnimRecord->m_lag_time = m_sim_time - m_old_sim_time;
	pNewAnimRecord->m_lag = game::TIME_TO_TICKS(pNewAnimRecord->m_lag_time);

	if (!pPreviousRecord || abs(pNewAnimRecord->m_lag) >= 32) {
		pNewAnimRecord->m_lag_time = g_csgo.m_globals->m_interval;
		pNewAnimRecord->m_lag = 1;
		pPreviousRecord = nullptr;
	}

	CorrectVelocity(pPlayer, pNewAnimRecord, pPreviousRecord, pPenultimateRecord);

	// we'll need information from the previous record in order to further
	// fix animations, skip everything and invalidate crucial data
	if (!pPreviousRecord) {
		pNewAnimRecord->m_broke_lc = false;
		pNewAnimRecord->m_velocity.clear();

		// we're done here
		return;
	}

	// detect players breaking teleport distance
	// https://github.com/perilouswithadollarsign/cstrike15_src/blob/master/game/server/player_lagcompensation.cpp#L384-L388
	if ((pNewAnimRecord->m_origin - pPreviousRecord->m_origin).length_2d_sqr() > 4096.f) {
		pNewAnimRecord->m_broke_lc = true;
	}

	// NITRO CODE NITRO CODE NITRO CODE

	// set previous flags.
	if (pPreviousRecord && pNewAnimRecord->m_lag > 2) {
		m_player->m_fFlags() = pPreviousRecord->m_flags;

		// strip the on ground flag.
		m_player->m_fFlags() &= ~FL_ONGROUND;

		// been onground for 2 consecutive ticks? fuck yeah.
		if (pNewAnimRecord->m_flags & FL_ONGROUND && pPreviousRecord->m_flags & FL_ONGROUND) {
			m_player->m_fFlags() |= FL_ONGROUND;
		}
		else {
			// fix jump_fall.
			if (pNewAnimRecord->m_server_layers[4].m_weight != 1.f && pPreviousRecord->m_server_layers[4].m_weight == 1.f && pNewAnimRecord->m_server_layers[5].m_weight != 0.f) {
				m_player->m_fFlags() |= FL_ONGROUND;
			}

			if (pNewAnimRecord->m_flags & FL_ONGROUND && !(pPreviousRecord->m_flags & FL_ONGROUND)) {
				m_player->m_fFlags() &= ~FL_ONGROUND;
			}
		}
	}
}

void AnimationData::Update() {
	if (!m_player || m_AnimationRecord.empty())
		return;

	CCSGOPlayerAnimState* animstate = m_player->m_PlayerAnimState();
	if (!animstate)
		return;

	AnimationRecord* newrecord = &m_AnimationRecord[0];
	if (!newrecord)
		return;

	AnimationRecord* prevrecord = nullptr;
	if (m_AnimationRecord.size() > 1) {
		prevrecord = &m_AnimationRecord[1];
	}

	const float curtimebackup = g_csgo.m_globals->m_curtime;
	const float frametimebackup = g_csgo.m_globals->m_frametime;

	AnimationBackup_t backup;
	backup.m_origin = m_player->m_vecOrigin();
	backup.m_abs_origin = m_player->GetAbsOrigin();
	backup.m_velocity = m_player->m_vecVelocity();
	backup.m_abs_velocity = m_player->m_vecAbsVelocity();
	backup.m_flags = m_player->m_fFlags();
	backup.m_eflags = m_player->m_iEFlags();
	backup.m_duck = m_player->m_flDuckAmount();
	backup.m_body = m_player->m_flLowerBodyYawTarget();

	if (newrecord->m_lag > 18) {
		g_csgo.m_globals->m_curtime = newrecord->m_sim_time;
	}
	else {
		g_csgo.m_globals->m_curtime = newrecord->m_anim_time;
	}

	// update with ticks instead of frames like server does.
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// force velocity.
	m_player->m_iEFlags() &= ~0x1000;

	// apply record data.
	m_player->m_vecVelocity() = newrecord->m_velocity;
	m_player->SetAbsVelocity(newrecord->m_abs_velocity);
	m_player->SetAbsOrigin(newrecord->m_origin);
	m_player->m_fFlags() = newrecord->m_flags;
	m_player->m_flDuckAmount() = newrecord->m_duck;
	m_player->m_flLowerBodyYawTarget() = newrecord->m_body;

	if (m_player->enemy(g_cl.m_local)) {
		g_resolver.ResolvePlayer(newrecord, m_resolver_was_dormant);
		newrecord->m_eye_angles.z = 0.0f;
	}

	if (newrecord->m_lag > 1 && prevrecord) {
		// apply resolved angle.
		m_player->m_angEyeAngles() = newrecord->m_eye_angles;
		m_player->m_flDuckAmount() = math::Lerp((newrecord->m_anim_time - newrecord->m_sim_time) / (prevrecord->m_sim_time - newrecord->m_sim_time), prevrecord->m_duck, newrecord->m_duck);

		// 1 tick of paler movement or some shit idfk.
		// 1st choked command gets animated but we recieve on sendpacket so we need 1 tick ahead.
		//g_lagcomp.SimulateMovement(m_player, newrecord->m_origin, newrecord->m_velocity, newrecord->m_flags, prevrecord->m_flags & FL_ONGROUND);
	}

	if (animstate->m_last_update_frame >= g_csgo.m_globals->m_frame)
		animstate->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	m_player->m_bClientSideAnimation() = true;
	m_player->UpdateClientSideAnimation();
	m_player->m_bClientSideAnimation() = false;

	m_player->SetAnimLayers(newrecord->m_server_layers.data());
	m_player->InvalidatePhysicsRecursive(ANGLES_CHANGED | ANIMATION_CHANGED | SEQUENCE_CHANGED);

	m_player->InvalidateBoneCache();
	m_player->SetupBones(newrecord->m_matrix, 128, BONE_USED_BY_HITBOX, newrecord->m_sim_time);
	newrecord->m_bone_count = m_player->m_BoneCache().m_CachedBoneCount;

	m_player->m_vecOrigin() = backup.m_origin;
	m_player->m_vecVelocity() = backup.m_velocity;
	m_player->m_vecAbsVelocity() = backup.m_abs_velocity;
	m_player->m_fFlags() = backup.m_flags;
	m_player->m_iEFlags() = backup.m_eflags;
	m_player->m_flDuckAmount() = backup.m_duck;
	m_player->SetAbsOrigin(backup.m_abs_origin);
	m_player->m_flLowerBodyYawTarget() = backup.m_body;

	g_csgo.m_globals->m_curtime = curtimebackup;
	g_csgo.m_globals->m_frametime = frametimebackup;
}
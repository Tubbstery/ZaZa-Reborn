#include "includes.h"

local_animations g_localanimations;

void local_animations::GhettoUpdateClientSideAnimationsEx() {
	if (!g_hooks.m_UpdateClientSideAnimation) {
		return;
	}

	const auto clientsideanimationBackup = g_cl.m_local->m_bClientSideAnimation();

	g_cl.m_local->m_bClientSideAnimation() = true; // disable CGlobalVarsBase::curtime interpolation
	g_cl.m_fuck_hltv = true; // disable velocity and duck amount interpolation.
	g_hooks.m_UpdateClientSideAnimation(g_cl.m_local); // update anims.

	g_cl.m_local->m_bClientSideAnimation() = clientsideanimationBackup;
	g_cl.m_fuck_hltv = false;
}

void local_animations::UpdateInformation() {
	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	if (g_cl.m_lag > 0)
		return;

	g_cl.m_anim_frame = g_csgo.m_globals->m_curtime - g_cl.m_anim_time;
	g_cl.m_anim_time = g_csgo.m_globals->m_curtime;

	g_cl.m_angle = g_cl.m_cmd->m_view_angles;
	if (g_cl.m_hide_angles) {
		// fix goal feet yaw so our moving desync shit works lol.
		state->m_abs_yaw = g_cl.m_angle.y;
		g_cl.m_angle = g_cl.m_hidden_angles;
	}
	math::clamp(g_cl.m_angle.x, -90.f, 90.f);
	g_cl.m_angle.normalize();

	g_csgo.m_prediction->SetLocalViewAngles(g_cl.m_angle);

	if (state->m_last_update_frame >= g_csgo.m_globals->m_frame)
		state->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	HandleAnimationEvents(g_cl.m_local, predict_animation_state(g_cl.m_local).first, g_cl.m_local->m_AnimOverlay(), g_cl.m_cmd);

	GhettoUpdateClientSideAnimationsEx();

	g_cl.m_local->SetupBones(nullptr, 128, BONE_USED_BY_ANYTHING, g_cl.m_local->m_flSimulationTime());

	std::memcpy(g_cl.m_vecbonepos, g_cl.m_local->m_vecBonePos(), g_cl.m_local->m_BoneCache().m_CachedBoneCount * sizeof(vec3_t));
	std::memcpy(g_cl.m_quatbonerot, g_cl.m_local->m_quatBoneRot(), g_cl.m_local->m_BoneCache().m_CachedBoneCount * sizeof(quaternion_t));

	g_cl.m_abs_yaw = state->m_abs_yaw;

	if (state->m_velocity_length_xy > 0.1f) {
		g_cl.m_body_pred = g_cl.m_anim_time + 0.242f;
		g_cl.m_body_lol = g_cl.m_abs_yaw;
	}
	else if (g_cl.m_anim_time > g_cl.m_body_pred && abs(math::NormalizedAngle(g_cl.m_abs_yaw - state->m_eye_yaw)) > 35.f) {
		g_cl.m_body_pred = g_cl.m_anim_time + 1.1f;
		g_cl.m_body_lol = g_cl.m_abs_yaw;
	}
}

bool has_sequence_completed(const float delta, float cycle, float playback) {
	return cycle + playback * delta >= 1.f;
}

void set_layer_weight_rate(const float delta, const float previous, float weightdelta, float weight) {
	if (delta == 0.f)
		return;

	weightdelta = (weight - previous) / delta;
}

mstudioseqdesc_t* get_seq_desc(CStudioHdr* hdr, const int index) {
	if (!hdr->m_pVModel)
		return hdr->m_pStudioHdr->get_local_seqdesc(index);

	static auto get_seq = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 79 04 00 75 25 8B 45 08 8B 09 85 C0 78 08 3B 81 ? ? ? ? 7C 02 33 C0 69 C0 ? ? ? ? 03").as<mstudioseqdesc_t * (__thiscall*)(void*, int32_t)>();

	return get_seq(hdr, index);
}

void local_animations::HandleAnimationEvents(Player* pLocal, CCSGOPlayerAnimState& pred_state, C_AnimationLayer* layers, CUserCmd* cmd) {
	if (!pLocal || !cmd)
		return;

	if (!pLocal->alive())
		return;

	if (!pLocal->m_PlayerAnimState() || !g_csgo.m_engine->IsInGame() || !g_csgo.m_engine->IsConnected())
		return;

	auto pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon)
		return;

	const auto p = &pred_state;

	// build activity modifiers
	CUtlVector<uint16_t> uModifiers = build_activity_modifiers(pLocal);
	auto wpn = pLocal->GetActiveWeapon();
	const auto s = pLocal->m_PlayerAnimState();

	bool m_bJumping = false;
	bool in_idle = false;
	float adjust_weight = 0.0f;
	bool in_deploy_rate_limit = false;
	bool swing_left = false;
	bool m_bOnGround = false;

	// CCSGOPlayerAnimState::DoAnimationEvent
	if (wpn && wpn->m_iItemDefinitionIndex() == WEAPONTYPE_KNIFE && wpn->m_flNextPrimaryAttack() + .4f < g_csgo.m_globals->m_curtime)
		swing_left = true;

	// CCSGOPlayerAnimState::DoAnimationEvent
	if (cmd->m_buttons & IN_JUMP && !(pLocal->m_fFlags() & FL_ONGROUND) && !pLocal->GetGroundEntity())
	{
		m_bJumping = true;
		try_initiate_animation(pLocal, 4, ACT_CSGO_JUMP, uModifiers);
	}

	// CCSGOPlayerAnimState::SetupVelocity
	auto& layer3 = pLocal->m_AnimOverlay()[3];
	const auto update_time = fmaxf(0.f, p->m_last_update_time - s->m_last_update_time);
	const auto layer3_act = pLocal->GetSequenceActivity(layer3.m_sequence);

	if (layer3_act == 980 || layer3_act == 979)
	{
		if (in_idle && p->m_speed_as_portion_of_crouch_top_speed <= .25f)
		{
			IncrementLayerCycleWeightRateGeneric(s, &layer3, update_time);
			in_idle = !has_sequence_completed(update_time, layer3.m_cycle, layer3.m_playback_rate);
		}
		else
		{
			const auto weight = layer3.m_weight;
			layer3.m_weight = math::Approach(0.f, weight, update_time * 5.f);
			set_layer_weight_rate(update_time, weight, layer3.m_weight_delta_rate, layer3.m_weight);
			in_idle = false;
		}
	}

	if (p->m_velocity_length_xy <= 1.f && s->m_on_ground && !s->m_on_ladder && !s->m_landing && s->m_last_update_increment > 0
		&& std::abs(math::NormalizedAngle(s->m_abs_yaw - p->m_abs_yaw) / update_time > 120.f))
	{
		try_initiate_animation(pLocal, 3, ACT_CSGO_IDLE_TURN_BALANCEADJUST, uModifiers);
		in_idle = true;
	}

	vec3_t forward{}, right{}, up{};
	math::AngleVectors(ang_t(0.f, s->m_abs_yaw, 0.f), &forward, &right, &up);
	right.normalize_in_place();
	const auto to_forward_dot = s->m_velocity_normalized_non_zero.dot(forward);
	const auto to_right_dot = s->m_velocity_normalized_non_zero.dot(right);

	const auto move_right = (cmd->m_buttons & IN_MOVERIGHT) != 0;
	const auto move_left = (cmd->m_buttons & IN_MOVELEFT) != 0;
	const auto move_forward = (cmd->m_buttons & IN_FORWARD) != 0;
	const auto move_backwards = (cmd->m_buttons & IN_BACK) != 0;
	const auto strafe_forward = s->m_speed_as_portion_of_run_top_speed >= .65f && move_forward && !move_backwards && to_forward_dot < -.55f;
	const auto strafe_backwards = s->m_speed_as_portion_of_run_top_speed >= .65f && move_backwards && !move_forward && to_forward_dot > .55f;
	const auto strafe_right = s->m_speed_as_portion_of_run_top_speed >= .73f && move_right && !move_left && to_right_dot < -.63f;
	const auto strafe_left = s->m_speed_as_portion_of_run_top_speed >= .73f && move_left && !move_right && to_right_dot > .63f;

	pLocal->m_bStrafing() = strafe_forward || strafe_backwards || strafe_right || strafe_left;

	const auto swapped_ground = s->m_on_ground != p->m_on_ground || s->m_on_ladder != p->m_on_ladder;

	if (p->m_on_ground)
	{
		auto& layer5 = pLocal->m_AnimOverlay()[5];

		if (!s->m_landing && swapped_ground)
			try_initiate_animation(pLocal, 5, s->m_duration_in_air > 1.f ? 989 : 988, uModifiers);

		if (p->m_landing && pLocal->GetSequenceActivity(layer5.m_sequence) != 987)
			m_bJumping = false;

		if (!p->m_landing && !m_bJumping && p->m_ladder_speed <= 0.f)
			layer5.m_weight = 0.f;
	}
	else if (swapped_ground && !m_bJumping)
		try_initiate_animation(pLocal, 4, 986, uModifiers);

	// CCSGOPlayerAnimState::SetupAliveLoop
	auto& alive = pLocal->m_AnimOverlay()[11];
	if (pLocal->GetSequenceActivity(alive.m_sequence) == 981)
	{
		if (p->m_weapon && p->m_weapon != p->m_weapon_last)
		{
			const auto cycle = alive.m_cycle;
			try_initiate_animation(pLocal, 11, 981, uModifiers);
			alive.m_cycle = cycle;
		}
		else if (!has_sequence_completed(update_time, alive.m_cycle, alive.m_playback_rate))
			alive.m_weight = 1.f - std::clamp((p->m_speed_as_portion_of_walk_top_speed - .55f) / .35f, 0.f, 1.f);
	}

	const auto world_model = wpn ? reinterpret_cast<Weapon*>(g_csgo.m_entlist->GetClientEntityFromHandle(wpn->m_hWeaponWorldModel())) : nullptr;

	// CCSGOPlayerAnimState::SetUpWeaponAction
	auto increment = true;
	auto& action = pLocal->m_AnimOverlay()[1];
	if (wpn && in_deploy_rate_limit && pLocal->GetSequenceActivity(action.m_sequence) == 972)
	{
		if (world_model)
			world_model->m_fEffects() |= EF_NODRAW;

		if (action.m_cycle >= .15f)
		{
			in_deploy_rate_limit = false;
			try_initiate_animation(pLocal, 1, 972, uModifiers);
			increment = false;
		}
	}

	auto& recrouch = pLocal->m_AnimOverlay()[2];
	auto recrouch_weight = 0.f;

	if (action.m_weight > 0.f)
	{
		if (recrouch.m_sequence <= 0)
		{
			_(r, "recrouch_generic");
			const auto seq = pLocal->LookupSequence(r.c_str());
			recrouch.m_playback_rate = pLocal->GetLayerSequenceCycleRate(&recrouch, seq);
			recrouch.m_sequence = seq;
			recrouch.m_cycle = recrouch.m_weight = 0.f;
		}

		auto has_modifier = false;
		_(c, "crouch");
		const auto& seqdesc = get_seq_desc(pLocal->GetModelPtr(), action.m_sequence);
		for (auto i = 0; i < seqdesc->num_activity_modifiers; i++)
			if (!strcmp(seqdesc->pActivityModifier(i)->pszName(), c.c_str()))
			{
				has_modifier = true;
				break;
			}

		if (has_modifier)
		{
			if (p->m_anim_duck_amount < 1.f)
				recrouch_weight = action.m_weight * (1.f - p->m_anim_duck_amount);
		}
		else if (p->m_anim_duck_amount > 0.f)
			recrouch_weight = action.m_weight * p->m_anim_duck_amount;
	}
	else if (recrouch.m_weight > 0.f)
		recrouch_weight = math::Approach(0.f, recrouch.m_weight, 4.f * update_time);

	recrouch.m_weight = std::clamp(recrouch_weight, 0.f, 1.f);

	if (increment)
	{
		IncrementLayerCycle(s, &action, false, update_time);
		GetLayerIdealWeightFromSeqCycle(s, &action);
		set_layer_weight_rate(p->m_last_update_increment, action.m_weight, action.m_weight_delta_rate, action.m_weight);

		action.m_cycle = std::clamp(action.m_cycle - p->m_last_update_increment * action.m_playback_rate, 0.f, 1.f);
		action.m_weight = std::clamp(action.m_weight - p->m_last_update_increment * action.m_weight_delta_rate, .0000001f, 1.f);
	}

	vm_thing = 0;
}

CUtlVector<uint16_t> local_animations::build_activity_modifiers(Player* player) {
	activity_modifiers_wrapper modifier_wrapper{};

	const auto state = player->m_PlayerAnimState();

	modifier_wrapper.add_modifier(state->GetWeaponPrefix());

	if (state->m_speed_as_portion_of_run_top_speed > 0.25f)
		modifier_wrapper.add_modifier("moving");

	if (state->m_anim_duck_amount > 0.55000001f)
		modifier_wrapper.add_modifier("crouch");

	return modifier_wrapper.get();
}

std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> local_animations::predict_animation_state(Player* player) {
	const auto backup_state = *player->m_PlayerAnimState();
	const auto backup_layers = player->m_AnimOverlay();
	const auto backup_poses = player->m_flPoseParameter();

	if (player->m_PlayerAnimState()->m_last_update_frame >= g_csgo.m_globals->m_frame)
		player->m_PlayerAnimState()->m_last_update_frame = g_csgo.m_globals->m_frame - 1;

	//g_hooks.m_bUpdatingCSA[m_local->index()] = true;
	player->m_PlayerAnimState()->update(g_cl.m_angle);
	//g_hooks.m_bUpdatingCSA[m_local->index()] = false;
	const auto pred = *player->m_PlayerAnimState();
	const auto layers = player->m_AnimOverlay();

	*player->m_PlayerAnimState() = backup_state;
	player->SetAnimLayers(backup_layers);
	player->SetPoseParameters(backup_poses);

	return { pred, layers };
}

void local_animations::try_initiate_animation(Player* player, size_t layer, int32_t activity, CUtlVector<uint16_t> modifiers) {
	typedef void* (__thiscall* find_mapping_t)(void*);
	static const auto find_mapping = pattern::find(g_csgo.m_server_dll, "55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17").as<find_mapping_t>();

	typedef int32_t(__thiscall* select_weighted_sequence_from_modifiers_t)(void*, void*, int32_t, const void*, int32_t);
	static const auto select_weighted_sequence_from_modifiers = pattern::find(g_csgo.m_server_dll, "55 8B EC 83 EC 2C 53 56 8B 75 08 8B D9").as< select_weighted_sequence_from_modifiers_t>();

	const auto mapping = find_mapping(player->m_studioHdr());
	const auto sequence = select_weighted_sequence_from_modifiers(mapping, player->m_studioHdr(), activity, &modifiers[0], modifiers.Count());

	if (sequence < 2)
		return;

	auto& l = player->m_AnimOverlay()[layer];
	l.m_playback_rate = player->GetLayerSequenceCycleRate(&l, sequence);
	l.m_sequence = sequence;
	l.m_cycle = l.m_weight = 0.f;
}

void local_animations::IncrementLayerCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, bool bAllowLoop, const float delta) {
	if (!pLayer || !m_pAnimstate)
		return;

	if (!m_pAnimstate->m_player)
		return;

	if (fabs(pLayer->m_playback_rate) <= 0.f)
		return;

	auto cur_cycle = pLayer->m_cycle;
	cur_cycle += delta * pLayer->m_playback_rate;

	if (!bAllowLoop && cur_cycle >= 1.f)
		cur_cycle = .999f;

	cur_cycle -= int(cur_cycle);

	if (cur_cycle < 0.f)
		cur_cycle += 1.f;
	else if (cur_cycle > 1.f)
		cur_cycle -= 1.f;

	pLayer->m_cycle = cur_cycle;
}

void local_animations::IncrementLayerWeight(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (!pLayer)
		return;

	if (abs(pLayer->m_weight_delta_rate) <= 0.f)
		return;

	float flCurrentWeight = pLayer->m_weight;
	flCurrentWeight += m_pAnimstate->m_last_update_increment * pLayer->m_weight_delta_rate;
	flCurrentWeight = std::clamp(flCurrentWeight, 0.f, 1.f);

	if (pLayer->m_weight != flCurrentWeight) {
		pLayer->m_weight = flCurrentWeight;
	}
}

float local_animations::GetLayerIdealWeightFromSeqCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (!pLayer)
		return 0.f;

	float flCycle = pLayer->m_cycle;
	if (flCycle >= 0.999f)
		flCycle = 1;

	float flEaseIn = pLayer->m_blend_in; // seqdesc.fadeintime;
	float flEaseOut = pLayer->m_blend_in; // seqdesc.fadeouttime;
	float flIdealWeight = 1;

	if (flEaseIn > 0 && flCycle < flEaseIn)
	{
		flIdealWeight = math::SmoothStepBounds(0, flEaseIn, flCycle);
	}
	else if (flEaseOut < 1 && flCycle > flEaseOut)
	{
		flIdealWeight = math::SmoothStepBounds(1.0f, flEaseOut, flCycle);
	}

	if (flIdealWeight < 0.0015f)
		return 0.f;

	return (std::clamp(flIdealWeight, 0.f, 1.f));
}

void local_animations::IncrementLayerCycleWeightRateGeneric(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, const float delta) {
	float flWeightPrevious = pLayer->m_weight;
	IncrementLayerCycle(m_pAnimstate, pLayer, false, delta);
	pLayer->m_weight = GetLayerIdealWeightFromSeqCycle(m_pAnimstate, pLayer);
	pLayer->m_weight_delta_rate = flWeightPrevious;
}
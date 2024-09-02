#include "includes.h"

Resolver g_resolver{};;

void Resolver::FixOnshot(AnimationRecord* record, AimPlayer* data) {
	Weapon* weapon = record->m_entity->GetActiveWeapon();
	if (!weapon)
		return;

	WeaponInfo* wpn_data = weapon->GetWpnData();
	if (!wpn_data)
		return;

	if ((wpn_data->m_weapon_type != WEAPONTYPE_GRENADE
		&& wpn_data->m_weapon_type > WEAPONTYPE_MACHINEGUN)
		|| wpn_data->m_weapon_type <= WEAPONTYPE_KNIFE)
		return;

	int shot_tick = game::TIME_TO_TICKS(weapon->m_fLastShotTime());

	// they shot.
	if (data->m_prev_shot_tick != shot_tick) {
		int anim_tick = game::TIME_TO_TICKS(record->m_anim_time);
		// shot cmd never got animated.
		if (shot_tick != anim_tick) {
			// lagrecords are always at least one update behind.
			for (int i = 0; i < data->m_records.size(); i++) {
				LagRecord* cur = &data->m_records[i];
				if (!cur)
					continue;

				if (abs(math::NormalizedAngle(record->m_eye_angles.x - cur->m_eye_angles.x)) > 10.f) {
					record->m_eye_angles.x = cur->m_eye_angles.x;
					break;
				}
			}
		}

		data->m_prev_shot_tick = shot_tick;
	}
}

void Resolver::OnBodyUpdate(Player* player, float value) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
	if (!data)
		return;

	// set data.
	data->m_old_body_proxy = data->m_body_proxy;
	data->m_body_proxy = value;
}

float Resolver::AntiFreestand(AnimationRecord* record, std::vector<AdaptiveAngle> alternate_angles, bool use_alternate_angles) {
	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// get the away angle for this record.
	float away = GetAwayAngle(record);

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	if (use_alternate_angles && !alternate_angles.empty()) {
		angles = alternate_angles;
	}
	else {
		angles.emplace_back(away + 90.f);
		angles.emplace_back(away - 90.f);
		angles.emplace_back(away + 180.f);
	}

	// start the trace at the enemy shoot pos.
	vec3_t start = g_cl.m_shoot_pos;

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// get the enemies shoot pos.
	vec3_t shoot_pos = record->m_entity->GetShootPosition();

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ shoot_pos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			shoot_pos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.5f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		return away + 180.f;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();
	return best->m_yaw;
}

bool Resolver::IsYawSideways(Player* entity, float yaw) {
	const float delta = fabs(math::NormalizedAngle((math::CalcAngle(g_cl.m_local->m_vecOrigin(),entity->m_vecOrigin()).y) - yaw));
	return delta >= 40.f && delta < 150.f;
}

float Resolver::GetAwayAngle(AnimationRecord* record) {
	ang_t  away;
	math::VectorAngles(g_cl.m_shoot_pos - record->m_origin, away);
	return away.y;
}

float Resolver::SnapToNearestYaw(float yaw, std::vector<float> options) {
	float best_delta = -1.f; // difference cant be > 180 lol
	float best_yaw = yaw;

	for (int i = 0; i < options.size(); i++) {
		auto delta = abs(math::NormalizedAngle(options[i] - yaw));
		if (delta < best_delta || best_delta == -1.f) {
			best_yaw = options[i];
			best_delta = delta;
		}
	}

	return best_yaw;
}

void Resolver::DetectFake(AnimationRecord* record, AimPlayer* data) {
	static const int detection_range = 8;

	data->m_fake_type = FAKE_UNKNOWN;

	// dont even have enough data.
	if (data->m_records.size() < detection_range) {
		return;
	}

	// these are actually lagrecords, so delayed by 1.
	bool change = abs(math::NormalizedAngle(record->m_fake_angles.y - data->m_records[0].m_fake_angles.y)) > 0.f 
		|| abs(math::NormalizedAngle(data->m_records[0].m_fake_angles.y - data->m_records[1].m_fake_angles.y)) > 0.f;

	if (change) {
		/*data->m_fake_type = FAKE_JITTER;

		for (int i = 0; i < detection_range - 1; i++) {
			if (abs(math::NormalizedAngle(data->m_records[i].m_fake_angles.y - data->m_records[i + 1].m_fake_angles.y)) > 90.f + 17.5f) {
				data->m_fake_type = FAKE_RANDOM;
			}
		}*/
		data->m_fake_type = FAKE_RANDOM;
	}
	else {
		data->m_fake_type = FAKE_STATIC;
	}
}

// fake yaw resolving is usually really good, except when retards (edmond dingusmeat) have retarded fake yaws that completely break the resolver.
// this should help to circumvent that.
void Resolver::ValidateAngle(AnimationRecord* record, AimPlayer* data) {
	data->m_unlikely_resolve = 0;
	record->m_eye_angles.y = data->m_resolved_yaw;

	if (data->m_records.empty()) {
		return;
	}

	if (record->m_resolver_mode > 4 || data->m_fake_type == FAKE_RANDOM || data->m_fake_type == FAKE_UNKNOWN) {
		record->m_eye_angles.y = data->m_resolved_yaw;

		CCSGOPlayerAnimState* animstate = record->m_entity->m_PlayerAnimState();
		if (!animstate)
			return;

		bool client979 = abs(math::NormalizedAngle(animstate->m_abs_yaw - animstate->m_abs_yaw_last) / animstate->m_last_update_increment > 120.f);

		if (client979) {
			if (!record->m_server_layers[3].m_cycle) {
				data->m_unlikely_resolve = 2;
			}
		}
		else {
			// delayed by 1 tick, animstate is not updated yet.
			if (data->m_records[0].m_layers[3].m_cycle) {
				data->m_unlikely_resolve = 1;
			}
		}

		return;
	}

	if (data->m_fake_type == FAKE_STATIC && data->m_records[0].m_resolver_mode <= 4) {
		/*float previous = data->m_records[0].m_eye_angles.y;
		float cur = record->m_eye_angles.y;

		float previous_fake = data->m_records[0].m_fake_angles.y;
		float cur_fake = record->m_fake_angles.y;

		// angle is probably shit.
		if (abs(math::NormalizedAngle((cur - previous) - (cur_fake - previous_fake))) > 17.5f) {
			record->m_eye_angles.y = previous;
			g_cl.print("rejected resolved yaw: static delta\n");
			data->m_unlikely_resolve = 1;
			return;
		}*/
	}
	else {

	}
}

void Resolver::LogShot(LagRecord* record, bool miss) {
	// crash.
	if (!record) {
		return;
	}

	// WHAT THE FUCK.
	if (record->m_bullshit_index < 0 || record->m_bullshit_index > 64) {
		return;
	}		

	AimPlayer* data = &g_aimbot.m_players[record->m_bullshit_index - 1];
	if (!data) {
		return;
	}

	// lol dont log air shit.
	if (!(record->m_flags & FL_ONGROUND)) {
		return;
	}

	// bruh.
	if (record->m_resolver_mode == RESOLVE_NONE || record->m_resolver_mode == RESOLVE_WALK || record->m_resolver_mode == RESOLVE_STAND_STOPPED_MOVING) {
		return;
	}

	// makes sense to lby misses if the delta is low.
	if (record->m_resolver_mode == RESOLVE_BODY) {
		if (abs(math::NormalizedAngle(record->m_resolver_lby_delta)) > 17.5f) {
			return;
		}
	}

	if (miss) {
		// increment missed shots counter.
		++data->m_shot_logs.m_missed_shots;

		// push this twice to make it double as significant lol.
		if (record->m_resolver_mode == RESOLVE_STAND_LBY_DELTA_FIRST_TICK) {
			data->m_shot_logs.m_missed_lby_deltas.push_back(record->m_resolver_lby_delta);
			data->m_shot_logs.m_missed_lby_deltas.push_back(record->m_resolver_lby_delta);
			return;
		}

		// just add once.
		data->m_shot_logs.m_missed_lby_deltas.push_back(record->m_resolver_lby_delta);
	}
	else {
		// push this twice to make it double as significant lol.
		if (record->m_resolver_mode == RESOLVE_STAND_LBY_DELTA_FIRST_TICK) {
			data->m_shot_logs.m_hit_lby_deltas.push_back(record->m_resolver_lby_delta);
			data->m_shot_logs.m_hit_lby_deltas.push_back(record->m_resolver_lby_delta);
			return;
		}

		// just add once.
		data->m_shot_logs.m_hit_lby_deltas.push_back(record->m_resolver_lby_delta);
	}
}

float Resolver::GetDeltaChance(AimPlayer* data, float delta) {
	int delta_hits = 0;
	int delta_misses = 0;

	for (int j = 0; j < data->m_shot_logs.m_hit_lby_deltas.size(); j++) {
		if (abs(math::NormalizedAngle(data->m_shot_logs.m_hit_lby_deltas[j] - delta)) <= 17.5f) {
			++delta_hits;
		}
	}

	for (int j = 0; j < data->m_shot_logs.m_missed_lby_deltas.size(); j++) {
		if (abs(math::NormalizedAngle(data->m_shot_logs.m_missed_lby_deltas[j] - delta)) <= 17.5f) {
			++delta_misses;
		}
	}

	int attempts = delta_hits + delta_misses;
	if (!attempts)
		return -1.f;

	return static_cast<float>(delta_hits) / attempts * 100.f;
}

float Resolver::GetMostAccurateLBYDelta(AimPlayer* data) {
	float highest_hitchance = -1.f;
	float new_lby_delta_from_hits = data->m_og_lby_delta;

	// get most accurate delta.
	for (int i = 0; i < data->m_shot_logs.m_hit_lby_deltas.size(); i++) {
		float cur_delta = data->m_shot_logs.m_hit_lby_deltas[i];
		float hitchance = GetDeltaChance(data, cur_delta);

		if (hitchance > highest_hitchance) {
			new_lby_delta_from_hits = cur_delta;
			highest_hitchance = hitchance;
		}
	}

	return new_lby_delta_from_hits;
}

void Resolver::ResolvePlayer(AnimationRecord* record, bool& was_dormant) {
	if (game::IsFakePlayer(record->m_entity->index()))
		return;

	AimPlayer* data = &g_aimbot.m_players[record->m_entity->index() - 1];
	if (!data)
		return;

	FixOnshot(record, data);
	DetectFake(record, data);

	if (was_dormant) {
		data->m_calc_lby = true;

		if ((record->m_origin - data->resolver_last_origin).length() > 0.1f || !(data->resolver_last_origin.x && data->resolver_last_origin.y && data->resolver_last_origin.z)) {
			data->m_cant_hit_lby = true;
		}
		else {
			if (record->m_anim_time > data->m_lby_timer + g_csgo.m_globals->m_interval) {
				data->m_lby_timer = record->m_anim_time + (1.1f - fmod(record->m_anim_time - data->m_lby_timer, 1.1f));
			}
		}
	}

	if (record->m_flags & FL_ONGROUND) {
		// dormant check makes sure we dont call this and disable da calc lby stuff.
		if (record->m_velocity.length_2d() > 0.1f && !record->m_fake_walk && !was_dormant) {
			ResolveWalk(record, data);
		}
		else {
			ResolveStand(record, data, was_dormant);
		}
		data->m_last_ground_yaw = record->m_eye_angles.y;
	}
	else {
		ResolveAir(record, data);
	}

	// validate the angle change with animlayers and set to eyeangles if appropriate.
	ValidateAngle(record, data);

	if (g_input.GetKeyState(g_menu.main.aimbot.override_resolver.get()) && record->m_entity->index() == g_aimbot.m_resolver_override_index) {
		if (record->m_resolver_mode == RESOLVE_WALK) {
			if (g_menu.main.aimbot.override_resolver_moving_feetyaw.get()) {
				CCSGOPlayerAnimState* animstate = record->m_entity->m_PlayerAnimState();
				if (animstate) {
					animstate->m_abs_yaw = g_aimbot.m_resolver_override_side > 0 ? record->m_eye_angles.y + 90 : record->m_eye_angles.y - 90;
				}
			}
		}
		else if (record->m_resolver_mode != RESOLVE_BODY) {
			float awayangle = GetAwayAngle(record);

			if (g_aimbot.m_resolver_override_side > 0) {
				record->m_eye_angles.y = awayangle - 90;
			}
			else {
				record->m_eye_angles.y = awayangle + 90;
			}
		}
	}

	data->resolver_last_origin = record->m_origin;
	was_dormant = false;
}

void Resolver::ResolveStand(AnimationRecord* record, AimPlayer* data, bool fuckniggadormant) {
	float freestand_yaw = AntiFreestand(record);
	float most_accurate = abs(GetMostAccurateLBYDelta(data));

	if ((record->m_anim_time > data->m_lby_timer && !data->m_cant_hit_lby) || (data->m_body_proxy != data->m_old_body_proxy && !fuckniggadormant)) {
		data->m_resolved_yaw = record->m_body;
		record->m_resolver_mode = RESOLVE_BODY;
		data->m_lby_timer = record->m_anim_time + 1.1f;

		if (!data->m_calc_lby) {
			data->m_lby_delta = data->m_og_lby_delta = data->m_body_proxy - data->m_old_body_proxy;
			data->m_calc_lby = true;
			data->m_allow_freestand_lby_delta = false;

			if (abs(math::NormalizedAngle(data->m_lby_delta)) <= 35.f) {
				data->m_lby_delta = data->m_og_lby_delta = 0.f;
			}

			data->m_shot_logs.m_missed_shots = data->m_prev_missed_shots_lol = 0;
		}
		else {
			if (data->m_body_proxy != data->m_old_body_proxy) {
				++data->m_times_lby_changed_since_first_break;
				data->m_allow_freestand_lby_delta = true;
			}

			if (data->m_allow_freestand_lby_delta && !data->m_shot_logs.m_missed_shots) {
				if (abs(math::NormalizedAngle(data->m_move_lby - (record->m_body - data->m_lby_delta))) >
					abs(math::NormalizedAngle(data->m_move_lby - (record->m_body + data->m_lby_delta))))
				{
					data->m_lby_delta = -data->m_lby_delta;
				}
			}
		}

		data->m_updates_since_lby_update_tick = 0;
		++data->m_lby_updates_since_moving;
		data->m_cant_hit_lby = false;
	}

	// lol fix against retarded ppl with shit lby breakers or no quickstop.
	if (record->m_sim_time - data->m_move_sim_time < 1.f && !data->m_cant_hit_lby && !fuckniggadormant) {
		if (data->m_body_proxy != data->m_old_body_proxy && data->m_calc_lby && data->m_prev_calc_lby) {
			data->m_delayed_lby_delta = data->m_body_proxy - data->m_old_body_proxy;

			if (abs(math::NormalizedAngle(most_accurate - data->m_delayed_lby_delta)) <= 17.5f) {
				data->m_lby_delta = data->m_delayed_lby_delta;
				data->m_recalculate_fake_offsets = true;
			}
		}
	}
	
	while (true) {
		int missedshotsthing = data->m_shot_logs.m_missed_shots * (data->m_shot_logs.m_missed_shots - data->m_prev_missed_shots_lol);
		if (!missedshotsthing) {
			break;
		}

		if (missedshotsthing == 1) {
			data->m_brute_angles.clear();

			if (abs(math::NormalizedAngle(data->m_lby_delta - data->m_delayed_lby_delta)) > 17.5f) {
				data->m_brute_angles.push_back(data->m_delayed_lby_delta);
			}

			if (abs(math::NormalizedAngle(data->m_lby_delta)) > 17.5f) {
				data->m_brute_angles.push_back(0.f);
			}

			if (abs(math::NormalizedAngle(data->m_lby_delta - data->m_og_lby_delta)) > 17.5f) {
				data->m_brute_angles.push_back(data->m_og_lby_delta);
			}

			data->m_brute_angles.push_back(110.f);
			data->m_brute_angles.push_back(90.f);
			data->m_brute_angles.push_back(30.f);
			data->m_brute_angles.push_back(0.f);
			data->m_brute_angles.push_back(180.f);

			std::sort(data->m_brute_angles.begin(), data->m_brute_angles.end(), [&](float a, float b) -> bool {
				return GetDeltaChance(data, a) > GetDeltaChance(data, b);
			});
		}

		bool prev = data->m_lby_delta;

		// just fkn antifreestand them lol.
		if ((missedshotsthing - 1) / 2 >= data->m_brute_angles.size()) {
			data->m_lby_delta = record->m_body - freestand_yaw;
			break;
		}

		if (missedshotsthing % 2 == 1) {
			data->m_lby_delta = -data->m_lby_delta;
		}
		else {
			float angle = data->m_brute_angles[(missedshotsthing - 1) / 2];

			if (abs(math::NormalizedAngle(freestand_yaw - (record->m_body - angle))) >
				abs(math::NormalizedAngle(freestand_yaw - (record->m_body + angle))))
			{
				data->m_lby_delta = -angle;
			}
			else {
				data->m_lby_delta = angle;
			}
		}

		if (math::NormalizedAngle(data->m_lby_delta - prev) <= 17.5f) {
			data->m_prev_missed_shots_lol = data->m_shot_logs.m_missed_shots;
			data->m_shot_logs.m_missed_shots++;
			continue;
		}

		break;
	}

	// bruh.
	data->m_prev_missed_shots_lol = data->m_shot_logs.m_missed_shots;

	AnimationData* animdata = &g_animationsystem.m_animated_players[record->m_entity->index() - 1];
	if (animdata && animdata->m_AnimationRecord.size() > 1) {
		AnimationRecord* previous = &animdata->m_AnimationRecord[1];
		if (previous) {
			if (data->m_calc_lby 
				&& data->m_prev_calc_lby 
				&& record->m_entity->GetSequenceActivity(record->m_server_layers[3].m_sequence) == 979 
				&& record->m_server_layers[3].m_cycle 
				&& !previous->m_server_layers[3].m_cycle
				&& !data->m_cant_hit_lby) 
			{
				//g_cl.print(std::to_string(data->m_updates_since_lby_update_tick) + "\n");

				if (data->m_updates_since_lby_update_tick == 0) {
					data->m_lby_delta = -abs(data->m_lby_delta);
					data->m_allow_freestand_lby_delta = false;
				}

				if (data->m_updates_since_lby_update_tick == 1) {
					// we dont want to shoot this record, since we now know the delta was wrong.
					if (math::NormalizedAngle(data->m_lby_delta) < 0.f) {
						// SHOULD BE IDX 0 BECAUSE THIS ANIMRECORD HASNT BEEN ADDED TO LAGRECORDS AT THE TIME OF RUNNING RESOLVER.
						data->m_records[0].m_immune = 1.f;

						// goal feet yaw gets fked when this happens, so manually fix it.
						CCSGOPlayerAnimState* animstate = record->m_entity->m_PlayerAnimState();
						if (animstate)
							animstate->m_abs_yaw = record->m_body;
					}

					data->m_lby_delta = abs(data->m_lby_delta);
					data->m_allow_freestand_lby_delta = false;
				}
			}
		}
	}

	float starting_yaw = record->m_body - data->m_lby_delta;
	float average_fake_yaw = 0;

	const int fake_range = std::min(8, int(data->m_records.size()));
	if (fake_range > 0) {
		for (int i = 0; i < fake_range; i++) {
			average_fake_yaw += i == 0 ? record->m_fake_angles.y : data->m_records[i - 1].m_fake_angles.y;
		}
		average_fake_yaw /= fake_range;
	}
	else {
		if (data->m_fake_type == FAKE_JITTER) {
			data->m_fake_type = FAKE_UNKNOWN;
		}
	}

	float rounded_average_fake_yaw = SnapToNearestYaw(average_fake_yaw, { starting_yaw, starting_yaw + 180, starting_yaw + 90, starting_yaw - 90 });

	if (record->m_resolver_mode == RESOLVE_BODY) {
		/*if ((data->m_calc_lby && !data->m_prev_calc_lby) || data->m_recalculate_fake_offsets) {
			data->m_static_fake_delta = record->m_fake_angles.y - (starting_yaw);
			data->m_jitter_fake_delta = rounded_average_fake_yaw - (starting_yaw);
		}*/
		data->m_static_fake_delta = record->m_fake_angles.y - (starting_yaw);
		data->m_jitter_fake_delta = rounded_average_fake_yaw - (starting_yaw);
		return;
	}

	data->m_resolved_yaw = starting_yaw;
	record->m_resolver_mode = (data->m_updates_since_lby_update_tick == 0 && !data->m_cant_hit_lby) ? RESOLVE_STAND_LBY_DELTA_FIRST_TICK : RESOLVE_STAND_LBY_DELTA;

	if (record->m_resolver_mode != RESOLVE_STAND_LBY_DELTA_FIRST_TICK && !data->m_shot_logs.m_missed_shots) {
		if (data->m_fake_type == FAKE_STATIC) {
			data->m_resolved_yaw = record->m_fake_angles.y - data->m_static_fake_delta;
			record->m_resolver_mode = RESOLVE_STAND_FAKE_STATIC;
		}
		else if (data->m_fake_type == FAKE_JITTER) {
			data->m_resolved_yaw = rounded_average_fake_yaw - data->m_jitter_fake_delta;
			record->m_resolver_mode = RESOLVE_STAND_FAKE_JITTER;
		}
	}

	if (!data->m_calc_lby) {
		record->m_resolver_mode = RESOLVE_STAND_STOPPED_MOVING;
	}

	++data->m_updates_since_lby_update_tick;
	data->m_prev_calc_lby = data->m_calc_lby;
}

void Resolver::ResolveWalk(AnimationRecord* record, AimPlayer* data) {
	data->m_resolved_yaw = data->m_move_lby = record->m_body;
	record->m_resolver_mode = RESOLVE_WALK;

	data->m_cant_hit_lby = false;

	data->m_lby_timer = record->m_anim_time + 0.242f;
	data->m_move_sim_time = record->m_sim_time;
	data->m_updates_since_lby_update_tick = 0;
	data->m_lby_updates_since_moving = 0;
	data->m_times_lby_changed_since_first_break = 0;

	data->m_lby_delta = data->m_og_lby_delta = data->m_delayed_lby_delta = 0.f; // or else the logging when they stop moving will be wrong.
	data->m_calc_lby = false;
	data->m_prev_calc_lby = false;
	data->m_recalculate_fake_offsets = false;

	data->m_shot_logs.m_missed_shots = data->m_prev_missed_shots_lol = 0;
}

void Resolver::ResolveAir(AnimationRecord* record, AimPlayer* data) {
	if (data->m_body_proxy != data->m_old_body_proxy) {
		data->m_resolved_yaw = data->m_last_ground_yaw = record->m_body;
		record->m_resolver_mode = RESOLVE_BODY;
		return;
	}

	float starting_yaw = GetAwayAngle(record) + 180.f;//float starting_yaw = data->m_last_ground_yaw;
	float average_fake_yaw = 0;

	const int fake_range = std::min(8, int(data->m_records.size()));
	if (fake_range > 0) {
		for (int i = 0; i < fake_range; i++) {
			average_fake_yaw += i == 0 ? record->m_fake_angles.y : data->m_records[i - 1].m_fake_angles.y;
		}
		average_fake_yaw /= fake_range;
	}
	else {
		if (data->m_fake_type == FAKE_JITTER) {
			data->m_fake_type = FAKE_UNKNOWN;
		}
	}

	float rounded_average_fake_yaw = SnapToNearestYaw(average_fake_yaw, { starting_yaw, starting_yaw + 180, starting_yaw + 90, starting_yaw - 90 });

	data->m_resolved_yaw = data->m_last_ground_yaw;//starting_yaw;
	record->m_resolver_mode = RESOLVE_STAND_LBY_DELTA;

	if (record->m_resolver_mode != RESOLVE_STAND_LBY_DELTA_FIRST_TICK) {
		if (data->m_fake_type == FAKE_STATIC) {
			data->m_resolved_yaw = record->m_fake_angles.y - data->m_static_fake_delta;
			record->m_resolver_mode = RESOLVE_STAND_FAKE_STATIC;
		}
		else if (data->m_fake_type == FAKE_JITTER) {
			data->m_resolved_yaw = rounded_average_fake_yaw - data->m_jitter_fake_delta;
			record->m_resolver_mode = RESOLVE_STAND_FAKE_JITTER;
		}
	}
}
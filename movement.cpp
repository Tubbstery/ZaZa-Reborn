#include "includes.h"

Movement g_movement{ };;

void Movement::Main() {
	if (!g_cl.m_processing)
		return;

	auto backup_pressing_move = g_cl.m_pressing_move;

	if (g_menu.main.antiaim.override_retardmode.get()) {
		if (g_cl.m_pressing_move && !g_cl.m_prev_pressing_move) {
			g_hvh.m_retardmode = true;
			g_hvh.m_pre_retardmode = true;
			g_hvh.m_pre_retardmode_swap_next_tick = false;
		}

		if (g_hvh.m_pre_retardmode) {
			if (g_hvh.m_pre_retardmode_swap_next_tick) {
				g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.0f;
				g_cl.m_pressing_move = false;
			}
			else {
				g_cl.m_cmd->m_forward_move = 455.f;
				g_cl.m_cmd->m_side_move = 0.0f;
			}
		}
	}
	else {
		g_hvh.m_retardmode = false;

		if (g_menu.main.antiaim.moving_feetyaw_desync.get() && !g_hvh.m_desync_walk && g_cl.m_pressing_move && !g_cl.m_prev_pressing_move) {
			g_hvh.m_desync_walk = true;
		}

		if (g_hvh.m_desync_walk) {
			g_cl.m_cmd->m_forward_move = g_cl.m_cmd->m_side_move = 0.0f;
			g_cl.m_pressing_move = false;
		}
	}

	JumpRelated();
	Strafe();
	FakeWalk();
	AutoPeek();
	g_cl.m_pressing_move = backup_pressing_move;
}

void Movement::JumpRelated() {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP)
		return;

	if ((g_cl.m_cmd->m_buttons & IN_JUMP) && !(g_cl.m_flags & FL_ONGROUND)) {
		// bhop.
		if (g_menu.main.movement.bhop.get())
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;

		// duck jump ( crate jump ).
		if (g_menu.main.movement.airduck.get())
			g_cl.m_cmd->m_buttons |= IN_DUCK;
	}
}

void Movement::Strafe() {
	vec3_t velocity;
	float  delta, abs_delta, velocity_delta, correct;

	// don't strafe while we prolly want to jump scout..
	// if (g_movement.m_slow_motion)
	//    return;

	// don't strafe while noclipping or on ladders..
	if (g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	// get networked velocity ( maybe absvelocity better here? ).
	// meh, should be predicted anyway? ill see.
	velocity = g_cl.m_local->m_vecAbsVelocity();

	// get the velocity len2d ( speed ).
	m_speed = velocity.length_2d();

	// compute the ideal strafe angle for our velocity.
	m_ideal = (m_speed > 0.f) ? math::rad_to_deg(std::asin(15.f / m_speed)) : 90.f;
	m_ideal2 = (m_speed > 0.f) ? math::rad_to_deg(std::asin(30.f / m_speed)) : 90.f;

	// some additional sanity.
	math::clamp(m_ideal, 0.f, 90.f);
	math::clamp(m_ideal2, 0.f, 90.f);

	// save entity bounds ( used much in circle-strafer ).
	m_mins = g_cl.m_local->m_vecMins();
	m_maxs = g_cl.m_local->m_vecMaxs();

	// save our origin
	m_origin = g_cl.m_local->m_vecOrigin();

	// disable strafing while pressing shift.
	if ((g_cl.m_buttons & IN_SPEED) || (g_cl.m_flags & FL_ONGROUND))
		return;

	// for changing direction.
	// we want to change strafe direction every call.
	m_switch_value *= -1.f;

	// for allign strafer.
	++m_strafe_index;

	if (g_cl.m_pressing_move && g_menu.main.movement.autostrafe.get()) {
		// took this idea from stacker, thank u !!!!
		enum EDirections {
			FORWARDS = 0,
			BACKWARDS = 180,
			LEFT = 90,
			RIGHT = -90,
			BACK_LEFT = 135,
			BACK_RIGHT = -135
		};

		float wish_dir{ };

		// get our key presses.
		bool holding_w = g_cl.m_buttons & IN_FORWARD;
		bool holding_a = g_cl.m_buttons & IN_MOVELEFT;
		bool holding_s = g_cl.m_buttons & IN_BACK;
		bool holding_d = g_cl.m_buttons & IN_MOVERIGHT;

		// move in the appropriate direction.
		if (holding_w) {
			//    forward left
			if (holding_a) {
				wish_dir += (EDirections::LEFT / 2);
			}
			//    forward right
			else if (holding_d) {
				wish_dir += (EDirections::RIGHT / 2);
			}
			//    forward
			else {
				wish_dir += EDirections::FORWARDS;
			}
		}
		else if (holding_s) {
			//    back left
			if (holding_a) {
				wish_dir += EDirections::BACK_LEFT;
			}
			//    back right
			else if (holding_d) {
				wish_dir += EDirections::BACK_RIGHT;
			}
			//    back
			else {
				wish_dir += EDirections::BACKWARDS;
			}

			g_cl.m_cmd->m_forward_move = 0;
		}
		else if (holding_a) {
			//    left
			wish_dir += EDirections::LEFT;
		}
		else if (holding_d) {
			//    right
			wish_dir += EDirections::RIGHT;
		}

		g_cl.m_strafe_angles.y += math::NormalizedAngle(wish_dir);
	}

	// cancel out any forwardmove values.
	g_cl.m_cmd->m_forward_move = 0.f;

	// do allign strafer.
	if (g_input.GetKeyState(g_menu.main.movement.astrafe.get())) {
		float angle = std::max(m_ideal2, 4.f);

		if (angle > m_ideal2 && !(m_strafe_index % 5))
			angle = m_ideal2;

		// add the computed step to the steps of the previous circle iterations.
		m_circle_yaw = math::NormalizedAngle(m_circle_yaw + angle);

		// apply data to usercmd.
		g_cl.m_strafe_angles.y = m_circle_yaw;
		g_cl.m_cmd->m_side_move = -450.f;

		return;
	}

	// do ciclestrafer
	else if (g_input.GetKeyState(g_menu.main.movement.cstrafe.get())) {
		// if no duck jump.
		if (!g_menu.main.movement.airduck.get()) {
			// crouch to fit into narrow areas.
			g_cl.m_cmd->m_buttons |= IN_DUCK;
		}

		DoPrespeed();
		return;
	}

	else if (g_input.GetKeyState(g_menu.main.movement.zstrafe.get())) {
		float freq = (g_menu.main.movement.z_freq.get() * 0.2f) * g_csgo.m_globals->m_realtime;

		// range [ 1, 100 ], aka grenerates a factor.
		float factor = g_menu.main.movement.z_dist.get() * 0.5f;

		g_cl.m_strafe_angles.y += (factor * std::sin(freq));
	}

	if (!g_menu.main.movement.autostrafe.get())
		return;

	// get our viewangle change.
	delta = math::NormalizedAngle(g_cl.m_strafe_angles.y - m_old_yaw);

	// convert to absolute change.
	abs_delta = std::abs(delta);

	// save old yaw for next call.
	m_circle_yaw = m_old_yaw = g_cl.m_strafe_angles.y;

	// set strafe direction based on mouse direction change.
	if (delta > 0.f)
		g_cl.m_cmd->m_side_move = -450.f;

	else if (delta < 0.f)
		g_cl.m_cmd->m_side_move = 450.f;

	// we can accelerate more, because we strafed less then needed
	// or we got of track and need to be retracked.
	if (abs_delta <= m_ideal || abs_delta >= 30.f) {
		// compute angle of the direction we are traveling in.
		ang_t velocity_angle;
		math::VectorAngles(velocity, velocity_angle);

		// get the delta between our direction and where we are looking at.
		velocity_delta = math::NormalizedAngle(g_cl.m_strafe_angles.y - velocity_angle.y);

		// correct our strafe amongst the path of a circle.
		correct = m_ideal;

		if (velocity_delta <= correct || m_speed <= 15.f) {
			// not moving mouse, switch strafe every tick.
			if (-correct <= velocity_delta || m_speed <= 15.f) {
				g_cl.m_strafe_angles.y += (m_ideal * m_switch_value);
				g_cl.m_cmd->m_side_move = 450.f * m_switch_value;
			}

			else {
				g_cl.m_strafe_angles.y = velocity_angle.y - correct;
				g_cl.m_cmd->m_side_move = 450.f;
			}
		}

		else {
			g_cl.m_strafe_angles.y = velocity_angle.y + correct;
			g_cl.m_cmd->m_side_move = -450.f;
		}
	}
}

void Movement::DoPrespeed() {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	// min and max values are based on 128 ticks.
	mod = g_csgo.m_globals->m_interval * 128.f;

	// scale min and max based on tickrate.
	min = 2.25f * mod;
	max = 5.f * mod;

	// compute ideal strafe angle for moving in a circle.
	strafe = m_ideal * 2.f;

	// clamp ideal strafe circle value to min and max step.
	math::clamp(strafe, min, max);

	// calculate time.
	time = 320.f / m_speed;

	// clamp time.
	math::clamp(time, 0.35f, 1.f);

	// init step.
	step = strafe;

	while (true) {
		// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
		if (!WillCollide(time, step) || max <= step)
			break;

		// if we will collide with an object with the current strafe step then increment step to prevent a collision.
		step += 0.2f;
	}

	if (step > max) {
		// reset step.
		step = strafe;

		while (true) {
			// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
			if (!WillCollide(time, step) || step <= -min)
				break;

			// if we will collide with an object with the current strafe step decrement step to prevent a collision.
			step -= 0.2f;
		}

		if (step < -min) {
			if (GetClosestPlane(plane)) {
				// grab the closest object normal
				// compute the angle of the normal
				// and push us away from the object.
				angle = math::rad_to_deg(std::atan2(plane.y, plane.x));
				step = -math::NormalizedAngle(m_circle_yaw - angle) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	// add the computed step to the steps of the previous circle iterations.
	m_circle_yaw = math::NormalizedAngle(m_circle_yaw + step);

	// apply data to usercmd.
	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_side_move = (step >= 0.f) ? -450.f : 450.f;
}

bool Movement::GetClosestPlane(vec3_t& plane) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	// trace around us in a circle
	for (float step{ }; step <= math::pi_2; step += (math::pi / 10.f)) {
		// extend endpoint x units.
		vec3_t end = start;
		end.x += std::cos(step) * dist;
		end.y += std::sin(step) * dist;

		g_csgo.m_engine_trace->TraceRay(Ray(start, end, m_mins, m_maxs), CONTENTS_SOLID, &filter, &trace);

		// we found an object closer, then the previouly found object.
		if (trace.m_fraction < smallest) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// did we find any valid object?
	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide(float time, float change) {
	struct PredictionData_t {
		vec3_t start;
		vec3_t end;
		vec3_t velocity;
		float  direction;
		bool   ground;
		float  predicted;
	};

	PredictionData_t      data;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// set base data.
	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity();
	data.direction = math::rad_to_deg(std::atan2(data.velocity.y, data.velocity.x));

	for (data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval) {
		// predict movement direction by adding the direction change.
		// make sure to normalize it, in case we go over the -180/180 turning point.
		data.direction = math::NormalizedAngle(data.direction + change);

		// pythagoras.
		float hyp = data.velocity.length_2d();

		// adjust velocity for new direction.
		data.velocity.x = std::cos(math::deg_to_rad(data.direction)) * hyp;
		data.velocity.y = std::sin(math::deg_to_rad(data.direction)) * hyp;

		// assume we bhop, set upwards impulse.
		if (data.ground)
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat();

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval;

		// we adjusted the velocity for our new direction.
		// see if we can move in this direction, predict our new origin if we were to travel at this velocity.
		data.end += (data.velocity * g_csgo.m_globals->m_interval);

		// trace
		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		// check if we hit any objects.
		if (trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f)
			return true;
		if (trace.m_startsolid || trace.m_allsolid)
			return true;

		// adjust start and end point.
		data.start = data.end = trace.m_endpos;

		// move endpoint 2 units down, and re-trace.
		// do this to check if we are on th floor.
		g_csgo.m_engine_trace->TraceRay(Ray(data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs), MASK_PLAYERSOLID, &filter, &trace);

		// see if we moved the player into the ground for the next iteration.
		data.ground = trace.hit() && trace.m_plane.m_normal.z > 0.7f;
	}

	// the entire loop has ran
	// we did not hit shit.
	return false;
}

void Movement::MoonWalk(CUserCmd* cmd) {
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	const float forwardmove = cmd->m_forward_move;
	const float sidemove = cmd->m_side_move;
	int new_buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

	if (!g_menu.main.antiaim.leg_movement.get()) {

		if (forwardmove <= 0.f) {
			if (forwardmove < 0.f)
				new_buttons |= IN_BACK;
		}
		else
			new_buttons |= IN_FORWARD;

		if (sidemove >= 0.f) {

			if (sidemove > 0.f)
				goto LABEL_15;

			goto LABEL_18;
		}

		goto LABEL_17;
	}

	if (g_menu.main.antiaim.leg_movement.get() != 1)
		goto LABEL_18;

	if (forwardmove <= 0.f)
	{
		if (forwardmove < 0.f)
			new_buttons |= IN_FORWARD;
	}
	else
		new_buttons |= IN_BACK;

	if (sidemove > 0.f) {

	LABEL_17:
		new_buttons |= IN_MOVELEFT;
		goto LABEL_18;
	}

	if (sidemove < 0.f) {
	LABEL_15:
		new_buttons |= IN_MOVERIGHT;
	}

LABEL_18:
	cmd->m_buttons = new_buttons;
}

void Movement::FixMove(CUserCmd* cmd, const ang_t& wish_angles) {

	vec3_t  move, dir;
	float   delta, len;
	ang_t   move_angle;

	// roll nospread fix.
	if (!(g_cl.m_flags & FL_ONGROUND) && cmd->m_view_angles.z != 0.f)
		cmd->m_side_move = 0.f;

	// convert movement to vector.
	move = { cmd->m_forward_move, cmd->m_side_move, 0.f };

	// get move length and ensure we're using a unit vector ( vector with length of 1 ).
	len = move.normalize();
	if (!len)
		return;

	// convert move to an angle.
	math::VectorAngles(move, move_angle);

	// calculate yaw delta.
	delta = (cmd->m_view_angles.y - wish_angles.y);

	// accumulate yaw delta.
	move_angle.y += delta;

	// calculate our new move direction.
	// dir = move_angle_forward * move_length
	math::AngleVectors(move_angle, &dir);

	// scale to og movement.
	dir *= len;

	// strip old flags.
	g_cl.m_cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);

	// fix ladder and noclip.
	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER) {
		// invert directon for up and down.
		if (cmd->m_view_angles.x >= 45.f && wish_angles.x < 45.f && std::abs(delta) <= 65.f)
			dir.x = -dir.x;

		// write to movement.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if (cmd->m_forward_move > 200.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < -200.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 200.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < -200.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}

	// we are moving normally.
	else {
		// we must do this for pitch angles that are out of bounds.
		if (cmd->m_view_angles.x < -90.f || cmd->m_view_angles.x > 90.f)
			dir.x = -dir.x;

		// set move.
		cmd->m_forward_move = dir.x;
		cmd->m_side_move = dir.y;

		// set new button flags.
		if (cmd->m_forward_move > 0.f)
			cmd->m_buttons |= IN_FORWARD;

		else if (cmd->m_forward_move < 0.f)
			cmd->m_buttons |= IN_BACK;

		if (cmd->m_side_move > 0.f)
			cmd->m_buttons |= IN_MOVERIGHT;

		else if (cmd->m_side_move < 0.f)
			cmd->m_buttons |= IN_MOVELEFT;
	}

	MoonWalk(g_cl.m_cmd);
}

void Movement::AutoPeek() {
	bool in_autopeek = false;

	if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
		if ((g_cl.m_local->m_vecOrigin() - g_cl.m_pre_autopeek_pos).length_2d() < 10.f) {
			m_autopeek_return = false;
		}

		if (m_autopeek_return) {
			vec3_t ang = math::CalcAngle(g_cl.m_local->m_vecOrigin(), g_cl.m_pre_autopeek_pos);

			g_cl.m_cmd->m_forward_move = 450.f;
			g_cl.m_cmd->m_side_move = 0.f;
			g_cl.m_strafe_angles.y = ang.y;
			in_autopeek = true;

			if (g_menu.main.movement.autopeek_knife.get() && g_cl.m_old_shot) {
				auto weapons = g_cl.m_local->m_hMyWeapons();

				int i = 0;
				while (weapons[i].IsValid()) {
					Weapon* weapon = (Weapon*)g_csgo.m_entlist->GetClientEntityFromHandle< Weapon* >(weapons[i]);
					if (!weapon)
						continue;

					if (weapon->IsKnife()) {
						g_cl.m_cmd->m_weapon_select = weapon->index();
						break;
					}
					
					i++;
				}
			}
		}
	}
	else {
		m_autopeek_return = false;
		g_cl.m_pre_autopeek_pos = g_cl.m_local->m_vecOrigin();
		std::memcpy(g_cl.m_pre_autopeek_bones, g_cl.m_local->m_BoneCache().m_pCachedBones, sizeof(matrix3x4_t) * g_cl.m_local->m_BoneCache().m_CachedBoneCount);
	}

	if (((g_aimbot.m_stop || g_aimbot.m_stop_early) && g_cl.m_weapon_id != ZEUS && g_cl.m_flags & FL_ONGROUND) || (!g_cl.m_pressing_move && g_menu.main.movement.quickstop.get() && !in_autopeek)) {
		Movement::QuickStop();
	}
}

/* thanks onetap.com */
void Movement::ClampMovementSpeed(float speed) {
	float final_speed = speed;

	if (!g_cl.m_cmd || !g_cl.m_processing)
		return;

	g_cl.m_cmd->m_buttons |= IN_SPEED;

	float squirt = std::sqrtf((g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move) + (g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move));

	if (squirt >= speed) {
		float squirt2 = std::sqrtf((g_cl.m_cmd->m_forward_move * g_cl.m_cmd->m_forward_move) + (g_cl.m_cmd->m_side_move * g_cl.m_cmd->m_side_move));

		float cock1 = g_cl.m_cmd->m_forward_move / squirt2;
		float cock2 = g_cl.m_cmd->m_side_move / squirt2;

		g_cl.m_cmd->m_forward_move = cock1 * final_speed;
		g_cl.m_cmd->m_side_move = cock2 * final_speed;
	}
}

void Movement::QuickStop() {
	if (!g_cl.m_weapon_info)
		return;

	// convert velocity to angular momentum.
	ang_t angle;
	math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

	// get our current speed of travel.
	float speed = g_cl.m_local->m_vecVelocity().length();

	// fix direction by factoring in where we are looking.
	angle.y = g_cl.m_view_angles.y - angle.y;

	// convert corrected angle back to a direction.
	vec3_t direction;
	math::AngleVectors(angle, &direction);

	vec3_t stop = direction * -speed;

	// get the max possible speed whilest we are still accurate.
	float flMaxSpeed = g_cl.m_local->m_bIsScoped() > 0 ? g_cl.m_weapon_info->m_max_player_speed_alt : g_cl.m_weapon_info->m_max_player_speed;
	float flDesiredSpeed = (flMaxSpeed * 0.33);

	float speedlength = g_cl.m_local->m_vecVelocity().length_2d();

	if (speedlength - flDesiredSpeed > 1.f || !g_cl.m_pressing_move) {
		if (speedlength > 13.f) {
			g_cl.m_cmd->m_forward_move = stop.x;
			g_cl.m_cmd->m_side_move = stop.y;
		}
		else {
			g_cl.m_cmd->m_forward_move = 0.0f;
			g_cl.m_cmd->m_side_move = 0.0f;
		}
	}
	else {
		ClampMovementSpeed(flDesiredSpeed);
	}
}

void Movement::NullVelocity(CUserCmd* cmd) {
	vec3_t vel = g_cl.m_local->m_vecVelocity();

	if (vel.length_2d() < 15.f) {
		cmd->m_forward_move = cmd->m_side_move = 0.f;
		return;
	}

	ang_t direction;
	math::VectorAngles(vel, direction);

	ang_t view_angles;
	g_csgo.m_engine->GetViewAngles(view_angles);

	direction.y = view_angles.y - direction.y;

	vec3_t forward;
	math::AngleVectors(direction, &forward);

	static ConVar* cl_forwardspeed = g_csgo.m_cvar->FindVar(HASH("cl_forwardspeed"));
	static ConVar* cl_sidespeed = g_csgo.m_cvar->FindVar(HASH("cl_sidespeed"));

	const vec3_t negative_forward_direction = forward * -cl_forwardspeed->GetFloat();
	const vec3_t negative_side_direction = forward * -cl_sidespeed->GetFloat();

	cmd->m_forward_move = negative_forward_direction.x;
	cmd->m_side_move = negative_side_direction.y;
}

void Movement::FakeWalk() {
	vec3_t velocity = g_cl.m_local->m_vecVelocity();
	float speed_2d = velocity.length_2d();
	if (!g_input.GetKeyState(g_menu.main.antiaim.fakewalk.get())) {
		m_moving_before_fakewalk = speed_2d >= 0.1f;
		m_fake_walking = false;
		return;
	}

	if (!g_cl.m_local->GetGroundEntity()) {
		NullVelocity(g_cl.m_cmd);
		m_fake_walking = false;
		return;
	}

	// our feetyaw is desynced just slowwalk lol.
	if (g_hvh.m_desync_walk_active && g_menu.main.antiaim.fakewalk_disable_desync.get()) {
		float flMaxSpeed = g_cl.m_local->m_bIsScoped() > 0 ? g_cl.m_weapon_info->m_max_player_speed_alt : g_cl.m_weapon_info->m_max_player_speed;
		ClampMovementSpeed(flMaxSpeed * 0.33);
		return;
	}

	g_hvh.m_desync_walk_active = false;
	g_hvh.m_desync_walk = false;
	m_fake_walking = true;

	if (m_moving_before_fakewalk) {
		if (speed_2d >= 0.1f) {
			g_cl.m_pressing_move = false;
			QuickStop();
			return;
		}
	}

	m_moving_before_fakewalk = false;

	// reference:
	// https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/game/shared/gamemovement.cpp#L1612

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat() * g_cl.m_local->m_surfaceFriction();
	int ticks = 0;
	for (; ticks < g_cl.m_max_lag; ++ticks) {
		// calculate speed.
		float speed = velocity.length();

		// if too slow return.
		if (speed <= 0.1f)
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max(speed, g_csgo.sv_stopspeed->GetFloat());

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max(0.f, speed - drop);

		if (newspeed != speed) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	// zero forwardmove and sidemove.
	if (ticks >= g_cl.m_max_lag - g_csgo.m_cl->m_choked_commands || !g_csgo.m_cl->m_choked_commands) {
		g_cl.m_pressing_move = false;
		QuickStop();
	}
}
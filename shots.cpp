#include "includes.h"
#include "hitsounds.h"

#include <urlmon.h>
#include <mmsystem.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment(lib, "Iphlpapi.lib")

Shots g_shots{ };

void Shots::OnFSN() {
	if (m_impacts.empty())
		return;

	for (auto i = 0; i < m_impacts.size(); i++) {
		auto cur = &m_impacts[i];
		if (!cur)
			return;

		if (cur->m_hit)
			continue;

		if (!cur->m_shot->m_target->alive()) {
			g_notify.add("missed shot due to enemy death\n");
		}
		else {
			BackupRecord backup;
			backup.store(cur->m_shot->m_target);

			cur->m_shot->m_record->cache();

			auto start = cur->m_shot->m_pos;
			auto dir = (cur->m_pos - start).normalized();
			auto end = start + (dir * cur->m_shot->m_weapon_dist);

			CGameTrace trace;
			g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT, cur->m_shot->m_target, &trace);

			if (!trace.m_entity || !trace.m_entity->IsPlayer() || trace.m_entity != cur->m_shot->m_target) {
				g_notify.add(tfm::format(XOR("missed shot due to spread\n")));
			}
			else if (trace.m_entity == cur->m_shot->m_target) {
				if (cur->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_WALK || cur->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_NONE) {
					g_notify.add(XOR("missed shot due to unknown\n"));
				}
				else {
					if (cur->m_shot->m_record->m_broke_lc) {
						LagRecord* front = &g_aimbot.m_players[cur->m_shot->m_target->index() - 1].m_records[0];

						// dont need to store a backup record here since we restore the og after anyways.
						front->cache();

						auto start = cur->m_shot->m_pos;
						auto dir = (cur->m_pos - start).normalized();
						auto end = start + (dir * cur->m_shot->m_weapon_dist);

						CGameTrace tr2;
						g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT, cur->m_shot->m_target, &tr2);

						if (!tr2.m_entity || !tr2.m_entity->IsPlayer() || tr2.m_entity != cur->m_shot->m_target) {
							g_notify.add(cur->m_shot->m_record->m_lagfixed ? "missed shot due to extrapolation\n" : "missed shot due to lag compensation\n");
						}
						else {
							g_notify.add(XOR("missed shot due to resolver\n"));
						}
					}
					else {
						if (cur->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_BODY) {
							g_notify.add(XOR("missed shot due to resolver (lby)\n"));
						}
						else {
							g_notify.add(XOR("missed shot due to resolver\n"));
						}

						if (cur->m_shot->m_hitbox == HITBOX_HEAD) { // idk if should be head only. but since hits are it makes sense.
							g_resolver.LogShot(cur->m_shot->m_record, true);
						}
					}
				}
			}

			backup.restore(cur->m_shot->m_target);
		}

		m_impacts.clear();
	}
}

void Shots::OnShotFire(Player* target, float damage, int bullets, LagRecord* record, int hitbox) {

	// iterate all bullets in this shot.
	for (int i{ }; i < bullets; ++i) {
		// setup new shot data.
		ShotRecord shot;
		shot.m_target = target;
		shot.m_record = record;
		shot.m_time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
		shot.m_lat = g_cl.m_latency + (g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING) * 2);
		shot.m_damage = damage;
		shot.m_hitbox = hitbox;
		shot.m_pos = g_cl.m_shoot_pos;
		if (g_cl.m_weapon_info) {
			shot.m_weapon_dist = g_cl.m_weapon_info->m_range;
		}

		// add to tracks.
		m_shots.push_front(shot);
	}

	// no need to keep an insane amount of shots.
	while (m_shots.size() > 128)
		m_shots.pop_back();
}

void Shots::OnImpact(IGameEvent* evt) {
	int        attacker;
	vec3_t     pos, dir, start, end;
	float      time;
	CGameTrace trace;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;

	// decode impact coordinates and convert to vec3.
	pos = {
		evt->m_keys->FindKey(HASH("x"))->GetFloat(),
		evt->m_keys->FindKey(HASH("y"))->GetFloat(),
		evt->m_keys->FindKey(HASH("z"))->GetFloat()
	};

	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (!attacker)
		return;

	if (attacker != g_csgo.m_engine->GetLocalPlayer()) {
		auto ent = (Player*)g_csgo.m_entlist->GetClientEntity(attacker);
		if (!ent)
			return;

		if (g_menu.main.visuals.impact_beams_select.get(1))
			m_vis_impacts.push_back({ pos, ent->GetShootPosition(), g_cl.m_local->m_nTickBase(), false });

		return;
	}

	if (g_menu.main.misc.bullet_impacts.get())
		g_csgo.m_debug_overlay->AddBoxOverlay(pos, vec3_t(-2.0f, -2.0f, -2.0f), vec3_t(2.0f, 2.0f, 2.0f), ang_t(0.0f, 0.0f, 0.0f), 0, 0, 255, 127, 3.0f);

	// ADD TO DIS BITCH FOR DA HITMARKERS NIGGAAAAAAAAAAAAAAAA.
	impact_info info;
	info.pos = pos;
	info.time = g_csgo.m_globals->m_curtime;
	g_visuals.m_impacts.push_back(info);

	// get prediction time at this point.
	time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	// add to visual impacts if we have features that rely on it enabled.
	// todo - dex; need to match shots for this to have proper GetShootPosition, don't really care to do it anymore.
	if (g_menu.main.visuals.impact_beams_select.get(0))
		m_vis_impacts.push_back({ pos, g_cl.m_local->GetShootPosition(), g_cl.m_local->m_nTickBase(), true });

	// we did not take a shot yet.
	if (m_shots.empty())
		return;

	struct ShotMatch_t { float delta; ShotRecord* shot; };
	ShotMatch_t match;
	match.delta = std::numeric_limits< float >::max();
	match.shot = nullptr;

	// iterate all shots.
	for (auto& s : m_shots) {

		// this shot was already matched
		// with a 'bullet_impact' event.
		if (s.m_matched)
			continue;

		// add the latency to the time when we shot.
		// to predict when we would receive this event.
		float predicted = s.m_time + s.m_lat;

		// get the delta between the current time
		// and the predicted arrival time of the shot.
		float delta = std::abs(time - predicted);

		// fuck this.
		if (delta > 1.f)
			continue;

		// store this shot as being the best for now.
		if (delta < match.delta) {
			match.delta = delta;
			match.shot = &s;
		}
	}

	// no valid shotrecord was found.
	ShotRecord* shot = match.shot;
	if (!shot)
		return;

	// this shot was matched.
	shot->m_matched = true;

	// draw debug overlay.
	if (g_menu.main.aimbot.aimbot_debug.get(0)) {
		g_visuals.DrawHitboxMatrix(shot->m_record, g_aimbot.m_backup[shot->m_target->index() - 1].m_matrix, shot->m_hitbox, Color(0, 255, 0));
	}

	// create new impact instance that we can match with a player hurt.
	ImpactRecord impact;
	impact.m_shot = shot;
	impact.m_tick = g_cl.m_local->m_nTickBase();
	impact.m_pos = pos;

	// add to track.
	m_impacts.push_front(impact);

	// no need to keep an insane amount of impacts.
	while (m_impacts.size() > 128)
		m_impacts.pop_back();
}

void Shots::OnHurt(IGameEvent* evt) {
	int         attacker, victim, group, hp;
	float       damage;
	std::string name;

	if (!evt || !g_cl.m_local)
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
	victim = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if (attacker < 1 || attacker > 64 || victim < 1 || victim > 64)
		return;

	// we were not the attacker or we hurt ourselves.
	else if (attacker != g_csgo.m_engine->GetLocalPlayer() || victim == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey(HASH("hitgroup"))->GetInt();

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if (group == HITGROUP_GEAR)
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity< Player* >(victim);
	if (!target)
		return;

	// get player info.
	player_info_t info;
	if (!g_csgo.m_engine->GetPlayerInfo(victim, &info))
		return;

	// get player name;
	name = std::string(info.m_name).substr(0, 24);

	// get damage reported by the server.
	damage = (float)evt->m_keys->FindKey(HASH("dmg_health"))->GetInt();

	// get remaining hp.
	hp = evt->m_keys->FindKey(HASH("health"))->GetInt();

	// hitmarker.
	g_visuals.m_hit_duration = 2.f;
	g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
	g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;

	switch (g_menu.main.misc.hitsound.get()) {
	case 1:
		PlaySoundA(reinterpret_cast<char*>(hitsound_1), nullptr, SND_ASYNC | SND_MEMORY);
		break;
	case 2:
		PlaySoundA(reinterpret_cast<char*>(hitsound_2), nullptr, SND_ASYNC | SND_MEMORY);
		break;
	case 3:
		g_csgo.m_sound->EmitAmbientSound(XOR("buttons/arena_switch_press_02.wav"), 1.f);
		break;
	}

	// print this shit.
	if (g_menu.main.misc.notifications.get(1)) {
		std::string out = tfm::format(XOR("hit %s in the %s for %i damage (%i health remaining)\n"), name, m_groups[group], (int)damage, hp);
		g_notify.add(out);
	}

	if (group == HITGROUP_GENERIC)
		return;

	// if we hit a player, mark vis impacts.
	if (!m_vis_impacts.empty()) {
		for (auto& i : m_vis_impacts) {
			if (i.m_tickbase == g_cl.m_local->m_nTickBase())
				i.m_hit_player = true;
		}
	}

	// no impacts to match.
	if (m_impacts.empty())
		return;

	ImpactRecord* impact{ nullptr };

	// iterate stored impacts.
	for (auto& i : m_impacts) {

		// this impact doesnt match with our current hit.
		if (i.m_tick != g_cl.m_local->m_nTickBase())
			continue;

		// wrong player.
		if (i.m_shot->m_target != target)
			continue;

		// shit fond.
		impact = &i;
		break;
	}

	// no impact matched.
	if (!impact)
		return;

	// this impact hit a player yaaaaaaaaay.
	impact->m_hit = true;

	// forward to resolver hit processing.
	if (group == HITGROUP_HEAD) {
		g_resolver.LogShot(impact->m_shot->m_record, false);
		++g_cl.m_miracle_taps;
	}

	// missmatch.
	/*if (group != g_aimbot.HitboxToHitgroup(impact->m_shot->m_hitbox)) {
		// we are going to alter this player.
        // store all his og data.
		BackupRecord backup;
		backup.store(impact->m_shot->m_target);

		// write historical matrix of the time that we shot
		// into the games bone cache, so we can trace against it.
		impact->m_shot->m_record->cache();

		// start position of trace is where we took the shot.
		auto start = impact->m_shot->m_pos;

		// the impact pos contains the spread from the server
		// which is generated with the server seed, so this is where the bullet
		// actually went, compute the direction of this from where the shot landed
		// and from where we actually took the shot.
		auto dir = (impact->m_pos - start).normalized();

		// get end pos by extending direction forward.
		auto end = start + (dir * impact->m_shot->m_weapon_dist);

		// intersect our historical matrix with the path the shot took.
		CGameTrace trace;
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(start, end), MASK_SHOT, impact->m_shot->m_target, &trace);

		if (trace.m_entity == impact->m_shot->m_target) {
			if (trace.m_hitgroup == group) {
				if (impact->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_WALK || impact->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_NONE) {
					g_notify.add(XOR("missed aim point due to animations\n"));
				}
				else {
					if (impact->m_shot->m_record->m_resolver_mode == Resolver::Modes::RESOLVE_BODY) {
						g_notify.add(XOR("missed aim point due to resolver (lby)\n"));
					}
					else {
						g_notify.add(XOR("missed aim point due to resolver\n"));
					}

					// forward to resolver miss processing.
					if (impact->m_shot->m_hitbox == HITBOX_HEAD) { // idk if should be head only. but since hits are it makes sense.
						g_resolver.LogShot(impact->m_shot->m_record, true);
					}
				}
			}
			else {
				g_notify.add(XOR("missed aim point due to spread\n"));
			}
		}

		// restore player to his original state.
		backup.restore(impact->m_shot->m_target);
	}*/
}
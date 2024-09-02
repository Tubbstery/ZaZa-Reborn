#pragma once

class BackupRecord {
public:
	matrix3x4_t  m_matrix[128];
	int          m_bone_count;
	vec3_t       m_origin, m_abs_origin;
	vec3_t       m_mins;
	vec3_t       m_maxs;
	ang_t        m_abs_ang;

public:
	__forceinline void store(Player* player) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		// store bone data.
		m_bone_count = cache->m_CachedBoneCount;
		memcpy(m_matrix, cache->m_pCachedBones, sizeof(matrix3x4_t) * m_bone_count);

		m_origin = player->m_vecOrigin();
		m_mins = player->m_vecMins();
		m_maxs = player->m_vecMaxs();
		m_abs_origin = player->GetAbsOrigin();
		m_abs_ang = player->GetAbsAngles();
	}

	__forceinline void restore(Player* player) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		memcpy(cache->m_pCachedBones, m_matrix, m_bone_count * sizeof(matrix3x4_t));
		cache->m_CachedBoneCount = m_bone_count;

		player->m_vecOrigin() = m_origin;
		player->m_vecMins() = m_mins;
		player->m_vecMaxs() = m_maxs;
		player->SetAbsAngles(m_abs_ang);
		player->SetAbsOrigin(m_origin);
	}
};

class LagRecord {
public:
	// data.
	Player* m_player;
	float   m_immune;
	int     m_tick;
	int     m_lag;
	float m_creation_time;

	// fucking retarded.
	int m_bullshit_index;

	// netvars.
	float  m_sim_time;
	int    m_flags;
	vec3_t m_origin;
	vec3_t m_velocity;
	vec3_t m_mins;
	vec3_t m_maxs;
	ang_t  m_eye_angles;
	ang_t  m_fake_angles;
	float  m_bullshit_fake_offset;
	ang_t  m_abs_ang;
	float  m_body;
	float  m_duck;

	// lagfix stuff.
	bool   m_broke_lc;
	bool   m_lagfixed;

	// resolver stuff.
	size_t m_resolver_mode;
	float  m_anim_time;
	float  m_resolver_lby_delta;

	// history matrix.
	float m_interp_time;

	// anim stuff.
	C_AnimationLayer m_layers[13];
	float            m_poses[24];

	// bone stuff.
	matrix3x4_t m_matrix[128];
	int         m_bone_count;

public:
	// function: writes current record to bone cache.
	__forceinline void cache() {
		// get bone cache ptr.
		CBoneCache* cache = &m_player->m_BoneCache();

		memcpy(cache->m_pCachedBones, m_matrix, m_bone_count * sizeof(matrix3x4_t));
		cache->m_CachedBoneCount = m_bone_count;

		m_player->m_vecOrigin() = m_origin;
		m_player->m_vecMins() = m_mins;
		m_player->m_vecMaxs() = m_maxs;

		m_player->SetAbsAngles(m_abs_ang);
		m_player->SetAbsOrigin(m_origin);
	}

	__forceinline bool immune() {
		return m_immune > 0.f;
	}

	// function: checks if LagRecord obj is hittable if we were to fire at it now.
	bool valid() {
		// use prediction curtime for this.
		float curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

		// correct is the amount of time we have to correct game time,
		float correct = g_cl.m_lerp + g_cl.m_latency;

		// stupid fake latency goes into the incoming latency.
		correct += g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING);

		// check bounds [ 0, sv_maxunlag ]
		math::clamp(correct, 0.f, g_csgo.sv_maxunlag->GetFloat());

		// calculate difference between tick sent by player and our latency based tick.
		// ensure this record isn't too old.
		return std::abs(correct - (curtime - game::TICKS_TO_TIME(game::TIME_TO_TICKS(m_sim_time)))) < 0.19f;
	}
};
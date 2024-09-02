#pragma once

struct AnimationBackup_t {
	vec3_t           m_origin, m_abs_origin;
	vec3_t           m_velocity, m_abs_velocity;
	int              m_flags, m_eflags;
	float            m_duck, m_body;
};

class AnimationRecord {
public:
	Player* m_entity;

	vec3_t m_origin;
	vec3_t m_velocity;
	vec3_t m_abs_velocity;
	ang_t  m_eye_angles;
	ang_t  m_fake_angles;
	float  m_bullshit_fake_offset;

	int m_flags;
	int m_lag;
	size_t m_resolver_mode;
	float m_resolver_lby_delta;

	bool m_fake_walk;
	bool m_broke_lc;

	float m_lag_time;
	float m_sim_time;
	float m_anim_time;
	float m_body;
	float m_duck;

	matrix3x4_t m_matrix[128];
	int m_bone_count;

	std::array<C_AnimationLayer, 13> m_server_layers;
	std::array<float, 20> m_poses;

	CCSGOPlayerAnimState animstate;
};

class AnimationData {
public:
	void Update();
	void CorrectVelocity(Player* m_player, AnimationRecord* m_record, AnimationRecord* m_previous, AnimationRecord* m_pre_previous);
	void Collect(Player* pPlayer);

	Player* m_player;

	float m_sim_time;
	float m_old_sim_time;

	struct
	{
		float m_flCycle;
		float m_flPlaybackRate;
	} last_alive_loop_data;

	bool m_was_dormant = false;
	bool m_anim_updated = false;

	bool m_resolver_was_dormant = false;

	bool m_alive = false;

	std::deque<AnimationRecord> m_AnimationRecord;
};

class AnimationSystem {
public:
	std::array< AnimationData, 64 > m_animated_players;
	void FrameStage();
};

extern AnimationSystem g_animationsystem;
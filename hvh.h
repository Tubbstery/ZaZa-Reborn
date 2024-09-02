#pragma once

class AdaptiveAngle {
public:
	float m_yaw;
	float m_dist;

public:
	// ctor.
	__forceinline AdaptiveAngle(float yaw, float penalty = 0.f) {
		// set yaw.
		m_yaw = math::NormalizedAngle(yaw);

		// init distance.
		m_dist = 0.f;

		// remove penalty.
		m_dist -= penalty;
	}
};

enum AntiAimMode : size_t {
	STAND = 0,
	WALK,
	AIR,
};

enum ManualAADirection : size_t {
	MANUAL_NONE = 0,
	MANUAL_FORWARD,
	MANUAL_LEFT,
	MANUAL_BACK,
	MANUAL_RIGHT,
};

class HVH {
public:
	size_t m_mode;
	size_t m_manual_direction;

	bool m_desync_walk;
	bool m_desync_walk_active;

	bool m_pre_retardmode_swap_next_tick;
	bool m_pre_retardmode;
	bool m_retardmode;
	bool   m_step_switch;
	int    m_random_lag;


public:
	Player* GetBestPlayer();
	float BossStanding();
	void AntiAim();
	void SendPacket();
};

extern HVH g_hvh;
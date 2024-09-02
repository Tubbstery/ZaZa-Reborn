#pragma once

class ShotRecord {
public:
	__forceinline ShotRecord() : m_target{}, m_record{}, m_time{}, m_lat{}, m_damage{}, m_pos{}, m_matched{} {}

public:
	Player* m_target;
	LagRecord* m_record;
	float      m_time, m_lat, m_damage;
	vec3_t     m_pos;
	bool       m_matched;
	int		   m_hitbox;
	int        m_weapon_dist;
};

class VisualImpactData_t {
public:
	vec3_t m_impact_pos, m_shoot_pos;
	int    m_tickbase;
	bool   m_hit_player, m_from_local;
	float  m_brightness;

public:
	__forceinline VisualImpactData_t(const vec3_t& impact_pos, const vec3_t& shoot_pos, int tickbase, bool from_local) :
		m_impact_pos{ impact_pos }, m_shoot_pos{ shoot_pos }, m_tickbase{ tickbase }, m_hit_player{ false }, m_from_local{ from_local }, m_brightness{ 255.f } {}
};

class ImpactRecord {
public:
	__forceinline ImpactRecord() : m_shot{}, m_pos{}, m_tick{} {}

public:
	ShotRecord* m_shot;
	int         m_tick;
	vec3_t      m_pos;
	bool        m_hit;
};

class Shots {

public:
	void OnFSN();
	void OnShotFire(Player* target, float damage, int bullets, LagRecord* record, int hitbox);
	void OnImpact(IGameEvent* evt);
	void OnHurt(IGameEvent* evt);

public:
	std::array< std::string, 11 > m_groups = {
		XOR("body"),
		XOR("head"),
		XOR("chest"),
		XOR("stomach"),
		XOR("left arm"),
		XOR("right arm"),
		XOR("left leg"),
		XOR("right leg"),
		XOR("neck"),
		XOR("unknown"),
		XOR("gear")
	};

	std::deque< ShotRecord >          m_shots;
	std::vector< VisualImpactData_t > m_vis_impacts;
	std::deque< ImpactRecord >        m_impacts;
};

extern Shots g_shots;
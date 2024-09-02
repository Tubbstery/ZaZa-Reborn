#pragma once

enum CheatNames : int {
	CHEAT_UNKNOWN = 0,
	/*CHEAT_CHEESE_CRACK,
	CHEAT_KAABA_CRACK,
	CHEAT_PANDORA,*/

	CHEAT_GOOFYHOOK_OLD,
	CHEAT_GOOFYHOOK,
};

enum HitscanMode : int {
	NORMAL = 0,
	LETHAL = 1,
	LETHAL2 = 3,
	PREFER = 4
};

struct BestTarget_t {
	Player* player;
	vec3_t pos;
	float damage, point_safety;
	LagRecord* record;
	int hitbox, hitgroup;

	__forceinline BestTarget_t() :
		player{ nullptr },
		pos{ 0.f, 0.f, 0.f },
		damage{ -1.f },
		point_safety{ -1.f },
		record{ nullptr },
		hitbox{ -1 }, 
		hitgroup{ -1 }
	{}
};

struct HitscanData_t {
	float  m_damage;
	vec3_t m_pos;
	int    m_hitbox, m_hitgroup;

	__forceinline HitscanData_t() : m_damage{ 0.f }, m_pos{}, m_hitbox{ -1 }, m_hitgroup{ -1 } {}
};

struct AimPoint_t {
	vec3_t m_pos;
	int    m_hitbox;

	float  m_hitbox_radius;
	float  m_point_safety;

	bool   m_rotated;

	__forceinline AimPoint_t() : m_pos{}, m_hitbox{ -1 }, m_hitbox_radius{ -1.f }, m_point_safety{ -1.f }, m_rotated { false } {}
};

struct ResolverShotLogging_t {
	std::vector<float> m_missed_lby_deltas;
	std::vector<float> m_hit_lby_deltas;
	int m_missed_shots;
};

struct PeekData_t {
	int m_localpeek_ticks;
	int m_enemypeek_ticks;

	// shouldnt really need damage delay shot with headaim.
	//int m_peek_damage_head;
	int m_peek_damage_body;

	bool m_filtered;
};

class AimPlayer {
public:
	// essential data.
	Player*   m_player;
	float	  m_spawn;
	std::deque<LagRecord> m_records;

	PeekData_t m_peek_data;
	int m_delay_shot_ticks;

	int m_lagfix_mode;

	int m_voice_cheat;
	bool m_goofy_whitelist;
	std::deque<std::string> m_goofy_voice_messages;
	int m_goofy_id;






	bool m_cant_hit_lby;
	vec3_t resolver_last_origin;
	float m_last_ground_yaw;

	float m_lby_timer;
	float m_move_lby;
	float m_move_sim_time;

	float m_lby_delta;
	float m_delayed_lby_delta;
	float m_og_lby_delta;

	bool  m_calc_lby;
	bool  m_prev_calc_lby;
	int   m_updates_since_lby_update_tick;
	int   m_lby_updates_since_moving;
	int   m_times_lby_changed_since_first_break;

	int   m_allow_freestand_lby_delta;

	float m_static_fake_delta;
	float m_jitter_fake_delta;

	ResolverShotLogging_t m_shot_logs;
	int m_prev_missed_shots_lol;

	size_t m_fake_type;
	bool m_recalculate_fake_offsets;

	int m_unlikely_resolve;
	std::vector<float> m_brute_angles;

	float m_resolved_yaw;

	float m_prev_shot_tick;




	int m_first_record_hitscan_preference;

	// body proxy.
	float     m_body_proxy;
	float     m_old_body_proxy;

public:
	void AddRecord(Player* player);
	void OnRoundStart(Player* player);
	void SetupHitboxPoints(LagRecord* record, matrix3x4_t* record_matrix, float mindmg, bool force_baim_misses, std::vector< AimPoint_t >& points);

public:
	void reset() {
		m_player = nullptr;
		m_spawn = 0.f;
		
		m_records.clear();
	}
};

class Aimbot {
private:
	struct target_t {
		Player* m_player;
		AimPlayer* m_data;
	};

public:
	std::array< AimPlayer, 64 > m_players;
	std::vector< AimPlayer* > m_targets;
	std::vector< int > m_scanned_targets;

	BackupRecord m_backup[64];

	std::vector<std::string> m_debug_render;

	// found target stuff.
	Player*    m_target;
	ang_t      m_angle;
	vec3_t     m_aim;
	float      m_damage;
	LagRecord* m_record;
	int        m_hitbox;
	int        m_hitgroup;

	// fake latency stuff.
	bool m_fake_latency;

	// autostop stuff.
	bool m_stop;
	bool m_stop_early;
	bool m_damage_override_toggle;

	int m_resolver_override_index;
	int m_resolver_override_side;

	// hitchance.
	std::vector<std::tuple<float, float, float>> precomputed_seeds;

public:
	__forceinline void reset() {
		// reset aimbot data.
		init();

		// reset all players data.
		for (auto& p : m_players)
			p.reset();
	}

	__forceinline bool IsValidTarget(Player* player) {
		if (!player)
			return false;

		if (!player->IsPlayer())
			return false;

		if (!player->alive())
			return false;

		if (player->m_bIsLocalPlayer())
			return false;

		if (!player->enemy(g_cl.m_local))
			return false;

		return true;
	}

	int HitboxToHitgroup(int hitbox) {
		switch (hitbox) {
		case HITBOX_HEAD:
		case HITBOX_NECK:
		case HITBOX_LOWER_NECK:
			return HITGROUP_HEAD;
		case HITBOX_UPPER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_THORAX:
		case HITBOX_L_UPPER_ARM:
		case HITBOX_R_UPPER_ARM:
			return HITGROUP_CHEST;
		case HITBOX_PELVIS:
		case HITBOX_L_THIGH:
		case HITBOX_R_THIGH:
		case HITBOX_BODY:
			return HITGROUP_STOMACH;
		case HITBOX_L_CALF:
		case HITBOX_L_FOOT:
			return HITGROUP_LEFTLEG;
		case HITBOX_R_CALF:
		case HITBOX_R_FOOT:
			return HITGROUP_RIGHTLEG;
		case HITBOX_L_FOREARM:
		case HITBOX_L_HAND:
			return HITGROUP_LEFTARM;
		case HITBOX_R_FOREARM:
		case HITBOX_R_HAND:
			return HITGROUP_RIGHTARM;
		default:
			return HITGROUP_MISS;
		}
	}



	std::string getHitboxName(int hitbox) {
		std::string hitboxNames[] = { "Head", "Head", "Chest", "Stomach", "Left Leg", "Right Leg", "Left Arm", "Right Arm", "unk1", "unk2", "unk3", "unk4", "unk5", "unk6" };

		if (hitbox >= 0 && hitbox <= 13) {
			return hitboxNames[hitbox];
		}
		else {
			return "Unknown Hitbox";
		}
	}




public:
	// misc aimbot funcs.
	void init();
	void UpdateShootpos(float pitch);
	bool AllowHitbox(Player* player, int hitbox, float damage);
	float GetMinDamage(float hp);
	void GetPeekData(AimPlayer* data, LagRecord* record);
	bool ShouldDelayShot(AimPlayer* data, LagRecord* record);
	bool CheckHitchance(Player* player, const ang_t& angle, LagRecord* lagrecord, int hitbox);

	// main fucs.
	void think();
	void GatherTargets(int& scans_this_tick);
	void find();

	bool CanHit(vec3_t start, vec3_t end, LagRecord* record, int box, bool write_matrix);
};

extern Aimbot g_aimbot;
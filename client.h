#pragma once

class NetPos {
public:
	float  m_time;
	vec3_t m_pos;

public:
	__forceinline NetPos() : m_time{}, m_pos{} {};
	__forceinline NetPos(float time, vec3_t pos) : m_time{ time }, m_pos{ pos } {};
};

struct ping_records {
	float lowest;
	float current;
};

class Client {
public:
	// hack thread.
	static ulong_t __stdcall init(void* arg);
	std::string GetClientName(int id);

	void StartMove(CUserCmd* cmd);
	void EndMove(CUserCmd* cmd);
	void BackupPlayers(bool restore);
	void DoMove();
	void DrawHUD();
	void UnlockHiddenConvars();
	void ClanTag();
	void Skybox();

	void SetAngles();
	void KillFeed();

	void OnPaint();
	void OnMapload();
	void OnTick(CUserCmd* cmd);

	// debugprint function.
	void print(const std::string text, ...);

	// check if we are able to fire this tick.
	bool CanFireWeapon();
	void UpdateRevolverCock();
	void MouseFix(CUserCmd* cmd);

public:
	struct incoming_seq_t {
		std::ptrdiff_t m_in_seq{}, m_in_rel_state{};
	};

	std::vector < incoming_seq_t > m_inc_seq{};

	std::array<ping_records, 64> m_ping_records;

	// local player variables.
	Player* m_local;
	bool	         m_processing;
	int	             m_flags;
	vec3_t	         m_shoot_pos;
	bool	         m_player_fire;
	bool	         m_shot;
	bool	         m_old_shot;
	float            m_abs_yaw;
	float			 m_left_thickness[64], m_right_thickness[64], m_at_target_angle[64];
	bool             m_pressing_move;
	bool             m_prev_pressing_move;

	// active weapon variables.
	Weapon* m_weapon;
	int         m_weapon_id;
	WeaponInfo* m_weapon_info;
	int         m_weapon_type;
	bool        m_weapon_fire;

	// revolver variables.
	int	 m_revolver_cock;
	int	 m_revolver_query;
	bool m_revolver_fire;

	// general game varaibles.
	bool     m_round_end;
	Stage_t	 m_stage;
	int	     m_max_lag;
	int      m_lag;
	int	     m_old_lag;
	bool* m_packet;
	bool* m_final_packet;
	bool	 m_old_packet;
	float	 m_lerp;
	float    m_latency;
	int      m_latency_ticks;
	int      m_server_tick;
	int      m_arrival_tick;
	int      m_width, m_height;

	// usercommand variables.
	CUserCmd* m_cmd;
	int	      m_tick;
	int	      m_rate;
	int	      m_buttons;
	int       m_old_buttons;
	ang_t     m_view_angles;
	ang_t	  m_strafe_angles;
	vec3_t	  m_forward_dir;

	penetration::PenetrationOutput_t m_pen_data;

	std::deque< NetPos >   m_net_pos;

	// animation variables.
	ang_t  m_angle;
	ang_t  m_radar;
	float  m_anim_time;
	float  m_anim_frame;
	bool   m_lagcomp;
	float  m_body_pred;
	float  m_body_lol;

	bool   m_hide_angles;
	ang_t  m_hidden_angles;

	bool   m_fuck_hltv;
	bool   m_need_to_send_voice_message;

	int    m_miracle_taps;

	bool   m_fake_ping_delayed;

	int m_ticks_to_stop;

	vec3_t m_vecbonepos[256];
	quaternion_t m_quatbonerot[256];

	vec3_t m_pre_autopeek_pos;
	matrix3x4_t m_pre_autopeek_bones[128];

	CSPlayerResource** m_resource;

	// hack username.
	int m_user_id;
	std::string m_user_name;

	Color m_copied_color;
};

extern Client g_cl;
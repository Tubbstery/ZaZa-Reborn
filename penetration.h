#pragma once

namespace penetration {
	struct PenetrationInput_t {
		Player* m_from;
		Player* m_target;
		vec3_t  m_pos;
		float	m_damage;
		bool	m_can_pen;
	};

	struct PenetrationOutput_t {
		Player* m_target;
		float   m_damage;
		int     m_hitgroup;
		bool    m_pen;

        __forceinline PenetrationOutput_t() : m_target{ nullptr }, m_damage{ 0.f }, m_hitgroup{ -1 }, m_pen{ false } {}
	};


	// util.
    float ScaleDamage( Player* player, float damage, float armor_ratio, int hitgroup );
    bool  TraceToExit( const vec3_t& start, const vec3_t& dir, vec3_t& out, CGameTrace* enter_trace, CGameTrace* exit_trace );
	void  ClipTraceToPlayer( const vec3_t& start, const vec3_t& end, uint32_t mask, CGameTrace* tr, Player* player, float min );

	// shit depracated legacy function for old penetration system ( is never going to get used ).
	void GetBulletTypeParameters(float& maxRange, float& maxDistance, const char* bulletType, bool sv_penetration_type);

	// main stuff.
	bool HandleBulletPenetration(WeaponInfo* weapondata, CGameTrace& enterTrace, vec3_t& eyePosition, vec3_t direction, int& possibleHitsRemaining, float& currentDamage, 
		float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, Player* from);
	bool FireBullet(Player* from, Player* target, WeaponInfo* weaponinfo, vec3_t shootpos, vec3_t& direction, PenetrationInput_t* input, PenetrationOutput_t* output);

	// usage.
    bool  run( PenetrationInput_t* input, PenetrationOutput_t* output );
}
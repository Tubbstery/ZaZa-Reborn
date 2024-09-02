#include "includes.h"

float penetration::ScaleDamage( Player* player, float damage, float armor_ratio, int hitgroup ) {
	bool hasHeavyArmor = player->m_bHasHeavyArmor();
	int armorValue = std::abs(player->m_ArmorValue()); // need to do abs because randomly returns a negative value??? causes awall to think they dont have armor.
	//g_cl.print(std::to_string(armorValue) + "\n");

	//Does the person have armor on for the hitbox checked?
	auto IsArmored = [&]()-> bool {
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return !!player->m_bHasHelmet(); //force-convert it to a bool via (!!)
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	switch (hitgroup) {
	case HITGROUP_HEAD:
		damage *= hasHeavyArmor ? 2.f : 4.f; //Heavy Armor does 1/2 damage
		break;
	case HITGROUP_STOMACH:
		damage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		damage *= 0.75f;
		break;
	default:
		break;
	}

	if (armorValue > 0 && IsArmored()) {
		float bonusValue = 1.f, armorBonusRatio = 0.5f, armorRatio = armor_ratio / 2.f;

		//Damage gets modified for heavy armor users
		if (hasHeavyArmor) {
			armorBonusRatio = 0.33f;
			armorRatio *= 0.5f;
			bonusValue = 0.33f;
		}

		auto NewDamage = damage * armorRatio;

		if (hasHeavyArmor)
			NewDamage *= 0.85f;

		if (((damage - (damage * armorRatio)) * (bonusValue * armorBonusRatio)) > armorValue)
			NewDamage = damage - (armorValue / armorBonusRatio);

		damage = NewDamage;
	}

	return std::floor( damage );
}

bool penetration::TraceToExit( const vec3_t &start, const vec3_t& dir, vec3_t& out, CGameTrace* enter_trace, CGameTrace* exit_trace ) {
    static CTraceFilterSimple_game filter{};

	float  dist{};
	vec3_t new_end;
	int    contents, first_contents{};

	// max pen distance is 90 units.
	while( dist <= 90.f ) {
		// step forward a bit.
		dist += 4.f;

		// set out pos.
		out = start + ( dir * dist );

		if( !first_contents )
			first_contents = g_csgo.m_engine_trace->GetPointContents( out, MASK_SHOT, nullptr );

		contents = g_csgo.m_engine_trace->GetPointContents( out, MASK_SHOT, nullptr );

		if( ( contents & MASK_SHOT_HULL ) && ( !( contents & CONTENTS_HITBOX ) || ( contents == first_contents ) ) )
			continue;

		// move end pos a bit for tracing.
		new_end = out - ( dir * 4.f );

		// do first trace aHR0cHM6Ly9zdGVhbWNvbW11bml0eS5jb20vaWQvc2ltcGxlcmVhbGlzdGlj.
		g_csgo.m_engine_trace->TraceRay( Ray( out, new_end ), MASK_SHOT, nullptr, exit_trace );

        // note - dex; this is some new stuff added sometime around late 2017 ( 10.31.2017 update? ).
        if( g_csgo.sv_clip_penetration_traces_to_players->GetInt( ) )
            game::UTIL_ClipTraceToPlayers( out, new_end, MASK_SHOT, nullptr, exit_trace, -60.f );

        // we hit an ent's hitbox, do another trace.
        if( exit_trace->m_startsolid && ( exit_trace->m_surface.m_flags & SURF_HITBOX ) ) {
			filter.SetPassEntity( exit_trace->m_entity );
        
			g_csgo.m_engine_trace->TraceRay( Ray( out, start ), MASK_SHOT_HULL, (ITraceFilter *)&filter, exit_trace );
        
			if( exit_trace->hit( ) && !exit_trace->m_startsolid ) {
                out = exit_trace->m_endpos;
                return true;
            }

            continue;
		}

        if( !exit_trace->hit( ) || exit_trace->m_startsolid ) {
            if( game::IsBreakable( enter_trace->m_entity ) ) {
                *exit_trace          = *enter_trace;
                exit_trace->m_endpos = start + dir;
            	return true;
            }

            continue;
        }

        if( ( exit_trace->m_surface.m_flags & SURF_NODRAW ) ) {
            // note - dex; ok, when this happens the game seems to not ignore world?
            if( game::IsBreakable( exit_trace->m_entity ) && game::IsBreakable( enter_trace->m_entity ) ) {
                out = exit_trace->m_endpos;
                return true;
            } 

            if( !( enter_trace->m_surface.m_flags & SURF_NODRAW ) )
                continue;
        }

        if( exit_trace->m_plane.m_normal.dot( dir ) <= 1.f ) {
            out -= ( dir * ( exit_trace->m_fraction * 4.f ) );
            return true;
        }
	}

	return false;
}

void penetration::ClipTraceToPlayer( const vec3_t& start, const vec3_t& end, uint32_t mask, CGameTrace* tr, Player* player, float min ) {
	vec3_t     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray        ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos         = player->m_vecOrigin( ) + ( ( player->m_vecMins( ) + player->m_vecMaxs( ) ) * 0.5f );
	to          = pos - start;
	dir         = start - end;
	len         = dir.normalize( );
	range_along = dir.dot( to );

	// off start point.
	if( range_along < 0.f )	
		range = -( to ).length( );

	// off end point.
	else if( range_along > len ) 
		range = -( pos - end ).length( );

	// within ray bounds.
	else {
		on_ray = start + ( dir * range_along );
		range  = ( pos - on_ray ).length( );
	}

	if( /*min <= range &&*/ range <= 60.f ) {
		// clip to player.
		g_csgo.m_engine_trace->ClipRayToEntity( Ray( start, end ), mask, player, &new_trace );

		if( tr->m_fraction > new_trace.m_fraction )
			*tr = new_trace;
	}
}

// sick function thanks UC.
void penetration::GetBulletTypeParameters(float& maxRange, float& maxDistance, const char* bulletType, bool sv_penetration_type) {
	if (sv_penetration_type) {
		maxRange = 35.0;
		maxDistance = 3000.0;
	}
	else {
		//Play tribune to framerate. Thanks, stringcompare
		//Regardless I doubt anyone will use the old penetration system anyway; so it won't matter much.
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_338MAG")))
		{
			maxRange = 45.0;
			maxDistance = 8000.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_762MM")))
		{
			maxRange = 39.0;
			maxDistance = 5000.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_556MM")) || !strcmp(bulletType, XOR("BULLET_PLAYER_556MM_SMALL")) || !strcmp(bulletType, XOR("BULLET_PLAYER_556MM_BOX")))
		{
			maxRange = 35.0;
			maxDistance = 4000.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_57MM")))
		{
			maxRange = 30.0;
			maxDistance = 2000.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_50AE")))
		{
			maxRange = 30.0;
			maxDistance = 1000.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_357SIG")) || !strcmp(bulletType, XOR("BULLET_PLAYER_357SIG_SMALL")) || !strcmp(bulletType, XOR("BULLET_PLAYER_357SIG_P250")) || !strcmp(bulletType, XOR("BULLET_PLAYER_357SIG_MIN")))
		{
			maxRange = 25.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_9MM")))
		{
			maxRange = 21.0;
			maxDistance = 800.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_45ACP")))
		{
			maxRange = 15.0;
			maxDistance = 500.0;
		}
		if (!strcmp(bulletType, XOR("BULLET_PLAYER_BUCKSHOT")))
		{
			maxRange = 0.0;
			maxDistance = 0.0;
		}
	}
}

bool penetration::HandleBulletPenetration(WeaponInfo* weapondata, CGameTrace& enterTrace, vec3_t& eyePosition, vec3_t direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, bool sv_penetration_type, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, Player* from) {
	CGameTrace exitTrace;
	Entity* pEnemy = (Player*)enterTrace.m_entity;
	surfacedata_t* enterSurfaceData = g_csgo.m_phys_props->GetSurfaceData(enterTrace.m_surface.m_surface_props);
	int enterMaterial = enterSurfaceData->m_game.m_material;

	float enterSurfPenetrationModifier = enterSurfaceData->m_game.m_penetration_modifier;
	float enterDamageModifier = enterSurfaceData->m_game.m_damage_modifier;
	float thickness, modifier, lostDamage, finalDamageModifier, combinedPenetrationModifier;
	bool isSolidSurf = ((enterTrace.m_contents >> 3) & CONTENTS_SOLID);
	bool isLightSurf = ((enterTrace.m_surface.m_flags >> 7) & SURF_LIGHT);

	vec3_t PISSOUT;
	if (possibleHitsRemaining <= 0
		//Test for "DE_CACHE/DE_CACHE_TELA_03" as the entering surface and "CS_ITALY/CR_MISCWOOD2B" as the exiting surface.
		//Fixes a wall in de_cache which seems to be broken in some way. Although bullet penetration is not recorded to go through this wall
		//Decals can be seen of bullets within the glass behind of the enemy. Hacky method, but works.
		//You might want to have a check for this to only be activated on de_cache.
		|| (enterTrace.m_surface.m_name == (const char*)0x2227c261 && exitTrace.m_surface.m_name == (const char*)0x2227c868)
		|| (!possibleHitsRemaining && !isLightSurf && !isSolidSurf && enterMaterial != CHAR_TEX_GRATE && enterMaterial != CHAR_TEX_GLASS)
		|| weapondata->m_penetration <= 0.f
		|| !TraceToExit(enterTrace.m_endpos, direction, PISSOUT, &enterTrace, &exitTrace)
		&& !(g_csgo.m_engine_trace->GetPointContents(enterTrace.m_endpos, MASK_SHOT_HULL, nullptr) & MASK_SHOT_HULL))
		return false;

	surfacedata_t* exitSurfaceData = g_csgo.m_phys_props->GetSurfaceData(exitTrace.m_surface.m_surface_props);
	int exitMaterial = exitSurfaceData->m_game.m_material;
	float exitSurfPenetrationModifier = exitSurfaceData->m_game.m_penetration_modifier;
	float exitDamageModifier = exitSurfaceData->m_game.m_damage_modifier;

	//Are we using the newer penetration system?
	if (sv_penetration_type)
	{
		if (enterMaterial == CHAR_TEX_GRATE || enterMaterial == CHAR_TEX_GLASS)
		{
			combinedPenetrationModifier = 3.f;
			finalDamageModifier = 0.05f;
		}
		else if (isSolidSurf || isLightSurf)
		{
			combinedPenetrationModifier = 1.f;
			finalDamageModifier = 0.16f;
		}
		else if (enterMaterial == CHAR_TEX_FLESH && (from->m_iTeamNum() == pEnemy->m_iTeamNum() && ff_damage_reduction_bullets == 0.f)) //TODO: Team check config
		{
			//Look's like you aren't shooting through your teammate today
			if (ff_damage_bullet_penetration == 0.f)
				return false;

			//Let's shoot through teammates and get kicked for teamdmg! Whatever, atleast we did damage to the enemy. I call that a win.
			combinedPenetrationModifier = ff_damage_bullet_penetration;
			finalDamageModifier = 0.16f;
		}
		else
		{
			combinedPenetrationModifier = (enterSurfPenetrationModifier + exitSurfPenetrationModifier) / 2.f;
			finalDamageModifier = 0.16f;
		}

		//Do our materials line up?
		if (enterMaterial == exitMaterial)
		{
			if (exitMaterial == CHAR_TEX_CARDBOARD || exitMaterial == CHAR_TEX_WOOD)
				combinedPenetrationModifier = 3.f;
			else if (exitMaterial == CHAR_TEX_PLASTIC)
				combinedPenetrationModifier = 2.f;
		}

		//Calculate thickness of the wall by getting the length of the range of the trace and squaring
		thickness = (exitTrace.m_endpos - enterTrace.m_endpos).length_sqr();
		modifier = fmaxf(1.f / combinedPenetrationModifier, 0.f);

		//This calculates how much damage we've lost depending on thickness of the wall, our penetration, damage, and the modifiers set earlier
		lostDamage = fmaxf(
			((modifier * thickness) / 24.f)
			+ ((currentDamage * finalDamageModifier)
				+ (fmaxf(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

		//Did we loose too much damage?
		if (lostDamage > currentDamage)
			return false;

		//We can't use any of the damage that we've lost
		if (lostDamage > 0.f)
			currentDamage -= lostDamage;

		//Do we still have enough damage to deal?
		if (currentDamage < 1.f)
			return false;

		eyePosition = exitTrace.m_endpos;
		--possibleHitsRemaining;

		return true;
	}
	else //Legacy penetration system
	{
		combinedPenetrationModifier = 1.f;

		if (isSolidSurf || isLightSurf)
			finalDamageModifier = 0.99f; //Good meme :^)
		else
		{
			finalDamageModifier = fminf(enterDamageModifier, exitDamageModifier);
			combinedPenetrationModifier = fminf(enterSurfPenetrationModifier, exitSurfPenetrationModifier);
		}

		if (enterMaterial == exitMaterial && (exitMaterial == CHAR_TEX_METAL || exitMaterial == CHAR_TEX_WOOD))
			combinedPenetrationModifier += combinedPenetrationModifier;

		thickness = (exitTrace.m_endpos - enterTrace.m_endpos).length_sqr();

		if (sqrt(thickness) <= combinedPenetrationModifier * penetrationPower)
		{
			currentDamage *= finalDamageModifier;
			eyePosition = exitTrace.m_endpos;
			--possibleHitsRemaining;

			return true;
		}

		return false;
	}
}

bool penetration::FireBullet(Player* from, Player* target, WeaponInfo* weaponinfo, vec3_t shootpos, vec3_t& direction, PenetrationInput_t* input, PenetrationOutput_t* output) {
	if (!from)
		return false;

	bool sv_penetration_type;
	//	  Current bullet travel Power to penetrate Distance to penetrate Range               Player bullet reduction convars			  Amount to extend ray by
	float currentDistance = 0.f, penetrationPower, penetrationDistance, maxRange, ff_damage_reduction_bullets, ff_damage_bullet_penetration, rayExtension = 40.f, tempdamage;

	//For being superiour when the server owners think your autowall isn't well reversed. Imagine a meme HvH server with the old penetration system- pff
	static ConVar* penetrationSystem = g_csgo.m_cvar->FindVar(HASH("sv_penetration_type"));
	static ConVar* damageReductionBullets = g_csgo.m_cvar->FindVar(HASH("ff_damage_reduction_bullets"));
	static ConVar* damageBulletPenetration = g_csgo.m_cvar->FindVar(HASH("ff_damage_bullet_penetration"));

	sv_penetration_type = penetrationSystem->GetInt();
	ff_damage_reduction_bullets = damageReductionBullets->GetFloat();
	ff_damage_bullet_penetration = damageBulletPenetration->GetFloat();

	CGameTrace enterTrace;

	//We should be skipping localplayer when casting a ray to players.
	CTraceFilter filter;
	filter.skip_entity = from;

	maxRange = weaponinfo->m_range;

	// gotta get this workin
	GetBulletTypeParameters(penetrationPower, penetrationDistance, weaponinfo->m_ammo_type, sv_penetration_type);

	if (sv_penetration_type)
		penetrationPower = weaponinfo->m_penetration;

	//This gets set in FX_Firebullets to 4 as a pass-through value.
	//CS:GO has a maximum of 4 surfaces a bullet can pass-through before it 100% stops.
	//Excerpt from Valve: https://steamcommunity.com/sharedfiles/filedetails/?id=275573090
	//"The total number of surfaces any bullet can penetrate in a single flight is capped at 4." -CS:GO Official
	int possibleHitsRemaining = 4;

	//Set our current damage to what our gun's initial damage reports it will do
	tempdamage = weaponinfo->m_damage;

	//If our damage is greater than (or equal to) 1, and we can shoot, let's shoot.
	while (possibleHitsRemaining > 0 && tempdamage >= 1.f)
	{
		//Calculate max bullet range
		maxRange -= currentDistance;

		//Create endpoint of bullet
		vec3_t end = shootpos + direction * maxRange;

		g_csgo.m_engine_trace->TraceRay(Ray(shootpos, end), MASK_SHOT_HULL | CONTENTS_HITBOX, (ITraceFilter*)&filter, &enterTrace);

		game::UTIL_ClipTraceToPlayers(shootpos, end + direction * rayExtension, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace, -60.f);

		//We have to do this *after* tracing to the player.
		surfacedata_t* enterSurfaceData = g_csgo.m_phys_props->GetSurfaceData(enterTrace.m_surface.m_surface_props);
		float enterSurfPenetrationModifier = enterSurfaceData->m_game.m_penetration_modifier;
		int enterMaterial = enterSurfaceData->m_game.m_material;

		//"Fraction == 1" means that we didn't hit anything. We don't want that- so let's break on it.
		if (enterTrace.m_fraction == 1.f)
			break;

		//calculate the damage based on the distance the bullet traveled.
		currentDistance += enterTrace.m_fraction * maxRange;

		//Let's make our damage drops off the further away the bullet is.
		tempdamage *= pow(weaponinfo->m_range_modifier, (currentDistance / 500.f));

		//Sanity checking / Can we actually shoot through?
		if (currentDistance > penetrationDistance && weaponinfo->m_penetration > 0.f || enterSurfPenetrationModifier < 0.1f)
			break;

		bool canDoDamage = (enterTrace.m_hitgroup != HITGROUP_GEAR && enterTrace.m_hitgroup != HITGROUP_GENERIC);
		bool isPlayer = ((enterTrace.m_entity)->IsPlayer()); //((enterTrace.m_entity)->GetClientClass()->m_ClassID == ClassId_CCSPlayer);
		bool isEnemy = from->enemy((Player*)enterTrace.m_entity);
		bool onTeam = (((Player*)enterTrace.m_entity)->m_iTeamNum() == TEAM_COUNTERTERRORISTS || ((Player*)enterTrace.m_entity)->m_iTeamNum() == TEAM_TERRORISTS);

		//TODO: Team check config
		if ((canDoDamage && isPlayer && isEnemy) && onTeam)
		{
			output->m_target = (Player*)enterTrace.m_entity;
			output->m_damage = ScaleDamage((Player*)enterTrace.m_entity, tempdamage, weaponinfo->m_armor_ratio, enterTrace.m_hitgroup);
			output->m_hitgroup = enterTrace.m_hitgroup;
			output->m_pen = possibleHitsRemaining != 4;
			return true;
		}

		//Calling HandleBulletPenetration here reduces our penetrationCounter, and if it returns true, we can't shoot through it.
		if (!HandleBulletPenetration(weaponinfo, enterTrace, shootpos, direction, possibleHitsRemaining, tempdamage, penetrationPower, sv_penetration_type, ff_damage_reduction_bullets, ff_damage_bullet_penetration, from))
			break;
	}

	return false;
}

bool penetration::run(PenetrationInput_t* input, PenetrationOutput_t* output) {
	vec3_t		start, dir;
	Weapon*     weapon;
	WeaponInfo* weapon_info;

	// if we are tracing from our local player perspective.
	if (input->m_from->m_bIsLocalPlayer()) {
		weapon = g_cl.m_weapon;
		weapon_info = g_cl.m_weapon_info;
		start = g_cl.m_shoot_pos;
	}
	// not local player.
	else {
		weapon = input->m_from->GetActiveWeapon();
		if (!weapon)
			return false;

		// get weapon info.
		weapon_info = weapon->GetWpnData();
		if (!weapon_info)
			return false;

		// set trace start.
		start = input->m_from->GetShootPosition();
	}

	// get direction to end point.
	dir = (input->m_pos - start).normalized();

	// run awall.
	FireBullet(input->m_from, input->m_target, weapon_info, start, dir, input, output);

	// check that it is over our minimum damage.
	return output->m_damage >= input->m_damage;
}
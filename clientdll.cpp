#include "includes.h"

void Hooks::LevelInitPreEntity( const char* map ) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue( rate );
	g_csgo.cl_cmdrate->SetValue( rate );

	g_aimbot.reset( );
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >( CHLClient::LEVELINITPREENTITY )( this, map );
}

void Hooks::LevelInitPostEntity( ) {
	g_cl.OnMapload( );

	// from L3D.
	/*g_cl.m_latency = 0.f;
	g_cl.m_lerp = 0.f;
	g_shots.m_shots.clear();
	g_hooks.m_net_channel.reset();
	g_hooks.m_client_state.reset();*/

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >( CHLClient::LEVELINITPOSTENTITY )( this );
}

void Hooks::LevelShutdown( ) {
	g_aimbot.reset( );

	// from L3D.
	/*
	g_cl.m_latency = 0.f;
	g_cl.m_lerp = 0.f;
	g_shots.m_shots.clear();
	g_hooks.m_net_channel.reset();
	g_hooks.m_client_state.reset();*/

	g_cl.m_local       = nullptr;
	g_cl.m_weapon      = nullptr;
	g_cl.m_processing  = false;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end   = false;

	g_cl.m_inc_seq.clear();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >( CHLClient::LEVELSHUTDOWN )( this );
}

/*int Hooks::IN_KeyEvent( int evt, int key, const char* bind ) {
	// see if this key event was fired for the drop bind.
	/*if( bind && FNV1a::get( bind ) == HASH( "drop" ) ) {
		// down.
		if( evt ) {
			g_cl.m_drop = true;
			g_cl.m_drop_query = 2;
			g_cl.print( "drop\n" );
		}

		// up.
		else 
			g_cl.m_drop = false;

		// ignore the event.
		return 0;
	}

	return g_hooks.m_client.GetOldMethod< IN_KeyEvent_t >( CHLClient::INKEYEVENT )( this, evt, key, bind );
}*/

void Hooks::FrameStageNotify( Stage_t stage ) {

	// save stage.
	if( stage != FRAME_START )
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	if( stage == FRAME_RENDER_START ) {	
		// apply local player animated angles.
		g_cl.SetAngles( );

		// process shots.
		g_shots.OnFSN();

        // draw our custom beams.
        g_visuals.DrawBeams( );
	}

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >( CHLClient::FRAMESTAGENOTIFY )( this, stage );

	if( stage == FRAME_RENDER_START ) {
		// ...
	}

	else if( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START ) {
		if (g_csgo.m_cvar->FindVar(HASH("cl_extrapolate"))->GetInt() != 0)
			g_csgo.m_cvar->FindVar(HASH("cl_extrapolate"))->SetValue(0);

		if (g_csgo.m_cvar->FindVar(HASH("cl_csm_shadows"))->GetInt() == g_menu.main.visuals.vis_removals.get(5))
			g_csgo.m_cvar->FindVar(HASH("cl_csm_shadows"))->SetValue(!g_menu.main.visuals.vis_removals.get(5));

		if (g_csgo.m_cvar->FindVar(HASH("mat_postprocess_enable"))->GetInt() == g_menu.main.visuals.vis_removals.get(6))
			g_csgo.m_cvar->FindVar(HASH("mat_postprocess_enable"))->SetValue(!g_menu.main.visuals.vis_removals.get(6));

		g_cl.Skybox();
		g_cl.ClanTag();
		g_skins.think( );
	}

	else if( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END ) {
		g_visuals.NoSmoke( );
	}

	else if( stage == FRAME_NET_UPDATE_END ) {
		// update all players.
		g_animationsystem.FrameStage();

		// update lagrecords.
		g_lagcomp.Update();

		// update simulated local lby timers.
		//g_cl.SimulatedLBYTimer();
	}
}
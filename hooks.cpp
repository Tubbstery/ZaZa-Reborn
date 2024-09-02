#include "includes.h"

Hooks                g_hooks{ };;
CustomEntityListener g_custom_entity_listener{ };;

void Pitch_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// normalize this fucker.
	math::NormalizeAngle( data->m_Value.m_Float );

	// clamp to remove retardedness.
	math::clamp( data->m_Value.m_Float, -90.f, 90.f );

	// call original netvar proxy.
	if ( g_hooks.m_Pitch_original )
		g_hooks.m_Pitch_original( data, ptr, out );
}

void Body_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	Stack stack;

	static Address RecvTable_Decode{ pattern::find( g_csgo.m_engine_dll, XOR( "EB 0D FF 77 10" ) ) };

	// call from entity going into pvs.
	if ( stack.next( ).next( ).ReturnAddress( ) != RecvTable_Decode ) {
		// convert to player.
		Player *player = ptr.as< Player * >( );

		// store data about the update.
		g_resolver.OnBodyUpdate( player, data->m_Value.m_Float );
	}

	// call original proxy.
	if ( g_hooks.m_Body_original )
		g_hooks.m_Body_original( data, ptr, out );
}

void AbsYaw_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// convert to ragdoll.
	//Ragdoll* ragdoll = ptr.as< Ragdoll* >( );

	// get ragdoll owner.
	//Player* player = ragdoll->GetPlayer( );

	// get data for this player.
	/*AimPlayer* aim = &g_aimbot.m_players[ player->index( ) - 1 ];

	if( player && aim ) {
	if( !aim->m_records.empty( ) ) {
	LagRecord* match{ nullptr };

	// iterate records.
	for( const auto &it : aim->m_records ) {
	// find record that matches with simulation time.
	if( it->m_sim_time == player->m_flSimulationTime( ) ) {
	match = it.get( );
	break;
	}
	}

	// we have a match.
	// and it is standing
	// TODO; add air?
	if( match /*&& match->m_mode == Resolver::Modes::RESOLVE_STAND*/// ) {
	/*	RagdollRecord record;
	record.m_record   = match;
	record.m_rotation = math::NormalizedAngle( data->m_Value.m_Float );
	record.m_delta    = math::NormalizedAngle( record.m_rotation - match->m_lbyt );

	float death = math::NormalizedAngle( ragdoll->m_flDeathYaw( ) );

	// store.
	//aim->m_ragdoll.push_front( record );

	//g_cl.print( tfm::format( XOR( "rot %f death %f delta %f\n" ), record.m_rotation, death, record.m_delta ).data( ) );
	}
	}*/
	//}

	// call original netvar proxy.
	if ( g_hooks.m_AbsYaw_original )
		g_hooks.m_AbsYaw_original( data, ptr, out );
}

void Force_proxy( CRecvProxyData *data, Address ptr, Address out ) {
	// convert to ragdoll.
	Ragdoll *ragdoll = ptr.as< Ragdoll * >( );

	// get ragdoll owner.
	Player *player = ragdoll->GetPlayer( );

	// we only want this happening to noobs we kill.
	if ( g_menu.main.misc.ragdoll_force.get( ) && g_cl.m_local && player && player->enemy( g_cl.m_local ) ) {
		// get m_vecForce.
		vec3_t vel = { data->m_Value.m_Vector[ 0 ], data->m_Value.m_Vector[ 1 ], data->m_Value.m_Vector[ 2 ] };

		// give some speed to all directions.
		vel *= 1000.f;

		// boost z up a bit.
		if ( vel.z <= 1.f )
			vel.z = 2.f;

		vel.z *= 2.f;

		// don't want crazy values for this... probably unlikely though?
		math::clamp( vel.x, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.y, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );
		math::clamp( vel.z, std::numeric_limits< float >::lowest( ), std::numeric_limits< float >::max( ) );

		// set new velocity.
		data->m_Value.m_Vector[ 0 ] = vel.x;
		data->m_Value.m_Vector[ 1 ] = vel.y;
		data->m_Value.m_Vector[ 2 ] = vel.z;
	}

	if ( g_hooks.m_Force_original )
		g_hooks.m_Force_original( data, ptr, out );
}

DECLSPEC_NOINLINE bool __fastcall SetupBones(void* ECX, void* EDX, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) {
	Player* player = (Player*)(uintptr_t(ECX) - 4);

	if (player->index() > 64 || player->index() < 0)
		return ((Hooks::SetupBones_t)g_hooks.m_SetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

	if (g_csgo.m_allow_setupbones[player->index() - 1]) {
		auto og = ((Hooks::SetupBones_t)g_hooks.m_SetupBones)(ECX, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		for (int i = 0; i < player->m_AnimOverlayCount(); ++i) {
			auto& curLayer = player->m_AnimOverlay()[i];

			if (player != curLayer.m_owner)
				curLayer.m_owner = player;
		}

		return og;
	}

	if (pBoneToWorldOut) {
		if (nMaxBones > 0)
			std::memcpy(pBoneToWorldOut, player->m_BoneCache().m_pCachedBones, sizeof(matrix3x4_t) * nMaxBones);
	}

	return true;
} 

DECLSPEC_NOINLINE void __fastcall InterpolateServerEntities() {
	// call original to interpolate players.
	((Hooks::InterpolateServerEntities_t)g_hooks.m_InterpolateServerEntities)();

	// fix local origin.
	if (g_cl.m_local && g_cl.m_processing) {
		g_cl.m_local->SetAbsAngles(ang_t(0.f, g_cl.m_abs_yaw, 0.f));

		matrix3x4_t matWorldMatrix{ };
		math::AngleMatrix(ang_t(0.f, g_cl.m_abs_yaw, 0.f), g_cl.m_local->GetAbsOrigin(), matWorldMatrix);

		auto pStudioHdr = g_cl.m_local->m_studioHdr();
		if (pStudioHdr) {
			uint8_t uBoneComputed[0x20] = { 0 };
			g_cl.m_local->BuildTransformations(pStudioHdr, g_cl.m_vecbonepos, g_cl.m_quatbonerot, matWorldMatrix, BONE_USED_BY_ANYTHING, uBoneComputed);

			auto pBackupBones = g_cl.m_local->m_BoneAccessor().m_pBones;

			g_cl.m_local->InvalidateBoneCache();
			g_cl.m_local->m_BoneAccessor().m_pBones = g_cl.m_local->m_BoneCache().m_pCachedBones;

			using AttachmentHelper_t = void(__thiscall*)(Player*, CStudioHdr*);
			static AttachmentHelper_t AttachmentHelperFn = (AttachmentHelper_t)g_csgo.m_AttachmentHelper;
			AttachmentHelperFn(g_cl.m_local, pStudioHdr);

			g_cl.m_local->m_BoneAccessor().m_pBones = pBackupBones;
		}
	}

	// build visual matrices with interpolated data.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player || !player->alive() || player->m_bIsLocalPlayer())
			continue;

		// build visual matrix.
		player->SetupBones(nullptr, 128, BONE_USED_BY_ANYTHING, player->m_flSimulationTime());
	}
}

DECLSPEC_NOINLINE void __fastcall ModifyEyePosition(CCSGOPlayerAnimState* ecx, void* edx, vec3_t& pos) {
	return;
}

DECLSPEC_NOINLINE bool __fastcall ShouldSkipAnimationFrame(void* ecx, uint32_t*) {
	return false;
}


using FnListInLeavesBox = int(__fastcall*)(const std::uintptr_t, const std::uintptr_t, const vec3_t&, const vec3_t&, const uint16_t* const, const int);
FnListInLeavesBox oListInLeavesBox;

int __fastcall list_leaves_in_box(const std::uintptr_t ecx, const std::uintptr_t edx, const vec3_t& mins, const vec3_t& maxs, const uint16_t* const list, const int max) {

	if (!g_cl.m_local)
		return oListInLeavesBox(ecx, edx, mins, maxs, list, max);

	if (*(uint32_t*)_ReturnAddress() != 0x8B087D8B)
		return oListInLeavesBox(ecx, edx, mins, maxs, list, max);

	struct renderable_info_t {
		IClientRenderable* m_renderable{};
		std::uintptr_t	m_alpha_property{};
		int				m_enum_count{};
		int				m_render_frame{};
		std::uint16_t	m_first_shadow{};
		std::uint16_t	m_leaf_list{};
		short			m_area{};
		std::uint16_t	m_flags0{};
		std::uint16_t	m_flags1{};
		vec3_t			m_bloated_abs_min{};
		vec3_t			m_bloated_abs_max{};
		vec3_t			m_abs_min{};
		vec3_t			m_abs_max{};
		char			pad0[4u]{};
	};

	const auto info = *reinterpret_cast<renderable_info_t**>(
		reinterpret_cast<std::uintptr_t>(_AddressOfReturnAddress()) + 0x14u
		);
	if (!info
		|| !info->m_renderable)
		return oListInLeavesBox(ecx, edx, mins, maxs, list, max);

	const auto entity = info->m_renderable->GetIClientUnknown()->GetBaseEntity();
	if (!entity)
		return oListInLeavesBox(ecx, edx, mins, maxs, list, max);

	info->m_flags0 &= ~0x100;
	info->m_flags1 |= 0xC0;

	return oListInLeavesBox(ecx, edx, { -16384.f, -16384.f, -16384.f }, { 16384.f, 16384.f, 16384.f }, list, max);
}



DECLSPEC_NOINLINE bool __fastcall SendNetMsg(INetChannel* pNetChan, void* edx, INetMessage& msg, bool bForceReliable, bool bVoice) {
	static const auto og = ((Hooks::SendNetMsg_t)g_hooks.m_SendNetMsg);

	if (g_csgo.m_engine->GetNetChannelInfo() != pNetChan)
		return og(pNetChan, msg, bForceReliable, bVoice);

	// i think this is sv_pure bypass lol, #pasted.
	if (msg.GetType() == 14) // Return and don't send messsage if its FileCRCCheck
		return false;

	// idk wtf this is but its the one we need lol #pasted.
	if (msg.GetGroup() == 11 && g_cl.m_need_to_send_voice_message) {
		struct lame_string_t {
			char data[16]{};
			uint32_t current_len = 0;
			uint32_t max_len = 15;
		};

		// here we are setting the data about the packet we will send.
		goofyhook_voice_new packet;
		strcpy(packet.cheat_name, XOR("goofy"));
		strcpy(packet.whitelist_xor_key, g_menu.main.aimbot.enable_goofy_whitelist.get() ? XOR("goofy") : XOR(" "));
		packet.user_id = g_cl.m_user_id;

		// copying the data to an actual data struct thingy.
		VoiceDataCustom newdata;
		memcpy(newdata.get_raw_data(), &packet, sizeof(packet));

		// making the message??? idk.
		CCLCMsg_VoiceData_Legacy msg;
		memset(&msg, 0, sizeof(msg));

		static const DWORD m_construct_voice_message = (DWORD)pattern::find(g_csgo.m_engine_dll, "56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7");
		static const auto fnConstructVoiceMessage = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
		fnConstructVoiceMessage((void*)&msg, nullptr);

		// filling the message with the data we want.
		msg.set_data(&newdata);

		// dafuk.
		lame_string_t lame_string;

		// yea idk about this lol.
		msg.data = &lame_string;
		msg.format = 0; // VoiceFormat_Steam
		msg.flags = 63; // all flags!

		// send dis bitch.
		og(pNetChan, (INetMessage&)msg, false, true);
		
		// make sure we dont send this again until its needed.
		// LOL SPAM THIS BITCH.
		//g_cl.m_need_to_send_voice_message = false;
	}
	// VOICE CHAT WOOO.
	else if (msg.GetGroup() == 9) {
		bVoice = true; // fix vc with fakelag?
	}

	return og(pNetChan, msg, bForceReliable, bVoice);
}

void Hooks::init() {
	// hook wndproc.
	auto m_hWindow = FindWindowA(XOR("Valve001"), NULL);
	m_old_wndproc = (WNDPROC)g_winapi.SetWindowLongA(m_hWindow, GWL_WNDPROC, util::force_cast<LONG>(Hooks::WndProc));

	// setup normal VMT hooks.
	m_panel.init(g_csgo.m_panel);
	m_panel.add(IPanel::PAINTTRAVERSE, util::force_cast(&Hooks::PaintTraverse));

	m_client.init(g_csgo.m_client);
	m_client.add(CHLClient::LEVELINITPREENTITY, util::force_cast(&Hooks::LevelInitPreEntity));
	m_client.add(CHLClient::LEVELINITPOSTENTITY, util::force_cast(&Hooks::LevelInitPostEntity));
	m_client.add(CHLClient::LEVELSHUTDOWN, util::force_cast(&Hooks::LevelShutdown));
	m_client.add(CHLClient::FRAMESTAGENOTIFY, util::force_cast(&Hooks::FrameStageNotify));

	m_engine.init(g_csgo.m_engine);
	m_engine.add(IVEngineClient::ISCONNECTED, util::force_cast(&Hooks::IsConnected));
	m_engine.add(IVEngineClient::ISHLTV, util::force_cast(&Hooks::IsHLTV));

	m_prediction.init(g_csgo.m_prediction);
	m_prediction.add(CPrediction::INPREDICTION, util::force_cast(&Hooks::InPrediction));
	m_prediction.add(CPrediction::RUNCOMMAND, util::force_cast(&Hooks::RunCommand));

	m_client_mode.init(g_csgo.m_client_mode);
	m_client_mode.add(IClientMode::SHOULDDRAWPARTICLES, util::force_cast(&Hooks::ShouldDrawParticles));
	m_client_mode.add(IClientMode::SHOULDDRAWFOG, util::force_cast(&Hooks::ShouldDrawFog));
	m_client_mode.add(IClientMode::OVERRIDEVIEW, util::force_cast(&Hooks::OverrideView));
	m_client_mode.add(IClientMode::CREATEMOVE, util::force_cast(&Hooks::CreateMove));
	m_client_mode.add(IClientMode::DOPOSTSPACESCREENEFFECTS, util::force_cast(&Hooks::DoPostScreenSpaceEffects));

	m_surface.init(g_csgo.m_surface);
	m_surface.add(ISurface::LOCKCURSOR, util::force_cast(&Hooks::LockCursor));
	m_surface.add(ISurface::PLAYSOUND, util::force_cast(&Hooks::PlaySound));
	m_surface.add(ISurface::ONSCREENSIZECHANGED, util::force_cast(&Hooks::OnScreenSizeChanged));

	m_model_render.init(g_csgo.m_model_render);
	m_model_render.add(IVModelRender::DRAWMODELEXECUTE, util::force_cast(&Hooks::DrawModelExecute));

	m_render_view.init(g_csgo.m_render_view);

	m_shadow_mgr.init(g_csgo.m_shadow_mgr);
	m_shadow_mgr.add(IClientShadowMgr::COMPUTESHADOWDEPTHTEXTURES, util::force_cast(&Hooks::ComputeShadowDepthTextures));

	m_view_render.init(g_csgo.m_view_render);
	m_view_render.add(CViewRender::ONRENDERSTART, util::force_cast(&Hooks::OnRenderStart));
	m_view_render.add(CViewRender::RENDERVIEW, util::force_cast(&Hooks::RenderView));
	m_view_render.add(CViewRender::RENDER2DEFFECTSPOSTHUD, util::force_cast(&Hooks::Render2DEffectsPostHUD));
	m_view_render.add(CViewRender::RENDERSMOKEOVERLAY, util::force_cast(&Hooks::RenderSmokeOverlay));

	m_match_framework.init(g_csgo.m_match_framework);
	m_match_framework.add(CMatchFramework::GETMATCHSESSION, util::force_cast(&Hooks::GetMatchSession));

	m_material_system.init(g_csgo.m_material_system);
	m_material_system.add(IMaterialSystem::OVERRIDECONFIG, util::force_cast(&Hooks::OverrideConfig));

	m_fire_bullets.init(g_csgo.TEFireBullets);
	m_fire_bullets.add(7, util::force_cast(&Hooks::PostDataUpdate));

	// this is what crashes default supremacy.
	m_client_state.init(g_csgo.m_hookable_cl);
	m_client_state.add(CClientState::TEMPENTITIES, util::force_cast(&Hooks::TempEntities));
	m_client_state.add(24, util::force_cast(&Hooks::VoiceData));

	auto setupBones = (DWORD)pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F0 B8 D8"));
	m_SetupBones = (DWORD)DetourFunction((byte*)setupBones, (byte*)SetupBones);

	auto interpolateServerEntities = (DWORD)pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56"));
	m_InterpolateServerEntities = (DWORD)DetourFunction((byte*)interpolateServerEntities, (byte*)InterpolateServerEntities);

	auto modifyEyePos = (DWORD)pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60 00 "));
	(DWORD)DetourFunction((byte*)modifyEyePos, (byte*)ModifyEyePosition);

	auto shouldSkipAnimFrame = (DWORD)pattern::find(g_csgo.m_client_dll, XOR("E8 ? ? ? ? 88 44 24 0B")).rel32(1);
	(DWORD)DetourFunction((byte*)shouldSkipAnimFrame, (byte*)ShouldSkipAnimationFrame);

	auto sendNetMsg = (DWORD)pattern::find(g_csgo.m_engine_dll, XOR("55 8B EC 56 8B F1 8B 86 ? ? ? ? 85 C0 74 24 48 83 F8 02 77 2C 83 BE ? ? ? ? ? 8D 8E ? ? ? ? 74 06 32 C0 84 C0 EB 10 E8 ? ? ? ? 84 C0 EB 07 83 BE ? ? ? ? ? 0F 94 C0 84 C0 74 07 B0 01 5E 5D C2 0C 00"));
	m_SendNetMsg = (DWORD)DetourFunction((byte*)sendNetMsg, (byte*)SendNetMsg);

	//auto dwListLeavesInBox = (void*)util::GetVFunc(g_csgo.m_engine->GetBSPTreeQuery(), 6);
	//(DWORD)DetourFunction((byte*)dwListLeavesInBox, (byte*)list_leaves_in_box);

	g_custom_entity_listener.init();

	// cvar hooks.
	m_debug_spread.init(g_csgo.weapon_debug_spread_show);
	m_debug_spread.add(ConVar::GETINT, util::force_cast(&Hooks::DebugSpreadGetInt));

	//m_net_show_fragments.init(g_csgo.net_showfragments);
	//m_net_show_fragments.add(ConVar::GETBOOL, util::force_cast(&Hooks::NetShowFragmentsGetBool)); shouldnt be needed anymore.

	// set netvar proxies.
	g_netvars.SetProxy(HASH("DT_CSPlayer"), HASH("m_angEyeAngles[0]"), Pitch_proxy, m_Pitch_original);
	g_netvars.SetProxy(HASH("DT_CSPlayer"), HASH("m_flLowerBodyYawTarget"), Body_proxy, m_Body_original);
	g_netvars.SetProxy(HASH("DT_CSRagdoll"), HASH("m_vecForce"), Force_proxy, m_Force_original);
	g_netvars.SetProxy(HASH("DT_CSRagdoll"), HASH("m_flAbsYaw"), AbsYaw_proxy, m_AbsYaw_original);
}
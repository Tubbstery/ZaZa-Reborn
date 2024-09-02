#include "includes.h"

bool Hooks::TempEntities( void *msg ) {
	if( !g_cl.m_processing ) {
		return g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );
	}

	const bool ret = g_hooks.m_client_state.GetOldMethod< TempEntities_t >( CClientState::TEMPENTITIES )( this, msg );

	CEventInfo *ei = g_csgo.m_cl->m_events; 
	CEventInfo *next = nullptr;

	if( !ei ) {
		return ret;
	}

	do {
		next = *reinterpret_cast< CEventInfo ** >( reinterpret_cast< uintptr_t >( ei ) + 0x38 );

		uint16_t classID = ei->m_class_id - 1;

		auto m_pCreateEventFn = ei->m_client_class->m_pCreateEvent;
		if( !m_pCreateEventFn ) {
			continue;
		}

		void *pCE = m_pCreateEventFn( );
		if( !pCE ) {
			continue;
		}

		if( classID == 170 ){
			ei->m_fire_delay = 0.0f;
		}
		ei = next;
	} while( next != nullptr );

	return ret;
}

void Hooks::VoiceData(void* msg) {
	static const auto og = g_hooks.m_client_state.GetOldMethod<VoiceData_t>(24);

	// wtf let the og function handle dis.
	if (!msg || !g_csgo.m_engine->IsConnected())
		og(this, msg);

	// WADAFAK.
	CSVCMsg_VoiceData_Legacy* message = (CSVCMsg_VoiceData_Legacy*)msg;
	if (!message || message->format != 0)
		return og(this, msg);

	// get pasted data thingy lol idek what this does.
	VoiceDataCustom message_data = message->get_data();

	// check if its empty
	if (message_data.section_number == 0 && message_data.sequence_bytes == 0 && message_data.uncompressed_sample_offset == 0)
		return og(this, msg);

	// the "client" thing we recieve is 1 behind player index.
	int index = message->client + 1;

	Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(index);
	if (!player)
		return og(this, msg);

	if (!player->IsPlayer() || player->m_bIsLocalPlayer())
		return og(this, msg);

	AimPlayer* data = &g_aimbot.m_players[index - 1];
	if (!data)
		return og(this, msg);

	data->m_voice_cheat = CHEAT_UNKNOWN;

	goofyhook_voice_new* new_packet = (goofyhook_voice_new*)message_data.get_raw_data();
	if (!strcmp(new_packet->cheat_name, XOR("goofy"))) {
		data->m_voice_cheat = CHEAT_GOOFYHOOK;
		data->m_goofy_whitelist = false;
		data->m_goofy_id = new_packet->user_id;

		data->m_goofy_voice_messages.push_front(new_packet->whitelist_xor_key);
		while (data->m_goofy_voice_messages.size() > 8) {
			data->m_goofy_voice_messages.pop_back();
		}

		for (std::string &str : data->m_goofy_voice_messages) {
			if (!strcmp(str.c_str(), XOR("goofy"))) {
				data->m_goofy_whitelist = true;
				break;
			}
		}

		//g_cl.print(g_cl.GetClientName(data->m_miracle_id) + "\n");
	}
	else {
		goofyhook_voice_old* packet = (goofyhook_voice_old*)message_data.get_raw_data();
		if (!strcmp(packet->cheat_name, XOR("goofy"))) {
			data->m_voice_cheat = CHEAT_GOOFYHOOK_OLD;
		}
	}

	// make sure to call og.
	og(this, msg);
}
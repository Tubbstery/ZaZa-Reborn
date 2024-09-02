#pragma once

// bruh.
struct goofyhook_voice_new {
	char cheat_name[5];
	char whitelist_xor_key[6];
	int user_id;
};
struct goofyhook_voice_old {
	char cheat_name[5];
	char whitelist_xor_key[6];
};

struct VoiceDataCustom {
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t sequence_bytes{};
	uint32_t section_number{};
	uint32_t uncompressed_sample_offset{};

	__forceinline uint8_t* get_raw_data() {
		return (uint8_t*)this;
	}
};

struct CSVCMsg_VoiceData_Legacy {
	char pad_0000[8]; //0x0000
	int32_t client; //0x0008
	int32_t audible_mask; //0x000C
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	void* voide_data_; //0x0018
	int32_t proximity; //0x001C
	//int32_t caster; //0x0020
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C

	__forceinline VoiceDataCustom get_data() {
		VoiceDataCustom cdata;
		cdata.xuid_low = xuid_low;
		cdata.xuid_high = xuid_high;
		cdata.sequence_bytes = sequence_bytes;
		cdata.section_number = section_number;
		cdata.uncompressed_sample_offset = uncompressed_sample_offset;
		return cdata;
	}
};

struct CCLCMsg_VoiceData_Legacy {
	uint32_t INetMessage_Vtable; //0x0000
	char pad_0004[4]; //0x0004
	uint32_t CCLCMsg_VoiceData_Vtable; //0x0008
	char pad_000C[8]; //0x000C
	void* data; //0x0014
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C
	int32_t cached_size; //0x0030

	uint32_t flags; //0x0034

	uint8_t no_stack_overflow[0xFF];

	__forceinline void set_data(VoiceDataCustom* cdata) {
		xuid_low = cdata->xuid_low;
		xuid_high = cdata->xuid_high;
		sequence_bytes = cdata->sequence_bytes;
		section_number = cdata->section_number;
		uncompressed_sample_offset = cdata->uncompressed_sample_offset;
	}
};

class INetMessage {
public:
	virtual	~INetMessage() = default;

	virtual void SetNetChannel(void* netchan) = 0;
	virtual void SetReliable(bool state) = 0;
	virtual bool Process() = 0;
	virtual	bool ReadFromBuffer(void* buffer) = 0;
	virtual	bool WriteToBuffer(void* buffer) = 0;
	virtual bool IsReliable() const = 0;
	virtual int	GetType() const = 0;
	virtual int	GetGroup() const = 0;
	virtual const char* GetName() const = 0;
	virtual void* GetNetChannel() const = 0;
	virtual const char* ToString() const = 0;
};

class INetChannel {
public:
	enum {
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		TEMPENTS,		// temp entities
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		TOTAL,			// must be last and is not a real group
	};

	enum {
		FLOW_OUTGOING = 0,
		FLOW_INCOMING = 1,
		MAX_FLOWS     = 2
	};

	enum indices : size_t {
		GETLATENCY    = 9,
		GETAVGLATENCY = 10,
		PROCESSPACKET = 41,
		SENDNETMSG    = 42,
		SENDDATAGRAM  = 48,
	};

public:
	__forceinline float GetLatency( int flow ) {
		return util::get_method< float( __thiscall* )( decltype( this ), int ) >( this, GETLATENCY )( this, flow );
	}

	__forceinline float GetAvgLatency( int flow ) {
		return util::get_method< float( __thiscall* )( decltype( this ), int ) >( this, GETAVGLATENCY )( this, flow );
	}


	__forceinline bool SendNetMsg( void* msg, bool rel = false, bool voice = false ) {
		return util::get_method< bool( __thiscall* )( decltype( this ), void*, bool, bool ) >( this, SENDNETMSG )( this, msg, rel, voice );
	}

private:
	PAD( 0x14 );

public:
	bool m_processing_messages;		// 0x0014
	bool m_should_delete;			// 0x0015

private:
	PAD( 0x2 );

public:
	int m_out_seq;					// 0x0018 last send outgoing sequence number
	int m_in_seq;					// 0x001C last received incoming sequnec number
	int m_out_seq_ack;				// 0x0020 last received acknowledge outgoing sequnce number
	int m_out_rel_state;			// 0x0024 state of outgoing reliable data (0/1) flip flop used for loss detection
	int m_in_rel_state;				// 0x0028 state of incoming reliable data
	int m_choked_packets;			// 0x002C number of choked packets

private:
	PAD( 0x414 );					// 0x0030
};

struct player_info_t {
	uint64_t      m_data_map;
	union {
		int64_t   m_xuid;
		struct {
			int   m_xuid_low;
			int   m_xuid_high;
		};
	};
	char          m_name[ 128 ];
	int           m_user_id;
	char          m_guid[ 33 ];
	uint32_t      m_friends_id;
	char          m_friends_name[ 128 ];
	bool          m_fake_player;
	bool          m_is_hltv;
	uint32_t      m_custom_files[ 4 ];
	uint8_t       m_files_downloaded;
};

class CEventInfo {
public:
	enum {
		EVENT_INDEX_BITS    = 8,
		EVENT_DATA_LEN_BITS = 11,
		MAX_EVENT_DATA      = 192,  // ( 1<<8 bits == 256, but only using 192 below )
	};

	// 0 implies not in use
	short					m_class_id;
	float					m_fire_delay;
	const void*				m_send_table;
	const ClientClass*		m_client_class;
	int						m_bits;
	uint8_t*				m_data;
	int						m_flags;
	PAD( 0x18 );
	CEventInfo*				m_next;
};

class CClientState {
private:
	PAD( 0x9C );                                // 0x0000

public:
	INetChannel*    m_net_channel;				// 0x009C

private:
	PAD( 0x70 );                                // 0x00A0

public:
	int				m_next_message_time;		// 0x0110

public:
	float           m_net_cmd_time;             // 0x0114
	uint32_t        m_server_count;             // 0x0118
private:
	PAD( 0x4C );								// 0x011C

public:
	int             m_unk;                      // 0x0168
	int             m_server_tick;              // 0x016C
	int             m_client_tick;              // 0x0170
	int             m_delta_tick;               // 0x0174

private:
	PAD( 0x4B30 );                              // 0x0178

public:
	float           m_frame_time;               // 0x4CA8
	int             m_last_outgoing_command;    // 0x4CAC
	int             m_choked_commands;          // 0x4CB0
	int             m_last_command_ack;         // 0x4CB4
	uint32_t	    m_nSoundSequence;
	PAD( 0x134 );                               // 0x4CB8
	CEventInfo*     m_events;					// 0x4DEC

	enum indices : size_t {
		TEMPENTITIES = 36,
	};
};

class CGlobalState {
public:
	uintptr_t*    m_manager;
	CClientState* m_client_state;
};

class IVEngineClient {
public:
	enum indices : size_t {
		GETSCREENSIZE            = 5,
		GETPLAYERINFO            = 8,
		GETPLAYERFORUSERID       = 9,
		GETLOCALPLAYER           = 12,
		GETLASTTIMESTAMP         = 14,
		GETVIEWANGLES            = 18,
		SETVIEWANGLES            = 19,
		//GETMAXCLIENTS          = 20,
		GETBSPTREEQUERY          = 43,
		ISINGAME                 = 26,
		ISCONNECTED              = 27,
		WORLDTOSCREENMATRIX      = 37,
		FIREEVENTS               = 59,
		GETNETCHANNELINFO        = 78,
		ISPAUSED                 = 90,
		//ISTAKINGSCREENSHOT     = 92,
		ISHLTV                   = 93,
		//SETOCCLUSIONPARAMETERS = 96,
		EXECUTECLIENTCMD         = 108,
	};

public:
	__forceinline void GetScreenSize( int& width, int& height ) {
		return util::get_method< void( __thiscall* )( decltype( this ), int&, int& ) >( this, GETSCREENSIZE )( this, width, height );
	}

	__forceinline bool GetPlayerInfo( int index, player_info_t* info ) {
		return util::get_method< bool( __thiscall* )( decltype( this ), int, player_info_t* ) >( this, GETPLAYERINFO )( this, index, info );
	}

	__forceinline int GetPlayerForUserID( int uid ) {
		return util::get_method< int( __thiscall* )( decltype( this ), int ) >( this, GETPLAYERFORUSERID )( this, uid );
	}

	__forceinline int GetLocalPlayer( ) {
		return util::get_method< int( __thiscall* )( decltype( this ) ) >( this, GETLOCALPLAYER )( this );
	}

	__forceinline float GetLastTimestamp( ) {
		return util::get_method< float( __thiscall* )( decltype( this ) ) >( this, GETLASTTIMESTAMP )( this );
	}

	__forceinline void GetViewAngles( ang_t& angle ) {
		return util::get_method< void( __thiscall* )( decltype( this ), ang_t& ) >( this, GETVIEWANGLES )( this, angle );
	}

	__forceinline void SetViewAngles( ang_t& angle ) {
		return util::get_method< void( __thiscall* )( decltype( this ), ang_t& ) >( this, SETVIEWANGLES )( this, angle );
	}

	__forceinline bool IsInGame( ) {
		return util::get_method< bool( __thiscall* )( decltype( this ) ) >( this, ISINGAME )( this );
	}

	__forceinline void* GetBSPTreeQuery() {
		return util::get_method< void* (__thiscall*)(decltype(this)) >(this, GETBSPTREEQUERY)(this);
	}

	__forceinline bool IsConnected() {
		return util::get_method< bool(__thiscall*)(decltype(this)) >(this, ISCONNECTED)(this);
	}

	__forceinline const VMatrix& WorldToScreenMatrix( ) {
		return util::get_method< const VMatrix&( __thiscall* )( decltype( this ) ) >( this, WORLDTOSCREENMATRIX )( this );
	}

	__forceinline void FireEvents( ) {
		return util::get_method< void( __thiscall* )( decltype( this ) ) >( this, FIREEVENTS )( this );
	}

	__forceinline INetChannel* GetNetChannelInfo( ) {
		return util::get_method< INetChannel*( __thiscall* )( decltype( this ) ) >( this, GETNETCHANNELINFO )( this );
	}

	__forceinline bool IsPaused( ) {
		return util::get_method< bool( __thiscall* )( decltype( this ) ) >( this, ISPAUSED )( this );
	}

	__forceinline void ExecuteClientCmd( const char* cmd ) {
		return util::get_method< void( __thiscall* )( decltype( this ), const char* )>( this, EXECUTECLIENTCMD )( this, cmd );
	}

	//__forceinline void SetOcclusionParameters( float min, float max ) {
	//	float arr[] = { min, max };
	//	return util::get_method< void( __thiscall* )( decltype( this ), float* ) >( this, SETOCCLUSIONPARAMETERS )( this, arr );
	//}
};
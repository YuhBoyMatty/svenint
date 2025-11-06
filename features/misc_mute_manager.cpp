// SvenInt (c) Sw1ft
// misc_mute_manager.cpp

#include "stdafx.h"
#include "misc_mute_manager.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare Hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( void, CVoiceBanMgr__SetPlayerBan, void *, char *, bool );
DECLARE_CLASS_HOOK( void *, CVoiceBanMgr__InternalFindPlayerSquelch, void *, char * );

DECLARE_CLASS_HOOK( bool, CVoiceStatus__IsPlayerBlocked, void *, int );
DECLARE_CLASS_HOOK( void, CVoiceStatus__SetPlayerBlockedState, void *, int, bool );
DECLARE_CLASS_HOOK( void, CVoiceStatus__UpdateServerState, void *, bool );

DECLARE_HOOK( bool, __cdecl, HACK_GetPlayerUniqueID, int, char * );

//-----------------------------------------------------------------------------
// Feature gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Client
		{
			DEFINE_PATTERNS_2( CVoiceBanMgr__SetPlayerBan,
							   "5.25",
							   "56 FF 74 24 08 8B F1 E8 ? ? ? ? 80 7C 24 0C 00 74 13 85 C0 75 32 FF 74 24 08 8B CE E8",
							   "5.26",
							   "56 FF 74 24 08 8B F1 E8 ? ? ? ? 80 7C 24 0C" );

			DEFINE_PATTERNS_2( CVoiceBanMgr__InternalFindPlayerSquelch,
							   "5.25",
							   "53 55 8B 6C 24 0C 56 57 0F 10 4D 00 0F 28 C1 66 0F 73 D8 08 66 0F FC C8 0F 10 C1 66 0F 73 D8 04",
							   "5.26",
							   "53 55 8B 6C 24 0C 56" );

			DEFINE_PATTERNS_2( CVoiceStatus__IsPlayerBlocked,
							   "5.25",
							   "83 EC 14 A1 ? ? ? ? 33 C4 89 44 24 10 56 8D 44 24 04 8B F1 50 FF 74 24 20 FF 15",
							   "5.26",
							   "83 EC ? A1 ? ? ? ? 33 C4 89 44 24 10 A1 ? ? ? ? 56" );

			DEFINE_PATTERNS_2( CVoiceStatus__SetPlayerBlockedState,
							   "5.25",
							   "81 EC ? ? 00 00 A1 ? ? ? ? 33 C4 89 84 24 ? ? 00 00 53 68 ? ? ? ? 8B D9 FF 15 ? ? ? ? D9 5C 24 08",
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 14 01 00 00" );

			DEFINE_PATTERNS_2( CVoiceStatus__UpdateServerState,
							   "5.25",
							   "81 EC ? ? 00 00 A1 ? ? ? ? 33 C4 89 84 24 ? ? 00 00 53 8B D9 89 5C 24 08",
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 18 0B 00 00" );

			DEFINE_PATTERNS_2( HACK_GetPlayerUniqueID,
							   "5.25",
							   "FF 74 24 08 FF 74 24 08 FF 15 ? ? ? ? 83 C4 08 85 C0 0F 95 C0 C3",
							   "5.26",
							   "FF 74 24 08 A1" );
		}
	}
}

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

// Muted players container specifics
#define IMM_VERSION ( 1 )
#define IMM_HEADER ( 0x2F77 )

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CMuteManager, soundcache, "Misc", "Mute Manager" );

static UserMsgHookFn ORIG_UserMsgHook_SayText = NULL;

//-----------------------------------------------------------------------------
// ConCommands
//-----------------------------------------------------------------------------

static bool CheckPlayer( int entindex )
{
	return entindex > 0 && entindex <= cl_enginefuncs->GetMaxClients() &&
		cls->state == ca_active &&
		cl_enginefuncs->GetEntityByIndex( entindex ) &&
		cl_enginefuncs->GetEntityByIndex( entindex ) != cl_enginefuncs->GetLocalPlayer();
}

CON_COMMAND( imm_mute_voice, "Mute player's voice communication" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->AddMutedPlayer( steamid, MUTE_VOICE );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" muted (voice)\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_mute_voice <player index>\n" );
	}
}

CON_COMMAND( imm_mute_chat, "Mute player's chat communication" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->AddMutedPlayer( steamid, MUTE_CHAT );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" muted (chat)\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_mute_chat <player index>\n" );
	}
}

CON_COMMAND( imm_mute_all, "Mute all player communications" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->AddMutedPlayer( steamid, MUTE_ALL );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" muted\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_mute_all <player index>\n" );
	}
}

CON_COMMAND( imm_unmute_voice, "Unmute player's voice communication" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_VOICE );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" unmuted (voice)\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_unmute_voice <player index>\n" );
	}
}

CON_COMMAND( imm_unmute_chat, "Unmute player's chat communication" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_CHAT );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" unmuted (chat)\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_unmute_chat <player index>\n" );
	}
}

CON_COMMAND( imm_unmute_all, "Unmute all player communications" )
{
	if ( args.ArgC() > 1 )
	{
		int nPlayerIndex = atoi( args[ 1 ] );

		if ( CheckPlayer( nPlayerIndex ) )
		{
			uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

			THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_ALL );

			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( nPlayerIndex - 1 );

			if ( pPlayerInfo && *pPlayerInfo->name )
				THIS_FEATURE()->PrintMsg( "Player \"%s\" unmuted\n", pPlayerInfo->name );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_unmute_all <player index>\n" );
	}
}

CON_COMMAND( imm_unmute_by_steamid64, "Unmute all player communications with given Steam64 ID" )
{
	if ( args.ArgC() > 1 )
	{
		uint64 steamid = atoll( args[ 1 ] );

		if ( THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_ALL ) )
		{
			THIS_FEATURE()->PrintMsg( "SteamID %llu has been removed\n", steamid );
		}
		else
		{
			THIS_FEATURE()->PrintMsg( "SteamID %llu not found\n", steamid );
		}
	}
	else
	{
		ConMsg( "Usage:  imm_unmute_by_steamid64 <Steam64 ID>\n" );
	}
}

CON_COMMAND( imm_save_to_file, "Save all muted players to file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"" )
{
	THIS_FEATURE()->SaveMutedPlayers();
}

CON_COMMAND( imm_print_muted_players, "Print all muted players" )
{
	Msg( "====================== Muted Players ======================\n" );

	int current_players = 1;

	auto mutedPlayers = THIS_FEATURE()->GetMutedPlayersTable();
	for ( int i = 0; i < mutedPlayers.Count(); i++ )
	{
		HashTableIterator_t it = mutedPlayers.First( i );

		while ( mutedPlayers.IsValidIterator( it ) )
		{
			uint64 &steamid = mutedPlayers.KeyAt( i, it );
			uint32 &mute_flags = mutedPlayers.ValueAt( i, it );

			Msg( "%d >> SteamID: %llu | Voice: %d | Chat: %d\n", current_players, steamid, ( mute_flags & MUTE_VOICE ) != 0, ( mute_flags & MUTE_CHAT ) != 0 );

			it = mutedPlayers.Next( i, it );
			++current_players;
		}
	}

	Msg( "====================== Muted Players ======================\n" );
}

CON_COMMAND( imm_print_current_muted_players, "Print currently muted players" )
{
	if ( cls->state != ca_active )
		return;

	Msg( "====================== Muted Players ======================\n" );

	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
	int nLocalPlayer = pLocal->index;

	for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); ++i )
	{
		if ( i == nLocalPlayer )
			continue;

		uint64 steamid = gameutils->GetSteamID( i );

		if ( !steamid )
			continue;

		uint32 *mute_flags = THIS_FEATURE()->GetMutedPlayer( steamid );

		if ( !mute_flags )
			continue;

		player_info_t *pPlayerInfo = enginestudio->PlayerInfo( i - 1 );

		Msg( "#%d >> Player: \"%s\" | Voice: %d | Chat: %d\n", i, pPlayerInfo->name, ( *mute_flags & MUTE_VOICE ) != 0, ( *mute_flags & MUTE_CHAT ) != 0 );
	}

	Msg( "====================== Muted Players ======================\n" );
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_CVoiceBanMgr__SetPlayerBan, void *thisptr, char *pszPlayerUniqueID, bool bMute )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_CVoiceBanMgr__SetPlayerBan( thisptr, pszPlayerUniqueID, bMute );
		return;
	}

	uint64 steamid = gameutils->GetSteamID( THIS_FEATURE()->GetLastIndexedPlayer() );

	if ( !steamid )
		return;

	if ( bMute )
		THIS_FEATURE()->AddMutedPlayer( steamid, MUTE_VOICE );
	else
		THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_VOICE );
}

// Get pointer to muted player (won't hook CVoiceBanMgr::GetPlayerBan to save perfomance speed)

DECLARE_CLASS_FUNC( void *, HOOKED_CVoiceBanMgr__InternalFindPlayerSquelch, void *thisptr, char *pszPlayerUniqueID )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		return ORIG_CVoiceBanMgr__InternalFindPlayerSquelch( thisptr, pszPlayerUniqueID );
	}

	uint64 steamid = gameutils->GetSteamID( THIS_FEATURE()->GetLastIndexedPlayer() );

	if ( !steamid )
		return NULL;

	uint32 *mute_flags = THIS_FEATURE()->GetMutedPlayer( steamid );

	if ( mute_flags && ( *mute_flags & MUTE_VOICE || THIS_FEATURE()->AllCommunicationsMuted() ) )
		return mute_flags;

	return NULL;
}

DECLARE_CLASS_FUNC( bool, HOOKED_CVoiceStatus__IsPlayerBlocked, void *thisptr, int nPlayerIndex )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		return ORIG_CVoiceStatus__IsPlayerBlocked( thisptr, nPlayerIndex );
	}

	uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

	if ( !steamid )
		return false;

	uint32 *mute_flags = THIS_FEATURE()->GetMutedPlayer( steamid );

	if ( mute_flags )
	{
		if ( THIS_FEATURE()->AllCommunicationsMuted() )
			return true;

		if ( THIS_FEATURE()->IsProcessingChat() )
		{
			if ( *mute_flags & MUTE_CHAT )
				return true;
		}
		else if ( *mute_flags & MUTE_VOICE )
		{
			return true;
		}
	}

	return false;
}

// Called when you (un)mute player via scoreboard

DECLARE_CLASS_FUNC( void, HOOKED_CVoiceStatus__SetPlayerBlockedState, void *thisptr, int nPlayerIndex, bool bMute )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_CVoiceStatus__SetPlayerBlockedState( thisptr, nPlayerIndex, bMute );
		return;
	}

	uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

	if ( !steamid )
		return;

	if ( bMute )
		THIS_FEATURE()->AddMutedPlayer( steamid, MUTE_VOICE );
	else
		THIS_FEATURE()->RemoveMutedPlayer( steamid, MUTE_VOICE );
}

// Send to server the mask of muted players that we don't want to hear

DECLARE_CLASS_FUNC( void, HOOKED_CVoiceStatus__UpdateServerState, void *thisptr, bool bForce )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_CVoiceStatus__UpdateServerState( thisptr, bForce );
		return;
	}

	THIS_FEATURE()->UpdateServerState( thisptr, bForce );
}

DECLARE_FUNC( bool, __cdecl, HOOKED_HACK_GetPlayerUniqueID, int nPlayerIndex, char *pszPlayerUniqueID )
{
	THIS_FEATURE()->SetLastIndexedPlayer( nPlayerIndex );
	return ORIG_HACK_GetPlayerUniqueID( nPlayerIndex, pszPlayerUniqueID );
}

static int UserMsgHook_SayText( const char *pszName, int iSize, void *pBuffer )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		return ORIG_UserMsgHook_SayText( pszName, iSize, pBuffer );
	}

	CMessageBuffer SayTextBuffer;
	SayTextBuffer.Init( pszName, pBuffer, iSize, true );
	SayTextBuffer.BeginReading();

	int result = 0;
	int nPlayerIndex = SayTextBuffer.ReadByte();

	uint64 steamid = gameutils->GetSteamID( nPlayerIndex );

	if ( !steamid )
	{
		THIS_FEATURE()->SetProcessingChat( true );

		result = ORIG_UserMsgHook_SayText( pszName, iSize, pBuffer );

		THIS_FEATURE()->SetProcessingChat( false );

		return result;
	}

	uint32 *mute_flags = THIS_FEATURE()->GetMutedPlayer( steamid );

	if ( mute_flags )
	{
		if ( THIS_FEATURE()->AllCommunicationsMuted() || *mute_flags & MUTE_CHAT )
			return 0;
	}

	THIS_FEATURE()->SetProcessingChat( true );

	result = ORIG_UserMsgHook_SayText( pszName, iSize, pBuffer );

	THIS_FEATURE()->SetProcessingChat( false );

	return result;
}

//-----------------------------------------------------------------------------
// UpdateServerState event
//-----------------------------------------------------------------------------

void CMuteManager::UpdateServerState( void *thisptr, bool bForce )
{
	if ( cls->state != ca_active )
		return;

	static float flForceBanMaskTime = 0.f;
	static char command_buffer[ 128 ];

	char const *pLevelName = cl_enginefuncs->pfnGetLevelName();
	bool bClientDebug = bool( voice_clientdebug->value );

	if ( *pLevelName == 0 && bClientDebug )
	{
		Msg( "CVoiceStatus::UpdateServerState: pLevelName[0]==0\n" );
		return;
	}

	uint32 banMask = 0;

	bool bMuteEverything = m_pMuteAllCommunications->GetBool();
	bool bVoiceModEnable = static_cast<bool>( voice_modenable->value );

	// thisptr members
	float *m_LastUpdateServerState = (float *)( (unsigned char *)thisptr + 0x18 );
	int *m_bServerModEnable = (int *)( (unsigned char *)thisptr + 0x1C );

	// validate cvar 'voice_modenable'
	if ( bForce || bool( *m_bServerModEnable ) != bVoiceModEnable )
	{
		*m_bServerModEnable = static_cast<int>( bVoiceModEnable );

		snprintf( command_buffer, sizeof( command_buffer ), "VModEnable %d", bVoiceModEnable );
		cl_enginefuncs->pfnClientCmd( command_buffer );

		command_buffer[ sizeof( command_buffer ) - 1 ] = 0;

		if ( bClientDebug )
			Msg( "CVoiceStatus::UpdateServerState: Sending '%s'\n", command_buffer );
	}

	// build ban mask
	for ( uint32 i = 1; i <= (uint32)cl_enginefuncs->GetMaxClients(); ++i )
	{
		uint64 steamid = gameutils->GetSteamID( i );

		if ( !steamid )
			continue;

		uint32 *mute_flags = THIS_FEATURE()->GetMutedPlayer( steamid );

		if ( mute_flags && ( bMuteEverything || *mute_flags & MUTE_VOICE ) )
			banMask |= 1 << ( i - 1 ); // one bit, one client
	}

	if ( cl_enginefuncs->GetClientTime() - flForceBanMaskTime < 0.f )
	{
		flForceBanMaskTime = 0.f;
	}

	if ( m_BanMask != banMask || ( cl_enginefuncs->GetClientTime() - flForceBanMaskTime >= 5.0f ) )
	{
		snprintf( command_buffer, sizeof( command_buffer ), "vban %X", banMask ); // vban [ban_mask]

		if ( bClientDebug )
			Msg( "CVoiceStatus::UpdateServerState: Sending '%s'\n", command_buffer );

		cl_enginefuncs->pfnClientCmd( command_buffer );
		m_BanMask = banMask;
	}
	else if ( bClientDebug )
	{
		Msg( "CVoiceStatus::UpdateServerState: no change\n" );
	}

	*m_LastUpdateServerState = flForceBanMaskTime = cl_enginefuncs->GetClientTime();
}

//-----------------------------------------------------------------------------
// Purpose: load muted players in hash table from file muted_players.bin
//-----------------------------------------------------------------------------

void CMuteManager::LoadMutedPlayers( void )
{
	FILE *file = fopen( SVENINT_FOLDER_NAME "/muted_players.bin", "rb" );

	if ( file )
	{
		int buffer = 0;

		fread( &buffer, 1, sizeof( short ), file );

		if ( buffer != IMM_HEADER )
		{
			PrintWarning( "Invalid header of file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n" );
			return;
		}

		buffer = 0;
		fread( &buffer, 1, sizeof( char ), file );

		if ( buffer < 1 )
		{
			PrintWarning( "Invalid version of file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n" );
			return;
		}

		static struct MutedPlayerEntry
		{
			uint32 steamid_high;
			uint32 steamid_low;
			uint32 flags;
		} s_MutedPlayerBuffer;

		int loaded_players = 0;
		while ( fread( &s_MutedPlayerBuffer, 1, sizeof( MutedPlayerEntry ), file ) == sizeof( MutedPlayerEntry ) )
		{
			uint64 steamid = *reinterpret_cast<uint64_t *>( &s_MutedPlayerBuffer.steamid_high );

			m_MutedPlayers.Insert( steamid, s_MutedPlayerBuffer.flags );
			++loaded_players;
		}

		fclose( file );

		PrintMsg( "Loaded %d players from file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n", loaded_players );
	}
	else
	{
		PrintWarning( "Missing file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: save hash table in file muted_players.bim
//-----------------------------------------------------------------------------

void CMuteManager::SaveMutedPlayers( void )
{
	FILE *file = fopen( SVENINT_FOLDER_NAME "/muted_players.bin", "wb" );

	if ( file )
	{
		int buffer = 0;
		int saved_players = 0;

		buffer = IMM_HEADER;
		fwrite( &buffer, 1, sizeof( short ), file );

		buffer = IMM_VERSION;
		fwrite( &buffer, 1, sizeof( char ), file );

		for ( int i = 0; i < m_MutedPlayers.Count(); i++ )
		{
			HashTableIterator_t it = m_MutedPlayers.First( i );

			while ( m_MutedPlayers.IsValidIterator( it ) )
			{
				uint64 &steamid = m_MutedPlayers.KeyAt( i, it );
				uint32 &mute_flags = m_MutedPlayers.ValueAt( i, it );

				fwrite( &steamid, 1, sizeof( uint64 ), file );
				fwrite( &mute_flags, 1, sizeof( uint32 ), file );

				it = m_MutedPlayers.Next( i, it );
				++saved_players;
			}
		}

		fclose( file );

		PrintMsg( "Saved %d players in file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n", saved_players );
	}
	else
	{
		PrintWarning( "Cannot create file \"./" SVENINT_FOLDER_NAME "/muted_players.bin\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CMuteManager::CMuteManager( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ), m_MutedPlayers( 511 )
{
	m_pMuteAllCommunications = NULL;
	m_pAutosave = NULL;

	m_bProcessingChat = false;
	m_nLastIndexedPlayer = -1;
	m_BanMask = 0;

	voice_clientdebug = NULL;
	voice_modenable = NULL;

	m_pfnCVoiceBanMgr__SetPlayerBan = NULL;
	m_pfnCVoiceBanMgr__InternalFindPlayerSquelch = NULL;
	m_pfnCVoiceStatus__IsPlayerBlocked = NULL;
	m_pfnCVoiceStatus__SetPlayerBlockedState = NULL;
	m_pfnCVoiceStatus__UpdateServerState = NULL;
	m_pfnHACK_GetPlayerUniqueID = NULL;

	m_hCVoiceBanMgr__SetPlayerBan = DETOUR_INVALID_HANDLE;
	m_hCVoiceBanMgr__InternalFindPlayerSquelch = DETOUR_INVALID_HANDLE;
	m_hCVoiceStatus__IsPlayerBlocked = DETOUR_INVALID_HANDLE;
	m_hCVoiceStatus__SetPlayerBlockedState = DETOUR_INVALID_HANDLE;
	m_hCVoiceStatus__UpdateServerState = DETOUR_INVALID_HANDLE;
	m_hHACK_GetPlayerUniqueID = DETOUR_INVALID_HANDLE;

	m_hUserMsgHook_SayText = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CMuteManager::OnEnable( void )
{
	LoadMutedPlayers();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CMuteManager::OnDisable( void )
{
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CMuteManager::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pMuteAllCommunications = Modules::menu->AddParamBool( this, "MuteAllCommunications", NULL, false );
	m_pAutosave = Modules::menu->AddParamBool( this, "Autosave", NULL, true );

	// Get native cvars
	voice_clientdebug = cvar->FindCvar( "voice_clientdebug" );
	FEATURE_CHECK_SYMBOL( voice_clientdebug, "voice_clientdebug" );

	voice_modenable = cvar->FindCvar( "voice_modenable" );
	FEATURE_CHECK_SYMBOL( voice_modenable, "voice_modenable" );

	int patternIndex;
	bool bOK = true;

	DEFINE_PATTERNS_FUTURE( fCVoiceBanMgr__SetPlayerBan );
	DEFINE_PATTERNS_FUTURE( fCVoiceBanMgr__InternalFindPlayerSquelch );
	DEFINE_PATTERNS_FUTURE( fCVoiceStatus__IsPlayerBlocked );
	DEFINE_PATTERNS_FUTURE( fCVoiceStatus__SetPlayerBlockedState );
	DEFINE_PATTERNS_FUTURE( fCVoiceStatus__UpdateServerState );
	DEFINE_PATTERNS_FUTURE( fHACK_GetPlayerUniqueID );

	// Find signatures
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CVoiceBanMgr__SetPlayerBan, fCVoiceBanMgr__SetPlayerBan );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CVoiceBanMgr__InternalFindPlayerSquelch, fCVoiceBanMgr__InternalFindPlayerSquelch );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CVoiceStatus__IsPlayerBlocked, fCVoiceStatus__IsPlayerBlocked );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CVoiceStatus__SetPlayerBlockedState, fCVoiceStatus__SetPlayerBlockedState );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CVoiceStatus__UpdateServerState, fCVoiceStatus__UpdateServerState );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::HACK_GetPlayerUniqueID, fHACK_GetPlayerUniqueID );

	// CVoiceBanMgr::SetPlayerBan
	m_pfnCVoiceBanMgr__SetPlayerBan = MemoryUtils()->GetPatternFutureValue( fCVoiceBanMgr__SetPlayerBan, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCVoiceBanMgr__SetPlayerBan,
										  "CVoiceBanMgr::SetPlayerBan",
										  FeaturesGameData::Patterns::Client::CVoiceBanMgr__SetPlayerBan,
										  patternIndex );

	// CVoiceBanMgr::InternalFindPlayerSquelch
	m_pfnCVoiceBanMgr__InternalFindPlayerSquelch = MemoryUtils()->GetPatternFutureValue( fCVoiceBanMgr__InternalFindPlayerSquelch, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCVoiceBanMgr__InternalFindPlayerSquelch,
										  "CVoiceBanMgr::InternalFindPlayerSquelch",
										  FeaturesGameData::Patterns::Client::CVoiceBanMgr__InternalFindPlayerSquelch,
										  patternIndex );

	// CVoiceStatus::IsPlayerBlocked
	m_pfnCVoiceStatus__IsPlayerBlocked = MemoryUtils()->GetPatternFutureValue( fCVoiceStatus__IsPlayerBlocked, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCVoiceStatus__IsPlayerBlocked,
										  "CVoiceStatus::IsPlayerBlocked",
										  FeaturesGameData::Patterns::Client::CVoiceStatus__IsPlayerBlocked,
										  patternIndex );

	// CVoiceStatus::SetPlayerBlockedState
	m_pfnCVoiceStatus__SetPlayerBlockedState = MemoryUtils()->GetPatternFutureValue( fCVoiceStatus__SetPlayerBlockedState, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCVoiceStatus__SetPlayerBlockedState,
										  "CVoiceStatus::SetPlayerBlockedState",
										  FeaturesGameData::Patterns::Client::CVoiceStatus__SetPlayerBlockedState,
										  patternIndex );

	// CVoiceStatus::UpdateServerState
	m_pfnCVoiceStatus__UpdateServerState = MemoryUtils()->GetPatternFutureValue( fCVoiceStatus__UpdateServerState, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCVoiceStatus__UpdateServerState,
										  "CVoiceStatus::UpdateServerState",
										  FeaturesGameData::Patterns::Client::CVoiceStatus__UpdateServerState,
										  patternIndex );

	// HACK_GetPlayerUniqueID
	m_pfnHACK_GetPlayerUniqueID = MemoryUtils()->GetPatternFutureValue( fHACK_GetPlayerUniqueID, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnHACK_GetPlayerUniqueID,
										  "HACK_GetPlayerUniqueID",
										  FeaturesGameData::Patterns::Client::HACK_GetPlayerUniqueID,
										  patternIndex );

	if ( !bOK )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CMuteManager::PostLoad( void )
{
	m_hCVoiceBanMgr__SetPlayerBan = Detours()->DetourFunction( m_pfnCVoiceBanMgr__SetPlayerBan, HOOKED_CVoiceBanMgr__SetPlayerBan, GET_FUNC_PTR( ORIG_CVoiceBanMgr__SetPlayerBan ) );
	m_hCVoiceBanMgr__InternalFindPlayerSquelch = Detours()->DetourFunction( m_pfnCVoiceBanMgr__InternalFindPlayerSquelch, HOOKED_CVoiceBanMgr__InternalFindPlayerSquelch, GET_FUNC_PTR( ORIG_CVoiceBanMgr__InternalFindPlayerSquelch ) );
	m_hCVoiceStatus__IsPlayerBlocked = Detours()->DetourFunction( m_pfnCVoiceStatus__IsPlayerBlocked, HOOKED_CVoiceStatus__IsPlayerBlocked, GET_FUNC_PTR( ORIG_CVoiceStatus__IsPlayerBlocked ) );
	m_hCVoiceStatus__SetPlayerBlockedState = Detours()->DetourFunction( m_pfnCVoiceStatus__SetPlayerBlockedState, HOOKED_CVoiceStatus__SetPlayerBlockedState, GET_FUNC_PTR( ORIG_CVoiceStatus__SetPlayerBlockedState ) );
	m_hCVoiceStatus__UpdateServerState = Detours()->DetourFunction( m_pfnCVoiceStatus__UpdateServerState, HOOKED_CVoiceStatus__UpdateServerState, GET_FUNC_PTR( ORIG_CVoiceStatus__UpdateServerState ) );
	m_hHACK_GetPlayerUniqueID = Detours()->DetourFunction( m_pfnHACK_GetPlayerUniqueID, HOOKED_HACK_GetPlayerUniqueID, GET_FUNC_PTR( ORIG_HACK_GetPlayerUniqueID ) );

	m_hUserMsgHook_SayText = gamehooks->HookUserMessage( "SayText", UserMsgHook_SayText, &ORIG_UserMsgHook_SayText );

	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_mute_voice ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_mute_chat ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_mute_all ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_voice ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_chat ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_all ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_by_steamid64 ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_save_to_file ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_print_muted_players ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( imm_print_current_muted_players ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CMuteManager::Unload( void )
{
	if ( m_pAutosave->GetBool() )
		SaveMutedPlayers();

	RemoveMutedPlayers();

	Detours()->RemoveDetour( m_hCVoiceBanMgr__SetPlayerBan );
	Detours()->RemoveDetour( m_hCVoiceBanMgr__InternalFindPlayerSquelch );
	Detours()->RemoveDetour( m_hCVoiceStatus__IsPlayerBlocked );
	Detours()->RemoveDetour( m_hCVoiceStatus__SetPlayerBlockedState );
	Detours()->RemoveDetour( m_hCVoiceStatus__UpdateServerState );
	Detours()->RemoveDetour( m_hHACK_GetPlayerUniqueID );

	gamehooks->UnhookUserMessage( m_hUserMsgHook_SayText );

	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_mute_voice ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_mute_chat ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_mute_all ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_voice ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_chat ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_all ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_unmute_by_steamid64 ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_save_to_file ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_print_muted_players ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( imm_print_current_muted_players ) );

	m_MutedPlayers.Purge();
}
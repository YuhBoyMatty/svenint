// SvenInt (c) Sw1ft
// misc_votebot.cpp

#include "stdafx.h"
#include "misc_votebot.h"
#include "hud_vote_popup.h"
#include "modules/menu.h"
#include "game/messagebuffer.h"
#include "utils/util.h"
#include <regex>

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

static UserMsgHookFn ORIG_UserMsgHook_TextMsg = NULL;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CVoteBot, votebot, "Misc", "Votebot" );

static const char *g_szVoteCommands[] =
{
	"votekill",
	"votekick",
	"voteban"
};

//-----------------------------------------------------------------------------
// VoteAgainstPlayer
//-----------------------------------------------------------------------------

static inline void VoteAgainstPlayer( int iVoteCommand, int iPlayerIndex )
{
	char s_szVoteBuffer[ 512 ];
	player_info_t *pPlayerInfo;

	if ( pPlayerInfo = enginestudio->PlayerInfo( iPlayerIndex - 1 ) )
	{
		snprintf( s_szVoteBuffer, sizeof( s_szVoteBuffer ), "%s \"%s\"\n", g_szVoteCommands[ iVoteCommand ], pPlayerInfo->name );
		cl_enginefuncs->pfnClientCmd( s_szVoteBuffer );
	}
}

//-----------------------------------------------------------------------------
// Reload targets
//-----------------------------------------------------------------------------

void CVoteBot::ReloadTargets( void )
{
	char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/votebot/targets.txt", "r" );

	if ( file != NULL )
	{
		m_targets.clear();
		int nLine = 0;

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( (char *)szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			uint64 steamID = atoll( buffer );
			if ( steamID != 0uLL )
				m_targets.push_back( steamID );
		}

		fclose( file );

		if ( !m_targets.empty() )
			std::sort( m_targets.begin(), m_targets.end() );
	}
	else
	{
		PrintWarning( "Missing file \"./" SVENINT_FOLDER_NAME "/votebot/targets.txt\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Reload friends
//-----------------------------------------------------------------------------

void CVoteBot::ReloadFriends( void )
{
	char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/votebot/friends.txt", "r" );

	if ( file != NULL )
	{
		m_friends.clear();
		int nLine = 0;

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( (char *)szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			uint64 steamID = atoll( buffer );
			if ( steamID != 0uLL )
				m_friends.push_back( steamID );
		}

		fclose( file );

		if ( !m_friends.empty() )
			std::sort( m_friends.begin(), m_friends.end() );
	}
	else
	{
		PrintWarning( "Missing file \"./" SVENINT_FOLDER_NAME "/votebot/friends.txt\"\n" );
	}
}

//-----------------------------------------------------------------------------
// User msg hook TextMsg
//-----------------------------------------------------------------------------

static int UserMsgHook_TextMsg( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->TextMsgEvent( pszUserMsg, iSize, pBuffer );

	return ORIG_UserMsgHook_TextMsg( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// TextMsg event
//-----------------------------------------------------------------------------

void CVoteBot::TextMsgEvent( const char *pszUserMsg, int iSize, void *pBuffer )
{
	CMessageBuffer TextMsgBuffer;
	TextMsgBuffer.Init( pszUserMsg, pBuffer, iSize, true );
	TextMsgBuffer.BeginReading();

	int msg_dest = TextMsgBuffer.ReadByte();

	if ( msg_dest == HUD_PRINTNOTIFY )
	{
		std::cmatch match;
		std::regex regex_vote_pattern( "Vote to (kill|kick|ban) \"%s\" started by \"%s\".\n" );

		const char *pszMsgName = TextMsgBuffer.ReadString();

		if ( std::regex_search( pszMsgName, match, regex_vote_pattern ) )
		{
			TextMsgBuffer.ReadString();
			const char *pszCaller = MemStrdup( TextMsgBuffer.ReadString() );

			player_info_s *pPlayerInfo = enginestudio->PlayerInfo( playermove->player_index() );

			if ( pPlayerInfo && !strcmp( pszCaller, pPlayerInfo->name ) && m_pAutoReconnect->GetBool() )
			{
				if ( m_pMode->GetInt() > 0 )
				{
					if ( Features::votepopup->IsEnabled() )
					{
						cl_funcs->HUD_Key_Event( 1, m_pMode->GetInt() == 1 ? K_F1 : K_F2, NULL );
					}
					else
					{
						cl_enginefuncs->pfnClientCmd( m_pMode->GetInt() == 1 ? "voteyes\n" : "voteno\n" );
					}
				}

				netadr_t addr;
				char buffer[ 128 ];

			#if 0
				engineclient->GetServerAddress( &addr );
			#else
				static net_status_t status;
				cl_enginefuncs->pNetAPI->Status( &status );
				memcpy( &addr, &status.remote_address, sizeof( netadr_t ) );
			#endif

				int port = ( addr.port << 8 ) | ( addr.port >> 8 );

				snprintf( buffer, sizeof( buffer ) / sizeof( char ), "connect %hhu.%hhu.%hhu.%hhu:%hu\n", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ], port );
				cl_enginefuncs->pfnClientCmd( buffer );

				//cl_enginefuncs->pfnClientCmd("reconnect\n");
			}

			MemFree( (void *)pszCaller );
		}
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CVoteBot::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_flVoteCooldown = -1.f;
		return kHookContinue;
	}

	// Host_Filter post event
	if ( !pEvent->GetReturn<qboolean>() )
		return kHookContinue;

	if ( m_prevstate != cls->state && m_prevstate < ca_active )
	{
		m_flVoteCooldown = cl_enginefuncs->GetClientTime() + 1.5f;
	}

	if ( cls->state == ca_active && cl_enginefuncs->GetClientTime() >= m_flVoteCooldown )
	{
		if ( m_pVoteInObserver->GetBool() && !localplayer->IsSpectating() )
			return kHookContinue;

		m_voteTargets.clear();

		int iVoteCommand = m_pCallVoteMode->GetInt();

		switch ( m_pTargetMode->GetInt() )
		{
		case 0: // Vote against a random player
		{
			cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

			for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
			{
				cl_entity_t *pPlayer = cl_enginefuncs->GetEntityByIndex( i );

				if ( pPlayer && pPlayer != pLocal )
				{
					if ( m_pIgnoreDeadPlayers->GetBool() && !( extraplayerinfo->GetHealth( i ) < -1.f || extraplayerinfo->GetHealth( i ) > 0.f ) )
						continue;

					if ( m_pFilterFriends->GetBool() )
					{
						uint64 steamID = gameutils->GetSteamID( i );

						if ( steamID != 0uLL && FindFriend( steamID ) != 0uLL )
							continue;
					}

					m_voteTargets.push_back( i );
				}
			}

			if ( m_voteTargets.size() > 0 )
			{
				if ( m_voteTargets.size() == 1 )
				{
					VoteAgainstPlayer( iVoteCommand, m_voteTargets[ 0 ] );
				}
				else
				{
					int iPlayerIndex = cl_enginefuncs->pfnRandomLong( 0, m_voteTargets.size() - 1 );
					VoteAgainstPlayer( iVoteCommand, iPlayerIndex );
				}
			}

			break;
		}

		case 1: // Vote against yourself
		{
			VoteAgainstPlayer( iVoteCommand, localplayer->GetPlayerIndex() );
			break;
		}

		case 2: // Vote against players from file "../votebot/targets.txt"
		{
			for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
			{
				cl_entity_t *pPlayer = cl_enginefuncs->GetEntityByIndex( i );

				if ( pPlayer )
				{
					if ( m_pIgnoreDeadPlayers->GetBool() && !( extraplayerinfo->GetHealth( i ) < -1.f || extraplayerinfo->GetHealth( i ) > 0.f ) )
						continue;

					uint64 steamID = gameutils->GetSteamID( i );

					if ( FindTarget( steamID ) == 0uLL )
						continue;

					m_voteTargets.push_back( i );
				}
			}

			if ( m_voteTargets.size() > 0 )
			{
				if ( m_voteTargets.size() == 1 )
				{
					VoteAgainstPlayer( iVoteCommand, m_voteTargets[ 0 ] );
				}
				else
				{
					int iPlayerIndex = cl_enginefuncs->pfnRandomLong( 0, m_voteTargets.size() - 1 );
					VoteAgainstPlayer( iVoteCommand, iPlayerIndex );
				}
			}

			break;
		}
		}
	}

	m_prevstate = (int)cls->state;
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CVoteBot::CVoteBot( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pAutoReconnect = NULL;
	m_pFilterFriends = NULL;
	m_pIgnoreDeadPlayers = NULL;
	m_pVoteInObserver = NULL;
	m_pMode = NULL;
	m_pTargetMode = NULL;
	m_pCallVoteMode = NULL;

	m_prevstate = ca_dedicated;
	m_flVoteCooldown = -1.f;
	m_hUserMsgHook_TextMsg = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CVoteBot::OnEnable( void )
{
	ReloadTargets();
	ReloadFriends();
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CVoteBot::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CVoteBot::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pAutoReconnect = Modules::menu->AddParamBool( this, "AutoReconnect", NULL, true );
	m_pFilterFriends = Modules::menu->AddParamBool( this, "FilterFriends", NULL, true );
	m_pIgnoreDeadPlayers = Modules::menu->AddParamBool( this, "IgnoreDeadPlayers", NULL, false );
	m_pVoteInObserver = Modules::menu->AddParamBool( this, "VoteInObserver", NULL, false );
	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Don't vote\0 1 - Vote \"Yes\"\0 2 - Vote \"No\"\0\0" );
	m_pTargetMode = Modules::menu->AddParamList( this, "TargetMode", NULL, 0, " 0 - Random player\0 1 - Against yourself\0 2 - Players from targets.txt file\0\0" );
	m_pCallVoteMode = Modules::menu->AddParamList( this, "CallVoteMode", NULL, 0, " 0 - Votekill\0 1 - Votekick\0 2 - Voteban\0\0" );

	m_prevstate = cls->state;

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CVoteBot::PostLoad( void )
{
	m_hUserMsgHook_TextMsg = gamehooks->HookUserMessage( "TextMsg", UserMsgHook_TextMsg, &ORIG_UserMsgHook_TextMsg );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CVoteBot::Unload( void )
{
	Detours()->RemoveDetour( m_hUserMsgHook_TextMsg );
}
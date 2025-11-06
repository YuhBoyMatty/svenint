// SvenInt (c) Sw1ft
// server.cpp

#include "stdafx.h"
#include "server.h"
#include "scripts.h"
#include "game/messagebuffer.h"
#include "features/st_timescale.h"
#include "features/st_timer.h"
#include "features/st_player_hulls.h"

namespace Modules { static CServerModule serverModule; CServerModule *server = &serverModule; }

//-----------------------------------------------------------------------------
// CSurvivalMode
//-----------------------------------------------------------------------------

class CSurvivalMode
{
public:
	void		*vptr;
	bool		m_bEnabledNow;
	char		unknown[ 7 ];
	bool		m_bEnabled;
};

//-----------------------------------------------------------------------------
// SvenInt user message
//-----------------------------------------------------------------------------

int SVC_SVENINT = 146;

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( setpos, "Set local player's position" )
{
	if ( args.ArgC() > 1 && Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( Globals::localplayer->GetPlayerIndex() );

		if ( !Modules::server->FNullEnt( pPlayer ) && Modules::server->IsValidEntity( pPlayer ) )
		{
			Vector vecOrigin = pPlayer->v.origin;

			vecOrigin.x = (float)atof( args[ 1 ] );

			if ( args.ArgC() > 2 )
			{
				vecOrigin.y = (float)atof( args[ 2 ] );
			}

			if ( args.ArgC() > 3 )
			{
				vecOrigin.z = (float)atof( args[ 3 ] );
			}

			pPlayer->v.origin = vecOrigin;
		}
	}
}

CON_COMMAND( setpos_exact, "Set local player's position" )
{
	if ( args.ArgC() > 1 && Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( Globals::localplayer->GetPlayerIndex() );

		if ( !Modules::server->FNullEnt( pPlayer ) && Modules::server->IsValidEntity( pPlayer ) )
		{
			Vector vecOrigin = pPlayer->v.origin;

			vecOrigin.x = (float)atof( args[ 1 ] );

			if ( args.ArgC() > 2 )
			{
				vecOrigin.y = (float)atof( args[ 2 ] );
			}

			if ( args.ArgC() > 3 )
			{
				vecOrigin.z = (float)atof( args[ 3 ] ) - pPlayer->v.view_ofs.z;
			}

			pPlayer->v.origin = vecOrigin;
		}
	}
}

static Vector savedpos;
CON_COMMAND( savepos, "Save local player's position" )
{
	if ( args.ArgC() > 1 && Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( Globals::localplayer->GetPlayerIndex() );

		if ( !Modules::server->FNullEnt( pPlayer ) && Modules::server->IsValidEntity( pPlayer ) )
		{
			savedpos = pPlayer->v.origin;
			Msg( "savepos: %.6f %.6f %.6f\n", VectorExpand( savedpos ) );
			Globals::gameutils->PrintChatText( "<SvenInt> Saved current position" );
		}
	}
}

CON_COMMAND( setpos_saved, "Set last saved local player's position" )
{
	if ( args.ArgC() > 1 && Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( Globals::localplayer->GetPlayerIndex() );

		if ( !Modules::server->FNullEnt( pPlayer ) && Modules::server->IsValidEntity( pPlayer ) )
		{
			pPlayer->v.origin = savedpos;
		}
	}
}

CON_COMMAND( setvel, "Set local player's position" )
{
	if ( args.ArgC() > 1 && Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( Globals::localplayer->GetPlayerIndex() );

		if ( !Modules::server->FNullEnt( pPlayer ) && Modules::server->IsValidEntity( pPlayer ) )
		{
			Vector vecVelocity = pPlayer->v.velocity;

			vecVelocity.x = (float)atof( args[ 1 ] );

			if ( args.ArgC() > 2 )
			{
				vecVelocity.y = (float)atof( args[ 2 ] );
			}

			if ( args.ArgC() > 3 )
			{
				vecVelocity.z = (float)atof( args[ 3 ] );
			}

			pPlayer->v.velocity = vecVelocity;
		}
	}
}

ConVar sc_sv_disable_spread( "sc_sv_disable_spread", "0", FCVAR_EXTDLL, "Disables spread on server-side" );

//-----------------------------------------------------------------------------
// User message hook
//-----------------------------------------------------------------------------

static int UserMsgHook_SvenInt( const char *pszUserMsg, int iSize, void *pBuffer )
{
	CMessageBuffer message( pszUserMsg, pBuffer, iSize, true );

	int type = message.ReadByte();

	if ( type == SVENINT_COMM_SETCVAR )
	{
		cvar_t *pCvar;
		const char *pszCvar = message.ReadString();

		if ( ( pCvar = Globals::cvar->FindCvar( pszCvar ) ) != NULL )
		{
			const char *pszValue = message.ReadString();
			Globals::cvar->SetValue( pCvar, pszValue );
		}
	}
	else if ( type == SVENINT_COMM_EXECUTE )
	{
		const char *pszCommand = message.ReadString();
		Globals::cl_enginefuncs->pfnClientCmd( pszCommand );
	}
	else if ( type == SVENINT_COMM_TIMER )
	{
		float time = message.ReadCoord();
		Features::timer->ShowTimer( time, false );
	}
	else if ( type == SVENINT_COMM_TIMESCALE )
	{
		extern ConVar sc_st_ignore_timescale;

		bool notify = !!message.ReadByte();

		float framerate = Long32ToFloat( message.ReadLong() );
		float fpsmax = Long32ToFloat( message.ReadLong() );
		float min_frametime = Long32ToFloat( message.ReadLong() );

		if ( !sc_st_ignore_timescale.GetBool() &&
			 !Globals::demoplayback &&
			 !Modules::server->Host_IsServerActive() )
		{
			Features::timescale->SetTimescale_Comm( notify, framerate, fpsmax, min_frametime );
		}
	}
	else if ( type == SVENINT_COMM_DISPLAY_PLAYER_HULL )
	{
		struct
		{
			unsigned char client : 6;
			unsigned char dead : 1;
			unsigned char duck : 1;
		} displayInfo;

		Vector vecOrigin;

		*(unsigned char *)&displayInfo = message.ReadByte();

		vecOrigin.x = Long32ToFloat( message.ReadLong() );
		vecOrigin.y = Long32ToFloat( message.ReadLong() );
		vecOrigin.z = Long32ToFloat( message.ReadLong() );

		//if ( !g_bPlayingbackDemo )
		//{
			Features::playerhulls->DrawPlayerHull_Comm( displayInfo.client, displayInfo.dead, vecOrigin, !!displayInfo.duck );
		//}
	}
	else if ( type == SVENINT_COMM_SCRIPTS )
	{
		int scriptMsgType = message.ReadByte();

		if ( !Globals::demoplayback && scriptMsgType == 0 ) // signal from the server
		{
			Modules::scripts->Callbacks()->OnServerSignal( (int)message.ReadLong() );
		}
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Network message 'ServerInfo' event
//-----------------------------------------------------------------------------

void CServerModule::OnServerInfo( void )
{
	if ( !Modules::server->Host_IsServerActive() )
		return;

	SVC_SVENINT = Globals::sv_enginefuncs->pfnRegUserMsg( "SvenInt", -1 );
}

//-----------------------------------------------------------------------------
// ClientPutInServer event
//-----------------------------------------------------------------------------

void CServerModule::OnClientPutInServer( edict_t *pPlayer )
{
	Globals::sv_enginefuncs->pfnMessageBegin( MSG_ONE, SVC_NEWUSERMSG, NULL, pPlayer );
		Globals::sv_enginefuncs->pfnWriteByte( SVC_SVENINT );
		Globals::sv_enginefuncs->pfnWriteByte( 255 );
		Globals::sv_enginefuncs->pfnWriteLong( 0x6E657653 ); // nevS
		Globals::sv_enginefuncs->pfnWriteLong( 0x00746E49 ); // tnI
		Globals::sv_enginefuncs->pfnWriteLong( 0x0 );
		Globals::sv_enginefuncs->pfnWriteLong( 0x0 );
	Globals::sv_enginefuncs->pfnMessageEnd();

	Features::timescale->SendTimescale( pPlayer );
}

//-----------------------------------------------------------------------------
// Wrapper for Host_IsServerActive
//-----------------------------------------------------------------------------

bool CServerModule::Host_IsServerActive( void )
{
	return !!function_cast<qboolean ( __cdecl * )( void )>( GameData::Pointers::Engine::Host_IsServerActive )( );
}

//-----------------------------------------------------------------------------
// FNullEnt
//-----------------------------------------------------------------------------

bool CServerModule::FNullEnt( edict_t *pEntity )
{
	return pEntity == NULL || Globals::sv_enginefuncs->pfnEntOffsetOfPEntity( pEntity ) == 0;
}

//-----------------------------------------------------------------------------
// IsValidEntity
//-----------------------------------------------------------------------------

bool CServerModule::IsValidEntity( edict_t *pEntity )
{
	if ( pEntity == NULL || pEntity->free || pEntity->pvPrivateData == NULL || ( pEntity->v.flags & FL_KILLME ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Checks whether survival mode enabled or not
//-----------------------------------------------------------------------------

bool CServerModule::IsSurvivalModeEnabled( void )
{
	CSurvivalMode *pSurvivalMode = reinterpret_cast<CSurvivalMode *>( GetSurvivalModeInstance() );
	if ( pSurvivalMode != NULL )
	{
		return pSurvivalMode->m_bEnabled;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Enables survival mode
//-----------------------------------------------------------------------------

bool CServerModule::EnableSurvivalMode( void )
{
	CSurvivalMode *pSurvivalMode = reinterpret_cast<CSurvivalMode *>( GetSurvivalModeInstance() );
	if ( pSurvivalMode == NULL )
		return false;

	bool bEnabled = pSurvivalMode->m_bEnabled;
	if ( bEnabled )
		return false;

	pSurvivalMode->m_bEnabledNow = ( pSurvivalMode->m_bEnabled == false );

	function_cast<void ( __cdecl * )( void )>( GameData::Pointers::Server::toggle_survival_mode_Callback )();
	return true;
}

//-----------------------------------------------------------------------------
// Disables survival mode
//-----------------------------------------------------------------------------

bool CServerModule::DisableSurvivalMode( void )
{
	CSurvivalMode *pSurvivalMode = reinterpret_cast<CSurvivalMode *>( GetSurvivalModeInstance() );
	if ( pSurvivalMode == NULL )
		return false;

	bool bEnabled = pSurvivalMode->m_bEnabled;
	if ( !bEnabled )
		return false;

	pSurvivalMode->m_bEnabledNow = ( pSurvivalMode->m_bEnabled == false );

	function_cast<void( __cdecl * )( void )>( GameData::Pointers::Server::toggle_survival_mode_Callback )( );
	return true;
}

//-----------------------------------------------------------------------------
// Instance of survival mode
//-----------------------------------------------------------------------------

void *CServerModule::GetSurvivalModeInstance( void )
{
	void **ppSurvivalModeInstance = (void **)GameData::Pointers::Server::pSurvivalModeInstance;
	if ( ppSurvivalModeInstance == NULL )
		return NULL;

	void *pSurvivalModeInstance = *ppSurvivalModeInstance;
	if ( ppSurvivalModeInstance == NULL )
		return NULL;

	return pSurvivalModeInstance;
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

CServerModule::CServerModule()
{
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CServerModule::Init( void )
{
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( setpos ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( setpos_exact ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( savepos ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( setpos_saved ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( setvel ) );
	Globals::cvar->RegisterConCommand( &sc_sv_disable_spread );

	Globals::cl_enginefuncs->pfnHookUserMsg( "SvenInt", UserMsgHook_SvenInt );

	usermsg_t *pUserMsg = const_cast<usermsg_t *>( Globals::gameutils->FindUserMessage( "SvenInt" ) );
	if ( pUserMsg != NULL )
	{
		DevMsg( "[SvenInt::Server] Added user message \"SvenInt\" (0x%X) (%d)\n", pUserMsg, pUserMsg->msgid );
		pUserMsg->function = UserMsgHook_SvenInt;
		if ( pUserMsg->msgid > 0 )
			SVC_SVENINT = pUserMsg->msgid;
	}
	else
	{
		Warning2( "[SvenInt::Server] Failed to add user message \"SvenInt\"\n" );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CServerModule::Shutdown( void )
{
	usermsg_t *pUserMsg = const_cast<usermsg_t *>( Globals::gameutils->FindUserMessage( "SvenInt" ) );
	if ( pUserMsg != NULL )
	{
		pUserMsg->function = NULL;
		DevMsg( "[SvenInt::Server] Removed callback function on user message \"SvenInt\" (0x%X) (%d)\n", pUserMsg, pUserMsg->msgid );
	}

	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( setpos ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( setpos_exact ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( savepos ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( setpos_saved ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( setvel ) );
	Globals::cvar->UnregisterConCommand( &sc_sv_disable_spread );
}
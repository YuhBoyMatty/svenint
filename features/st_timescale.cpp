// SvenInt (c) Sw1ft
// st_timescale.cpp

#include "stdafx.h"
#include "st_timescale.h"
#include "modules/menu.h"
#include "modules/server.h"

#include <cmath>

using namespace Globals;

//-----------------------------------------------------------------------------
// Feature gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERN( host_framerate_patch, "74 ? DD ? B8" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CTimescale, timescale, "Speedrun Tools", "Timescale" );

static bool gIgnoreCvarChange = false;
static bool gNotifyTimescaleChanged = false;

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar st_ignore_fps_change( "st_ignore_fps_change", "0", FCVAR_EXTDLL, "Ignore FPS change" );
ConVar st_ignore_timescale( "st_ignore_timescale", "0", FCVAR_EXTDLL, "Ignore timescale set/change" );
ConVar st_min_frametime( "st_min_frametime", "0", FCVAR_EXTDLL, "Min frametime to run a frame" );
ConVar st_transmit_timescale( "st_transmit_timescale", "1", FCVAR_EXTDLL, "Transmit to all clients current timescale" );

CON_COMMAND( st_timescale, "Set timescale" )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->SetTimescale( (float)atof( args[ 1 ] ) );
		gNotifyTimescaleChanged = true;
	}
	else
	{
		if ( st_min_frametime.GetFloat() != 0.f )
			Msg( "Current timescale: %.3f\n", 1.f / ( st_min_frametime.GetFloat() * GameData::Cvars::fps_max->value ) );
		else
			Msg( "Current timescale: 1.000\n" );
	}
}

static void CvarChangeHook_fps_max( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	if ( st_ignore_fps_change.GetBool() || gIgnoreCvarChange || flOldValue == pCvar->value )
		return;

	if ( st_min_frametime.GetFloat() != 0.f )
	{
		float timescale = 1.f / ( st_min_frametime.GetFloat() * flOldValue );
		float multiplier = 1.f / timescale;

		st_min_frametime.SetValue( multiplier / pCvar->value );
		cvar->SetValue( GameData::Cvars::host_framerate, 1.f / pCvar->value );

		gameutils->PrintChatText( "<SvenInt> Automatically adjusted timescale %.2f for %d fps\n", timescale, (int)pCvar->value );
	}
}

//-----------------------------------------------------------------------------
// Send timescale to everyone
//-----------------------------------------------------------------------------

void CTimescale::BroadcastTimescale( void )
{
	if ( cls->state != ca_active || !Modules::server->Host_IsServerActive() || GameData::Cvars::fps_max->value == 20.f )
		return;

	if ( st_transmit_timescale.GetBool() )
	{
		sv_enginefuncs->pfnMessageBegin( MSG_BROADCAST, SVC_SVENINT, NULL, NULL );
			sv_enginefuncs->pfnWriteByte( SVENINT_COMM_TIMESCALE );
			sv_enginefuncs->pfnWriteByte( gNotifyTimescaleChanged ? 1 : 0 );
			sv_enginefuncs->pfnWriteLong( FloatToLong32( GameData::Cvars::host_framerate->value ) );
			sv_enginefuncs->pfnWriteLong( FloatToLong32( GameData::Cvars::fps_max->value ) );
			sv_enginefuncs->pfnWriteLong( FloatToLong32( st_min_frametime.GetFloat() ) );
		sv_enginefuncs->pfnMessageEnd();
	}

	gNotifyTimescaleChanged = false;
}

//-----------------------------------------------------------------------------
// Send timescale to a single player
//-----------------------------------------------------------------------------

void CTimescale::SendTimescale( edict_t *pPlayer )
{
	if ( !Modules::server->Host_IsServerActive() )
		return;
	
	sv_enginefuncs->pfnMessageBegin( MSG_ONE_UNRELIABLE, SVC_SVENINT, NULL, pPlayer );
		sv_enginefuncs->pfnWriteByte( SVENINT_COMM_TIMESCALE );
		sv_enginefuncs->pfnWriteByte( 1 );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( GameData::Cvars::host_framerate->value ) );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( GameData::Cvars::fps_max->value ) );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( st_min_frametime.GetFloat() ) );
	sv_enginefuncs->pfnMessageEnd();

	gNotifyTimescaleChanged = false;
}

//-----------------------------------------------------------------------------
// Set timescale
//-----------------------------------------------------------------------------

void CTimescale::SetTimescale( float timescale )
{
	if ( timescale == 1.f )
	{
		st_min_frametime.SetValue( 0.f );
		cvar->SetValue( GameData::Cvars::host_framerate, 0.f );
		return;
	}
	else if ( timescale > 1.f )
	{
		Msg( "Timescale can't be bigger than 1\n" );
		return;
	}
	else if ( timescale <= 0.f )
	{
		Msg( "Timescale must be bigger than 0\n" );
		return;
	}

	float multiplier = 1.f / timescale;

	st_min_frametime.SetValue( multiplier / GameData::Cvars::fps_max->value );
	cvar->SetValue( GameData::Cvars::host_framerate, 1.f / GameData::Cvars::fps_max->value );

	gameutils->PrintChatText( "<SvenInt> Timescale has been set to %.2f\n", timescale );
}

//-----------------------------------------------------------------------------
// Set timescale from SvenInt user message
//-----------------------------------------------------------------------------

void CTimescale::SetTimescale_Comm( bool notify, float framerate, float fpsmax, float min_frametime )
{
	gIgnoreCvarChange = true;

	cvar->SetValue( GameData::Cvars::host_framerate, framerate );
	cvar->SetValue( GameData::Cvars::fps_max, fpsmax );

	st_min_frametime.SetValue( min_frametime );

	if ( notify )
	{
		if ( min_frametime != 0.f )
			gameutils->PrintChatText( "<SvenInt-Comm> Timescale has been changed to %.2f\n", 1.f / ( min_frametime * fpsmax ) );
		else
			gameutils->PrintChatText( "<SvenInt-Comm> Timescale has been changed to 1.0\n" );
	}

	gIgnoreCvarChange = false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CTimescale::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	auto time = pEvent->GetArg<float>( "time" );
	auto simulate = pEvent->GetReturn<qboolean>();

	const float flMinFrametime = st_min_frametime.GetFloat();

	if ( !bPostCall )
	{
		if ( flMinFrametime == 0.f )
		{
			m_dbTimeCounter = 0.0;
			m_bUsePassedTime = false;
			return kHookContinue;
		}

		m_dbTimeCounter += time;

		if ( m_dbTimeCounter < flMinFrametime )
		{
			simulate = 0;
			return kHookSupercede;
		}

		if ( !m_bUsePassedTime )
			time = static_cast<float>( m_dbTimeCounter );

		return kHookHandled;
	}
	else if ( pEvent->GetHookResult() == kHookHandled )
	{
		if ( simulate )
		{
			m_bUsePassedTime = false;
			m_dbTimeCounter = std::fmod( m_dbTimeCounter, flMinFrametime );

			if ( cls->state == ca_active )
				BroadcastTimescale();
		}
		else
		{
			m_bUsePassedTime = true;
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CTimescale::CTimescale( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_dbTimeCounter = 0.0;
	m_bUsePassedTime = false;

	m_pJumpOpCode = NULL;
	m_PatchedJumpOpCode = 0x9090;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CTimescale::Load( void )
{
	Modules::menu->BindFeature( this, false );

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		m_pJumpOpCode = (uint16_t *)gamedata->FindRVA( GameData::Modules::Engine, "Engine", "host_framerate (Patch)" );
		if ( m_pJumpOpCode == NULL )
			return false;
	}
	else
	{
	#ifdef WIN32
		m_pJumpOpCode = (uint16_t *)MemoryUtils()->FindPatternWithin( GameData::Modules::Engine,
																	  FeaturesGameData::Patterns::Engine::host_framerate_patch,
																	  GameData::Pointers::Engine::Host_FilterTime,
																	  (uint8_t *)GameData::Pointers::Engine::Host_FilterTime + 128 );
	#endif

		FEATURE_CHECK_SYMBOL_PATTERN( m_pJumpOpCode, "host_framerate (Patch)" );
	}

	if ( m_pJumpOpCode != NULL )
		m_PatchedJumpOpCode = *m_pJumpOpCode;

	GAMEDATA_DUMP_FILE_OFFSET( "host_framerate (Patch)", m_pJumpOpCode, GameData::Modules::Engine );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CTimescale::PostLoad( void )
{
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookCall, kHookPriorityHigh );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall, kHookPriorityHigh );

	gamehooks->HookCvarChange( GameData::Cvars::fps_max, CvarChangeHook_fps_max );

	MemoryUtils()->VirtualProtect( m_pJumpOpCode, sizeof( uint16_t ), PAGE_EXECUTE_READWRITE, NULL );
	*m_pJumpOpCode = 0x9090; // nopu nopu

	FEATURE_REGISTER_CCMD( st_timescale );
	FEATURE_REGISTER_CVAR( st_ignore_fps_change );
	FEATURE_REGISTER_CVAR( st_ignore_timescale );
	FEATURE_REGISTER_CVAR( st_min_frametime );
	FEATURE_REGISTER_CVAR( st_transmit_timescale );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CTimescale::Unload( void )
{
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );

	gamehooks->UnhookCvarChange( GameData::Cvars::fps_max, CvarChangeHook_fps_max );

	*m_pJumpOpCode = m_PatchedJumpOpCode;

	FEATURE_UNREGISTER_CCMD( st_timescale );
	FEATURE_UNREGISTER_CVAR( st_ignore_fps_change );
	FEATURE_UNREGISTER_CVAR( st_ignore_timescale );
	FEATURE_UNREGISTER_CVAR( st_min_frametime );
	FEATURE_UNREGISTER_CVAR( st_transmit_timescale );
}

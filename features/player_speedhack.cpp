// SvenInt (c) Sw1ft
// player_speedhack.cpp

#include "stdafx.h"
#include "player_speedhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

#ifdef WIN32
DECLARE_HOOK( BOOL, WINAPI, QueryPerformanceCounter, LARGE_INTEGER * );
#endif

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpeedhack, speedhack, "Player", "Speedhack" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

#ifdef WIN32
ConVar sc_app_speed( "sc_app_speed", "1", FCVAR_EXTDLL, "Speed of application", true, 0.1f, false, FLT_MAX );
#endif
ConVar sc_speedhack( "sc_speedhack", "1", FCVAR_EXTDLL, "Game's speed", true, 0.f, false, FLT_MAX );
ConVar sc_speedhack_ltfx( "sc_speedhack_ltfx", "0", FCVAR_EXTDLL, "Engine's time-based speed", true, -1.f, false, FLT_MAX );

//-----------------------------------------------------------------------------
// QueryPerformanceCounter
//-----------------------------------------------------------------------------

#ifdef WIN32
DECLARE_FUNC( BOOL, WINAPI, HOOKED_QueryPerformanceCounter, LARGE_INTEGER *lpPerformanceCount )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
		return ORIG_QueryPerformanceCounter( lpPerformanceCount );

	static LONGLONG oldfakevalue = 0;
	static LONGLONG oldrealvalue = 0;

	LONGLONG newvalue;

	if ( oldfakevalue == 0 || oldrealvalue == 0 )
	{
		oldfakevalue = lpPerformanceCount->QuadPart;
		oldrealvalue = lpPerformanceCount->QuadPart;
	}

	BOOL result = ORIG_QueryPerformanceCounter( lpPerformanceCount );

	double flFactor = (double)THIS_FEATURE()->GetAppSpeed();
	if ( flFactor > 1.f )
		flFactor = ceil( flFactor );

	newvalue = lpPerformanceCount->QuadPart;
	newvalue = oldfakevalue + (LONGLONG)( ( newvalue - oldrealvalue ) * flFactor );

	oldrealvalue = lpPerformanceCount->QuadPart;
	oldfakevalue = newvalue;

	lpPerformanceCount->QuadPart = newvalue;

	return result;
}
#endif

//-----------------------------------------------------------------------------
// Gets QueryPerformanceCounter
//-----------------------------------------------------------------------------

#ifdef WIN32
QueryPerformanceCounterWinFn CSpeedhack::GetQueryPerformanceCounter( void )
{
	return ORIG_QueryPerformanceCounter;
}
#endif

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpeedhack::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove event
	SetGameSpeed( m_pGameSpeed->GetFloat() );
	SetLTFX( m_pLTFX->GetFloat() );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpeedhack::CSpeedhack( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pGameSpeed = NULL;
	m_pLTFX = NULL;
#ifdef WIN32
	m_pAppSpeed = NULL;
#endif

	m_dbGameSpeed = NULL;

#ifdef WIN32
	m_pfnQueryPerformanceCounter = NULL;
	m_hQueryPerformanceCounter = DETOUR_INVALID_HANDLE;
#endif
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpeedhack::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookCall, kHookPriorityHigh );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpeedhack::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpeedhack::Load( void )
{
	bool bOK = true;
	ud_t inst;

	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pGameSpeed = Modules::menu->AddParamFloat( this, "GameSpeed", NULL, 1.f, 0.f, 100000.f );
	m_pLTFX = Modules::menu->AddParamFloat( this, "LTFX", NULL, 0.f, -1.f, 10.f );
#ifdef WIN32
	m_pAppSpeed = Modules::menu->AddParamFloat( this, "AppSpeed", NULL, 1.f, 0.1f, 100.f );
#endif

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
	#ifdef WIN32
		m_dbGameSpeed = (double *)gamedata->FindRVA( GameData::Modules::Engine, "Engine", "gamespeed" );
	#else
		m_dbGameSpeed = (float *)gamedata->FindRVA( GameData::Modules::Engine, "Engine", "gamespeed" );
	#endif
		if ( m_dbGameSpeed == NULL )
			bOK = false;
	}
	else
	{
	#ifdef WIN32
		void *pTextureLoadAddress = MemoryUtils()->FindString( GameData::Modules::Engine, "Texture load: %6.1fms" );
		FEATURE_CHECK_SYMBOL_STATUS( pTextureLoadAddress, "TextureLoadAddress" );
		if ( bOK )
		{
			void *pTextureLoad = MemoryUtils()->FindAddress( GameData::Modules::Engine, pTextureLoadAddress );
			FEATURE_CHECK_SYMBOL_STATUS( pTextureLoad, "TextureLoad" );
			if ( bOK )
			{
				// g_dbGameSpeed
				MemoryUtils()->InitDisasm( &inst, (uint8_t *)pTextureLoad - 0xA, 32, 15 );
				MemoryUtils()->Disassemble( &inst );

				if ( inst.mnemonic == UD_Ifmul && inst.operand[ 0 ].type == UD_OP_MEM )
					m_dbGameSpeed = reinterpret_cast<double *>( inst.operand[ 0 ].lval.udword );

				FEATURE_CHECK_SYMBOL_STATUS( m_dbGameSpeed, "gamespeed" );
			}
		}
	#else
		bOK = false;
	#endif
	}

	if ( bOK )
	{
		MemoryUtils()->VirtualProtect( m_dbGameSpeed, sizeof( double ), PAGE_EXECUTE_READWRITE, NULL );
	}
	else
	{
		PrintWarning2( "Speedhack method based on %s's speed is not available\n", "game" );
		bOK = true;
	}

	Modules::menu->BindConVar( m_pGameSpeed, &sc_speedhack );
	Modules::menu->BindConVar( m_pLTFX, &sc_speedhack_ltfx );
#ifdef WIN32
	Modules::menu->BindConVar( m_pAppSpeed, &sc_app_speed );

	m_pfnQueryPerformanceCounter = (void *)QueryPerformanceCounter;
	FEATURE_CHECK_SYMBOL_STATUS( m_pfnQueryPerformanceCounter, "QueryPerformanceCounter" );
	if ( !bOK )
		PrintWarning2( "Speedhack method based on %s's speed is not available\n", "application" );
#endif

	GAMEDATA_DUMP_FILE_OFFSET( "gamespeed", m_dbGameSpeed, GameData::Modules::Engine );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSpeedhack::PostLoad( void )
{
#ifdef WIN32
	m_hQueryPerformanceCounter = Detours()->DetourFunction( m_pfnQueryPerformanceCounter, HOOKED_QueryPerformanceCounter, GET_FUNC_PTR( ORIG_QueryPerformanceCounter ) );

	cvar->RegisterConCommand( &sc_app_speed );
#endif
	cvar->RegisterConCommand( &sc_speedhack );
	cvar->RegisterConCommand( &sc_speedhack_ltfx );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSpeedhack::Unload( void )
{
	SetGameSpeed( 1.0 );

#ifdef WIN32
	Detours()->RemoveDetour( m_hQueryPerformanceCounter );

	cvar->UnregisterConCommand( &sc_app_speed );
#endif
	cvar->UnregisterConCommand( &sc_speedhack );
	cvar->UnregisterConCommand( &sc_speedhack_ltfx );
}

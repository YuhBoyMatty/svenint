// SvenInt (c) Sw1ft
// misc_game_patches.cpp

#include "stdafx.h"
#include "misc_game_patches.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( int, CALLCONV_CDECL, SDL_GL_ExtensionSupported, const char * );
DECLARE_HOOK( const char *, CALLCONV_STDCALL, glGetString, int );
DECLARE_HOOK( bool, CALLCONV_CDECL, CCamera__Process, void *, void * );
DECLARE_HOOK( void, CALLCONV_CDECL, CL_ComputeClientInterpolationAmount, usercmd_t *cmd );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERNS( Host_FilterTime_31fps,
							 "5.25",
							 "D9 ? ? ? ? ? EB ? D8 ? DF E0 F6 C4 ? 75 ? DD ? D9 C0",
							 "5.11",
							 "D9 ? ? ? ? ? D9 ? ? D9 ? ? DD ? ? ? ? ? D8 ? DF E0" );

			DEFINE_PATTERN( R_DrawViewModel_glClear, "68 ? ? ? ? FF 15 ? ? ? ? 83 3D ? ? ? ? ? 0F 95 C3" );

			DEFINE_PATTERNS( CL_ComputeClientInterpolationAmount,
							 "5.25",
							 "51 D9 ? ? ? ? ? D9 ? ? ? ? ? D8 ? DF E0 56 BE",
							 "5.11",
							 "51 D9 ? ? ? ? ? 56 D9 ? ? ? ? ? 57" );
		}

		namespace Client
		{
			DEFINE_PATTERN( CCamera__Process, "56 8B 74 24 0C 57 8B 7C 24 0C 81" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CGamePatches, gamepatches, "Misc", "Game Patches" );

//-----------------------------------------------------------------------------
// Disable multitexture to increase framerate
// Credits: @xWhitey
//-----------------------------------------------------------------------------

#define DISABLED_EXTENSION "GL_ARB_multitexture"
#define FAKE_EXTENSION "GL_ARB_valvefixplz)"
#ifndef GL_EXTENSIONS
#define GL_EXTENSIONS 0x1F03
#endif //!GL_EXTENSIONS

DECLARE_FUNC( int, CALLCONV_CDECL, HOOKED_SDL_GL_ExtensionSupported, const char *pszExtension )
{
	if ( !strnicmp( pszExtension, DISABLED_EXTENSION, sizeof( DISABLED_EXTENSION ) - 1 ) )
		return 0;

	return ORIG_SDL_GL_ExtensionSupported( pszExtension );
}

DECLARE_FUNC( const char *, CALLCONV_STDCALL, HOOKED_glGetString, int type )
{
	const char *pszResult = ORIG_glGetString( type );

	if ( type == GL_EXTENSIONS )
	{
		const char *pszMultitexture = strstr( pszResult, DISABLED_EXTENSION );
		if ( pszMultitexture != NULL )
		{
			const char *pszFakeExtension = FAKE_EXTENSION;
			memcpy( (void *)pszMultitexture, pszFakeExtension, sizeof( FAKE_EXTENSION ) - 1 );
		}

		Detours()->PauseDetour( THIS_FEATURE()->GetGLStringDetour() );
	}

	return pszResult;
}

//-----------------------------------------------------------------------------
// Tertiary Attack Glitch dummy method
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, DUMMY_CBasePlayerWeapon__TertiaryAttack, void *thisptr )
{
	return;
}

//-----------------------------------------------------------------------------
// CCamera::Process hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( bool, CALLCONV_CDECL, HOOKED_CCamera__Process, void *thisptr, void *unk )
{
	if ( cl_enginefuncs->GetLocalPlayer() == NULL )
		return true;

	if ( THIS_FEATURE()->IgnoreAltInThirdPerson() )
		return true; // a1 does good, unlike Sniper.

	return ORIG_CCamera__Process( thisptr, unk );
}

//-----------------------------------------------------------------------------
// CL_ComputeClientInterpolationAmount hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, CALLCONV_CDECL, HOOKED_CL_ComputeClientInterpolationAmount, usercmd_t *cmd )
{
	static float flInterpolationAmount = 0.1f;
	static cvar_t *ex_interp = NULL;

	if ( ex_interp == NULL )
	{
		ex_interp = cvar->FindCvar( "ex_interp" );
		if ( ex_interp == NULL )
			return;
	}

	float interp_sec = ex_interp->value;

	// CL_DriftInterpolationAmount
	if ( interp_sec != flInterpolationAmount )
	{
		float maxmove = (float)*host_frametime * 0.05f;
		float diff = interp_sec - flInterpolationAmount;

		if ( diff > 0.f )
		{
			if ( diff > maxmove )
				diff = maxmove;
		}
		else
		{
			diff = -diff;

			if ( diff > maxmove )
				diff = -maxmove;
		}

		flInterpolationAmount += diff;
	}

	interp_sec = Q_clamp( 0.f, flInterpolationAmount * 1000.f, 100.f );
	cmd->lerp_msec = (short)interp_sec;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

bool CGamePatches::GuessTertiaryAttackVtidx( void )
{
	if ( gamedata->Initialized() )
	{
		m_vtidx_CBasePlayerWeapon_TertiaryAttack = gamedata->FindOffset( GameData::Modules::Client, "Client", "CBasePlayerWeapon::TertiaryAttack" );
		m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server = gamedata->FindOffset( GameData::Modules::Server, "Server", "CBasePlayerWeapon::TertiaryAttack" );

		if ( m_vtidx_CBasePlayerWeapon_TertiaryAttack == ~0 )
			return false;
		if ( m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server == ~0 )
			return false;
	}
	else
	{
	#ifdef WIN32
		switch ( SVEN_VERSION() )
		{
		case SVEN_VERSION_CHECK( 5, 26, 0 ):
		{
			m_vtidx_CBasePlayerWeapon_TertiaryAttack = 150;
			m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server = 153;
			break;
		}

		case SVEN_VERSION_CHECK( 5, 25, 0 ):
		{
			m_vtidx_CBasePlayerWeapon_TertiaryAttack = 150;
			m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server = 151;
			break;
		}

		default:
			PrintWarning( "Unable to guess index of virtual method \"CBasePlayerWeapon::TertiaryAttack\" for the game & client DLL (game version: %d)\n", gameversion );
			return false;
		}
	#else
		return false;
	#endif
	}

	return true;
}

//-----------------------------------------------------------------------------
// InitTertiaryAttackPatches
//-----------------------------------------------------------------------------

void CGamePatches::InitTertiaryAttackPatches( void )
{
	Detours()->AutoPauseDetours( true );

	static void *origs[ 4 ], *origs_server[ 5 ];
	void *vtables[ 4 ] = { 0 }, *vtables_server[ 5 ] = { 0 };

	auto fpGaussVTable = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Client, "CGauss" ); } );
	auto fpHandGrenadeVTable = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Client, "CHandGrenade" ); } );
	auto fpMinigunVTable = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Client, "CMinigun" ); } );
	auto fpShockRifleVTable = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Client, "CShockRifle" ); } );

	auto fpGaussVTableServer = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Server, "CGauss" ); } );
	auto fpHandGrenadeVTableServer = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Server, "CHandGrenade" ); } );
	auto fpMinigunVTableServer = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Server, "CMinigun" ); } );
	auto fpShockRifleVTableServer = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Server, "CShockRifle" ); } );
	auto fpEgonVTableServer = std::async( [] { return MemoryUtils()->FindVTable( GameData::Modules::Server, "CEgon" ); } );

	vtables[ 0 ] = fpGaussVTable.get();
	vtables[ 1 ] = fpHandGrenadeVTable.get();
	vtables[ 2 ] = fpMinigunVTable.get();
	vtables[ 3 ] = fpShockRifleVTable.get();

	vtables_server[ 0 ] = fpGaussVTableServer.get();
	vtables_server[ 1 ] = fpHandGrenadeVTableServer.get();
	vtables_server[ 2 ] = fpMinigunVTableServer.get();
	vtables_server[ 3 ] = fpShockRifleVTableServer.get();
	vtables_server[ 4 ] = fpEgonVTableServer.get();

	for ( int i = 0; i < Q_ARRAYSIZE( vtables ); i++ )
	{
		if ( vtables[ i ] == NULL )
			continue;

		void *pDummyInstance = vtables[ i ];
		DetourHandle_t hDetour = Detours()->DetourVirtualFunction( &pDummyInstance,
																   m_vtidx_CBasePlayerWeapon_TertiaryAttack,
																   DUMMY_CBasePlayerWeapon__TertiaryAttack,
																   origs + i );

		if ( hDetour == DETOUR_INVALID_HANDLE )
			continue;

		m_TertiaryAttackGlitchPatches.push_back( hDetour );
	}

	for ( int i = 0; i < Q_ARRAYSIZE( vtables_server ); i++ )
	{
		if ( vtables_server[ i ] == NULL )
			continue;

		void *pDummyInstance = vtables_server[ i ];
		DetourHandle_t hDetour = Detours()->DetourVirtualFunction( &pDummyInstance,
																   m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server,
																   DUMMY_CBasePlayerWeapon__TertiaryAttack,
																   origs_server + i );

		if ( hDetour == DETOUR_INVALID_HANDLE )
			continue;

		m_TertiaryAttackGlitchPatches.push_back( hDetour );
	}

	Detours()->AutoPauseDetours( false );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CGamePatches::CGamePatches( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pTertiaryAttackGlitch = NULL;
	m_pIgnoreAltInThirdPerson = NULL;

	m_bTertiaryAttackGlitchPatchable = false;
	m_vtidx_CBasePlayerWeapon_TertiaryAttack = ~0;
	m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server = ~0;

	m_pglClearArg1 = NULL;
	m_p31fpsFPU = NULL;
	m_pfnCCamera__Process = NULL;
	m_pfnCL_ComputeClientInterpolationAmount = NULL;

	m_hSDL_GL_ExtensionSupported = DETOUR_INVALID_HANDLE;
	m_hglGetString = DETOUR_INVALID_HANDLE;
	m_hCCamera__Process = DETOUR_INVALID_HANDLE;
	m_hCL_ComputeClientInterpolationAmount = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CGamePatches::OnEnable( void )
{
	if ( m_bTertiaryAttackGlitchPatchable && m_pTertiaryAttackGlitch != NULL && m_pTertiaryAttackGlitch->GetBool() )
	{
		Detours()->EnableSuspendThreads( false );

		for ( const DetourHandle_t &detour : m_TertiaryAttackGlitchPatches )
			Detours()->UnpauseDetour( detour );

		Detours()->EnableSuspendThreads( true );
	}
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CGamePatches::OnDisable( void )
{
	if ( m_bTertiaryAttackGlitchPatchable )
	{
		Detours()->EnableSuspendThreads( false );

		for ( const DetourHandle_t &detour : m_TertiaryAttackGlitchPatches )
			Detours()->PauseDetour( detour );

		Detours()->EnableSuspendThreads( true );
	}
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CGamePatches::Load( void )
{
	bool bOK = true;

	Modules::menu->BindFeature( this );

	m_pIgnoreAltInThirdPerson = Modules::menu->AddParamBool( this, "IgnoreAltInThirdPerson", NULL, true );

	if ( SVEN_VERSION() >= SVEN_VERSION_CHECK( 5, 24, 0 ) )
	{
		if ( GuessTertiaryAttackVtidx() )
		{
			m_pTertiaryAttackGlitch = Modules::menu->AddParamBool( this, "TertiaryAttackGlitch", NULL, true );

			m_bTertiaryAttackGlitchPatchable = true;
			InitTertiaryAttackPatches();
		}
		else
		{
			PrintWarning2( "Tertiary Attack Glitch is unavailable\n" );
		}
	}

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		std::future<void *> fm_pglClearArg1;
		std::future<void *> fm_pfnCCamera__Process;

		MAKE_ASYNC( fm_p31fpsFPU, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "Host_FilterTime (31 FPS Unlock)" ); } );
		MAKE_ASYNC( fm_pfnCL_ComputeClientInterpolationAmount, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "CL_ComputeClientInterpolationAmount" ); } );

		if ( SVEN_VERSION() >= SVEN_VERSION_CHECK( 5, 26, 0 ) )
		{
			fm_pglClearArg1 = std::async( [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "R_DrawViewModel (glClear)" ); } );
		}
		if ( SVEN_VERSION() == SVEN_VERSION_CHECK( 5, 26, 0 ) )
		{
			fm_pfnCCamera__Process = std::async( [] { return gamedata->FindRVA( GameData::Modules::Client, "Client", "CCamera::Process" ); } );
		}

		m_p31fpsFPU = fm_p31fpsFPU.get();
		m_pfnCL_ComputeClientInterpolationAmount = fm_pfnCL_ComputeClientInterpolationAmount.get();

		if ( m_pfnCL_ComputeClientInterpolationAmount == NULL )
			PrintWarning2( "Interpolation bounds weren't patched\n" );
		if ( fm_pglClearArg1.valid() )
			m_pglClearArg1 = fm_pglClearArg1.get();
		if ( fm_pfnCCamera__Process.valid() )
			m_pfnCCamera__Process = fm_pfnCCamera__Process.get();
	}
	else
	{
	#ifdef WIN32
		int patternIndex;
		DEFINE_PATTERNS_FUTURE( fCL_ComputeClientInterpolationAmount );
		DEFINE_PATTERNS_FUTURE( fHost_FilterTime_31fps );

		MemoryUtils()->FindPatternAsync( GameData::Modules::Engine,
										 FeaturesGameData::Patterns::Engine::CL_ComputeClientInterpolationAmount,
										 fCL_ComputeClientInterpolationAmount );

		MemoryUtils()->FindPatternAsync( GameData::Modules::Engine,
										 FeaturesGameData::Patterns::Engine::Host_FilterTime_31fps,
										 fHost_FilterTime_31fps );

		m_pfnCL_ComputeClientInterpolationAmount = MemoryUtils()->GetPatternFutureValue( fCL_ComputeClientInterpolationAmount, &patternIndex );
		FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCL_ComputeClientInterpolationAmount,
											  "CL_ComputeClientInterpolationAmount",
											  FeaturesGameData::Patterns::Engine::CL_ComputeClientInterpolationAmount,
											  patternIndex );

		if ( !bOK )
		{
			PrintWarning2( "Interpolation bounds weren't patched\n" );
		}

		if ( SVEN_VERSION() >= SVEN_VERSION_CHECK( 5, 26, 0 ) )
		{
			m_pglClearArg1 = MemoryUtils()->FindPattern( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::R_DrawViewModel_glClear );
			FEATURE_CHECK_SYMBOL_PATTERN_STATUS( m_pglClearArg1, "R_DrawViewModel (glClear)" );
		}

		// Sniper is some kind of joke ngl, fix camera crash when we're not even playing
		// Credits: @xWhitey
		if ( SVEN_VERSION() == SVEN_VERSION_CHECK( 5, 26, 0 ) )
		{
			m_pfnCCamera__Process = MemoryUtils()->FindPattern( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CCamera__Process );
		}

		m_p31fpsFPU = MemoryUtils()->GetPatternFutureValue( fHost_FilterTime_31fps, &patternIndex );
		FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_p31fpsFPU,
											  "Host_FilterTime (Forced 31 FPS)",
											  FeaturesGameData::Patterns::Engine::Host_FilterTime_31fps,
											  patternIndex );
	#endif
	}

	GAMEDATA_DUMP_FILE_OFFSET( "Host_FilterTime (Forced 31 FPS)", m_p31fpsFPU, GameData::Modules::Engine );
	GAMEDATA_DUMP_FILE_OFFSET( "R_DrawViewModel (glClear)", m_pglClearArg1, GameData::Modules::Engine );
	GAMEDATA_DUMP_FILE_OFFSET( "CCamera::Process", m_pfnCCamera__Process, GameData::Modules::Client );
	GAMEDATA_DUMP_FILE_OFFSET( "CL_ComputeClientInterpolationAmount", m_pfnCL_ComputeClientInterpolationAmount, GameData::Modules::Engine );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CGamePatches::PostLoad( void )
{
	// Patch depth buffer clear up
	if ( m_pglClearArg1 != NULL )
	{
		int patchValue = 0;
	#ifdef WIN32
		MemoryUtils()->PatchMemory( (uint8_t *)m_pglClearArg1 + 0x1, (uint8_t *)&patchValue, sizeof( patchValue ) );
	#else
		MemoryUtils()->PatchMemory( (uint8_t *)m_pglClearArg1 + 0x3, (uint8_t *)&patchValue, sizeof( patchValue ) );
	#endif
	}

	// Patch locked 31 FPS at game loading
	// Credits: @xWhitey
#ifdef WIN32
	if ( m_p31fpsFPU != NULL && **(float **)( (uint8_t *)m_p31fpsFPU + 2 ) == 31.f )
	{
		MemoryUtils()->VirtualProtect( (float *)( (uint8_t *)m_p31fpsFPU + 0x2 ), sizeof( float * ), PAGE_EXECUTE_READWRITE, NULL );
		*(float **)( (uint8_t *)m_p31fpsFPU + 0x2 ) = &( GameData::Cvars::fps_max->value );
	}
#else
	if ( m_p31fpsFPU != NULL )
	{
		MemoryUtils()->VirtualProtect( m_p31fpsFPU, sizeof( float ), PAGE_EXECUTE_READWRITE, NULL );
		*(float *)( m_p31fpsFPU ) = 200.f;
	}
#endif

	m_hSDL_GL_ExtensionSupported = Detours()->DetourFunctionByName( GameData::Modules::SDL2,
																	"SDL_GL_ExtensionSupported",
																	HOOKED_SDL_GL_ExtensionSupported,
																	GET_FUNC_PTR( ORIG_SDL_GL_ExtensionSupported ) );

	m_hglGetString = Detours()->DetourFunctionByName( GameData::Modules::OpenGL,
													  "glGetString",
													  HOOKED_glGetString,
													  GET_FUNC_PTR( ORIG_glGetString ) );

	if ( m_pfnCCamera__Process != NULL )
	{
		m_hCCamera__Process = Detours()->DetourFunction( m_pfnCCamera__Process, HOOKED_CCamera__Process, GET_FUNC_PTR( ORIG_CCamera__Process ) );
	}

	if ( m_pfnCL_ComputeClientInterpolationAmount != NULL )
	{
		m_hCL_ComputeClientInterpolationAmount = Detours()->DetourFunction( m_pfnCL_ComputeClientInterpolationAmount,
																			HOOKED_CL_ComputeClientInterpolationAmount,
																			GET_FUNC_PTR( ORIG_CL_ComputeClientInterpolationAmount ) );
	}
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CGamePatches::Unload( void )
{
	if ( m_hSDL_GL_ExtensionSupported != DETOUR_INVALID_HANDLE )
		Detours()->RemoveDetour( m_hSDL_GL_ExtensionSupported );

	if ( m_hglGetString != DETOUR_INVALID_HANDLE )
		Detours()->RemoveDetour( m_hglGetString );

	if ( m_pfnCCamera__Process != NULL )
		Detours()->RemoveDetour( m_hCCamera__Process );

	Detours()->RemoveDetour( m_hCL_ComputeClientInterpolationAmount );

	for ( const DetourHandle_t &detour : m_TertiaryAttackGlitchPatches )
		Detours()->RemoveDetour( detour );
	m_TertiaryAttackGlitchPatches.clear();
}

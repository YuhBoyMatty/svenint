// SvenInt (c) Sw1ft
// r_skybox.cpp

#include "stdafx.h"
#include "r_skybox.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( int, __cdecl, R_LoadSkyBoxInt, const char * );

//-----------------------------------------------------------------------------
// Feature patterns & gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERNS_2( R_LoadSkyboxInt,
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 20 01 00 00 53 8B 9C 24 2C 01 00 00",
							   "5.25",
							   "81 EC ? ? 00 00 A1 ? ? ? ? 33 C4 89 84 24 ? ? 00 00 53 8B 9C 24 ? ? 00 00 55 56 57 89 5C 24 24" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSkybox, skybox, "Render", "Skybox" );

static const char *g_szSkyboxes[] =
{
	"-",
	"desert",
	"2desert",
	"desnoon",
	"morning",
	"cliff",
	"dfcliff",
	"dustbowl",
	"sandstone",
	"sky_blu_",
	"sky16",
	"sky35",
	"sky45",
	"tornsky",
	"twildes",
	"crashsite",
	"doom1",
	"dusk",
	"fodrian",
	"night",
	"carnival",
	"theyh2",
	"theyh3",
	"thn",
	"forest512_",
	"tetris",
	"2vs",
	"ac_",
	"arcn",
	"black",
	"coliseum",
	"gmcity",
	"grassy",
	"toon",
	"parallax-errorlf256_",
	"necros-hell256_",
	"space",
	"hplanet",
	"vreality_sky",
	"neb1",
	"neb2b",
	"neb6",
	"neb7",
	"alien1",
	"alien2",
	"alien3",
	"xen8",
	"xen9",
	"xen10"
};

//-----------------------------------------------------------------------------
// R_LoadSkyBoxInt hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, __cdecl, HOOKED_R_LoadSkyBoxInt, const char *pszSkyboxName )
{
	int loaded = ORIG_R_LoadSkyBoxInt( pszSkyboxName );

	THIS_FEATURE()->R_LoadSkyBoxIntEvent( pszSkyboxName, loaded );

	return loaded;
}

//-----------------------------------------------------------------------------
// Skybox management
//-----------------------------------------------------------------------------

void CSkybox::R_LoadSkyBoxIntEvent( const char *pszSkyboxName, int loaded )
{
	if ( m_bLoadingSkybox )
	{
		m_bSkyboxLoaded = loaded;
	}
	else
	{
		SaveOriginalSkybox( pszSkyboxName );
	}
}

void CSkybox::Replace( const char *pszSkyboxName )
{
	strcpy_s( m_szSkyboxName, sizeof( m_szSkyboxName ), pszSkyboxName );

	*m_szCurrentSkyboxName = 0;
	m_bSkyboxReplaced = true;
}

void CSkybox::Reset()
{
	if ( m_bSkyboxReplaced && playermove->movevars() != NULL && *m_szOriginalSkyboxName )
	{
		engineclient->ChangeSkymap( m_szOriginalSkyboxName );
	}

	*m_szSkyboxName = 0;
	*m_szOriginalSkyboxName = 0;
	*m_szCurrentSkyboxName = 0;

	m_bSkyboxReplaced = false;
}

void CSkybox::SaveOriginalSkybox( const char *pszSkyboxName )
{
	strcpy_s( m_szOriginalSkyboxName, sizeof( m_szOriginalSkyboxName ), pszSkyboxName );
	strcpy_s( m_szCurrentSkyboxName, sizeof( m_szCurrentSkyboxName ), pszSkyboxName );
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CSkybox::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( !IsEnabled() )
		return;

	if ( pButton == m_pButtonChangeSkybox )
	{
		if ( m_pSkyboxName->GetInt() > 0 )
			Replace( g_szSkyboxes[ m_pSkyboxName->GetInt() ] );
	}
	else if ( pButton == m_pButtonResetSkybox )
	{
		*m_pSkyboxName->GetCfgProperty()->GetIntRef() = 0;
		Reset();
	}
}

//-----------------------------------------------------------------------------
// Config load callback
//-----------------------------------------------------------------------------

void CSkybox::OnConfigLoad( const char *pszFilename, bool bShaderConfig )
{
	if ( !bShaderConfig || !IsLoaded() )
		return;

	if ( m_pSkyboxName->GetInt() > 0 && m_pSkyboxName->GetInt() < Q_ARRAYSIZE( g_szSkyboxes ) )
		Replace( g_szSkyboxes[ m_pSkyboxName->GetInt() ] );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSkybox::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_VidInit event
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_flNextThinkTime = -1.f;
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		if ( m_bSkyboxReplaced && playermove->movevars() != NULL )
		{
			if ( m_flNextThinkTime > cl_enginefuncs->GetClientTime() )
				return kHookContinue;
			
			m_flNextThinkTime = cl_enginefuncs->GetClientTime() + 0.25f;

			if ( stricmp( m_szSkyboxName, m_szCurrentSkyboxName ) )
			{
				m_bLoadingSkybox = true;

				engineclient->ChangeSkymap( m_szSkyboxName );

				m_bLoadingSkybox = false;

				if ( !m_bSkyboxLoaded )
				{
					m_bSkyboxReplaced = false;

					*m_szSkyboxName = 0;
					*m_szCurrentSkyboxName = 0;
					*m_szOriginalSkyboxName = 0;
				}
				else
				{
					strcpy_s( m_szOriginalSkyboxName, sizeof( movevars_s::skyName ), playermove->movevars()->skyName);
					strcpy_s( m_szCurrentSkyboxName, sizeof( m_szCurrentSkyboxName ), m_szSkyboxName );
				}
			}
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSkybox::CSkybox( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonChangeSkybox = NULL;
	m_pButtonResetSkybox = NULL;
	m_pSkyboxName = NULL;

	m_pfnR_LoadSkyboxInt = NULL;
	m_hR_LoadSkyboxInt = DETOUR_INVALID_HANDLE;

	m_bSkyboxReplaced = false;
	m_bLoadingSkybox = false;
	m_bSkyboxLoaded = false;

	m_flNextThinkTime = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSkybox::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSkybox::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSkybox::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pButtonChangeSkybox = Modules::menu->AddElementButton( this, this, "Change Skybox" ); Modules::menu->AddElementSameLine( this );
	m_pButtonResetSkybox = Modules::menu->AddElementButton( this, this, "Reset Skybox" );
	m_pSkyboxName = Modules::menu->AddParamList( this, "SkyboxName", NULL, 0, " 0 - None\0 1 - desert\0 2 - 2desert\0 3 - desnoon\0 4 - morning\0 5 - cliff\0 6 - dfcliff\0 7 - dustbowl\0 8 - sandstone\0 9 - sky_blu_\0 10 - sky16\0 11 - sky35\0 12 - sky45\0 13 - tornsky\0 14 - twildes\0 15 - crashsite\0 16 - doom1\0 17 - dusk\0 18 - fodrian\0 19 - night\0 20 - carnival\0 21 - theyh2\0 22 - theyh3\0 23 - thn\0 24 - forest512_\0 25 - tetris\0 26 - 2vs\0 27 - ac_\0 28 - arcn\0 29 - black\0 30 - coliseum\0 31 - gmcity\0 32 - grassy\0 33 - toon\0 34 - parallax-errorlf256_\0 35 - necros-hell256_\0 44 - alien2\0 45 - alien3\0 46 - xen8\0 47 - xen9\0 48 - xen10\0\0" );

	FEATURE_REQUIRE_GAMEDATA( engineclient->GetInstance(), "engineclient");

	int patternIndex;
	DEFINE_PATTERNS_FUTURE( fR_LoadSkyboxInt );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::R_LoadSkyboxInt, fR_LoadSkyboxInt );

	m_pfnR_LoadSkyboxInt = MemoryUtils()->GetPatternFutureValue( fR_LoadSkyboxInt, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS( m_pfnR_LoadSkyboxInt,
								   "R_LoadSkyboxInt",
								   FeaturesGameData::Patterns::Engine::R_LoadSkyboxInt,
								   patternIndex );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CSkybox::PostLoad( void )
{
	m_hR_LoadSkyboxInt = Detours()->DetourFunction( m_pfnR_LoadSkyboxInt, HOOKED_R_LoadSkyBoxInt, GET_FUNC_PTR( ORIG_R_LoadSkyBoxInt ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CSkybox::Unload( void )
{
	Detours()->RemoveDetour( m_hR_LoadSkyboxInt );
}
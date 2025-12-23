// SvenInt (c) Sw1ft
// r_fog.cpp

#include "stdafx.h"
#include "r_fog.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( void, CALLCONV_CDECL, R_SetupFrame );
DECLARE_HOOK( void, CALLCONV_CDECL, V_RenderView );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERNS_2( R_SetupFrame,
							   "5.26",
							   "83 EC ? A1 ? ? ? ? 33 C4 89 44 24 20 D9 ? ? ? ? ? D9 ? ? ? ? ? DA",
							   "5.25",
							   "83 EC 24 A1 ? ? ? ? 33 C4 89 44 24 20 33 C0 83 3D ? ? ? ? 01 0F 9F C0 50 E8 ? ? ? ? E8" );

			DEFINE_PATTERNS_2( V_RenderView,
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 F0 01 00 00",
							   "5.25",
							   "81 EC ? ? 00 00 A1 ? ? ? ? 33 C4 89 84 24 ? ? 00 00 D9 EE D9 15" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFog, fog, "Render", "Fog" );

//-----------------------------------------------------------------------------
// R_SetupFrame hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, CALLCONV_CDECL, HOOKED_R_SetupFrame )
{
	ORIG_R_SetupFrame();

	THIS_FEATURE()->R_SetupFrameEvent();
}

//-----------------------------------------------------------------------------
// V_RenderView hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, CALLCONV_CDECL, HOOKED_V_RenderView )
{
	THIS_FEATURE()->V_RenderViewEvent();

	ORIG_V_RenderView();
}

//-----------------------------------------------------------------------------
// R_SetupFrame hook
//-----------------------------------------------------------------------------

void CFog::R_SetupFrameEvent( void )
{
	if ( refparams->waterlevel != WL_EYES )
		return;

	float rgColor[ 3 ] = { 0.f, 0.f, 0.f };

	if ( m_pRemoveWaterFog->GetBool() )
	{
		glDisable( GL_FOG );

		if ( IsEnabled() )
			cl_enginefuncs->pTriAPI->Fog( rgColor, m_pFogStart->GetFloat(), m_pFogEnd->GetFloat(), 0 );
	}
	else if ( IsEnabled() )
	{
		cl_enginefuncs->pTriAPI->Fog( rgColor, m_pFogStart->GetFloat(), m_pFogEnd->GetFloat(), 0 );
	}
}

//-----------------------------------------------------------------------------
// V_RenderView event
//-----------------------------------------------------------------------------

void CFog::V_RenderViewEvent( void )
{
	GLfloat glColor[] =
	{
		m_pFogColor->GetColor()[ 0 ] * 255.0f,
		m_pFogColor->GetColor()[ 1 ] * 255.0f,
		m_pFogColor->GetColor()[ 2 ] * 255.0f,
	};
	
	if ( IsEnabled() )
		cl_enginefuncs->pTriAPI->FogParams( m_pFogDensity->GetFloat() / 200.f, int( m_pFogSkybox->GetBool() ) );

	cl_enginefuncs->pTriAPI->Fog( glColor, m_pFogStart->GetFloat(), m_pFogEnd->GetFloat(), IsEnabled() );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFog::CFog( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pRemoveWaterFog = NULL;
	m_pFogSkybox = NULL;
	m_pFogStart = NULL;
	m_pFogEnd = NULL;
	m_pFogDensity = NULL;
	m_pFogColor = NULL;

	m_pfnR_SetupFrame = NULL;
	m_pfnV_RenderView = NULL;

	m_hR_SetupFrame = DETOUR_INVALID_HANDLE;
	m_hV_RenderView = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFog::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pRemoveWaterFog = Modules::menu->AddParamBool( this, "RemoveWaterFog", NULL, false );
	m_pFogSkybox = Modules::menu->AddParamBool( this, "FogSkybox", NULL, true );
	m_pFogStart = Modules::menu->AddParamFloat( this, "Start", NULL, 0.f, 0.f, 10000.f );
	m_pFogEnd = Modules::menu->AddParamFloat( this, "End", NULL, 1000.f, 0.f, 10000.f );
	m_pFogDensity = Modules::menu->AddParamFloat( this, "Density", NULL, 0.25f, 0.f, 10.f );
	m_pFogColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, Color( 255, 255, 255, 255 ) );

	bool bOK = true;

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		MAKE_ASYNC( fm_pfnR_SetupFrame, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "R_SetupFrame" ); } );
		MAKE_ASYNC( fm_pfnV_RenderView, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "V_RenderView" ); } );

		m_pfnR_SetupFrame = fm_pfnR_SetupFrame.get();
		m_pfnV_RenderView = fm_pfnV_RenderView.get();

		if ( m_pfnR_SetupFrame == NULL )
			bOK = false;
		if ( m_pfnV_RenderView == NULL )
			bOK = false;
	}
	else
	{
	#ifdef WIN32
		int patternIndex;

		DEFINE_PATTERNS_FUTURE( fR_SetupFrame );
		DEFINE_PATTERNS_FUTURE( fV_RenderView );

		// Find signatures
		MemoryUtils()->FindPatternAsync( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::R_SetupFrame, fR_SetupFrame );
		MemoryUtils()->FindPatternAsync( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::V_RenderView, fV_RenderView );

		// R_SetupFrame
		m_pfnR_SetupFrame = MemoryUtils()->GetPatternFutureValue( fR_SetupFrame, &patternIndex );
		FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnR_SetupFrame,
											  "R_SetupFrame",
											  FeaturesGameData::Patterns::Engine::R_SetupFrame,
											  patternIndex );

		// V_RenderView
		m_pfnV_RenderView = MemoryUtils()->GetPatternFutureValue( fV_RenderView, &patternIndex );
		FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnV_RenderView,
											  "V_RenderView",
											  FeaturesGameData::Patterns::Engine::V_RenderView,
											  patternIndex );
	#else
		bOK = false;
	#endif
	}

	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnR_SetupFrame", m_pfnR_SetupFrame, GameData::Modules::Engine );
	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnV_RenderView", m_pfnV_RenderView, GameData::Modules::Engine );

	return bOK;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CFog::PostLoad( void )
{
	m_hR_SetupFrame = Detours()->DetourFunction( m_pfnR_SetupFrame, HOOKED_R_SetupFrame, GET_FUNC_PTR( ORIG_R_SetupFrame ) );
	m_hV_RenderView = Detours()->DetourFunction( m_pfnV_RenderView, HOOKED_V_RenderView, GET_FUNC_PTR( ORIG_V_RenderView ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CFog::Unload( void )
{
	Detours()->RemoveDetour( m_hR_SetupFrame );
	Detours()->RemoveDetour( m_hV_RenderView );
}

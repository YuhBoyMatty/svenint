// SvenInt (c) Sw1ft
// r_lightmap.cpp

#include "stdafx.h"
#include "r_lightmap.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CLightmap, lightmap, "Render", "Lightmap" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CLightmap::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// kHUD_Redraw_HookEvent post event
	cl_enginefuncs->pfnSetFilterBrightness( m_pBrightness->GetFloat() );
	cl_enginefuncs->pfnSetFilterColor( m_pColor->GetColor()[ 0 ], m_pColor->GetColor()[ 1 ], m_pColor->GetColor()[ 2 ] );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CLightmap::CLightmap( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pBrightness = NULL;
	m_pColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CLightmap::OnEnable( void )
{
	cl_enginefuncs->pfnSetFilterMode( 1 );
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CLightmap::OnDisable( void )
{
	cl_enginefuncs->pfnSetFilterMode( 0 );
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CLightmap::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pBrightness = Modules::menu->AddParamFloat( this, "Brightness", NULL, 1.f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, Color( 255, 255, 255, 255 ) );

	return true;
}
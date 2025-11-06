// SvenInt (c) Sw1ft
// visual_crosshair.cpp

#include "stdafx.h"
#include "visual_crosshair.h"
#include "player_camhack.h"
#include "r_drawing.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CCrosshair, crosshair, "Visual", "Crosshair" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CCrosshair::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// VGuiClientPanelPaint event
	if ( UTIL_IsSpectating() )
		return kHookContinue;

	if ( Features::camhack->IsEnabled() )
		return kHookContinue;

	//if ( g_Config.cvars.fp_roaming_draw_crosshair && g_FirstPersonRoaming.GetTargetPlayer() ) )
	//	return kHookContinue;

	const int iWidth = gameutils->GetScreenWidth();
	const int iHeight = gameutils->GetScreenHeight();

	if ( m_pDot->GetBool() )
	{
		if ( m_pOutline->GetBool() )
		{
			Features::drawing->DrawDotShadow( ( iWidth / 2 ) - 1,
											  ( iHeight / 2 ) - 1,
											  m_pOutlineColor->GetColor32().r,
											  m_pOutlineColor->GetColor32().g,
											  m_pOutlineColor->GetColor32().b,
											  m_pOutlineColor->GetColor32().a,
											  m_pThickness->GetInt(),
											  m_pOutlineThickness->GetInt() );
		}

		Features::drawing->DrawDot( ( iWidth / 2 ) - 1,
									( iHeight / 2 ) - 1,
									m_pColor->GetColor32().r,
									m_pColor->GetColor32().g,
									m_pColor->GetColor32().b,
									m_pColor->GetColor32().a,
									m_pThickness->GetInt() );
	}

	if ( m_pCross->GetBool() )
	{
		if ( m_pOutline->GetBool() )
		{
			Features::drawing->DrawCrosshairShadow( ( iWidth / 2 ) - 1,
													( iHeight / 2 ) - 1,
													m_pOutlineColor->GetColor32().r,
													m_pOutlineColor->GetColor32().g,
													m_pOutlineColor->GetColor32().b,
													m_pOutlineColor->GetColor32().a,
													m_pSize->GetInt(),
													m_pGap->GetInt(),
													m_pThickness->GetInt(),
													m_pOutlineThickness->GetInt() );
		}

		Features::drawing->DrawCrosshair( ( iWidth / 2 ) - 1,
										  ( iHeight / 2 ) - 1,
										  m_pColor->GetColor32().r,
										  m_pColor->GetColor32().g,
										  m_pColor->GetColor32().b,
										  m_pColor->GetColor32().a,
										  m_pSize->GetInt(),
										  m_pGap->GetInt(),
										  m_pThickness->GetInt() );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CCrosshair::CCrosshair( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pCross = NULL;
	m_pDot = NULL;
	m_pOutline = NULL;
	m_pSize = NULL;
	m_pGap = NULL;
	m_pThickness = NULL;
	m_pOutlineThickness = NULL;
	m_pColor = NULL;
	m_pOutlineColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CCrosshair::OnEnable( void )
{
	cl_enginefuncs->pfnClientCmd( "crosshair 0" );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CCrosshair::OnDisable( void )
{
	cl_enginefuncs->pfnClientCmd( "crosshair 1" );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CCrosshair::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pCross = Modules::menu->AddParamBool( this, "Cross", NULL, true );
	m_pDot = Modules::menu->AddParamBool( this, "Dot", NULL, false );
	m_pOutline = Modules::menu->AddParamBool( this, "Outline", NULL, true );
	m_pSize = Modules::menu->AddParamInteger( this, "Size", NULL, 10, 1, 50 );
	m_pGap = Modules::menu->AddParamInteger( this, "Gap", NULL, 4, 0, 50 );
	m_pThickness = Modules::menu->AddParamInteger( this, "Thickness", NULL, 2, 1, 50 );
	m_pOutlineThickness = Modules::menu->AddParamInteger( this, "OutlineThickness", NULL, 2, 1, 50 );
	m_pColor = Modules::menu->AddParamColorRGBA( this, "Color", NULL, Color( 255, 255, 255, 255 ) );
	m_pOutlineColor = Modules::menu->AddParamColorRGBA( this, "OutlineColor", NULL, Color( 0, 0, 0, 255 ) );

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CCrosshair::Unload( void )
{
	cl_enginefuncs->pfnClientCmd( "crosshair 1" );
}
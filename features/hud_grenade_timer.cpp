// SvenInt (c) Sw1ft
// hud_grenade_timer.cpp

#include "stdafx.h"
#include "hud_grenade_timer.h"
#include "r_drawing.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CGrenadeTimer, grenadetimer, "HUD", "Grenade Timer" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CGrenadeTimer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw post event
	if ( localplayer->GetCurrentWeaponID() != WEAPON_HANDGRENADE )
		return kHookContinue;

	float flTime = clientweapon->GetWeaponData()->fuser1;
	if ( flTime >= 0.f )
		return kHookContinue;

	flTime *= -1; // revert

	int r = m_pColor->GetColor32().r;
	int g = m_pColor->GetColor32().g;
	int b = m_pColor->GetColor32().b;

	if ( flTime >= 4.f )
	{
		r = m_pColorExplosion->GetColor32().r;
		g = m_pColorExplosion->GetColor32().g;
		b = m_pColorExplosion->GetColor32().b;

		if ( flTime > 10.f )
			flTime = 10.f;
	}

	int x = int( (float)gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() );

	int iSpriteWidth = Features::drawing->GetNumberSpriteWidth();
	int iSpriteHeight = Features::drawing->GetNumberSpriteHeight();

	int iThickness = int( (float)iSpriteWidth / 8.f );

	Features::drawing->DrawNumber( int( flTime ), x, y, r, g, b, FONT_ALIGN_LEFT ); // seconds

	if ( flTime < 10.f )
	{
		Features::drawing->FillArea( x + iSpriteWidth + ( iSpriteWidth / 2 ) - iThickness,
									 y + iSpriteHeight - iThickness * 2,
									 iThickness * 2,
									 iThickness * 2,
									 r, g, b, 232 );

		float ms = ( flTime - floorf( flTime ) ) * 1000.f;

		Features::drawing->DrawNumber( int( ms ), x + iSpriteWidth * 2, y, r, g, b, FONT_ALIGN_LEFT ); // milliseconds
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CGrenadeTimer::CGrenadeTimer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;
	m_pColor = NULL;
	m_pColorExplosion = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CGrenadeTimer::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CGrenadeTimer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CGrenadeTimer::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clrTimer, clrTimerExplosion; // ( 255, 65, 65, 255 )
	UnpackRGB( *(unsigned char *)&clrTimer.r,
			   *(unsigned char *)&clrTimer.g,
			   *(unsigned char *)&clrTimer.b,
			   RGB_BLUEISH );
	UnpackRGB( *(unsigned char *)&clrTimerExplosion.r,
			   *(unsigned char *)&clrTimerExplosion.g,
			   *(unsigned char *)&clrTimerExplosion.b,
			   RGB_REDISH );

	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.01f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.5f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, clrTimer );
	m_pColorExplosion = Modules::menu->AddParamColorRGB( this, "ColorExplosion", NULL, clrTimerExplosion );

	return true;
}
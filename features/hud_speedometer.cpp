// SvenInt (c) Sw1ft
// hud_speedometer.cpp

#include "stdafx.h"
#include "hud_speedometer.h"
#include "player_camhack.h"
#include "r_drawing.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpeedometer, speedometer, "HUD", "Speedometer" );

//-----------------------------------------------------------------------------
// ResetJumpSpeed
//-----------------------------------------------------------------------------

void CSpeedometer::ResetJumpSpeed( void )
{
	m_vecVelocity = m_vecPrevVelocity = demoplayback ? refparams->simvel : *playermove->velocity();

	m_flPrevTime = m_flTime;
	m_flFadeTime = m_pJumpSpeedFade->GetFloat();
	m_flJumpSpeed = 0.f;

	m_clFadeFrom[ 0 ] = m_pColor->GetColor32().r;
	m_clFadeFrom[ 1 ] = m_pColor->GetColor32().g;
	m_clFadeFrom[ 2 ] = m_pColor->GetColor32().b;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpeedometer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		ResetJumpSpeed();
		return kHookContinue;
	}

	// HUD_Redraw post event
	m_flTime = pEvent->GetArg<float>( "time" );
	m_vecVelocity = demoplayback ? refparams->simvel : *playermove->velocity();

	if ( UTIL_IsSpectating() || Features::camhack->IsEnabled() )
	{
		m_flPrevTime = m_flTime;
		m_vecPrevVelocity = m_vecVelocity;
		return kHookContinue;
	}

	const Color clrSpeed = m_pColor->GetColor32();
	const float flSpeed = m_vecVelocity.Length2D();

	Features::drawing->DrawNumber( flSpeed > 0.f ? int( floor( flSpeed ) ) : int( ceil( flSpeed ) ),
								   int( gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() ),
								   int( gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() ),
								   clrSpeed.r,
								   clrSpeed.g,
								   clrSpeed.b,
								   FONT_ALIGN_CENTER );

	if ( m_pJumpSpeed->GetBool() )
	{
		int r = clrSpeed.r;
		int g = clrSpeed.g;
		int b = clrSpeed.b;

		float flFadeDuration = m_pJumpSpeedFade->GetFloat();
		int iSpriteHeight = Features::drawing->GetNumberSpriteHeight();

		if ( flFadeDuration > 0.0f )
		{
			if ( ( m_vecVelocity[ 2 ] != 0.0f && m_vecPrevVelocity[ 2 ] == 0.0f ) ||
				 ( m_vecVelocity[ 2 ] > 0.0f && m_vecPrevVelocity[ 2 ] < 0.0f ) )
			{
				float flDifference = flSpeed - m_flJumpSpeed;
				if ( flDifference != 0.0f )
				{
					if ( flDifference > 0.0f )
					{
						m_clFadeFrom[ 0 ] = 0;
						m_clFadeFrom[ 1 ] = 255;
						m_clFadeFrom[ 2 ] = 0;
					}
					else
					{
						m_clFadeFrom[ 0 ] = 255;
						m_clFadeFrom[ 1 ] = 0;
						m_clFadeFrom[ 2 ] = 0;
					}

					m_flFadeTime = 0.0f;
					m_flJumpSpeed = flSpeed;
				}
			}

			float flDelta = Q_max( m_flTime - m_flPrevTime, 0.0f );

			m_flFadeTime += flDelta;

			if ( m_flFadeTime > flFadeDuration || !Vec_IsFloatFinite( m_flFadeTime ) )
				m_flFadeTime = flFadeDuration;

			float flFadeFrom_R = clrSpeed.r - m_clFadeFrom[ 0 ] / flFadeDuration;
			float flFadeFrom_G = clrSpeed.g - m_clFadeFrom[ 1 ] / flFadeDuration;
			float flFadeFrom_B = clrSpeed.b - m_clFadeFrom[ 2 ] / flFadeDuration;

			r = int( clrSpeed.r - flFadeFrom_R * ( flFadeDuration - m_flFadeTime ) );
			g = int( clrSpeed.g - flFadeFrom_G * ( flFadeDuration - m_flFadeTime ) );
			b = int( clrSpeed.b - flFadeFrom_B * ( flFadeDuration - m_flFadeTime ) );

			m_flPrevTime = m_flTime;
		}

		Features::drawing->DrawNumber( m_flJumpSpeed > 0.f ? int( floor( m_flJumpSpeed ) ) : int( ceil( m_flJumpSpeed ) ),
									   int( gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() ),
									   int( gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() ) - ( iSpriteHeight + iSpriteHeight / 4 ),
									   r, g, b,
									   FONT_ALIGN_CENTER );
	}

	m_vecPrevVelocity = m_vecVelocity;

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpeedometer::CSpeedometer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pJumpSpeed = NULL;
	m_pJumpSpeedFade = NULL;
	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;
	m_pColor = NULL;

	m_flTime = 0.f;
	m_flPrevTime = 0.f;
	m_flFadeTime = 0.f;
	m_flJumpSpeed = 0.f;

	m_clFadeFrom[ 0 ] = m_clFadeFrom[ 1 ] = m_clFadeFrom[ 2 ] = 0;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpeedometer::OnEnable( void )
{
	ResetJumpSpeed();

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpeedometer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpeedometer::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clrSpeed;
	UnpackRGB( *(unsigned char *)&clrSpeed.r,
			   *(unsigned char *)&clrSpeed.g,
			   *(unsigned char *)&clrSpeed.b,
			   RGB_BLUEISH );

	m_pJumpSpeed = Modules::menu->AddParamBool( this, "JumpSpeed", NULL, true );
	m_pJumpSpeedFade = Modules::menu->AddParamFloat( this, "JumpSpeedFade", NULL, 0.7f, 0.1f, 2.f );
	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.5f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.9f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, clrSpeed );

	return true;
}
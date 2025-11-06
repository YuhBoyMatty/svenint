// SvenInt (c) Sw1ft
// hud_speedometer.h

#ifndef SINT_FEATURE_SPEEDOMETER_H
#define SINT_FEATURE_SPEEDOMETER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Speedometer feature
//-----------------------------------------------------------------------------

class CSpeedometer final : public CBaseFeature, IHookEventListener
{
public:
	CSpeedometer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void ResetJumpSpeed( void );

private:
	CMenuValueBool *m_pJumpSpeed;
	CMenuValueFloat *m_pJumpSpeedFade;
	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;
	CMenuValueColorRGB *m_pColor;

	Vector m_vecVelocity, m_vecPrevVelocity;

	float m_flTime;
	float m_flPrevTime;
	float m_flFadeTime;
	float m_flJumpSpeed;

	int m_clFadeFrom[ 3 ];
};

EXTERN_FEATURE( CSpeedometer, speedometer );

#endif // SINT_FEATURE_SPEEDOMETER_H
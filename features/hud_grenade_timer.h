// SvenInt (c) Sw1ft
// hud_grenade_timer.h

#ifndef SINT_FEATURE_GRENADE_TIMER_H
#define SINT_FEATURE_GRENADE_TIMER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Grenade timer feature
//-----------------------------------------------------------------------------

class CGrenadeTimer final : public CBaseFeature, IHookEventListener
{
public:
	CGrenadeTimer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;
	CMenuValueColorRGB *m_pColor;
	CMenuValueColorRGB *m_pColorExplosion;
};

EXTERN_FEATURE( CGrenadeTimer, grenadetimer );

#endif // SINT_FEATURE_GRENADE_TIMER_H
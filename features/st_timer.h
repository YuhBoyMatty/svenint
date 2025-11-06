// SvenInt (c) Sw1ft
// st_timer.h

#ifndef SINT_FEATURE_SPEEDRUN_TIMER_H
#define SINT_FEATURE_SPEEDRUN_TIMER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Timer feature
//-----------------------------------------------------------------------------

class CSpeedrunTimer final : public CBaseFeature, IHookEventListener
{
public:
	CSpeedrunTimer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	float SegmentCurrentTime( void );

	void ShowTimer( float flTime, bool bServer );
	void StartTimer( void );
	void StopTimer( void );

private:
	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;
	CMenuValueColorRGB *m_pColor;

	bool m_bSegmentStarted;

	bool m_bIsC17;
	int m_iNihilanthIndex;
	entvars_t *m_pNihilanthVars;

	float m_flSegmentStart;
	float m_flSegmentTime;

	float m_flTimerTime;
	float m_flLastTimerUpdate;
};

EXTERN_FEATURE( CSpeedrunTimer, timer );

#endif // SINT_FEATURE_SPEEDRUN_TIMER_H
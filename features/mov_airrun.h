// SvenInt (c) Sw1ft
// mov_airrun.h

#ifndef SINT_FEATURE_AIRRUN_H
#define SINT_FEATURE_AIRRUN_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Air run feature
//-----------------------------------------------------------------------------

class CAirRun final : public CBaseFeature, IHookEventListener
{
public:
	CAirRun( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
};

EXTERN_FEATURE( CAirRun, airrun );

#endif // SINT_FEATURE_AIRRUN_H
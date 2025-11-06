// SvenInt (c) Sw1ft
// player_autoreload.h

#ifndef SINT_FEATURE_AUTORELOAD_H
#define SINT_FEATURE_AUTORELOAD_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Auto reload feature
//-----------------------------------------------------------------------------

class CAutoReload final : public CBaseFeature, IHookEventListener
{
public:
	CAutoReload( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
};

EXTERN_FEATURE( CAutoReload, autoreload );

#endif // SINT_FEATURE_AUTORELOAD_H
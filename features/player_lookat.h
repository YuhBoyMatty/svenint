// SvenInt (c) Sw1ft
// player_lookat.h

#ifndef SINT_FEATURE_LOOKAT_H
#define SINT_FEATURE_LOOKAT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Look at an entity feature
//-----------------------------------------------------------------------------

class CLookAt final : public CBaseFeature, IHookEventListener
{
public:
	CLookAt( const char *pszCategoryName, const char *pszName );

	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
};

EXTERN_FEATURE( CLookAt, lookat );

#endif // SINT_FEATURE_LOOKAT_H
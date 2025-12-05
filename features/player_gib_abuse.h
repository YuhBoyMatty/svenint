// SvenInt (c) Sw1ft
// player_gib_abuse.h

#ifndef SINT_FEATURE_GIB_ABUSE_H
#define SINT_FEATURE_GIB_ABUSE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Gib Abuse feature ( just spam gibme cmd in a specific interval to fly )
//-----------------------------------------------------------------------------

class CGibAbuse final : public CBaseFeature, IHookEventListener
{
public:
	CGibAbuse( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueInteger *m_pCommandAmount;
	CMenuValueFloat *m_pCommandInterval;
	CMenuValueFloat *m_pPreserveHealthAmount;

	float m_flLastIssuedCommand;
};

EXTERN_FEATURE( CGibAbuse, gibabuse );

#endif // SINT_FEATURE_GIB_ABUSE_H
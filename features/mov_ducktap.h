// SvenInt (c) Sw1ft
// mov_ducktap.h

#ifndef SINT_FEATURE_DUCKTAP_H
#define SINT_FEATURE_DUCKTAP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Auto Jump feature
//-----------------------------------------------------------------------------

class CDucktap final : public CBaseFeature, IHookEventListener
{
public:
	CDucktap( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	int m_prevground;
};

EXTERN_FEATURE( CDucktap, ducktap );

#endif // SINT_FEATURE_DUCKTAP_H
// SvenInt (c) Sw1ft
// mov_selfsink.h

#ifndef SINT_FEATURE_SELFSINK_H
#define SINT_FEATURE_SELFSINK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto selfsink feature
//-----------------------------------------------------------------------------

class CSelfsink final : public CBaseFeature, IHookEventListener
{
public:
	CSelfsink( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueList *m_pMode;

	int m_iState;
	int m_iFrames;
};

EXTERN_FEATURE( CSelfsink, selfsink );

#endif // SINT_FEATURE_SELFSINK_H
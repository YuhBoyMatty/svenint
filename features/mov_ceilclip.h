// SvenInt (c) Sw1ft
// mov_ceilclip.h

#ifndef SINT_FEATURE_AUTO_CEIL_CLIPPING_H
#define SINT_FEATURE_AUTO_CEIL_CLIPPING_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto ceil-clipping feature
//-----------------------------------------------------------------------------

class CAutoCeilClipping final : public CBaseFeature, IHookEventListener
{
public:
	CAutoCeilClipping( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
};

EXTERN_FEATURE( CAutoCeilClipping, autoceilclipping );

#endif // SINT_FEATURE_AUTO_CEIL_CLIPPING_H
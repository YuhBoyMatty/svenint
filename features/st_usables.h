// SvenInt (c) Sw1ft
// st_usables.h

#ifndef SINT_FEATURE_USABLES_H
#define SINT_FEATURE_USABLES_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Usable entities visualization feature
//-----------------------------------------------------------------------------

class CUsables final : public CBaseFeature, IHookEventListener
{
public:
	CUsables( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueFloat *m_pRadius;
};

EXTERN_FEATURE( CUsables, usables );

#endif // SINT_FEATURE_USABLES_H
// SvenInt (c) Sw1ft
// mov_fastrun.h

#ifndef SINT_FEATURE_FASTRUN_H
#define SINT_FEATURE_FASTRUN_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Fastrun feature
//-----------------------------------------------------------------------------

class CFastrun final : public CBaseFeature, IHookEventListener
{
public:
	CFastrun( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pBypassAntiStrafer;
	bool m_bSideway;
};

EXTERN_FEATURE( CFastrun, fastrun );

#endif // SINT_FEATURE_FASTRUN_H

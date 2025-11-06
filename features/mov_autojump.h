// SvenInt (c) Sw1ft
// mov_autojump.h

#ifndef SINT_FEATURE_AUTOJUMP_H
#define SINT_FEATURE_AUTOJUMP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto Jump feature
//-----------------------------------------------------------------------------

class CAutoJump final : public CBaseFeature, IHookEventListener
{
public:
	CAutoJump( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueList *m_pType;
};

EXTERN_FEATURE( CAutoJump, autojump );

#endif // SINT_FEATURE_AUTOJUMP_H
// SvenInt (c) Sw1ft
// mov_wallstrafe.h

#ifndef SINT_FEATURE_WALLSTRAFE_H
#define SINT_FEATURE_WALLSTRAFE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Wallstrafe feature
//-----------------------------------------------------------------------------

class CWallStrafe final : public CBaseFeature, IHookEventListener
{
public:
	CWallStrafe( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueFloat *m_pAngle;
	CMenuValueFloat *m_pDistance;
};

EXTERN_FEATURE( CWallStrafe, wallstrafe );

#endif // SINT_FEATURE_WALLSTRAFE_H
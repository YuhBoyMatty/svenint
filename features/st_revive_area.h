// SvenInt (c) Sw1ft
// st_revive_area.h

#ifndef SINT_FEATURE_REVIVE_AREA_H
#define SINT_FEATURE_REVIVE_AREA_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Revive area feature
//-----------------------------------------------------------------------------

class CReviveArea final : public CBaseFeature, IHookEventListener
{
public:
	CReviveArea( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pShowLocalPlayer;
	CMenuValueBool *m_pDrawSmallTestHull;
	CMenuValueBool *m_pDrawMediumTestHull;
	CMenuValueBool *m_pDrawLargeTestHull;
	CMenuValueFloat *m_pSmallHullWidth;
	CMenuValueFloat *m_pMediumHullWidth;
	CMenuValueFloat *m_pLargeHullWidth;
	CMenuValueColorRGBA *m_pSmallHullColor;
	CMenuValueColorRGBA *m_pMediumHullColor;
	CMenuValueColorRGBA *m_pLargeHullColor;
};

EXTERN_FEATURE( CReviveArea, revivearea );

#endif // SINT_FEATURE_REVIVE_AREA_H
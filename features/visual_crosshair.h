// SvenInt (c) Sw1ft
// visual_crosshair.h

#ifndef SINT_FEATURE_CROSSHAIR_H
#define SINT_FEATURE_CROSSHAIR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Crosshair feature
//-----------------------------------------------------------------------------

class CCrosshair final : public CBaseFeature, IHookEventListener
{
public:
	CCrosshair( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pCross;
	CMenuValueBool *m_pDot;
	CMenuValueBool *m_pOutline;
	CMenuValueInteger *m_pSize;
	CMenuValueInteger *m_pGap;
	CMenuValueInteger *m_pThickness;
	CMenuValueInteger *m_pOutlineThickness;
	CMenuValueColorRGBA *m_pColor;
	CMenuValueColorRGBA *m_pOutlineColor;
};

EXTERN_FEATURE( CCrosshair, crosshair );

#endif // SINT_FEATURE_CROSSHAIR_H
// SvenInt (c) Sw1ft
// r_lightmap.h

#ifndef SINT_FEATURE_LIGHTMAP_H
#define SINT_FEATURE_LIGHTMAP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Lightmap feature
//-----------------------------------------------------------------------------

class CLightmap final : public CBaseFeature, IHookEventListener
{
public:
	CLightmap( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueFloat *m_pBrightness;
	CMenuValueColorRGB *m_pColor;
};

EXTERN_FEATURE( CLightmap, lightmap );

#endif // SINT_FEATURE_LIGHTMAP_H
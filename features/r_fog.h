// SvenInt (c) Sw1ft
// r_fog.h

#ifndef SINT_FEATURE_FOG_H
#define SINT_FEATURE_FOG_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Fog feature
//-----------------------------------------------------------------------------

class CFog final : public CBaseFeature
{
public:
	CFog( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	void R_SetupFrameEvent( void );
	void V_RenderViewEvent( void );

private:
	CMenuValueBool *m_pRemoveWaterFog;
	CMenuValueBool *m_pFogSkybox;
	CMenuValueFloat *m_pFogStart;
	CMenuValueFloat *m_pFogEnd;
	CMenuValueFloat *m_pFogDensity;
	CMenuValueColorRGB *m_pFogColor;

	void *m_pfnR_SetupFrame;
	void *m_pfnV_RenderView;

	DetourHandle_t m_hR_SetupFrame;
	DetourHandle_t m_hV_RenderView;
};

EXTERN_FEATURE( CFog, fog );

#endif // SINT_FEATURE_FOG_H
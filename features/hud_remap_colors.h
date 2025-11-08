// SvenInt (c) Sw1ft
// hud_remap_colors.h

#ifndef SINT_FEATURE_REMAPCOLORS_H
#define SINT_FEATURE_REMAPCOLORS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Remap colors feature
//-----------------------------------------------------------------------------

class CRemapColors final : public CBaseFeature, IMenuElementButtonCallback
{
public:
	CRemapColors( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

public:
	inline Color GetRemapColor( void ) { return m_pHUDColor->GetColor32(); }

private:
	CMenuElementButton *m_pButtonSven;
	CMenuElementButton *m_pButtonYellow;
	CMenuElementButton *m_pButtonGreen;
	CMenuElementButton *m_pButtonBlue;
	CMenuElementButton *m_pButtonRed;
	CMenuValueColorRGB *m_pHUDColor;

	void *m_pfnSPR_Set;
	void *m_pfnFillRGBA;

	DetourHandle_t m_hSPR_Set;
	DetourHandle_t m_hFillRGBA;
};

EXTERN_FEATURE( CRemapColors, remapcolors );

#endif // SINT_FEATURE_REMAPCOLORS_H
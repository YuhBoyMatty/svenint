// SvenInt (c) Sw1ft
// st_hud.h

#ifndef SINT_FEATURE_SPEEDRUN_HUD_H
#define SINT_FEATURE_SPEEDRUN_HUD_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Speedrun's HUD info feature
//-----------------------------------------------------------------------------

class CSpeedrunHUD final : public CBaseFeature, IHookEventListener
{
public:
	CSpeedrunHUD( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	vgui::HFont GetEngineFont( void );
	Color GetColor( void );

private:
	void ShowViewangles( int r, int g, int b );
	void ShowPosition( int r, int g, int b );
	void ShowVelocity( int r, int g, int b );
	void ShowGaussBoostInfo( int r, int g, int b );
	void ShowSelfgaussInfo( int r, int g, int b );
	void ShowEntityInfo( int r, int g, int b );

private:
	CMenuValueColorRGB *m_pColor;

	CMenuValueBool *m_pShowViewAngles;
	CMenuValueFloat *m_pViewAnglesWidthScreenFraction;
	CMenuValueFloat *m_pViewAnglesHeightScreenFraction;

	CMenuValueBool *m_pShowPosition;
	CMenuValueBool *m_pShowPositionViewOrigin;
	CMenuValueFloat *m_pPositionWidthScreenFraction;
	CMenuValueFloat *m_pPositionHeightScreenFraction;

	CMenuValueBool *m_pShowVelocity;
	CMenuValueFloat *m_pVelocityWidthScreenFraction;
	CMenuValueFloat *m_pVelocityHeightScreenFraction;

	CMenuValueBool *m_pShowGaussBoostInfo;
	CMenuValueFloat *m_pGaussBoostInfoWidthScreenFraction;
	CMenuValueFloat *m_pGaussBoostInfoHeightScreenFraction;

	CMenuValueBool *m_pShowSelfGaussInfo;
	CMenuValueFloat *m_pSelfGaussInfoWidthScreenFraction;
	CMenuValueFloat *m_pSelfGaussInfoHeightScreenFraction;

	CMenuValueBool *m_pShowEntityInfo;
	CMenuValueBool *m_pShowEntityInfoCheckPlayers;
	CMenuValueFloat *m_pEntityInfoWidthScreenFraction;
	CMenuValueFloat *m_pEntityInfoHeightScreenFraction;

	vgui::HFont m_hEngineFont;
};

EXTERN_FEATURE( CSpeedrunHUD, sthud );

#endif // SINT_FEATURE_SPEEDRUN_HUD_H
// SvenInt (c) Sw1ft
// st_revive_info.h

#ifndef SINT_FEATURE_REVIVE_INFO_H
#define SINT_FEATURE_REVIVE_INFO_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Revive info feature
//-----------------------------------------------------------------------------

class CReviveInfo final : public CBaseFeature, IHookEventListener
{
public:
	CReviveInfo( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void Reset( void );
	void DrawReviveInfo( void );
	void ShowReviveInfo( int r, int g, int b );

private:
	CMenuValueBool *m_pShowWithAnyWeapon;
	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;
	CMenuValueColorRGBA *m_pColor;
	CMenuValueColorRGBA *m_pNoAmmoColor;

	bool m_bShowReviveInfo;
	cl_entity_t *m_pReviveTarget;
	float m_flReviveDistance;
};

EXTERN_FEATURE( CReviveInfo, reviveinfo );

#endif // SINT_FEATURE_REVIVE_INFO_H
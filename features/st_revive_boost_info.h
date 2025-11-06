// SvenInt (c) Sw1ft
// st_revive_boost_info.h

#ifndef SINT_FEATURE_REVIVE_BOOST_INFO_H
#define SINT_FEATURE_REVIVE_BOOST_INFO_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Revive boost info feature
//-----------------------------------------------------------------------------

class CReviveBoostInfo final : public CBaseFeature, IHookEventListener
{
public:
	CReviveBoostInfo( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void Reset( void );
	void DrawReviveBoostInfo( void );
	void DrawPredictedReviveBoost( void );
	void ShowReviveBoostInfo( int r, int g, int b );

private:
	CMenuValueBool *m_pShowWithAnyWeapon;
	CMenuValueBool *m_pWireframeHull;
	CMenuValueBool *m_pWireframeDirectionBox;

	CMenuValueList *m_pDirectionType;
	CMenuValueFloat *m_pDirectionLength;
	CMenuValueFloat *m_pDirectionBoxExtent;
	CMenuValueFloat *m_pDirectionLineWidth;
	CMenuValueFloat *m_pWireframeHullWidth;

	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;

	CMenuValueColorRGBA *m_pHullColor;
	CMenuValueColorRGBA *m_pDirectionColor;

	CMenuValueBool *m_pTrajectory;
	CMenuValueBool *m_pCollision;
	CMenuValueFloat *m_pCollisionWidth;
	CMenuValueColorRGBA *m_pTrajectoryColor;
	CMenuValueColorRGBA *m_pCollisionColor;

	bool m_bShowReviveBoostInfo;
	cl_entity_t *m_pReviveBoostTarget;
	float m_flReviveBoostDistance;
	float m_flReviveBoostAngle;
};

EXTERN_FEATURE( CReviveBoostInfo, reviveboostinfo );

#endif // SINT_FEATURE_REVIVE_BOOST_INFO_H
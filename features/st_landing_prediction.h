// SvenInt (c) Sw1ft
// st_landing_prediction.h

#ifndef SINT_FEATURE_LANDING_PREDICTION_H
#define SINT_FEATURE_LANDING_PREDICTION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Landing prediction feature
//-----------------------------------------------------------------------------

class CLandingPrediction final : public CBaseFeature, IHookEventListener
{
public:
	CLandingPrediction( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pLandingDrawHull;
	CMenuValueBool *m_pLandingDrawHullWireframe;
	CMenuValueBool *m_pLandingPoint;
	CMenuValueInteger *m_pLandingMaxPoints;
	CMenuValueFloat *m_pLandingHullWidth;
	CMenuValueColorRGBA *m_pLandingHullColor;
	CMenuValueColorRGBA *m_pLandingPointColor;
};

EXTERN_FEATURE( CLandingPrediction, landprediction );

#endif // SINT_FEATURE_LANDING_PREDICTION_H
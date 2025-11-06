// SvenInt (c) Sw1ft
// visual_projectile_prediction.h

#ifndef SINT_FEATURE_PROJECTILE_PREDICTION_H
#define SINT_FEATURE_PROJECTILE_PREDICTION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Projectile prediction feature
//-----------------------------------------------------------------------------

class CProjectilePrediction final : public CBaseFeature, IHookEventListener
{
public:
	CProjectilePrediction( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void PredictGrenadeTrajectory( void );
	void PredictARGrenadeTrajectory( void );
	void PredictSporeTrajectory( void );

private:
	CMenuValueBool *m_pPredictGrenade;
	CMenuValueColorRGBA *m_pGrenadeColor;
	CMenuValueColorRGBA *m_pGrenadeImpactColor;

	CMenuValueBool *m_pPredictARGrenade;
	CMenuValueColorRGBA *m_pARGrenadeColor;
	CMenuValueColorRGBA *m_pARGrenadeImpactColor;

	CMenuValueBool *m_pPredictSpore;
	CMenuValueList *m_pSporeAttackMode;
	CMenuValueColorRGBA *m_pSporeColor;
	CMenuValueColorRGBA *m_pSporeImpactColor;
};

EXTERN_FEATURE( CProjectilePrediction, projprediction );

#endif // SINT_FEATURE_PROJECTILE_PREDICTION_H
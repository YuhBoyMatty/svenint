// SvenInt (c) Sw1ft
// r_bobbing.h

#ifndef SINT_FEATURE_BOBBING_H
#define SINT_FEATURE_BOBBING_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define HL2_BOB_CYCLE_MIN 1.0f
#define HL2_BOB_CYCLE_MAX 0.45f
#define HL2_BOB_UP 0.5f

//-----------------------------------------------------------------------------
// Viewmodel bobbing feature
//-----------------------------------------------------------------------------

class CBobbing final : public CBaseFeature, IHookEventListener
{
public:
	CBobbing( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	float RemapVal( float val, float A, float B, float C, float D );
	float V_CalcNewBob( struct ref_params_s *pparams );

private:
	CMenuValueBool *m_pBobStrengthIsUncapped;
	CMenuValueInteger *m_pBobStrengthSpeedCap;

	float m_flLateralBob;
	float m_flVerticalBob;
	float m_flBobTime;
	float m_flLastBobTime;
};

EXTERN_FEATURE( CBobbing, bobbing );

#endif // SINT_FEATURE_BOBBING_H
// SvenInt (c) Sw1ft
// mov_autoclimb.h

#ifndef SINT_FEATURE_AUTO_LADDER_CLIMB_H
#define SINT_FEATURE_AUTO_LADDER_CLIMB_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto ladder climb feature
//-----------------------------------------------------------------------------

class CAutoLadderClimb final : public CBaseFeature, IHookEventListener
{
public:
	CAutoLadderClimb( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pTriggerWhenPressingMovementButtons;
	CMenuValueFloat *m_pThresholdAngle;

	bool m_bForcePitch;
	float m_flSavedPitchAngle;
};

EXTERN_FEATURE( CAutoLadderClimb, autoclimb );

#endif // SINT_FEATURE_AUTO_LADDER_CLIMB_H
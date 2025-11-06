// SvenInt (c) Sw1ft
// player_stick.h

#ifndef SINT_FEATURE_STICK_H
#define SINT_FEATURE_STICK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"
#include "features/mov_strafer.h"

//-----------------------------------------------------------------------------
// Stick to player(s) feature
//-----------------------------------------------------------------------------

class CStick final : public CBaseFeature, IHookEventListener
{
public:
	CStick( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	cl_entity_t *FindTarget( void );
	void Idle( usercmd_t *cmd );
	void SetViewAngles( cl_entity_t *pPlayer, usercmd_t *cmd );
	void StealModel( void );
	bool TryUnstuck( usercmd_t *cmd, Vector2D &vecDir );
	void TryMove( cl_entity_t *pPlayer, usercmd_t *cmd, Vector &vecPredictPos, Vector2D &vecDir );
	bool TryMoveOnLadder( cl_entity_t *pPlayer, usercmd_t *cmd );
	void TryMimic( cl_entity_t *pPlayer, usercmd_t *cmd );

private:
	CMenuValueBool *m_pAuto;
	CMenuValueBool *m_pStealModel;
	CMenuValueBool *m_pLookAtTarget;
	CMenuValueBool *m_pOvercomeObstacles;
	CMenuValueBool *m_pEdgejump;
	CMenuValueBool *m_pMimic;
	CMenuValueList *m_pStrafeMode;

	int m_iClimb;
	double m_flSwitchTargetTime;

	bool m_bForcePitch;
	float m_flSavedPitchAngle;

	Strafe::StrafeData m_strafeData;
};

EXTERN_FEATURE( CStick, stick );

#endif // SINT_FEATURE_STICK_H
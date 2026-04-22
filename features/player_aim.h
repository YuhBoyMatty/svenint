// SvenInt (c) Sw1ft
// player_aim.h

#ifndef SINT_FEATURE_AIM_H
#define SINT_FEATURE_AIM_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"
#include "misc_entity_list.h"

//-----------------------------------------------------------------------------
// Aim feature
//-----------------------------------------------------------------------------

class CAim final : public CBaseFeature, IHookEventListener
{
public:
	CAim( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	bool NoRecoil( usercmd_t *cmd );
	bool Aimbot( usercmd_t *cmd, int iMode, bool bChangeAnglesBack, bool &bAnglesChanged );

private:
	void SetChangeAnglesBack( void );

	bool IsUsingMountedGun( void );

	bool CheckReload( int iWeaponID, int iClip, usercmd_t *cmd );
	bool IsHoldingAppropriateWeapon( int iWeaponID );
	bool IsDistanceAllowsUseWeapon( int iWeaponID, float flDistance );
	bool IsTargetCanBeHurted( EEntityClassID iClassID, int iWeaponID );

	int ConcludeAttackButton( int iWeaponID, int iClip, float flDistance );

	bool IsFiring( int iWeaponID, usercmd_t *cmd );
	bool IsStillFiring( int iWeaponID, usercmd_t *cmd );

	CEntity *FindBestTarget( void );
	bool IsCanSeeTarget( CEntity *pEntity, Vector &vecEyes, Vector &vecPoint );
	void DirectionToAngles( Vector &vecDir, Vector &vecAngles );

private:
	CMenuValueList *m_pAimbotMode;
	CMenuValueBool *m_pAimChangeCameraBack;
	CMenuValueBool *m_pAimHitboxes;
	CMenuValueBool *m_pAimHead;
	CMenuValueBool *m_pAimNeck;
	CMenuValueBool *m_pAimChest;
	CMenuValueBool *m_pIgnoreUnknownEnts;
	CMenuValueBool *m_pIgnoreBlockingEnts;
	CMenuValueBool *m_pIgnoreGlass;
	CMenuValueBool *m_pConsiderFOV;
	CMenuValueFloat *m_pFOV;
	CMenuValueFloat *m_pDistance;
	CMenuValueFloat *m_pLimitFireRate;

	CMenuValueBool *m_pNoRecoil;
	CMenuValueBool *m_pNoRecoilVisual;

	CMenuValueBool *m_pVisualizeFireDirection;
	CMenuValueBool *m_pVisualizeAimbotTarget;

	bool m_bChangeAnglesBack;
	bool m_bAimbotFired;
	float m_flAimbotLastFired;

	Vector m_vecChangeAnglesTarget;
	Vector m_vecTargetPoint;
	Vector m_vecPunchAngle;
	Vector m_vecEvPunchAngle;

	Vector *ev_punchangle;
};

EXTERN_FEATURE( CAim, aim );

#endif // SINT_FEATURE_AIM_H
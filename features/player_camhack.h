// SvenInt (c) Sw1ft
// player_camhack.h

#ifndef SINT_FEATURE_CAMHACK_H
#define SINT_FEATURE_CAMHACK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Camhack feature
//-----------------------------------------------------------------------------

class CCamHack final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CCamHack( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void PostLoad( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

public:
	void Attach( int iPlayerIndex );

private:
	void PM_NoClip( usercmd_t *cmd );
	void ClampViewAngles( Vector &va );

private:
	CMenuElementButton *m_pButtonResetRoll;
	CMenuElementButton *m_pButtonResetOrientation;
	CMenuValueBool *m_pShowModel;
	CMenuValueBool *m_pHideHUD;
	CMenuValueBool *m_pIgnorePitch;
	CMenuValueFloat *m_pSpeedFactor;

	bool m_bChangeCameraState;
	bool m_bChangeToThirdPerson;
	int m_iAttachTarget;
	float m_flSavedPitchAngle;

	Vector m_vecViewAngles;
	Vector m_vecOldViewangles;
	Vector m_vecNewViewangles;
	Vector m_vecCameraOrigin;
	Vector m_vecCameraAngles;

	usercmd_t m_dummyCmd;

	bool keydown_w;
	bool keydown_s;
	bool keydown_a;
	bool keydown_d;
	bool keydown_space;
	bool keydown_ctrl;
	bool keydown_shift;
	bool keydown_mouse1;
	bool keydown_mouse2;
};

EXTERN_FEATURE( CCamHack, camhack );

#endif // SINT_FEATURE_CAMHACK_H
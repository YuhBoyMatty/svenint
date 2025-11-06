// SvenInt (c) Sw1ft
// player_antiafk.h

#ifndef SINT_FEATURE_THIRDPERSON_H
#define SINT_FEATURE_THIRDPERSON_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Enhanced third person feature
//-----------------------------------------------------------------------------

class CThirdPerson final : public CBaseFeature, IHookEventListener
{
public:
	CThirdPerson( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void PM_NoClip( usercmd_t *cmd );
	void ClampViewAngles( void );

private:
	CMenuValueBool *m_pIgnorePitch;
	CMenuValueBool *m_pIgnoreYaw;
	CMenuValueBool *m_pEditMode;
	CMenuValueBool *m_pHideHUD;
	CMenuValueBool *m_pClipToWall;
	CMenuValueList *m_pTraceType;
	CMenuValueVector *m_pAngles;
	CMenuValueVector *m_pOrigin;

	usercmd_t m_dummyCmd;
	Vector m_vecOldViewangles;
	Vector m_vecNewViewangles;
	Vector m_vecViewAngles;
	float m_flSavedPitchAngle;
	bool m_bEditModeWasEnabled;

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

EXTERN_FEATURE( CThirdPerson, thirdperson );

#endif // SINT_FEATURE_THIRDPERSON_H
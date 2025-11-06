// SvenInt (c) Sw1ft
// player_antiafk.h

#ifndef SINT_FEATURE_ANTIAFK_H
#define SINT_FEATURE_ANTIAFK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Anti-AFK feature
//-----------------------------------------------------------------------------

class CAntiAFK final : public CBaseFeature, IHookEventListener
{
public:
	CAntiAFK( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void OnRespawn( void );

private:
	void AntiAFK( usercmd_t *cmd );
	void WalkAround( usercmd_t *cmd, int &delay, int &movement_button, const int delay_count );
	void RotateCamera( void );
	void OnDie( void );
	void Reset( void );

private:
	CMenuValueList *m_pType;
	CMenuValueBool *m_pRotateCamera;
	CMenuValueBool *m_pStayWithinRadius;
	CMenuValueBool *m_pResetStayPos;
	CMenuValueFloat *m_pStayRadius;
	CMenuValueFloat *m_pStayRadiusOffsetAngle;
	CMenuValueFloat *m_pRotationAngle;

	bool m_bDead;
	bool m_bComingBackToAFKPoint;

	bool m_bWaitingForClientdata;
	bool m_bWaitingForRespawn;

	Vector2D m_vecAFKPoint; // 2D point only
	float m_flComingBackStartTime;
	float m_flClientDataLastUpdate;

	DetourHandle_t m_hUserMsgHook_Health;
};

EXTERN_FEATURE( CAntiAFK, antiafk );

#endif // SINT_FEATURE_ANTIAFK_H
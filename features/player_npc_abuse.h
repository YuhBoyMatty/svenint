// SvenInt (c) Sw1ft
// player_npc_abuse.h

#ifndef SINT_FEATURE_NPC_ABUSE_H
#define SINT_FEATURE_NPC_ABUSE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Auto NPC Abuse feature
//-----------------------------------------------------------------------------

class CAutoNPCAbuse final : public CBaseFeature, IHookEventListener
{
public:
	CAutoNPCAbuse( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void MoveToDesiredPoint( usercmd_t *cmd, const Vector2D &vecDesiredPoint );

	void execute_hl_c04( usercmd_t *cmd );
	void execute_hl_c10( usercmd_t *cmd );

private:
	CMenuValueBool *m_pHL_C04;
	CMenuValueBool *m_pHL_C10;

	float m_flWaitForNPC;
};

EXTERN_FEATURE( CAutoNPCAbuse, npcabuse );

#endif // SINT_FEATURE_NPC_ABUSE_H
// SvenInt (c) Sw1ft
// player_firstperson_roaming.h

#ifndef SINT_FEATURE_FIRSTPERSON_ROAMING_H
#define SINT_FEATURE_FIRSTPERSON_ROAMING_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// First person roaming feature
//-----------------------------------------------------------------------------

class CFirstPersonRoaming final : public CBaseFeature, IHookEventListener
{
public:
	CFirstPersonRoaming( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline cl_entity_t *GetTargetPlayer( void ) { return m_pTarget; }
	inline Vector &GetLerpViewAngles( void ) { return m_vecPrevAngles; }

private:
	void GetPlayerViewAngles( Vector &vecOutput );

private:
	CMenuValueBool *m_pLerp;
	CMenuValueFloat *m_pLerpValue;

	int m_iTarget;
	int m_iSpectatorMode;

	cl_entity_t *m_pTarget;
	Vector m_vecPrevAngles;
};

EXTERN_FEATURE( CFirstPersonRoaming, firstpersonroaming );

#endif // SINT_FEATURE_FIRSTPERSON_ROAMING_H
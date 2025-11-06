// SvenInt (c) Sw1ft
// visual_hitmarkers.h

#ifndef SINT_FEATURE_HITMARKERS_H
#define SINT_FEATURE_HITMARKERS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "game/messagebuffer.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Hit markers feature
//-----------------------------------------------------------------------------

class CHitMarkers final : public CBaseFeature, IHookEventListener
{
public:
	CHitMarkers( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void AddHitmarker( const char *pszUserMsg, int iSize, void *pBuffer );

private:
	struct hitmarker_s
	{
		Vector origin;
		float time;
	};

	CMenuValueBool *m_pSound;
	CMenuValueInteger *m_pSize;
	CMenuValueFloat *m_pStayTime;

	bool m_bHitmarkerOnce;
	float m_flLastWeaponAttack;
	int m_hHitMarkerTexture;
	DetourHandle_t m_hUserMsgHook_CreateBlood;
	CMessageBuffer m_CreateBloodBuffer;;
	std::vector<hitmarker_s> m_hitMarkers;
};

EXTERN_FEATURE( CHitMarkers, crosshair );

#endif // SINT_FEATURE_HITMARKERS_H
// SvenInt (c) Sw1ft
// player_dynamic_glow.h

#ifndef SINT_FEATURE_DYNAMIC_GLOW_H
#define SINT_FEATURE_DYNAMIC_GLOW_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Dynamic glow feature
//-----------------------------------------------------------------------------

class CDynamicGlow final : public CBaseFeature, IHookEventListener
{
public:
	CDynamicGlow( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void CreateDynamicLight( int entindex, float *vOrigin, float *pColor24, float flRadius, float flDecay, float flDieTime );

private:
	CMenuValueBool *m_pAttach;

	CMenuValueBool *m_pSelf;
	CMenuValueFloat *m_pSelfRadius;
	CMenuValueFloat *m_pSelfDecay;
	CMenuValueColorRGB *m_pSelfColor;

	CMenuValueBool *m_pPlayers;
	CMenuValueFloat *m_pPlayersRadius;
	CMenuValueFloat *m_pPlayersDecay;
	CMenuValueColorRGB *m_pPlayersColor;

	CMenuValueBool *m_pEntities;
	CMenuValueFloat *m_pEntitiesRadius;
	CMenuValueFloat *m_pEntitiesDecay;
	CMenuValueColorRGB *m_pEntitiesColor;

	CMenuValueBool *m_pItems;
	CMenuValueFloat *m_pItemsRadius;
	CMenuValueFloat *m_pItemsDecay;
	CMenuValueColorRGB *m_pItemsColor;

};

EXTERN_FEATURE( CDynamicGlow, dynamicglow );

#endif // SINT_FEATURE_DYNAMIC_GLOW_H
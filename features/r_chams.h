// SvenInt (c) Sw1ft
// r_chams.h

#ifndef SINT_FEATURE_CHAMS_H
#define SINT_FEATURE_CHAMS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Viewmodel tweaks feature
//-----------------------------------------------------------------------------

class CChams final : public CBaseFeature, IHookEventListener
{
public:
	CChams( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void glColor4fEvent( GLfloat &red, GLfloat &green, GLfloat &blue );

private:
	bool Glow( cl_entity_s *pEntity, int iGlow, bool bGlowWall, int iChams, int iGlowWidth, float flGlowColor[ 3 ] );
	bool Chams( cl_entity_s *pEntity, int iChams, bool bChamsWall, float flChamsColor[ 3 ], float flChamsWallColor[ 3 ] );

private:
	CMenuValueBool *m_pChams;

	CMenuValueBool *m_pChamsPlayersBehindWall;
	CMenuValueList *m_pChamsPlayersMode;
	CMenuValueColorRGB *m_pChamsPlayersColor;
	CMenuValueColorRGB *m_pChamsPlayersBehindWallColor;
	
	CMenuValueBool *m_pChamsEntitiesBehindWall;
	CMenuValueList *m_pChamsEntitiesMode;
	CMenuValueColorRGB *m_pChamsEntitiesColor;
	CMenuValueColorRGB *m_pChamsEntitiesBehindWallColor;
	
	CMenuValueBool *m_pChamsItemsBehindWall;
	CMenuValueList *m_pChamsItemsMode;
	CMenuValueColorRGB *m_pChamsItemsColor;
	CMenuValueColorRGB *m_pChamsItemsBehindWallColor;

	CMenuValueBool *m_pGlow;
	CMenuValueBool *m_pGlowOptimize;

	CMenuValueBool *m_pGlowPlayersBehindWall;
	CMenuValueList *m_pGlowPlayersMode;
	CMenuValueInteger *m_pGlowPlayersWidth;
	CMenuValueColorRGB *m_pGlowPlayersColor;

	CMenuValueBool *m_pGlowEntitiesBehindWall;
	CMenuValueList *m_pGlowEntitiesMode;
	CMenuValueInteger *m_pGlowEntitiesWidth;
	CMenuValueColorRGB *m_pGlowEntitiesColor;

	CMenuValueBool *m_pGlowItemsBehindWall;
	CMenuValueList *m_pGlowItemsMode;
	CMenuValueInteger *m_pGlowItemsWidth;
	CMenuValueColorRGB *m_pGlowItemsColor;

	bool m_bOverrideColor;
	float *m_flOverrideColor;
	int m_iChamsMode;

	void *m_pfnglColor4f;
	DetourHandle_t m_hglColor4f;
};

EXTERN_FEATURE( CChams, chams );

#endif // SINT_FEATURE_CHAMS_H
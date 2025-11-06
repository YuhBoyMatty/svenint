// SvenInt (c) Sw1ft
// visual_radar.h

#ifndef SINT_FEATURE_RADAR_H
#define SINT_FEATURE_RADAR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Radar feature
//-----------------------------------------------------------------------------

class CRadar final : public CBaseFeature, IHookEventListener
{
public:
	CRadar( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pShowPlayerName;
	CMenuValueBool *m_pShowEntityName;
	CMenuValueList *m_pType;
	CMenuValueInteger *m_pSize;
	CMenuValueFloat *m_pViewDistance;
	CMenuValueFloat *m_pScreenWidthFraction;
	CMenuValueFloat *m_pScreenHeightFraction;

	int m_hRadarRoundTexture;
};

EXTERN_FEATURE( CRadar, radar );

#endif // SINT_FEATURE_RADAR_H
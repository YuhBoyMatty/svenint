// SvenInt (c) Sw1ft
// visual_player_sight_direction.h

#ifndef SINT_FEATURE_PLAYER_SIGHT_DIRECTION_H
#define SINT_FEATURE_PLAYER_SIGHT_DIRECTION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Player sight direction feature
//-----------------------------------------------------------------------------

class CPlayerSightDirection final : public CBaseFeature, IHookEventListener
{
public:
	CPlayerSightDirection( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueFloat *m_pBeamLength;
	CMenuValueFloat *m_pBeamWidth;
	CMenuValueColorRGB *m_pColor;

	int m_iBeamIndex;
};

EXTERN_FEATURE( CPlayerSightDirection, sightdir );

#endif // SINT_FEATURE_PLAYER_SIGHT_DIRECTION_H
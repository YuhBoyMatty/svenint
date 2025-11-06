// SvenInt (c) Sw1ft
// visual_player_push_direction.h

#ifndef SINT_FEATURE_PLAYER_PUSH_DIRECTION_H
#define SINT_FEATURE_PLAYER_PUSH_DIRECTION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Player push direction feature
//-----------------------------------------------------------------------------

class CPlayerPushDirection final : public CBaseFeature, IHookEventListener
{
public:
	CPlayerPushDirection( const char *pszCategoryName, const char *pszName );

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

EXTERN_FEATURE( CPlayerPushDirection, pushdir );

#endif // SINT_FEATURE_PLAYER_PUSH_DIRECTION_H
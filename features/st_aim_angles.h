// SvenInt (c) Sw1ft
// st_aim_angles.h

#ifndef SINT_FEATURE_ST_AIM_ANGLES_H
#define SINT_FEATURE_ST_AIM_ANGLES_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Aim angles feature
//-----------------------------------------------------------------------------

class CAimAngles final : public CBaseFeature, IHookEventListener
{
public:
	CAimAngles( const char *pszCategoryName, const char *pszName );

	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void SetAngles( float pitch, float yaw, int frames );
	void SetAngles2( float pitch, float yaw, float lerp );
	void FollowPoint( float x, float y, float z, float lerp );

	inline void StopSetAngles( void ) { m_bSetAngles = false; }
	inline void StopSetAngles2( void ) { m_bSetAngles2 = false; }
	inline void StopFollowingPoint( void ) { m_bFollowPoint = false; }

private:
	bool m_bSetAngles;
	bool m_bSetAngles2;
	Vector m_vecSetAngles;
	Vector m_vecSetAnglesSpeed;
	Vector m_vecSetAngles2;
	float m_flSetAngles2Lerp;

	bool m_bFollowPoint;
	float m_flFollowPointLerp;
	Vector m_vecFollowPoint;
};

EXTERN_FEATURE( CAimAngles, aimangles );

#endif // SINT_FEATURE_ST_AIM_ANGLES_H
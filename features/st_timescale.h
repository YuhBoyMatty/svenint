// SvenInt (c) Sw1ft
// st_timescale.h

#ifndef SINT_FEATURE_TIMESCALE_H
#define SINT_FEATURE_TIMESCALE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Timescale feature
//-----------------------------------------------------------------------------

class CTimescale final : public CBaseFeature, IHookEventListener
{
public:
	CTimescale( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void BroadcastTimescale( void );
	void SendTimescale( edict_t *pPlayer );
	void SetTimescale( float timescale );
	void SetTimescale_Comm( bool notify, float framerate, float fpsmax, float min_frametime );

private:
	double m_dbTimeCounter;
	bool m_bUsePassedTime;

	uint16_t *m_pJumpOpCode;
	uint16_t m_PatchedJumpOpCode;
};

EXTERN_FEATURE( CTimescale, timescale );

#endif // SINT_FEATURE_TIMESCALE_H
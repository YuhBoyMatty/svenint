// SvenInt (c) Sw1ft
// player_speedhack.h

#ifndef SINT_FEATURE_SPEEDHACK_H
#define SINT_FEATURE_SPEEDHACK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

#ifdef WIN32
typedef BOOL ( WINAPI *QueryPerformanceCounterWinFn )( LARGE_INTEGER * );
#endif

//-----------------------------------------------------------------------------
// Speedhack feature
//-----------------------------------------------------------------------------

class CSpeedhack final : public CBaseFeature, IHookEventListener
{
public:
	CSpeedhack( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline void SetGameSpeed( float factor )
	{
		if ( m_dbGameSpeed != NULL )
		#ifdef WIN32
			*m_dbGameSpeed = static_cast<double>( factor ) * 1000.0;
		#else // lol))
			*m_dbGameSpeed = factor * 1000.f;
		#endif
	}

	inline void SetGameSpeed( double factor )
	{
		if ( m_dbGameSpeed != NULL )
		#ifdef WIN32
			*m_dbGameSpeed = factor * 1000.0;
		#else // lol))
			*m_dbGameSpeed = static_cast<float>( factor ) * 1000.f;
		#endif
	}
	
	inline double GetGameSpeed( void ) const
	{
		if ( m_dbGameSpeed != NULL )
		#ifdef WIN32
			return *m_dbGameSpeed;
		#else
			return static_cast<double>( *m_dbGameSpeed );
		#endif
		return 1000.0;
	}

	inline void SetLTFX( float value ) { *Globals::realtime += static_cast<float>( value ); }
	inline void SetLTFX( double value ) { *Globals::realtime += value; }

#ifdef WIN32
	inline float GetAppSpeed( void ) const { return m_pAppSpeed->GetFloat(); }

	QueryPerformanceCounterWinFn GetQueryPerformanceCounter( void );
#endif

private:
	CMenuValueFloat *m_pGameSpeed;
	CMenuValueFloat *m_pLTFX;
#ifdef WIN32
	CMenuValueFloat *m_pAppSpeed;
#endif

#ifdef WIN32
	double *m_dbGameSpeed;

	void *m_pfnQueryPerformanceCounter;
	DetourHandle_t m_hQueryPerformanceCounter;
#else
	float *m_dbGameSpeed;
#endif
};

EXTERN_FEATURE( CSpeedhack, speedhack );

#endif // SINT_FEATURE_SPEEDHACK_H

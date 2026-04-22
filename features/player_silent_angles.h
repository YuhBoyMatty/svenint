// SvenInt (c) Sw1ft
// player_silent_angles.h

#ifndef SINT_FEATURE_SILENT_ANGLES_H
#define SINT_FEATURE_SILENT_ANGLES_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"

//-----------------------------------------------------------------------------
// Abort flags
//-----------------------------------------------------------------------------

#define SILENT_ANGLES_ABORT_NOT_WALKING		( 1 << 0 )
#define SILENT_ANGLES_ABORT_IN_WATER		( 1 << 1 )
#define SILENT_ANGLES_ABORT_USE				( 1 << 2 )
#define SILENT_ANGLES_ABORT_SPRAY			( 1 << 3 )
#define SILENT_ANGLES_ABORT_FIRING			( 1 << 4 )
#define SILENT_ANGLES_ABORT_LONGJUMP		( 1 << 5 )
#define SILENT_ANGLES_ABORT_ALL				( -1 )

//-----------------------------------------------------------------------------
// Silent angles feature
//-----------------------------------------------------------------------------

class CSilentAngles final : public CBaseFeature
{
public:
	CSilentAngles( const char *pszCategoryName, const char *pszName );

	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	inline bool SetAngles( const Vector &angles, int fAbortFlags = -1 )
	{
		if ( m_bLockAngles )
			return false;

		m_bSetAngles = true;
		m_fAbortFlags = fAbortFlags;
		m_vecAngles = angles;
		return true;
	}

	inline void LockAngles( void ) // Lock angles so they can't be changed until next frame or you cancel them
	{
		m_bLockAngles = true;
	}
	
	inline void Cancel( void )
	{
		m_bSetAngles = false;
		m_bLockAngles = false;
		m_fAbortFlags = -1;
	}

	inline bool IsSet( void ) const { return m_bSetAngles; }
	inline bool IsLocked( void ) const { return m_bLockAngles; }

	inline bool AddAngles( const Vector &angles )
	{
		if ( m_bLockAngles )
			return false;

		m_vecAngles += angles;
		return true;
	}

	inline bool SubtractAngles( const Vector &angles )
	{
		if ( m_bLockAngles )
			return false;

		m_vecAngles -= angles;
		return true;
	}

	inline QAngle GetAngles( void ) { return m_vecAngles; }

	bool CanSetAngles( usercmd_t *cmd, int fAbortFlags );
	void Process( usercmd_t *cmd );
	void FixPlayerModelLean( void );
	void OverrideEventWeaponAngles( int entindex, Vector &angles );

private:
	void FixMoveStart( usercmd_t *cmd );
	void FixMoveEnd( usercmd_t *cmd );

private:
	bool m_bSetAngles;
	bool m_bLockAngles;
	int m_fAbortFlags;
	float m_flPlayerModelPitch;
	Vector m_vecAngles;

	float m_flForwardMove, m_flSideMove, m_flUpMove;
	Vector vecViewForward, vecViewRight, vecViewUp, vecAimForward, vecAimRight, vecAimUp;

	bool m_bOverrideVirtualVA;
	Vector m_vecLastVirtualVA;
	std::vector<DetourHandle_t> m_eventhooks;
};

EXTERN_FEATURE( CSilentAngles, silentangles );

#endif // SINT_FEATURE_SILENT_ANGLES_H
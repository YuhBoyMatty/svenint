// SvenInt (c) Sw1ft
// game_utils.h

#ifndef SINT_GAME_UTILS_H
#define SINT_GAME_UTILS_H

#ifdef _WIN32
#pragma once
#endif

#include "game/mathlib.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class CMessageBuffer;

//-----------------------------------------------------------------------------
// CNetMessageParams
//-----------------------------------------------------------------------------

class CNetMessageParams
{
public:
	const struct sizebuf_s *buffer;
	int readcount;
	bool badread;
};

//-----------------------------------------------------------------------------
// Purpose: utility functions
//-----------------------------------------------------------------------------

class CGameUtils
{
public:
	CGameUtils();

	void Init( void );

	inline int GetScreenWidth( void ) const { return m_iScreenWidth; }
	inline int GetScreenHeight( void ) const { return m_iScreenHeight; }

	bool			WorldToScreen( Vector &vWorldOrigin, Vector2D &vScreen );
	void			ScreenToWorld( Vector2D &vScreen, Vector &vWorldOrigin );

	uint64			GetSteamID( int iPlayerIndex );

	void			GetViewAngles( int iPlayerIndex, Vector &va );
	void			GetViewAngles( struct cl_entity_s *pPlayer, Vector &va );

	CNetMessageParams *GetNetMessageParams( void );
	void			ApplyReadToNetMessageBuffer( class CMessageBuffer *buffer );
	void			ApplyReadToNetMessageBuffer( int readcount, int badread );

	const struct netmsg_s *FindNetworkMessage( int iNetMessageType );
	const struct netmsg_s *FindNetworkMessage( const char *pszName );
	const struct usermsg_s *FindUserMessage( const char *pszName );
	const struct event_hook_s *FindEventHook( const char *pszName );

	void			PrintChatText( const char *pszMessage, ... );

	void			DrawSetTextColor( float r, float g, float b );
	int				DrawConsoleString( int x, int y, const char *pszFormat, ... );

private:
	int m_iScreenWidth;
	int m_iScreenHeight;

	CNetMessageParams m_NetMessageParams;
};

CGameUtils *GameUtils();

//-----------------------------------------------------------------------------
// Rest utilities
//-----------------------------------------------------------------------------

inline long FloatToLong32( float val )
{
	union
	{
		float un_fl;
		unsigned long un_ul;
	};

	un_fl = val;

	return un_ul;
}

inline float Long32ToFloat( long val )
{
	union
	{
		float un_fl;
		unsigned long un_ul;
	};

	un_ul = val;

	return un_fl;
};

char *UTIL_GetLongPathName( void );

// Client utilities
bool UTIL_IsDead( void );
bool UTIL_IsSpectating( void );
int UTIL_GetLocalPlayerIndex( void );

// Intersection tests
#define UTIL_IsPointInsideAABB( point, mins, maxs ) UTIL_IsAABBIntersectingAABB( point, point, mins, maxs )

inline bool UTIL_IsAABBIntersectingAABB( const Vector &vecBoxMins1, const Vector &vecBoxMaxs1, const Vector &vecBoxMins2, const Vector &vecBoxMaxs2 )
{
	return ( vecBoxMins1.x <= vecBoxMaxs2.x && vecBoxMaxs1.x >= vecBoxMins2.x ) &&
		( vecBoxMins1.y <= vecBoxMaxs2.y && vecBoxMaxs1.y >= vecBoxMins2.y ) &&
		( vecBoxMins1.z <= vecBoxMaxs2.z && vecBoxMaxs1.z >= vecBoxMins2.z );
}

bool UTIL_IsLineIntersectingAABB( const Vector &p1, const Vector &p2, const Vector &vecBoxMins, const Vector &vecBoxMaxs );
bool UTIL_IsSphereIntersectingAABB( const Vector &vecCenter, const float flRadiusSqr, const Vector &vecAbsMins, const Vector &vecAbsMaxs, float *pflOutDistance );
bool UTIL_IsRayIntersectingAABB( const Vector &vecBoxMins, const Vector &vecBoxMaxs, const Vector &vecRayOrigin, const Vector &vecRayDir, float *pflMinIntersection, float *pflMaxIntersection );

// Random
int UTIL_SharedRandomLong( unsigned int seed, int low, int high );
float UTIL_SharedRandomFloat( unsigned int seed, float low, float high );

// String conversions
const wchar_t *UTIL_CStringToWideCString( const char *pszString );

// Player move utilities
int UTIL_ClipVelocity( const Vector &in, const Vector &normal, Vector &out, float overbounce );
void UTIL_AddCorrectGravity( Vector &vecVelocity, float frametime );
void UTIL_FixupGravityVelocity( Vector &vecVelocity, float frametime );
void UTIL_AddCorrectGravity( Vector &vecVelocity, float gravity, float entgravity, float frametime );
void UTIL_FixupGravityVelocity( Vector &vecVelocity, float gravity, float entgravity, float frametime );

// Hulls smh
void UTIL_FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity );
void UTIL_FindHullIntersectionClient( const Vector &vecSrc, pmtrace_t &tr, float *mins, float *maxs, int ignore_ent );

// Linear algebra smh
Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize );

inline float UTIL_SimpleSpline( float dt )
{
	const float dt_sqr = dt * dt;
	return 3.f * dt_sqr - 2.f * dt_sqr * dt;
}

// Viewport transformations
bool UTIL_WorldToScreen( float *pflOrigin, float *pflVecScreen );
void UTIL_ScreenToWorld( float *pflNDC, float *pflWorldOrigin );

bool UTIL_IsBusyWithLongJump( struct usercmd_s *cmd );

// Aim-related
bool UTIL_IsContiniousFiring( struct usercmd_s *cmd );
bool UTIL_IsFiring( struct usercmd_s *cmd );

#endif // SINT_GAME_UTILS_H

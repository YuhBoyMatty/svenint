// SvenInt (c) Sw1ft
// utils.cpp

#include "stdafx.h"
#include "game_utils.h"

#include "game/dbg.h"
#include "game/messagebuffer.h"
#include "game/localplayer.h"

#include <stdarg.h>

#ifndef WIN32
#include <unistd.h>
#include <limits.h>
#endif

//-----------------------------------------------------------------------------
// CGameUtils implementation
//-----------------------------------------------------------------------------

CGameUtils::CGameUtils()
{
	m_iScreenWidth = 1280;
	m_iScreenHeight = 720;

	memset( &m_NetMessageParams, 0, sizeof( CNetMessageParams ) );
}

void CGameUtils::Init( void )
{
	if ( *Globals::videomode_ptr != NULL )
	{
		vmode_t *mode = ( *Globals::videomode_ptr )->GetCurrentMode();

		m_iScreenWidth = mode->width;
		m_iScreenHeight = mode->height;

		DevMsg( "<SvenInt::GameUtils> Received screen size (%dx%d)\n", m_iScreenWidth, m_iScreenHeight );
	}
	else
	{
		Warning( "[SvenInt] NULL videomode_ptr!\n" );
	}
}

//-----------------------------------------------------------------------------
// Screen utilities
//-----------------------------------------------------------------------------

bool CGameUtils::WorldToScreen( Vector &vWorldOrigin, Vector2D &vScreen )
{
	int iResult = Globals::cl_enginefuncs->pTriAPI->WorldToScreen( vWorldOrigin, vScreen );

	if ( !iResult && vScreen.x <= 1 && vScreen.y <= 1 && vScreen.x >= -1 && vScreen.y >= -1 )
	{
		vScreen.x = ( m_iScreenWidth / 2 * vScreen.x ) + ( vScreen.x + m_iScreenWidth / 2 );
		vScreen.y = -( m_iScreenHeight / 2 * vScreen.y ) + ( vScreen.y + m_iScreenHeight / 2 );

		return true;
	}

	return false;
}

void CGameUtils::ScreenToWorld( Vector2D &vScreen, Vector &vWorldOrigin )
{
	Globals::cl_enginefuncs->pTriAPI->ScreenToWorld( vScreen, vWorldOrigin );
}

//-----------------------------------------------------------------------------
// Player utilities
//-----------------------------------------------------------------------------

uint64 CGameUtils::GetSteamID( int iPlayerIndex )
{
	if ( iPlayerIndex > 0 && iPlayerIndex <= MAX_CLIENTS )
	{
		player_info_s *pPlayerInfo = Globals::enginestudio->PlayerInfo( iPlayerIndex - 1 );

		if ( pPlayerInfo == NULL )
			return 0uLL;

		return pPlayerInfo->m_nSteamID;
	}

	return 0uLL;
}

void CGameUtils::GetViewAngles( int iPlayerIndex, Vector &va )
{
	if ( iPlayerIndex <= 0 || iPlayerIndex > Globals::cl_enginefuncs->GetMaxClients() )
	{
		va.Zero();
		return;
	}

	cl_entity_t *pPlayer = Globals::cl_enginefuncs->GetEntityByIndex( iPlayerIndex );
	if ( pPlayer == NULL )
	{
		va.Zero();
		return;
	}

	va.x = pPlayer->curstate.angles.x * ( 89.0f / 9.8876953125f );
	va.y = pPlayer->angles.y;
	va.z = pPlayer->angles.z;
}

void CGameUtils::GetViewAngles( cl_entity_t *pPlayer, Vector &va )
{
	if ( pPlayer->index <= 0 || pPlayer->index > Globals::cl_enginefuncs->GetMaxClients() )
	{
		va.Zero();
		return;
	}

	va.x = pPlayer->curstate.angles.x * ( 89.0f / 9.8876953125f );
	va.y = pPlayer->angles.y;
	va.z = pPlayer->angles.z;
}

//-----------------------------------------------------------------------------
// Network Message
//-----------------------------------------------------------------------------

CNetMessageParams *CGameUtils::GetNetMessageParams( void )
{
	m_NetMessageParams.buffer = Globals::net_message;
	m_NetMessageParams.readcount = *Globals::msg_readcount;
	m_NetMessageParams.badread = !!( *Globals::msg_badread );

	return &m_NetMessageParams;
}

void CGameUtils::ApplyReadToNetMessageBuffer( CMessageBuffer *buffer )
{
	*Globals::msg_readcount = buffer->GetReadCount();
	*Globals::msg_badread = !buffer->ReadOK();
}

void CGameUtils::ApplyReadToNetMessageBuffer( int readcount, int badread )
{
	*Globals::msg_readcount = readcount;
	*Globals::msg_badread = badread;
}

const netmsg_t *CGameUtils::FindNetworkMessage( int iType )
{
	if ( iType > SVC_NOP && iType <= SVC_LASTMSG )
	{
		return const_cast<const netmsg_t *>( &Globals::g_NetworkMessages[ iType ] );
	}
	else
	{
		Warning( "[SvenInt] CGameUtils::FindNetworkMessage: bad network message type (%d)\n", iType );
	}

	return NULL;
}

const netmsg_t *CGameUtils::FindNetworkMessage( const char *pszName )
{
	for ( int i = SVC_NOP + 1; i <= SVC_LASTMSG; i++ )
	{
		netmsg_t *pNetMsg = &Globals::g_NetworkMessages[ i ];

		if ( !stricmp( pszName, pNetMsg->name ) )
		{
			return const_cast<const netmsg_t *>( pNetMsg );
		}
	}

	Warning( "[SvenInt] CHooks::HookNetworkMessage: bad network message name (%s)\n", pszName );

	return NULL;
}

//-----------------------------------------------------------------------------
// User Message
//-----------------------------------------------------------------------------

const usermsg_t *CGameUtils::FindUserMessage( const char *pszName )
{
	usermsg_t *pUserMsg = *Globals::g_pClientUserMsgs;

	while ( pUserMsg )
	{
		if ( !stricmp( pszName, pUserMsg->name ) )
		{
			return const_cast<const usermsg_t *>( pUserMsg );
		}

		pUserMsg = pUserMsg->next;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Event Hook
//-----------------------------------------------------------------------------

const event_hook_t *CGameUtils::FindEventHook( const char *pszName )
{
	event_hook_t *pEventHook = Globals::g_pEventHooks;

	while ( pEventHook )
	{
		if ( pEventHook->name )
		{
			if ( !stricmp( pszName, pEventHook->name ) )
			{
				return const_cast<const event_hook_t *>( pEventHook );
			}
		}

		pEventHook = pEventHook->next;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Print to client's chat, not visible to others and when playing back demo
//-----------------------------------------------------------------------------

void CGameUtils::PrintChatText( const char *pszMessage, ... )
{
	size_t len;

	static char buffer[ 1024 ];
	static char szFormattedMsg[ 1024 ];
	static const usermsg_t *pSayText = NULL;

	if ( Globals::cls->state < ca_uninitialized || pszMessage == NULL || ( len = strlen( pszMessage ) ) == 0 )
		return;

	if ( pSayText == NULL )
		pSayText = FindUserMessage( "SayText" );

	va_list args;
	va_start( args, pszMessage );
	vsnprintf( szFormattedMsg, Q_ARRAYSIZE( szFormattedMsg ), pszMessage, args );
	va_end( args );

	szFormattedMsg[ Q_ARRAYSIZE( szFormattedMsg ) - 1 ] = 0;

	CMessageBuffer msgbuffer( buffer, Q_ARRAYSIZE( buffer ) );

	msgbuffer.WriteByte( 0 );
	msgbuffer.WriteString( szFormattedMsg );

	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	//Msg( pszMessage );
	pSayText->function( "SayText", msgbuffer.GetBuffer()->cursize, msgbuffer.GetBuffer()->data );
}

//-----------------------------------------------------------------------------
// Draw utilities
//-----------------------------------------------------------------------------

void CGameUtils::DrawSetTextColor( float r, float g, float b )
{
	Globals::cl_enginefuncs->pfnDrawSetTextColor( r, g, b );
}

int CGameUtils::DrawConsoleString( int x, int y, const char *pszFormat, ... )
{
	static char szFormattedMsg[ 4096 ] = { 0 };

	va_list args;
	va_start( args, pszFormat );
	vsnprintf( szFormattedMsg, Q_ARRAYSIZE( szFormattedMsg ), pszFormat, args );
	va_end( args );

	szFormattedMsg[ Q_ARRAYSIZE( szFormattedMsg ) - 1 ] = 0;

	return Globals::cl_enginefuncs->pfnDrawConsoleString( x, y, szFormattedMsg );
}

CGameUtils gGameUtils;
CGameUtils *GameUtils()
{
	return &gGameUtils;
}

//-----------------------------------------------------------------------------
// Rest utilities
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: Gets full path of the current process
//-----------------------------------------------------------------------------

char *UTIL_GetLongPathName( void )
{
#ifdef WIN32
	char szShortPath[ MAX_PATH ];
	static char szLongPath[ MAX_PATH ];
	char *pszPath = NULL;

	szShortPath[ 0 ] = 0;
	szLongPath[ 0 ] = 0;

	if ( GetModuleFileName( NULL, szShortPath, sizeof( szShortPath ) ) )
	{
		GetLongPathName( szShortPath, szLongPath, sizeof( szLongPath ) );
		pszPath = strrchr( szLongPath, '\\' );

		if ( pszPath[ 0 ] )
			pszPath[ 1 ] = 0;

		size_t len = strlen( szLongPath );

		if ( len > 0 )
		{
			if ( szLongPath[ len - 1 ] == '\\' )
				szLongPath[ len - 1 ] = 0;
		}
	}
#else
	static char szLongPath[ PATH_MAX ];

	szLongPath[ 0 ] = 0;

	if ( readlink( "/proc/self/exe", szLongPath, PATH_MAX ) != -1 )
	{
		char *pszPath = strrchr( szLongPath, '/' );

		if ( pszPath[ 0 ] )
			pszPath[ 1 ] = 0;

		size_t len = strlen( szLongPath );

		if ( len > 0 )
		{
			if ( szLongPath[ len - 1 ] == '/' )
				szLongPath[ len - 1 ] = 0;
		}
	}
#endif

	return szLongPath;
}

//-----------------------------------------------------------------------------
// Client utilities
//-----------------------------------------------------------------------------

bool UTIL_IsDead( void )
{
	return Globals::demoplayback ?
		( Globals::refparams->health == 0 ) || ( Globals::gpHideHUD != NULL && ( *Globals::gpHideHUD & HIDEHUD_HEALTH ) ) :
		( Globals::playermove->iuser1() != 0 || Globals::playermove->dead() );
}

bool UTIL_IsSpectating( void )
{
	return Globals::demoplayback ?
		( Globals::gpHideHUD != NULL && ( *Globals::gpHideHUD & HIDEHUD_HEALTH ) ) :
		( Globals::playermove->iuser1() != 0 );
}

int UTIL_GetLocalPlayerIndex( void )
{
	return Globals::demoplayback ?
		Globals::cl_enginefuncs->GetLocalPlayer()->index :
		Globals::playermove->player_index() + 1;
}

bool UTIL_GetPlayerMoveLadder( pm_ladder_t *ladder )
{
	if ( ladder == NULL )
		return false;

	ladder->pe = NULL;
	ladder->angle = 0.f;

	if ( Globals::localplayer->GetMoveType() != MOVETYPE_FLY || Globals::localplayer->IsDead() )
		return false;

	physent_t *pe;
	Vector &vecOrigin = *Globals::playermove->origin();

	Vector vecPlayerMins = vecOrigin;
	Vector vecPlayerMaxs = vecOrigin;

	if ( Globals::playermove->flags() & FL_DUCKING )
	{
		vecPlayerMins += VEC_DUCK_HULL_MIN;
		vecPlayerMaxs += VEC_DUCK_HULL_MAX;
	}
	else
	{
		vecPlayerMins += VEC_HULL_MIN;
		vecPlayerMaxs += VEC_HULL_MAX;
	}

	for ( int i = 0; i < Globals::playermove->nummoveent(); i++ )
	{
		pe = &Globals::playermove->moveents()[ i ];

		if ( pe->model == NULL || pe->model->type != mod_brush || pe->skin != CONTENTS_LADDER )
			continue;

		pmtrace_t trace;

		const Vector &vecMins = pe->model->mins;
		const Vector &vecMaxs = pe->model->maxs;

		if ( vecPlayerMins.z > vecMaxs.z || vecPlayerMaxs.z < vecMins.z )
			continue;

		if ( !UTIL_IsAABBIntersectingAABB( vecMins, vecMaxs, vecPlayerMins + Vector( 0.f, 0.f, -2.f ), vecPlayerMaxs + Vector( 0.f, 0.f, 2.f ) ) )
			continue;

		Vector vecLadderCenter = ( vecMins + vecMaxs ) * 0.5f;
		Globals::playermove->funcs()->PM_TraceModel( pe, vecOrigin, vecLadderCenter, &trace );

		ladder->pe = pe;
		ladder->normal = trace.plane.normal;
		ladder->angle = VEC_RAD2DEG( atan2f( trace.plane.normal.y, trace.plane.normal.x ) );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Intersection tests
//-----------------------------------------------------------------------------

bool UTIL_IsLineIntersectingAABB( const Vector &p1, const Vector &p2, const Vector &vecBoxMins, const Vector &vecBoxMaxs )
{
	Vector vecLineDir = ( p2 - p1 ) * 0.5f;
	Vector vecBoxMid = ( vecBoxMaxs - vecBoxMins ) * 0.5f;
	Vector p3 = p1 + vecLineDir - ( vecBoxMins + vecBoxMaxs ) * 0.5f;
	Vector vecAbsLineMid = Vector( abs( vecLineDir.x ), abs( vecLineDir.y ), abs( vecLineDir.z ) );

	if ( abs( p3.x ) > vecBoxMid.x + vecAbsLineMid.x || abs( p3.y ) > vecBoxMid.y + vecAbsLineMid.y || abs( p3.z ) > vecBoxMid.z + vecAbsLineMid.z )
		return false;

	if ( abs( vecLineDir.y * p3.z - vecLineDir.z * p3.y ) > vecBoxMid.y * vecAbsLineMid.z + vecBoxMid.z * vecAbsLineMid.y )
		return false;

	if ( abs( vecLineDir.z * p3.x - vecLineDir.x * p3.z ) > vecBoxMid.z * vecAbsLineMid.x + vecBoxMid.x * vecAbsLineMid.z )
		return false;

	if ( abs( vecLineDir.x * p3.y - vecLineDir.y * p3.x ) > vecBoxMid.x * vecAbsLineMid.y + vecBoxMid.y * vecAbsLineMid.x )
		return false;

	return true;
}

bool UTIL_IsSphereIntersectingAABB( const Vector &vecCenter, const float flRadiusSqr, const Vector &vecAbsMins, const Vector &vecAbsMaxs, float *pflOutDistance )
{
	float flDistanceToEdge = 0.f;
	float flDistanceSqr = 0.f;

	for ( int i = 0; i < 3 /* && flDistanceSqr <= flRadiusSqr */; i++ )
	{
		if ( vecCenter[ i ] < vecAbsMins[ i ] )
		{
			flDistanceToEdge = vecCenter[ i ] - vecAbsMins[ i ];
		}
		else if ( vecCenter[ i ] > vecAbsMaxs[ i ] )
		{
			flDistanceToEdge = vecCenter[ i ] - vecAbsMaxs[ i ];
		}
		else
		{
			flDistanceToEdge = 0.f;
		}

		flDistanceSqr += flDistanceToEdge * flDistanceToEdge;
	}

	if ( flDistanceSqr <= flRadiusSqr )
	{
		if ( pflOutDistance != NULL )
			*pflOutDistance = sqrtf( flDistanceSqr );

		return true;
	}

	if ( pflOutDistance != NULL )
		*pflOutDistance = -1.f;

	return false;
}

bool UTIL_IsRayIntersectingAABB( const Vector &vecBoxMins, const Vector &vecBoxMaxs, const Vector &vecRayOrigin, const Vector &vecRayDir, float *pflMinIntersection, float *pflMaxIntersection )
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	if ( pflMinIntersection != NULL )
		*pflMinIntersection = 0.f;

	if ( pflMaxIntersection != NULL )
		*pflMaxIntersection = 0.f;

	if ( vecRayDir.x >= 0 )
	{
		tmin = ( vecBoxMins.x - vecRayOrigin.x ) / vecRayDir.x;
		tmax = ( vecBoxMaxs.x - vecRayOrigin.x ) / vecRayDir.x;
	}
	else
	{
		tmin = ( vecBoxMaxs.x - vecRayOrigin.x ) / vecRayDir.x;
		tmax = ( vecBoxMins.x - vecRayOrigin.x ) / vecRayDir.x;
	}

	if ( vecRayDir.y >= 0 )
	{
		tymin = ( vecBoxMins.y - vecRayOrigin.y ) / vecRayDir.y;
		tymax = ( vecBoxMaxs.y - vecRayOrigin.y ) / vecRayDir.y;
	}
	else
	{
		tymin = ( vecBoxMaxs.y - vecRayOrigin.y ) / vecRayDir.y;
		tymax = ( vecBoxMins.y - vecRayOrigin.y ) / vecRayDir.y;
	}

	if ( tmin > tymax || tymin > tmax )
		return false;

	if ( tymin > tmin )
		tmin = tymin;

	if ( tymax < tmax )
		tmax = tymax;

	if ( vecRayDir.z >= 0 )
	{
		tzmin = ( vecBoxMins.z - vecRayOrigin.z ) / vecRayDir.z;
		tzmax = ( vecBoxMaxs.z - vecRayOrigin.z ) / vecRayDir.z;
	}
	else
	{
		tzmin = ( vecBoxMaxs.z - vecRayOrigin.z ) / vecRayDir.z;
		tzmax = ( vecBoxMins.z - vecRayOrigin.z ) / vecRayDir.z;
	}

	if ( tmin > tzmax || tzmin > tmax )
		return false;

	if ( tzmin > tmin )
		tmin = tzmin;

	if ( tzmax < tmax )
		tmax = tzmax;

	if ( pflMinIntersection != NULL )
		*pflMinIntersection = tmin;

	if ( pflMaxIntersection != NULL )
		*pflMaxIntersection = tmax;

	return true;
}


bool UTIL_IsRayIntersectingSphere( const Vector &rayOrigin, const Vector &rayDir, const Vector &vecCenter, const float flRadius, float *pflOutDistance )
{
	Vector L = rayOrigin - vecCenter;

	float radiusSq = ( flRadius * flRadius ) * 0.25f;

	float a = rayDir.Dot( rayDir );
	float b = 2.0f * L.Dot( rayDir );
	float c = L.Dot( L ) - radiusSq;

	float discr = b * b - 4.0f * a * c;

	if ( discr < 0 ) return false;

	float sqrtDiscr = sqrtf( discr );
	float t0 = ( -b - sqrtDiscr ) / ( 2.0f * a );
	float t1 = ( -b + sqrtDiscr ) / ( 2.0f * a );

	if ( t0 > t1 ) std::swap( t0, t1 );

	if ( t0 < 0 )
	{
		t0 = t1;
		if ( t0 < 0 )
			return false;
	}

	if ( pflOutDistance != NULL )
		*pflOutDistance = t0;

	return true;
}

//-----------------------------------------------------------------------------
// Shared Random
//-----------------------------------------------------------------------------

static unsigned int glSeed = 0;

static unsigned int seed_table[ 256 ] =
{
	28985U, 27138U, 26457U, 9451U, 17764U, 10909U, 28790U, 8716U, 6361U, 4853U, 17798U, 21977U, 19643U, 20662U, 10834U, 20103,
	27067U, 28634U, 18623U, 25849U, 8576U, 26234U, 23887U, 18228U, 32587U, 4836U, 3306U, 1811U, 3035U, 24559U, 18399U, 315,
	26766U, 907U, 24102U, 12370U, 9674U, 2972U, 10472U, 16492U, 22683U, 11529U, 27968U, 30406U, 13213U, 2319U, 23620U, 16823,
	10013U, 23772U, 21567U, 1251U, 19579U, 20313U, 18241U, 30130U, 8402U, 20807U, 27354U, 7169U, 21211U, 17293U, 5410U, 19223,
	10255U, 22480U, 27388U, 9946U, 15628U, 24389U, 17308U, 2370U, 9530U, 31683U, 25927U, 23567U, 11694U, 26397U, 32602U, 15031,
	18255U, 17582U, 1422U, 28835U, 23607U, 12597U, 20602U, 10138U, 5212U, 1252U, 10074U, 23166U, 19823U, 31667U, 5902U, 24630,
	18948U, 14330U, 14950U, 8939U, 23540U, 21311U, 22428U, 22391U, 3583U, 29004U, 30498U, 18714U, 4278U, 2437U, 22430U, 3439,
	28313U, 23161U, 25396U, 13471U, 19324U, 15287U, 2563U, 18901U, 13103U, 16867U, 9714U, 14322U, 15197U, 26889U, 19372U, 26241,
	31925U, 14640U, 11497U, 8941U, 10056U, 6451U, 28656U, 10737U, 13874U, 17356U, 8281U, 25937U, 1661U, 4850U, 7448U, 12744,
	21826U, 5477U, 10167U, 16705U, 26897U, 8839U, 30947U, 27978U, 27283U, 24685U, 32298U, 3525U, 12398U, 28726U, 9475U, 10208,
	617U, 13467U, 22287U, 2376U, 6097U, 26312U, 2974U, 9114U, 21787U, 28010U, 4725U, 15387U, 3274U, 10762U, 31695U, 17320,
	18324U, 12441U, 16801U, 27376U, 22464U, 7500U, 5666U, 18144U, 15314U, 31914U, 31627U, 6495U, 5226U, 31203U, 2331U, 4668,
	12650U, 18275U, 351U, 7268U, 31319U, 30119U, 7600U, 2905U, 13826U, 11343U, 13053U, 15583U, 30055U, 31093U, 5067U, 761,
	9685U, 11070U, 21369U, 27155U, 3663U, 26542U, 20169U, 12161U, 15411U, 30401U, 7580U, 31784U, 8985U, 29367U, 20989U, 14203,
	29694U, 21167U, 10337U, 1706U, 28578U, 887U, 3373U, 19477U, 14382U, 675U, 7033U, 15111U, 26138U, 12252U, 30996U, 21409,
	25678U, 18555U, 13256U, 23316U, 22407U, 16727U, 991U, 9236U, 5373U, 29402U, 6117U, 15241U, 27715U, 19291U, 19888U, 19847U
};

static unsigned int U_Random()
{
	glSeed *= 69069;
	glSeed += seed_table[ glSeed & 0xFF ] + 1;
	return ( glSeed & 0xFFFFFFF );
}

static void U_Srand( unsigned int seed )
{
	glSeed = seed_table[ seed & 0xFF ];
}

int UTIL_SharedRandomLong( unsigned int seed, int low, int high )
{
	unsigned int range = high - low + 1;
	U_Srand( (unsigned int)( high + low + seed ) );
	if ( range != 1 )
	{
		int rnum = U_Random();
		int offset = rnum % range;
		return ( low + offset );
	}

	return low;
}

float UTIL_SharedRandomFloat( unsigned int seed, float low, float high )
{
	unsigned int range = (unsigned int)( high - low );
	U_Srand( (unsigned int)seed + *(unsigned int *)&low + *(unsigned int *)&high );

	U_Random();
	U_Random();

	if ( range )
	{
		int tensixrand = U_Random() & 0xFFFFu;
		float offset = float( tensixrand ) / 0x10000u;
		return ( low + offset * range );
	}

	return low;

	//int v3; // ecx
	//int v4; // ecx
	//int v5; // ecx

	//v3 = 69069 * seed_table[(unsigned __int8)(seed + LOBYTE(low) + LOBYTE(high))];
	//v4 = 69069 * (v3 + 1 + seed_table[(unsigned __int8)v3]);
	//v5 = seed_table[(unsigned __int8)v4] + v4 + 1;
	//glSeed = v5;
	//if ((float)(high - low) == 0.0)
	//	return low;
	//glSeed = 69069 * v5 + seed_table[(unsigned __int8)(-51 * v5)] + 1;
	//return (float)((float)((float)((float)(unsigned __int16)glSeed * 0.0000152587890625) * (float)(high - low)) + low);
}

//-----------------------------------------------------------------------------
// char * to wchar_t *
//-----------------------------------------------------------------------------

const wchar_t *UTIL_CStringToWideCString( const char *pszString )
{
	const size_t length = strlen( pszString ) + 1;
	wchar_t *wcString = new wchar_t[ length ];

	mbstowcs( wcString, pszString, length );

	return wcString;
}

//-----------------------------------------------------------------------------
// Player move utilities
//-----------------------------------------------------------------------------

int UTIL_ClipVelocity( const Vector &in, const Vector &normal, Vector &out, float overbounce )
{
	float	backoff;
	float	change;
	float angle;
	int		i, blocked;

	const float STOP_EPSILON = 0.1f;

	angle = normal[ 2 ];

	blocked = 0x00;            // Assume unblocked.
	if ( angle > 0 )      // If the plane that is blocking us has a positive z component, then assume it's a floor.
		blocked |= 0x01;		// 
	if ( !angle )         // If the plane has no Z, it is vertical (wall/step)
		blocked |= 0x02;		// 

	// Determine how far along plane to slide based on incoming direction.
	// Scale by overbounce factor.
	backoff = DotProduct( in, normal ) * overbounce;

	for ( i = 0; i < 3; i++ )
	{
		change = normal[ i ] * backoff;
		out[ i ] = in[ i ] - change;
		// If out velocity is too small, zero it out.
		if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON )
			out[ i ] = 0.f;
	}

	// Return blocking flags.
	return blocked;
}

void UTIL_AddCorrectGravity( Vector &vecVelocity, float frametime )
{
	float ent_gravity;

	if ( Globals::playermove->gravity() )
		ent_gravity = Globals::playermove->gravity();
	else
		ent_gravity = 1.f;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	vecVelocity.z -= ( ent_gravity * Globals::playermove->movevars()->gravity * 0.5f * frametime );
}

void UTIL_FixupGravityVelocity( Vector &vecVelocity, float frametime )
{
	float ent_gravity;

	if ( Globals::playermove->gravity() )
		ent_gravity = Globals::playermove->gravity();
	else
		ent_gravity = 1.f;

	vecVelocity.z -= ( ent_gravity * Globals::playermove->movevars()->gravity * frametime * 0.5f );
}

void UTIL_AddCorrectGravity( Vector &vecVelocity, float gravity, float entgravity, float frametime )
{
	float ent_gravity;

	if ( entgravity )
		ent_gravity = entgravity;
	else
		ent_gravity = 1.f;

	// Add gravity so they'll be in the correct position during movement
	// yes, this 0.5 looks wrong, but it's not.  
	vecVelocity.z -= ( ent_gravity * gravity * 0.5f * frametime );
}

void UTIL_FixupGravityVelocity( Vector &vecVelocity, float gravity, float entgravity, float frametime )
{
	float ent_gravity;

	if ( entgravity )
		ent_gravity = entgravity;
	else
		ent_gravity = 1.f;

	vecVelocity.z -= ( ent_gravity * gravity * frametime * 0.5f );
}

//-----------------------------------------------------------------------------
// Hull intersection
//-----------------------------------------------------------------------------

void UTIL_FindHullIntersection( const Vector &vecSrc, TraceResult &tr, float *mins, float *maxs, edict_t *pEntity )
{
	int			i, j, k;
	float		distance;
	float *minmaxs[ 2 ] = { mins, maxs };
	TraceResult tmpTrace;
	Vector		vecHullEnd = tr.vecEndPos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );
	Globals::sv_enginefuncs->pfnTraceLine( vecSrc, vecHullEnd, 0, pEntity, &tmpTrace );

	if ( tmpTrace.flFraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[ i ][ 0 ];
				vecEnd.y = vecHullEnd.y + minmaxs[ j ][ 1 ];
				vecEnd.z = vecHullEnd.z + minmaxs[ k ][ 2 ];

				Globals::sv_enginefuncs->pfnTraceLine( vecSrc, vecEnd, 0, pEntity, &tmpTrace );
				if ( tmpTrace.flFraction < 1.0 )
				{
					float thisDistance = ( tmpTrace.vecEndPos - vecSrc ).Length();
					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

void UTIL_FindHullIntersectionClient( const Vector &vecSrc, pmtrace_t &tr, float *mins, float *maxs, int ignore_ent )
{
	int			i, j, k;
	float		distance;
	float *minmaxs[ 2 ] = { mins, maxs };
	pmtrace_t	tmpTrace;
	Vector		vecHullEnd = tr.endpos;
	Vector		vecEnd;

	distance = 1e6f;

	vecHullEnd = vecSrc + ( ( vecHullEnd - vecSrc ) * 2 );

	Globals::cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	Globals::cl_enginefuncs->pEventAPI->EV_PlayerTrace( (float *)&vecSrc, vecHullEnd, PM_NORMAL, ignore_ent, &tmpTrace );

	if ( tmpTrace.fraction < 1.0 )
	{
		tr = tmpTrace;
		return;
	}

	for ( i = 0; i < 2; i++ )
	{
		for ( j = 0; j < 2; j++ )
		{
			for ( k = 0; k < 2; k++ )
			{
				vecEnd.x = vecHullEnd.x + minmaxs[ i ][ 0 ];
				vecEnd.y = vecHullEnd.y + minmaxs[ j ][ 1 ];
				vecEnd.z = vecHullEnd.z + minmaxs[ k ][ 2 ];

				Globals::cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
				Globals::cl_enginefuncs->pEventAPI->EV_PlayerTrace( (float *)&vecSrc, vecEnd, PM_NORMAL, ignore_ent, &tmpTrace );

				if ( tmpTrace.fraction < 1.0 )
				{
					float thisDistance = ( tmpTrace.endpos - vecSrc ).Length();

					if ( thisDistance < distance )
					{
						tr = tmpTrace;
						distance = thisDistance;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Linear algebra smh
//-----------------------------------------------------------------------------

Vector UTIL_ClampVectorToBox( const Vector &input, const Vector &clampSize )
{
	Vector sourceVector = input;

	if ( sourceVector.x > clampSize.x )
		sourceVector.x -= clampSize.x;
	else if ( sourceVector.x < -clampSize.x )
		sourceVector.x += clampSize.x;
	else
		sourceVector.x = 0;

	if ( sourceVector.y > clampSize.y )
		sourceVector.y -= clampSize.y;
	else if ( sourceVector.y < -clampSize.y )
		sourceVector.y += clampSize.y;
	else
		sourceVector.y = 0;

	if ( sourceVector.z > clampSize.z )
		sourceVector.z -= clampSize.z;
	else if ( sourceVector.z < -clampSize.z )
		sourceVector.z += clampSize.z;
	else
		sourceVector.z = 0;

	return sourceVector.Normalize();
}

//-----------------------------------------------------------------------------
// Viewport transformations
//-----------------------------------------------------------------------------

bool UTIL_WorldToScreen( float *pflOrigin, float *pflVecScreen )
{
	return Globals::gameutils->WorldToScreen( *(Vector *)pflOrigin, *(Vector2D *)pflVecScreen );
}

void UTIL_ScreenToWorld( float *pflNDC, float *pflWorldOrigin )
{
	Globals::cl_enginefuncs->pTriAPI->ScreenToWorld( pflNDC, pflWorldOrigin );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool UTIL_IsBusyWithLongJump( struct usercmd_s *cmd )
{
	if ( cmd->buttons & IN_JUMP &&
		 Globals::localplayer->IsOnGround() &&
		 Globals::localplayer->IsDucking() &&
		 cmd->buttons & IN_DUCK /* && g_pPlayerMove->flDuckTime > 0.f */ )
	{
		const char *pszValue = Globals::cl_enginefuncs->PhysInfo_ValueForKey( "slj" );
		bool bCanSuperJump = ( pszValue != NULL && *pszValue == '1' );

		if ( bCanSuperJump && Globals::localplayer->GetVelocity().Length() > 50.f )
		{
			return true;
		}
	}

	return false;
}

bool UTIL_IsContiniousFiring( struct usercmd_s *cmd )
{
	if ( Globals::localplayer->HasWeapon() )
	{
		switch ( Globals::localplayer->GetCurrentWeaponID() )
		{
		case WEAPON_M16:
			if ( Globals::clientweapon->GetWeaponData()->fuser2 != 0.f )
				return true;

			break;
		}
	}

	return false;
}

bool UTIL_IsFiring( struct usercmd_s *cmd )
{
	static int throw_nade_state = 0;

	if ( Globals::localplayer->HasWeapon() )
	{
		switch ( Globals::localplayer->GetCurrentWeaponID() )
		{
		case WEAPON_M16:
			if ( Globals::clientweapon->GetWeaponData()->fuser2 != 0.f )
				return true;

			break;

		case WEAPON_RPG:
			if ( Globals::clientweapon->GetWeaponData()->iuser4 && Globals::clientweapon->GetWeaponData()->fuser1 != 0.f )
				return true;

			if ( cmd->buttons & IN_ATTACK2 )
				return false;

			break;

		case WEAPON_GAUSS:
			if ( Globals::clientweapon->GetWeaponData()->fuser4 > 0.f )
			{
				if ( Globals::localplayer->ButtonLast() & IN_ATTACK2 )
				{
					if ( !( cmd->buttons & IN_ATTACK2 ) )
						return true;
				}
				else if ( Globals::localplayer->ButtonLast() & IN_ALT1 )
				{
					if ( !( cmd->buttons & IN_ALT1 ) )
						return true;
				}
				else if ( Globals::clientweapon->GetWeaponData()->fuser4 == 1.f )
				{
					return true;
				}

				return false;
			}
			else if ( cmd->buttons & IN_ATTACK2 )
			{
				return false;
			}

			break;

		case WEAPON_HANDGRENADE:
			if ( Globals::clientweapon->GetWeaponData()->fuser1 < 0.f && throw_nade_state != 2 )
			{
				throw_nade_state = 1;

				if ( Globals::localplayer->ButtonLast() & ( IN_ATTACK | IN_ATTACK2 ) )
				{
					if ( !( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) )
						return true;
				}
				else
				{
					if ( !( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) ) )
						throw_nade_state = 2;
				}
			}

			if ( Globals::clientweapon->GetWeaponData()->fuser2 < 0.f && throw_nade_state == 2 )
				return true;

			throw_nade_state = 0;
			return false;

		case WEAPON_DISPLACER:
			if ( Globals::clientweapon->GetWeaponData()->fuser1 == 1.f )
				return true;

			return false;
		}

		if ( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) && Globals::localplayer->CanAttack() && !Globals::clientweapon->IsReloading() )
		{
			if ( cmd->buttons & IN_ATTACK )
			{
				if ( Globals::clientweapon->CanPrimaryAttack() )
					return true;
			}
			else
			{
				if ( Globals::clientweapon->CanSecondaryAttack() )
					return true;
			}
		}
	}

	return false;
}

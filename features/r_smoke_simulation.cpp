// SvenInt (c) Sw1ft
// Smoke simulation logic & rendering code by Admer456
// https://github.com/Admer456/halflife-smoke
// r_smoke_simulation.cpp

#include "stdafx.h"
#include "misc_random_generator.h"
#include "misc_entity_list.h"
#include "r_draw_context.h"
#include "r_smoke_simulation.h"
#include "game/messagebuffer.h"
#include "utils/prof.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSmokeSimulation, smokesim, "Render", "Smoke Simulation" );

static NetMsgHookFn ORIG_NetMsgHook_TempEntity = NULL;

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

#ifdef DEBUG
CON_COMMAND( sc_smoke, "" )
{
	Vector vecEyes = localplayer->GetEyePosition();
	QAngle va;

	cl_enginefuncs->GetViewAngles( va );

	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
											   vecEyes + va.GetForward() * 2048,
											   PM_NORMAL,
											   -1,
											   &trace );

	THIS_FEATURE()->SpawnSmoke( trace.endpos );
}

CON_COMMAND( sc_smoke_trace_bullet, "" )
{
	Vector vecEyes = localplayer->GetEyePosition();
	QAngle va;

	cl_enginefuncs->GetViewAngles( va );

	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
											   vecEyes + va.GetForward() * 2048,
											   PM_NORMAL,
											   -1,
											   &trace );


	THIS_FEATURE()->TraceBullet( 0, vecEyes, trace.endpos );
}

CON_COMMAND( sc_smoke_trace_boom, "" )
{
	if ( args.ArgC() < 3 )
		return;

	Vector vecEyes = localplayer->GetEyePosition();
	QAngle va;

	cl_enginefuncs->GetViewAngles( va );

	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
											   vecEyes + va.GetForward() * 2048,
											   PM_NORMAL,
											   -1,
											   &trace );


	THIS_FEATURE()->TraceExplosion( trace.endpos, (float)atof( args[ 1 ] ), (float)atof( args[ 2 ] ) );
}

CON_COMMAND( sc_smoke_flash, "" )
{
	if ( args.ArgC() < 5 )
		return;

	float r, g, b, intensity;

	r = (float)atof( args[ 1 ] );
	g = (float)atof( args[ 2 ] );
	b = (float)atof( args[ 3 ] );
	intensity = (float)atof( args[ 4 ] );

	THIS_FEATURE()->OnFlash( localplayer->GetEyePosition(), Vector( r, g, b ), intensity );
}
#endif

//-----------------------------------------------------------------------------
// NetMsgHook_TempEntity
//-----------------------------------------------------------------------------

static void HOOKED_NetMsgHook_TempEntity( void )
{
	if ( !THIS_FEATURE_IS_RUNNING() )
	{
		ORIG_NetMsgHook_TempEntity();
		return;
	}

	static CMessageBuffer msg;
	CNetMessageParams *params = Globals::gameutils->GetNetMessageParams();
	msg.Init( params->buffer, params->readcount, params->badread );

	int entitytype = msg.ReadByte();
	if ( entitytype == TE_EXPLOSION )
	{
		float x = msg.ReadCoord();
		float y = msg.ReadCoord();
		float z = msg.ReadCoord();
		msg.ReadShort();
		int scale = msg.ReadByte();

		THIS_FEATURE()->OnExplosion( Vector( x, y, z ), (float)scale * 5.f, 2.5f );
	}

	ORIG_NetMsgHook_TempEntity();
}

//-----------------------------------------------------------------------------
// CSmokeParticle::Update
//-----------------------------------------------------------------------------

void CSmokeParticle::Update( const float time, const float frametime )
{
	m_vecPos += m_vecVelocity * frametime;

	// Particles with much velocity die faster
	if ( m_state != kSmokeParticleInactive && m_state != kSmokeParticleShot )
	{
		const float flSpeed = m_vecVelocity.Length();
		if ( flSpeed > 10.0f )
		{
			const float flExpansionRate = flSpeed * 0.1f;
			m_flRadius += flExpansionRate * frametime;

			const float flLifeLoss = ( flSpeed * 0.2f ) * frametime;
			m_flLifeTime -= flLifeLoss;
		}
	}

	const float flTimeDeath = m_flSpawnTime + m_flLifeTime;
	const float alpha = Transparency( time );

	switch ( m_state )
	{
	case kSmokeParticleForming:
		m_vecVelocity -= m_vecVelocity * frametime * 0.2f;
		m_flRadius += m_flRadius * frametime * 0.03f;

		if ( time - m_flSpawnTime > m_flFadeTime )
		{
			m_state = kSmokeParticleSpreading;
		}

		break;

	case kSmokeParticleSpreading:
		m_vecVelocity -= m_vecVelocity * frametime * 0.1f;
		m_flRadius += m_flRadius * frametime * 0.025f;

		if ( ( flTimeDeath - time ) <= m_flFadeTime )
		{
			m_state = kSmokeParticleDissipating;
		}

		break;

	case kSmokeParticleDissipating:
		m_flRadius += m_flRadius * frametime * 0.06f;
		m_vecVelocity.z += ( 1.f - alpha ) * frametime * 20.f;

		if ( time >= flTimeDeath )
		{
			m_state = kSmokeParticleInactive;
		}

		break;

	case kSmokeParticleShot:
		m_flRadius += frametime * 200.f;
		m_flFadeTime -= frametime * 5.f;

		if ( m_flFadeTime < 0.f )
		{
			m_state = kSmokeParticleInactive;
		}

		break;
	}
}

//-----------------------------------------------------------------------------
// CheckPVS
//-----------------------------------------------------------------------------

bool CSmokeCloud::CheckPVS( void )
{
	return m_bInPVS = ( cl_enginefuncs->pTriAPI->BoxInPVS( GetMins(), GetMaxs() ) != 0 );
}

//-----------------------------------------------------------------------------
// TraceBullet
//-----------------------------------------------------------------------------

void CSmokeCloud::TraceBullet( int iWeaponID, const Vector &p1, const Vector &p2, float flForce )
{
	if ( !UTIL_IsLineIntersectingAABB( p1, p2, GetMins(), GetMaxs() ) )
		return;

	// The ray
	Vector dir = p2 - p1;
	float lineLen = dir.Length();

	if ( lineLen < 0.0001f )
		return;

	// Normalize it
	dir /= lineLen;

	// Current pos within our grid
	Vector startInGrid = ( p1 - GetGrid().m_vecMinBounds ) / GetGrid().m_flCellSize;

	int curX = Q_clamp( (int)startInGrid.x, 0, GetGrid().m_iResolution - 1 );
	int curY = Q_clamp( (int)startInGrid.y, 0, GetGrid().m_iResolution - 1 );
	int curZ = Q_clamp( (int)startInGrid.z, 0, GetGrid().m_iResolution - 1 );

	// Step dir
	int stepX = ( dir.x > 0 ) ? 1 : -1;
	int stepY = ( dir.y > 0 ) ? 1 : -1;
	int stepZ = ( dir.z > 0 ) ? 1 : -1;

	// Direction to nearest bound of a cell
	Vector tDelta;
	tDelta.x = std::abs( GetGrid().m_flCellSize / dir.x );
	tDelta.y = std::abs( GetGrid().m_flCellSize / dir.y );
	tDelta.z = std::abs( GetGrid().m_flCellSize / dir.z );

	Vector tMax;
	// Distance to nearest bound of a cell
	auto CalcTMax = [ & ]( float pos, float d, float cellSize, int step )
	{
		if ( std::abs( d ) < 0.000001f )
			return 1e30f;

		float cellBoundary = ( step > 0 ) ? floorf( pos ) + 1.0f : ceilf( pos ) - 1.0f;
		return ( cellBoundary - pos ) * ( cellSize / d );
	};

	tMax.x = CalcTMax( startInGrid.x, dir.x, GetGrid().m_flCellSize, stepX );
	tMax.y = CalcTMax( startInGrid.y, dir.y, GetGrid().m_flCellSize, stepY );
	tMax.z = CalcTMax( startInGrid.z, dir.z, GetGrid().m_flCellSize, stepZ );

	if ( THIS_FEATURE()->IsDebugDraw() )
	{
		Features::drawcontext->DrawLine( p1, p2, Color( 232, 50, 50, 50 ), 7.f, 2.f );
	}

	// Amanatides-Woo Algorithm
	// Iterate through cells
	while ( curX >= 0 && curX < GetGrid().m_iResolution &&
			curY >= 0 && curY < GetGrid().m_iResolution &&
			curZ >= 0 && curZ < GetGrid().m_iResolution )
	{
		int cellIdx = curX + curY * GetGrid().m_iResolution + curZ * GetGrid().m_iResolution * GetGrid().m_iResolution;
		int start = GetGrid().m_CellOffsets[ cellIdx ];
		int count = GetGrid().m_CellCounts[ cellIdx ];

		bool bSuccessTrace = false;

		for ( int i = 0; i < count; i++ )
		{
			int pIdx = GetGrid().m_ParticleIndexes[ start + i ];
			CSmokeParticle &particle = GetParticles()[ pIdx ];

			if ( !particle.IsActive() || particle.GetState() == kSmokeParticleShot )
				continue;

			float flOutDistance;
			if ( !UTIL_IsRayIntersectingSphere( p1, dir, particle.m_vecPos, particle.m_flRadius, &flOutDistance ) || flOutDistance > lineLen )
				continue;

			const Vector hitPosition = p1 + dir * flOutDistance;
			const Vector normal = ( hitPosition - particle.m_vecPos ) / particle.m_flRadius; // avoid using Normalize here

			// Larger particles will move slower, i.e. be less affected by a bullet.
			// Later we may simulate turbulence and other stuff.
			particle.m_vecVelocity -= normal * ( flForce / particle.m_flRadius ) * 50.f;
			particle.m_state = kSmokeParticleShot;
			particle.m_flFadeTime = 1.0f;
			particle.m_MetaData_ShotByEgonOrShockRifle = ( iWeaponID == WEAPON_EGON || iWeaponID == WEAPON_SHOCK_RIFLE );

			bSuccessTrace = true;
		}

		if ( THIS_FEATURE()->IsDebugDraw() )
		{
			Vector vecMins, vecMaxs;
			GetGrid().GetCellBounds( cellIdx, vecMins, vecMaxs );
			Features::drawcontext->DrawBox( vecMins, Vector(), vecMaxs - vecMins, bSuccessTrace ? Color( 232, 50, 232, 127 ) : Color( 232, 232, 50, 50 ), 2.f );
		}

		// Move to next cell
		if ( tMax.x < tMax.y )
		{
			if ( tMax.x < tMax.z )
			{
				if ( tMax.x > lineLen ) break;
				curX += stepX;
				tMax.x += tDelta.x;
			}
			else
			{
				if ( tMax.z > lineLen ) break;
				curZ += stepZ;
				tMax.z += tDelta.z;
			}
		}
		else
		{
			if ( tMax.y < tMax.z )
			{
				if ( tMax.y > lineLen ) break;
				curY += stepY;
				tMax.y += tDelta.y;
			}
			else
			{
				if ( tMax.z > lineLen ) break;
				curZ += stepZ;
				tMax.z += tDelta.z;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// TraceExplosion
//-----------------------------------------------------------------------------

void CSmokeCloud::TraceExplosion( const Vector &vecPos, float flRadius, float flForce )
{
	if ( !UTIL_IsSphereIntersectingAABB( vecPos, flRadius * flRadius, GetMins(), GetMaxs(), NULL ) )
		return;

	// Check intersection between explosion sphere and AABB cells
	int minX = Q_max( 0, (int)( ( vecPos.x - flRadius - GetGrid().m_vecMinBounds.x ) / GetGrid().m_flCellSize ) );
	int maxX = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecPos.x + flRadius - GetGrid().m_vecMinBounds.x ) / GetGrid().m_flCellSize ) );

	int minY = Q_max( 0, (int)( ( vecPos.y - flRadius - GetGrid().m_vecMinBounds.y ) / GetGrid().m_flCellSize ) );
	int maxY = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecPos.y + flRadius - GetGrid().m_vecMinBounds.y ) / GetGrid().m_flCellSize ) );

	int minZ = Q_max( 0, (int)( ( vecPos.z - flRadius - GetGrid().m_vecMinBounds.z ) / GetGrid().m_flCellSize ) );
	int maxZ = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecPos.z + flRadius - GetGrid().m_vecMinBounds.z ) / GetGrid().m_flCellSize ) );

	const float flCriticalRadius = flRadius * 0.4f;

	for ( int z = minZ; z <= maxZ; z++ )
	{
		for ( int y = minY; y <= maxY; y++ )
		{
			for ( int x = minX; x <= maxX; x++ )
			{
				int cellIdx = x + y * GetGrid().m_iResolution + z * GetGrid().m_iResolution * GetGrid().m_iResolution;
				int start = GetGrid().m_CellOffsets[ cellIdx ];
				int count = GetGrid().m_CellCounts[ cellIdx ];

				for ( int i = 0; i < count; i++ )
				{
					int pIdx = GetGrid().m_ParticleIndexes[ start + i ];
					CSmokeParticle &particle = GetParticles()[ pIdx ];

					if ( !particle.IsActive() || particle.GetState() == kSmokeParticleShot )
						continue;

					Vector vecDir = particle.m_vecPos - vecPos;
					const float flDistance = vecDir.NormalizeInPlace();

					const float flFalloff = 1.0f - ( flDistance / flRadius );
					particle.m_vecVelocity += vecDir * ( flForce * flFalloff );

					if ( flDistance <= flRadius && flDistance <= flCriticalRadius )
					{
						particle.m_state = kSmokeParticleShot;
						particle.m_flFadeTime = 1.0f;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// TraceBodyMove
//-----------------------------------------------------------------------------

void CSmokeCloud::TraceBodyMove( const Vector &vecBodyMins, const Vector &vecBodyMaxs, const Vector &vecMove )
{
	if ( !UTIL_IsAABBIntersectingAABB( vecBodyMins, vecBodyMaxs, GetMins(), GetMaxs() ) )
		return;

	const float flSpeed = vecMove.Length();
	if ( flSpeed < 0.5f )
		return;

	const Vector vecNewVelocity = vecMove / flSpeed;

	// Intersection bounds
	int minX = Q_max( 0, (int)( ( vecBodyMins.x - GetGrid().m_vecMinBounds.x ) / GetGrid().m_flCellSize ) );
	int maxX = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecBodyMaxs.x - GetGrid().m_vecMinBounds.x ) / GetGrid().m_flCellSize ) );

	int minY = Q_max( 0, (int)( ( vecBodyMins.y - GetGrid().m_vecMinBounds.y ) / GetGrid().m_flCellSize ) );
	int maxY = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecBodyMaxs.y - GetGrid().m_vecMinBounds.y ) / GetGrid().m_flCellSize ) );

	int minZ = Q_max( 0, (int)( ( vecBodyMins.z - GetGrid().m_vecMinBounds.z ) / GetGrid().m_flCellSize ) );
	int maxZ = Q_min( GetGrid().m_iResolution - 1, (int)( ( vecBodyMaxs.z - GetGrid().m_vecMinBounds.z ) / GetGrid().m_flCellSize ) );

	for ( int z = minZ; z <= maxZ; z++ )
	{
		for ( int y = minY; y <= maxY; y++ )
		{
			for ( int x = minX; x <= maxX; x++ )
			{

				int cellIdx = x + ( y * GetGrid().m_iResolution ) + ( z * GetGrid().m_iResolution * GetGrid().m_iResolution );
				int start = GetGrid().m_CellOffsets[ cellIdx ];
				int count = GetGrid().m_CellCounts[ cellIdx ];

				for ( int i = 0; i < count; i++ )
				{
					int pIdx = GetGrid().m_ParticleIndexes[ start + i ];
					CSmokeParticle &p = GetParticles()[ pIdx ];

					if ( !p.IsActive() )
						continue;

					if ( p.m_vecPos.x >= vecBodyMins.x && p.m_vecPos.x <= vecBodyMaxs.x &&
						 p.m_vecPos.y >= vecBodyMins.y && p.m_vecPos.y <= vecBodyMaxs.y &&
						 p.m_vecPos.z >= vecBodyMins.z && p.m_vecPos.z <= vecBodyMaxs.z )
					{
						Vector vecBodyCenter = ( vecBodyMins + vecBodyMaxs ) * 0.5f;
						Vector vecPushOut = ( p.m_vecPos - vecBodyCenter ).Normalize();
						p.m_vecVelocity += vecPushOut * 0.15f;
						p.m_vecVelocity += vecNewVelocity * 0.15f;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// GetCellBounds
//-----------------------------------------------------------------------------

void CSmokeCloudGrid::GetCellBounds( int cellndx, Vector &vecMins, Vector &vecMaxs ) const
{
	int iz = cellndx / ( m_iResolution * m_iResolution );
	int iy = ( cellndx % ( m_iResolution * m_iResolution ) ) / m_iResolution;
	int ix = cellndx % m_iResolution;

	vecMins.x = m_vecMinBounds.x + (float)ix * m_flCellSize;
	vecMins.y = m_vecMinBounds.y + (float)iy * m_flCellSize;
	vecMins.z = m_vecMinBounds.z + (float)iz * m_flCellSize;

	vecMaxs.x = vecMins.x + m_flCellSize;
	vecMaxs.y = vecMins.y + m_flCellSize;
	vecMaxs.z = vecMins.z + m_flCellSize;
};

//-----------------------------------------------------------------------------
// Rebuild grid
//-----------------------------------------------------------------------------

void CSmokeCloudGrid::Rebuild( const std::vector<CSmokeParticle> &particles, float flTime )
{
	PROF_AUTO_NAME();

	if ( flTime - m_flLastUpdateTime < 0.1 )
		return;

	m_flLastUpdateTime = flTime;

	m_CellParticlesData.assign( m_iTotalCells, CSmokeCloudParticlesData() );
	memset( m_CellCounts.data(), 0, m_iTotalCells * sizeof( int ) );
	memset( m_TempCellCounts.data(), 0, m_iTotalCells * sizeof( int ) );

	m_vecAvgPos.Zero();
	int iTotalAvgPositions = 0;

	auto GetCellAndSphereBounds = [ & ]( const CSmokeParticle &p, int &minX, int &maxX, int &minY, int &maxY, int &minZ, int &maxZ )
	{
		minX = Q_max( 0, (int)( ( p.m_vecPos.x - p.m_flRadius - m_vecMinBounds.x ) / m_flCellSize ) );
		maxX = Q_min( m_iResolution - 1, (int)( ( p.m_vecPos.x + p.m_flRadius - m_vecMinBounds.x ) / m_flCellSize ) );

		minY = Q_max( 0, (int)( ( p.m_vecPos.y - p.m_flRadius - m_vecMinBounds.y ) / m_flCellSize ) );
		maxY = Q_min( m_iResolution - 1, (int)( ( p.m_vecPos.y + p.m_flRadius - m_vecMinBounds.y ) / m_flCellSize ) );

		minZ = Q_max( 0, (int)( ( p.m_vecPos.z - p.m_flRadius - m_vecMinBounds.z ) / m_flCellSize ) );
		maxZ = Q_min( m_iResolution - 1, (int)( ( p.m_vecPos.z + p.m_flRadius - m_vecMinBounds.z ) / m_flCellSize ) );
	};

	// Check for any intersections
	int totalEntries = 0;
	for ( const auto &p : particles )
	{
		if ( !p.IsActive() )
			continue;

		int x0, x1, y0, y1, z0, z1;
		GetCellAndSphereBounds( p, x0, x1, y0, y1, z0, z1 );

		for ( int z = z0; z <= z1; z++ )
		{
			for ( int y = y0; y <= y1; y++ )
			{
				for ( int x = x0; x <= x1; x++ )
				{
					m_CellCounts[ x + y * m_iResolution + z * m_iResolution * m_iResolution ]++;
					totalEntries++;
				}
			}
		}
	}

	// Calc offsets
	m_CellOffsets[ 0 ] = 0;
	for ( int i = 1; i < m_iTotalCells; i++ )
	{
		m_CellOffsets[ i ] = m_CellOffsets[ i - 1 ] + m_CellCounts[ i - 1 ];
	}

	// Fill up particle indexes
	m_ParticleIndexes.resize( totalEntries );

	for ( int i = 0; i < (int)particles.size(); i++ )
	{
		const auto &p = particles[ i ];
		if ( !p.IsActive() )
			continue;

		int x0, x1, y0, y1, z0, z1;
		GetCellAndSphereBounds( p, x0, x1, y0, y1, z0, z1 );

		for ( int z = z0; z <= z1; z++ )
		{
			for ( int y = y0; y <= y1; y++ )
			{
				for ( int x = x0; x <= x1; x++ )
				{
					int cellIdx = x + y * m_iResolution + z * m_iResolution * m_iResolution;
					int writePos = m_CellOffsets[ cellIdx ] + m_TempCellCounts[ cellIdx ];
					m_ParticleIndexes[ writePos ] = i;
					m_TempCellCounts[ cellIdx ]++;
				}
			}
		}
	}

	// Agregate data
	for ( int i = 0; i < m_iTotalCells; i++ )
	{
		int num = m_CellCounts[ i ];
		if ( num == 0 )
			continue;

		int start = m_CellOffsets[ i ];
		Vector sumPos( 0, 0, 0 );
		float sumAlpha = 0;
		int activeInCell = 0;

		for ( int j = 0; j < num; j++ )
		{
			const CSmokeParticle &p = particles[ m_ParticleIndexes[ start + j ] ];
			if ( p.IsActive() )
			{
				sumPos += p.m_vecPos;
				sumAlpha += p.Transparency( flTime );
				activeInCell++;
			}
		}

		if ( activeInCell > 0 )
		{
			auto &data = m_CellParticlesData[ i ];
			data.m_vecAvgPos = sumPos / (float)activeInCell;
			data.m_flAvgAlpha = sumAlpha / (float)activeInCell;
			data.m_iCount = activeInCell;

			m_vecAvgPos += data.m_vecAvgPos;
			iTotalAvgPositions++;
		}
	}

	if ( iTotalAvgPositions > 0 )
		m_vecAvgPos /= (float)iTotalAvgPositions;
}

//-----------------------------------------------------------------------------
// GetRandomVector
//-----------------------------------------------------------------------------

Vector CSmokeSimulation::GetRandomVector( void )
{
	return Vector( Features::random->RandomFloat( -1.f, 1.f ),
				   Features::random->RandomFloat( -1.f, 1.f ),
				   Features::random->RandomFloat( -1.f, 1.f ) );
}

//-----------------------------------------------------------------------------
// LoadSprite
//-----------------------------------------------------------------------------

void CSmokeSimulation::LoadSprite( void )
{
	m_pSmokeTexture = NULL;

	HSPRITE_HL hSmokeSprite = cl_enginefuncs->pfnSPR_Load( "sprites/svenint/smoke.spr" );
	if ( hSmokeSprite > 0 )
	{
		m_pSmokeTexture = (model_s *)cl_enginefuncs->GetSpritePointer( hSmokeSprite );
		if ( m_pSmokeTexture != NULL )
			return;
	}

	PrintWarning( "Failed to load smoke sprite from path \"./%s\"\n", "sprites/svenint/smoke.spr" );
}

//-----------------------------------------------------------------------------
// OnExplosion event
//-----------------------------------------------------------------------------

void CSmokeSimulation::OnExplosion( const Vector &vecOrigin, float flRadius, float flForce )
{
	if ( flRadius < 100.f )
		return;

	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( const_cast<Vector &>( vecOrigin ),
											   vecOrigin - Vector( 0.f, 0.f, 100.f ),
											   PM_WORLD_ONLY,
											   -1,
											   &trace );

	TraceExplosion( trace.endpos, flRadius, flForce );
	OnFlash( trace.endpos, Vector( 1.f, 0.8f, 0.5f ), flRadius * 1.2f, 1.f );

	SpawnSmoke( trace.endpos, m_pCloudSize->GetFloat(), m_pParticlesSize->GetFloat() );
}

//-----------------------------------------------------------------------------
// OnPlayerShoot event
//-----------------------------------------------------------------------------

void CSmokeSimulation::OnPlayerShoot( int iWeaponID, const Vector &vecStart, const Vector &vecEnd, float flForce /* = 10.f */ )
{
	TraceBullet( iWeaponID, vecStart, vecEnd, flForce );

	const Vector vecFlashColor = ( iWeaponID == WEAPON_EGON || iWeaponID == WEAPON_SHOCK_RIFLE ) ? Vector( 0.f, 0.f, 1.f ) : Vector( 1.f, 0.8f, 0.5f );
	OnFlash( vecStart, vecFlashColor, 80.f, 1.f );
}

//-----------------------------------------------------------------------------
// OnFlash event
//-----------------------------------------------------------------------------

void CSmokeSimulation::OnFlash( const Vector &vecOrigin, const Vector &vecColor, float flRadius, float flIntensity )
{
	m_SmokeFlashes.push_back( { vecOrigin, vecColor, flRadius, flIntensity, cl_enginefuncs->GetClientTime() + 0.1f } );
}

//-----------------------------------------------------------------------------
// SpawnSmoke
//-----------------------------------------------------------------------------

void CSmokeSimulation::SpawnSmoke( const Vector &vecOrigin, float flRadius /* = 128.f */, float flParticleSize /* = 16.f */ )
{
	if ( (int)m_SmokeClouds.size() >= m_pLimit->GetInt() )
		return;

	const float flTime = cl_enginefuncs->GetClientTime();
	const float flCloudSize = flRadius * 3.f;
	const float flNumParticlesPerRow = 2.f * ( flRadius / flParticleSize );
	const uint32_t iNumParticlesInSphere = uint32_t( flNumParticlesPerRow * flNumParticlesPerRow * flNumParticlesPerRow * 0.65f );
	const Vector vecCloudSize( flCloudSize, flCloudSize, flCloudSize );

	m_SmokeClouds.push_back( CSmokeCloud() );
	CSmokeCloud &smokeCloud = m_SmokeClouds.back();

	smokeCloud.Init( vecOrigin - vecCloudSize, vecOrigin + vecCloudSize, m_pSpacePartitionGrid->GetInt(), cl_enginefuncs->GetClientTime(), true );

	for ( uint32_t i = 0; i < iNumParticlesInSphere; i++ )
	{
		smokeCloud.GetParticles().push_back( CSmokeParticle() );
		CSmokeParticle &smokeParticle = smokeCloud.GetParticles().back();

		const Vector vecRandomOffset = ( GetRandomVector() + Vector( 0.f, 0.f, 0.9f ) ).Normalize() * Features::random->RandomFloat( 0.f, 1.f );

		const Vector vecPos = vecOrigin + vecRandomOffset * flRadius * 0.35f;
		const Vector vecVelocity = vecRandomOffset * 15.f;

		const float flRadius = flParticleSize * Features::random->RandomFloat( 0.6f, 2.5f );
		const float flSpawnTime = flTime;
		const float flLifeTime = 30.f + Features::random->RandomFloat( 0.f, 10.f ) - vecRandomOffset.Length() * 10.f;
		const float flFadeTime = 5.f;

		smokeParticle.Init( kSmokeParticleForming, vecPos, vecVelocity, flRadius, flSpawnTime, flLifeTime, flFadeTime );
	}

	extern ConVar sc_prof;
	const bool bSavedValue = sc_prof.GetBool();
	sc_prof.SetValue( false );

	smokeCloud.GetGrid().Rebuild( smokeCloud.GetParticles(), smokeCloud.GetLastUpdateTime() );

	sc_prof.SetValue( bSavedValue );
}

//-----------------------------------------------------------------------------
// Trace a bullet
//-----------------------------------------------------------------------------

void CSmokeSimulation::TraceBullet( int iWeaponID, const Vector &p1, const Vector &p2, float flForce /* = 10.f */ )
{
	PROF_AUTO_NAME();

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];
		if ( !smokeCloud.IsActive() )
			continue;

		smokeCloud.TraceBullet( iWeaponID, p1, p2, flForce );
	}
}

//-----------------------------------------------------------------------------
// Trace a bullet
//-----------------------------------------------------------------------------

void CSmokeSimulation::TraceExplosion( const Vector &vecPos, float flRadius, float flForce )
{
	PROF_AUTO_NAME();

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];
		if ( !smokeCloud.IsActive() )
			continue;

		// Spawn protection
		if ( smokeCloud.GetParticles().empty() || m_flTime - smokeCloud.GetParticles().back().m_flSpawnTime < 0.5f )
			continue;

		smokeCloud.TraceExplosion( vecPos, flRadius, flForce );
	}
}

//-----------------------------------------------------------------------------
// TraceBodyMove
//-----------------------------------------------------------------------------

void CSmokeSimulation::TraceBodyMove( const Vector &vecBodyMins, const Vector &vecBodyMaxs, const Vector &vecVelocity )
{
	PROF_AUTO_NAME();

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];
		if ( !smokeCloud.IsActive() )
			continue;

		smokeCloud.TraceBodyMove( vecBodyMins, vecBodyMaxs, vecVelocity );
	}
}

//-----------------------------------------------------------------------------
// Reset
//-----------------------------------------------------------------------------

void CSmokeSimulation::Reset( void )
{
	if ( cls->state >= ca_connecting )
		LoadSprite();

	m_flTime = cl_enginefuncs->GetClientTime();
	m_flLastUpdateTime = m_flTime;
	m_flLastTraceBodyMoveTime = m_flTime;
	m_flAlphaModifier = 1.f;

	m_SmokeClouds.clear();
	m_SmokeFlashes.clear();
}

//-----------------------------------------------------------------------------
// Update smoke
//-----------------------------------------------------------------------------

void CSmokeSimulation::Update( void )
{
	PROF_AUTO_NAME();

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];
		if ( !smokeCloud.IsActive() )
			continue;

		const float flFrametime = m_flTime - smokeCloud.GetLastUpdateTime();
		const float flTimeDiff = flFrametime;

		smokeCloud.CheckPVS();

		// Lazily update clouds that are not in PVS
		if ( !smokeCloud.IsInPVS() && flTimeDiff < 0.4 )
			continue;

		// Lazily update clouds that are far away from us
		if ( smokeCloud.m_LevelOfDetail >= kSmokeCloudLOD_UltraLow && flTimeDiff < 0.25 )
			continue;
		if ( smokeCloud.m_LevelOfDetail >= kSmokeCloudLOD_Low && flTimeDiff < 0.1 )
			continue;

		smokeCloud.m_flLastUpdateTime = m_flTime;

		bool bCloudIsActive = false;

		for ( size_t j = 0; j < smokeCloud.GetParticles().size(); j++ )
		{
			CSmokeParticle &smokeParticle = smokeCloud.GetParticles()[ j ];
			if ( !smokeParticle.IsActive() )
				continue;

			bCloudIsActive = true;
			smokeParticle.Update( m_flTime, flFrametime );
		}

		if ( !bCloudIsActive )
		{
			m_SmokeClouds.erase( m_SmokeClouds.begin() + i );
			i--;
			continue;
		}

		if ( !smokeCloud.IsInPVS() )
			continue;

		smokeCloud.GetGrid().Rebuild( smokeCloud.GetParticles(), m_flTime );
	}
}

//-----------------------------------------------------------------------------
// Render smoke
//-----------------------------------------------------------------------------

void CSmokeSimulation::Render( void )
{
	PROF_AUTO_NAME();

	if ( m_pSmokeTexture == NULL )
		return;

	if ( m_SmokeClouds.empty() || m_pVizualize->GetBool() )
		return;

	QAngle va;
	Vector vecEyes = localplayer->GetEyePosition();

	cl_enginefuncs->GetViewAngles( va );
	AngleVectors( va, NULL, &m_vecRight, &m_vecUp );

	cl_enginefuncs->pTriAPI->SpriteTexture( (model_s *)m_pSmokeTexture, 0 );
	cl_enginefuncs->pTriAPI->RenderMode( kRenderTransAlpha );
	cl_enginefuncs->pTriAPI->CullFace( TRI_NONE );

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];
		if ( !smokeCloud.m_bActive || !smokeCloud.IsInPVS() )
			continue;

		const float flDistance = ( smokeCloud.GetGrid().m_vecAvgPos - vecEyes ).Length();
		RenderParticles_LOD( smokeCloud, flDistance );
	}

	cl_enginefuncs->pTriAPI->RenderMode( kRenderNormal );
	cl_enginefuncs->pTriAPI->CullFace( TRI_FRONT );
}

//-----------------------------------------------------------------------------
// CalculateDynamicLighting
//-----------------------------------------------------------------------------

Vector CSmokeSimulation::CalculateDynamicLighting( const Vector &vecPos, const Vector &vecBaseColor )
{
	Vector vecFinalColor = vecBaseColor;

	for ( size_t i = 0; i < m_SmokeFlashes.size(); i++ )
	{
		CSmokeFlash &smokeFlash = m_SmokeFlashes[ i ];
		if ( m_flTime > smokeFlash.m_flDieTime )
		{
			m_SmokeFlashes.erase( m_SmokeFlashes.begin() + i );
			i--;
			continue;
		}

		Vector dir = smokeFlash.m_vecPos - vecPos;
		float distSq = dir.LengthSqr();

		const float flMaxDistSq = smokeFlash.m_flRadius * smokeFlash.m_flRadius;

		if ( distSq < flMaxDistSq )
		{
			float dist = sqrtf( distSq );
			float falloff = 1.f - ( dist / smokeFlash.m_flRadius );

			vecFinalColor += smokeFlash.m_vecColor * ( smokeFlash.m_flIntensity * falloff );
		}
	}

	vecFinalColor.x = Q_min( 1.f, vecFinalColor.x );
	vecFinalColor.y = Q_min( 1.f, vecFinalColor.y );
	vecFinalColor.z = Q_min( 1.f, vecFinalColor.z );

	return vecFinalColor;
}

//-----------------------------------------------------------------------------
// Render a single particle
//-----------------------------------------------------------------------------

void CSmokeSimulation::RenderParticle( const Vector &vecPos, float flRadius, float flAlpha, float flLightLevel /* = 0.75f */ )
{
	const Vector width = m_vecRight * flRadius;
	const Vector height = m_vecUp * flRadius;

	const Vector lowLeft = vecPos - ( width * 0.5f ) - ( m_vecUp * flRadius * 0.5f );

	const Vector lowRight = lowLeft + width;
	const Vector topLeft = lowLeft + height;
	const Vector topRight = lowRight + height;

	const Vector vecBaseColor = Vector( 1.f, 1.f, 1.f ) * flLightLevel;
	const Vector vecFlashColor( 1.f, 0.8f, 0.5f );

	Vector vecEnvironmentLight, vecFinalColor;

	if ( m_pRenderedSmokeParticle != NULL && m_pRenderedSmokeParticle->GetState() == kSmokeParticleShot &&
		 !m_pRenderedSmokeParticle->m_MetaData_ShotByEgonOrShockRifle && m_pIlluminateSmokeByBullets->GetBool() )
	{
		vecFinalColor = vecBaseColor + ( vecFlashColor - vecBaseColor ) * m_pRenderedSmokeParticle->m_flFadeTime * m_pRenderedSmokeParticle->m_flFadeTime;
	}
	else
	{
		vecFinalColor = vecBaseColor;
	}

	if ( m_pDynamicLighting->GetBool() )
		vecFinalColor = CalculateDynamicLighting( vecPos, vecFinalColor );

	cl_enginefuncs->pTriAPI->LightAtPoint( const_cast<Vector &>( vecPos ), vecEnvironmentLight );
	vecEnvironmentLight = vecEnvironmentLight / 255.f;

	cl_enginefuncs->pTriAPI->Begin( TRI_QUADS );

		cl_enginefuncs->pTriAPI->Color4f( vecFinalColor.x * vecEnvironmentLight.x,
										  vecFinalColor.y * vecEnvironmentLight.y,
										  vecFinalColor.z * vecEnvironmentLight.z,
										  flAlpha * m_flAlphaModifier );

		cl_enginefuncs->pTriAPI->TexCoord2f( 0, 0 );
		cl_enginefuncs->pTriAPI->Vertex3fv( topLeft );

		cl_enginefuncs->pTriAPI->TexCoord2f( 0, 1 );
		cl_enginefuncs->pTriAPI->Vertex3fv( lowLeft );

		cl_enginefuncs->pTriAPI->TexCoord2f( 1, 1 );
		cl_enginefuncs->pTriAPI->Vertex3fv( lowRight );

		cl_enginefuncs->pTriAPI->TexCoord2f( 1, 0 );
		cl_enginefuncs->pTriAPI->Vertex3fv( topRight );

	cl_enginefuncs->pTriAPI->End();
}

//-----------------------------------------------------------------------------
// RenderParticles_LOD
//-----------------------------------------------------------------------------

void CSmokeSimulation::RenderParticles_LOD( CSmokeCloud &smokeCloud, float flDistance )
{
	auto GetFadeFalloff = [ & ]( float dist, float edge )
	{
		return Q_clamp( ( dist - ( edge - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE ) ) / SMOKE_CLOUD_LOD_TRANSITION_DISTANCE, 0.f, 1.f );
	};

	if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_HIGH - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE )
	{
		RenderParticles_Normal( smokeCloud );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_High;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_HIGH )
	{
		float t = GetFadeFalloff( flDistance, SMOKE_CLOUD_LOD_DISTANCE_HIGH );
		m_flAlphaModifier = 1.f - t;
		RenderParticles_Normal( smokeCloud );

		m_flAlphaModifier = t;
		RenderParticles_Cluster( smokeCloud, 1 );

		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_High;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_MEDIUM - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE )
	{
		m_flAlphaModifier = 1.f;
		RenderParticles_Cluster( smokeCloud, 1 );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_Medium;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_MEDIUM )
	{
		float t = GetFadeFalloff( flDistance, SMOKE_CLOUD_LOD_DISTANCE_MEDIUM );
		m_flAlphaModifier = 1.f - t;
		RenderParticles_Cluster( smokeCloud, 1 );

		m_flAlphaModifier = t;
		RenderParticles_Cluster( smokeCloud, 2 );

		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_Medium;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_LOW - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE )
	{
		m_flAlphaModifier = 1.f;
		RenderParticles_Cluster( smokeCloud, 2 );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_Low;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_LOW )
	{
		float t = GetFadeFalloff( flDistance, SMOKE_CLOUD_LOD_DISTANCE_LOW );
		m_flAlphaModifier = 1.f - t;
		RenderParticles_Cluster( smokeCloud, 2 );

		m_flAlphaModifier = t;
		RenderParticles_Cluster( smokeCloud, 3 );

		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_Low;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_VERYLOW - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE )
	{
		m_flAlphaModifier = 1.f;
		RenderParticles_Cluster( smokeCloud, 3 );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_VeryLow;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_VERYLOW )
	{
		float t = GetFadeFalloff( flDistance, SMOKE_CLOUD_LOD_DISTANCE_VERYLOW );
		m_flAlphaModifier = 1.f - t;
		RenderParticles_Cluster( smokeCloud, 3 );

		m_flAlphaModifier = t;
		RenderParticles_Cluster( smokeCloud, 4 );

		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_VeryLow;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_ULTRALOW - SMOKE_CLOUD_LOD_TRANSITION_DISTANCE )
	{
		m_flAlphaModifier = 1.f;
		RenderParticles_Cluster( smokeCloud, 4 );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_UltraLow;
	}
	else if ( flDistance < SMOKE_CLOUD_LOD_DISTANCE_ULTRALOW )
	{
		float t = GetFadeFalloff( flDistance, SMOKE_CLOUD_LOD_DISTANCE_ULTRALOW );
		m_flAlphaModifier = 1.f - t;
		RenderParticles_Cluster( smokeCloud, 4 );

		m_flAlphaModifier = t;
		RenderParticles_Potato( smokeCloud );

		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_UltraLow;
	}
	else
	{
		RenderParticles_Potato( smokeCloud );
		smokeCloud.m_LevelOfDetail = kSmokeCloudLOD_UltraLow;
	}

	m_flAlphaModifier = 1.f;
}

//-----------------------------------------------------------------------------
// RenderParticles_Normal
//-----------------------------------------------------------------------------

void CSmokeSimulation::RenderParticles_Normal( CSmokeCloud &smokeCloud )
{
	for ( size_t i = 0; i < smokeCloud.GetParticles().size(); i++ )
	{
		CSmokeParticle &smokeParticle = smokeCloud.GetParticles()[ i ];
		if ( !smokeParticle.IsActive() )
			continue;

		m_pRenderedSmokeParticle = &smokeParticle;

		RenderParticle( smokeParticle.m_vecPos,
						smokeParticle.m_flRadius,
						smokeParticle.Transparency( m_flTime ) );

		m_pRenderedSmokeParticle = NULL;
	}
}

//-----------------------------------------------------------------------------
// RenderParticles_Cluster
//-----------------------------------------------------------------------------

void CSmokeSimulation::RenderParticles_Cluster( CSmokeCloud &smokeCloud, int iClusterSize )
{
	CSmokeCloudGrid &grid = smokeCloud.GetGrid();
	const int iRes = grid.m_iResolution;
	float renderRadius = grid.m_flCellSize * (float)iClusterSize * 1.4f;

	for ( int z = 0; z < iRes; z += iClusterSize )
	{
		for ( int y = 0; y < iRes; y += iClusterSize )
		{
			for ( int x = 0; x < iRes; x += iClusterSize )
			{
				Vector clusterPos( 0, 0, 0 );
				float clusterAlpha = 0;
				int cellsWithData = 0;

				for ( int sz = 0; sz < iClusterSize && ( z + sz ) < iRes; sz++ )
				{
					for ( int sy = 0; sy < iClusterSize && ( y + sy ) < iRes; sy++ )
					{
						for ( int sx = 0; sx < iClusterSize && ( x + sx ) < iRes; sx++ )
						{
							int ndx = ( x + sx ) + ( y + sy ) * iRes + ( z + sz ) * iRes * iRes;
							if ( grid.m_CellParticlesData[ ndx ].m_iCount > 0 )
							{
								clusterPos += grid.m_CellParticlesData[ ndx ].m_vecAvgPos;
								clusterAlpha += grid.m_CellParticlesData[ ndx ].m_flAvgAlpha;
								cellsWithData++;
							}
						}
					}
				}

				if ( cellsWithData > 0 )
				{
					float avgAlpha = clusterAlpha / (float)cellsWithData;
					float clusterAlphaBoost = 1.0f + ( (float)iClusterSize - 1.0f ) * 0.9f;
					float finalAlpha = Q_min( avgAlpha * clusterAlphaBoost, 1.0f );

					// Give it more volume depending on finalAlpha
					float dynamicRadius = renderRadius * ( 0.8f + finalAlpha * 0.4f );

					RenderParticle( clusterPos / (float)cellsWithData, dynamicRadius, finalAlpha );
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// RenderParticles_Potato
//-----------------------------------------------------------------------------

void CSmokeSimulation::RenderParticles_Potato( CSmokeCloud &smokeCloud )
{
	Vector vecCenter = smokeCloud.GetGrid().m_vecAvgPos;
	float flTotalAlpha = 0;
	int iActiveCount = 0;

	for ( auto &p : smokeCloud.GetParticles() )
	{
		if ( p.IsActive() )
		{
			flTotalAlpha += p.Transparency( m_flTime );
			iActiveCount++;
		}
	}

	if ( iActiveCount > 0 )
	{
		float finalRadius = ( smokeCloud.GetMaxs() - smokeCloud.GetMins() ).Length() * 0.5f;
		RenderParticle( vecCenter, finalRadius, Q_clamp( ( flTotalAlpha / (float)iActiveCount ) * 2.f, 0.f, 1.f ) );
	}
}

//-----------------------------------------------------------------------------
// DebugDraw
//-----------------------------------------------------------------------------

void CSmokeSimulation::DebugDraw( void )
{
	if ( !m_pVizualize->GetBool() )
		return;

	for ( size_t i = 0; i < m_SmokeClouds.size(); i++ )
	{
		CSmokeCloud &smokeCloud = m_SmokeClouds[ i ];

		if ( cl_enginefuncs->pTriAPI->BoxInPVS( smokeCloud.GetMins(), smokeCloud.GetMaxs() ) == 0 )
			continue;

		CWireframeBox *pCloudDrawContext = new CWireframeBox( smokeCloud.GetMins(), Vector(), smokeCloud.GetMaxs() - smokeCloud.GetMins(), Color( 232, 232, 0, 232 ), 2.f, false );
		Features::drawcontext->AddDrawContext( pCloudDrawContext );

		for ( int i = 0; i < smokeCloud.GetGrid().m_iTotalCells; i++ )
		{
			// Does have one particle at least
			if ( smokeCloud.GetGrid().m_CellCounts[ i ] > 0 )
			{
				Vector vecMins, vecMaxs;
				smokeCloud.GetGrid().GetCellBounds( i, vecMins, vecMaxs );

				Features::drawcontext->DrawBox( vecMins, Vector(), vecMaxs - vecMins, Color( 50, 232, 50, 30 ) );
			}
		}

		for ( size_t j = 0; j < smokeCloud.GetParticles().size(); j++ )
		{
			CSmokeParticle &smokeParticle = smokeCloud.GetParticles()[ j ];
			if ( !smokeParticle.IsActive() )
				continue;

			const float flSize = smokeParticle.m_flRadius / 8.f;
			Vector vecSize( flSize, flSize, flSize );

			//CWireframeBox *pParticleDrawContext = new CWireframeBox( smokeParticle.m_vecPos, -vecSize, vecSize, Color( 0, 127, 232, 232 ), 1.f, false );
			//Features::drawcontext->AddDrawContext( pParticleDrawContext );

			Features::drawcontext->DrawBox( smokeParticle.m_vecPos, -vecSize, vecSize, Color( 0, 127, 232, 60 ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSmokeSimulation::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	auto IsShootableWeapon = []( int iWeaponID ) -> bool
	{
		switch ( iWeaponID )
		{
		case WEAPON_CROWBAR:
		case WEAPON_CHAINGUN:
		case WEAPON_CROSSBOW:
		case WEAPON_RPG:
		case WEAPON_HORNETGUN:
		case WEAPON_HANDGRENADE:
		case WEAPON_TRIPMINE:
		case WEAPON_SATCHEL:
		case WEAPON_SNARK:
		case WEAPON_MEDKIT:
		case WEAPON_CROWBAR_ELECTRIC:
		case WEAPON_WRENCH:
		case WEAPON_MINIGUN:
		case WEAPON_BARNACLE_GRAPPLE:
		case WEAPON_SPORE_LAUNCHER:
		case WEAPON_DISPLACER:
			return false;
		}

		return true;
	};

	auto CanSecondaryAttackFromThisWeapon = []( int iWeaponID ) -> bool
	{
		switch ( iWeaponID )
		{
		case WEAPON_PYTHON:
		case WEAPON_MP5:
		case WEAPON_GAUSS:
		case WEAPON_EGON:
		case WEAPON_UZI:
		case WEAPON_MINIGUN:
		case WEAPON_SNIPER_RIFLE:
		case WEAPON_M249:
		case WEAPON_M16:
		case WEAPON_DESERT_EAGLE:
			return false;
		}

		return true;
	};

	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		Reset();
		return kHookContinue;
	}
	else if ( pEvent->GetType() == kHUD_AddEntity_HookEvent )
	{
		auto type = pEvent->GetArg<int>( "type" );
		auto ent = pEvent->GetArg<cl_entity_t *>( "ent" );

		if ( type != ET_PLAYER || ent->index == localplayer->GetPlayerIndex() )
			return kHookContinue;

		if ( ent->curstate.effects & EF_MUZZLEFLASH )
		{
			Vector va;
			Vector vecEyes = ent->origin + Vector( 0.f, 0.f, ent->curstate.usehull ? 12.5f : 28.5f /* VEC_DUCK_VIEW.z : VEC_VIEW.z */ );

			gameutils->GetViewAngles( ent, va );

			pmtrace_t trace;

			cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
			cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
													   vecEyes + static_cast<QAngle>( va ).GetForward() * 8192,
													   PM_NORMAL,
													   -1,
													   &trace );

			OnPlayerShoot( 0, vecEyes, trace.endpos );
		}
		else if ( cl_enginefuncs->GetClientTime() - m_flLastTraceBodyMoveTime >= 0.07f )
		{
			CEntity *pEnts = Features::entitylist->GetList();
			CEntity *pEntity = pEnts + ent->index;

			if ( !pEntity->m_bValid || pEntity->m_vecVelocity.IsZeroFast() )
				return kHookContinue;

			TraceBodyMove( pEntity->m_vecOrigin + pEntity->m_vecMins, pEntity->m_vecOrigin + pEntity->m_vecMaxs, pEntity->m_vecVelocity );
		}

		return kHookContinue;
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

		if ( localplayer->IsDead() )
			return kHookContinue;

		if ( cl_enginefuncs->GetClientTime() - m_flLastTraceBodyMoveTime >= 0.07f )
		{
			CEntity *pEnts = Features::entitylist->GetList();
			CEntity *pLocal = pEnts + localplayer->GetPlayerIndex();

			if ( pLocal->m_bValid && !playermove->velocity()->IsZeroFast() )
				TraceBodyMove( pLocal->m_vecOrigin + pLocal->m_vecMins, pLocal->m_vecOrigin + pLocal->m_vecMaxs, *playermove->velocity() * playermove->frametime() );
		}

		if ( !localplayer->HasWeapon() )
			return kHookContinue;

		if ( !localplayer->CanAttack() )
			return kHookContinue;

		if ( clientweapon->IsReloading() )
			return kHookContinue;

		int iWeaponID = localplayer->GetCurrentWeaponID();
		if ( !IsShootableWeapon( iWeaponID ) )
			return kHookContinue;

		bool bFired = false;
		bool bCustomWeapon = clientweapon->IsCustom();

		if ( ( cmd->buttons & IN_ATTACK ) && ( bCustomWeapon || ( !bCustomWeapon && clientweapon->CanPrimaryAttack() ) ) )
		{
			bFired = true;
		}
		else if ( ( cmd->buttons & IN_ATTACK2 ) && ( bCustomWeapon || ( !bCustomWeapon && clientweapon->CanSecondaryAttack() ) ) )
		{
			if ( !CanSecondaryAttackFromThisWeapon( iWeaponID ) )
				return kHookContinue;

			bFired = true;
		}

		if ( bFired )
		{
			pmtrace_t trace;
			Vector vecEyes = localplayer->GetEyePosition();

			cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
			cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
													   vecEyes + static_cast<QAngle>( cmd->viewangles ).GetForward() * 8192,
													   PM_NORMAL,
													   -1,
													   &trace );

			OnPlayerShoot( iWeaponID, vecEyes, trace.endpos );
		}

		return kHookContinue;
	}

	// HUD_DrawTransparentTriangles event
	PROF_AUTO_NAME();

	m_flLastUpdateTime = m_flTime;
	m_flTime = cl_enginefuncs->GetClientTime();

	if ( m_flTime - m_flLastTraceBodyMoveTime >= 0.1f )
		m_flLastTraceBodyMoveTime = m_flTime;

	const float flFrametime = m_flTime - m_flLastUpdateTime;
	if ( flFrametime <= 0.f )
		return kHookContinue;

	m_flFrametime = flFrametime;

	Update();
	Render();
	DebugDraw();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSmokeSimulation::CSmokeSimulation( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pLimit = NULL;
	m_pCloudSize = NULL;
	m_pParticlesSize = NULL;
	m_pSpacePartitionGrid = NULL;
	m_pIlluminateSmokeByBullets = NULL;
	m_pDynamicLighting = NULL;
	m_pVizualize = NULL;

	m_pRenderedSmokeParticle = NULL;
	m_pSmokeTexture = NULL;

	m_flTime = 0.f;
	m_flLastUpdateTime = 0.f;
	m_flLastTraceBodyMoveTime = 0.f;
	m_flFrametime = 0.f;
	m_flAlphaModifier = 0.f;

	m_hNetMsgHook_TempEntity = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSmokeSimulation::OnEnable( void )
{
	Reset();

	hookevents->RegisterListener( this, kHUD_DrawTransparentTriangles_HookEvent );
	hookevents->RegisterListener( this, kHUD_AddEntity_HookEvent );
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSmokeSimulation::OnDisable( void )
{
	Reset();

	hookevents->UnregisterListener( this, kHUD_DrawTransparentTriangles_HookEvent );
	hookevents->UnregisterListener( this, kHUD_AddEntity_HookEvent );
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSmokeSimulation::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pIlluminateSmokeByBullets = Modules::menu->AddParamBool( this, "IlluminateSmokeByBullets", NULL, true );
	m_pDynamicLighting = Modules::menu->AddParamBool( this, "DynamicLighting", NULL, true );
	m_pVizualize = Modules::menu->AddParamBool( this, "VizualizeSmoke", NULL, false );
	m_pLimit = Modules::menu->AddParamInteger( this, "Limit", NULL, 5, 0, 20 );
	m_pSpacePartitionGrid = Modules::menu->AddParamInteger( this, "SpacePartitionGrid", NULL, 16, 2, 64 );
	m_pCloudSize = Modules::menu->AddParamFloat( this, "CloudSize", NULL, 128.f, 16.f, 256.f );
	m_pParticlesSize = Modules::menu->AddParamFloat( this, "ParticlesSize", NULL, 16.f, 1.f, 64.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSmokeSimulation::PostLoad( void )
{
#ifdef DEBUG
	FEATURE_REGISTER_CCMD( sc_smoke );
	FEATURE_REGISTER_CCMD( sc_smoke_trace_bullet );
	FEATURE_REGISTER_CCMD( sc_smoke_trace_boom );
	FEATURE_REGISTER_CCMD( sc_smoke_flash );
#endif

	m_hNetMsgHook_TempEntity = gamehooks->HookNetworkMessage( SVC_TEMPENTITY, HOOKED_NetMsgHook_TempEntity, &ORIG_NetMsgHook_TempEntity );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSmokeSimulation::Unload( void )
{
#ifdef DEBUG
	FEATURE_UNREGISTER_CCMD( sc_smoke );
	FEATURE_UNREGISTER_CCMD( sc_smoke_trace_bullet );
	FEATURE_UNREGISTER_CCMD( sc_smoke_trace_boom );
	FEATURE_UNREGISTER_CCMD( sc_smoke_flash );
#endif

	gamehooks->UnhookNetworkMessage( m_hNetMsgHook_TempEntity );
}

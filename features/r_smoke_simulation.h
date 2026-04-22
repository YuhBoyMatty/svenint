// SvenInt (c) Sw1ft
// Smoke simulation logic & rendering code by Admer456
// https://github.com/Admer456/halflife-smoke
// r_smoke_simulation.h

#ifndef SINT_FEATURE_SMOKE_SIMULATION_H
#define SINT_FEATURE_SMOKE_SIMULATION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define SMOKE_CLOUD_LOD_TRANSITION_DISTANCE ( 120.f )

#define SMOKE_CLOUD_LOD_DISTANCE_HIGH		( 500.f )
#define SMOKE_CLOUD_LOD_DISTANCE_MEDIUM		( 700.f )
#define SMOKE_CLOUD_LOD_DISTANCE_LOW		( 1000.f )
#define SMOKE_CLOUD_LOD_DISTANCE_VERYLOW	( 1300.f )
#define SMOKE_CLOUD_LOD_DISTANCE_ULTRALOW	( 2000.f )

//-----------------------------------------------------------------------------
// ESmokeCloudLOD
//-----------------------------------------------------------------------------

typedef enum
{
	kSmokeCloudLOD_High = 0,
	kSmokeCloudLOD_Medium,
	kSmokeCloudLOD_Low,
	kSmokeCloudLOD_VeryLow,
	kSmokeCloudLOD_UltraLow,
} ESmokeCloudLOD;

//-----------------------------------------------------------------------------
// ESmokeParticleState
//-----------------------------------------------------------------------------

typedef enum
{
	kSmokeParticleInactive = 0,
	kSmokeParticleForming,
	kSmokeParticleSpreading,
	kSmokeParticleDissipating,
	kSmokeParticleShot,
} ESmokeParticleState;

//-----------------------------------------------------------------------------
// CSmokeParticle
//-----------------------------------------------------------------------------

class CSmokeParticle
{
public:
	CSmokeParticle() : m_flRadius( 16.f ), m_flSpawnTime( 0.f ), m_flLifeTime( 0.f ), m_flFadeTime( 0.f ), m_state( kSmokeParticleInactive )
	{
		m_MetaData_ShotByEgonOrShockRifle = 0;
	}

	inline void Init( ESmokeParticleState state, const Vector &vecPos, const Vector &vecVelocity, float flRadius, float flSpawnTime, float flLifeTime, float flFadeTime )
	{
		m_state = state;

		m_vecPos = vecPos;
		m_vecVelocity = vecVelocity;

		m_flRadius = flRadius;

		m_flSpawnTime = flSpawnTime;
		m_flLifeTime = flLifeTime;
		m_flFadeTime = flFadeTime;
	}

	void Update( const float time, const float frametime );

public:
	inline bool IsActive( void ) const { return m_state != kSmokeParticleInactive; }
	inline ESmokeParticleState GetState( void ) const { return (ESmokeParticleState)m_state; }

	inline float Transparency( const float flTime ) const
	{
		switch ( m_state )
		{
		case kSmokeParticleInactive:
			return 0.f;

		case kSmokeParticleForming:
			return ( flTime - m_flSpawnTime ) / m_flFadeTime;

		case kSmokeParticleDissipating:
		{
			const float flTimeOfDeath = m_flSpawnTime + m_flLifeTime;
			return ( flTimeOfDeath - flTime ) / m_flFadeTime;
		}

		case kSmokeParticleShot:
		#ifdef DEBUG
			Assert( m_flFadeTime <= 1.f );
		#endif
			return m_flFadeTime * m_flFadeTime;
		}

		return 1.0f;
	}

public:
	Vector m_vecPos;
	Vector m_vecVelocity;

	float m_flRadius;

	float m_flSpawnTime;
	float m_flLifeTime;
	float m_flFadeTime;

	unsigned short m_state;
	unsigned short m_MetaData_ShotByEgonOrShockRifle : 1;
};

//-----------------------------------------------------------------------------
// CSmokeCloudParticlesData
//-----------------------------------------------------------------------------

class CSmokeCloudParticlesData
{
public:
	CSmokeCloudParticlesData() : m_flAvgAlpha( 0.f ), m_iCount( 0 )
	{
	}

	inline void Reset( void )
	{
		m_vecAvgPos.Zero();
		m_flAvgAlpha = 0.f;
		m_iCount = 0;
	}

public:
	Vector m_vecAvgPos;
	float m_flAvgAlpha;
	int m_iCount;
};

//-----------------------------------------------------------------------------
// CSmokeCloudGrid
//-----------------------------------------------------------------------------

class CSmokeCloudGrid
{
public:
	CSmokeCloudGrid() : m_iResolution( 0 ), m_iTotalCells( 0 ), m_flCellSize( 0.f ), m_flLastUpdateTime( 0.f )
	{
	}

	inline void Init( int iResolution, const Vector &vecMins, const Vector &vecMaxs )
	{
		m_iResolution = iResolution;
		m_iTotalCells = iResolution * iResolution * iResolution;
		m_flCellSize = ( vecMaxs.x - vecMins.x ) / (float)m_iResolution;
		m_flLastUpdateTime = -1.f;
		m_vecMinBounds = vecMins;

		m_CellCounts.assign( m_iTotalCells, 0 );
		m_CellOffsets.assign( m_iTotalCells, 0 );
		m_TempCellCounts.assign( m_iTotalCells, 0 );
	}

	void Rebuild( const std::vector<CSmokeParticle> &particles, float flTime );
	void GetCellBounds( int cellndx, Vector &vecMins, Vector &vecMaxs ) const;

public:
	int m_iResolution; // Grid resolution ( NxNxN )
	int m_iTotalCells;
	float m_flCellSize;
	float m_flLastUpdateTime;
	Vector m_vecMinBounds;
	Vector m_vecAvgPos;

	std::vector<int> m_CellCounts;      // Amount of particles in each cell
	std::vector<int> m_CellOffsets;     // Offset to each cell in m_ParticleIndexes
	std::vector<int> m_ParticleIndexes;
	std::vector<int> m_TempCellCounts;
	std::vector<CSmokeCloudParticlesData> m_CellParticlesData; // Averaged particles data for each cell
};

//-----------------------------------------------------------------------------
// CSmokeFlash
//-----------------------------------------------------------------------------

class CSmokeFlash
{
public:
	Vector m_vecPos;
	Vector m_vecColor;
	float  m_flRadius;
	float  m_flIntensity;
	float  m_flDieTime;
};

//-----------------------------------------------------------------------------
// CSmokeCloud
//-----------------------------------------------------------------------------

class CSmokeCloud
{
public:
	CSmokeCloud() : m_flLastUpdateTime( -1.f ), m_bActive( false ), m_bInPVS( false ), m_LevelOfDetail( kSmokeCloudLOD_High )
	{
	}

	inline void Init( const Vector &vecMins, const Vector &vecMaxs, int iGridSize, float flCurTime, bool bActive )
	{
		m_vecMins = vecMins;
		m_vecMaxs = vecMaxs;

		m_flLastUpdateTime = flCurTime;
		m_bActive = bActive;

		m_Grid.Init( iGridSize, vecMins, vecMaxs );
	}

	bool CheckPVS( void );

	void TraceBullet( int iWeaponID, const Vector &p1, const Vector &p2, float flForce );
	void TraceExplosion( const Vector &vecPos, float flRadius, float flForce );
	void TraceBodyMove( const Vector &vecBodyMins, const Vector &vecBodyMaxs, const Vector &vecMove );

public:
	inline Vector &GetMins( void ) { return m_vecMins; }
	inline Vector &GetMaxs( void ) { return m_vecMaxs; }

	inline float GetLastUpdateTime( void ) const { return m_flLastUpdateTime; }
	inline ESmokeCloudLOD GetLOD( void ) const { return m_LevelOfDetail; }

	inline std::vector<CSmokeParticle> &GetParticles( void ) { return m_SmokeParticles; }
	inline CSmokeCloudGrid &GetGrid( void ) { return m_Grid; }

	inline bool IsActive( void ) const { return m_bActive; }
	inline bool IsInPVS( void ) const { return m_bInPVS; }

public:
	Vector m_vecMins;
	Vector m_vecMaxs;

	float m_flLastUpdateTime;
	ESmokeCloudLOD m_LevelOfDetail;

	std::vector<CSmokeParticle> m_SmokeParticles;
	CSmokeCloudGrid				m_Grid;

	bool m_bActive;
	bool m_bInPVS;
};

//-----------------------------------------------------------------------------
// Smoke simulation feature
//-----------------------------------------------------------------------------

class CSmokeSimulation final : public CBaseFeature, IHookEventListener
{
public:
	CSmokeSimulation( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void	OnExplosion( const Vector &vecOrigin, float flRadius, float flForce );
	void	OnPlayerShoot( int iWeaponID, const Vector &vecStart, const Vector &vecEnd, float flForce = 10.f );
	void	OnFlash( const Vector &vecOrigin, const Vector &vecColor, float flRadius, float flIntensity );

	void	SpawnSmoke( const Vector &vecOrigin, float flRadius = 128.f, float flParticleSize = 16.f );
	void	TraceBullet( int iWeaponID, const Vector &p1, const Vector &p2, float flForce = 10.f );
	void	TraceExplosion( const Vector &vecPos, float flRadius, float flForce );
	void	TraceBodyMove( const Vector &vecBodyMins, const Vector &vecBodyMaxs, const Vector &vecMove );

	inline bool IsDebugDraw( void ) const { return m_pVizualize->GetBool(); }

private:
	Vector	GetRandomVector( void );
	void	LoadSprite( void );

	void	Reset( void );
	void	Update( void );
	void	Render( void );
	void	DebugDraw( void );

	Vector	CalculateDynamicLighting( const Vector &vecPos, const Vector &vecBaseColor );

	void	RenderParticle( const Vector &vecPos, float flRadius, float flAlpha, float flLightLevel = 0.75f );

	void	RenderParticles_LOD( CSmokeCloud &smokeCloud, float flDistance );
	void	RenderParticles_Normal( CSmokeCloud &smokeCloud );
	void	RenderParticles_Cluster( CSmokeCloud &smokeCloud, int iClusterSize );
	void	RenderParticles_Potato( CSmokeCloud &smokeCloud );

private:
	CMenuValueBool *m_pIlluminateSmokeByBullets;
	CMenuValueBool *m_pDynamicLighting;
	CMenuValueBool *m_pVizualize;
	CMenuValueInteger *m_pLimit;
	CMenuValueInteger *m_pSpacePartitionGrid;
	CMenuValueFloat *m_pCloudSize;
	CMenuValueFloat *m_pParticlesSize;

	std::vector<CSmokeCloud> m_SmokeClouds;
	std::vector<CSmokeFlash> m_SmokeFlashes;

	CSmokeParticle *m_pRenderedSmokeParticle;
	model_s *m_pSmokeTexture;

	Vector m_vecRight;
	Vector m_vecUp;

	float m_flTime;
	float m_flLastUpdateTime;
	float m_flLastTraceBodyMoveTime;
	float m_flFrametime;
	float m_flAlphaModifier;

	DetourHandle_t m_hNetMsgHook_TempEntity;
};

EXTERN_FEATURE( CSmokeSimulation, smokesim );

#endif // SINT_FEATURE_SMOKE_SIMULATION_H

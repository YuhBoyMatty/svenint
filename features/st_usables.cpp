// SvenInt (c) Sw1ft
// st_usables.cpp

#include "stdafx.h"
#include "st_usables.h"
#include "r_draw_context.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

// These are caps bits to indicate what an object's capabilities (currently used for save/restore and level transitions)
#define		FCAP_CUSTOMSAVE				0x00000001
#define		FCAP_ACROSS_TRANSITION		0x00000002		// should transfer between transitions
#define		FCAP_MUST_SPAWN				0x00000004		// Spawn after restore
#define		FCAP_DONT_SAVE				0x80000000		// Don't save this
#define		FCAP_IMPULSE_USE			0x00000008		// can be used by the player
#define		FCAP_CONTINUOUS_USE			0x00000010		// can be used by the player
#define		FCAP_ONOFF_USE				0x00000020		// can be used by the player
#define		FCAP_DIRECTIONAL_USE		0x00000040		// Player sends +/- 1 when using (currently only tracktrains)
#define		FCAP_MASTER					0x00000080		// Can be used to "master" other entities (like multisource)

// UNDONE: This will ignore transition volumes (trigger_transition), but not the PVS!!!
#define		FCAP_FORCE_TRANSITION		0x00000080		// ALWAYS goes across transitions

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CUsables, usables, "Speedrun Tools", "Show Usables" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CUsables::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	using namespace GameData::Offsets::Server;
	FUNC_SIGNATURE( int, CALLCONV_THISCALL, CBaseEntity__ObjectCapsFn, CBaseEntity * );

	if ( vtidx_CBaseEntity_ObjectCaps == ~0 || !Modules::server->Host_IsServerActive() )
		return kHookContinue;

	const edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );
	if ( pPlayer == NULL )
		return kHookContinue;

	std::vector<const edict_t *> usableEntities;

	edict_t *pObject = NULL;
	const Vector vecOrigin = pPlayer->v.origin;
	const float flSearchRadius = m_pRadius->GetFloat();

	while ( !Modules::server->FNullEnt( pObject = sv_enginefuncs->pfnFindEntityInSphere( pObject, vecOrigin, flSearchRadius ) ) )
	{
		// Valid
		if ( !Modules::server->IsValidEntity( pObject ) )
			continue;

		auto CBaseEntity__ObjectCaps = (CBaseEntity__ObjectCapsFn)MemoryUtils()->GetVirtualFunction( pObject->pvPrivateData, vtidx_CBaseEntity_ObjectCaps );
		if ( CBaseEntity__ObjectCaps == NULL )
			continue;

		// Usable
		if ( !( CBaseEntity__ObjectCaps( (CBaseEntity *)pObject->pvPrivateData ) & ( FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE ) ) )
			continue;

		usableEntities.push_back( pObject );
	}

	if ( usableEntities.empty() )
		return kHookContinue;

	constexpr float VIEW_FIELD_NARROW = 0.7f;

	Vector vecForward;
	float flMaxDot = VIEW_FIELD_NARROW;
	const edict_t *pTargetObject = NULL;
	CDrawBoxNoDepthBuffer *pDrawBoxTarget = NULL;

	AngleVectors( pPlayer->v.v_angle, &vecForward, NULL, NULL );

	for ( const edict_t *pObject : usableEntities )
	{
		Vector vecModelOrigin = pObject->v.absmin + pObject->v.size * 0.5f;
		const auto disp = vecModelOrigin - vecOrigin - pPlayer->v.view_ofs;

		float dot = DotProduct( UTIL_ClampVectorToBox( disp, pObject->v.size * 0.5f ), vecForward );

		if ( dot > flMaxDot )
		{
			pTargetObject = pObject;
			flMaxDot = dot;
		}
	}

	for ( const edict_t *pObject : usableEntities )
	{
		if ( pObject == pTargetObject )
			continue;

		Vector vecModelOrigin = pObject->v.absmin + pObject->v.size * 0.5f;
		const auto disp = vecModelOrigin - vecOrigin - pPlayer->v.view_ofs;

		if ( DotProduct( vecForward, disp ) > 0.0f )
		{
			pDrawBoxTarget = new CDrawBoxNoDepthBuffer( vecModelOrigin,
														Vector( -2, -2, -2 ),
														Vector( 2, 2, 2 ),
														{ 255, 0, 0, 127 } );

			Features::drawcontext->AddDrawContext( pDrawBoxTarget );
		}
	}

	if ( pTargetObject != NULL )
	{
		Vector vecModelOrigin = pTargetObject->v.absmin + pTargetObject->v.size * 0.5f;
		const auto disp = vecModelOrigin - vecOrigin - pPlayer->v.view_ofs;

		if ( DotProduct( vecForward, disp ) > 0.0f )
		{
			pDrawBoxTarget = new CDrawBoxNoDepthBuffer( vecModelOrigin,
														Vector( -2, -2, -2 ),
														Vector( 2, 2, 2 ),
														{ 0, 255, 0, 127 } );

			Features::drawcontext->AddDrawContext( pDrawBoxTarget );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CUsables::CUsables( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
	m_pRadius = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CUsables::OnEnable( void )
{
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CUsables::OnDisable( void )
{
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CUsables::Load( void )
{
	Modules::menu->BindFeature( this );
	m_pRadius = Modules::menu->AddParamFloat( this, "Radius", NULL, 64.f, 1.f, 512.f );
	return true;
}

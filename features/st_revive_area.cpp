// SvenInt (c) Sw1ft
// st_revive_area.cpp

#include "stdafx.h"
#include "st_revive_area.h"
#include "r_draw_context.h"
#include "misc_entity_list.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CReviveArea, revivearea, "Speedrun Tools", "Revive Area" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CReviveArea::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( UTIL_IsDead() )
		return kHookContinue;

	pmtrace_t tr;
	bool bDucking;
	Vector vecOrigin;

	int iLocalPlayer = UTIL_GetLocalPlayerIndex();

	CEntity *pEnts = Features::entitylist->GetList();

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		if ( ent.m_classInfo.id != CLASS_DEAD_PLAYER && !ent.m_bPlayer )
			continue;

		if ( iLocalPlayer == i && ( !m_pShowLocalPlayer->GetBool() || UTIL_IsSpectating() ) )
			continue;

		bDucking = false;
		vecOrigin = ( iLocalPlayer == i ) ? ( demoplayback ? refparams->simorg : *playermove->origin() ) : ent.m_pEntity->origin;

		if ( ent.m_bPlayer )
		{
			cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_PLAYER );
			cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecOrigin, vecOrigin, PM_NORMAL, -1, &tr );

			// Clipped with world
			if ( tr.startsolid )
			{
				bDucking = true;
			}
		}
		else if ( ent.m_pEntity->curstate.movetype == MOVETYPE_NONE ) // Sinking corpse
		{
			bDucking = true;
		}

		// Inconsistent!! but it's needed. For perfomance reasons I don't want to trace line / hull
		if ( bDucking )
		{
			// FixPlayerCrouchStuck, trace hull
			vecOrigin.z += 18.f;
		}
		else
		{
			// Trace line up to 32 units
			vecOrigin.z += 32.f;
		}

		// Draw small hull
		if ( m_pDrawSmallTestHull->GetBool() )
		{
			DrawBox( vecOrigin,
					 ( bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN ) + Vector( -16.f, -16.f, -16.f ),
					 ( bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX ) + Vector( 16.f, 16.f, 16.f ),
					 m_pSmallHullColor->GetColor()[ 0 ],
					 m_pSmallHullColor->GetColor()[ 1 ],
					 m_pSmallHullColor->GetColor()[ 2 ],
					 m_pSmallHullColor->GetColor()[ 3 ],
					 m_pSmallHullWidth->GetFloat(),
					 true );
		}

		// Draw medium hull
		if ( m_pDrawMediumTestHull->GetBool() )
		{
			DrawBox( vecOrigin,
					 ( bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN ) + Vector( -32.f, -32.f, -32.f ),
					 ( bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX ) + Vector( 32.f, 32.f, 32.f ),
					 m_pMediumHullColor->GetColor()[ 0 ],
					 m_pMediumHullColor->GetColor()[ 1 ],
					 m_pMediumHullColor->GetColor()[ 2 ],
					 m_pMediumHullColor->GetColor()[ 3 ],
					 m_pMediumHullWidth->GetFloat(),
					 true );
		}

		if ( m_pDrawLargeTestHull->GetBool() )
		{
			// Draw large hull
			DrawBox( vecOrigin,
					 ( bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN ) + Vector( -48.f, -48.f, -48.f ),
					 ( bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX ) + Vector( 48.f, 48.f, 48.f ),
					 m_pLargeHullColor->GetColor()[ 0 ],
					 m_pLargeHullColor->GetColor()[ 1 ],
					 m_pLargeHullColor->GetColor()[ 2 ],
					 m_pLargeHullColor->GetColor()[ 3 ],
					 m_pLargeHullWidth->GetFloat(),
					 true );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CReviveArea::CReviveArea( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pShowLocalPlayer = NULL;
	m_pDrawSmallTestHull = NULL;
	m_pDrawMediumTestHull = NULL;
	m_pDrawLargeTestHull = NULL;
	m_pSmallHullWidth = NULL;
	m_pMediumHullWidth = NULL;
	m_pLargeHullWidth = NULL;
	m_pSmallHullColor = NULL;
	m_pMediumHullColor = NULL;
	m_pLargeHullColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CReviveArea::OnEnable( void )
{
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CReviveArea::OnDisable( void )
{
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CReviveArea::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShowLocalPlayer = Modules::menu->AddParamBool( this, "ShowLocalPlayer", NULL, true );
	m_pDrawSmallTestHull = Modules::menu->AddParamBool( this, "DrawSmallTestHull", NULL, true );
	m_pDrawMediumTestHull = Modules::menu->AddParamBool( this, "DrawMediumTestHull", NULL, true );
	m_pDrawLargeTestHull = Modules::menu->AddParamBool( this, "DrawLargeTestHull", NULL, true );
	m_pSmallHullWidth = Modules::menu->AddParamFloat( this, "SmallHullWidth", NULL, 3.f, 1.f, 10.f );
	m_pMediumHullWidth = Modules::menu->AddParamFloat( this, "MediumHullWidth", NULL, 3.f, 1.f, 10.f );
	m_pLargeHullWidth = Modules::menu->AddParamFloat( this, "LargeHullWidth", NULL, 3.f, 1.f, 10.f );
	m_pSmallHullColor = Modules::menu->AddParamColorRGBA( this, "SmallHullColor", NULL, Color( 1.f, 0.f, 0.f, 0.5f ) );
	m_pMediumHullColor = Modules::menu->AddParamColorRGBA( this, "MediumHullColor", NULL, Color( 0.f, 1.f, 0.f, 0.5f ) );
	m_pLargeHullColor = Modules::menu->AddParamColorRGBA( this, "LargeHullColor", NULL, Color( 0.f, 0.f, 1.f, 0.5f ) );

	return true;
}
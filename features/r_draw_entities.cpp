// SvenInt (c) Sw1ft
// r_draw_entities.cpp

#include "stdafx.h"
#include "r_draw_entities.h"
#include "r_wallhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC_PTR( int, __cdecl, ORIG_StudioDrawPlayer, int, entity_state_t * );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDrawEntities, drawentities, "Render", "Draw Entities" );

//-----------------------------------------------------------------------------
// Studio renderer hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, __cdecl, HOOKED_StudioDrawPlayer, int flags, entity_state_t *pplayer )
{
	cl_entity_t *pEntity = enginestudio->GetCurrentEntity();

	if ( pEntity != NULL && THIS_FEATURE()->GetMode() == 7 )
		return 0;

	return ORIG_StudioDrawPlayer( flags, pplayer );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDrawEntities::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// StudioRenderModel event
	float flDrawEntitiesMode = float( m_pMode->GetInt() ) + 1.0f;

	if ( m_pMode->GetInt() == 7 ) // don't draw player models
	{
		flDrawEntitiesMode = 1.0f;
	}

	if ( flDrawEntitiesMode == 6.0f )
	{
		cl_entity_s *pEntity = enginestudio->GetCurrentEntity();

		if ( pEntity->player )
			r_drawentities->value = 2.0f;
		else
			r_drawentities->value = 1.0f;
	}
	else if ( flDrawEntitiesMode == 7.0f )
	{
		cl_entity_s *pEntity = enginestudio->GetCurrentEntity();

		if ( pEntity->player )
			r_drawentities->value = 3.0f;
		else
			r_drawentities->value = 1.0f;
	}
	else
	{
		r_drawentities->value = flDrawEntitiesMode;
	}

	if ( Features::wallhack->IsEnabled() &&
		 Features::wallhack->WallhackObjects() &&
		 r_drawentities->value >= 2.0f &&
		 r_drawentities->value <= 5.0f )
	{
		glDisable( GL_DEPTH_TEST );

		studiorenderer->StudioRenderFinal_Hardware();

		glEnable( GL_DEPTH_TEST );

		return kHookSupercede;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDrawEntities::CDrawEntities( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pMode = NULL;
	r_drawentities = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CDrawEntities::OnEnable( void )
{
	ORIG_StudioDrawPlayer = studioapi->StudioDrawPlayer;
	studioapi->StudioDrawPlayer = HOOKED_StudioDrawPlayer;

	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CDrawEntities::OnDisable( void )
{
	if ( ORIG_StudioDrawPlayer != NULL )
		studioapi->StudioDrawPlayer = ORIG_StudioDrawPlayer;

	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CDrawEntities::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Default\0 1 - Bones\0 2 - Hitboxes\0 3 - Model & Hitboxes\0 4 - Hulls\0 5 - Player Bones\0 6 - Player Hitboxes\0 7 - Skip Player Models\0\0" );

	r_drawentities = cvar->FindCvar( "r_drawentities" );
	FEATURE_CHECK_SYMBOL( r_drawentities, "r_drawentities" );

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CDrawEntities::Unload( void )
{
	if ( ORIG_StudioDrawPlayer != NULL )
		studioapi->StudioDrawPlayer = ORIG_StudioDrawPlayer;
}
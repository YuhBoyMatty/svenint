// SvenInt (c) Sw1ft
// visual_player_sight_direction.cpp

#include "stdafx.h"
#include "visual_player_sight_direction.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CPlayerSightDirection, sightdir, "Visual", "Player Sight Direction" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CPlayerSightDirection::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_iBeamIndex = 0;
		return kHookContinue;
	}

	if ( pEvent->GetReturn<int>() &&
		 pEvent->GetArg<int>( "type" ) == ET_PLAYER &&
		 pEvent->GetArg<cl_entity_t *>( "ent" )->index != localplayer->GetPlayerIndex() )
	{
		auto ent = pEvent->GetArg<cl_entity_t *>( "ent" );

		Vector vecEyes, vecEnd, vecForward, vecAngles;

		gameutils->GetViewAngles( ent, vecAngles );
		//vecAngles = ent->angles;
		//vecAngles.x *= -3.f;

		AngleVectors( vecAngles, &vecForward, NULL, NULL );

		vecEyes = ent->origin + Vector( 0.f, 0.f, ent->curstate.usehull ? 12.5f : 28.5f /* VEC_DUCK_VIEW.z : VEC_VIEW.z */ );
		vecEnd = vecEyes + vecForward * m_pBeamLength->GetFloat();

		if ( !m_iBeamIndex )
			m_iBeamIndex = cl_enginefuncs->pEventAPI->EV_FindModelIndex( "sprites/laserbeam.spr" );

		cl_enginefuncs->pEfxAPI->R_BeamPoints( vecEyes,
											   vecEnd,
											   m_iBeamIndex,
											   0.001f, // life time
											   m_pBeamWidth->GetFloat(),
											   0.f, // amplitude
											   32.f, // brightness
											   2.f, // speed
											   0, // startFrame
											   0.f, // framerate
											   m_pColor->GetColor()[ 0 ],
											   m_pColor->GetColor()[ 1 ],
											   m_pColor->GetColor()[ 2 ] );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CPlayerSightDirection::CPlayerSightDirection( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pBeamLength = NULL;
	m_pBeamWidth = NULL;
	m_pColor = NULL;

	m_iBeamIndex = 0;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CPlayerSightDirection::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CPlayerSightDirection::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CPlayerSightDirection::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pBeamLength = Modules::menu->AddParamFloat( this, "BeamLength", NULL, 48.f, 1.f, 1024.f );
	m_pBeamWidth = Modules::menu->AddParamFloat( this, "BeamWidth", NULL, 1.f, 1.f, 16.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, Color( 255, 255, 255, 255 ) );

	return true;
}
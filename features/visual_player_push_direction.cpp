// SvenInt (c) Sw1ft
// visual_player_push_direction.cpp

#include "stdafx.h"
#include "visual_player_push_direction.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CPlayerPushDirection, pushdir, "Visual", "Player Push Direction" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CPlayerPushDirection::OnEvent( CHookEvent *pEvent, bool bPostCall )
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

		Vector vecEnd;
		Vector vecEnd2;

		Vector vecBegin = ent->origin;

		Vector vecPushDir( 1.f, 0.f, 0.f );
		Vector vecPushDir2( 0.f, 1.f, 0.f );

		if ( ent->curstate.usehull )
			vecBegin[ 2 ] += VEC_DUCK_HULL_MIN.z + 1.5f;
		else
			vecBegin[ 2 ] += VEC_HULL_MIN.z + 1.5f;

		vecEnd = vecBegin + vecPushDir * m_pBeamLength->GetFloat();
		vecEnd2 = vecBegin + vecPushDir2 * m_pBeamLength->GetFloat() * ( 1.f / 3.f );

		if ( !m_iBeamIndex )
			m_iBeamIndex = cl_enginefuncs->pEventAPI->EV_FindModelIndex( "sprites/laserbeam.spr" );

		// Opposite direction
		cl_enginefuncs->pEfxAPI->R_BeamPoints( vecBegin,
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

		// 90 deg. opposite direction that also lets you to push a player
		cl_enginefuncs->pEfxAPI->R_BeamPoints( vecBegin,
											   vecEnd2,
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

CPlayerPushDirection::CPlayerPushDirection( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
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

void CPlayerPushDirection::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CPlayerPushDirection::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CPlayerPushDirection::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pBeamLength = Modules::menu->AddParamFloat( this, "BeamLength", NULL, 16.f, 1.f, 128.f );
	m_pBeamWidth = Modules::menu->AddParamFloat( this, "BeamWidth", NULL, 1.f, 1.f, 16.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, Color( 255, 0, 0, 255 ) );

	return true;
}
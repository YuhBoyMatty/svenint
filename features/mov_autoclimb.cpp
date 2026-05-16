// SvenInt (c) Sw1ft
// mov_autoclimb.cpp

#include "stdafx.h"
#include "mov_autoclimb.h"
#include "player_stick.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAutoLadderClimb, autoclimb, "Movement", "Auto Climb" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_autoclimb, "Automatically climb when on a ladder" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAutoLadderClimb::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		if ( !m_bForcePitch )
			return kHookContinue;

		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
		if ( pLocal == NULL )
			return kHookContinue;

		pLocal->angles.x = m_flSavedPitchAngle;
		pLocal->curstate.angles.x = m_flSavedPitchAngle;
		pLocal->prevstate.angles.x = m_flSavedPitchAngle;
		pLocal->latched.prevangles.x = m_flSavedPitchAngle;

		m_bForcePitch = false;
		return kHookHandled;
	}

	// CL_CreateMove post event
	pm_ladder_t ladder;
	Vector va, vecClimbAngles;
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	const bool bTriggerWhenPressingMovementButtons = m_pTriggerWhenPressingMovementButtons->GetBool();
	const bool bPressedAnyMovementButton = ( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) );

	if ( bTriggerWhenPressingMovementButtons ^ bPressedAnyMovementButton )
		return kHookContinue;

	if ( !bTriggerWhenPressingMovementButtons && ( cmd->buttons & IN_USE ) )
		return kHookContinue;

	cl_enginefuncs->GetViewAngles( va );

	if ( bTriggerWhenPressingMovementButtons )
	{
		Vector2D vecMove;

		if ( cmd->buttons & IN_FORWARD )
			vecMove.x += 1.f;
		if ( cmd->buttons & IN_BACK )
			vecMove.x += -1.f;

		if ( vecMove.x == 0.f )
			return kHookContinue;

		if ( vecMove.x < 0.f )
			va.x *= -1.f;

		cmd->buttons &= ~( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT );
	}

	if ( Features::stick->IsEnabled() ||
		 va.x <= m_pThresholdAngle->GetFloat() && va.x >= -m_pThresholdAngle->GetFloat() ||
		 !UTIL_GetPlayerMoveLadder( &ladder ) )
	{
		return kHookContinue;
	}

	vecClimbAngles.x = 89.f;
	vecClimbAngles.y = NormalizeAngle( ladder.angle - 90.f );
	vecClimbAngles.z = 0.f;

	cmd->viewangles = vecClimbAngles;

	m_bForcePitch = true;
	m_flSavedPitchAngle = cmd->viewangles.x / -3.f;

	cmd->forwardmove = 0.f;
	cmd->sidemove = 0.f;
	cmd->upmove = 0.f;

	if ( va.x < 0.f )
	{
		// Down
		cmd->forwardmove = -playermove->maxspeed();
		cmd->sidemove = playermove->maxspeed();
		cmd->buttons |= ( IN_BACK | IN_MOVERIGHT );
	}
	else
	{
		// Up
		cmd->forwardmove = playermove->maxspeed();
		cmd->sidemove = -playermove->maxspeed();
		cmd->buttons |= ( IN_FORWARD | IN_MOVELEFT );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAutoLadderClimb::CAutoLadderClimb( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pTriggerWhenPressingMovementButtons = NULL;
	m_pThresholdAngle = NULL;

	m_bForcePitch = false;
	m_flSavedPitchAngle = 0.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAutoLadderClimb::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAutoLadderClimb::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAutoLadderClimb::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pTriggerWhenPressingMovementButtons = Modules::menu->AddParamBool( this, "TriggerWhenPressingMovementButtons", NULL, false );
	m_pThresholdAngle = Modules::menu->AddParamFloat( this, "ThresholdAngle", "Threshold pitch angle to trigger auto climb", 15.f, 0.1f, 89.f);

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAutoLadderClimb::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_autoclimb );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAutoLadderClimb::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_autoclimb );
}
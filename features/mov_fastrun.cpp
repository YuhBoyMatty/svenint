// SvenInt (c) Sw1ft
// mov_fastrun.cpp

#include "stdafx.h"
#include "mov_fastrun.h"
#include "player_silent_angles.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFastrun, fastrun, "Movement", "Fastrun" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_fastrun, "Move faster on ground" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CFastrun::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( !localplayer->IsOnGround() && !localplayer->IsSpectating() )
		return kHookContinue;

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( !( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
		return kHookContinue;

	const float flMaxSpeed = localplayer->GetMaxSpeed();
	Vector2D vecMove;

	// We do the mapping now
	if ( cmd->buttons & IN_FORWARD )
		vecMove.x += 1.f;
	if ( cmd->buttons & IN_BACK )
		vecMove.x -= 1.f;
	if ( cmd->buttons & IN_MOVELEFT )
		vecMove.y -= 1.f;
	if ( cmd->buttons & IN_MOVERIGHT )
		vecMove.y += 1.f;

	vecMove.NormalizeInPlace();

	cmd->forwardmove = vecMove.x * flMaxSpeed;
	cmd->sidemove = vecMove.y * flMaxSpeed;

	const float thetaRotation = VEC_DEG2RAD( 45.f );
	const float fm = cmd->forwardmove, sm = cmd->sidemove;
	const float cy = cosf( thetaRotation ), sy = sinf( thetaRotation );

	// Rotate to wish direction
	if ( m_bSideway )
	{
		cmd->forwardmove = fm * cy - sm * sy;
		cmd->sidemove = fm * sy + sm * sy;
	}
	else
	{
		cmd->forwardmove = fm * cy + sm * sy;
		cmd->sidemove = -fm * sy + sm * sy;
	}

	m_bSideway = !m_bSideway;

	if ( m_pBypassAntiStrafer->GetBool() )
	{
		float thetaMove = atan2f( cmd->sidemove, cmd->forwardmove );
		float flSpeed = flMaxSpeed; // sqrtf( cmd->sidemove * cmd->sidemove + cmd->forwardmove * cmd->forwardmove );
		float moveDirDeg = NormalizeAngle( cmd->viewangles[ 1 ] - VEC_RAD2DEG( thetaMove ) );

		float flNewYaw;
		if ( !m_bSideway ) // strafing to right
			flNewYaw = NormalizeAngle( moveDirDeg + 90.0f );
		else
			flNewYaw = NormalizeAngle( moveDirDeg - 90.0f );

		Vector vecNewAngles = cmd->viewangles;
		vecNewAngles.y = flNewYaw;

		Features::silentangles->SetAngles( vecNewAngles );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFastrun::CFastrun( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pBypassAntiStrafer = NULL;
	m_bSideway = false;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CFastrun::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CFastrun::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFastrun::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pBypassAntiStrafer = Modules::menu->AddParamBool( this, "BypassAntiStrafer", NULL, false );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CFastrun::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_fastrun );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CFastrun::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_fastrun );
}

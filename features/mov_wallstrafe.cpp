// SvenInt (c) Sw1ft
// mov_wallstrafe.cpp

#include "stdafx.h"
#include "mov_wallstrafe.h"
#include "st_input_manager.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CWallStrafe, wallstrafe, "Movement", "Wallstrafe" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_wallstrafe, "Strafe along a nearby wall" );

ConVar sc_wallstrafe_yaw( "sc_wallstrafe_yaw", "", FCVAR_CLIENTDLL, "Yaw angle to wall strafe" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CWallStrafe::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsDead() || localplayer->GetWaterLevel() != WL_NOT_IN_WATER || !localplayer->IsOnGround() )
		return kHookContinue;

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
	if ( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_LEFT | IN_RIGHT | IN_RUN ) )
		return kHookContinue;

	pmtrace_t trace;
	Vector2D vecNormal;

	Vector va;
	Vector vecForward, vecRight, vecLeft;
	Vector vecOrigin = localplayer->GetOrigin();

	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

	bool bWallStrafe = false;
	bool bRight = false;

	if ( sc_wallstrafe_yaw.GetString()[ 0 ] == '\0' )
		cl_enginefuncs->GetViewAngles( va );
	else
		va.y = sc_wallstrafe_yaw.GetFloat();

	vecForward.x = cosf( va.y * static_cast<float>( M_PI ) / 180.f );
	vecForward.y = sinf( va.y * static_cast<float>( M_PI ) / 180.f );
	vecForward.z = 0.f;

	vecRight.x = vecForward.y * m_pDistance->GetFloat();
	vecRight.y = -vecForward.x * m_pDistance->GetFloat();
	vecRight.z = 0.f;

	vecLeft = -vecRight;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( ( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecOrigin, vecOrigin + vecRight, PM_WORLD_ONLY, -1, &trace );

	if ( trace.fraction < 1.f )
	{
		bWallStrafe = true;
		bRight = true;
		vecNormal = trace.plane.normal.AsVector2D();
	}
	else
	{
		cl_enginefuncs->pEventAPI->EV_SetTraceHull( ( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
		cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecOrigin, vecOrigin + vecLeft, PM_WORLD_ONLY, -1, &trace );

		if ( trace.fraction < 1.f )
		{
			bWallStrafe = true;
			vecNormal = trace.plane.normal.AsVector2D();
		}
	}

	if ( bWallStrafe )
	{
		Vector vecDir;

		float flBestStrafeAngle = ( m_pAngle->GetFloat() + 90.f + 45.f ) * static_cast<float>( M_PI ) / 180.f; // radians

		if ( bRight )
			flBestStrafeAngle *= -1.f;

		vecNormal.NormalizeInPlace();

		// Rotate
		vecDir.x = vecNormal.x * cosf( flBestStrafeAngle ) - vecNormal.y * sinf( flBestStrafeAngle );
		vecDir.y = vecNormal.x * sinf( flBestStrafeAngle ) + vecNormal.y * cosf( flBestStrafeAngle );
		vecDir.z = 0.f;

		vecForward.x = cosf( cmd->viewangles.y * static_cast<float>( M_PI ) / 180.f );
		vecForward.y = sinf( cmd->viewangles.y * static_cast<float>( M_PI ) / 180.f );
		vecForward.z = 0.f;

		vecRight.x = vecForward.y;
		vecRight.y = -vecForward.x;
		vecRight.z = 0.f;

		vecForward *= localplayer->GetMaxSpeed();
		vecRight *= localplayer->GetMaxSpeed();

		float forwardmove = DotProduct( vecForward, vecDir );
		float sidemove = DotProduct( vecRight, vecDir );

		cmd->forwardmove = forwardmove;
		cmd->sidemove = sidemove;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CWallStrafe::CWallStrafe( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pAngle = NULL;
	m_pDistance = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CWallStrafe::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CWallStrafe::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CWallStrafe::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pAngle = Modules::menu->AddParamFloat( this, "Angle", NULL, 6.5f, 0.f, 90.f );
	m_pDistance = Modules::menu->AddParamFloat( this, "WallDistance", "Max distance to a wall", 32.f, 0.f, 256.f);

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CWallStrafe::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_wallstrafe );
	FEATURE_REGISTER_CVAR( sc_wallstrafe_yaw );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CWallStrafe::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_wallstrafe );
	FEATURE_UNREGISTER_CVAR( sc_wallstrafe_yaw );
}
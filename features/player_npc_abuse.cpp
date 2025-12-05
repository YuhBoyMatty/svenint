// SvenInt (c) Sw1ft
// player_npc_abuse.cpp

#include "stdafx.h"
#include "player_npc_abuse.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAutoNPCAbuse, npcabuse, "Player", "NPC Abuse" );

//-----------------------------------------------------------------------------
// MoveToDesiredPoint
//-----------------------------------------------------------------------------

void CAutoNPCAbuse::MoveToDesiredPoint( usercmd_t *cmd, const Vector2D &vecDesiredPoint )
{
	Vector2D vecForward;
	Vector2D vecRight;

	Vector2D vecDir = vecDesiredPoint - playermove->origin()->AsVector2D();
	vecDir.NormalizeInPlace();

	// Forward angles
	vecForward.x = cosf( VEC_DEG2RAD( cmd->viewangles.y ) );
	vecForward.y = sinf( VEC_DEG2RAD( cmd->viewangles.y ) );

	// Make a right vector of angles. Rotate forward vector as a complex number by 90 deg.
	vecRight.x = vecForward.y;
	vecRight.y = -vecForward.x;

	const float flSpeed = 50.f;

	// Multiply by max movement speed
	vecForward *= flSpeed;
	vecRight *= flSpeed;

	// Project onto direction vector
	float forwardmove = DotProduct( vecForward, vecDir );
	float sidemove = DotProduct( vecRight, vecDir );

	// Apply moves
	cmd->forwardmove = forwardmove;
	cmd->sidemove = sidemove;
}

//-----------------------------------------------------------------------------
// hl_c10
// 
// How it works:
// 
// You should stand somewhere near the door leading out, as well
// the NPC must follow you, make sure the NPC is near to you.
// 
// Then, you enable this feature, you will move to the position where you
// will perform NPC Abuse. When this step is done, your camera is set to the position
// where the NPC must stand, abd then we send cmd 'npc_moveto' to tell that NPC to move
// to the desired position. 
// 
// Feature waits 1 sec for the NPC to move to the desired position, then camera is set
// to the door and we send the last cmd 'npc_moveto'.
// 
// Door opens, we're done.
//-----------------------------------------------------------------------------

void CAutoNPCAbuse::execute_hl_c10( usercmd_t *cmd )
{
	const Vector2D vecPerformOrigin( 37.875000f, -841.312500f );
	QAngle vecAnglesNpcInitialPoint( 50.280048f, -178.434448f, 0.000000f );
	QAngle vecAnglesNpcMovePoint( 51.600052f, -71.548462f, 0.000000f );

	if ( playermove->origin()->AsVector2D().DistTo( vecPerformOrigin ) > 1.f )
	{
		MoveToDesiredPoint( cmd, vecPerformOrigin );
		m_flWaitForNPC = -1.f;
		return;
	}

	if ( m_flWaitForNPC == -1.f )
	{
		cl_enginefuncs->SetViewAngles( vecAnglesNpcInitialPoint );
		cl_enginefuncs->pfnClientCmd( "npc_moveto" );

		m_flWaitForNPC = cl_enginefuncs->GetClientTime() + 1.f;
	}
	else if ( cl_enginefuncs->GetClientTime() - m_flWaitForNPC >= 0.f )
	{
		cl_enginefuncs->SetViewAngles( vecAnglesNpcMovePoint );
		cl_enginefuncs->pfnClientCmd( "npc_moveto" );

		Disable();
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAutoNPCAbuse::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsDead() )
	{
		Disable();
		return kHookContinue;
	}

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
	const char *pszMapname = cl_enginefuncs->pfnGetLevelName();

	if ( pszMapname == NULL )
		return kHookContinue;

	if ( m_pHL_C10->GetBool() && strstr( pszMapname, "_c10.bsp" ) )
	{
		execute_hl_c10( cmd );
	}
	else
	{
		Disable();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAutoNPCAbuse::CAutoNPCAbuse( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
	
	m_pHL_C10 = NULL;
	m_flWaitForNPC = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAutoNPCAbuse::OnEnable( void )
{
	m_flWaitForNPC = -1.f;

	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAutoNPCAbuse::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAutoNPCAbuse::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pHL_C10 = Modules::menu->AddParamBool( this, "hl_c10", NULL, true );

	return true;
}
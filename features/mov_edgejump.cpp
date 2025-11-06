// SvenInt (c) Sw1ft
// mov_edgejump.cpp

#include "stdafx.h"
#include "mov_edgejump.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CEdgeJump, edgejump, "Movement", "Edgejump" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_edgejump, "Auto jump on edge" );

//-----------------------------------------------------------------------------
// Execute edge jump
//-----------------------------------------------------------------------------

void CEdgeJump::Execute( usercmd_t *cmd, float frametime )
{
	Vector vecPredictVelocity = localplayer->GetVelocity() * frametime;
	Vector vecPredictOrigin = localplayer->GetOrigin() + vecPredictVelocity;

	Vector vecEnd = vecPredictOrigin;
	vecEnd.z -= m_pHeight->GetFloat();

	pmtrace_t *pTrace = cl_enginefuncs->PM_TraceLine( vecPredictOrigin,
													  vecEnd,
													  PM_NORMAL,
													  ( localplayer->GetFlags() & FL_DUCKING ) ? 1 : 0,
													  -1 );

	if ( pTrace->fraction == 1.f )
	{
		cmd->buttons |= IN_JUMP;
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CEdgeJump::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( !localplayer->IsOnGround() )
		return kHookContinue;

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
	auto frametime = pEvent->GetArg<float>( "frametime" );

	Execute( cmd, frametime );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CEdgeJump::CEdgeJump( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pHeight = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CEdgeJump::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CEdgeJump::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CEdgeJump::Load( void )
{
	Modules::menu->BindFeature( this );
	m_pHeight = Modules::menu->AddParamFloat( this, "Height", NULL, 3.f, 1.f, 10.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CEdgeJump::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_edgejump );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CEdgeJump::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_edgejump );
}
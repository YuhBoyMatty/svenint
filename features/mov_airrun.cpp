// SvenInt (c) Sw1ft
// mov_airrun.cpp

#include "stdafx.h"
#include "mov_airrun.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAirRun, airrun, "Movement", "Air Run" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAirRun::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsSpectating() || localplayer->GetMoveType() != MOVETYPE_WALK )
		return kHookContinue;

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	cmd->buttons |= ( 1 << 22 ); // IN_BULLRUSH

	if ( cmd->buttons & IN_JUMP )
	{
		cmd->buttons |= IN_DUCK;

		if ( cmd->buttons & IN_DUCK && !localplayer->IsOnGround() )
			cmd->buttons &= ~IN_DUCK;
	};

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAirRun::CAirRun( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAirRun::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAirRun::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAirRun::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}
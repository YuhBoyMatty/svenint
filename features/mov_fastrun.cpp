// SvenInt (c) Sw1ft
// mov_fastrun.cpp

#include "stdafx.h"
#include "mov_fastrun.h"

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
	const float flMaxSpeed = localplayer->GetMaxSpeed();

	if ( ( cmd->buttons & IN_FORWARD && cmd->buttons & IN_MOVELEFT ) ||
		 ( cmd->buttons & IN_BACK && cmd->buttons & IN_MOVERIGHT ) )
	{
		if ( m_bSideway )
		{
			cmd->sidemove -= flMaxSpeed; // sqrtf(2.0f) * flMaxSpeed   vvv
			cmd->forwardmove -= flMaxSpeed;

			m_bSideway = false;
		}
		else
		{
			cmd->sidemove += flMaxSpeed;
			cmd->forwardmove += flMaxSpeed;

			m_bSideway = true;
		}
	}
	else if ( ( cmd->buttons & IN_FORWARD && cmd->buttons & IN_MOVERIGHT ) ||
			  ( cmd->buttons & IN_BACK && cmd->buttons & IN_MOVELEFT ) )
	{
		if ( m_bSideway )
		{
			cmd->sidemove -= flMaxSpeed;
			cmd->forwardmove += flMaxSpeed;

			m_bSideway = false;
		}
		else
		{
			cmd->sidemove += flMaxSpeed;
			cmd->forwardmove -= flMaxSpeed; // sqrtf(2.0f) * flMaxSpeed  ^^^

			m_bSideway = true;
		}
	}
	else if ( cmd->buttons & IN_FORWARD || cmd->buttons & IN_BACK )
	{
		if ( m_bSideway )
		{
			cmd->sidemove -= flMaxSpeed;
			m_bSideway = false;
		}
		else
		{
			cmd->sidemove += flMaxSpeed;
			m_bSideway = true;
		}
	}
	else if ( cmd->buttons & IN_MOVELEFT || cmd->buttons & IN_MOVERIGHT )
	{
		if ( m_bSideway )
		{
			cmd->forwardmove -= flMaxSpeed;
			m_bSideway = false;
		}
		else
		{
			cmd->forwardmove += flMaxSpeed;
			m_bSideway = true;
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFastrun::CFastrun( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

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
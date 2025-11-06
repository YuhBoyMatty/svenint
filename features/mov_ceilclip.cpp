// SvenInt (c) Sw1ft
// mov_ceilclip.cpp

#include "stdafx.h"
#include "mov_ceilclip.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAutoCeilClipping, autoceilclipping, "Movement", "Auto Ceil-Clipping" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_autoceilclipping, "Automatically suicide under the ceil" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAutoCeilClipping::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	static bool jumped = false;

	if ( localplayer->IsDead() )
	{
		jumped = false;
		return kHookContinue;
	}

	if ( jumped )
	{
		if ( !localplayer->IsOnGround() && localplayer->GetWaterLevel() <= WL_FEET )
		{
			pEvent->GetArg<usercmd_t *>( "cmd" )->buttons |= IN_DUCK;

			// Suicide only if we reached apex or started falling
			if ( localplayer->GetVelocity().z <= 0.f )
			{
				Vector vecStart = *playermove->origin();
				Vector vecEnd = vecStart + Vector( 0.f, 0.f, VEC_DUCK_HULL_MAX.z );

				pmtrace_t *pTrace = cl_enginefuncs->PM_TraceLine( vecStart, vecEnd, PM_NORMAL, ( localplayer->GetFlags() & FL_DUCKING ) ? 1 : 0, -1 );

				if ( pTrace->fraction != 1.0f )
				{
					cl_enginefuncs->pfnClientCmd( "kill\n" );
					jumped = false;
				}
			}
		}
		else
		{
			jumped = false;
		}
	}
	else
	{
		if ( !localplayer->IsOnGround() && localplayer->GetVelocity().z > 0.f )
			jumped = true;
		else
			jumped = false;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAutoCeilClipping::CAutoCeilClipping( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAutoCeilClipping::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAutoCeilClipping::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAutoCeilClipping::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAutoCeilClipping::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_autoceilclipping );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAutoCeilClipping::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_autoceilclipping );
}
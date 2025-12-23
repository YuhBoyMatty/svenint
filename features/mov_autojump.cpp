// SvenInt (c) Sw1ft
// mov_autojump.cpp

#include "stdafx.h"
#include "mov_autojump.h"
#include "st_input_manager.h"
#ifndef WIN32
#include <SDL.h>
#endif

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAutoJump, autojump, "Movement", "Auto Jump" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_autojump, "Automatically jump when on ground" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAutoJump::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( Features::inputmanager->IsPlayingback() )
		return kHookContinue;

	if ( !( cmd->buttons & IN_JUMP ) )
		return kHookContinue;
	
	if ( playermove->movetype() != MOVETYPE_WALK )
		return kHookContinue;
	
	if ( localplayer->IsDead() )
		return kHookContinue;
	
	if ( playermove->movevars() == NULL )
		return kHookContinue;

	if ( playermove->waterlevel() >= WL_WAIST )
		return kHookContinue;
	
	if ( m_pType->GetInt() == 0 ) // Fall Velocity based
	{
		if ( playermove->velocity()->z != 0.f && playermove->flFallVelocity() != 0.f )
		{
			cmd->buttons &= ~IN_JUMP;
		}
		else if ( playermove->oldbuttons() & IN_JUMP )
		{
			cmd->buttons &= ~IN_JUMP;
		}
	}
	else if ( m_pType->GetInt() == 1 ) // Prediction
	{
		if ( playermove->onground() == -1 )
		{
			pmtrace_t *tr;
			Vector vecOrigin = *playermove->origin();
			Vector vecVelocity = *playermove->velocity();
			int usehull = ( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER;

			UTIL_AddCorrectGravity( vecVelocity, playermove->frametime() );

			const float step = 2 * vecVelocity.z * playermove->frametime();

			tr = cl_enginefuncs->PM_TraceLine( vecOrigin,
												vecOrigin - Vector( 0.f, 0.f, step ),
												PM_NORMAL,
												usehull /* playermove->usehull */,
												-1 );

			if ( tr->fraction != 0.f )
			{
				cmd->buttons &= ~IN_JUMP;
			}
		}
		else if ( playermove->oldbuttons() & IN_JUMP )
		{
			cmd->buttons &= ~IN_JUMP;
		}
	}
	else if ( m_pType->GetInt() == 2 ) // Legacy with speed loss
	{
		static bool s_bAllowJump = false;

		if ( cmd->buttons & IN_JUMP )
		{
		#ifdef WIN32
			if ( s_bAllowJump && GetAsyncKeyState( VK_SPACE ) )
		#else
			if ( s_bAllowJump && Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_SPACE ) )
		#endif
			{
				cmd->buttons &= ~IN_JUMP;
				s_bAllowJump = false;
			}
			else
			{
				s_bAllowJump = true;
			}
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAutoJump::CAutoJump( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pType = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAutoJump::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall, kHookPriorityHigh );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAutoJump::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAutoJump::Load( void )
{
	Modules::menu->BindFeature( this );
	m_pType = Modules::menu->AddParamList( this, "Type", NULL, 0, " 0 - FallVel\0 1 - Prediction\0 2 - Legacy\0\0" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAutoJump::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_autojump );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAutoJump::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_autojump );
}

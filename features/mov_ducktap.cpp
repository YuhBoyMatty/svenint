// SvenInt (c) Sw1ft
// mov_ducktap.cpp

#include "stdafx.h"
#include "mov_ducktap.h"
#include "player_speedhack.h"
#include "st_input_manager.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDucktap, ducktap, "Movement", "Ducktap" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_HOLD( sc_ducktap, "" );

ConVar sc_ducktap_adjust_fps( "sc_ducktap_adjust_fps", "0", FCVAR_EXTDLL, "Change FPS to the given value when ducktapping to minimize ground friction" );
ConVar sc_ducktap_adjust_fps_with_speedhack( "sc_ducktap_adjust_fps_with_speedhack", "0", FCVAR_EXTDLL, "Use speedhack feature to change FPS" );

//-----------------------------------------------------------------------------
// Execute auto ducktap
//-----------------------------------------------------------------------------

bool CDucktap::Execute( usercmd_t *cmd )
{
	if ( localplayer->IsDead() )
		return false;

	char rgszCmdBuffer[ 64 ];

	// Change FPS back to normal
	if ( m_bReturnFPS )
	{
		if ( !sc_ducktap_adjust_fps_with_speedhack.GetBool() )
		{
			CVar()->SetValue( GameData::Cvars::fps_max, m_iSavedFPS );

			// Record FPS change
			snprintf( rgszCmdBuffer, Q_ARRAYSIZE( rgszCmdBuffer ), "fps_max %d\n", m_iSavedFPS );
			Features::inputmanager->RecordCommandNow( rgszCmdBuffer );
		}
		else
		{
			const int iNoFrictionFPS = sc_ducktap_adjust_fps.GetInt();
			const float flCurrentFPS = GameData::Cvars::fps_max->value;
			Features::speedhack->SetGameSpeed( ( 1.f / flCurrentFPS ) * ( flCurrentFPS / iNoFrictionFPS ) * 1000.f );
		}

		m_bReturnFPS = false;
	}

	// No friction
	if ( sc_ducktap_adjust_fps.GetInt() > 20 && Modules::server->Host_IsServerActive() && !playermove->bInDuck() )
	{
		if ( playermove->onground() == -1 && playermove->velocity()->z < 0.f )
		{
			pmtrace_t tr;

			bool bPredictedOnGround = false;

			const int oldhull = playermove->usehull();
			float flFrametime = playermove->frametime();
			Vector vecVelocity = *playermove->velocity();
			Vector vecOrigin = *playermove->origin();

			UTIL_AddCorrectGravity( vecVelocity, flFrametime );

			Vector vecMove = vecVelocity * flFrametime;

			// Trace forward
			playermove->setusehull( ( playermove->flags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );

			tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_NORMAL, -1 );

			// Save trace pos
			vecOrigin = tr.endpos;

			// Did hit a wall or started in solid
			if ( tr.fraction != 1.f && !tr.allsolid && tr.plane.normal.z >= 0.7f )
			{
				bPredictedOnGround = true;
			}
			else
			{
				Vector point = vecOrigin;
				point.z -= 2.f;

				// Trace down
				playermove->setusehull( ( playermove->flags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );

				tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, point, PM_NORMAL, -1 );

				if ( tr.plane.normal.z >= 0.7f )
				{
					bPredictedOnGround = true;
				}
			}

			playermove->setusehull( oldhull );

			// Not needed actually
			//FixupGravityVelocity( vecVelocity, flFrametime );

			if ( bPredictedOnGround )
			{
				if ( !sc_ducktap_adjust_fps_with_speedhack.GetBool() )
				{
					m_iSavedFPS = int( GameData::Cvars::fps_max->value );

					CVar()->SetValue( GameData::Cvars::fps_max, sc_ducktap_adjust_fps.GetInt() );

					// Record FPS change
					snprintf( rgszCmdBuffer, Q_ARRAYSIZE( rgszCmdBuffer ), "fps_max %d\n", sc_ducktap_adjust_fps.GetInt() );
					Features::inputmanager->RecordCommandNow( rgszCmdBuffer );
				}

				m_bReturnFPS = true;
				return false;
			}
		}
	}

	bool bDuckTapped = false;
	const bool bOnGround = ( playermove->onground() != -1 );

	if ( bOnGround && !m_bWasOnGround && !playermove->bInDuck() )
	{
		cmd->buttons |= IN_DUCK;
		bDuckTapped = true;
	}

	m_bWasOnGround = bOnGround;
	return bDuckTapped;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDucktap::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	Execute( pEvent->GetArg<usercmd_t *>( "cmd" ) );
	
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDucktap::CDucktap( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bWasOnGround = true;
	m_bReturnFPS = false;
	m_iSavedFPS = 200;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CDucktap::OnEnable( void )
{
	if ( localplayer->IsOnGround() )
		cl_enginefuncs->pfnClientCmd( "+duck;wait;-duck" );

	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CDucktap::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CDucktap::Load( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CDucktap::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_ducktap_down );
	FEATURE_REGISTER_CCMD( sc_ducktap_up );
	FEATURE_REGISTER_CVAR( sc_ducktap_adjust_fps );
	FEATURE_REGISTER_CVAR( sc_ducktap_adjust_fps_with_speedhack );

	cl_enginefuncs->pfnClientCmd( "-sc_ducktap" );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CDucktap::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_ducktap_down );
	FEATURE_UNREGISTER_CCMD( sc_ducktap_up );
	FEATURE_UNREGISTER_CVAR( sc_ducktap_adjust_fps );
	FEATURE_UNREGISTER_CVAR( sc_ducktap_adjust_fps_with_speedhack );
}

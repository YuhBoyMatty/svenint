// SvenInt (c) Sw1ft
// mov_ducktap.cpp

#include "stdafx.h"
#include "mov_ducktap.h"
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

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDucktap::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsDead() )
		return kHookContinue;

	static char fps_buffer[ 64 ];
	static int onground_prev = 0;
	static int fps_prev = 200;
	static bool must_return_fps = false;

	// Change fps back to normal
	if ( must_return_fps )
	{
		CVar()->SetValue( GameData::Cvars::fps_max, fps_prev );

		// Record fps change
		if ( Features::inputmanager->IsRecording() && Features::inputmanager->GetInputContext().FrameCounter() - 1 >= 0 )
		{
			snprintf( fps_buffer, Q_ARRAYSIZE( fps_buffer ), "fps_max %d\n", fps_prev );

			std::vector<im_frame_t> &frames = Features::inputmanager->GetInputContext().Frames();
			int prevframe = Features::inputmanager->GetInputContext().FrameCounter() - 1;

			if ( frames[ prevframe ].commands != NULL )
			{
				std::string sCommandsBuffer = frames[ prevframe ].commands;
				sCommandsBuffer += fps_buffer;

				free( (void *)( frames[ prevframe ].commands ) );

				frames[ prevframe ].commands = strdup( sCommandsBuffer.c_str() );
			}
			else
			{
				frames[ prevframe ].commands = strdup( fps_buffer );
			}
		}

		must_return_fps = false;
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
				fps_prev = int( GameData::Cvars::fps_max->value );

				CVar()->SetValue( GameData::Cvars::fps_max, sc_ducktap_adjust_fps.GetInt() );

				// Record fps change
				if ( Features::inputmanager->IsRecording() && Features::inputmanager->GetInputContext().FrameCounter() - 1 >= 0 )
				{
					snprintf( fps_buffer, Q_ARRAYSIZE( fps_buffer ), "fps_max %d\n", sc_ducktap_adjust_fps.GetInt() );

					std::vector<im_frame_t> &frames = Features::inputmanager->GetInputContext().Frames();
					int prevframe = Features::inputmanager->GetInputContext().FrameCounter() - 1;

					if ( frames[ prevframe ].commands != NULL )
					{
						std::string sCommandsBuffer = frames[ prevframe ].commands;
						sCommandsBuffer += fps_buffer;

						free( (void *)( frames[ prevframe ].commands ) );

						frames[ prevframe ].commands = strdup( sCommandsBuffer.c_str() );
					}
					else
					{
						frames[ prevframe ].commands = strdup( fps_buffer );
					}
				}

				must_return_fps = true;
				return kHookContinue;
			}
		}
	}

	if ( playermove->onground() != -1 && m_prevground == -1 && !playermove->bInDuck() )
	{
		pEvent->GetArg<usercmd_t *>( "cmd" )->buttons |= IN_DUCK;
	}

	m_prevground = playermove->onground();
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDucktap::CDucktap( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_prevground = 0;
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
}
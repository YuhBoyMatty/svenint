// SvenInt (c) Sw1ft
// mov_selfsink.cpp

#include "stdafx.h"
#include "mov_selfsink.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Self sink state
//-----------------------------------------------------------------------------

enum ESelfSinkState
{
	kSelfSinkDucktap = 0,
	kSelfSinkHoldDuck,
	kSelfSinkUnduck,
	kSelfSinkPredictTouch
};

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSelfsink, selfsink, "Movement", "Selfsink" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_selfsink, "When falling on the ground automatically suicide to sink into the world" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSelfsink::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( localplayer->IsDead() )
	{
		Disable();
		m_iState = kSelfSinkDucktap;
		return kHookContinue;
	}

	if ( m_pMode->GetInt() == 0 )
	{
		switch ( m_iState )
		{
		case kSelfSinkDucktap:
		{
			if ( playermove->onground() != -1 )
			{
				cmd->buttons |= IN_DUCK;
				m_iState = kSelfSinkHoldDuck;
			}
			else
			{
				cmd->buttons |= IN_DUCK;
				m_iState = kSelfSinkPredictTouch;
			}

			m_iFrames = 0;
			break;
		}

		case kSelfSinkHoldDuck:
		{
			cmd->buttons |= IN_DUCK;

			if ( m_iFrames++ >= 5 )
			{
				m_iState = kSelfSinkUnduck;
				m_iFrames = 0;
			}

			break;
		}

		case kSelfSinkUnduck:
		{
			cmd->buttons &= ~IN_DUCK;

			if ( m_iFrames++ >= 5 )
				m_iState = kSelfSinkPredictTouch;

			break;
		}

		case kSelfSinkPredictTouch:
		{
			if ( playermove->onground() != -1 )
			{
				m_iState = kSelfSinkDucktap;
				Disable();
				break;
			}

			pmtrace_t tr;

			bool bPredictedOnGround = false;
			int oldhull = playermove->usehull();
			float flFrametime = playermove->frametime();
			Vector vecVelocity = *playermove->velocity();
			Vector vecOrigin = *playermove->origin();

			for ( int i = 0; i < 2; i++ )
			{
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

				UTIL_FixupGravityVelocity( vecVelocity, flFrametime );

				if ( bPredictedOnGround )
				{
					cl_enginefuncs->pfnClientCmd( "kill" );

					m_iState = kSelfSinkDucktap;
					Disable();

					break;
				}
			}

			cmd->buttons |= IN_DUCK;
			break;
		}
		}
	}
	else
	{
		cmd->buttons |= IN_DUCK;

		if ( playermove->view_ofs()->z == VEC_DUCK_VIEW.z )
		{
			cmd->buttons |= IN_JUMP;

			if ( playermove->onground() == -1 )
			{
				cl_enginefuncs->pfnClientCmd( "kill" );
				Disable();
			}
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSelfsink::CSelfsink( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pMode = NULL;

	m_iState = kSelfSinkDucktap;
	m_iFrames = 0;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSelfsink::OnEnable( void )
{
	m_iState = kSelfSinkDucktap;
	m_iFrames = 0;

	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSelfsink::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSelfsink::Load( void )
{
	Modules::menu->BindFeature( this );
	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Prediction\0 1 - Suicide on ground\0\0" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSelfsink::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_selfsink );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSelfsink::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_selfsink );
}
// SvenInt (c) Sw1ft
// mov_jumpbug.cpp

#include "stdafx.h"
#include "mov_jumpbug.h"
#include "player_speedhack.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CJumpbug, jumpbug, "Movement", "Jumpbug" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_jumpbug, "Cancels fall damage from high spots" );
FEATURE_CON_COMMAND_HOLD( sc_jumpbug, "" );

ConVar sc_jumpbug_mode( "sc_jumpbug_mode", "0", FCVAR_EXTDLL, "Modes:\n 0 - Speedhack\n 1 - Prediction" );
ConVar sc_jumpbug_min_fall_velocity( "sc_jumpbug_min_fall_velocity", "560", FCVAR_EXTDLL, "Minimal fall velocity to trigger auto jumpbug" );

//-----------------------------------------------------------------------------
// Predict jumpbug
//-----------------------------------------------------------------------------

void CJumpbug::Predict( bool &bInAir, bool &bDucking, Vector &vecOrigin, Vector &vecVelocity )
{
	pmtrace_t tr;
	int contents, placeHolder;

	// Predict jumpbug
	if ( !playermove->bInDuck() && !bDucking )
		return;

	Vector newOrigin = vecOrigin;

	playermove->setusehull( bDucking ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
	tr = playermove->funcs()->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1 );

	if ( tr.startsolid )
		return;

	playermove->setusehull( PM_HULL_PLAYER );
	tr = playermove->funcs()->PM_PlayerTrace( newOrigin, newOrigin, PM_NORMAL, -1 );

	if ( tr.startsolid )
		return;

	bDucking = false;

	// Check water. If we're under water, return here.

	Vector point = newOrigin;

	// Pick a spot just above the players feet.
	point[ 0 ] += ( bDucking ? ( VEC_DUCK_HULL_MIN[ 0 ] + VEC_DUCK_HULL_MAX[ 0 ] ) : ( VEC_HULL_MIN[ 0 ] + VEC_HULL_MAX[ 0 ] ) ) * 0.5f;
	point[ 1 ] += ( bDucking ? ( VEC_DUCK_HULL_MIN[ 1 ] + VEC_DUCK_HULL_MAX[ 1 ] ) : ( VEC_HULL_MIN[ 1 ] + VEC_HULL_MAX[ 1 ] ) ) * 0.5f;
	point[ 2 ] += ( ( bDucking ? VEC_DUCK_HULL_MIN[ 2 ] : VEC_HULL_MIN[ 2 ] ) + 1 );

	contents = playermove->funcs()->PM_PointContents( point, &placeHolder );

	if ( contents <= CONTENTS_WATER && contents > CONTENTS_TRANSLUCENT )
		return;
	
	// Check ground.
	if ( vecVelocity[ 2 ] > 180 )
		return;

	point = newOrigin;
	point[ 2 ] -= 2;

	playermove->setusehull( bDucking ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
	tr = playermove->funcs()->PM_PlayerTrace( newOrigin, point, PM_NORMAL, -1 );

	if ( !( tr.plane.normal[ 2 ] < 0.7 || tr.ent == -1 ) )
	{
		bInAir = false;
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CJumpbug::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsDead() )
		return kHookContinue;

	if ( playermove->flFallVelocity() <= sc_jumpbug_min_fall_velocity.GetFloat() ||
		 playermove->movevars() == NULL )
	{
		m_nJumpBugState = 0;
		return kHookContinue;
	}

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
	auto frametime = pEvent->GetArg<float>( "frametime" );

	if ( m_pMode->GetInt() == 0 )
	{
		int placeHolder, contents;

		Vector vecPredictVelocity = localplayer->GetVelocity() * frametime;

		vecPredictVelocity.z = 0.f; // 2D only, height will be predicted separately

		Vector vecPredictOrigin = *playermove->origin() + vecPredictVelocity;
		Vector vBottomOrigin = vecPredictOrigin;

		vBottomOrigin.z -= 8192.0f;

		pmtrace_t *pTrace = cl_enginefuncs->PM_TraceLine( vecPredictOrigin,
															vBottomOrigin,
															PM_TRACELINE_PHYSENTSONLY,
															( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER /* g_pPlayerMove->usehull */,
															-1 );

		float flHeight = fabsf( pTrace->endpos.z - vecPredictOrigin.z );
		float flGroundNormalAngle = acosf( pTrace->plane.normal.z );

		if ( flGroundNormalAngle > acosf( 0.7f ) || localplayer->GetWaterLevel() != WL_NOT_IN_WATER )
		{
			m_nJumpBugState = 0;
			return kHookContinue;
		}

		contents = playermove->funcs()->PM_PointContents( pTrace->endpos, &placeHolder );

		if ( contents <= CONTENTS_WATER && contents > CONTENTS_TRANSLUCENT )
		{
			m_nJumpBugState = 0;
			return kHookContinue;
		}

		// AddCorrectGravity
		float flFrameZDist = fabsf( ( -1.f * playermove->velocity()->z + ( playermove->movevars()->gravity * 0.5f * frametime ) ) * frametime );

		cmd->buttons |= IN_DUCK;
		cmd->buttons &= ~IN_JUMP;

		switch ( m_nJumpBugState )
		{
		case 1:
			cmd->buttons &= ~IN_DUCK;
			cmd->buttons |= IN_JUMP;

			m_nJumpBugState = 2;
			break;

		case 2:
			m_nJumpBugState = 0;
			break;

		default:
			if ( flFrameZDist > 0.f && fabsf( flHeight - flFrameZDist * 1.5f ) <= 20.f )
			{
				float flNeedSpeed = fabsf( flHeight - 19.f );
				float flScale = fabsf( flNeedSpeed / flFrameZDist );

				Features::speedhack->SetGameSpeed( flScale );

				m_nJumpBugState = 1;
			}
			break;
		}

		return kHookContinue;
	}

	// Prediction based jumpbug, credits to BXT contributors
	pmtrace_t tr;
	Vector vecOrigin, vecMove, vecVelocity, vecPredictOrigin, vecBottom;
	int placeHolder, contents;

	const int oldhull = playermove->usehull();

	vecOrigin = *playermove->origin();
	vecVelocity = *playermove->velocity();

	// Apply gravity
	UTIL_AddCorrectGravity( vecVelocity, frametime );

	// Step
	vecMove = vecVelocity * frametime;

	// Trace forward
	playermove->setusehull( ( playermove->flags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER);

	tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_NORMAL, -1 );

	vecPredictOrigin = tr.endpos;
	vecBottom = vecPredictOrigin - Vector( 0.f, 0.f, 8192.f );

	// Trace down
	tr = playermove->funcs()->PM_PlayerTrace( vecPredictOrigin, vecBottom, PM_NORMAL, -1 );

	playermove->setusehull( oldhull );

	if ( acos( tr.plane.normal.z ) > acos( 0.7f ) || playermove->waterlevel() != WL_NOT_IN_WATER )
		return kHookContinue;

	contents = playermove->funcs()->PM_PointContents( tr.endpos, &placeHolder );

	// In water, don't jumpbug
	if ( contents <= CONTENTS_WATER && contents > CONTENTS_TRANSLUCENT )
		return kHookContinue;

	bool bInAir = true;
	bool bDucking = ( playermove->flags() & FL_DUCKING );

	cmd->buttons |= IN_DUCK;
	cmd->buttons &= ~IN_JUMP;

	// Predict jumpbug
	Predict( bInAir, bDucking, *playermove->origin(), *playermove->velocity() );

	if ( !bInAir && !bDucking )
	{
		cmd->buttons |= IN_JUMP;
		cmd->buttons &= ~IN_DUCK;
	}

	playermove->setusehull( oldhull );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CJumpbug::CJumpbug( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pMode = NULL;
	m_pMinFallVelocity = NULL;

	m_nJumpBugState = 0;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CJumpbug::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall, kHookPriorityLow );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CJumpbug::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CJumpbug::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Speedhack\0 1 - Prediction\0\0" );
	m_pMinFallVelocity = Modules::menu->AddParamFloat( this, "MinFallVelocity", NULL, 560.f, 1.f, 1000.f );

	Modules::menu->BindConVar( m_pMode, &sc_jumpbug_mode );
	Modules::menu->BindConVar( m_pMinFallVelocity, &sc_jumpbug_min_fall_velocity );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CJumpbug::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_jumpbug );
	FEATURE_REGISTER_CCMD( sc_jumpbug_down );
	FEATURE_REGISTER_CCMD( sc_jumpbug_up );
	FEATURE_REGISTER_CVAR( sc_jumpbug_mode );
	FEATURE_REGISTER_CVAR( sc_jumpbug_min_fall_velocity );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CJumpbug::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_jumpbug );
	FEATURE_UNREGISTER_CCMD( sc_jumpbug_down );
	FEATURE_UNREGISTER_CCMD( sc_jumpbug_up );
	FEATURE_UNREGISTER_CVAR( sc_jumpbug_mode );
	FEATURE_UNREGISTER_CVAR( sc_jumpbug_min_fall_velocity );
}
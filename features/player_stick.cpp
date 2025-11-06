// SvenInt (c) Sw1ft
// player_stick.cpp

#include "stdafx.h"
#include "player_stick.h"
#include "mov_edgejump.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CStick, stick, "Player", "Stick" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_stick, "Stick to a player" );

ConVar sc_stick_target( "sc_stick_target", "0", FCVAR_EXTDLL, "Player's index to stick" );

//-----------------------------------------------------------------------------
// Find a target
//-----------------------------------------------------------------------------

cl_entity_t *CStick::FindTarget( void )
{
	cl_entity_t *pTarget = NULL;
	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

	float flDistanceSqr = FLT_MAX;

	for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
	{
		cl_entity_t *pPlayer = cl_enginefuncs->GetEntityByIndex( i );

		if ( pPlayer != NULL
			 && pPlayer != pLocal
			 && pPlayer->curstate.messagenum >= pLocal->curstate.messagenum &&
			 extraplayerinfo->GetHealth( i ) != 0.f &&
			 ( pPlayer->curstate.sequence < 12 ||
			 pPlayer->curstate.sequence > 18 ) )
		{
			float dist_sqr = ( pLocal->curstate.origin - pPlayer->curstate.origin ).LengthSqr();

			if ( dist_sqr < flDistanceSqr )
			{
				pTarget = pPlayer;
				flDistanceSqr = dist_sqr;
			}
		}
	}

	return pTarget;
}

//-----------------------------------------------------------------------------
// Idle
//-----------------------------------------------------------------------------

void CStick::Idle( usercmd_t *cmd )
{
	if ( m_pAuto->GetBool() &&
		 sc_stick_target.GetInt() == 0 &&
		 !( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
	{
		if ( localplayer->GetWaterLevel() == WL_EYES )
			cmd->upmove = localplayer->GetMaxSpeed();
		if ( localplayer->GetMoveType() == MOVETYPE_FLY )
			cmd->buttons |= IN_JUMP;

		static bool forward_step = true;

		cmd->forwardmove = forward_step ? 50.0f : -50.0f;

		forward_step = !forward_step;
	}
}

//-----------------------------------------------------------------------------
// Try to mimic. TODO: optimize
//-----------------------------------------------------------------------------

void CStick::TryMimic( cl_entity_t *pPlayer, usercmd_t *cmd )
{
	if ( pPlayer->curstate.usehull )
		cmd->buttons |= IN_DUCK;

	if ( pPlayer->curstate.sequence == 8 )
	{
		static bool jump = true;
		if ( jump )
			cmd->buttons |= IN_JUMP;
		jump = !jump;
	}

	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
	if ( pLocal == NULL || pPlayer->curstate.weaponmodel == 0 || pLocal->curstate.weaponmodel == 0 )
		return;

	if ( pPlayer->curstate.weaponmodel == cl_enginefuncs->GetLocalPlayer()->curstate.weaponmodel )
	{
		// Fire weapon
		if ( pPlayer->curstate.sequence == 43 || pPlayer->curstate.sequence == 46 ) // medkit
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 21 /* || pPlayer->curstate.sequence == 24 */ ) // crowbar / wrench
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 25 || pPlayer->curstate.sequence == 28 ) // wrench
		{
			cmd->buttons |= IN_ATTACK2;
		}
		else if ( pPlayer->curstate.sequence == 49 || pPlayer->curstate.sequence == 53 ) // glock
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 57 || pPlayer->curstate.sequence == 61 ) // .357 / deagle
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 132 || pPlayer->curstate.sequence == 140 ) // akimbo usiz
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 79 || pPlayer->curstate.sequence == 83 ) // mp5
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 65 || pPlayer->curstate.sequence == 69 ) // shotgun
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 113 || pPlayer->curstate.sequence == 119 ) // xbow
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 147 || pPlayer->curstate.sequence == 153 ) // m4
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 148 || pPlayer->curstate.sequence == 154 ) // m4 ar grenade
		{
			cmd->buttons |= IN_ATTACK2;
		}
		else if ( pPlayer->curstate.sequence == 87 || pPlayer->curstate.sequence == 91 ) // rpg
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 73 || pPlayer->curstate.sequence == 76 ) // gauss
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 107 || pPlayer->curstate.sequence == 110 ) // hornet gun
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 33 || pPlayer->curstate.sequence == 34 || pPlayer->curstate.sequence == 38 ) // hand grenade
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 43 || pPlayer->curstate.sequence == 46 ) // trip mine
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 101 || pPlayer->curstate.sequence == 104 ) // snark
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 159 || pPlayer->curstate.sequence == 165 ) // sniper rifle
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 171 || pPlayer->curstate.sequence == 175 ) // m249
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( pPlayer->curstate.sequence == 87 || pPlayer->curstate.sequence == 91 ) // spore launcher
		{
			cmd->buttons |= IN_ATTACK;
		}
		else if ( /* pPlayer->curstate.sequence == 94 || */ pPlayer->curstate.sequence == 97 ) // displacer gun
		{
			cmd->buttons |= IN_ATTACK;
		}

		return;
	}

	model_t *pWeaponModel = enginestudio->GetModelByIndex( pPlayer->curstate.weaponmodel );
	if ( pWeaponModel == NULL )
		return;

	for ( int i = 0; i < inventory->GetMaxWeaponSlots(); i++ )
	{
		for ( int j = 0; j < inventory->GetMaxWeaponPositions(); j++ )
		{
			WEAPON *pWeapon = inventory->GetWeapon( i, j );
			if ( pWeapon == NULL || !inventory->HasAmmo( pWeapon ) )
				continue;

			const char *buffer = pWeapon->szName;
			while ( *buffer != '\0' )
			{
				buffer++;

				if ( *buffer == '_' )
				{
					buffer++;
					break;
				}
			}

			if ( *buffer == '\0' )
				buffer = pWeapon->szName;

			if ( strstr( pWeaponModel->name, buffer ) )
			{
				inventory->SelectWeapon( pWeapon );
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// StealModel
//-----------------------------------------------------------------------------

void CStick::StealModel( void )
{
	player_info_t *pPlayerInfo = enginestudio->PlayerInfo( sc_stick_target.GetInt() - 1 );
	if ( pPlayerInfo != NULL )
	{
		static cvar_t *model = NULL;
		static cvar_t *topcolor = NULL;
		static cvar_t *bottomcolor = NULL;

		if ( model == NULL )
		{
			model = cvar->FindCvar( "model" );
			topcolor = cvar->FindCvar( "topcolor" );
			bottomcolor = cvar->FindCvar( "bottomcolor" );
		}

		if ( model != NULL && stricmp( pPlayerInfo->model, model->string ) )
		{
			cvar->SetValue( model, pPlayerInfo->model );
		}

		if ( topcolor != NULL && pPlayerInfo->topcolor != (int)topcolor->value )
		{
			cvar->SetValue( topcolor, pPlayerInfo->topcolor );
		}

		if ( bottomcolor != NULL && pPlayerInfo->bottomcolor != (int)bottomcolor->value )
		{
			cvar->SetValue( bottomcolor, pPlayerInfo->bottomcolor );
		}
	}
}

//-----------------------------------------------------------------------------
// SetViewAngles
//-----------------------------------------------------------------------------

void CStick::SetViewAngles( cl_entity_t *pPlayer, usercmd_t *cmd )
{
	if ( m_pLookAtTarget->GetBool() )
	{
		Vector dir;
		Vector vecAngles;

		Vector vecSrc = localplayer->GetEyePosition();
		Vector vecTarget = pPlayer->curstate.origin + Vector( 0.f, 0.f, pPlayer->curstate.usehull ? 12.5f : 28.5f );

		VectorSubtract( vecTarget, vecSrc, dir );

		vecAngles.x = VEC_RAD2DEG( -atan2f( dir.z, dir.Length2D() ) );
		vecAngles.y = VEC_RAD2DEG( atan2f( dir.y, dir.x ) );
		vecAngles.z = 0.f;

		NormalizeAngles( vecAngles );

		cmd->viewangles = vecAngles;
		
		m_bForcePitch = true;
		m_flSavedPitchAngle = NormalizeAngle( cmd->viewangles.x ) / -3.0f;
	}
	else if ( m_pMimic->GetBool() )
	{
		gameutils->GetViewAngles( pPlayer, cmd->viewangles );

		m_bForcePitch = true;
		m_flSavedPitchAngle = NormalizeAngle( cmd->viewangles.x ) / -3.0f;
	}
}

//-----------------------------------------------------------------------------
// TryMoveOnLadder
//-----------------------------------------------------------------------------

bool CStick::TryMoveOnLadder( cl_entity_t *pPlayer, usercmd_t *cmd )
{
	physent_t *pe;
	hull_t *hull;
	int num;
	Vector test;
	physent_t *pLadder = NULL;

	if ( !m_pOvercomeObstacles->GetBool() || localplayer->GetMoveType() != MOVETYPE_FLY )
	{
		m_iClimb = 0;
		return false;
	}

	for ( int i = 0; i < playermove->nummoveent(); i++ )
	{
		pe = &playermove->moveents()[ i ];

		if ( pe->model && (modtype_t)playermove->funcs()->PM_GetModelType( pe->model ) == mod_brush && pe->skin == CONTENTS_LADDER )
		{
			hull = (hull_t *)playermove->funcs()->PM_HullForBsp( pe, test );
			num = hull->firstclipnode;

			// Offset the test point appropriately for this hull.
			VectorSubtract( *playermove->origin(), test, test );

			// Test the player's hull for intersection with this model
			//if ( playermove->funcs()->PM_HullPointContents( hull, num, test ) == CONTENTS_EMPTY )
			//	continue;

			// Assume the ladder we're climbing on is the latest touched physent
			pLadder = pe;
			//break;
		}
	}

	if ( pLadder == NULL )
	{
		cmd->buttons |= IN_JUMP;
		m_iClimb = 0;
		return false;
	}

	trace_t trace;

	Vector vecAngles;
	Vector ladderCenter;
	Vector modelmins, modelmaxs;

	if ( m_iClimb == 0 )
	{
		if ( pPlayer->curstate.origin.z > playermove->origin()->z )
		{
			// up
			m_iClimb = 1;
		}
		else
		{
			// down
			m_iClimb = 2;
		}
	}

	playermove->funcs()->PM_GetModelBounds( pLadder->model, modelmins, modelmaxs );

	VectorAdd( modelmins, modelmaxs, ladderCenter );
	VectorScale( ladderCenter, 0.5, ladderCenter );

	playermove->funcs()->PM_TraceModel( pLadder, *playermove->origin(), ladderCenter, &trace );

	vecAngles.x = 89.f;
	vecAngles.y = VEC_RAD2DEG( atan2f( trace.plane.normal.y, trace.plane.normal.x ) );
	vecAngles.z = 0.f;

	vecAngles.y -= 90.f;

	cmd->viewangles = vecAngles;

	m_bForcePitch = true;
	m_flSavedPitchAngle = NormalizeAngle( cmd->viewangles.x ) / -3.0f;

	cmd->forwardmove = 0.f;
	cmd->sidemove = 0.f;
	cmd->upmove = 0.f;

	if ( m_iClimb == 1 )
	{
		if ( pPlayer->curstate.origin.z + 2.f <= playermove->origin()->z )
		{
			cmd->buttons |= IN_JUMP;
		}
		else
		{
			cmd->forwardmove = -playermove->maxspeed();
			cmd->sidemove = playermove->maxspeed();

			cmd->buttons |= ( IN_BACK | IN_MOVERIGHT );
		}
	}
	else
	{
		if ( pPlayer->curstate.origin.z >= playermove->origin()->z )
		{
			cmd->buttons |= IN_JUMP;
		}
		else
		{
			cmd->forwardmove = playermove->maxspeed();
			cmd->sidemove = -playermove->maxspeed();

			cmd->buttons |= ( IN_FORWARD | IN_MOVELEFT );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// TryMove
//-----------------------------------------------------------------------------

void CStick::TryMove( cl_entity_t *pPlayer, usercmd_t *cmd, Vector &vecPredictPos, Vector2D &vecDir )
{
	if ( !( m_pStrafeMode->GetInt() == 1 && !localplayer->IsOnGround() || m_pStrafeMode->GetInt() == 2 ) )
	{
		Vector2D vecForward;
		Vector2D vecRight;

		vecForward.x = cosf( cmd->viewangles.y * static_cast<float>( M_PI / 180.0 ) );
		vecForward.y = sinf( cmd->viewangles.y * static_cast<float>( M_PI / 180.0 ) );

		vecRight.x = vecForward.y;
		vecRight.y = -vecForward.x;

		vecForward *= localplayer->GetMaxSpeed();
		vecRight *= localplayer->GetMaxSpeed();

		float forwardmove = DotProduct( vecForward, vecDir );
		float sidemove = DotProduct( vecRight, vecDir );

		cmd->forwardmove = forwardmove;
		cmd->sidemove = sidemove;
	}
	else
	{
		extern bool g_bStrafedRight;
		static bool s_bLastStrafedRight = g_bStrafedRight;
		static bool s_bFlip = false;
		static bool s_bSkipFlip = false;

		//Vector va;
		//cl_enginefuncs->GetViewAngles( va );

		Features::strafer->UpdateStrafeData( m_strafeData,
											 true,
											 Strafe::StrafeDir::POINT,
											 Strafe::StrafeType::MAXACCEL,
											 cmd->viewangles.y,
											 vecPredictPos.x,
											 vecPredictPos.y );

		Strafe::ProcessedFrame out;
		out.Yaw = cmd->viewangles.y;

		Strafe::Friction( m_strafeData );
		Strafe::StrafeVectorial( m_strafeData, out, false );

		if ( out.Processed )
		{
			bool bOldLastStrafedRight = s_bLastStrafedRight;
			s_bLastStrafedRight = g_bStrafedRight;

			if ( Features::strafer->IsBypassEnabled() &&
				 s_bSkipFlip &&
				 bOldLastStrafedRight != g_bStrafedRight )
			{
				s_bSkipFlip = false;
			}
			else
			{
				s_bSkipFlip = true;

				cmd->forwardmove = out.Forwardspeed;
				cmd->sidemove = out.Sidespeed;

				if ( bOldLastStrafedRight == g_bStrafedRight )
					s_bFlip = false;
				else
					s_bFlip = !s_bFlip;
			}
		}

	}

	// Jump when too far
	if ( m_pStrafeMode->GetInt() > 0 && localplayer->IsOnGround() &&
		 ( vecPredictPos.AsVector2D() - localplayer->GetOrigin().AsVector2D() ).LengthSqr() > M_SQR( 300.f ) )
	{
		cmd->buttons |= IN_JUMP;
	}
}

//-----------------------------------------------------------------------------
// TryUnstuck
//-----------------------------------------------------------------------------

bool CStick::TryUnstuck( usercmd_t *cmd, Vector2D &vecDir )
{
	if ( !m_pOvercomeObstacles->GetBool() )
		return false;

	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( ( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( localplayer->GetOrigin(), localplayer->GetOrigin() + vecDir.ToVector() * 4.f, PM_WORLD_ONLY, -1, &trace );

	if ( trace.fraction == 1.f )
		return false;

	static bool bTryUnstuck = false;

	Vector vecUp = localplayer->GetOrigin() + Vector( 0, 0, 12 );

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( ( localplayer->GetFlags() & FL_DUCKING ) ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecUp, vecUp + vecDir.ToVector() * 4.f, PM_WORLD_ONLY, -1, &trace );

	if ( trace.fraction == 1.f )
		return false;

	bTryUnstuck = !bTryUnstuck;

	if ( bTryUnstuck )
	{
		cmd->buttons |= ( IN_JUMP | IN_DUCK );
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CStick::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
		if ( pLocal == NULL || !m_bForcePitch )
			return kHookContinue;

		pLocal->angles.x = m_flSavedPitchAngle;
		pLocal->curstate.angles.x = m_flSavedPitchAngle;
		pLocal->prevstate.angles.x = m_flSavedPitchAngle;
		pLocal->latched.prevangles.x = m_flSavedPitchAngle;
		m_bForcePitch = false;

		return kHookHandled;
	}

	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( m_pAuto->GetBool() && ( localplayer->IsDying() || localplayer->IsSpectating() ) )
	{
		cmd->buttons |= IN_ATTACK;
	}

	if ( localplayer->IsDead() )
	{
		if ( sc_stick_target.GetInt() != 0 )
			sc_stick_target.SetValue( 0 );

		m_iClimb = 0;
		return kHookContinue;
	}

	if ( m_pAuto->GetBool() )
	{
		int iPrevTarget = sc_stick_target.GetInt();
		cl_entity_t *pTarget = FindTarget();

		if ( pTarget != NULL &&
			 ( cl_enginefuncs->GetLocalPlayer()->curstate.origin - pTarget->curstate.origin ).LengthSqr() <= M_SQR( 512.f ) )
		{
			if ( cl_enginefuncs->GetAbsoluteTime() - m_flSwitchTargetTime > 0.5 )
			{
				if ( iPrevTarget != pTarget->index )
				{
					m_iClimb = 0;
				}

				sc_stick_target.SetValue( pTarget->index );
				m_flSwitchTargetTime = cl_enginefuncs->GetAbsoluteTime();
			}
		}
		else
		{
			sc_stick_target.SetValue( 0 );
		}
	}

	if ( sc_stick_target.GetInt() != 0 )
	{
		cl_entity_t *pPlayer = cl_enginefuncs->GetEntityByIndex( sc_stick_target.GetInt() );

		if ( pPlayer == NULL ||
			 pPlayer->curstate.messagenum < cl_enginefuncs->GetLocalPlayer()->curstate.messagenum )
		{
			sc_stick_target.SetValue( 0 );
			Idle( cmd );
			return kHookContinue;
		}

		Vector vecPredictPos = pPlayer->curstate.origin + ( pPlayer->curstate.origin - pPlayer->prevstate.origin );

		Vector2D vecDir = vecPredictPos.AsVector2D() - playermove->origin()->AsVector2D();
		vecDir.NormalizeInPlace();

		SetViewAngles( pPlayer, cmd );

		if ( m_pStealModel->GetBool() )
		{
			StealModel();
		}

		bool bMovedOnLadder = TryMoveOnLadder( pPlayer, cmd );

		if ( !bMovedOnLadder && !TryUnstuck( cmd, vecDir ) )
		{
			if ( m_pEdgejump->GetBool() )
				Features::edgejump->Execute( cmd, playermove->frametime() );

			if ( m_pMimic->GetBool() )
				TryMimic( pPlayer, cmd );
		}

		if ( !bMovedOnLadder )
		{
			TryMove( pPlayer, cmd, vecPredictPos, vecDir );
		}

		if ( localplayer->GetWaterLevel() > WL_NOT_IN_WATER &&
				pPlayer->curstate.origin.z >= playermove->origin()->z )
		{
			if ( localplayer->GetFlags() & FL_WATERJUMP )
				cmd->buttons |= IN_DUCK;

			cmd->upmove = localplayer->GetMaxSpeed();
		}
	}

	Idle( cmd );
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CStick::CStick( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pAuto = NULL;
	m_pStealModel = NULL;
	m_pLookAtTarget = NULL;
	m_pOvercomeObstacles = NULL;
	m_pEdgejump = NULL;
	m_pMimic = NULL;
	m_pStrafeMode = NULL;

	m_iClimb = 0;
	m_flSwitchTargetTime = 0.0;

	m_bForcePitch = false;
	m_flSavedPitchAngle = 0.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CStick::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CStick::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CStick::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pAuto = Modules::menu->AddParamBool( this, "Auto", NULL, true );
	m_pStealModel = Modules::menu->AddParamBool( this, "StealModel", NULL, false );
	m_pLookAtTarget = Modules::menu->AddParamBool( this, "LookAtTarget", NULL, false );
	m_pOvercomeObstacles = Modules::menu->AddParamBool( this, "OvercomeObstacles", NULL, true );
	m_pEdgejump = Modules::menu->AddParamBool( this, "Edgejump", NULL, true );
	m_pMimic = Modules::menu->AddParamBool( this, "Mimic", NULL, false );
	m_pStrafeMode = Modules::menu->AddParamList( this, "StrafeMode", NULL, 1, " 0 - None\0 1 - In Air\0 2 - Everytime\0\0" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CStick::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_stick );
	FEATURE_REGISTER_CVAR( sc_stick_target );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CStick::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_stick );
	FEATURE_UNREGISTER_CVAR( sc_stick_target );
}
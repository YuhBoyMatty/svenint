// SvenInt (c) Sw1ft
// mov_strafer.cpp

#include "stdafx.h"
#include "mov_strafer.h"
#include "mov_autojump.h"
#include "player_antiafk.h"
#include "player_stick.h"
#include "player_silent_angles.h"

using namespace Globals;
extern bool gbStrafedRight;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CStrafer, strafer, "Movement", "Strafer" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_strafe, "BunnymodXT's Strafer" );

ConVar sc_strafe_point_x( "sc_strafe_point_x", "0", FCVAR_EXTDLL, "Coordinate X to point strafe" );
ConVar sc_strafe_point_y( "sc_strafe_point_y", "0", FCVAR_EXTDLL, "Coordinate Y to point strafe" );
ConVar sc_strafe_buttons( "sc_strafe_buttons", "2 6 2 6", FCVAR_EXTDLL, "Sets the strafing buttons. The format is 4 digits: \"<AirLeft> <AirRight> <GroundLeft> <GroundRight>\". The default (auto-detect) is empty string: \"\".\nTable of buttons:\n\t0 - W\n\t1 - WA\n\t2 - A\n\t3 - SA\n\t4 - S\n\t5 - SD\n\t6 - D\n\t7 - WD\n" );
ConVar sc_strafe_ignore_ground( "sc_strafe_ignore_ground", "1", FCVAR_EXTDLL, "Don't strafe when on ground" );
ConVar sc_strafe_dir( "sc_strafe_dir", "3", FCVAR_EXTDLL, "Strafing direction. Directions:\n\t0 - to the left\n\t1 - to the right\n\t2 - best strafe\n\t3 - to view angles\n\t4 - to the point", true, 0.f, true, 4.f );
ConVar sc_strafe_type( "sc_strafe_type", "0", FCVAR_EXTDLL, "Strafing type. Types:\n\t0 - Max acceleration strafing\n\t1 - Max angle strafing\n\t2 - Max deceleration strafing\n\t3 - Const speed strafing", true, 0.f, true, 3.f );
ConVar sc_strafe_yaw( "sc_strafe_yaw", "", FCVAR_EXTDLL, "Strafe yaw" );
ConVar sc_strafe_vectorial( "sc_strafe_vectorial", "1", FCVAR_EXTDLL, "Strafe vectorial" );
ConVar sc_strafe_vectorial_increment( "sc_strafe_vectorial_increment", "0", FCVAR_EXTDLL, "Determines how fast the player yaw angle moves towards the target yaw angle. 0 for no movement, 180 for instant snapping. Has no effect on strafing speed" );
ConVar sc_strafe_vectorial_increment_invert( "sc_strafe_vectorial_increment_invert", "0", FCVAR_EXTDLL, "Invert direction yaw angle move towards the target yaw angle" );
ConVar sc_strafe_vectorial_offset( "sc_strafe_vectorial_offset", "0", FCVAR_EXTDLL, "Determines the target view angle offset from tas_strafe_yaw" );
ConVar sc_strafe_vectorial_snap( "sc_strafe_vectorial_snap", "170", FCVAR_EXTDLL, "Determines when the yaw angle snaps to the target yaw. Mainly used to prevent ABHing from resetting the yaw angle to the back on every jump" );

//-----------------------------------------------------------------------------
// Cvar change hooks
//-----------------------------------------------------------------------------

static void CvarChangeHook_sc_strafe_ignore_ground( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	gameutils->PrintChatText( "<SvenInt> Strafe on ground is %s\n", !( pCvar->value ) ? "ON" : "OFF" );
}

static void CvarChangeHook_sc_strafe_vectorial( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	gameutils->PrintChatText( "<SvenInt> Vectorial Strafer is %s\n", !!( pCvar->value ) ? "ON" : "OFF" );
}

static void CvarChangeHook_sc_strafe_dir( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	// The value clamped by a menu element to which was binded that cvar
	const char *pszDir = NULL;

	switch ( (int)pCvar->value )
	{
	case 0:
		pszDir = "To the left";
		break;

	case 1:
		pszDir = "To the right";
		break;

	case 2:
		pszDir = "Best strafe";
		break;

	case 3:
		pszDir = "To yaw";
		break;

	case 4:
		pszDir = "To the point";
		break;

	default:
		pszDir = "N/A";
		break;
	}

	gameutils->PrintChatText( "<SvenInt> Strafe Direction is %s\n", pszDir );
}

static void CvarChangeHook_sc_strafe_type( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	// The value clamped by a menu element to which was binded that cvar
	const char *pszType = NULL;

	switch ( (int)pCvar->value )
	{
	case 0:
		pszType = "Max Acceleration";
		break;

	case 1:
		pszType = "Max Angle";
		break;

	case 2:
		pszType = "Max Deceleration";
		break;

	case 3:
		pszType = "Const Speed";
		break;

	default:
		pszType = "N/A";
		break;
	}

	gameutils->PrintChatText( "<SvenInt> Strafe Type is %s\n", pszType );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

Strafe::Frame &CStrafer::GetStrafeFrame( usercmd_t *cmd, float frametime, bool bStrafe, bool bStrafeVectorial, Strafe::StrafeDir dir, Strafe::StrafeType type, float flYaw, float flPointX, float flPointY )
{
	const char *pszValueSLJ = Globals::cl_enginefuncs->PhysInfo_ValueForKey( "slj" );
	const bool bCanSuperJump = ( pszValueSLJ != NULL && *pszValueSLJ == '1' );

	m_strafeFrame.player.WaterLevel = playermove->waterlevel();
	m_strafeFrame.player.WaterType = playermove->watertype();

	m_strafeFrame.player.WaterJumpTime = (short)playermove->waterjumptime();
	m_strafeFrame.player.DuckTime = (short)playermove->flDuckTime();

	if ( cmd != NULL )
	{
		m_strafeFrame.player.Jump = !!( cmd->buttons & IN_JUMP );
		m_strafeFrame.player.Duck = !!( cmd->buttons & IN_DUCK );
		m_strafeFrame.player.Use = !!( cmd->buttons & IN_USE );
		m_strafeFrame.player.Walk = false;
	}

	m_strafeFrame.player.HasLongJumpModule = bCanSuperJump;
	m_strafeFrame.player.ReduceWishspeed = !!( playermove->flags() & FL_DUCKING );
	m_strafeFrame.player.Ducking = playermove->flags() & FL_DUCKING;
	m_strafeFrame.player.InDuck = playermove->bInDuck();
	m_strafeFrame.player.WaterJump = playermove->flags() & FL_WATERJUMP;

	m_strafeFrame.player.vecVelocity = *playermove->velocity();
	m_strafeFrame.player.vecOrigin = *playermove->origin();
	m_strafeFrame.player.vecBaseVelocity = *playermove->basevelocity();

	m_strafeFrame.player.PosType = ( playermove->onground() != -1 ? Strafe::PositionType::GROUND : Strafe::PositionType::AIR );
	m_strafeFrame.player.ViewHeight = playermove->view_ofs()->z;
	m_strafeFrame.player.EntGravity = playermove->gravity();
	m_strafeFrame.player.EntFriction = playermove->friction();
	m_strafeFrame.player.ClientMaxspeed = playermove->clientmaxspeed();

	const movevars_t *mv = playermove->movevars();
	if ( mv != NULL )
	{
		m_strafeFrame.vars.Gravity = mv->gravity;
		m_strafeFrame.vars.Maxvelocity = mv->maxvelocity;
		m_strafeFrame.vars.Maxspeed = mv->maxspeed;
		m_strafeFrame.vars.Stopspeed = mv->stopspeed;
		m_strafeFrame.vars.Stepsize = mv->stepsize;
		m_strafeFrame.vars.Bounce = mv->bounce;
		m_strafeFrame.vars.Friction = mv->friction;
		m_strafeFrame.vars.Edgefriction = mv->edgefriction;
		m_strafeFrame.vars.Accelerate = mv->accelerate;
		m_strafeFrame.vars.Airaccelerate = mv->airaccelerate;
	}
	else
	{
		m_strafeFrame.vars.Gravity = 800.f;
		m_strafeFrame.vars.Maxvelocity = 4096.f;
		m_strafeFrame.vars.Maxspeed = 320.f;
		m_strafeFrame.vars.Stopspeed = 100.f;
		m_strafeFrame.vars.Stepsize = 18.f;
		m_strafeFrame.vars.Bounce = 1.f;
		m_strafeFrame.vars.Friction = 4.f;
		m_strafeFrame.vars.Edgefriction = 2.f;
		m_strafeFrame.vars.Accelerate = 10.f;
		m_strafeFrame.vars.Airaccelerate = 10.f;
	}

	//m_strafeFrame.vars.Frametime = 1.f / CVar()->FindCvar("fps_max")->value;
	//m_strafeFrame.vars.Frametime = playermove->frametime(); // 1.0f / 200.0f (1.0f / fps_max)
	m_strafeFrame.vars.Frametime = frametime; // 1.0f / 200.0f (1.0f / fps_max)

	m_strafeFrame.Strafe = bStrafe;
	m_strafeFrame.StrafeVectorial = bStrafeVectorial;
	m_strafeFrame.AutoJump = false;
	m_strafeFrame.SetDir( dir );
	m_strafeFrame.SetType( type );

	m_strafeFrame.UseGivenButtons = false;

	m_strafeFrame.SetX( flPointX );
	m_strafeFrame.SetY( flPointY );

	m_strafeFrame.SetYaw( static_cast<double>( flYaw ) );

	m_strafeFrame.VectorialIncrement = sc_strafe_vectorial_increment.GetFloat();
	m_strafeFrame.VectorialIncrementInvert = sc_strafe_vectorial_increment_invert.GetBool();
	m_strafeFrame.VectorialOffset = sc_strafe_vectorial_offset.GetFloat();
	m_strafeFrame.VectorialSnap = sc_strafe_vectorial_snap.GetFloat();

	return m_strafeFrame;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CStrafer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	m_bStrafed = false;

	// CL_CreateMove post event
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	const bool bStrafeTowardsMovementButtons = m_pStrafeTowardsMovementButtons->GetBool();
	const bool bPressedAnyMovementButton = ( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) );

	if ( bStrafeTowardsMovementButtons ^ bPressedAnyMovementButton )
		return kHookContinue;

	if ( Features::antiafk->IsEnabled() ||
		 Features::stick->IsEnabled() ||
		 localplayer->IsDead() ||
		 playermove->movetype() != MOVETYPE_WALK ||
		 playermove->waterlevel() > WL_FEET )
	{
		return kHookContinue;
	}

	Vector va;
	float targetYaw;

	Strafe::StrafeDir strafeDir = static_cast<Strafe::StrafeDir>( sc_strafe_dir.GetInt() );
	Strafe::StrafeType strafeType = static_cast<Strafe::StrafeType>( sc_strafe_type.GetInt() );

	cl_enginefuncs->GetViewAngles( va );

	NormalizeAngles( va );

	if ( sc_strafe_yaw.GetString()[ 0 ] == '\0' )
	{
		if ( sc_strafe_vectorial.GetBool() )
		{
			targetYaw = va[ 1 ];

			if ( bStrafeTowardsMovementButtons )
			{
				Vector2D vecMove;

				if ( cmd->buttons & IN_FORWARD )
					vecMove.x += 1.f;
				if ( cmd->buttons & IN_BACK )
					vecMove.x += -1.f;
				if ( cmd->buttons & IN_MOVELEFT )
					vecMove.y += 1.f;
				if ( cmd->buttons & IN_MOVERIGHT )
					vecMove.y += -1.f;

				if ( vecMove.x == 0.f && vecMove.y == 0.f || ( cmd->buttons & IN_BACK ) && m_pStopWhenHoldingBackButton->GetBool() )
				{
					strafeType = Strafe::StrafeType::MAXDECCEL;
				}
				else
				{
					targetYaw = NormalizeAngle( targetYaw + VEC_RAD2DEG( atan2f( vecMove.y, vecMove.x ) ) );
				}
			}
		}
		else
		{
			targetYaw = 0.f;
		}
	}
	else
	{
		targetYaw = sc_strafe_yaw.GetFloat();
	}

	GetStrafeFrame( cmd,
					pEvent->GetArg<float>( "frametime" ),
					true,
					sc_strafe_vectorial.GetBool(),
					strafeDir,
					strafeType,
					targetYaw,
					sc_strafe_point_x.GetFloat(),
					sc_strafe_point_y.GetFloat() );

	if ( !m_strafeFrame.Strafe )
		return kHookContinue;
	
	Strafe::ProcessedFrame out;
	out.Yaw = va[ 1 ];
	const bool bWasStandingOnGround = ( m_strafeFrame.player.PosType == Strafe::PositionType::GROUND );

	// L4DST kicks in
	if ( Features::autojump->IsEnabled() && cmd->buttons & IN_JUMP )
	{
		m_strafeFrame.AutoJump = true;
		PredictJump( m_strafeFrame );
	}
	
	if ( sc_strafe_ignore_ground.GetBool() && m_strafeFrame.player.PosType == Strafe::PositionType::GROUND )
		return kHookContinue;

	Strafe::Friction( m_strafeFrame );

	if ( m_strafeFrame.StrafeVectorial )
	{
		Strafe::StrafeVectorial( m_strafeFrame, out, false /* gbYawChanged */ );
	}
	else if ( true /* !gbYawChanged */ )
	{
		auto btns = Strafe::StrafeButtons();
		bool usingButtons = sscanf( sc_strafe_buttons.GetString(), "%hhu %hhu %hhu %hhu", &btns.AirLeft, &btns.AirRight, &btns.GroundLeft, &btns.GroundRight );

		m_strafeFrame.UseGivenButtons = usingButtons;
		m_strafeFrame.buttons = btns;

		Strafe::Strafe( m_strafeFrame, out );
	}

	if ( !out.Processed )
		return kHookContinue;
	
	const bool bLastStrafedRightOld = m_bLastStrafedRight;
	m_bLastStrafedRight = gbStrafedRight;

	if ( BypassAntiStrafer( cmd, out, va, bLastStrafedRightOld, bWasStandingOnGround ) )
		return kHookContinue; // Skip flip

	m_bSkipFlip = true;
	m_bStrafed = true;

	cmd->forwardmove = out.Forwardspeed;
	cmd->sidemove = out.Sidespeed;
	cmd->viewangles[ 1 ] = va[ 1 ] = out.Yaw;

	if ( bLastStrafedRightOld == gbStrafedRight )
		m_bFlip = false;
	else
		m_bFlip = !m_bFlip;

	cl_enginefuncs->SetViewAngles( va );
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// BypassAntiStrafer
//-----------------------------------------------------------------------------

bool CStrafer::BypassAntiStrafer( usercmd_t *cmd, Strafe::ProcessedFrame out, float *va, const bool bLastStrafedRightOld, const bool bWasStandingOnGround )
{
	if ( !m_pBypassAntiStrafer->GetBool() )
		return false;
	
	// Mimic human inputs
	if ( m_pBypassMode->GetInt() == 0 )
	{
		if ( bLastStrafedRightOld != gbStrafedRight )
		{
			// L4DST kicks in.. AGAIN
			float thetaMove = atan2f( out.Sidespeed, out.Forwardspeed );
			//float flSpeed = sqrtf( out.Sidespeed * out.Sidespeed + out.Forwardspeed * out.Forwardspeed );
			float moveDirDeg = NormalizeAngle( va[ 1 ] - VEC_RAD2DEG( thetaMove ) );

			/*
			if ( g_bStrafedRight )
				cmd->viewangles[1] = NormalizeAngle( moveDirDeg + 90.0f );
			else
				cmd->viewangles[1] = NormalizeAngle( moveDirDeg - 90.0f );

			RotateMoveInputs( cmd, va[ 1 ] );

			cmd->forwardmove = 0.0f;
			*/

			float flNewYaw;
			if ( gbStrafedRight )
				flNewYaw = NormalizeAngle( moveDirDeg + 90.0f );
			else
				flNewYaw = NormalizeAngle( moveDirDeg - 90.0f );

			Vector vecNewAngles = cmd->viewangles;
			vecNewAngles.y = flNewYaw;

			Features::silentangles->SetAngles( vecNewAngles );
			Features::silentangles->LockAngles(); // no change allowed anymore
		}
	}
	// Skip flip
	else if ( m_pBypassMode->GetInt() == 1 &&
				m_bSkipFlip &&
				bLastStrafedRightOld != gbStrafedRight &&
				( !bWasStandingOnGround || ( m_strafeFrame.player.PosType == Strafe::PositionType::GROUND ) ) )
	{
		m_bSkipFlip = false;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CStrafer::CStrafer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pVectorialStrafer = NULL;
	m_pIgnoreGround = NULL;
	m_pBypassAntiStrafer = NULL;
	m_pBypassMode = NULL;
	m_pStrafeTowardsMovementButtons = NULL;
	m_pStopWhenHoldingBackButton = NULL;
	m_pStrafeDir = NULL;
	m_pStrafeType = NULL;

	m_bStrafed = false;
	m_bFlip = false;
	m_bSkipFlip = false;
	m_bLastStrafedRight = false;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CStrafer::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	m_bStrafed = false;
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CStrafer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	m_bStrafed = false;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CStrafer::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pVectorialStrafer = Modules::menu->AddParamBool( this, "VectorialStrafer", NULL, true );
	m_pIgnoreGround = Modules::menu->AddParamBool( this, "IgnoreGround", NULL, true );

	Modules::menu->AddElementSeparator( this, "Bypass Anti-Strafer" );
	m_pBypassAntiStrafer = Modules::menu->AddParamBool( this, "BypassAntiStrafer", "Bypass", false );
	m_pBypassMode = Modules::menu->AddParamList( this, "BypassMode", NULL, 0, " 0 - Mimic human inputs\0 1 - Skip flips\0\0" );

	Modules::menu->AddElementSeparator( this );
	m_pStrafeTowardsMovementButtons = Modules::menu->AddParamBool( this, "StrafeTowardsMovementButtons", "Strafe towards movement buttons", false );
	m_pStopWhenHoldingBackButton = Modules::menu->AddParamBool( this, "StopWhenHoldingBackButton", "Hold 'Back' button to quickly stop", false );

	Modules::menu->AddElementSeparator( this );
	m_pStrafeDir = Modules::menu->AddParamList( this, "StrafeDir", NULL, 3, " 0 - Left\0 1 - Right\0 2 - Best\0 3 - Viewangles\0 4 - Point\0\0" );
	m_pStrafeType = Modules::menu->AddParamList( this, "StrafeType", NULL, 0, " 0 - Max Acceleration\0 1 - Max Angle\0 2 - Max Deceleration\0 3 - Const Speed\0\0" );

	m_strafeFrame.UseGivenButtons = true;
	m_strafeFrame.buttons = Strafe::StrafeButtons();
	m_strafeFrame.buttons.AirLeft = Strafe::Button::LEFT;
	m_strafeFrame.buttons.AirRight = Strafe::Button::RIGHT;
	m_strafeFrame.buttons.GroundLeft = Strafe::Button::LEFT;
	m_strafeFrame.buttons.GroundRight = Strafe::Button::RIGHT;

	Modules::menu->BindConVar( m_pVectorialStrafer, &sc_strafe_vectorial );
	Modules::menu->BindConVar( m_pIgnoreGround, &sc_strafe_ignore_ground );
	Modules::menu->BindConVar( m_pStrafeDir, &sc_strafe_dir );
	Modules::menu->BindConVar( m_pStrafeType, &sc_strafe_type );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CStrafer::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_strafe );
	FEATURE_REGISTER_CVAR( sc_strafe_point_x );
	FEATURE_REGISTER_CVAR( sc_strafe_point_y );
	FEATURE_REGISTER_CVAR( sc_strafe_buttons );
	FEATURE_REGISTER_CVAR( sc_strafe_ignore_ground );
	FEATURE_REGISTER_CVAR( sc_strafe_dir );
	FEATURE_REGISTER_CVAR( sc_strafe_type );
	FEATURE_REGISTER_CVAR( sc_strafe_yaw );
	FEATURE_REGISTER_CVAR( sc_strafe_vectorial );
	FEATURE_REGISTER_CVAR( sc_strafe_vectorial_increment );
	FEATURE_REGISTER_CVAR( sc_strafe_vectorial_increment_invert );
	FEATURE_REGISTER_CVAR( sc_strafe_vectorial_offset );
	FEATURE_REGISTER_CVAR( sc_strafe_vectorial_snap );

	gamehooks->HookCvarChange( sc_strafe_ignore_ground.GetCvarPointer(), CvarChangeHook_sc_strafe_ignore_ground );
	gamehooks->HookCvarChange( sc_strafe_vectorial.GetCvarPointer(), CvarChangeHook_sc_strafe_vectorial );
	gamehooks->HookCvarChange( sc_strafe_dir.GetCvarPointer(), CvarChangeHook_sc_strafe_dir );
	gamehooks->HookCvarChange( sc_strafe_type.GetCvarPointer(), CvarChangeHook_sc_strafe_type );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CStrafer::Unload( void )
{
	gamehooks->UnhookCvarChange( sc_strafe_ignore_ground.GetCvarPointer(), CvarChangeHook_sc_strafe_ignore_ground );
	gamehooks->UnhookCvarChange( sc_strafe_vectorial.GetCvarPointer(), CvarChangeHook_sc_strafe_vectorial );
	gamehooks->UnhookCvarChange( sc_strafe_dir.GetCvarPointer(), CvarChangeHook_sc_strafe_dir );
	gamehooks->UnhookCvarChange( sc_strafe_type.GetCvarPointer(), CvarChangeHook_sc_strafe_type );

	FEATURE_UNREGISTER_CCMD( sc_strafe );
	FEATURE_UNREGISTER_CVAR( sc_strafe_point_x );
	FEATURE_UNREGISTER_CVAR( sc_strafe_point_y );
	FEATURE_UNREGISTER_CVAR( sc_strafe_buttons );
	FEATURE_UNREGISTER_CVAR( sc_strafe_ignore_ground );
	FEATURE_UNREGISTER_CVAR( sc_strafe_dir );
	FEATURE_UNREGISTER_CVAR( sc_strafe_type );
	FEATURE_UNREGISTER_CVAR( sc_strafe_yaw );
	FEATURE_UNREGISTER_CVAR( sc_strafe_vectorial );
	FEATURE_UNREGISTER_CVAR( sc_strafe_vectorial_increment );
	FEATURE_UNREGISTER_CVAR( sc_strafe_vectorial_increment_invert );
	FEATURE_UNREGISTER_CVAR( sc_strafe_vectorial_offset );
	FEATURE_UNREGISTER_CVAR( sc_strafe_vectorial_snap );
}

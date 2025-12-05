// SvenInt (c) Sw1ft
// mov_strafer.cpp

#include "stdafx.h"
#include "mov_strafer.h"
#include "mov_autojump.h"
#include "player_antiafk.h"
#include "player_stick.h"

using namespace Globals;

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
		pszDir = "To given yaw";
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

void CStrafer::UpdateStrafeData( Strafe::StrafeData &strafedata, float frametime, bool bStrafe, Strafe::StrafeDir dir, Strafe::StrafeType type, float flYaw, float flPointX, float flPointY )
{
	*reinterpret_cast<Vector *>( strafedata.player.Velocity ) = *playermove->velocity();
	*reinterpret_cast<Vector *>( strafedata.player.Origin ) = *playermove->origin();

	strafedata.vars.OnGround = playermove->onground() != -1;
	strafedata.vars.EntFriction = playermove->friction();
	strafedata.vars.Maxspeed = playermove->maxspeed();
	strafedata.vars.ReduceWishspeed = strafedata.vars.OnGround && ( playermove->flags() & FL_DUCKING );

	if ( playermove->movevars() != NULL )
	{
		strafedata.vars.Maxspeed = playermove->movevars()->maxspeed;
		strafedata.vars.Stopspeed = playermove->movevars()->stopspeed;
		strafedata.vars.Friction = playermove->movevars()->friction;
		strafedata.vars.Accelerate = playermove->movevars()->accelerate;
		strafedata.vars.Airaccelerate = playermove->movevars()->airaccelerate;
	}
	else
	{
		strafedata.vars.Stopspeed = sv_stopspeed->value;
		strafedata.vars.Friction = sv_friction->value;
		strafedata.vars.Accelerate = sv_accelerate->value;
		strafedata.vars.Airaccelerate = sv_airaccelerate->value;
	}

	//strafedata.vars.Frametime = 1.f / CVar()->FindCvar("fps_max")->value;
	//strafedata.vars.Frametime = playermove->frametime(); // 1.0f / 200.0f (1.0f / fps_max)
	strafedata.vars.Frametime = frametime; // 1.0f / 200.0f (1.0f / fps_max)

	strafedata.frame.Strafe = bStrafe;
	strafedata.frame.SetDir( dir );
	strafedata.frame.SetType( type );

	strafedata.frame.SetX( flPointX );
	strafedata.frame.SetY( flPointY );

	strafedata.frame.SetYaw( static_cast<double>( flYaw ) );

	strafedata.frame.VectorialIncrement = sc_strafe_vectorial_increment.GetFloat();
	strafedata.frame.VectorialIncrementInvert = sc_strafe_vectorial_increment_invert.GetBool();
	strafedata.frame.VectorialOffset = sc_strafe_vectorial_offset.GetFloat();
	strafedata.frame.VectorialSnap = sc_strafe_vectorial_snap.GetFloat();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CStrafer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	extern bool g_bStrafedRight;
	static bool s_bLastStrafedRight = g_bStrafedRight;
	static bool s_bFlip = false;
	static bool s_bSkipFlip = false;

	if ( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) )
		return kHookContinue;

	if ( Features::antiafk->IsEnabled() ||
		 Features::stick->IsEnabled() ||
		 playermove->dead() ||
		 playermove->iuser1() != 0 ||
		 playermove->movetype() != MOVETYPE_WALK ||
		 playermove->waterlevel() > WL_FEET )
		return kHookContinue;

	Vector va;
	float targetYaw;

	cl_enginefuncs->GetViewAngles( va );

	NormalizeAngles( va );

	if ( sc_strafe_yaw.GetString()[ 0 ] != '\0' )
	{
		targetYaw = sc_strafe_yaw.GetFloat();
		m_strafeData.frame.StrafeToViewAngles = false;
	}
	else
	{
		if ( sc_strafe_vectorial.GetBool() )
			targetYaw = va[ 1 ];
		else
			targetYaw = 0.f;

		m_strafeData.frame.StrafeToViewAngles = true;
	}

	UpdateStrafeData( m_strafeData,
					  pEvent->GetArg<float>( "frametime" ),
					  true,
					  static_cast<Strafe::StrafeDir>( sc_strafe_dir.GetInt() ),
					  static_cast<Strafe::StrafeType>( sc_strafe_type.GetInt() ),
					  targetYaw,
					  sc_strafe_point_x.GetFloat(),
					  sc_strafe_point_y.GetFloat() );

	if ( m_strafeData.frame.Strafe )
	{
		Strafe::ProcessedFrame out;
		out.Yaw = va[ 1 ];
		bool bWasStandingOnGround = m_strafeData.vars.OnGround;

		// L4DST kicks in
		if ( m_strafeData.vars.OnGround && Features::autojump->IsEnabled() && cmd->buttons & IN_JUMP )
		{
			m_strafeData.vars.OnGround = false;
			m_strafeData.vars.ReduceWishspeed = false;
		}
		else if ( sc_strafe_ignore_ground.GetBool() && m_strafeData.vars.OnGround )
		{
			return kHookContinue;
		}

		Strafe::Friction( m_strafeData );

		if ( sc_strafe_vectorial.GetBool() )
			Strafe::StrafeVectorial( m_strafeData, out, false /* g_bYawChanged */ );
		else if ( true /* !g_bYawChanged */ )
			Strafe::Strafe( m_strafeData, out );

		if ( out.Processed )
		{
			bool bOldLastStrafedRight = s_bLastStrafedRight;
			s_bLastStrafedRight = g_bStrafedRight;

			if ( m_pBypassAntiStrafer->GetBool() &&
				 s_bSkipFlip &&
				 bOldLastStrafedRight != g_bStrafedRight &&
				 ( !bWasStandingOnGround || m_strafeData.vars.OnGround ) )
			{
				s_bSkipFlip = false;
				return kHookContinue;
			}

			s_bSkipFlip = true;

			cmd->forwardmove = out.Forwardspeed;
			cmd->sidemove = out.Sidespeed;

			cmd->viewangles[ 1 ] = va[ 1 ] = out.Yaw;

			if ( bOldLastStrafedRight == g_bStrafedRight )
				s_bFlip = false;
			else
				s_bFlip = !s_bFlip;
		}
	}

	cl_enginefuncs->SetViewAngles( va );

	return kHookContinue;
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
	m_pStrafeDir = NULL;
	m_pStrafeType = NULL;

	sv_friction = NULL;
	sv_accelerate = NULL;
	sv_airaccelerate = NULL;
	sv_stopspeed = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CStrafer::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CStrafer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CStrafer::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pVectorialStrafer = Modules::menu->AddParamBool( this, "VectorialStrafer", NULL, true );
	m_pIgnoreGround = Modules::menu->AddParamBool( this, "IgnoreGround", NULL, true );
	m_pBypassAntiStrafer = Modules::menu->AddParamBool( this, "BypassAntiStrafer", NULL, false );
	m_pStrafeDir = Modules::menu->AddParamList( this, "StrafeDir", NULL, 3, " 0 - Left\0 1 - Right\0 2 - Best\0 3 - Viewangles\0 4 - Point\0\0" );
	m_pStrafeType = Modules::menu->AddParamList( this, "StrafeType", NULL, 0, " 0 - Max Acceleration\0 1 - Max Angle\0 2 - Max Deceleration\0 3 - Const Speed\0\0" );

	sv_friction = CVar()->FindCvar( "sv_friction" );
	sv_accelerate = CVar()->FindCvar( "sv_accelerate" );
	sv_airaccelerate = CVar()->FindCvar( "sv_airaccelerate" );
	sv_stopspeed = CVar()->FindCvar( "sv_stopspeed" );

	FEATURE_CHECK_SYMBOL( sv_friction, "sv_friction" );
	FEATURE_CHECK_SYMBOL( sv_accelerate, "sv_accelerate" );
	FEATURE_CHECK_SYMBOL( sv_airaccelerate, "sv_airaccelerate" );
	FEATURE_CHECK_SYMBOL( sv_stopspeed, "sv_stopspeed" );

	m_strafeData.frame.UseGivenButtons = true;
	m_strafeData.frame.buttons = Strafe::StrafeButtons();
	m_strafeData.frame.buttons.AirLeft = Strafe::Button::LEFT;
	m_strafeData.frame.buttons.AirRight = Strafe::Button::RIGHT;

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
// SvenInt (c) Sw1ft
// player_thirdperson.cpp

#include "stdafx.h"
#include "player_thirdperson.h"
#include "player_camhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CThirdPerson, thirdperson, "Player", "Third Person" );

//-----------------------------------------------------------------------------
// PM_NoClip
//-----------------------------------------------------------------------------

void CThirdPerson::PM_NoClip( usercmd_t *cmd )
{
	Vector		wishvel;
	Vector		forward;
	Vector		right;
	float		fmove, smove;

	Vector vecAngles = m_pAngles->GetVector();
	vecAngles.z = 0.f;

	AngleVectors( vecAngles, &forward, &right, NULL );

	fmove = cmd->forwardmove;
	smove = cmd->sidemove;

	for ( int i = 0; i < 3; ++i )
	{
		wishvel[ i ] = forward[ i ] * fmove + right[ i ] * smove;
	}

	wishvel[ 2 ] += cmd->upmove;

	//if ( g_Config.cvars.camhack_speed_factor >= 0.0f )
	//	wishvel = wishvel * g_Config.cvars.camhack_speed_factor;

	VectorMA( m_pOrigin->GetVector(), localplayer->Frametime(), wishvel, m_pOrigin->GetVector() );
}

//-----------------------------------------------------------------------------
// ClampViewAngles
//-----------------------------------------------------------------------------

void CThirdPerson::ClampViewAngles( void )
{
	if ( m_pAngles->GetVector()[ 0 ] > 89.0f )
		m_pAngles->GetVector()[ 0 ] = 89.0f;

	if ( m_pAngles->GetVector()[ 0 ] < -89.0f )
		m_pAngles->GetVector()[ 0 ] = -89.0f;

	if ( m_pAngles->GetVector()[ 2 ] > 89.0f )
		m_pAngles->GetVector()[ 2 ] = 89.0f;

	if ( m_pAngles->GetVector()[ 2 ] < -89.0f )
		m_pAngles->GetVector()[ 2 ] = -89.0f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CThirdPerson::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_Key_Event_HookEvent )
	{
		if ( !cl_funcs->CL_IsThirdPerson() || !m_pEditMode->GetBool() )
			return kHookContinue;

		bool bKeyDown = ( pEvent->GetArg<int>( "down" ) != 0 );
		switch ( pEvent->GetArg<int>( "keynum" ) )
		{
		case K_SPACE:
			keydown_space = bKeyDown;
			break;

		case 'w':
			keydown_w = bKeyDown;
			break;

		case 's':
			keydown_s = bKeyDown;
			break;

		case 'a':
			keydown_a = bKeyDown;
			break;

		case 'd':
			keydown_d = bKeyDown;
			break;

		case K_CTRL:
			keydown_ctrl = bKeyDown;
			break;

		case K_SHIFT:
			keydown_shift = bKeyDown;
			break;

		case K_MOUSE1:
			keydown_mouse1 = bKeyDown;
			break;

		case K_MOUSE2:
			keydown_mouse2 = bKeyDown;
			break;

		default:
			return kHookContinue;
		}

		pEvent->GetReturn<int>() = 0;
		return kHookSupercedeStop;
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		if ( !cl_funcs->CL_IsThirdPerson() || localplayer->IsSpectating() )
		{
			if ( m_pHideHUD->GetBool() )
				cvar->SetValue( GameData::Cvars::hud_draw, true );

			return kHookContinue;
		}
		else if ( m_pHideHUD->GetBool() )
		{
			cvar->SetValue( GameData::Cvars::hud_draw, false );
		}

		if ( m_bEditModeWasEnabled != m_pEditMode->GetBool() )
		{
			if ( m_pEditMode->GetBool() )
			{
				keydown_w = false;
				keydown_s = false;
				keydown_a = false;
				keydown_d = false;
				keydown_space = false;
				keydown_ctrl = false;
				keydown_shift = false;
				keydown_mouse1 = false;
				keydown_mouse2 = false;

				cl_enginefuncs->GetViewAngles( m_vecViewAngles );
				m_flSavedPitchAngle = NormalizeAngle( m_vecViewAngles.x ) / -3.0f;
			}
			else
			{
				cl_enginefuncs->SetViewAngles( m_vecViewAngles );
			}
		}

		m_bEditModeWasEnabled = m_pEditMode->GetBool();

		if ( m_pEditMode->GetBool() && !Features::camhack->IsEnabled() )
		{
			float flMaxSpeed = localplayer->GetMaxSpeed();

			m_dummyCmd.forwardmove = 0.f;
			m_dummyCmd.sidemove = 0.f;
			m_dummyCmd.upmove = 0.f;

			if ( keydown_shift )
				flMaxSpeed /= 2;
			if ( keydown_w )
				m_dummyCmd.forwardmove += flMaxSpeed;
			if ( keydown_s )
				m_dummyCmd.forwardmove -= flMaxSpeed;
			if ( keydown_d )
				m_dummyCmd.sidemove += flMaxSpeed;
			if ( keydown_a )
				m_dummyCmd.sidemove -= flMaxSpeed;
			if ( keydown_space )
				m_dummyCmd.upmove += flMaxSpeed;
			if ( keydown_ctrl )
				m_dummyCmd.upmove -= flMaxSpeed;
			if ( keydown_mouse1 )
				m_pAngles->GetVector()[ 2 ] -= 0.2f;
			if ( keydown_mouse2 )
				m_pAngles->GetVector()[ 2 ] += 0.2f;

			m_dummyCmd.upmove *= 0.75f;

			Vector va_delta = m_vecNewViewangles - m_vecOldViewangles;

			// ToDo: for better rotation, use quaternions when the camera is tilted
			m_pAngles->GetVector() += va_delta;

			NormalizeAngles( m_pAngles->GetVector() );
			ClampViewAngles();

			PM_NoClip( &m_dummyCmd );

			auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
			cmd->viewangles = m_vecViewAngles;

			cmd->forwardmove = 0.f;
			cmd->sidemove = 0.f;

			cmd->buttons &= ~( 0xFFFFFFFF & ~IN_DUCK );
		}
		else
		{
			NormalizeAngles( m_pAngles->GetVector() );
			ClampViewAngles();
		}
	}
	else if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		if ( Features::camhack->IsEnabled() )
			return kHookContinue;

		if ( pEvent->GetHookResult() != kHookContinue )
			return kHookContinue;

		if ( !cl_funcs->CL_IsThirdPerson() || cls->state != ca_active || localplayer->IsSpectating() )
			return kHookContinue;

		float localToWorld[ 3 ][ 4 ];
		Vector vecOrigin, vecAngles, vecForward, va, tmp;
		Vector vecEyePosition = localplayer->GetEyePosition();
		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

		if ( m_pEditMode->GetBool() )
			va = m_vecViewAngles;
		else
			cl_enginefuncs->GetViewAngles( va );

		if ( m_pIgnorePitch->GetBool() )
			va.x = 0.f;

		if ( m_pIgnoreYaw->GetBool() )
			va.y = 0.f;

		va.z = 0.f;

		AngleVectors( m_pAngles->GetVector(), &vecForward, NULL, NULL);
		AngleMatrix( va, localToWorld );

		VectorTransform( vecForward, localToWorld, tmp );

		localToWorld[ 0 ][ 3 ] = vecEyePosition.x;
		localToWorld[ 1 ][ 3 ] = vecEyePosition.y;
		localToWorld[ 2 ][ 3 ] = vecEyePosition.z;

		VectorTransform( m_pOrigin->GetVector(), localToWorld, vecOrigin );

		vecAngles.x = VEC_RAD2DEG( -atan2f( tmp.z, tmp.Length2D() ) );
		vecAngles.y = VEC_RAD2DEG( atan2f( tmp.y, tmp.x ) );
		vecAngles.z = m_pAngles->GetVector().z;

		if ( m_pClipToWall->GetBool() )
		{
			Vector vecOffset( 0.f, 0.f, ( localplayer->IsDucking() ? VEC_DUCK_HULL_MAX.z : VEC_HULL_MAX.z ) / 2.f );

			int oldhull = playermove->usehull();
			playermove->setusehull( m_pTraceType->GetInt() == 0 ? PM_HULL_POINT : PM_HULL_PLAYER );
			pmtrace_t tr = playermove->funcs()->PM_PlayerTrace( localplayer->GetOrigin() + vecOffset, vecOrigin, PM_STUDIO_IGNORE, -1 );
			playermove->setusehull( oldhull );

			vecOrigin = tr.endpos;
		}

		*reinterpret_cast<Vector *>( pEvent->GetArg<ref_params_t *>( "pparams" )->vieworg ) = vecOrigin;
		*reinterpret_cast<Vector *>( pEvent->GetArg<ref_params_t *>( "pparams" )->viewangles ) = vecAngles;

		if ( m_pEditMode->GetBool() )
		{
			pLocal->angles.x = m_flSavedPitchAngle;
			pLocal->curstate.angles.x = m_flSavedPitchAngle;
			pLocal->prevstate.angles.x = m_flSavedPitchAngle;
			pLocal->latched.prevangles.x = m_flSavedPitchAngle;
		}

		return kHookHandled;
	}
	else if ( bPostCall ) // kIN_Move post event
	{
		cl_enginefuncs->GetViewAngles( m_vecNewViewangles );
	}
	else // kIN_Move event
	{
		cl_enginefuncs->GetViewAngles( m_vecOldViewangles );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CThirdPerson::CThirdPerson( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pIgnorePitch = NULL;
	m_pIgnoreYaw = NULL;
	m_pEditMode = NULL;
	m_pHideHUD = NULL;
	m_pClipToWall = NULL;
	m_pTraceType = NULL;
	m_pAngles = NULL;
	m_pOrigin = NULL;

	memset( &m_dummyCmd, 0, sizeof( usercmd_t ) );
	m_flSavedPitchAngle = 0.f;
	m_bEditModeWasEnabled = false;

	keydown_w = false;
	keydown_s = false;
	keydown_a = false;
	keydown_d = false;
	keydown_space = false;
	keydown_ctrl = false;
	keydown_shift = false;
	keydown_mouse1 = false;
	keydown_mouse2 = false;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CThirdPerson::OnEnable( void )
{
	m_bEditModeWasEnabled = m_pEditMode->GetBool();

	hookevents->RegisterListener( this, kHUD_Key_Event_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kIN_Move_HookEvent );
	hookevents->RegisterListener( this, kIN_Move_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CThirdPerson::OnDisable( void )
{
	cvar->SetValue( GameData::Cvars::hud_draw, true );

	hookevents->UnregisterListener( this, kHUD_Key_Event_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kIN_Move_HookEvent );
	hookevents->UnregisterListener( this, kIN_Move_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CThirdPerson::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pIgnorePitch = Modules::menu->AddParamBool( this, "IgnorePitch", NULL, false );
	m_pIgnoreYaw = Modules::menu->AddParamBool( this, "IgnoreYaw", NULL, false );
	m_pEditMode = Modules::menu->AddParamBool( this, "EditMode", NULL, false );
	m_pHideHUD = Modules::menu->AddParamBool( this, "HideHUD", NULL, true );
	m_pClipToWall = Modules::menu->AddParamBool( this, "ClipToWall", NULL, true );
	m_pTraceType = Modules::menu->AddParamList( this, "TraceType", NULL, 1, " 0 - Trace Line\0 1 - Trace Hull\0\0" );
	m_pAngles = Modules::menu->AddParamVector( this, "Angles", NULL, vec3_origin, 0.1f, -180.f, 180.f );
	m_pOrigin = Modules::menu->AddParamVector( this, "Origin", NULL, Vector( -64.f, 0.f, 8.f ), 0.5f, -512.f, 512.f );

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CThirdPerson::Unload( void )
{
	cvar->SetValue( GameData::Cvars::hud_draw, true );
}
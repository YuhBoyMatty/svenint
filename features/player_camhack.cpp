// SvenInt (c) Sw1ft
// player_camhack.cpp

#include "stdafx.h"
#include "player_camhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CCamHack, camhack, "Player", "Cam Hack" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_camhack, "CamHack" );

CON_COMMAND( sc_camhack_attach, "Attaches camera to a specified player" )
{
	if ( !localplayer->IsInGame() )
		return;

	if ( args.ArgC() >= 2 )
	{
		int index = atoi( args[ 1 ] );

		if ( index > 0 && index <= refparams->maxclients )
		{
			THIS_FEATURE()->Attach( index );
		}
		else
		{
			THIS_FEATURE()->Disable();
		}
	}
	else
	{
		Msg( "Usage:  sc_camhack_attach <player index> (pass 0 to disable)\n" );
	}
}

//-----------------------------------------------------------------------------
// Attach camera
//-----------------------------------------------------------------------------

void CCamHack::Attach( int iPlayerIndex )
{
	float worldToLocal[ 3 ][ 4 ];
	cl_entity_t *pEntity = NULL;

	if ( ( pEntity = cl_enginefuncs->GetEntityByIndex( iPlayerIndex ) ) == NULL ||
		 pEntity->curstate.messagenum < cl_enginefuncs->GetLocalPlayer()->curstate.messagenum )
		return;

	Enable();

	m_iAttachTarget = iPlayerIndex;

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

	// Render origin & angles
	Vector vecTargetOrigin = pEntity->origin;
	Vector vecTargetAngles = pEntity->angles;

	Vector vecForward, tmp;
	Vector vecOrigin = vecTargetOrigin + VEC_VIEW;
	//Vector vecOrigin = localplayer->GetEyePosition();

	vecTargetAngles.x = 0.f;
	vecTargetAngles.z = 0.f;

	AngleIMatrix( vecTargetAngles, worldToLocal );
	AngleVectors( vecTargetAngles /* m_vecViewAngles */, &vecForward, NULL, NULL);

	// Translate origin to local space
	vecOrigin.x -= vecTargetOrigin.x;
	vecOrigin.y -= vecTargetOrigin.y;
	vecOrigin.z -= vecTargetOrigin.z;

	VectorTransform( vecOrigin, worldToLocal, m_vecCameraOrigin );

	// Translate angles to local space
	VectorTransform( vecForward, worldToLocal, tmp );

	m_vecCameraAngles.x = -atan2f( tmp.z, tmp.Length2D() ) * (float)( 180.0 / M_PI );
	m_vecCameraAngles.y = atan2f( tmp.y, tmp.x ) * (float)( 180.0 / M_PI );
	m_vecCameraAngles.z = 0.f;

	if ( m_pHideHUD->GetBool() )
	{
		cvar->SetValue( GameData::Cvars::hud_draw, false );
	}

	if ( m_pShowModel->GetBool() )
	{
		if ( !cl_funcs->CL_IsThirdPerson() )
		{
			cl_enginefuncs->pfnClientCmd( "thirdperson\n" );
			m_bChangeToThirdPerson = false;
		}
		else
		{
			m_bChangeToThirdPerson = true;
		}

		m_bChangeCameraState = true;
	}
	else
	{
		if ( cl_funcs->CL_IsThirdPerson() )
			cl_enginefuncs->pfnClientCmd( "firstperson\n" );

		m_bChangeCameraState = false;
	}
}

//-----------------------------------------------------------------------------
// PM_NoClip
//-----------------------------------------------------------------------------

void CCamHack::PM_NoClip( usercmd_t *cmd )
{
	Vector		wishvel;
	Vector		forward;
	Vector		right;
	float		fmove, smove;

	Vector vecAngles = m_vecCameraAngles;
	vecAngles.z = 0.f;

	AngleVectors( vecAngles, &forward, &right, NULL );

	fmove = cmd->forwardmove;
	smove = cmd->sidemove;

	for ( int i = 0; i < 3; ++i )
	{
		wishvel[ i ] = forward[ i ] * fmove + right[ i ] * smove;
	}

	wishvel[ 2 ] += cmd->upmove;

	if ( m_pSpeedFactor->GetFloat() >= 0.0f )
		wishvel = wishvel * m_pSpeedFactor->GetFloat();

	VectorMA( m_vecCameraOrigin, localplayer->Frametime(), wishvel, m_vecCameraOrigin );
}

//-----------------------------------------------------------------------------
// ClampViewAngles
//-----------------------------------------------------------------------------

void CCamHack::ClampViewAngles( Vector &va )
{
	if ( va[ 0 ] > 89.0f )
		va[ 0 ] = 89.0f;

	if ( va[ 0 ] < -89.0f )
		va[ 0 ] = -89.0f;

	if ( va[ 2 ] > 89.0f )
		va[ 2 ] = 89.0f;

	if ( va[ 2 ] < -89.0f )
		va[ 2 ] = -89.0f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CCamHack::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		Disable();
	}
	if ( pEvent->GetType() == kHUD_Key_Event_HookEvent )
	{
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
			m_vecCameraAngles.z -= 0.2f;

		if ( keydown_mouse2 )
			m_vecCameraAngles.z += 0.2f;

		m_dummyCmd.upmove *= 0.75f;

		Vector va_delta = m_vecNewViewangles - m_vecOldViewangles;

		// ToDo: for better rotation, use quaternions when the camera is tilted
		m_vecCameraAngles += va_delta;

		NormalizeAngles( m_vecCameraAngles );
		ClampViewAngles( m_vecCameraAngles );

		PM_NoClip( &m_dummyCmd );

		auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );
		cmd->viewangles = m_vecViewAngles;

		cmd->forwardmove = 0.f;
		cmd->sidemove = 0.f;

		cmd->buttons &= ~( 0xFFFFFFFF & ~IN_DUCK );
	}
	else if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		if ( UTIL_IsSpectating() && m_iAttachTarget == 0 )
		{
			Disable();
			return kHookContinue;
		}

		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

		auto pparams = pEvent->GetArg<ref_params_t *>( "pparams" );
		*reinterpret_cast<Vector *>( pparams->vieworg ) = m_vecCameraOrigin;
		*reinterpret_cast<Vector *>( pparams->viewangles ) = m_vecCameraAngles;

		pLocal->angles.x = m_flSavedPitchAngle;
		pLocal->curstate.angles.x = m_flSavedPitchAngle;
		pLocal->prevstate.angles.x = m_flSavedPitchAngle;
		pLocal->latched.prevangles.x = m_flSavedPitchAngle;

		if ( m_iAttachTarget == 0 )
			return kHookHandled;

		cl_entity_t *pEntity = cl_enginefuncs->GetEntityByIndex( m_iAttachTarget );

		if ( pEntity == NULL || pEntity->curstate.messagenum < pLocal->curstate.messagenum )
		{
			Disable();
			return kHookContinue;
		}

		float localToWorld[ 3 ][ 4 ];
		Vector vecOrigin, vecAngles, vecForward, tmp;
		Vector vecTargetAngles = pEntity->angles;

		if ( m_pIgnorePitch->GetBool() )
			vecTargetAngles.x = 0.f;
		else
			vecTargetAngles.x *= ( 1.f / 3.f );

		vecTargetAngles.z = 0.f;

		AngleVectors( m_vecCameraAngles, &vecForward, NULL, NULL );
		AngleMatrix( vecTargetAngles, localToWorld );

		VectorTransform( vecForward, localToWorld, tmp );

		localToWorld[ 0 ][ 3 ] = pEntity->origin.x;
		localToWorld[ 1 ][ 3 ] = pEntity->origin.y;
		localToWorld[ 2 ][ 3 ] = pEntity->origin.z;

		VectorTransform( m_vecCameraOrigin, localToWorld, vecOrigin );

		vecAngles.x = -atan2f( tmp.z, tmp.Length2D() ) * (float)( 180.0 / M_PI );
		vecAngles.y = atan2f( tmp.y, tmp.x ) * (float)( 180.0 / M_PI );
		vecAngles.z = 0.f;

		*reinterpret_cast<Vector *>( pparams->vieworg ) = vecOrigin;
		*reinterpret_cast<Vector *>( pparams->viewangles ) = vecAngles;

		return kHookHandled;
	}
	else if ( pEvent->GetType() == kStudioRenderModel_HookEvent )
	{
		if ( !cl_funcs->CL_IsThirdPerson() && studiorenderer->m_pCurrentEntity == cl_enginefuncs->GetViewModel() )
			return kHookSupercede;
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
// Press menu button event
//-----------------------------------------------------------------------------

void CCamHack::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonResetRoll )
	{
		m_vecCameraAngles.z = 0.0f;
	}
	else if ( pButton == m_pButtonResetOrientation )
	{
		m_vecCameraAngles = m_vecViewAngles;
		m_vecCameraOrigin = localplayer->GetEyePosition();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CCamHack::CCamHack( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonResetRoll = NULL;
	m_pButtonResetOrientation = NULL;
	m_pShowModel = NULL;
	m_pHideHUD = NULL;
	m_pIgnorePitch = NULL;
	m_pSpeedFactor = NULL;

	memset( &m_dummyCmd, 0, sizeof( usercmd_t ) );
	m_flSavedPitchAngle = 0.f;

	m_iAttachTarget = 0;
	m_flSavedPitchAngle = 0.0f;

	m_bChangeCameraState = false;
	m_bChangeToThirdPerson = false;

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

void CCamHack::OnEnable( void )
{
	cl_enginefuncs->GetViewAngles( m_vecViewAngles );

	m_vecCameraOrigin = localplayer->GetEyePosition();
	m_vecCameraAngles = m_vecViewAngles;

	m_flSavedPitchAngle = NormalizeAngle( m_vecViewAngles.x ) / -3.0f;

	if ( m_pHideHUD->GetBool() )
	{
		cvar->SetValue( GameData::Cvars::hud_draw, false );
	}

	if ( m_pShowModel->GetBool() )
	{
		if ( !cl_funcs->CL_IsThirdPerson() )
		{
			cl_enginefuncs->pfnClientCmd( "thirdperson\n" );
			m_bChangeToThirdPerson = false;
		}
		else
		{
			m_bChangeToThirdPerson = true;
		}

		m_bChangeCameraState = true;
	}
	else
	{
		if ( cl_funcs->CL_IsThirdPerson() )
			cl_enginefuncs->pfnClientCmd( "firstperson\n" );

		m_bChangeCameraState = false;
	}

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_Key_Event_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->RegisterListener( this, kIN_Move_HookEvent );
	hookevents->RegisterListener( this, kIN_Move_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CCamHack::OnDisable( void )
{
	m_iAttachTarget = 0;

	keydown_w = false;
	keydown_s = false;
	keydown_a = false;
	keydown_d = false;
	keydown_space = false;
	keydown_ctrl = false;
	keydown_shift = false;
	keydown_mouse1 = false;
	keydown_mouse2 = false;

	if ( m_bChangeCameraState )
	{
		if ( m_bChangeToThirdPerson )
		{
			cl_enginefuncs->pfnClientCmd( "thirdperson\n" );
		}
		else
		{
			cl_enginefuncs->pfnClientCmd( "firstperson\n" );
		}
	}

	cl_enginefuncs->SetViewAngles( m_vecViewAngles );
	m_bChangeCameraState = false;

	cvar->SetValue( GameData::Cvars::hud_draw, true );

	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_Key_Event_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->UnregisterListener( this, kIN_Move_HookEvent );
	hookevents->UnregisterListener( this, kIN_Move_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CCamHack::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pButtonResetRoll = Modules::menu->AddElementButton( this, this, "Reset Roll Axis" );
	m_pButtonResetOrientation = Modules::menu->AddElementButton( this, this, "Reset Orientation" );

	m_pShowModel = Modules::menu->AddParamBool( this, "ShowModel", NULL, true );
	m_pHideHUD = Modules::menu->AddParamBool( this, "HideHUD", NULL, true );
	m_pIgnorePitch = Modules::menu->AddParamBool( this, "AttachIgnorePitch", NULL, true );
	m_pSpeedFactor = Modules::menu->AddParamFloat( this, "SpeedFactor", NULL, 1.f, 0.f, 15.f );

	return true;
}

//-----------------------------------------------------------------------------
// PostLoad feature
//-----------------------------------------------------------------------------

void CCamHack::PostLoad( void )
{
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_camhack ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_camhack_attach ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CCamHack::Unload( void )
{
	cvar->SetValue( GameData::Cvars::hud_draw, true );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_camhack ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_camhack_attach ) );
}
// SvenInt (c) Sw1ft
// player_spinner.cpp

#include "stdafx.h"
#include "player_spinner.h"
#include "player_silent_angles.h"
#include "player_freeze.h"
#include "misc_random_generator.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Anti-Aim Modes ( Credits @guwi <3 )
//-----------------------------------------------------------------------------

// { "backwards", "sideways", "back jitter", "slowspin", "fastspin", "lisp", "fake left", "fake right" };
enum antiaim_yaw_modes : int
{
	y_none,
	y_zero,
	y_backwards,
	y_sideways,
	y_back_jitter,
	y_slowspin,
	y_fastspin,
	y_goldenspin,
	y_random,
	y_fakeleft,
	y_fakeright,
	y_switch,
	y_lisp
};

// { "emotion", "up", "down", "fakedown", "270.0f" };
enum antiaim_pitch_modes : int
{
	x_none,
	x_zero,
	x_emotion,
	x_up,
	x_down,
	x_up2,
	x_fakedown,
	x_fakeup,
	x_varie,
	x_lisp
};

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpinner, spinner, "Player", "Spinner" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_spinner, "Rotation of local player" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpinner::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	bool bAnglesChanged = false;
	constexpr int fAbortFlags = ~0 & ~( SILENT_ANGLES_ABORT_FIRING );
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( m_pRotateDeadBody->GetBool() && localplayer->IsDying() )
	{
		cl_enginefuncs->GetViewAngles( cmd->viewangles );
	}

	constexpr auto max_yaw = (float)MAXSHORT * 360.0f;
	constexpr auto max_pitch = (float)MAXSHORT * 180.0f;

	// Credits @guwi <3
	// Pitch
	switch ( m_pAntiAimModePitch->GetInt() )
	{
	case antiaim_pitch_modes::x_zero:
		m_vecSpinAngles.x = 0.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_emotion:
		m_vecSpinAngles.x = -29.67f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_up:
		m_vecSpinAngles.x = 29.67f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_down:
		m_vecSpinAngles.x = 89.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_up2:
		m_vecSpinAngles.x = -89.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_fakedown:
		m_vecSpinAngles.x = 180.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_fakeup:
		m_vecSpinAngles.x = -180.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_varie:
		m_vecSpinAngles.x = 140.0f;
		bAnglesChanged = true;
		break;

	case antiaim_pitch_modes::x_lisp:
		m_vecSpinAngles.x = max_pitch + 180.0f;
		bAnglesChanged = true;
		break;
	};

	// Yaw
	switch ( m_pAntiAimModeYaw->GetInt() )
	{
	case antiaim_yaw_modes::y_zero:
	{
		m_vecSpinAngles.y = 0.0f;
		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_backwards:
	{
		m_vecSpinAngles.y += 180.0f;
		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_sideways:
		m_vecSpinAngles.y += 90.0f;
		bAnglesChanged = true;
		break;

	case antiaim_yaw_modes::y_back_jitter:
	{
		static bool bjitter = false;
		bjitter = !bjitter;
		m_vecSpinAngles.y += bjitter ? 160.0f : -160.0f;
		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_slowspin:
		m_vecSpinAngles.y = cl_enginefuncs->GetClientTime() * 500.0f;
		bAnglesChanged = true;
		break;

	case antiaim_yaw_modes::y_fastspin:
		m_vecSpinAngles.y = cl_enginefuncs->GetClientTime() * 2000.0f;
		bAnglesChanged = true;
		break;

	case antiaim_yaw_modes::y_goldenspin:
		m_vecSpinAngles.y = cl_enginefuncs->GetClientTime() * 2000.0f / 1.61803398875f;
		bAnglesChanged = true;
		break;

	case antiaim_yaw_modes::y_random:
		m_vecSpinAngles.y = Features::random->RandomFloat( -180.0f, 180.0f );
		bAnglesChanged = true;
		break;

	case antiaim_yaw_modes::y_fakeleft:
	{
		static bool s_jitter = false;
		s_jitter = !s_jitter;
		static int m_antiaim_factor = 0;
		if ( m_antiaim_factor < 8 )
		{
			m_vecSpinAngles.y -= 45.0f;
			Features::freeze->CancelPacket( true );
			m_antiaim_factor++;
		}
		else
		{
			m_vecSpinAngles.y += 45.0f;
			Features::freeze->CancelPacket( false );
			m_antiaim_factor = 0;
		}

		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_fakeright:
	{
		static bool s_jitter = false;
		s_jitter = !s_jitter;
		static int m_antiaim_factor = 0;
		if ( m_antiaim_factor < 8 )
		{
			m_vecSpinAngles.y += 45.0f;
			Features::freeze->CancelPacket( true );
			m_antiaim_factor++;
		}
		else
		{
			m_vecSpinAngles.y -= 45.0f;
			Features::freeze->CancelPacket( true );
			m_antiaim_factor = 0;
		}

		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_switch:
	{
		static bool b_switch = false;
		if ( playermove->flags() & FL_ONGROUND )
			b_switch = !b_switch;

		m_vecSpinAngles.y += b_switch ? 90.0f : -90.0f;
		bAnglesChanged = true;
		break;
	}

	case antiaim_yaw_modes::y_lisp:
	{
		static bool ySwitch = false;

		if ( ySwitch )
			m_vecSpinAngles.y = max_yaw - 90.0f;
		else
			m_vecSpinAngles.y = max_yaw + 90.0f;

		ySwitch = !ySwitch;
		bAnglesChanged = true;
		break;
	}
	};

	if ( m_pSpinYaw->GetBool() )
	{
		if ( !m_pLockPitch->GetBool() && !m_pSpinPitch->GetBool() )
			m_vecSpinAngles.x = cmd->viewangles.x;

		m_vecSpinAngles.y += m_pSpinYawAngle->GetFloat();
		m_vecSpinAngles.y = NormalizeAngle( m_vecSpinAngles.y );

		bAnglesChanged = true;
	}
	else if ( m_pLockYaw->GetBool() )
	{
		if ( !m_pLockPitch->GetBool() && !m_pSpinPitch->GetBool() )
			m_vecSpinAngles.x = cmd->viewangles.x;

		m_vecSpinAngles.y = m_pYawAngle->GetFloat();
		bAnglesChanged = true;
	}
	else if ( m_pRevertYaw->GetBool() )
	{
		if ( !m_pLockPitch->GetBool() && !m_pSpinPitch->GetBool() && !m_pRevertPitch->GetBool() )
			m_vecSpinAngles.x = cmd->viewangles.x;

		m_vecSpinAngles.y = NormalizeAngle( cmd->viewangles.y - 180.f );
		bAnglesChanged = true;
	}

	if ( m_pSpinPitch->GetBool() )
	{
		if ( !m_pLockYaw->GetBool() && !m_pSpinYaw->GetBool() )
			m_vecSpinAngles.y = cmd->viewangles.y;

		m_vecSpinAngles.x += m_pSpinPitchAngle->GetFloat();
		m_vecSpinAngles.x = NormalizeAngle( m_vecSpinAngles.x );

		bAnglesChanged = true;
	}
	else if ( m_pLockPitch->GetBool() )
	{
		if ( !m_pLockYaw->GetBool() && !m_pSpinYaw->GetBool() )
			m_vecSpinAngles.y = cmd->viewangles.y;

		m_vecSpinAngles.x = m_pPitchAngle->GetFloat();

		bAnglesChanged = true;
	}
	else if ( m_pRevertPitch->GetBool() )
	{
		if ( !m_pLockYaw->GetBool() && !m_pSpinYaw->GetBool() && !m_pRevertYaw->GetBool() )
			m_vecSpinAngles.y = cmd->viewangles.y;

		m_vecSpinAngles.x = NormalizeAngle( -cmd->viewangles.x );

		bAnglesChanged = true;
	}

	if ( m_pLockRoll->GetBool() )
	{
		cmd->viewangles.z = m_pRollAngle->GetFloat();
	}

	if ( bAnglesChanged )
	{
		Features::silentangles->SetAngles( m_vecSpinAngles );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpinner::CSpinner( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pRotateDeadBody = NULL;

	m_pAntiAimModePitch = NULL;
	m_pAntiAimModeYaw = NULL;

	m_pRevertPitch = NULL;
	m_pRevertYaw = NULL;

	m_pLockPitch = NULL;
	m_pPitchAngle = NULL;

	m_pLockYaw = NULL;
	m_pYawAngle = NULL;
	
	m_pLockRoll = NULL;
	m_pRollAngle = NULL;

	m_pSpinPitch = NULL;
	m_pSpinPitchAngle = NULL;

	m_pSpinYaw = NULL;
	m_pSpinYawAngle = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpinner::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpinner::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpinner::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pRotateDeadBody = Modules::menu->AddParamBool( this, "RotateDeadBody", NULL, true );

	Modules::menu->AddElementSeparator( this, "Anti-Aim" );

	m_pAntiAimModePitch = Modules::menu->AddParamList( this, "AntiAimModePitch", "Anti-Aim Mode - Pitch", 0, " 0 - None\0 1 - Zero\0 2 - Emotion\0 3 - Up\0 4 - Down\0 5 - Up #2\0 6 - Fake Down\0 7 - Fake Up\0 8 - Varie\0 9 - Lisp\0\0" );
	m_pAntiAimModeYaw = Modules::menu->AddParamList( this, "AntiAimModeYaw", "Anti-Aim Mode - Yaw", 0, " 0 - None\0 1 - Zero\0 2 - Backwards\0 3 - Sideways\0 4 - Back Jitter\0 5 - Slow Spin\0 6 - Fast Spin\0 7 - Golden Spin\0 8 - Random\0 9 - Fake Left\0 10 - Fake Right\0 11 - Switch\0 12 - Lisp\0\0" );

	Modules::menu->AddElementSeparator( this );

	m_pRevertPitch = Modules::menu->AddParamBool( this, "RevertPitch", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pRevertYaw = Modules::menu->AddParamBool( this, "RevertYaw", NULL, false );

	Modules::menu->AddElementSeparator( this );

	m_pLockPitch = Modules::menu->AddParamBool( this, "LockPitch", NULL, false );
	m_pPitchAngle = Modules::menu->AddParamFloat( this, "PitchAngle", NULL, 0.f, -179.999f, 180.0f );
	
	m_pLockYaw = Modules::menu->AddParamBool( this, "LockYaw", NULL, false );
	m_pYawAngle = Modules::menu->AddParamFloat( this, "YawAngle", NULL, 0.f, 0.f, 360.f );
	
	m_pLockRoll = Modules::menu->AddParamBool( this, "LockRoll", NULL, false );
	m_pRollAngle = Modules::menu->AddParamFloat( this, "RollAngle", NULL, 0.f, 0.f, 360.f );

	Modules::menu->AddElementSeparator( this );

	m_pSpinPitch = Modules::menu->AddParamBool( this, "SpinPitch", NULL, false );
	m_pSpinPitchAngle = Modules::menu->AddParamFloat( this, "SpinPitchAngle", NULL, 0.5f, -10.f, 10.f );
	
	m_pSpinYaw = Modules::menu->AddParamBool( this, "SpinYaw", NULL, false );
	m_pSpinYawAngle = Modules::menu->AddParamFloat( this, "SpinYawAngle", NULL, 0.5f, -10.f, 10.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSpinner::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_spinner );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSpinner::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_spinner );
}
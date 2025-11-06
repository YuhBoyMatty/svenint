// SvenInt (c) Sw1ft
// player_spinner.cpp

#include "stdafx.h"
#include "player_spinner.h"
#include "player_silent_angles.h"

using namespace Globals;

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
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( m_pRotateDeadBody->GetBool() && localplayer->IsDying() )
	{
		cl_enginefuncs->GetViewAngles( cmd->viewangles );
	}

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
	m_pRevertPitch = NULL;
	m_pRevertYaw = NULL;

	m_pLockPitch = NULL;
	m_pPitchAngle = NULL;

	m_pLockYaw = NULL;
	m_pYawAngle = NULL;

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

	m_pRotateDeadBody = Modules::menu->AddParamBool( this, "RotateDeadBody", NULL, true );
	m_pRevertPitch = Modules::menu->AddParamBool( this, "RevertPitch", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pRevertYaw = Modules::menu->AddParamBool( this, "RevertYaw", NULL, false );

	Modules::menu->AddElementSeparator( this );

	m_pLockPitch = Modules::menu->AddParamBool( this, "LockPitch", NULL, false );
	m_pPitchAngle = Modules::menu->AddParamFloat( this, "PitchAngle", NULL, 0.f, -179.999f, 180.0f );
	
	m_pLockYaw = Modules::menu->AddParamBool( this, "LockYaw", NULL, false );
	m_pYawAngle = Modules::menu->AddParamFloat( this, "YawAngle", NULL, 0.f, 0.f, 360.f );

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
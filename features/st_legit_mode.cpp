// SvenInt (c) Sw1ft
// st_legit_mode.cpp

#include "stdafx.h"
#include "st_legit_mode.h"
#include "player_freeze.h"

#include "player_aim.h"
#include "player_spinner.h"
#include "player_firstperson_roaming.h"
#include "player_thirdperson.h"
#include "r_models_replacement.h"
#include "r_skybox.h"
#include "exploit_fakelag.h"
#include "exploit_action_burst.h"

#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpeedrunLegitMode, legitmode, "Speedrun Tools", "Legit Mode" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( st_legit_mode, "Toggle legit mode" );

ConVar st_legit_mode_ignore_freeze( "st_legit_mode_ignore_freeze", "0", FCVAR_EXTDLL, "Don't block freeze of the host when legit mode is on" );
ConVar st_legit_mode_block_freeze_mouse_input( "st_legit_mode_block_freeze_mouse_input", "1", FCVAR_EXTDLL, "When frozen, disable mouse input" );

//-----------------------------------------------------------------------------
// Legit mode
//-----------------------------------------------------------------------------

void CSpeedrunLegitMode::SetLegitMode( bool state )
{
	m_bLegitMode = state;
}

bool CSpeedrunLegitMode::IsLegitMode( void ) const
{
	return m_bLegitMode;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpeedrunLegitMode::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_bLegitMode = false;
	}
	else if ( pEvent->GetType() == kIN_Move_HookEvent )
	{
		if ( st_legit_mode_block_freeze_mouse_input.GetBool() &&
			 ( Features::freeze->IsTransmitCanceled() || Features::freeze->IsPacketCanceled() ) )
		{
			return kHookSupercede;
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpeedrunLegitMode::CSpeedrunLegitMode( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_bLegitMode = false;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpeedrunLegitMode::OnEnable( void )
{
	Features::aim->Disable();
	Features::spinner->Disable();
	Features::fakelag->Disable();
	Features::actionburst->Disable();
	Features::firstpersonroaming->Disable();
	Features::thirdperson->Disable();
	Features::modelsreplacement->Disable();
	Features::skybox->Disable();

	m_bLegitMode = true;

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kIN_Move_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpeedrunLegitMode::OnDisable( void )
{
	m_bLegitMode = false;

	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kIN_Move_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpeedrunLegitMode::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CSpeedrunLegitMode::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( st_legit_mode );
	FEATURE_REGISTER_CVAR( st_legit_mode_ignore_freeze );
	FEATURE_REGISTER_CVAR( st_legit_mode_block_freeze_mouse_input );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CSpeedrunLegitMode::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( st_legit_mode );
	FEATURE_UNREGISTER_CVAR( st_legit_mode_ignore_freeze );
	FEATURE_UNREGISTER_CVAR( st_legit_mode_block_freeze_mouse_input );
}
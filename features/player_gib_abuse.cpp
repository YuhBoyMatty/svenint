// SvenInt (c) Sw1ft
// player_gib_abuse.cpp

#include "stdafx.h"
#include "player_gib_abuse.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CGibAbuse, gibabuse, "Player", "Gib Abuse" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_HOLD( sc_gibabuse, "Fly with high HP" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CGibAbuse::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( localplayer->IsDead() )
		return kHookContinue;

	constexpr float flGibmeDmg = 5000.f;
	const float flHealth = extraplayerinfo->GetHealth( localplayer->GetPlayerIndex() );
	const float flDealenDmg = flGibmeDmg * (float)m_pCommandAmount->GetInt();

	if ( flHealth - flDealenDmg < m_pPreserveHealthAmount->GetFloat() )
		return kHookContinue;

	const float flTime = (float)cl_enginefuncs->GetAbsoluteTime();
	if ( flTime - m_flLastIssuedCommand < m_pCommandInterval->GetFloat() )
		return kHookContinue;

	for ( int i = 0; i < m_pCommandAmount->GetInt(); i++ )
		cl_enginefuncs->pfnClientCmd( "gibme" );

	m_flLastIssuedCommand = flTime;
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CGibAbuse::CGibAbuse( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
	
	m_pCommandAmount = NULL;
	m_pCommandInterval = NULL;
	m_pPreserveHealthAmount = NULL;

	m_flLastIssuedCommand = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CGibAbuse::OnEnable( void )
{
	m_flLastIssuedCommand = -1.f;

	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CGibAbuse::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CGibAbuse::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pCommandAmount = Modules::menu->AddParamInteger( this, "CommandAmount", NULL, 1, 1, 30 );
	m_pCommandInterval = Modules::menu->AddParamFloat( this, "CommandInterval", NULL, 0.5f, 0.f, 3.f );
	m_pPreserveHealthAmount = Modules::menu->AddParamFloat( this, "PreserveHealthAmount", NULL, 10000.f, 0.f, 100000.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CGibAbuse::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_gibabuse_down );
	FEATURE_REGISTER_CCMD( sc_gibabuse_up );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CGibAbuse::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_gibabuse_down );
	FEATURE_UNREGISTER_CCMD( sc_gibabuse_up );
}
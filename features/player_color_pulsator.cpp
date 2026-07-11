// SvenInt (c) Sw1ft
// player_color_pulsator.cpp

#include "stdafx.h"
#include "player_color_pulsator.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CColorPulsator, colorpulsator, "Player", "Color Pulsator" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CColorPulsator::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	char command_buffer[ 32 ];

	if ( m_pTopColor->GetBool() && cl_enginefuncs->GetAbsoluteTime() - m_flTopColorDelay >= m_pUpdateDelay->GetFloat() )
	{
		if ( m_iTopColorOffset > 12 )
			m_iTopColorOffset = 0;

		m_flTopColorDelay = (float)cl_enginefuncs->GetAbsoluteTime() + m_pUpdateDelay->GetFloat();

		snprintf( command_buffer, Q_ARRAYSIZE( command_buffer ), "topcolor %d\n", m_iTopColorOffset * 20 );
		cl_enginefuncs->pfnClientCmd( command_buffer );

		++m_iTopColorOffset;
	}

	if ( m_pBottomColor->GetBool() && cl_enginefuncs->GetAbsoluteTime() - m_flBottomColorDelay >= m_pUpdateDelay->GetFloat() )
	{
		if ( m_iBottomColorOffset > 12 )
			m_iBottomColorOffset = 0;

		m_flBottomColorDelay = (float)cl_enginefuncs->GetAbsoluteTime() + m_pUpdateDelay->GetFloat();

		snprintf( command_buffer, Q_ARRAYSIZE( command_buffer ), "bottomcolor %d\n", m_iBottomColorOffset * 20 );
		cl_enginefuncs->pfnClientCmd( command_buffer );

		++m_iBottomColorOffset;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CColorPulsator::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonSync )
	{
		SyncColors();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CColorPulsator::CColorPulsator( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonSync = NULL;
	m_pTopColor = NULL;
	m_pBottomColor = NULL;
	m_pUpdateDelay = NULL;

	m_iTopColorOffset = 0;
	m_flTopColorDelay = 0.f;

	m_iBottomColorOffset = 0;
	m_flBottomColorDelay = 0.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CColorPulsator::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CColorPulsator::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CColorPulsator::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pButtonSync = Modules::menu->AddElementButton( this, this, "Sync Colors" );

	m_pTopColor = Modules::menu->AddParamBool( this, "TopColor", NULL, true );
	m_pBottomColor = Modules::menu->AddParamBool( this, "BottomColor", NULL, true );
	m_pUpdateDelay = Modules::menu->AddParamFloat( this, "UpdateDelay", NULL, 0.5f, 0.1f, 2.5f );

	return true;
}
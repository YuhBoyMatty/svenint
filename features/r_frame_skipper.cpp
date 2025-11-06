// SvenInt (c) Sw1ft
// r_frame_skipper.cpp

#include "stdafx.h"
#include "r_frame_skipper.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFrameSkipper, frameskipper, "Render", "Frame Skipper" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CFrameSkipper::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	static int count = 0;

	if ( m_pSkipSequence->GetBool() )
	{
		if ( count <= m_pCount->GetInt() )
		{
			count++;
			return kHookSupercedeStop;
		}
		else
		{
			count = 0;
		}
	}
	else
	{
		if ( count <= m_pCount->GetInt() )
		{
			count++;
		}
		else
		{
			count = 0;
			return kHookSupercedeStop;
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFrameSkipper::CFrameSkipper( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pSkipSequence = NULL;
	m_pCount = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CFrameSkipper::OnEnable( void )
{
	hookevents->RegisterListener( this, kSCR_UpdateScreen_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CFrameSkipper::OnDisable( void )
{
	hookevents->UnregisterListener( this, kSCR_UpdateScreen_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFrameSkipper::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pSkipSequence = Modules::menu->AddParamBool( this, "SkipSequence", NULL, false );
	m_pCount = Modules::menu->AddParamInteger( this, "Count", NULL, 1, 1, 60 );

	return true;
}
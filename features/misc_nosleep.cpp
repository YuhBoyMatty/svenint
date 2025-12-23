// SvenInt (c) Sw1ft
// misc_nosleep.cpp

#include "stdafx.h"
#include "misc_nosleep.h"
#include "modules/menu.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( void, CGame__SleepUntilInput, void *thisptr, int nMaxSleepTime );

//-----------------------------------------------------------------------------
// Feature patterns & gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Offsets
	{
		namespace Engine
		{
			size_t vtidx_CGame__SleepUntilInput = 4;
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CNoSleep, nosleep, "Misc", "No Sleep" );

//-----------------------------------------------------------------------------
// CGame::SleepUntilInput hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_CGame__SleepUntilInput, void *thisptr, int nMaxSleepTime )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		nMaxSleepTime = 0;

	ORIG_CGame__SleepUntilInput( ARG_THISPTR( thisptr ), nMaxSleepTime );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CNoSleep::CNoSleep( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_hCGame__SleepUntilInput = DETOUR_INVALID_HANDLE;
	m_pfnCGame__SleepUntilInput = NULL;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CNoSleep::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pfnCGame__SleepUntilInput = MemoryUtils()->FindVTable( GameData::Modules::Engine, "CGame" );
	FEATURE_CHECK_SYMBOL( m_pfnCGame__SleepUntilInput, "CGame (VMT)" );

	if ( gamedata->Initialized() )
	{
		FeaturesGameData::Offsets::Engine::vtidx_CGame__SleepUntilInput = gamedata->FindOffset( GameData::Modules::Engine, "Engine", "CGame::SleepUntilInput" );
		if ( FeaturesGameData::Offsets::Engine::vtidx_CGame__SleepUntilInput == ~0 )
			return false;
	}
	else
	{
	#ifdef LINUX
		return false;
	#endif
	}

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CNoSleep::PostLoad( void )
{
	void *dummyInstance = m_pfnCGame__SleepUntilInput;
	m_hCGame__SleepUntilInput = Detours()->DetourVirtualFunction( &dummyInstance,
																  FeaturesGameData::Offsets::Engine::vtidx_CGame__SleepUntilInput,
																  HOOKED_CGame__SleepUntilInput,
																  GET_FUNC_PTR( ORIG_CGame__SleepUntilInput ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CNoSleep::Unload( void )
{
	Detours()->RemoveDetour( m_hCGame__SleepUntilInput );
}
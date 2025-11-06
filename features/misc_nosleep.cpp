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
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERNS_2( CGame__SleepUntilInput,
							   "5.25",
							   "83 EC ? A1 ? ? ? ? 33 C4 89 44 24 40 53 56 57",
							   "5.11",
							   "83 EC ? A1 ? ? ? ? 33 C4 89 44 24 40 53 55 56" );
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

	ORIG_CGame__SleepUntilInput( thisptr, nMaxSleepTime );
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

	int patternIndex;
	DEFINE_PATTERNS_FUTURE( fCGame__SleepUntilInput );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::CGame__SleepUntilInput, fCGame__SleepUntilInput );

	m_pfnCGame__SleepUntilInput = MemoryUtils()->GetPatternFutureValue( fCGame__SleepUntilInput, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS( m_pfnCGame__SleepUntilInput,
								   "CGame::SleepUntilInput",
								   FeaturesGameData::Patterns::Engine::CGame__SleepUntilInput,
								   patternIndex );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CNoSleep::PostLoad( void )
{
	m_hCGame__SleepUntilInput = Detours()->DetourFunction( m_pfnCGame__SleepUntilInput,
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
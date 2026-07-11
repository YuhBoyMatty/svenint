// SvenInt (c) Sw1ft
// misc_ignore_different_maps.cpp

#include "stdafx.h"
#include "misc_ignore_different_maps.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( int, CALLCONV_CDECL, CRC_MapFile, uint32_t *ulCRC, char *pszMapName );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERN( CRC_MapFile, "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 84 04 00 00" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CIgnoreDifferentMaps, ignorediffmaps, "Misc", "Ignore Different Maps" );

//-----------------------------------------------------------------------------
// CRC_MapFile hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, CALLCONV_CDECL, HOOKED_CRC_MapFile, uint32_t *ulCRC, char *pszMapName )
{
	int result = ORIG_CRC_MapFile( ulCRC, pszMapName );

	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->CheckMapCRC( ulCRC, pszMapName );

	return result;
}

//-----------------------------------------------------------------------------
// CRC_MapFile event
//-----------------------------------------------------------------------------

void CIgnoreDifferentMaps::CheckMapCRC( uint32_t *ulCRC, char *pszMapName )
{
	if ( !m_bCheckMapCRC )
		return;

	if ( *ulCRC != mapcrc )
	{
		if ( mapcrc != 0 && *ulCRC != 0xFFFFFFFF )
		{
			PrintWarning( "Uh oh, your version of the map is different from the server one. Don't worry, we'll keep connecting\n" );
			PrintWarning( "Client's CRC of the map: %X\n", mapcrc );
			PrintWarning( "Server's CRC of the map: %X\n", *ulCRC );
		}

		*ulCRC = mapcrc;
	}

	m_bCheckMapCRC = false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CIgnoreDifferentMaps::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// Netmsg ServerInfo event
	m_bCheckMapCRC = true;

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CIgnoreDifferentMaps::CIgnoreDifferentMaps( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bCheckMapCRC = false;
	m_pfnCRC_MapFile = NULL;
	m_hCRC_MapFile = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CIgnoreDifferentMaps::OnEnable( void )
{
	hookevents->RegisterListener( this, kServerInfo_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CIgnoreDifferentMaps::OnDisable( void )
{
	hookevents->UnregisterListener( this, kServerInfo_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CIgnoreDifferentMaps::Load( void )
{
	Modules::menu->BindFeature( this );

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		m_pfnCRC_MapFile = gamedata->FindRVA( GameData::Modules::Engine, "Engine", "CRC_MapFile" );
		if ( m_pfnCRC_MapFile == NULL )
			return false;
	}
	else
	{
		m_pfnCRC_MapFile = MemoryUtils()->FindPattern( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::CRC_MapFile );
		FEATURE_CHECK_SYMBOL_PATTERN( m_pfnCRC_MapFile, "CRC_MapFile" );
	}

	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnCRC_MapFile", m_pfnCRC_MapFile, GameData::Modules::Engine );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CIgnoreDifferentMaps::PostLoad( void )
{
	m_hCRC_MapFile = Detours()->DetourFunction( m_pfnCRC_MapFile, HOOKED_CRC_MapFile, GET_FUNC_PTR( ORIG_CRC_MapFile ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CIgnoreDifferentMaps::Unload( void )
{
	Detours()->RemoveDetour( m_hCRC_MapFile );
}

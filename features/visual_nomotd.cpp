// SvenInt (c) Sw1ft
// visual_nomotd.cpp

#include "stdafx.h"
#include "visual_nomotd.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CNoMOTD, nomotd, "Visual", "No MOTD" );

static UserMsgHookFn ORIG_UserMsgHook_MOTD = NULL;
static UserMsgHookFn ORIG_UserMsgHook_VGUIMenu = NULL;

//-----------------------------------------------------------------------------
// MOTD
//-----------------------------------------------------------------------------

static int UserMsgHook_MOTD( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		return 0;

	return ORIG_UserMsgHook_MOTD( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// VGUIMenu - Mission Briefing
//-----------------------------------------------------------------------------

static int UserMsgHook_VGUIMenu( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE_IS_ENABLED() )
	{
		CMessageBuffer message( pszUserMsg, pBuffer, iSize, true );
		if ( message.ReadByte() == 4 ) // MENU_MAPBRIEFING
			return 0;
	}

	return ORIG_UserMsgHook_VGUIMenu( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CNoMOTD::CNoMOTD( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_hUserMsgHook_MOTD = DETOUR_INVALID_HANDLE;
	m_hUserMsgHook_VGUIMenu = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CNoMOTD::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CNoMOTD::PostLoad( void )
{
	m_hUserMsgHook_MOTD = gamehooks->HookUserMessage( "MOTD", UserMsgHook_MOTD, &ORIG_UserMsgHook_MOTD );
	m_hUserMsgHook_VGUIMenu = gamehooks->HookUserMessage( "VGUIMenu", UserMsgHook_VGUIMenu, &ORIG_UserMsgHook_VGUIMenu );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CNoMOTD::Unload( void )
{
	gamehooks->UnhookUserMessage( m_hUserMsgHook_MOTD );
	gamehooks->UnhookUserMessage( m_hUserMsgHook_VGUIMenu );
}
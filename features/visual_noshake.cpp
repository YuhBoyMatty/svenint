// SvenInt (c) Sw1ft
// visual_noshake.cpp

#include "stdafx.h"
#include "visual_noshake.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CNoShake, noshake, "Visual", "No Shake" );

static UserMsgHookFn ORIG_UserMsgHook_ScreenShake = NULL;

//-----------------------------------------------------------------------------
// ScreenShake
//-----------------------------------------------------------------------------

static int UserMsgHook_ScreenShake( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		return 0;

	return ORIG_UserMsgHook_ScreenShake( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CNoShake::CNoShake( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_hUserMsgHook_ScreenShake = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CNoShake::Load( void )
{
	Modules::menu->BindFeature( this );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CNoShake::PostLoad( void )
{
	m_hUserMsgHook_ScreenShake = gamehooks->HookUserMessage( "ScreenShake", UserMsgHook_ScreenShake, &ORIG_UserMsgHook_ScreenShake );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CNoShake::Unload( void )
{
	gamehooks->UnhookUserMessage( m_hUserMsgHook_ScreenShake );
}
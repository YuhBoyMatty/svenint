// SvenInt (c) Sw1ft
// visual_noviewentity.cpp

#include "stdafx.h"
#include "visual_noviewentity.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CNoViewEntity, noviewentity, "Visual", "No View Entity" );

static NetMsgHookFn ORIG_NetMsgHook_SetView = NULL;

//-----------------------------------------------------------------------------
// SetView hook
//-----------------------------------------------------------------------------

static void NetMsgHook_SetView( void )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_NetMsgHook_SetView();
		return;
	}

	static CMessageBuffer msg;
	CNetMessageParams *params = gameutils->GetNetMessageParams();
	msg.Init( params->buffer, params->readcount, params->badread );

	int viewentity = msg.ReadShort();
	if ( viewentity == localplayer->GetPlayerIndex() )
	{
		ORIG_NetMsgHook_SetView();
		return;
	}

	gameutils->ApplyReadToNetMessageBuffer( &msg );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CNoViewEntity::CNoViewEntity( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
	m_hNetMsgHook_SetView = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CNoViewEntity::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CNoViewEntity::PostLoad( void )
{
	m_hNetMsgHook_SetView = gamehooks->HookNetworkMessage( SVC_SETVIEW, NetMsgHook_SetView, &ORIG_NetMsgHook_SetView );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CNoViewEntity::Unload( void )
{
	gamehooks->UnhookNetworkMessage( m_hNetMsgHook_SetView );
}
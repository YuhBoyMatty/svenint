// SvenInt (c) Sw1ft
// misc_bypass_cvar_query.cpp

#include "stdafx.h"
#include "misc_bypass_cvar_query.h"
#include "modules/menu.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

static NetMsgHookFn ORIG_NetMsgHook_SendCvarValue = NULL;
static NetMsgHookFn ORIG_NetMsgHook_SendCvarValue2 = NULL;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CBypassCvarQuery, bypasscvarquery, "Misc", "Bypass Cvar Query" );

//-----------------------------------------------------------------------------
// Netmsg cvar query hooks
//-----------------------------------------------------------------------------

static void HOOKED_NetMsgHook_SendCvarValue( void )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_NetMsgHook_SendCvarValue();
		return;
	}

	CMessageBuffer msg, cvarquerymsg;
	CNetMessageParams *params = gameutils->GetNetMessageParams();

	cvarquerymsg.Init( params->buffer, params->readcount, params->badread );
	msg.Init( clc_buffer );

	char *pszCvarName = cvarquerymsg.ReadString();

	if ( strlen( pszCvarName ) >= 0xFF )
	{
		msg.WriteByte( CLC_REQUESTCVARVALUE );
		msg.WriteString( (char *)"Bad CVAR request" );
	}
	else
	{
		cvar_t *pCvar = cvar->FindCvar( pszCvarName );
		if ( pCvar != NULL )
		{
			ConVar *pConvar = cvar->FindVar( pszCvarName );
			if ( pConvar != NULL )
			{
				Msg( "Rejected a server's attempt to query SvenInt's console variable \"%s\"\n", pszCvarName );

				msg.WriteByte( CLC_REQUESTCVARVALUE );
				msg.WriteString( (char *)"Bad CVAR request" );
			}
			else
			{
				msg.WriteByte( CLC_REQUESTCVARVALUE );
				msg.WriteString( (char *)pCvar->string );
			}
		}
		else
		{
			msg.WriteByte( CLC_REQUESTCVARVALUE );
			msg.WriteString( (char *)"Bad CVAR request" );
		}
	}

	gameutils->ApplyReadToNetMessageBuffer( &cvarquerymsg );
}

static void HOOKED_NetMsgHook_SendCvarValue2( void )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_NetMsgHook_SendCvarValue2();
		return;
	}

	CMessageBuffer msg, cvarquerymsg;
	CNetMessageParams *params = gameutils->GetNetMessageParams();

	cvarquerymsg.Init( params->buffer, params->readcount, params->badread );
	msg.Init( clc_buffer );

	int iRequestID = cvarquerymsg.ReadLong();
	char *pszCvarName = cvarquerymsg.ReadString();

	if ( strlen( pszCvarName ) >= 0xFF )
	{
		msg.WriteByte( CLC_REQUESTCVARVALUE2 );
		msg.WriteLong( iRequestID );
		msg.WriteString( pszCvarName );
		msg.WriteString( (char *)"Bad CVAR request" );
	}
	else
	{
		cvar_t *pCvar = cvar->FindCvar( pszCvarName );
		if ( pCvar != NULL )
		{
			ConVar *pConvar = cvar->FindVar( pszCvarName );
			if ( pConvar != NULL )
			{
				Msg( "Rejected a server's attempt to query SvenInt's console variable \"%s\"\n", pszCvarName );

				msg.WriteByte( CLC_REQUESTCVARVALUE2 );
				msg.WriteLong( iRequestID );
				msg.WriteString( pszCvarName );
				msg.WriteString( (char *)"Bad CVAR request" );
			}
			else
			{
				msg.WriteByte( CLC_REQUESTCVARVALUE2 );
				msg.WriteLong( iRequestID );
				msg.WriteString( pszCvarName );
				msg.WriteString( (char *)pCvar->string );
			}
		}
		else
		{
			msg.WriteByte( CLC_REQUESTCVARVALUE2 );
			msg.WriteLong( iRequestID );
			msg.WriteString( pszCvarName );
			msg.WriteString( (char *)"Bad CVAR request" );
		}
	}

	gameutils->ApplyReadToNetMessageBuffer( &cvarquerymsg );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CBypassCvarQuery::CBypassCvarQuery( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_hNetMsgHook_SendCvarValue = DETOUR_INVALID_HANDLE;
	m_hNetMsgHook_SendCvarValue2 = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CBypassCvarQuery::Load( void )
{
	Modules::menu->BindFeature( this );

	FEATURE_REQUIRE_GAMEDATA( Globals::clc_buffer, "clc_buffer" );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CBypassCvarQuery::PostLoad( void )
{
	m_hNetMsgHook_SendCvarValue = gamehooks->HookNetworkMessage( SVC_SENDCVARVALUE, HOOKED_NetMsgHook_SendCvarValue, &ORIG_NetMsgHook_SendCvarValue );
	m_hNetMsgHook_SendCvarValue2 = gamehooks->HookNetworkMessage( SVC_SENDCVARVALUE2, HOOKED_NetMsgHook_SendCvarValue2, &ORIG_NetMsgHook_SendCvarValue2 );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CBypassCvarQuery::Unload( void )
{
	Detours()->RemoveDetour( m_hNetMsgHook_SendCvarValue );
	Detours()->RemoveDetour( m_hNetMsgHook_SendCvarValue2 );
}
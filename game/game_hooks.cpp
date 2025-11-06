// SvenInt (c) Sw1ft
// game_hooks.cpp

#include "stdafx.h"
#include "game_hooks.h"

//-----------------------------------------------------------------------------
// CGameHooks implementation
//-----------------------------------------------------------------------------

CGameHooks::CGameHooks() : m_hCvarChangeHooks( 127 )
{
}

void CGameHooks::Init()
{
}

void CGameHooks::Shutdown()
{
	for ( int i = 0; i < m_hCvarChangeHooks.Count(); i++ )
	{
		HashTableIterator_t it = m_hCvarChangeHooks.First( i );

		while ( m_hCvarChangeHooks.IsValidIterator( it ) )
		{
			std::vector<CvarChangeHookFn> &hooks = m_hCvarChangeHooks.ValueAt( i, it );

			hooks.clear();

			it = m_hCvarChangeHooks.Next( i, it );
		}
	}

	m_hCvarChangeHooks.Purge();
}

//-----------------------------------------------------------------------------
// Cvar
//-----------------------------------------------------------------------------

void CGameHooks::CallCvarChangeChain( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	std::vector<CvarChangeHookFn> *pCvarChangeHooks = m_hCvarChangeHooks.Find( pCvar );
	if ( pCvarChangeHooks == NULL )
		return;

	for ( size_t i = 0; i < pCvarChangeHooks->size(); i++ )
	{
		pCvarChangeHooks->at( i )( pCvar, pszOldValue, flOldValue );
	}
}

bool CGameHooks::HookCvarChange( cvar_t *pCvar, CvarChangeHookFn pfnCvarChangeHook )
{
	if ( pCvar == NULL || pfnCvarChangeHook == NULL )
		return false;

	std::vector<CvarChangeHookFn> *pCvarChangeHooks = m_hCvarChangeHooks.Find( pCvar );
	if ( pCvarChangeHooks != NULL )
	{
		if ( std::find( pCvarChangeHooks->begin(), pCvarChangeHooks->end(), pfnCvarChangeHook ) != pCvarChangeHooks->end() )
		{
			Warning( "[SvenInt] CGameHooks::HookCvarChange: cvar change hook (0x%X) for cvar \"%s\" (0x%X) is already registered\n",
					 pfnCvarChangeHook, pCvar->name, pCvar );
			return false;
		}
		
		pCvarChangeHooks->push_back( pfnCvarChangeHook );
	}
	else
	{
		if ( !m_hCvarChangeHooks.Insert( pCvar, std::vector<CvarChangeHookFn>() ) )
		{
			Warning( "[SvenInt] CGameHooks::HookCvarChange: failed to register cvar change hook (0x%X) for cvar \"%s\" (0x%X)\n",
					 pfnCvarChangeHook, pCvar->name, pCvar );
			return false;
		}

		m_hCvarChangeHooks.Find( pCvar )->push_back( pfnCvarChangeHook );
	}

	DevMsg( "<SvenInt::GameHooks> Registered cvar change hook for \"%s\" (0x%X)\n", pCvar->name, pCvar );
	return true;
}

bool CGameHooks::UnhookCvarChange( cvar_t *pCvar, CvarChangeHookFn pfnCvarChangeHook )
{
	if ( pCvar == NULL || pfnCvarChangeHook == NULL )
		return false;

	std::vector<CvarChangeHookFn> *pCvarChangeHooks = m_hCvarChangeHooks.Find( pCvar );

	if ( pCvarChangeHooks == NULL )
	{
		Warning( "[SvenInt] CGameHooks::HookCvarChange: cvar change hook (0x%X) for cvar \"%s\" (0x%X) is not registered\n", pfnCvarChangeHook, pCvar->name, pCvar );
		return false;
	}

	std::vector<CvarChangeHookFn>::iterator it;
	if ( ( it = std::find( pCvarChangeHooks->begin(), pCvarChangeHooks->end(), pfnCvarChangeHook ) ) == pCvarChangeHooks->end() )
	{
		Warning( "[SvenInt] CGameHooks::HookCvarChange: cvar change hook (0x%X) for cvar \"%s\" (0x%X) is not registered\n",
				 pfnCvarChangeHook, pCvar->name, pCvar );
		return false;
	}

	pCvarChangeHooks->erase( it );

	if ( pCvarChangeHooks->size() == 0 )
	{
		m_hCvarChangeHooks.Remove( pCvar );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Network Message
//-----------------------------------------------------------------------------

DetourHandle_t CGameHooks::HookNetworkMessage( int iType, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook )
{
	if ( Globals::g_NetworkMessages == NULL )
		return DETOUR_INVALID_HANDLE;

	if ( iType > SVC_NOP && iType <= SVC_LASTMSG )
	{
		netmsg_t *pNetMsg = &Globals::g_NetworkMessages[ iType ];

		netmsg_t **ppNetMsg = &pNetMsg;
		constexpr size_t index = offsetof( netmsg_t, function ) / sizeof( void * );

		DevMsg( "<SvenInt::GameHooks> Hooked network message \"%s\" (%d) for detour at address 0x%X\n", pNetMsg->name, iType, pfnNetMsgHook );
		return Detours()->DetourVirtualFunction( ppNetMsg, index, pfnNetMsgHook, (void **)ppfnOriginalNetMsgHook );
	}
	else
	{
		Warning( "[SvenInt] CGameHooks::HookNetworkMessage: bad network message type (%d)\n", iType );
	}

	return DETOUR_INVALID_HANDLE;
}

DetourHandle_t CGameHooks::HookNetworkMessage( const char *pszName, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook )
{
	if ( Globals::g_NetworkMessages == NULL )
		return DETOUR_INVALID_HANDLE;

	for ( int i = SVC_NOP + 1; i <= SVC_LASTMSG; i++ )
	{
		netmsg_t *pNetMsg = &Globals::g_NetworkMessages[ i ];

		if ( !stricmp( pszName, pNetMsg->name ) )
		{
			netmsg_t **ppNetMsg = &pNetMsg;
			constexpr size_t index = offsetof( netmsg_t, function ) / sizeof( void * );

			DevMsg( "<SvenInt::GameHooks> Hooked network message \"%s\" (%d) for detour at address 0x%X\n", pNetMsg->name, i, pfnNetMsgHook );
			return Detours()->DetourVirtualFunction( ppNetMsg, index, pfnNetMsgHook, (void **)ppfnOriginalNetMsgHook );
		}
	}

	Warning( "[SvenInt] CGameHooks::HookNetworkMessage: bad network message name (%s)\n", pszName );

	return DETOUR_INVALID_HANDLE;
}

DetourHandle_t CGameHooks::HookNetworkMessage( netmsg_t *pNetMsg, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook )
{
	netmsg_t **ppNetMsg = &pNetMsg;
	constexpr size_t index = offsetof( netmsg_t, function ) / sizeof( void * );

	DevMsg( "<SvenInt::GameHooks> Hooked network message \"%s\" (%d) for detour at address 0x%X\n", pNetMsg->name, pNetMsg->index, pfnNetMsgHook );
	return Detours()->DetourVirtualFunction( ppNetMsg, index, pfnNetMsgHook, (void **)ppfnOriginalNetMsgHook );
}

bool CGameHooks::UnhookNetworkMessage( DetourHandle_t hNetMsgHook )
{
	return Detours()->RemoveDetour( hNetMsgHook );
}

//-----------------------------------------------------------------------------
// User Message
//-----------------------------------------------------------------------------

DetourHandle_t CGameHooks::HookUserMessage( const char *pszName, UserMsgHookFn pfnUserMsgHook, UserMsgHookFn *ppfnOriginalUserMsgHook )
{
	if ( Globals::g_pClientUserMsgs == NULL )
		return DETOUR_INVALID_HANDLE;

	// HACK: let Detours think we're going to hook a virtual function
	usermsg_t *pUserMsg = *Globals::g_pClientUserMsgs;
	int ndx = -1;

	while ( pUserMsg != NULL )
	{
		ndx++;

		if ( !stricmp( pszName, pUserMsg->name ) )
		{
			usermsg_t **ppUserMsg = &pUserMsg;
			constexpr size_t index = offsetof( usermsg_t, function ) / sizeof( void * );

			DevMsg( "<SvenInt::GameHooks> Hooked user message \"%s\" (%d) for detour at address 0x%X\n", pUserMsg->name, ndx, pfnUserMsgHook );
			return Detours()->DetourVirtualFunction( ppUserMsg, index, pfnUserMsgHook, (void **)ppfnOriginalUserMsgHook );
		}

		pUserMsg = pUserMsg->next;
	}

	Warning( "[SvenInt] CGameHooks::HookUserMessage: bad user message name (%s)\n", pszName );

	return DETOUR_INVALID_HANDLE;
}

DetourHandle_t CGameHooks::HookUserMessage( usermsg_t *pUserMsg, UserMsgHookFn pfnUserMsgHook, UserMsgHookFn *ppfnOriginalUserMsgHook )
{
	usermsg_t **ppUserMsg = &pUserMsg;
	constexpr size_t index = offsetof( usermsg_t, function ) / sizeof( void * );

	DevMsg( "<SvenInt::GameHooks> Hooked user message \"%s\" (%d) for detour at address 0x%X\n", pUserMsg->name, pUserMsg->msgid, pfnUserMsgHook );
	return Detours()->DetourVirtualFunction( ppUserMsg, index, pfnUserMsgHook, (void **)ppfnOriginalUserMsgHook );
}

bool CGameHooks::UnhookUserMessage( DetourHandle_t hUserMsgHook )
{
	return Detours()->RemoveDetour( hUserMsgHook );
}

//-----------------------------------------------------------------------------
// Event Hook
//-----------------------------------------------------------------------------

DetourHandle_t CGameHooks::HookEvent( const char *pszName, EventHookFn pfnEventHook, EventHookFn *ppfnOriginalEventHook )
{
	if ( Globals::g_pEventHooks == NULL )
		return DETOUR_INVALID_HANDLE;

	event_hook_t *pEventHook = Globals::g_pEventHooks;

	while ( pEventHook != NULL )
	{
		if ( pEventHook->name != NULL )
		{
			if ( !stricmp( pszName, pEventHook->name ) )
			{
				event_hook_t **ppEventHook = &pEventHook;
				constexpr size_t index = offsetof( event_hook_t, function ) / sizeof( void * );

				DevMsg( "<SvenInt::GameHooks> Hooked event hook \"%s\" for detour at address 0x%X\n", pEventHook->name, pfnEventHook );
				return Detours()->DetourVirtualFunction( ppEventHook, index, pfnEventHook, (void **)ppfnOriginalEventHook );
			}
		}

		pEventHook = pEventHook->next;
	}

	Warning( "[SvenInt] CGameHooks::HookEvent: bad event hook name (%s)\n", pszName );

	return DETOUR_INVALID_HANDLE;
}

DetourHandle_t CGameHooks::HookEvent( event_hook_t *pEventHook, EventHookFn pfnEventHook, EventHookFn *ppfnOriginalEventHook )
{
	event_hook_t **ppEventHook = &pEventHook;
	constexpr size_t index = offsetof( event_hook_t, function ) / sizeof( void * );

	DevMsg( "<SvenInt::GameHooks> Hooked event hook \"%s\" for detour at address 0x%X\n", pEventHook->name, pfnEventHook );
	return Detours()->DetourVirtualFunction( ppEventHook, index, pfnEventHook, (void **)ppfnOriginalEventHook );
}

bool CGameHooks::UnhookEvent( DetourHandle_t hEventHook )
{
	return Detours()->RemoveDetour( hEventHook );
}

//-----------------------------------------------------------------------------
// Console Command
//-----------------------------------------------------------------------------

DetourHandle_t CGameHooks::HookConsoleCommand( const char *pszName, CommandCallbackFn pfnCommandCallback, CommandCallbackFn *ppfnOriginalCommandCallback )
{
	cmd_function_t *pCmd = Globals::cvar->FindCmd( pszName );

	if ( pCmd == NULL )
		return DETOUR_INVALID_HANDLE;

	cmd_function_t **ppCmd = &pCmd;
	constexpr size_t index = offsetof( cmd_function_t, function ) / sizeof( void * );

	DevMsg( "<SvenInt::GameHooks> Hooked console command \"%s\" for detour at address 0x%X\n", pCmd->name, pfnCommandCallback );
	return Detours()->DetourVirtualFunction( ppCmd, index, pfnCommandCallback, (void **)ppfnOriginalCommandCallback );
}

DetourHandle_t CGameHooks::HookConsoleCommand( cmd_function_t *pCommand, CommandCallbackFn pfnCommandCallback, CommandCallbackFn *ppfnOriginalCommandCallback )
{
	cmd_function_t **ppCommand = &pCommand;
	constexpr size_t index = offsetof( cmd_function_t, function ) / sizeof( void * );

	DevMsg( "<SvenInt::GameHooks> Hooked console command \"%s\" for detour at address 0x%X\n", pCommand->name, pfnCommandCallback );
	return Detours()->DetourVirtualFunction( pCommand, index, pfnCommandCallback, (void **)ppfnOriginalCommandCallback );
}

bool CGameHooks::UnhookConsoleCommand( DetourHandle_t hCommandCallback )
{
	return Detours()->RemoveDetour( hCommandCallback );
}

//-----------------------------------------------------------------------------
// Export
//-----------------------------------------------------------------------------

static CGameHooks gGameHooks;
CGameHooks *GameHooks()
{
	return &gGameHooks;
}
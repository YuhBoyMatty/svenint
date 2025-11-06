// SvenInt (c) Sw1ft
// game_hooks.h

#ifndef SINT_GAME_HOOKS_H
#define SINT_GAME_HOOKS_H

#ifdef _WIN32
#pragma once
#endif

#include "utils/detours.h"
#include "utils/hashtable.h"

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef void ( *CvarChangeHookFn )( struct cvar_s *pCvar, const char *pszOldValue, float flOldValue );

using CommandCallbackFn = void ( __cdecl * )( void );
using NetMsgHookFn = void ( __cdecl * )( void );
using EventHookFn = void ( __cdecl * )( struct event_args_s *args );
using UserMsgHookFn = int( __cdecl * )( const char *pszName, int iSize, void *pbuf );

//-----------------------------------------------------------------------------
// CGameHooks
//-----------------------------------------------------------------------------

class CGameHooks
{
	friend DECLARE_FUNC( void, __cdecl, HOOKED_Cvar_DirectSet, struct cvar_s *pCvar, const char *pszValue );

public:
	CGameHooks();

	void					Init();
	void					Shutdown();

	virtual bool			HookCvarChange( struct cvar_s *pCvar, CvarChangeHookFn pfnCvarChangeHook );
	virtual bool			UnhookCvarChange( struct cvar_s *pCvar, CvarChangeHookFn pfnCvarChangeHook );

	virtual DetourHandle_t	HookNetworkMessage( int iType, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook );
	virtual DetourHandle_t	HookNetworkMessage( const char *pszName, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook );
	virtual DetourHandle_t	HookNetworkMessage( struct netmsg_s *pNetMsg, NetMsgHookFn pfnNetMsgHook, NetMsgHookFn *ppfnOriginalNetMsgHook );
	virtual bool			UnhookNetworkMessage( DetourHandle_t hNetMsgHook );

	virtual DetourHandle_t	HookUserMessage( const char *pszName, UserMsgHookFn pfnUserMsgHook, UserMsgHookFn *ppfnOriginalUserMsgHook );
	virtual DetourHandle_t	HookUserMessage( struct usermsg_s *pUserMsg, UserMsgHookFn pfnUserMsgHook, UserMsgHookFn *ppfnOriginalUserMsgHook );
	virtual bool			UnhookUserMessage( DetourHandle_t hUserMsgHook );

	virtual DetourHandle_t	HookEvent( const char *pszName, EventHookFn pfnEventHook, EventHookFn *ppfnOriginalEventHook );
	virtual DetourHandle_t	HookEvent( struct event_hook_s *pEventHook, EventHookFn pfnEventHook, EventHookFn *ppfnOriginalEventHook );
	virtual bool			UnhookEvent( DetourHandle_t hEventHook );

	virtual DetourHandle_t	HookConsoleCommand( const char *pszName, CommandCallbackFn pfnCommandCallback, CommandCallbackFn *ppfnOriginalCommandCallback );
	virtual DetourHandle_t	HookConsoleCommand( struct cmd_function_s *pCommand, CommandCallbackFn pfnCommandCallback, CommandCallbackFn *ppfnOriginalCommandCallback );
	virtual bool			UnhookConsoleCommand( DetourHandle_t hCommandCallback );

protected:
	void CallCvarChangeChain( struct cvar_s *pCvar, const char *pszOldValue, float flOldValue );

private:
	CHashTable<struct cvar_s *, std::vector<CvarChangeHookFn>> m_hCvarChangeHooks;
};

CGameHooks *GameHooks( void );

#endif // SINT_GAME_HOOKS_H

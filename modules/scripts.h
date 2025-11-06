// SvenInt (c) Sw1ft
// scripts.h

#ifndef SINT_SCRIPTS_MODULE_H
#define SINT_SCRIPTS_MODULE_H

#ifdef _WIN32
#pragma once
#endif

#include "scripts/lua/lua.hpp"

struct scriptref { int unused; }; typedef struct scriptref *scriptref_t;

//-----------------------------------------------------------------------------
// Scripts Callbacks
//-----------------------------------------------------------------------------

class CScriptCallbacks
{
public:
	void OnGameFrame( int state, double frametime, bool bPostRunCmd );

	void OnFirstClientdataReceived( float flTime );

	void OnBeginLoading( void );
	void OnEndLoading( void );

	void OnDisconnect( void );
	void OnRestart( void );

	void OnEntityUse( struct edict_s *pEntityUseEdict, struct edict_s *pEntityEdict );
	void OnEntityTouch( struct edict_s *pEntityTouchEdict, struct edict_s *pEntityEdict );
	void OnFireTargets( const char *pszTargetName, void *pActivator, void *pCaller, int useType, float flValue, float flDelay );

	void OnClientPutInServer( struct edict_s *pPlayerEdict );

	void OnPlayerSpawn( struct edict_s *pSpawnSpotEdict, struct edict_s *pPlayerEdict );
	void OnSpecialSpawn( struct edict_s *pPlayerEdict );
	void OnPlayerUnstuck( struct edict_s *pPlayerEdict );
	void OnBeginPlayerRevive( struct edict_s *pPlayerEdict );
	void OnEndPlayerRevive( struct edict_s *pPlayerEdict );
	void OnClientKill( struct edict_s *pPlayerEdict );

	void OnServerSignal( int value );
	void OnClientSignal( struct edict_s *pPlayerEdict, int value );
	void OnFireBulletsSpread( struct edict_s *pEntity, float *x, float *y );
	bool OnFilterAimbotTarget( int entindex );

	// Input Manager
	void OnPlayInput( const char *pszFilename, int frame, struct usercmd_s *cmd );
	void OnPlayEnd( const char *pszFilename, int frames );
};

//-----------------------------------------------------------------------------
// Scripts module
//-----------------------------------------------------------------------------

class CScriptsModule
{
	friend CScriptCallbacks;
	friend class CTimersHandler;
	friend class CClientTriggerManager;

public:
	CScriptsModule();

	bool		Init( void );
	void		Shutdown( void );
	
public:
	bool		InitVM( void );
	void		ShutdownVM( void );

	lua_State *GetVM( void );
	CScriptCallbacks *Callbacks( void );

	bool ClientCommand( struct edict_s *pPlayer );

	void		ResetStates( void );
	void		SetSearchPath( const char *pszSearchPath );

	bool		RunScript( const char *pszScript );
	bool		RunScriptFile( const char *pszFilename );

	scriptref_t LookupFunction( const char *pszFunction );
	void		ReleaseFunction( scriptref_t hFunction );

	void		ProtectedCall( lua_State *pLuaState, int args, int results, int errfunc );

private:
	void		PrintError( void );
	void		DumpStack( void );

private:
	lua_State *m_pLuaState;
};

namespace Modules { extern CScriptsModule *scripts; }

#endif // SINT_SCRIPTS_MODULE_H
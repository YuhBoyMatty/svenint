// SvenInt (c) Sw1ft
// scripts.cpp

#include "stdafx.h"
#include "svenint.h"
#include "scripts.h"

#include "scripts/scripts_binding.h"
#include "scripts/lua_debug.h"
#include "scripts/lua_vector.h"
#include "scripts/lua_cvar.h"
#include "scripts/lua_mod.h"
#include "scripts/lua_random.h"
#include "scripts/lua_logic.h"
#include "scripts/lua_triggers.h"
#include "scripts/lua_entity_dictionary.h"
#include "scripts/lua_entity_vars.h"
#include "scripts/lua_global_vars.h"
#include "scripts/lua_player_move.h"
#include "scripts/lua_usercmd.h"
#include "scripts/lua_input_manager.h"

#include "features/r_draw_context.h"

namespace Modules { static CScriptsModule scriptsModule; CScriptsModule *scripts = &scriptsModule; }

//-----------------------------------------------------------------------------
// Color print
//-----------------------------------------------------------------------------

static const Color clr_print( 80, 186, 255, 255 );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_enable_scripts( "sc_enable_scripts",
					  #ifdef _DEBUG // FIXME: game crash at VM startup
						  "0",
					  #else
						  "1",
					  #endif
						  FCVAR_EXTDLL,
						  "Enable scripts virtual machine" );

CON_COMMAND( sc_script, "Execute a script line" )
{
	if ( args.ArgC() > 1 )
	{
		if ( Modules::scripts->GetVM() == NULL )
		{
			ConColorMsg( clr_print, "Scripts virtual machine is not running\n" );
			return;
		}

		if ( args.ArgC() > 2 )
		{
			std::string sScript = args[ 1 ];

			for ( int i = 2; i < args.ArgC(); i++ )
			{
				sScript += " ";
				sScript += args[ i ];
			}

			Modules::scripts->RunScript( sScript.c_str() );
		}
		else
		{
			Modules::scripts->RunScript( args[ 1 ] );
		}
	}
	else
	{
		Msg( "Usage:  sc_script <script>\n" );
	}
}

CON_COMMAND( sc_script_execute, "Execute a script file" )
{
	if ( args.ArgC() > 1 )
	{
		if ( Modules::scripts->GetVM() == NULL )
		{
			ConColorMsg( clr_print, "Scripts virtual machine is not running\n" );
			return;
		}

		//Modules::scripts->RunScriptFile( args[1] );

		auto ends_with = []( std::string const &value, std::string const &ending ) -> bool
		{
			if ( ending.size() > value.size() )
				return false;

			return std::equal( ending.rbegin(), ending.rend(), value.rbegin() );
		};

		const char *pszFilename = args[ 1 ];

		std::string sFilePath = UTIL_GetLongPathName();

		sFilePath += "\\" SVENINT_FOLDER_NAME "\\scripts\\";
		sFilePath += pszFilename;

		if ( !ends_with( pszFilename, ".lua" ) )
		{
			sFilePath += ".lua";
		}

		Modules::scripts->RunScriptFile( sFilePath.c_str() );
	}
	else
	{
		Msg( "Usage:  sc_script_execute <filename>\n" );
	}
}

CON_COMMAND( sc_set_trigger_point, "Sets a point of trigger" )
{
	static bool s_bFirstPointSet = false;
	static Vector s_bFirstPoint;

	if ( !s_bFirstPointSet )
	{
		s_bFirstPoint = Globals::localplayer->GetEyePosition();
		s_bFirstPointSet = true;

		Msg( "First trigger point has ben set\n" );
		Globals::gameutils->PrintChatText( "First trigger point has been set\n" );
	}
	else
	{
		Vector vecMins, vecMaxs;

		Vector v1 = s_bFirstPoint;
		Vector v2 = Globals::localplayer->GetEyePosition();

		Vector vecPos = v1;
		float xDiff = v2.x - v1.x;
		float yDiff = v2.y - v1.y;
		float zDiff = v2.z - v1.z;

		if ( xDiff < 0 && yDiff < 0 )
		{
			xDiff *= -1;
			yDiff *= -1;
			vecPos.x -= xDiff;
			vecPos.y -= yDiff;
		}
		else if ( xDiff > 0 && yDiff < 0 )
		{
			yDiff *= -1;
			vecPos.y -= yDiff;
		}
		else if ( xDiff < 0 && yDiff > 0 )
		{
			xDiff *= -1;
			vecPos.x -= xDiff;
		}

		if ( zDiff > 0 )
		{
			vecMins.Zero();

			vecMaxs.x = xDiff;
			vecMaxs.y = yDiff;
			vecMaxs.z = zDiff;

			ConColorMsg( { 255, 255, 0, 255 }, "Dump: CreateTrigger(\"trigger\", Vector(%.3f, %.3f, %.3f), Vector(0, 0, 0), Vector(%.3f, %.3f, %.3f));\n", VectorExpand( vecPos ), xDiff, yDiff, zDiff );
		}
		else
		{
			vecMins.x = 0.f;
			vecMins.y = 0.f;
			vecMins.z = zDiff;

			vecMaxs.x = xDiff;
			vecMaxs.y = yDiff;
			vecMaxs.z = 0.f;

			ConColorMsg( { 255, 255, 0, 255 }, "Dump: CreateTrigger(\"trigger\", Vector(%.3f, %.3f, %.3f), Vector(0, 0, %.3f), Vector(%.3f, %.3f, 0));\n", VectorExpand( vecPos ), zDiff, xDiff, yDiff );
		}

		Features::drawcontext->DrawBox( vecPos, vecMins, vecMaxs, { 255, 255, 0, 100 }, 10.f );

		s_bFirstPointSet = false;
	}
}

//-----------------------------------------------------------------------------
// Scripts Callbacks
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnGameFrame( int state, double frametime, bool bPostRunCmd )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL || Globals::demoplayback )
		return;

	if ( !bPostRunCmd )
	{
		// Collect garbage
		lua_gc( pLuaState, LUA_GCCOLLECT );

		g_TimersHandler.Frame( pLuaState );
		g_LuaTriggerManager.Frame( pLuaState );
	}

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnGameFrame" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushinteger( pLuaState, (lua_Integer)state );
		lua_pushnumber( pLuaState, (lua_Number)frametime );
		lua_pushboolean( pLuaState, bPostRunCmd );

		Modules::scripts->ProtectedCall( pLuaState, 3, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Spawned on the server
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnFirstClientdataReceived( float flTime )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL || Globals::demoplayback )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnFirstClientdataReceived" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushnumber( pLuaState, (lua_Number)flTime );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Loading has started
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnBeginLoading( void )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL || Globals::demoplayback )
		return;

	scriptref_t hFunction;

	Modules::scripts->ResetStates();

	if ( hFunction = Modules::scripts->LookupFunction( "OnBeginLoading" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		Modules::scripts->ProtectedCall( pLuaState, 0, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Loading has finished
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnEndLoading( void )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL || Globals::demoplayback )
		return;

	scriptref_t hFunction;

	Modules::scripts->ResetStates();

	if ( hFunction = Modules::scripts->LookupFunction( "OnEndLoading" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		Modules::scripts->ProtectedCall( pLuaState, 0, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Disconnected from the server
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnDisconnect( void )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL || Globals::demoplayback )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnDisconnect" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		Modules::scripts->ProtectedCall( pLuaState, 0, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
//		Server-side callbacks
// 
// Pressed 'restart' console command
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnRestart( void )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState != NULL && !Globals::demoplayback )
	{
		scriptref_t hFunction;

		//Modules::scripts->ResetStates();

		if ( hFunction = Modules::scripts->LookupFunction( "OnRestart" ) )
		{
			lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

			Modules::scripts->ProtectedCall( pLuaState, 0, 0, 0 );
			Modules::scripts->ReleaseFunction( hFunction );
		}
	}

	Modules::scripts->ShutdownVM();
	Modules::scripts->InitVM();
}

//-----------------------------------------------------------------------------
// Entity use
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnEntityUse( edict_t *pEntityUseEdict, edict_t *pEntityEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnEntityUse" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pEntityUseEdict );
		lua_pushedict( pLuaState, pEntityEdict );

		Modules::scripts->ProtectedCall( pLuaState, 2, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Entity touch
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnEntityTouch( edict_t *pEntityTouchEdict, edict_t *pEntityEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnEntityTouch" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pEntityTouchEdict );
		lua_pushedict( pLuaState, pEntityEdict );

		Modules::scripts->ProtectedCall( pLuaState, 2, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// A player has joined the server
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnClientPutInServer( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnClientPutInServer" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// A player just spawned
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnPlayerSpawn( edict_t *pSpawnSpotEdict, edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnPlayerSpawn" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pSpawnSpotEdict );
		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 2, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Unstuck callback
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnPlayerUnstuck( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnPlayerUnstuck" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// A player has been revived
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnSpecialSpawn( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnSpecialSpawn" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Revival of a player has started
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnBeginPlayerRevive( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnBeginPlayerRevive" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Revival of a player has ended
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnEndPlayerRevive( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnEndPlayerRevive" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// A player died
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnClientKill( edict_t *pPlayerEdict )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnClientKill" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Fire targets I/O system
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnFireTargets( const char *pszTargetName, void *pActivator, void *pCaller, int useType, float flValue, float flDelay )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnFireTargets" ) )
	{
		entvars_t *pev;
		edict_t *activator, *caller;

		if ( pActivator != NULL )
		{
			pev = *(entvars_t **)( (unsigned long *)pActivator + 1 );
			activator = Globals::sv_enginefuncs->pfnFindEntityByVars( pev );
		}
		else
		{
			activator = NULL;
		}

		if ( pCaller != NULL )
		{
			pev = *(entvars_t **)( (unsigned long *)pCaller + 1 );
			caller = Globals::sv_enginefuncs->pfnFindEntityByVars( pev );
		}
		else
		{
			caller = NULL;
		}

		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushstring( pLuaState, pszTargetName );
		lua_pushedict( pLuaState, activator );
		lua_pushedict( pLuaState, caller );
		lua_pushinteger( pLuaState, useType );
		lua_pushnumber( pLuaState, flValue );
		lua_pushnumber( pLuaState, flDelay );

		Modules::scripts->ProtectedCall( pLuaState, 6, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Receive server's signal callback
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnServerSignal( int value )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnServerSignal" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushinteger( pLuaState, value );

		Modules::scripts->ProtectedCall( pLuaState, 1, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// sc_sendsignal callback
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnClientSignal( edict_t *pPlayerEdict, int value )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnClientSignal" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushedict( pLuaState, pPlayerEdict );
		lua_pushinteger( pLuaState, value );

		Modules::scripts->ProtectedCall( pLuaState, 2, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Adjust spread when bullets are about to fire
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnFireBulletsSpread( struct edict_s *pEntity, float *x, float *y )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hCallbackFunction;

	if ( hCallbackFunction = Modules::scripts->LookupFunction( "OnFireBulletsSpread" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hCallbackFunction );

		lua_pushedict( pLuaState, pEntity );
		lua_pushinteger( pLuaState, (lua_Integer)Globals::sv_enginefuncs->pfnIndexOfEdict( pEntity ) );
		lua_pushnumber( pLuaState, (lua_Number)*x );
		lua_pushnumber( pLuaState, (lua_Number)*y );

		Modules::scripts->ProtectedCall( pLuaState, 4, 2, 0 );

		if ( lua_isnumber( pLuaState, -2 ) && lua_isnumber( pLuaState, -1 ) )
		{
			*x = (float)lua_tonumber( pLuaState, -2 );
			*y = (float)lua_tonumber( pLuaState, -1 );
		}

		Modules::scripts->ReleaseFunction( hCallbackFunction );
	}
}

//-----------------------------------------------------------------------------
// Aimbot feature filter given target
//-----------------------------------------------------------------------------

bool CScriptCallbacks::OnFilterAimbotTarget( int entindex )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return true;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnFilterAimbotTarget" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushinteger( pLuaState, entindex );

		Modules::scripts->ProtectedCall( pLuaState, 1, 1, 0 );

		bool result = ( lua_isboolean( pLuaState, -1 ) ? lua_toboolean( pLuaState, -1 ) : true );

		Modules::scripts->ReleaseFunction( hFunction );
		return result;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Input manager playback an input
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnPlayInput( const char *pszFilename, int frame, usercmd_t *cmd )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnPlayInput" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushstring( pLuaState, pszFilename );
		lua_pushinteger( pLuaState, (lua_Integer)frame );
		lua_pushusercmd( pLuaState, cmd );

		Modules::scripts->ProtectedCall( pLuaState, 3, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

//-----------------------------------------------------------------------------
// Input manager playback end
//-----------------------------------------------------------------------------

void CScriptCallbacks::OnPlayEnd( const char *pszFilename, int frames )
{
	lua_State *pLuaState = Modules::scripts->GetVM();

	if ( pLuaState == NULL )
		return;

	scriptref_t hFunction;

	if ( hFunction = Modules::scripts->LookupFunction( "OnPlayEnd" ) )
	{
		lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hFunction );

		lua_pushstring( pLuaState, pszFilename );
		lua_pushinteger( pLuaState, (lua_Integer)frames );

		Modules::scripts->ProtectedCall( pLuaState, 2, 0, 0 );
		Modules::scripts->ReleaseFunction( hFunction );
	}
}

static CScriptCallbacks gScriptCallbacks;

//-----------------------------------------------------------------------------
// Init virtual machine
//-----------------------------------------------------------------------------

bool CScriptsModule::InitVM( void )
{
	if ( !sc_enable_scripts.GetBool() || Globals::demoplayback )
		return false;

	if ( m_pLuaState == NULL )
	{
		m_pLuaState = luaL_newstate();

		if ( m_pLuaState != NULL )
		{
			VLua::Init( m_pLuaState );

			luaL_openlibs( m_pLuaState );

			luaopen_print( m_pLuaState );
			luaopen_vector( m_pLuaState );
			luaopen_random( m_pLuaState );
			luaopen_playermove( m_pLuaState );
			luaopen_usercmd( m_pLuaState );
			luaopen_edict( m_pLuaState );
			luaopen_entvars( m_pLuaState );
			luaopen_globalvars( m_pLuaState );
			luaopen_cvar( m_pLuaState );
			luaopen_logic( m_pLuaState );
			luaopen_triggers( m_pLuaState );
			luaopen_inputmanager( m_pLuaState );
			luaopen_mod( m_pLuaState );

			VLua::SetupPropertiesInitialization();

			// SetSearchPath( SVENINT_FOLDER_NAME "\\scripts" );

			ConColorMsg( clr_print, "Started scripts virtual machine using scripting language \"" LUA_VERSION "\"\n" );

			ConColorMsg( clr_print, "Running a script file \"main.lua\"...\n" );
			RunScriptFile( SVENINT_FOLDER_NAME "/scripts/main.lua" );

			return true;
		}

		Warning( "Failed to start scripts virtual machine\n" );
	}
	else
	{
		// VM already exists
		ResetStates();
	}

	return false;
}

//-----------------------------------------------------------------------------
// Shutdown virtual machine
//-----------------------------------------------------------------------------

void CScriptsModule::ShutdownVM( void )
{
	if ( m_pLuaState != NULL )
	{
		ResetStates();

		lua_close( m_pLuaState );

		VLua::Shutdown();

		m_pLuaState = NULL;

		ConColorMsg( clr_print, "Stopped scripts virtual machine\n" );
	}
}

//-----------------------------------------------------------------------------
// Reset states
//-----------------------------------------------------------------------------

void CScriptsModule::ResetStates( void )
{
	if ( m_pLuaState == NULL )
		return;

	lua_setcurrentmap( m_pLuaState );

	g_TimersHandler.ClearTimers();
	g_LuaTriggerManager.ClearTriggers();
	g_LuaTriggerManager.ClearServerTriggers();
}

//-----------------------------------------------------------------------------
// Get pointer to virtual machine
//-----------------------------------------------------------------------------

lua_State *CScriptsModule::GetVM( void )
{
	return m_pLuaState;
}

//-----------------------------------------------------------------------------
// Get script callbacks
//-----------------------------------------------------------------------------

CScriptCallbacks *CScriptsModule::Callbacks( void )
{
	return &gScriptCallbacks;
}

//-----------------------------------------------------------------------------
// ClientCommand callback
//-----------------------------------------------------------------------------

bool CScriptsModule::ClientCommand( struct edict_s *pPlayer )
{
	if ( !strcmp( Globals::sv_enginefuncs->pfnCmd_Argv( 0 ), "sc_sendsignal" ) )
	{
		if ( Globals::sv_enginefuncs->pfnCmd_Argc() >= 2 )
			Callbacks()->OnClientSignal( pPlayer, atoi( Globals::sv_enginefuncs->pfnCmd_Argv( 1 ) ) );

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Set search path (not finished)
//-----------------------------------------------------------------------------

void CScriptsModule::SetSearchPath( const char *pszSearchPath )
{
	lua_getglobal( m_pLuaState, "package" );

	if ( lua_istable( m_pLuaState, -1 ) )
	{
		lua_getfield( m_pLuaState, -1, "path" );

		if ( lua_isstring( m_pLuaState, -1 ) )
		{
			std::string sPath = UTIL_GetLongPathName();

			sPath += "\\";
			sPath += pszSearchPath;
			sPath += "\\?.lua";
			sPath += ";.\\";
			sPath += pszSearchPath;
			sPath += "\\?.lua";

			lua_pushstring( m_pLuaState, sPath.c_str() );
			lua_setfield( m_pLuaState, -3, "path" );

			lua_pop( m_pLuaState, 2 );
		}
		else
		{
			//AssertMsg( 0, "path" );
			lua_pop( m_pLuaState, 1 );
		}
	}
	else
	{
		//AssertMsg( 0, "package" );
		lua_pop( m_pLuaState, 1 );
	}
}

//-----------------------------------------------------------------------------
// Execute a script line
//-----------------------------------------------------------------------------

bool CScriptsModule::RunScript( const char *pszScript )
{
	if ( m_pLuaState == NULL )
		return false;

	int luaResult = luaL_dostring( m_pLuaState, pszScript );

	if ( luaResult != LUA_OK )
	{
		PrintError();
		DumpStack();

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Execute a script file
//-----------------------------------------------------------------------------

bool CScriptsModule::RunScriptFile( const char *pszFilename )
{
	if ( m_pLuaState == NULL )
		return false;

	int luaResult = luaL_dofile( m_pLuaState, pszFilename );

	if ( luaResult != LUA_OK )
	{
		PrintError();
		DumpStack();

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Lookup for a script function
//-----------------------------------------------------------------------------

scriptref_t CScriptsModule::LookupFunction( const char *pszFunction )
{
	if ( m_pLuaState != NULL )
	{
		lua_getglobal( m_pLuaState, pszFunction );

		if ( !( lua_isfunction( m_pLuaState, -1 ) || lua_iscfunction( m_pLuaState, -1 ) ) )
		{
			lua_pop( m_pLuaState, 1 );
			return NULL;
		}

		scriptref_t func = (scriptref_t)luaL_ref( m_pLuaState, LUA_REGISTRYINDEX );

		return func;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Release reference to a script function
//-----------------------------------------------------------------------------

void CScriptsModule::ReleaseFunction( scriptref_t hFunction )
{
	if ( m_pLuaState != NULL )
	{
		luaL_unref( m_pLuaState, LUA_REGISTRYINDEX, (int)hFunction );
	}
}

//-----------------------------------------------------------------------------
// Protected call
//-----------------------------------------------------------------------------

void CScriptsModule::ProtectedCall( lua_State *pLuaState, int args, int results, int errfunc )
{
	int luaResult = lua_pcall( pLuaState, args, results, errfunc );

	if ( luaResult != LUA_OK )
	{
		Modules::scripts->PrintError();
	}
}

//-----------------------------------------------------------------------------
// Print an error
//-----------------------------------------------------------------------------

void CScriptsModule::PrintError( void )
{
	Warning( "\nAN ERROR HAS OCCURRED [ %s ]\n\n", lua_tostring( m_pLuaState, -1 ) );
}

//-----------------------------------------------------------------------------
// Dump stack
//-----------------------------------------------------------------------------

void CScriptsModule::DumpStack( void )
{
	Warning( "STACK TRACE:\n" );

	const int level = 5;

	int top = lua_gettop( m_pLuaState );
	int stack_end = top - level;

	if ( stack_end < 1 )
		stack_end = 1;

	for ( int i = top; i >= stack_end; i-- )
	{
		int type = lua_type( m_pLuaState, i );

		Warning( "%d. ", i );

		switch ( type )
		{
		case LUA_TNUMBER:
			Warning( "%g", lua_tonumber( m_pLuaState, i ) );
			break;

		case LUA_TSTRING:
			Warning( "`%s'", lua_tostring( m_pLuaState, i ) );
			break;

		case LUA_TBOOLEAN:
			Warning( lua_toboolean( m_pLuaState, i ) ? "true" : "false" );
			break;

		case LUA_TNIL:
			Warning( "nil" );
			break;

		default:
			Warning( "%s", lua_typename( m_pLuaState, type ) );
			break;
		}

		Warning( "\n\n" );
	}
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

CScriptsModule::CScriptsModule()
{
	m_pLuaState = NULL;
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CScriptsModule::Init( void )
{
	Globals::cvar->RegisterConCommand( &sc_enable_scripts );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_script ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_script_execute ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_set_trigger_point ) );

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CScriptsModule::Shutdown( void )
{
	ShutdownVM();

	Globals::cvar->UnregisterConCommand( &sc_enable_scripts );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_script ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_script_execute ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_set_trigger_point ) );
}
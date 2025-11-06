// SvenInt (c) Sw1ft
// lua_triggers.cpp

#include "stdafx.h"
#include "lua_triggers.h"
#include "lua_vector.h"
#include "lua_entity_dictionary.h"
#include "scripts_binding.h"
#include "modules/server.h"

//-----------------------------------------------------------------------------
// CLuaTriggerManager
//-----------------------------------------------------------------------------

CLuaTriggerManager g_LuaTriggerManager;

CLuaTriggerManager::CLuaTriggerManager()
{
}

void CLuaTriggerManager::Frame( lua_State *pLuaState )
{
	TriggersThink( pLuaState );

	if ( Modules::server->Host_IsServerActive() )
		ServerTriggersThink( pLuaState );
}

void CLuaTriggerManager::AddTrigger( const char *pszName, const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs )
{
	Lua_Trigger trigger;

	trigger.name = pszName;
	trigger.origin = vecOrigin;
	trigger.mins = vecMins;
	trigger.maxs = vecMaxs;

	m_vTriggers.push_back( trigger );
}

void CLuaTriggerManager::AddServerTrigger( const char *pszName, const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs )
{
	Lua_Trigger trigger;

	trigger.name = pszName;
	trigger.origin = vecOrigin;
	trigger.mins = vecMins;
	trigger.maxs = vecMaxs;

	m_vServerTriggers.push_back( trigger );
}

void CLuaTriggerManager::ClearTriggers()
{
	m_vTriggers.clear();
}

void CLuaTriggerManager::ClearServerTriggers()
{
	m_vServerTriggers.clear();
}

//-----------------------------------------------------------------------------
// Think
//-----------------------------------------------------------------------------

void CLuaTriggerManager::TriggersThink( lua_State *pLuaState )
{
	scriptref_t hCallbackFunction;

	Vector vecOrigin;
	Vector vecMins, vecMaxs;

	VectorCopy( Globals::localplayer->GetOrigin(), vecOrigin );
	VectorCopy( Globals::localplayer->GetOrigin(), vecMins );
	VectorCopy( Globals::localplayer->GetOrigin(), vecMaxs );

	VectorAdd( vecMins, ( Globals::localplayer->GetFlags() & FL_DUCKING ) ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN, vecMins );
	VectorAdd( vecMaxs, ( Globals::localplayer->GetFlags() & FL_DUCKING ) ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX, vecMaxs );

	//VectorAdd( vecMins, g_pPlayerMove->player_mins[ g_pPlayerMove->usehull ], vecMins );
	//VectorAdd( vecMaxs, g_pPlayerMove->player_maxs[ g_pPlayerMove->usehull ], vecMaxs );

	// Nothing to do here when we don't have a callback function
	if ( m_vTriggers.size() == 0 || !( hCallbackFunction = Modules::scripts->LookupFunction( "OnTouchTrigger" ) ) )
		return;

	for ( size_t i = 0; i < m_vTriggers.size(); i++ )
	{
		Lua_Trigger &trigger = m_vTriggers[ i ];

		Vector vecTriggerMins = trigger.origin + trigger.mins;
		Vector vecTriggerMaxs = trigger.origin + trigger.maxs;

		if ( UTIL_IsAABBIntersectingAABB( vecMins, vecMaxs, vecTriggerMins, vecTriggerMaxs ) )
		{
			if ( hCallbackFunction )
			{
				lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hCallbackFunction );

				lua_pushstring( pLuaState, trigger.name.c_str() );

				Modules::scripts->ProtectedCall( pLuaState, 1, 1, 0 );

				bool bRemoveTrigger = ( lua_isboolean( pLuaState, -1 ) ? lua_toboolean( pLuaState, -1 ) : false );

				if ( bRemoveTrigger )
				{
					trigger.name.erase();
					m_vTriggers.erase( m_vTriggers.begin() + i );
					i--;
				}
			}
		}
	}

	Modules::scripts->ReleaseFunction( hCallbackFunction );
}

void CLuaTriggerManager::ServerTriggersThink( lua_State *pLuaState )
{
	scriptref_t hCallbackFunction;

	Vector vecOrigin;
	Vector vecMins, vecMaxs;

	// Nothing to do here when we don't have a callback function
	if ( m_vServerTriggers.size() == 0 || !( hCallbackFunction = Modules::scripts->LookupFunction( "OnTouchServerTrigger" ) ) )
		return;

	for ( size_t i = 0; i < m_vServerTriggers.size(); i++ )
	{
		Lua_Trigger &trigger = m_vServerTriggers[ i ];

		Vector vecTriggerMins = trigger.origin + trigger.mins;
		Vector vecTriggerMaxs = trigger.origin + trigger.maxs;

		for ( int j = 1; j <= Globals::gpGlobals->maxClients; j++ )
		{
			edict_t *pPlayer = Globals::sv_enginefuncs->pfnPEntityOfEntIndex( j );

			if ( !Modules::server->IsValidEntity( pPlayer ) )
				continue;

			VectorCopy( pPlayer->v.origin, vecOrigin );
			VectorCopy( pPlayer->v.origin, vecMins );
			VectorCopy( pPlayer->v.origin, vecMaxs );

			VectorAdd( vecMins, pPlayer->v.mins, vecMins );
			VectorAdd( vecMaxs, pPlayer->v.maxs, vecMaxs );

			if ( UTIL_IsAABBIntersectingAABB( vecMins, vecMaxs, vecTriggerMins, vecTriggerMaxs ) )
			{
				lua_rawgeti( pLuaState, LUA_REGISTRYINDEX, (int)hCallbackFunction );

				lua_pushedict( pLuaState, pPlayer );
				lua_pushstring( pLuaState, trigger.name.c_str() );

				Modules::scripts->ProtectedCall( pLuaState, 2, 1, 0 );

				bool bRemoveTrigger = lua_toboolean( pLuaState, -1 );

				if ( bRemoveTrigger )
				{
					trigger.name.erase();

					m_vServerTriggers.erase( m_vServerTriggers.begin() + i );
					i--;

					break;
				}
			}
		}
	}

	Modules::scripts->ReleaseFunction( hCallbackFunction );
}

//-----------------------------------------------------------------------------
// C to Lua
//-----------------------------------------------------------------------------

DEFINE_SCRIPTFUNC( CreateTrigger )
{
	const char *pszName = lua_tostring( pLuaState, 1 );
	Vector *vecOrigin = lua_getvector( pLuaState, 2 );
	Vector *vecMins = lua_getvector( pLuaState, 3 );
	Vector *vecMaxs = lua_getvector( pLuaState, 4 );

	g_LuaTriggerManager.AddTrigger( pszName, *vecOrigin, *vecMins, *vecMaxs );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( CreateServerTrigger )
{
	const char *pszName = lua_tostring( pLuaState, 1 );
	Vector *vecOrigin = lua_getvector( pLuaState, 2 );
	Vector *vecMins = lua_getvector( pLuaState, 3 );
	Vector *vecMaxs = lua_getvector( pLuaState, 4 );

	g_LuaTriggerManager.AddServerTrigger( pszName, *vecOrigin, *vecMins, *vecMaxs );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( ClearTriggers )
{
	g_LuaTriggerManager.ClearTriggers();

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( ClearServerTriggers )
{
	g_LuaTriggerManager.ClearServerTriggers();

	return VLUA_RET_ARGS( 0 );
}

//-----------------------------------------------------------------------------
// Init lib
//-----------------------------------------------------------------------------

LUALIB_API int luaopen_triggers( lua_State *pLuaState )
{
	VLua::RegisterFunction( "CreateTrigger", SCRIPTFUNC( CreateTrigger ) );
	VLua::RegisterFunction( "CreateServerTrigger", SCRIPTFUNC( CreateServerTrigger ) );
	VLua::RegisterFunction( "ClearTriggers", SCRIPTFUNC( ClearTriggers ) );
	VLua::RegisterFunction( "ClearServerTriggers", SCRIPTFUNC( ClearServerTriggers ) );

	return 1;
}
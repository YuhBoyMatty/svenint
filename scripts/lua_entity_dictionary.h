// SvenInt (c) Sw1ft
// lua_entity_dictionary.h

#ifndef SINT_LUA_EDICT_H
#define SINT_LUA_EDICT_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_edict( lua_State *pLuaState );
extern bool lua_isedict( lua_State *pLuaState, int i );
extern struct edict_s *lua_getedict( lua_State *pLuaState, int i );
extern void lua_pushedict( lua_State *pLuaState, struct edict_s *pEdict );

#endif // SINT_LUA_EDICT_H
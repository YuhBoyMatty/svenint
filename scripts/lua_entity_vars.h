// SvenInt (c) Sw1ft
// lua_entity_vars.h

#ifndef SINT_LUA_ENTVARS_H
#define SINT_LUA_ENTVARS_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_entvars( lua_State *pLuaState );
extern bool lua_isentvars( lua_State *pLuaState, int i );
extern struct entvars_s *lua_getentvars( lua_State *pLuaState, int i );
extern void lua_pushentvars( lua_State *pLuaState, struct entvars_s *pEntityVars );

#endif // SINT_LUA_ENTVARS_H
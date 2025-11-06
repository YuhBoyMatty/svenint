// SvenInt (c) Sw1ft
// lua_global_vars.h

#ifndef SINT_LUA_GLOBALVARS_H
#define SINT_LUA_GLOBALVARS_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_globalvars( lua_State *pLuaState );
extern bool lua_isglobalvars( lua_State *pLuaState, int i );
extern globalvars_t *lua_getglobalvars( lua_State *pLuaState, int i );
extern void lua_pushglobalvars( lua_State *pLuaState, globalvars_t *pGlobals );

#endif // SINT_LUA_GLOBALVARS_H
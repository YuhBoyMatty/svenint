// SvenInt (c) Sw1ft
// lua_mod.h

#ifndef SINT_LUA_MOD_H
#define SINT_LUA_MOD_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern void lua_setcurrentmap( lua_State *pLuaState );
extern int luaopen_mod(lua_State *L);

#endif // SINT_LUA_MOD_H
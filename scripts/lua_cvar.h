// SvenInt (c) Sw1ft
// lua_cvar.h

#ifndef SINT_LUA_CVAR_H
#define SINT_LUA_CVAR_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_cvar(lua_State *L);

#endif // SINT_LUA_CVAR_H
// SvenInt (c) Sw1ft
// lua_debug.h

#ifndef SINT_LUA_DEBUG_H
#define SINT_LUA_DEBUG_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_print(lua_State *L);

#endif // SINT_LUA_DEBUG_H
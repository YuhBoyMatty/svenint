// SvenInt (c) Sw1ft
// lua_random.h

#ifndef SINT_LUA_RANDOM_H
#define SINT_LUA_RANDOM_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_random(lua_State *L);

#endif // SINT_LUA_RANDOM_H
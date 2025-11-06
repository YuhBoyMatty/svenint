// SvenInt (c) Sw1ft
// lua_usercmd.h

#ifndef SINT_LUA_USERCMD_H
#define SINT_LUA_USERCMD_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_usercmd( lua_State *pLuaState );
extern bool lua_isusercmd( lua_State *pLuaState, int i );
extern struct usercmd_s *lua_getusercmd( lua_State *pLuaState, int i );
extern void lua_pushusercmd( lua_State *pLuaState, struct usercmd_s *cmd );

#endif // SINT_LUA_USERCMD_H
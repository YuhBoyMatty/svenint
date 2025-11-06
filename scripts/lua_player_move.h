// SvenInt (c) Sw1ft
// lua_player_move.h

#ifndef SINT_LUA_PLAYERMOVE_H
#define SINT_LUA_PLAYERMOVE_H

#ifdef _WIN32
#pragma once
#endif

#include "lua/lua.hpp"

extern int luaopen_playermove( lua_State *pLuaState );
extern bool lua_isplayermove( lua_State *pLuaState, int i );
extern struct playermove_s *lua_getplayermove( lua_State *pLuaState, int i );
extern void lua_pushplayermove( lua_State *pLuaState, struct playermove_s *pPlayerMove );

#endif // SINT_
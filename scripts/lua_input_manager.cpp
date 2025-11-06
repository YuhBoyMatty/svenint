// SvenInt (c) Sw1ft
// lua_input_manager.cpp

#include "stdafx.h"
#include "scripts_binding.h"
#include "lua_input_manager.h"

#include "features/st_input_manager.h"

//-----------------------------------------------------------------------------
// Script funcs
//-----------------------------------------------------------------------------

DEFINE_SCRIPTFUNC( Record )
{
	const char *pszFilename = lua_tostring( pLuaState, 1 );

	lua_pushboolean( pLuaState, Features::inputmanager->Record( pszFilename ) );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( Playback )
{
	const char *pszFilename = lua_tostring( pLuaState, 1 );

	lua_pushboolean( pLuaState, Features::inputmanager->Playback( pszFilename ) );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( Split )
{
	lua_pushboolean( pLuaState, Features::inputmanager->Split() );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( Goto )
{
	int iFrame = (int)lua_tointeger( pLuaState, 1 );

	Features::inputmanager->Goto( iFrame );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( Forward )
{
	int iFrames = (int)lua_tointeger( pLuaState, 1 );

	Features::inputmanager->Forward( iFrames );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( Backward )
{
	int iFrames = (int)lua_tointeger( pLuaState, 1 );

	Features::inputmanager->Backward( iFrames );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( Stop )
{
	lua_pushboolean( pLuaState, Features::inputmanager->Stop() );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( RecordCommand )
{
	const char *pszCommand = lua_tostring( pLuaState, 1 );

	Features::inputmanager->RecordCommand( pszCommand );

	return VLUA_RET_ARGS( 0 );
}

DEFINE_SCRIPTFUNC( IsInAction )
{
	lua_pushboolean( pLuaState, Features::inputmanager->IsInAction() );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( IsRecording )
{
	lua_pushboolean( pLuaState, Features::inputmanager->IsRecording() );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( IsPlayingback )
{
	lua_pushboolean( pLuaState, Features::inputmanager->IsPlayingback() );

	return VLUA_RET_ARGS( 1 );
}

DEFINE_SCRIPTFUNC( GetCurrentFrame )
{
	lua_pushinteger( pLuaState, (lua_Integer)Features::inputmanager->GetCurrentFrame() );

	return VLUA_RET_ARGS( 1 );
}

REG_BEGIN( Registrations )
	REG_SCRIPTFUNC( "Record", Record )
	REG_SCRIPTFUNC( "Playback", Playback )
	REG_SCRIPTFUNC( "Split", Split )
	REG_SCRIPTFUNC( "Stop", Stop )
	REG_SCRIPTFUNC( "Goto", Goto )
	REG_SCRIPTFUNC( "Forward", Forward )
	REG_SCRIPTFUNC( "Backward", Backward )
	REG_SCRIPTFUNC( "RecordCommand", RecordCommand )
	REG_SCRIPTFUNC( "IsInAction", IsInAction )
	REG_SCRIPTFUNC( "IsRecording", IsRecording )
	REG_SCRIPTFUNC( "IsPlayingback", IsPlayingback )
	REG_SCRIPTFUNC( "GetCurrentFrame", GetCurrentFrame )
REG_END();

//-----------------------------------------------------------------------------
// Initialize library
//-----------------------------------------------------------------------------

LUALIB_API int luaopen_inputmanager( lua_State *pLuaState )
{
	VLua::RegisterTable( "InputManager" );
	VLua::RegisterTableFunctions( "InputManager", Registrations );

	return 1;
}
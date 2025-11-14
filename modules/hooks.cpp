// SvenInt (c) Sw1ft
// hooks.cpp

#include "stdafx.h"
#include "hooks.h"
#include "menu.h"
#include "server.h"
#include "scripts.h"

#include "modules/opengl.h"
#include "features/base_feature.h"
#include "features/player_silent_angles.h"
#include "features/st_input_manager.h"
#include "game/hook_events.h"
#include "game/messagebuffer.h"

#pragma warning( disable: 4390 )

namespace Modules { static CHooksModule hooksModule; CHooksModule *hooks = &hooksModule; }

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define CREATE_HOOK_EVENT( type ) \
	bool _Supercede = false; \
	constexpr int _HookEventType = type; \
	EHookResult _HookResult = kHookContinue; \
	CHookEvent hookEvent( type, _HookResult )

#define HOOK_EVENT_PUSH_ARG( argName ) hookEvent.PushArg( #argName, &argName )
#define HOOK_EVENT_SET_RETURN( argName ) hookEvent.SetReturn( &argName )
#define HOOK_EVENT_NO_RETURN() (void)0

// Never end macro with ' ; ' lol, I was trying to figure out why supercede wasn't working but that thingy...
#define HOOK_EVENT_CALL_CHAIN_EX( stopAction, returnVal ) \
	const HooksCallChain &hooks = Globals::hookevents->GetCallChain( _HookEventType, kHookCall ); \
	for ( IHookEventListener *listener : hooks ) { \
		const EHookResult r = listener->OnEvent( &hookEvent, false ); \
		if ( r > _HookResult ) \
			_HookResult = r; \
		if ( _HookResult == kHookSupercede ) \
			_Supercede = true; \
		else if ( _HookResult == kHookSupercedeStop ) { \
			do { \
				stopAction; \
			} while ( 0 ); \
			return returnVal; \
		} \
	} \
	if ( !_Supercede )

#define HOOK_EVENT_CALL_CHAIN( returnVal ) HOOK_EVENT_CALL_CHAIN_EX( (void)0, returnVal )
#define HOOK_EVENT_CALL_CHAIN_STOP_ACTION( stopAction, returnVal ) HOOK_EVENT_CALL_CHAIN_EX( stopAction, returnVal )

#define HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE() \
	const HooksCallChain &hooks = Globals::hookevents->GetCallChain( _HookEventType, kHookCall ); \
	for ( IHookEventListener *listener : hooks ) { \
		const EHookResult r = listener->OnEvent( &hookEvent, false ); \
		if ( r > _HookResult ) \
			_HookResult = r; \
	}

#define HOOK_EVENT_POST_CALL_CHAIN() \
	const HooksCallChain &posthooks = Globals::hookevents->GetCallChain( _HookEventType, kHookPostCall ); \
	for ( IHookEventListener *listener : posthooks ) { \
		const EHookResult r = listener->OnEvent( &hookEvent, true ); \
		if ( r > _HookResult ) \
			_HookResult = r; \
	}

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

// cldll_func hooks
DECLARE_HOOK( int, __cdecl, HUD_VidInit );
DECLARE_HOOK( int, __cdecl, HUD_Redraw, float time, int intermission );
DECLARE_HOOK( int, __cdecl, HUD_UpdateClientData, client_data_t *pcldata, float flTime );
DECLARE_HOOK( void, __cdecl, CL_CreateMove, float frametime, usercmd_t *cmd, int active );
DECLARE_HOOK( void, __cdecl, V_CalcRefdef, ref_params_t *pparams );
DECLARE_HOOK( int, __cdecl, HUD_AddEntity, int type, cl_entity_t *ent, const char *modelname );
DECLARE_HOOK( void, __cdecl, HUD_DrawTransparentTriangles );
DECLARE_HOOK( void, __cdecl, HUD_StudioEvent, const mstudioevent_t *studio_event, const cl_entity_t *entity );
DECLARE_HOOK( void, __cdecl, HUD_PostRunCmd, local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed );
DECLARE_HOOK( void, __cdecl, Demo_ReadBuffer, int size, unsigned const char *buffer );
DECLARE_HOOK( void, __cdecl, HUD_Frame, double time );
DECLARE_HOOK( int, __cdecl, HUD_Key_Event, int down, int keynum, const char *pszCurrentBinding );
DECLARE_HOOK( void, __cdecl, HUD_OnClientDisconnect );

DECLARE_HOOK( void, __cdecl, IN_Move, float frametime, usercmd_t *cmd );
DECLARE_HOOK( void, __cdecl, Key_Event, int key, int down );

DECLARE_HOOK( BOOL, WINAPI, SetCursorPos, int x, int y );
DECLARE_HOOK( BOOL, APIENTRY, wglSwapBuffers, HDC );
DECLARE_CLASS_HOOK( void, Panel__PaintTraverse, vgui::IPanel *thisptr, vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce );

DECLARE_HOOK( qboolean, __cdecl, Host_FilterTime, float time );
DECLARE_HOOK( void, __cdecl, Cvar_DirectSet, cvar_t *pCvar, const char *pszValue );

DECLARE_HOOK( void, __cdecl, SCR_UpdateScreen );
DECLARE_HOOK( void, __cdecl, SCR_BeginLoadingPlaque, int );
DECLARE_HOOK( void, __cdecl, SCR_EndLoadingPlaque );

DECLARE_HOOK( BOOL, WINAPI, fQueryPerformanceCounter, LARGE_INTEGER * );
DECLARE_HOOK( void, __cdecl, Netchan_Transmit, netchan_t *, int, unsigned char * );

DECLARE_HOOK( void, __cdecl, MSG_WriteUsercmd, sizebuf_t *, usercmd_t *, usercmd_t * );

DECLARE_CLASS_HOOK( void, StudioSetupBones, CStudioModelRenderer *thisptr );
DECLARE_CLASS_HOOK( void, StudioRenderModel, CStudioModelRenderer *thisptr );

// Server-side hooks
DECLARE_HOOK( void, __cdecl, Use, edict_t *, edict_t * );
DECLARE_HOOK( void, __cdecl, Touch, edict_t *, edict_t * );
DECLARE_HOOK( void, __cdecl, PlayerSpawns, edict_t *, edict_t * );
DECLARE_HOOK( bool, __cdecl, FixPlayerStuck, edict_t * );
DECLARE_HOOK( void, __cdecl, ClientKill, edict_t * );
DECLARE_HOOK( void, __cdecl, ClientPutInServer, edict_t * );
DECLARE_HOOK( void, __cdecl, ClientCommand, edict_t * );

DECLARE_CLASS_HOOK( void, CBasePlayer__SpecialSpawn, void * );
DECLARE_CLASS_HOOK( void, CBasePlayer__BeginRevive, void *, float );
DECLARE_CLASS_HOOK( void, CBasePlayer__EndRevive, void *, float );

DECLARE_CLASS_HOOK( void, CBaseEntity__FireBullets, void *thisptr, unsigned int, Vector, Vector, Vector, float, int, int, int, entvars_t *, int );
DECLARE_HOOK( void, __cdecl, UTIL_GetCircularGaussianSpread, float *, float * );

DECLARE_CLASS_HOOK( entvars_t *, CopyPEntityVars, entvars_t *pev_dst, entvars_t *pev_src );
DECLARE_HOOK( void, __cdecl, FireTargets, const char *, void *, void *, int, float, float );

//-----------------------------------------------------------------------------
// First client data receive related
//-----------------------------------------------------------------------------

static bool s_bLoading = false;
void CheckClientData( client_data_t *pcldata, float flTime );

//-----------------------------------------------------------------------------
// cldll_func hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, __cdecl, HOOKED_HUD_VidInit )
{
	Modules::menu->OnVidInit();

	Globals::clientweapon->SetCurrentWeaponID( -1 );
	Globals::clientweapon->SetCurrentWeaponCustom( false );

	int result = 0;

	CREATE_HOOK_EVENT( kHUD_VidInit_HookEvent );
	HOOK_EVENT_SET_RETURN( result );
	HOOK_EVENT_CALL_CHAIN( result )

	result = ORIG_HUD_VidInit();

	HOOK_EVENT_POST_CALL_CHAIN();

	return result;
}

DECLARE_FUNC( int, __cdecl, HOOKED_HUD_Redraw, float time, int intermission )
{
	int result = 0;

	CREATE_HOOK_EVENT( kHUD_Redraw_HookEvent );
	HOOK_EVENT_PUSH_ARG( time );
	HOOK_EVENT_PUSH_ARG( intermission );
	HOOK_EVENT_SET_RETURN( result );
	HOOK_EVENT_CALL_CHAIN_STOP_ACTION( Globals::localplayer->DrawDebugInfo(), result )
	
	result = ORIG_HUD_Redraw( time, intermission );

	HOOK_EVENT_POST_CALL_CHAIN();

	Globals::localplayer->DrawDebugInfo();

	return result;
}

DECLARE_FUNC( int, __cdecl, HOOKED_HUD_UpdateClientData, client_data_t *pcldata, float flTime )
{
	int changed = 0;

	CREATE_HOOK_EVENT( kHUD_UpdateClientData_HookEvent );
	HOOK_EVENT_PUSH_ARG( pcldata );
	HOOK_EVENT_PUSH_ARG( flTime );
	HOOK_EVENT_SET_RETURN( changed );
	HOOK_EVENT_CALL_CHAIN_STOP_ACTION( CheckClientData( pcldata, flTime ), changed )
	
	changed = ORIG_HUD_UpdateClientData( pcldata, flTime );

	CheckClientData( pcldata, flTime );

	HOOK_EVENT_POST_CALL_CHAIN();

	return changed;
}

DECLARE_FUNC( void, __cdecl, HOOKED_CL_CreateMove, float frametime, usercmd_t *cmd, int active )
{
	if ( Modules::menu->IsOpen() )
		cmd->viewangles = Modules::menu->GetFrozenCameraAngles();

	Features::silentangles->Cancel();

	CREATE_HOOK_EVENT( kCL_CreateMove_HookEvent );
	HOOK_EVENT_PUSH_ARG( frametime );
	HOOK_EVENT_PUSH_ARG( cmd );
	HOOK_EVENT_PUSH_ARG( active );
	HOOK_EVENT_CALL_CHAIN_STOP_ACTION(
		if ( Globals::clientweapon->IsForceReload() )
		{
			Globals::clientweapon->SetForceReload( false );
			cmd->buttons |= IN_RELOAD;
		}, HOOK_EVENT_NO_RETURN() )

	ORIG_CL_CreateMove( frametime, cmd, active );

	HOOK_EVENT_POST_CALL_CHAIN();

	// Here here and only here
	Features::silentangles->Process( cmd );
	Features::inputmanager->CreateMove( frametime, cmd, active );

	if ( Globals::clientweapon->IsForceReload() )
	{
		Globals::clientweapon->SetForceReload( false );
		cmd->buttons |= IN_RELOAD;
	}
}

DECLARE_FUNC( void, __cdecl, HOOKED_V_CalcRefdef, ref_params_t *pparams )
{
	CREATE_HOOK_EVENT( kV_CalcRefdef_HookEvent );
	HOOK_EVENT_PUSH_ARG( pparams );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_V_CalcRefdef( pparams );

	Features::silentangles->FixPlayerModelLean();

	memcpy( Globals::refparams, pparams, sizeof( ref_params_t ) );
	if ( pparams->movevars != NULL )
	{
		memcpy( Globals::refparams_movevars, pparams->movevars, sizeof( movevars_t ) );
	}
	else
	{
		Globals::refparams_movevars->gravity = 800.f;
		Globals::refparams_movevars->entgravity = 1.f;
	}

	HOOK_EVENT_POST_CALL_CHAIN();
}

DECLARE_FUNC( int, __cdecl, HOOKED_HUD_AddEntity, int type, cl_entity_t *ent, const char *modelname )
{
	int visible = 0;

	CREATE_HOOK_EVENT( kHUD_AddEntity_HookEvent );
	HOOK_EVENT_PUSH_ARG( type );
	HOOK_EVENT_PUSH_ARG( ent );
	HOOK_EVENT_PUSH_ARG( modelname );
	HOOK_EVENT_SET_RETURN( visible );
	HOOK_EVENT_CALL_CHAIN( visible )

	visible = ORIG_HUD_AddEntity( type, ent, modelname );

	HOOK_EVENT_POST_CALL_CHAIN();

	return visible;
}

DECLARE_FUNC( void, __cdecl, HOOKED_HUD_DrawTransparentTriangles )
{
	CREATE_HOOK_EVENT( kHUD_DrawTransparentTriangles_HookEvent );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_HUD_DrawTransparentTriangles();

	HOOK_EVENT_POST_CALL_CHAIN();
}

DECLARE_FUNC( void, __cdecl, HOOKED_HUD_StudioEvent, const mstudioevent_t *studio_event, const cl_entity_t *entity )
{
	CREATE_HOOK_EVENT( kHUD_StudioEvent_HookEvent );
	HOOK_EVENT_PUSH_ARG( studio_event );
	HOOK_EVENT_PUSH_ARG( entity );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_HUD_StudioEvent( studio_event, entity );

	HOOK_EVENT_POST_CALL_CHAIN();
}

DECLARE_FUNC( void, __cdecl, HOOKED_HUD_PostRunCmd, local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed )
{
	CREATE_HOOK_EVENT( kHUD_PostRunCmd_HookEvent );
	HOOK_EVENT_PUSH_ARG( from );
	HOOK_EVENT_PUSH_ARG( to );
	HOOK_EVENT_PUSH_ARG( cmd );
	HOOK_EVENT_PUSH_ARG( runfuncs );
	HOOK_EVENT_PUSH_ARG( time );
	HOOK_EVENT_PUSH_ARG( random_seed );
	HOOK_EVENT_CALL_CHAIN_STOP_ACTION( Globals::localplayer->Update( from, to, cmd, time, random_seed ), HOOK_EVENT_NO_RETURN() )
	
	ORIG_HUD_PostRunCmd( from, to, cmd, runfuncs, time, random_seed );

	Globals::localplayer->Update( from, to, cmd, time, random_seed );
	Modules::scripts->Callbacks()->OnGameFrame( Globals::cls->state, *Globals::host_frametime, true );

	HOOK_EVENT_POST_CALL_CHAIN();

	Features::inputmanager->GameFrame( true );
}

DECLARE_FUNC( void, __cdecl, HOOKED_Demo_ReadBuffer, int size, unsigned const char *buffer )
{
	CREATE_HOOK_EVENT( kDemo_ReadBuffer_HookEvent );
	HOOK_EVENT_PUSH_ARG( size );
	HOOK_EVENT_PUSH_ARG( buffer );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_Demo_ReadBuffer( size, buffer );

	HOOK_EVENT_POST_CALL_CHAIN();
}

DECLARE_FUNC( void, __cdecl, HOOKED_HUD_Frame, double time )
{
	CREATE_HOOK_EVENT( kHUD_Frame_HookEvent );
	HOOK_EVENT_PUSH_ARG( time );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_HUD_Frame( time );

	HOOK_EVENT_POST_CALL_CHAIN();
}

DECLARE_FUNC( int, __cdecl, HOOKED_HUD_Key_Event, int down, int keynum, const char *pszCurrentBinding )
{
	if ( Modules::menu->IsOpen() && down )
		return 0;

	int process_key = 0;

	CREATE_HOOK_EVENT( kHUD_Key_Event_HookEvent );
	HOOK_EVENT_PUSH_ARG( down );
	HOOK_EVENT_PUSH_ARG( keynum );
	HOOK_EVENT_PUSH_ARG( pszCurrentBinding );
	HOOK_EVENT_SET_RETURN( process_key );
	HOOK_EVENT_CALL_CHAIN( process_key )

	process_key = ORIG_HUD_Key_Event( down, keynum, pszCurrentBinding );

	HOOK_EVENT_POST_CALL_CHAIN();

	return process_key;
}

DECLARE_FUNC( void, __cdecl, HOOKED_HUD_OnClientDisconnect )
{
	s_bLoading = false;

	Modules::scripts->Callbacks()->OnDisconnect();
	Modules::scripts->ShutdownVM();

	CREATE_HOOK_EVENT( kHUD_OnClientDisconnect_HookEvent );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();

	ORIG_HUD_OnClientDisconnect();
}

//-----------------------------------------------------------------------------
// OnFirstClientdataReceived
//-----------------------------------------------------------------------------

static void CheckClientData( client_data_t *pcldata, float flTime )
{
	if ( !s_bLoading )
		return;

	Modules::scripts->Callbacks()->OnFirstClientdataReceived( flTime );

	CREATE_HOOK_EVENT( kOnFirstClientdataReceived_HookEvent );
	HOOK_EVENT_PUSH_ARG( pcldata );
	HOOK_EVENT_PUSH_ARG( flTime );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();

	s_bLoading = false;
}

//-----------------------------------------------------------------------------
// CurWeapon usermsg
//-----------------------------------------------------------------------------

static UserMsgHookFn ORIG_UserMsgHook_CurWeapon = NULL;
static int UserMsgHook_CurWeapon( const char *pszName, int iSize, void *pBuffer )
{
	CMessageBuffer buffer( pszName, pBuffer, iSize, true );
	buffer.BeginReading();

	int iId;
	int iState = buffer.ReadByte();
	// MAX_WEAPONS = 256 since SC 5.22
	if ( Globals::gameversion >= 522 )
		iId = buffer.ReadShort();
	else
		iId = buffer.ReadChar();

	if ( iState )
		Globals::clientweapon->SetCurrentWeaponID( iId );
	else if ( iId <= WEAPON_NONE )
		Globals::clientweapon->SetCurrentWeaponID( WEAPON_NONE );

	extern ConVar sc_debug_show_weapondata;
	if ( sc_debug_show_weapondata.GetBool() )
	{
		Globals::cl_enginefuncs->Con_Printf( const_cast<char *>( "CurWeapon : iState = %d\n" ), iState );
		Globals::cl_enginefuncs->Con_Printf( const_cast<char *>( "CurWeapon : iId = %d\n" ), iId );
	}

	return ORIG_UserMsgHook_CurWeapon( pszName, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// IN_Move hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_IN_Move, float frametime, usercmd_t *cmd )
{
	if ( Modules::menu->IsOpen() )
		return;

	CREATE_HOOK_EVENT( kIN_Move_HookEvent );
	HOOK_EVENT_PUSH_ARG( frametime );
	HOOK_EVENT_PUSH_ARG( cmd );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_IN_Move( frametime, cmd );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// Key_Event hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_Key_Event, int key, int down )
{
	if ( Modules::menu->IsOpen() && down )
		return;

	CREATE_HOOK_EVENT( kKey_Event_HookEvent );
	HOOK_EVENT_PUSH_ARG( key );
	HOOK_EVENT_PUSH_ARG( down );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_Key_Event( key, down );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// SetCursorPos hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( BOOL, WINAPI, HOOKED_SetCursorPos, int X, int Y )
{
	if ( Modules::menu->IsOpen() )
		return FALSE;

	return ORIG_SetCursorPos( X, Y );
}

//-----------------------------------------------------------------------------
// wglSwapBuffers hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( BOOL, APIENTRY, HOOKED_wglSwapBuffers, HDC hdc )
{
	CREATE_HOOK_EVENT( kwglSwapBuffers_HookEvent );
	HOOK_EVENT_PUSH_ARG( hdc );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();

	Modules::menu->wglSwapBuffers( hdc );

	return ORIG_wglSwapBuffers( hdc );
}

//-----------------------------------------------------------------------------
// vgui2::Panel::PaintTraverse hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_Panel__PaintTraverse, vgui::IPanel *thisptr, vgui::VPANEL vguiPanel, bool forceRepaint, bool allowForce )
{
	static vgui::VPANEL hClientPanel = 0;
	if ( hClientPanel == 0 )
	{
		vgui::IPanel *pClientPanel = Globals::enginevgui->GetPanel( vgui::PANEL_CLIENTDLL );
		if ( (vgui::VPANEL)pClientPanel == vguiPanel )
			hClientPanel = vguiPanel;
	}

	ORIG_Panel__PaintTraverse( thisptr, vguiPanel, forceRepaint, allowForce );

	if ( hClientPanel == vguiPanel && Globals::cl_enginefuncs->GetLocalPlayer() != NULL )
	{
		CREATE_HOOK_EVENT( kVGuiClientPanelPaint_HookEvent );
		HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();
	}
}

//-----------------------------------------------------------------------------
// Game frame simulation start
//-----------------------------------------------------------------------------

DECLARE_FUNC( qboolean, __cdecl, HOOKED_Host_FilterTime, float time )
{
	static bool bInitOnce = true;
	if ( bInitOnce )
	{
		if ( !Globals::commandline->HasParm("-sint_noshaders") )
		{
			Modules::opengl->Init();
			LoadShaderFeatures();
			PostLoadShaderFeatures();
		}
		else
		{
			DisableShaderFeatures();
			DevMsg( "[SvenInt] Shaders are disabled\n" );
		}

		bInitOnce = false;

		// Load default config or create a new one
		if ( !Modules::config->Load( "default.ini" ) )
			Modules::config->Save( "default.ini" );
	}

	qboolean simulate = FALSE;

	CREATE_HOOK_EVENT( kHost_FilterTime_HookEvent );
	HOOK_EVENT_PUSH_ARG( time );
	HOOK_EVENT_SET_RETURN( simulate );
	HOOK_EVENT_CALL_CHAIN_STOP_ACTION( if ( simulate ) {
		Modules::scripts->Callbacks()->OnGameFrame( Globals::cls->state, *Globals::host_frametime, false );
	}, simulate )

	simulate = ORIG_Host_FilterTime( time );

	if ( simulate )
	{
		Globals::demoplayback = Globals::cl_enginefuncs->pDemoAPI->IsPlayingback();
		Modules::scripts->Callbacks()->OnGameFrame( Globals::cls->state, *Globals::host_frametime, false );
	}

	HOOK_EVENT_POST_CALL_CHAIN();

	if ( simulate )
		Features::inputmanager->GameFrame( false );

	return simulate;
}

//-----------------------------------------------------------------------------
// Cvar change hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_Cvar_DirectSet, cvar_t *pCvar, const char *pszValue )
{
	static bool processing_hooks = false;

	const char *pszOldValue = strdup( pCvar->string );
	float flOldValue = pCvar->value;

	ORIG_Cvar_DirectSet( pCvar, pszValue );

	if ( !processing_hooks && stricmp( pCvar->string, pszOldValue ) )
	{
		processing_hooks = true;

		Modules::menu->OnCvarChange( pCvar, pszOldValue, flOldValue );
		Globals::gamehooks->CallCvarChangeChain( pCvar, pszOldValue, flOldValue );

		processing_hooks = false;
	}

	free( (void *)pszOldValue );
}

//-----------------------------------------------------------------------------
// SCR_UpdateScreen
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_SCR_UpdateScreen )
{
	CREATE_HOOK_EVENT( kSCR_UpdateScreen_HookEvent );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_SCR_UpdateScreen();

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// SCR_BeginLoadingPlaque
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_SCR_BeginLoadingPlaque, int unk )
{
	s_bLoading = false;

	Modules::scripts->Callbacks()->OnBeginLoading();

	CREATE_HOOK_EVENT( kSCR_BeginLoadingPlaque_HookEvent );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();

	ORIG_SCR_BeginLoadingPlaque( unk );
}

//-----------------------------------------------------------------------------
// SCR_EndLoadingPlaque
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_SCR_EndLoadingPlaque )
{
	s_bLoading = true;

	Modules::scripts->Callbacks()->OnEndLoading();

	CREATE_HOOK_EVENT( kSCR_EndLoadingPlaque_HookEvent );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE();

	ORIG_SCR_EndLoadingPlaque();
}

//-----------------------------------------------------------------------------
// Netchan_Transmit hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_Netchan_Transmit, netchan_t *chan, int lengthInBytes, unsigned char *data )
{
	CREATE_HOOK_EVENT( kNetchan_Transmit_HookEvent );
	HOOK_EVENT_PUSH_ARG( chan );
	HOOK_EVENT_PUSH_ARG( lengthInBytes );
	HOOK_EVENT_PUSH_ARG( data );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_Netchan_Transmit( chan, lengthInBytes, data );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// MSG_WriteUsercmd hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_MSG_WriteUsercmd, sizebuf_t *buf, usercmd_t *to, usercmd_t *from )
{
	CREATE_HOOK_EVENT( kMSG_WriteUsercmd_HookEvent );
	HOOK_EVENT_PUSH_ARG( buf );
	HOOK_EVENT_PUSH_ARG( to );
	HOOK_EVENT_PUSH_ARG( from );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_MSG_WriteUsercmd( buf, to, from );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// CStudioModelRenderer::StudioSetupBones hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_StudioSetupBones, CStudioModelRenderer *thisptr )
{
	// not pushing thisptr since we have the access to Globals::studiorenderer
	CREATE_HOOK_EVENT( kStudioSetupBones_HookEvent );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_StudioSetupBones( thisptr );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// CStudioModelRenderer::StudioRenderModel hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_StudioRenderModel, CStudioModelRenderer *thisptr )
{
	// not pushing thisptr since we have the access to Globals::studiorenderer
	CREATE_HOOK_EVENT( kStudioRenderModel_HookEvent );
	HOOK_EVENT_CALL_CHAIN( HOOK_EVENT_NO_RETURN() )

	ORIG_StudioRenderModel( thisptr );

	HOOK_EVENT_POST_CALL_CHAIN();
}

//-----------------------------------------------------------------------------
// Network message 'ServerInfo'
//-----------------------------------------------------------------------------

static NetMsgHookFn ORIG_NetMsgHook_ServerInfo = NULL;
static void HOOKED_NetMsgHook_ServerInfo( void )
{
	CNetMessageParams *params = Globals::gameutils->GetNetMessageParams();

	CMessageBuffer msg;
	msg.Init( params->buffer, params->readcount, params->badread );

	msg.ReadLong(); // Protocol

	Globals::servernumber = msg.ReadLong();
	Globals::mapcrc = (uint32_t)msg.ReadLong();

	CREATE_HOOK_EVENT( kServerInfo_HookEvent );
	HOOK_EVENT_CALL_CHAIN_NO_SUPERCEDE()

	ORIG_NetMsgHook_ServerInfo();

	HOOK_EVENT_POST_CALL_CHAIN();

	Modules::server->OnServerInfo();
	Modules::scripts->InitVM();
}

//-----------------------------------------------------------------------------
// Network message 'TempEntity'
//-----------------------------------------------------------------------------

extern ConVar sc_disable_monster_info;
extern ConVar sc_disable_sprays;

static NetMsgHookFn ORIG_NetMsgHook_TempEntity = NULL;
static void HOOKED_NetMsgHook_TempEntity( void )
{
	static CMessageBuffer msg;
	CNetMessageParams *params = Globals::gameutils->GetNetMessageParams();
	msg.Init( params->buffer, params->readcount, params->badread );

	int entitytype = msg.ReadByte();
	if ( entitytype == TE_TEXTMESSAGE )
	{
		float fxTime = -1.f;
		char szMessage[ 512 ];

		byte color1[ 4 ];
		byte color2[ 4 ];

		int channel = msg.ReadByte();

		float x = msg.ReadShort() * ( 1.f / ( 1 << 13 ) );
		float y = msg.ReadShort() * ( 1.f / ( 1 << 13 ) );

		int effect = msg.ReadByte();

		color1[ 0 ] = msg.ReadByte();
		color1[ 1 ] = msg.ReadByte();
		color1[ 2 ] = msg.ReadByte();
		color1[ 3 ] = msg.ReadByte();

		color2[ 0 ] = msg.ReadByte();
		color2[ 1 ] = msg.ReadByte();
		color2[ 2 ] = msg.ReadByte();
		color2[ 3 ] = msg.ReadByte();

		float fadeinTime = msg.ReadShort() * ( 1.f / ( 1 << 8 ) );
		float fadeoutTime = msg.ReadShort() * ( 1.f / ( 1 << 8 ) );
		float holdTime = msg.ReadShort() * ( 1.f / ( 1 << 8 ) );

		if ( effect == 2 )
		{
			fxTime = msg.ReadShort() * ( 1.f / ( 1 << 8 ) );
		}

		const char *pszMessage = msg.ReadString();

		strncpy( szMessage, pszMessage, sizeof( szMessage ) / sizeof( *szMessage ) );
		szMessage[ ( sizeof( szMessage ) / sizeof( *szMessage ) ) - 1 ] = 0;

		if ( *reinterpret_cast<int *>( color1 ) == ( ( (byte)171 ) | ( (byte)23 << 8 ) | ( (byte)7 << 16 ) ) &&
			 *reinterpret_cast<int *>( color2 ) == ( ( (byte)207 ) | ( (byte)23 << 8 ) | ( (byte)7 << 16 ) | ( (byte)0xFF << 24 ) ) )
		{
			constexpr size_t playerStrLength = ( sizeof( "Player:  " ) / sizeof( char ) ) - 1;

			// Starts with
			if ( !strncmp( "Player:  ", szMessage, playerStrLength ) )
			{
				const char *pszPlayerName = szMessage + playerStrLength;
				char *plname_buffer = szMessage + playerStrLength;

				while ( *plname_buffer )
				{
					if ( *plname_buffer == '\n' )
					{
						*plname_buffer = 0;
						break;
					}

					plname_buffer++;
				}

				for ( int i = 1; i <= Globals::cl_enginefuncs->GetMaxClients(); i++ )
				{
					cl_entity_t *pPlayer = Globals::cl_enginefuncs->GetEntityByIndex( i );

					if ( pPlayer == NULL )
						continue;

					player_info_t *pPlayerInfo = Globals::enginestudio->PlayerInfo( i - 1 );

					if ( !strcmp( pPlayerInfo->name, pszPlayerName ) )
					{
						const float flHealth = Globals::extraplayerinfo->GetHealth( i );
						if ( flHealth != -128.f )
						{
							Globals::extraplayerinfo->SetHealth( i, -128.f );
						}

						break;
					}
				}
			}
		}
		else if ( *reinterpret_cast<int *>( color1 ) == ( ( (byte)7 ) | ( (byte)171 << 8 ) | ( (byte)95 << 16 ) ) &&
				  *reinterpret_cast<int *>( color2 ) == ( ( (byte)7 ) | ( (byte)207 << 8 ) | ( (byte)95 << 16 ) | ( (byte)0xFF << 24 ) ) )
		{
			constexpr size_t playerStrLength = ( sizeof( "Player:  " ) / sizeof( char ) ) - 1;

			// Starts with
			if ( !strncmp( "Player:  ", szMessage, playerStrLength ) )
			{
				const char *pszPlayerName = szMessage + playerStrLength;
				char *plname_buffer = szMessage + playerStrLength;

				while ( *plname_buffer )
				{
					if ( *plname_buffer == '\n' )
					{
						*plname_buffer = 0;
						break;
					}

					plname_buffer++;
				}

				for ( int i = 1; i <= Globals::cl_enginefuncs->GetMaxClients(); i++ )
				{
					cl_entity_t *pPlayer = Globals::cl_enginefuncs->GetEntityByIndex( i );

					if ( pPlayer == NULL )
						continue;

					player_info_t *pPlayerInfo = Globals::enginestudio->PlayerInfo( i - 1 );

					if ( !strcmp( pPlayerInfo->name, pszPlayerName ) )
					{
						const float flHealth = Globals::extraplayerinfo->GetHealth( i );
						if ( flHealth == -128.f )
						{
							Globals::extraplayerinfo->SetHealth( i, 100.f );
						}

						break;
					}
				}
			}
		}

		if ( sc_disable_monster_info.GetBool() )
		{
			Globals::gameutils->ApplyReadToNetMessageBuffer( &msg );
			return;
		}
	}
	else if ( entitytype == TE_PLAYERDECAL )
	{
		msg.ReadByte(); // playernum
		msg.ReadCoord(); // pTrace->vecEndPos.x
		msg.ReadCoord(); // pTrace->vecEndPos.y
		msg.ReadCoord(); // pTrace->vecEndPos.z
		msg.ReadShort(); // (short)ENTINDEX( pTrace->pHit )
		msg.ReadByte(); // index

		if ( sc_disable_sprays.GetBool() )
		{
			Globals::gameutils->ApplyReadToNetMessageBuffer( &msg );
			return;
		}
	}

	ORIG_NetMsgHook_TempEntity();
}

//-----------------------------------------------------------------------------
// Server hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_Use, edict_t *pUseEntity, edict_t *pOther )
{
	ORIG_Use( pUseEntity, pOther );

	Modules::scripts->Callbacks()->OnEntityUse( pUseEntity, pOther );
}

DECLARE_FUNC( void, __cdecl, HOOKED_Touch, edict_t *pTouchEntity, edict_t *pOther )
{
	ORIG_Touch( pTouchEntity, pOther );

	Modules::scripts->Callbacks()->OnEntityTouch( pTouchEntity, pOther );
}

DECLARE_FUNC( void, __cdecl, HOOKED_PlayerSpawns, edict_t *pSpawnSpot, edict_t *pPlayer )
{
	ORIG_PlayerSpawns( pSpawnSpot, pPlayer );

	Modules::scripts->Callbacks()->OnPlayerSpawn( pSpawnSpot, pPlayer );
}

static bool bRevivePreUnstuck = false;
static Vector vecRevivePreUnstuckOrigin;

DECLARE_FUNC( bool, __cdecl, HOOKED_FixPlayerStuck, edict_t *pPlayer )
{
	bool bUnstuck = ORIG_FixPlayerStuck( pPlayer );

	if ( bUnstuck )
	{
		Vector unstuckBoundsMin = vecRevivePreUnstuckOrigin + Vector( -48.f, -48.f, -48.f );
		Vector unstuckBoundsMax = vecRevivePreUnstuckOrigin + Vector( 48.f, 48.f, 48.f );

		Modules::scripts->Callbacks()->OnPlayerUnstuck( pPlayer );

		// Outside of the largest test hull !!
		if ( !UTIL_IsPointInsideAABB( pPlayer->v.origin, unstuckBoundsMin, unstuckBoundsMax ) )
		{
			Globals::cl_enginefuncs->pfnClientCmd( "say \"FixPlayerStuck: NOT LEGIT UNSTUCK DETECTED.\"" );
			Globals::cl_enginefuncs->pfnClientCmd( "say \"FixPlayerStuck: the unstuck position is outside the largest test hull.\"" );

			Warning( "FixPlayerStuck: pre-unstuck origin %.6f %.6f %.6f\n", VectorExpand( vecRevivePreUnstuckOrigin ) );
		}
	}

	return bUnstuck;
}

DECLARE_FUNC( void, __cdecl, HOOKED_ClientKill, edict_t *pPlayer )
{
	ORIG_ClientKill( pPlayer );

	Modules::scripts->Callbacks()->OnClientKill( pPlayer );
}

DECLARE_FUNC( void, __cdecl, HOOKED_ClientPutInServer, edict_t *pPlayer )
{
	ORIG_ClientPutInServer( pPlayer );

	Modules::server->OnClientPutInServer( pPlayer );
	Modules::scripts->Callbacks()->OnClientPutInServer( pPlayer );
}

DECLARE_FUNC( void, __cdecl, HOOKED_ClientCommand, edict_t *pPlayer )
{
	if ( !pPlayer->pvPrivateData )
		return;

	if ( Modules::scripts->ClientCommand( pPlayer ) )
		return;

	ORIG_ClientCommand( pPlayer );
}

DECLARE_CLASS_FUNC( void, HOOKED_CBasePlayer__SpecialSpawn, void *thisptr )
{
	ORIG_CBasePlayer__SpecialSpawn( thisptr );

	// Skip vtable and then get player's entvars
	entvars_t *entvars = *(entvars_t **)( (unsigned long *)thisptr + 1 );
	edict_t *pPlayer = Globals::sv_enginefuncs->pfnFindEntityByVars( entvars );

	if ( pPlayer != NULL )
	{
		Modules::scripts->Callbacks()->OnSpecialSpawn( pPlayer );

		if ( bRevivePreUnstuck )
		{
			Vector unstuckBoundsMin = vecRevivePreUnstuckOrigin + Vector( -48.f, -48.f, -48.f );
			Vector unstuckBoundsMax = vecRevivePreUnstuckOrigin + Vector( 48.f, 48.f, 48.f );

			// Outside of the largest test hull !!
			if ( !UTIL_IsPointInsideAABB( pPlayer->v.origin, unstuckBoundsMin, unstuckBoundsMax ) )
			{
				Globals::cl_enginefuncs->pfnClientCmd( "say \"CBasePlayer::SpecialSpawn -> FixPlayerStuck: NOT LEGIT REVIVE DETECTED.\"" );
				Globals::cl_enginefuncs->pfnClientCmd( "say \"CBasePlayer::SpecialSpawn -> FixPlayerStuck: the revive position is outside the largest test hull.\"" );

				Warning( "FixPlayerStuck: pre-revive origin %.6f %.6f %.6f\n", VectorExpand( vecRevivePreUnstuckOrigin ) );
			}
		}
	}

	bRevivePreUnstuck = false;
}

DECLARE_CLASS_FUNC( void, HOOKED_CBasePlayer__BeginRevive, void *thisptr, float flNextThink )
{
	ORIG_CBasePlayer__BeginRevive( thisptr, flNextThink );

	entvars_t *entvars = *(entvars_t **)( (unsigned long *)thisptr + 1 );
	edict_t *pPlayer = Globals::sv_enginefuncs->pfnFindEntityByVars( entvars );

	if ( pPlayer != NULL )
		Modules::scripts->Callbacks()->OnBeginPlayerRevive( pPlayer );
}

DECLARE_CLASS_FUNC( void, HOOKED_CBasePlayer__EndRevive, void *thisptr, float flNextThink )
{
	ORIG_CBasePlayer__EndRevive( thisptr, flNextThink );

	entvars_t *entvars = *(entvars_t **)( (unsigned long *)thisptr + 1 );
	edict_t *pPlayer = Globals::sv_enginefuncs->pfnFindEntityByVars( entvars );

	if ( pPlayer != NULL )
		Modules::scripts->Callbacks()->OnEndPlayerRevive( pPlayer );
}

extern ConVar sc_sv_disable_spread;
static bool inside_CBaseEntity__FireBullets = false;
static void *inside_CBaseEntity__FireBullets_thisptr = NULL;

DECLARE_CLASS_FUNC( void, HOOKED_CBaseEntity__FireBullets, void *thisptr, unsigned int cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, int iBulletType, int iTracerFeq, int iDamage, entvars_t *pAttacker, int fDraw )
{
	inside_CBaseEntity__FireBullets = true;
	inside_CBaseEntity__FireBullets_thisptr = thisptr;

	if ( sc_sv_disable_spread.GetBool() )
	{
		vecSpread.Zero();
	}

	ORIG_CBaseEntity__FireBullets( thisptr, cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFeq, iDamage, pAttacker, fDraw );

	inside_CBaseEntity__FireBullets = false;
}

DECLARE_FUNC( void, __cdecl, HOOKED_UTIL_GetCircularGaussianSpread, float *x, float *y )
{
	ORIG_UTIL_GetCircularGaussianSpread( x, y );

	if ( inside_CBaseEntity__FireBullets )
	{
		if ( sc_sv_disable_spread.GetBool() )
		{
			*x = *y = 0.f;
		}

		entvars_t *pev = *(entvars_t **)( (unsigned long *)inside_CBaseEntity__FireBullets_thisptr + 1 );
		edict_t *pEntity = Globals::sv_enginefuncs->pfnFindEntityByVars( pev );

		Modules::scripts->Callbacks()->OnFireBulletsSpread( pEntity, x, y );
	}
}

DECLARE_CLASS_FUNC( entvars_t *, HOOKED_CopyPEntityVars, entvars_t *pev_dst, entvars_t *pev_src )
{
	bRevivePreUnstuck = true;
	vecRevivePreUnstuckOrigin = pev_src->origin;

	return ORIG_CopyPEntityVars( pev_dst, pev_src );
}

DECLARE_FUNC( void, __cdecl, HOOKED_FireTargets, const char *pszTargetName, void *pActivator, void *pCaller, int useType, float flValue, float flDelay )
{
	ORIG_FireTargets( pszTargetName, pActivator, pCaller, useType, flValue, flDelay );

	Modules::scripts->Callbacks()->OnFireTargets( pszTargetName, pActivator, pCaller, useType, flValue, flDelay );
}

static CommandCallbackFn ORIG_restart = NULL;
static DECLARE_FUNC( void, __cdecl, HOOKED_restart )
{
	if ( !Modules::server->Host_IsServerActive() )
		return;

	Modules::scripts->Callbacks()->OnRestart();

	ORIG_restart();
}

//-----------------------------------------------------------------------------
// Attach detours
//-----------------------------------------------------------------------------

void CHooksModule::AttachDetours( void )
{
	using namespace GameData;
	DetourHandle_t hDetour;

	AttachDetour( "HUD_VidInit", Globals::cl_funcs->HUD_VidInit, HOOKED_HUD_VidInit, GET_FUNC_PTR( ORIG_HUD_VidInit ) );
	AttachDetour( "HUD_Redraw", Globals::cl_funcs->HUD_Redraw, HOOKED_HUD_Redraw, GET_FUNC_PTR( ORIG_HUD_Redraw ) );
	AttachDetour( "HUD_UpdateClientData", Globals::cl_funcs->HUD_UpdateClientData, HOOKED_HUD_UpdateClientData, GET_FUNC_PTR( ORIG_HUD_UpdateClientData ) );
	AttachDetour( "CL_CreateMove", Globals::cl_funcs->CL_CreateMove, HOOKED_CL_CreateMove, GET_FUNC_PTR( ORIG_CL_CreateMove ) );
	AttachDetour( "V_CalcRefdef", Globals::cl_funcs->V_CalcRefdef, HOOKED_V_CalcRefdef, GET_FUNC_PTR( ORIG_V_CalcRefdef ) );
	AttachDetour( "HUD_AddEntity", Globals::cl_funcs->HUD_AddEntity, HOOKED_HUD_AddEntity, GET_FUNC_PTR( ORIG_HUD_AddEntity ) );
	AttachDetour( "HUD_DrawTransparentTriangles", Globals::cl_funcs->HUD_DrawTransparentTriangles, HOOKED_HUD_DrawTransparentTriangles, GET_FUNC_PTR( ORIG_HUD_DrawTransparentTriangles ) );
	AttachDetour( "HUD_StudioEvent", Globals::cl_funcs->HUD_StudioEvent, HOOKED_HUD_StudioEvent, GET_FUNC_PTR( ORIG_HUD_StudioEvent ) );
	AttachDetour( "HUD_PostRunCmd", Globals::cl_funcs->HUD_PostRunCmd, HOOKED_HUD_PostRunCmd, GET_FUNC_PTR( ORIG_HUD_PostRunCmd ) );
	AttachDetour( "Demo_ReadBuffer", Globals::cl_funcs->Demo_ReadBuffer, HOOKED_Demo_ReadBuffer, GET_FUNC_PTR( ORIG_Demo_ReadBuffer ) );
	AttachDetour( "HUD_Frame", Globals::cl_funcs->HUD_Frame, HOOKED_HUD_Frame, GET_FUNC_PTR( ORIG_HUD_Frame ) );
	AttachDetour( "HUD_Key_Event", Globals::cl_funcs->HUD_Key_Event, HOOKED_HUD_Key_Event, GET_FUNC_PTR( ORIG_HUD_Key_Event ) );
	AttachDetour( "HUD_OnClientDisconnect", Globals::cl_funcs->HUD_OnClientDisconnect, HOOKED_HUD_OnClientDisconnect, GET_FUNC_PTR( ORIG_HUD_OnClientDisconnect ) );

	AttachDetour( "IN_Move", Pointers::Client::IN_Move, HOOKED_IN_Move, GET_FUNC_PTR( ORIG_IN_Move ) );
	AttachDetour( "Key_Event", Pointers::Engine::Key_Event, HOOKED_Key_Event, GET_FUNC_PTR( ORIG_Key_Event ) );

	AttachDetour( "Host_FilterTime", Pointers::Engine::Host_FilterTime, HOOKED_Host_FilterTime, GET_FUNC_PTR( ORIG_Host_FilterTime ) );
	AttachDetour( "Cvar_DirectSet", Pointers::Engine::Cvar_DirectSet, HOOKED_Cvar_DirectSet, GET_FUNC_PTR( ORIG_Cvar_DirectSet ) );
	
	AttachDetour( "SCR_UpdateScreen", Pointers::Engine::SCR_UpdateScreen, HOOKED_SCR_UpdateScreen, GET_FUNC_PTR( ORIG_SCR_UpdateScreen ) );
	AttachDetour( "SCR_BeginLoadingPlaque", Pointers::Engine::SCR_BeginLoadingPlaque, HOOKED_SCR_BeginLoadingPlaque, GET_FUNC_PTR( ORIG_SCR_BeginLoadingPlaque ) );
	AttachDetour( "SCR_EndLoadingPlaque", Pointers::Engine::SCR_EndLoadingPlaque, HOOKED_SCR_EndLoadingPlaque, GET_FUNC_PTR( ORIG_SCR_EndLoadingPlaque ) );

	AttachDetour( "Netchan_Transmit", Pointers::Engine::Netchan_Transmit, HOOKED_Netchan_Transmit, GET_FUNC_PTR( ORIG_Netchan_Transmit ) );
	AttachDetour( "MSG_WriteUsercmd", Pointers::Engine::MSG_WriteUsercmd, HOOKED_MSG_WriteUsercmd, GET_FUNC_PTR( ORIG_MSG_WriteUsercmd ) );

	HMODULE hUser32 = GetModuleHandle( "user32.dll" );
	if ( hUser32 != NULL ) {
		void *pfnSetCursorPos = GetProcAddress( hUser32, "SetCursorPos" );
		if ( *(uint8_t *)pfnSetCursorPos == 0xE9 )
			pfnSetCursorPos = MemoryUtils()->CalcAbsoluteAddress(pfnSetCursorPos);
		AttachDetour( "SetCursorPos", pfnSetCursorPos, HOOKED_SetCursorPos, GET_FUNC_PTR( ORIG_SetCursorPos ) );
	}
	
	void *pfnwglSwapBuffers = GetProcAddress( (HMODULE)GameData::Modules::OpenGL, "wglSwapBuffers" );
	if ( *(uint8_t *)pfnwglSwapBuffers == 0xE9 )
		pfnwglSwapBuffers = MemoryUtils()->CalcAbsoluteAddress( pfnwglSwapBuffers );
	AttachDetour( "wglSwapBuffers", pfnwglSwapBuffers, HOOKED_wglSwapBuffers, GET_FUNC_PTR( ORIG_wglSwapBuffers ) );

	AttachDetour( "VPanelWrapper::PaintTraverse",
				  Globals::panel,
				  GameData::Offsets::VGUI2::vtidx_IPanel__PaintTraverse,
				  HOOKED_Panel__PaintTraverse,
				  GET_FUNC_PTR( ORIG_Panel__PaintTraverse ) );

	AttachDetour( "CStudioModelRenderer::StudioSetupBones",
				  Globals::studiorenderer,
				  GameData::Offsets::Client::vtidx_CStudioModelRenderer__StudioSetupBones,
				  HOOKED_StudioSetupBones,
				  GET_FUNC_PTR( ORIG_StudioSetupBones ) );
	
	AttachDetour( "CStudioModelRenderer::StudioRenderModel",
				  Globals::studiorenderer,
				  GameData::Offsets::Client::vtidx_CStudioModelRenderer__StudioRenderModel,
				  HOOKED_StudioRenderModel,
				  GET_FUNC_PTR( ORIG_StudioRenderModel ) );

	hDetour = Globals::gamehooks->HookUserMessage( "CurWeapon", UserMsgHook_CurWeapon, &ORIG_UserMsgHook_CurWeapon );
	if ( hDetour != DETOUR_INVALID_HANDLE )
		m_hDetours.insert( m_hDetours.begin(), hDetour );

	hDetour = Globals::gamehooks->HookNetworkMessage( SVC_SERVERINFO, HOOKED_NetMsgHook_ServerInfo, &ORIG_NetMsgHook_ServerInfo );
	if ( hDetour != DETOUR_INVALID_HANDLE )
		m_hDetours.insert( m_hDetours.begin(), hDetour );
	
	hDetour = Globals::gamehooks->HookNetworkMessage( SVC_TEMPENTITY, HOOKED_NetMsgHook_TempEntity, &ORIG_NetMsgHook_TempEntity );
	if ( hDetour != DETOUR_INVALID_HANDLE )
		m_hDetours.insert( m_hDetours.begin(), hDetour );

	hDetour = Globals::gamehooks->HookConsoleCommand( "restart", HOOKED_restart, &ORIG_restart );
	if ( hDetour != DETOUR_INVALID_HANDLE )
		m_hDetours.insert( m_hDetours.begin(), hDetour );

	// Server hooks
	AttachDetour( "DLL_FUNCTIONS::Use", Globals::dllFuncs->pfnUse, HOOKED_Use, GET_FUNC_PTR( ORIG_Use ) );
	AttachDetour( "DLL_FUNCTIONS::Touch", Globals::dllFuncs->pfnTouch, HOOKED_Touch, GET_FUNC_PTR( ORIG_Touch ) );
	AttachDetour( "DLL_FUNCTIONS::ClientKill", Globals::dllFuncs->pfnClientKill, HOOKED_ClientKill, GET_FUNC_PTR( ORIG_ClientKill ) );
	AttachDetour( "DLL_FUNCTIONS::ClientPutInServer", Globals::dllFuncs->pfnClientPutInServer, HOOKED_ClientPutInServer, GET_FUNC_PTR( ORIG_ClientPutInServer ) );
	AttachDetour( "DLL_FUNCTIONS::ClientCommand", Globals::dllFuncs->pfnClientCommand, HOOKED_ClientCommand, GET_FUNC_PTR( ORIG_ClientCommand ) );
	AttachDetour( "PlayerSpawns", Pointers::Server::PlayerSpawns, HOOKED_PlayerSpawns, GET_FUNC_PTR( ORIG_PlayerSpawns ) );
	AttachDetour( "FixPlayerStuck", Pointers::Server::FixPlayerStuck, HOOKED_FixPlayerStuck, GET_FUNC_PTR( ORIG_FixPlayerStuck ) );
	AttachDetour( "CBaseEntity::FireBullets", Pointers::Server::CBaseEntity__FireBullets, HOOKED_CBaseEntity__FireBullets, GET_FUNC_PTR( ORIG_CBaseEntity__FireBullets ) );
	AttachDetour( "UTIL_GetCircularGaussianSpread", Pointers::Server::UTIL_GetCircularGaussianSpread, HOOKED_UTIL_GetCircularGaussianSpread, GET_FUNC_PTR( ORIG_UTIL_GetCircularGaussianSpread ) );
	AttachDetour( "FireTargets", Pointers::Server::FireTargets, HOOKED_FireTargets, GET_FUNC_PTR( ORIG_FireTargets ) );
	AttachDetour( "CopyPEntityVars", Pointers::Server::CopyPEntityVars, HOOKED_CopyPEntityVars, GET_FUNC_PTR( ORIG_CopyPEntityVars ) );

	void *dummyBasePlayer = Pointers::Server::vmt_CBasePlayer;
	AttachDetour( "CBasePlayer::SpecialSpawn",
				  &dummyBasePlayer,
				  GameData::Offsets::Server::vtidx_CBasePlayer_SpecialSpawn,
				  HOOKED_CBasePlayer__SpecialSpawn,
				  GET_FUNC_PTR( ORIG_CBasePlayer__SpecialSpawn ) );
	
	AttachDetour( "CBasePlayer::BeginRevive",
				  &dummyBasePlayer,
				  GameData::Offsets::Server::vtidx_CBasePlayer_BeginRevive,
				  HOOKED_CBasePlayer__BeginRevive,
				  GET_FUNC_PTR( ORIG_CBasePlayer__BeginRevive ) );
	
	AttachDetour( "CBasePlayer::EndRevive",
				  &dummyBasePlayer,
				  GameData::Offsets::Server::vtidx_CBasePlayer_EndRevive,
				  HOOKED_CBasePlayer__EndRevive,
				  GET_FUNC_PTR( ORIG_CBasePlayer__EndRevive ) );
}

//-----------------------------------------------------------------------------
// Deattach detours
//-----------------------------------------------------------------------------

void CHooksModule::DeattachDetours( void )
{
	for ( const DetourHandle_t &detour : m_hDetours )
	{
		Detours()->RemoveDetour( detour );
	}

	m_hDetours.clear();
}

//-----------------------------------------------------------------------------
// Attach a detour
//-----------------------------------------------------------------------------

bool CHooksModule::AttachDetour( const char *pszFunctionName, void *pFunction, void *pDetourFunction, void **ppOriginalFunction )
{
	if ( pFunction == NULL )
	{
		Warning2( "[SvenInt::Hooks] Detour target \"%s\" is NULL\n", pszFunctionName );
		return false;
	}

	DetourHandle_t hDetour = Detours()->DetourFunction( pFunction, pDetourFunction, ppOriginalFunction );
	if ( hDetour == DETOUR_INVALID_HANDLE )
	{
		Warning2( "[SvenInt::Hooks] Failed to attach detour to function \"%s\"\n", pszFunctionName );
		return false;
	}

	m_hDetours.insert( m_hDetours.begin(), hDetour );
	DevMsg( "<SvenInt::Hooks> Attached detour (0x%X) to function \"%s\" at address 0x%X\n", pDetourFunction, pszFunctionName, pFunction );

	return true;
}

//-----------------------------------------------------------------------------
// Hook virtual function
//-----------------------------------------------------------------------------

bool CHooksModule::AttachDetour( const char *pszFunctionName, void *pClassInstance, int vtidx, void *pDetourFunction, void **ppOriginalFunction )
{
	if ( pClassInstance == NULL )
	{
		Warning2( "[SvenInt::Hooks] Class instance of method \"%s\" is NULL\n", pszFunctionName );
		return false;
	}
	
	if ( vtidx == ~0 )
	{
		Warning2( "[SvenInt::Hooks] Undefined vtidx of method \"%s\"\n", pszFunctionName );
		return false;
	}

	void *pFunction = GetVTableFunction( pClassInstance, vtidx );

	DetourHandle_t hDetour = Detours()->DetourVirtualFunction( pClassInstance, vtidx, pDetourFunction, ppOriginalFunction );
	if ( hDetour == DETOUR_INVALID_HANDLE )
	{
		Warning2( "[SvenInt::Hooks] Failed to hook virtual method \"%s\"\n", pszFunctionName );
		return false;
	}

	m_hDetours.insert( m_hDetours.begin(), hDetour );
	DevMsg( "<SvenInt::Hooks> Hooked virtual method \"%s\" at address 0x%X (vtidx: %d)\n", pszFunctionName, pFunction, vtidx );

	return true;
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

CHooksModule::CHooksModule()
{
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CHooksModule::Init( void )
{
	Globals::gamehooks->Init();
	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CHooksModule::Shutdown( void )
{
	Globals::gamehooks->Shutdown();
}

/*

DECLARE_HOOK( int, __cdecl, HUD_VidInit, void );
DECLARE_HOOK( int, __cdecl, HUD_Redraw, float time, int intermission );
DECLARE_HOOK( int, __cdecl, HUD_UpdateClientData, client_data_t *pcldata, float flTime );
//DECLARE_HOOK( void, __cdecl, HUD_PlayerMove, playermove_t *ppmove, int server );
//DECLARE_HOOK( void, __cdecl, IN_ActivateMouse, void );
//DECLARE_HOOK( void, __cdecl, IN_DeactivateMouse, void );
//DECLARE_HOOK( void, __cdecl, IN_MouseEvent, int mstate );
//DECLARE_HOOK( void, __cdecl, IN_ClearStates, void );
//DECLARE_HOOK( void, __cdecl, IN_Accumulate, void );
DECLARE_HOOK( void, __cdecl, CL_CreateMove, float frametime, usercmd_t *cmd, int active );
//DECLARE_HOOK( int, __cdecl, CL_IsThirdPerson );
//DECLARE_HOOK( kbutton_t *, __cdecl, KB_Find, const char *name );
//DECLARE_HOOK( void, __cdecl, CAM_Think, void );
DECLARE_HOOK( void, __cdecl, V_CalcRefdef, ref_params_t *pparams );
DECLARE_HOOK( int, __cdecl, HUD_AddEntity, int type, cl_entity_t *ent, const char *modelname );
//DECLARE_HOOK( void, __cdecl, HUD_CreateEntities, void );
//DECLARE_HOOK( void, __cdecl, HUD_DrawNormalTriangles, void );
DECLARE_HOOK( void, __cdecl, HUD_DrawTransparentTriangles, void );
DECLARE_HOOK( void, __cdecl, HUD_StudioEvent, const mstudioevent_t *studio_event, const cl_entity_t *entity );
DECLARE_HOOK( void, __cdecl, HUD_PostRunCmd, local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed );
//DECLARE_HOOK( void, __cdecl, HUD_TxferLocalOverrides, entity_state_t *state, const clientdata_t *client );
//DECLARE_HOOK( void, __cdecl, HUD_ProcessPlayerState, entity_state_t *dst, const entity_state_t *src );
//DECLARE_HOOK( void, __cdecl, HUD_TxferPredictionData, entity_state_t *ps, const entity_state_t *pps, clientdata_t *pcd, const clientdata_t *ppcd, weapon_data_t *wd, const weapon_data_t *pwd );
DECLARE_HOOK( void, __cdecl, Demo_ReadBuffer, int size, unsigned const char *buffer );
//DECLARE_HOOK( int, __cdecl, HUD_ConnectionlessPacket, netadr_t *net_from, const char *args, const char *response_buffer, int *response_buffer_size );
//DECLARE_HOOK( int, __cdecl, HUD_GetHullBounds, int hullnumber, float *mins, float *maxs );
DECLARE_HOOK( void, __cdecl, HUD_Frame, double time );
DECLARE_HOOK( int, __cdecl, HUD_Key_Event, int down, int keynum, const char *pszCurrentBinding );
//DECLARE_HOOK( void, __cdecl, HUD_TempEntUpdate, double frametime, double client_time, double cl_gravity, TEMPENTITY **ppTempEntFree, TEMPENTITY **ppTempEntActive, int ( *Callback_AddVisibleEntity )( cl_entity_t *pEntity ), void ( *Callback_TempEntPlaySound )( TEMPENTITY *pTemp, float damp ) );
//DECLARE_HOOK( cl_entity_t *, __cdecl, HUD_GetUserEntity, int index );
//DECLARE_HOOK( void, __cdecl, HUD_VoiceStatus, int entindex, qboolean bTalking );
//DECLARE_HOOK( void, __cdecl, HUD_DirectorMessage, unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags );
//DECLARE_HOOK( void, __cdecl, HUD_ChatInputPosition, int *x, int *y );
DECLARE_HOOK( void, __cdecl, HUD_OnClientDisconnect );

*/
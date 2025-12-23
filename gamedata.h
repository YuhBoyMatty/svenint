// SvenInt (c) Sw1ft
// gamedata.h

#ifndef SINT_GAMEDATA_H
#define SINT_GAMEDATA_H

#ifdef WIN32
#pragma once
#endif

#include "utils/memory_utils.h"

#include "game/gameui/IGameUI.h"
#include "game/gameui/IGameConsole.h"
#include "game/gameui/IRunGameEngine.h"

#include "game/vgui2/IVGui.h"
#include "game/vgui2/ISurface.h"
#include "game/vgui2/IInput.h"
#include "game/vgui2/IInputInternal.h"
#include "game/vgui2/IKeyValues.h"
#include "game/vgui2/ILocalize.h"
#include "game/vgui2/IPanel.h"
#include "game/vgui2/IScheme.h"
#include "game/vgui2/ISystem.h"
#include "game/vgui2/IBaseUI.h"
#include "game/vgui2/IGameUIFuncs.h"
#include "game/vgui2/IEngineSurface.h"
#include "game/vgui2/IEngineVGui.h"

#include "game/interface.h"
#include "game/localplayer.h"
#include "game/CBaseEngineClient.h"
#include "game/CBaseExtraPlayerInfo.h"
#include "game/CBasePlayerMove.h"
#include "game/CBaseWeaponsResource.h"
#include "game/IFileSystem.h"
#include "game/commandline.h"
#include "game/game_hooks.h"
#include "game/game_utils.h"
#include "game/hook_events.h"
#include "game/convar.h"
#include "game/cvar.h"
#include "game/dbg.h"
#include "game/hlsdk_mini.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define DUMP_FILE_OFFSET ( 0 )

#if DUMP_FILE_OFFSET
void InitPrintDumpFileOffset( void );
void ShutdownPrintDumpFileOffset( void );
void PrintDumpFileOffset( const char *pszSymbolName, uint32_t ptr, module_t hModule );
#define GAMEDATA_DUMP_FILE_OFFSET( name, ptr, lib ) PrintDumpFileOffset( name, (uint32_t)ptr, lib );
#else
#define GAMEDATA_DUMP_FILE_OFFSET( name, ptr, lib ) (void)0
#endif

#define SVEN_VERSION_CHECK( major, minor, patch ) ( ( major << 16 ) | ( minor << 8 ) | ( patch ) )
#define SVEN_VERSION() SVEN_VERSION_CHECK( Globals::gameversion_major, Globals::gameversion_minor, Globals::gameversion_patch )

//-----------------------------------------------------------------------------
// Game data finder
//-----------------------------------------------------------------------------

namespace KeyValuesParser
{
	struct KeyValues;
}

class CGameDataFinder
{
public:
	CGameDataFinder() : m_pGamedata( NULL ), m_bPreferRVA( false )
	{
	}

	bool        Init( void );
	void        Shutdown( void );

	// Tries to automatically find a given symbol (i.e. a function), first is an RVA offset,
	// if it fails we do switch to FindPattern (Windows) or FindSymbol (Linux)
	// RVA offset name and pattern/symbol name are supposed to be the same
	void		*AutoFind( module_t hModule, const char *pszModuleName, const char *pszSymbolName, bool bNotify = true );

	void		*FindRVA( module_t hModule, const char *pszModuleName, const char *pszRvaOffsetName, bool bNotify = true );
	void		*FindPattern( module_t hModule, const char *pszModuleName, const char *pszPatternName, bool bNotify = true, uint32_t offset = 0 );
	void		*FindSymbol( module_t hModule, const char *pszModuleName, const char *pszSymbolName, bool bNotify = true );
	uint32_t    FindOffset( module_t hModule, const char *pszModuleName, const char *pszOffsetName, bool bNotify = true );

	inline bool Initialized( void ) const { return m_pGamedata != NULL; }
	inline bool	PreferRVA( void ) const { return m_bPreferRVA; }

private:
	KeyValuesParser::KeyValues *m_pGamedata;
	bool						m_bPreferRVA;
};

//-----------------------------------------------------------------------------
// Various gamedata (patterns, modules, interfaces, function pointers, variables, etc.)
//-----------------------------------------------------------------------------

namespace Globals
{
	extern CGameDataFinder *gamedata;

	extern int gameversion;
	extern int gameversion_major;
	extern int gameversion_minor;
	extern int gameversion_patch;

	extern char szGameVersion[ 16 ];

	extern double *host_frametime;
	extern double *realtime;

	extern bool demoplayback;
	extern int servernumber;
	extern uint32_t mapcrc;
	extern int *gpHideHUD;

	extern sizebuf_t *clc_buffer;
	extern sizebuf_t *net_message;
	extern int *msg_readcount;
	extern int *msg_badread;

	extern int *cmd_argc;
	extern const char **cmd_argv;
	extern cvar_t **cvar_vars;
	extern cmd_function_t **cmd_functions;
	extern CCvar *cvar;

	extern netmsg_t *g_NetworkMessages;
	extern usermsg_t **g_pClientUserMsgs;
	extern event_hook_t *g_pEventHooks;

	extern CGameHooks *gamehooks;
	extern CGameUtils *gameutils;
	extern CHookEvents *hookevents;

	extern globalvars_t *gpGlobals;
	extern enginefuncs_t *sv_enginefuncs;
	extern DLL_FUNCTIONS *dllFuncs;
	extern NEW_DLL_FUNCTIONS *dllFuncs2;

	extern cl_enginefunc_t *cl_enginefuncs;
	extern cldll_func_t *cl_funcs;
	extern engine_studio_api_t *enginestudio;
	extern r_studio_interface_t *studioapi;
	extern CStudioModelRenderer *studiorenderer;
	extern CBaseEngineClient *engineclient;
	extern CBaseExtraPlayerInfo *extraplayerinfo;
	extern CBasePlayerMove *playermove;
	extern CBaseWeaponsResource *weaponsresource;
	extern IVideoMode **videomode_ptr;
	extern client_static_t *cls;
	extern ref_params_t *refparams;
	extern movevars_t *refparams_movevars;

	extern CLocalPlayer *localplayer;
	extern CClientWeapon *clientweapon;
	extern CInventory *inventory;

	extern IFileSystem *filesystem;
	extern CCommandLine *commandline;

	extern IGameUI *gameUI;
	extern IGameConsole *gameconsole;
	extern IRunGameEngine *rungameengine;

	extern vgui::IVGui *vgui;
	extern IBaseUI *baseUI;
	extern IGameUIFuncs *gameUIFuncs;
	extern IEngineSurface *enginesurface;
	extern vgui::IEngineVGui *enginevgui;
	extern vgui::ISurface *surface;
	extern vgui::ISchemeManager *schememanager;
	extern vgui::ILocalize *localize;
	extern vgui::IPanel *panel;
	extern vgui::IInput *input;
	extern vgui::IInputInternal *inputinternal;
	extern vgui::ISystem *system;
	extern vgui::IKeyValues *keyvalues;
}

namespace GameData
{
	namespace Modules
	{
		extern module_t Engine;
		extern module_t Client;
		extern module_t Server;
		extern module_t FileSystem;
		extern module_t GameUI;
		extern module_t VGUI2;
		extern module_t SDL2;
		extern module_t OpenGL;
	}

	namespace InterfaceFactories
	{
		extern CreateInterfaceFn Engine;
		extern CreateInterfaceFn Client;
		extern CreateInterfaceFn Server;
		extern CreateInterfaceFn FileSystem;
		extern CreateInterfaceFn GameUI;
		extern CreateInterfaceFn VGUI2;
	}

	namespace Patterns
	{
		namespace Engine
		{
			EXTERN_PATTERN( Sys_InitializeGameDLL );
			EXTERN_PATTERN( ClientDLL_Init );
			EXTERN_PATTERN( Netchan_Transmit );
			EXTERN_PATTERN( SCR_BeginLoadingPlaque );
			EXTERN_PATTERN( V_EngineStudio_Init );
			EXTERN_PATTERN( Z_Free );
			EXTERN_PATTERN( Mem_Free );
			EXTERN_PATTERN( Cmd_AddCommand );

			EXTERN_PATTERNS( Cvar_RemoveClientDLLCvars );
			EXTERN_PATTERNS( Cvar_DirectSet );
			EXTERN_PATTERNS( LoadInsecureClient );
			EXTERN_PATTERNS( MSG_WriteUsercmd );
			EXTERN_PATTERNS( SCR_UpdateScreen );
			EXTERN_PATTERNS( SCR_EndLoadingPlaque );
			EXTERN_PATTERNS( V_VideoMode_Create );
			EXTERN_PATTERNS( Host_FilterTime );
			EXTERN_PATTERNS( MSG_ReadByte );
		}

		namespace Client
		{
			EXTERN_PATTERNS( IN_Move );
			EXTERN_PATTERNS( GetClientColor );
			EXTERN_PATTERNS( CHudAmmo__VidInit );
			EXTERN_PATTERNS( __MsgFunc_ServerVer );
		}

		namespace Server
		{
			EXTERN_PATTERNS( PlayerSpawns );
			EXTERN_PATTERNS( FixPlayerStuck );
			EXTERN_PATTERNS( CBaseEntity__FireBullets );
			EXTERN_PATTERNS( UTIL_GetCircularGaussianSpread );
			EXTERN_PATTERNS( FireTargets );
			EXTERN_PATTERNS( CopyPEntityVars );
		}

		namespace GameUI
		{
			EXTERN_PATTERNS( CGameConsoleDialog__DPrint );
		}
	}

	namespace Cvars
	{
		extern cvar_t *host_framerate;
		extern cvar_t *fps_max;
		extern cvar_t *hud_draw;
	}

	namespace Offsets
	{
		namespace Server
		{
			extern size_t vtidx_CBaseEntity_ObjectCaps;

			extern size_t vtidx_CBasePlayer_IsAlive;
			extern size_t vtidx_CBasePlayer_BeginRevive;
			extern size_t vtidx_CBasePlayer_EndRevive;
			extern size_t vtidx_CBasePlayer_SpecialSpawn;
			extern size_t vtidx_CBasePlayer_IsConnected;
		}

		namespace Client
		{
			extern size_t vtidx_CStudioModelRenderer__StudioSetupBones;
			extern size_t vtidx_CStudioModelRenderer__StudioRenderModel;
		}

		namespace VGUI2
		{
			extern size_t vtidx_IPanel__PaintTraverse;
		}
	}

	namespace Pointers
	{
		namespace Engine
		{
			extern void *Z_Free;
			extern void *Mem_Free;
			extern void *Cvar_DirectSet;
			extern void *GL_Bind;
			extern void *Host_FilterTime;
			extern void *Host_IsServerActive;
			extern void *Key_Event;
			extern void *Netchan_Transmit;
			extern void *MSG_WriteUsercmd;
			extern void *SCR_UpdateScreen;
			extern void *SCR_BeginLoadingPlaque;
			extern void *SCR_EndLoadingPlaque;
		}

		namespace Client
		{
			extern void *IN_Move;
			extern void *GetClientColor;
		}

		namespace Server
		{
			extern void *vmt_CBasePlayer;
			extern void *pSurvivalModeInstance;
			extern void *toggle_survival_mode_Callback;
			extern void *PlayerSpawns;
			extern void *FixPlayerStuck;
			extern void *CBaseEntity__FireBullets;
			extern void *UTIL_GetCircularGaussianSpread;
			extern void *FireTargets;
			extern void *CopyPEntityVars;
		}

		namespace GameUI
		{
			extern void *RichText__InsertColorChange;
			extern void *RichText__InsertString;
		}
	}

	bool GetGameModules( void );
	bool GetGameInterfaceFactories( void );
	bool GetGameInterfaces( void );

	bool FindGameSymbols( void );

	void *GetInterfaceIteratively( CreateInterfaceFn interfaceFactory, const char *pszInterfaceVersion );
}

#endif // !SINT_GAMEDATA_H

// SvenInt (c) Sw1ft
// gamedata.cpp

#include "stdafx.h"
#include "game/dbg.h"

#include <Windows.h>

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#if 1
#define MsgWrapper DevMsg
#else
#define MsgWrapper Msg
#endif

// welp yeah
#define GAMEDATA_CHECK_SYMBOL( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_STATUS( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning2( "<SvenInt::GameData> Failed to locate symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL_STATUS( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning2( "<SvenInt::GameData> Failed to locate symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERN( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning2( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL_STATUS( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning2( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

#define GAMEDATA_CHECK_MODULE( var, modulename, futurevar ) \
	if ( ( Modules::var = futurevar.get() ) == NULL ) { Warning( "<SvenInt::GameData> Failed to get library \"%s\"\n", modulename ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", modulename, Modules::var ); }

#define GAMEDATA_CHECK_MODULE_INFO( modulename, futurevar ) \
	if ( !futurevar.get() ) { Warning( "<SvenInt::GameData> Failed to retrieve module's info of library \"%s\"\n", modulename ); bOK = false; }

#define GAMEDATA_CHECK_IFACE_FACTORY( var, modulename, futurevar ) \
	if ( ( InterfaceFactories::var = (CreateInterfaceFn)futurevar.get() ) == NULL ) { Warning( "<SvenInt::GameData> Failed to get interface factory for library \"%s\"\n", modulename ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Received interface factory of library \"%s\" at address 0x%X\n", modulename, InterfaceFactories::var ); } \

#define GAMEDATA_CHECK_IFACE( var, name, vername ) \
	if ( Globals::var == NULL ) { Warning( "<SvenInt::GameData> Failed to get interface \"%s\"\n", name ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Received interface \"%s\" of version \"%s\" at address 0x%X\n", name, vername, Globals::var ); } \

#define GAMEDATA_CHECK_IFACE_NOT_CRITICAL( var, name, vername ) \
	if ( Globals::var == NULL ) { Warning2( "<SvenInt::GameData> Failed to get interface \"%s\"\n", name ); } \
	else { MsgWrapper( "<SvenInt::GameData> Received interface \"%s\" of version \"%s\" at address 0x%X\n", name, vername, Globals::var ); } \

//-----------------------------------------------------------------------------
// Various gamedata (patterns, modules, interfaces, function pointers, variables, etc.)
//-----------------------------------------------------------------------------

namespace Globals
{
	int gameversion = 0;
	double *host_frametime = NULL;
	double *realtime = NULL;

	bool demoplayback = 0;
	int servernumber = 0;
	uint32_t mapcrc = 0;
	int *gpHideHUD = NULL;

	sizebuf_t *clc_buffer = NULL;
	sizebuf_t *net_message = NULL;
	int *msg_readcount = NULL;
	int *msg_badread = NULL;

	int *cmd_argc = NULL;
	const char **cmd_argv = NULL;
	cvar_t **cvar_vars = NULL;
	cmd_function_t **cmd_functions = NULL;
	CCvar *cvar = NULL;

	netmsg_t *g_NetworkMessages = NULL;
	usermsg_t **g_pClientUserMsgs = NULL;
	event_hook_t *g_pEventHooks = NULL;

	CGameHooks *gamehooks = NULL;
	CGameUtils *gameutils = NULL;
	CHookEvents *hookevents = NULL;

	globalvars_t *gpGlobals = NULL;
	enginefuncs_t *sv_enginefuncs = NULL;
	DLL_FUNCTIONS *dllFuncs = NULL;
	NEW_DLL_FUNCTIONS *dllFuncs2 = NULL;

	cl_enginefunc_t *cl_enginefuncs = NULL;
	cldll_func_t *cl_funcs = NULL;
	engine_studio_api_t *enginestudio = NULL;
	r_studio_interface_t *studioapi = NULL;
	CStudioModelRenderer *studiorenderer = NULL;
	CBaseEngineClient *engineclient = NULL;
	CBaseExtraPlayerInfo *extraplayerinfo = NULL;
	CBasePlayerMove *playermove = NULL;
	CBaseWeaponsResource *weaponsresource = NULL;
	IVideoMode **videomode_ptr = NULL;
	client_static_t *cls = NULL;
	ref_params_t *refparams = NULL;
	movevars_t *refparams_movevars = NULL;

	CLocalPlayer *localplayer = NULL;
	CClientWeapon *clientweapon = NULL;
	CInventory *inventory = NULL;

	IFileSystem *filesystem = NULL;
	CCommandLine *commandline = NULL;

	IGameUI *gameUI = NULL;
	IGameConsole *gameconsole = NULL;
	IRunGameEngine *rungameengine = NULL;

	vgui::IVGui *vgui = NULL;
	IBaseUI *baseUI = NULL;
	IGameUIFuncs *gameUIFuncs = NULL;
	IEngineSurface *enginesurface = NULL;
	vgui::IEngineVGui *enginevgui = NULL;
	vgui::ISurface *surface = NULL;
	vgui::ISchemeManager *schememanager = NULL;
	vgui::ILocalize *localize = NULL;
	vgui::IPanel *panel = NULL;
	vgui::IInput *input = NULL;
	vgui::IInputInternal *inputinternal = NULL;
	vgui::ISystem *system = NULL;
	vgui::IKeyValues *keyvalues = NULL;

	static DLL_FUNCTIONS s_dllFuncs;
	static NEW_DLL_FUNCTIONS s_dllFuncs2;

	static ref_params_t s_refparams;
	static movevars_t s_refparams_movevars;
}

namespace GameData
{
	namespace Modules
	{
		module_t Engine = NULL;
		module_t Client = NULL;
		module_t Server = NULL;
		module_t FileSystem = NULL;
		module_t GameUI = NULL;
		module_t VGUI2 = NULL;
		module_t SDL2 = NULL;
		module_t OpenGL = NULL;
	}

	namespace InterfaceFactories
	{
		CreateInterfaceFn Engine = NULL;
		CreateInterfaceFn Client = NULL;
		CreateInterfaceFn Server = NULL;
		CreateInterfaceFn FileSystem = NULL;
		CreateInterfaceFn GameUI = NULL;
		CreateInterfaceFn VGUI2 = NULL;
	}

	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERN( Sys_InitializeGameDLL, "E8 ? ? ? ? 33 C0 83 3D ? ? ? ? ? 0F 9F C0 50 E8 ? ? ? ? 83 C4" );
			DEFINE_PATTERN( ClientDLL_Init, "68 ? ? ? ? FF 15 ? ? ? ? A1 ? ? ? ? 83 C4" );
			DEFINE_PATTERN( Netchan_Transmit, "B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? ? ? ? 8B 84 24 ? ? ? ? 53 55" );
			DEFINE_PATTERN( SCR_BeginLoadingPlaque, "6A ? E8 ? ? ? ? A1 ? ? ? ? 83 C4 ? 83 F8" );
			DEFINE_PATTERN( V_EngineStudio_Init, "68 ? ? ? ? 68 ? ? ? ? 6A ? FF D0 83 C4 ? 85 C0 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? E8 ? ? ? ? 68" );
			DEFINE_PATTERN( Z_Free, "56 8B 74 24 08 85 F6 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 83 C6" );
			DEFINE_PATTERN( Mem_Free, "8B FF 55 8B EC 83 7D 08 ? 74 ? FF 75 08 6A" );
			DEFINE_PATTERN( Cmd_AddCommand, "57 8B 7C 24 08 57 E8 ? ? ? ? 83 C4 ? 80 38" );

			DEFINE_PATTERNS_2( Cvar_RemoveClientDLLCvars,
							   "5.22",
							   "56 8B 35 ? ? ? ? 57 33 FF 85 F6 74 ? 53",
							   "5.11",
							   "55 56 8B 35 ? ? ? ? 33 ED" );
			DEFINE_PATTERNS_2( Cvar_DirectSet,
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? 04 00 00 53 8B 9C 24 ? 04 00 00 57 8B BC 24",
							   "5.11",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? 04 00 00 56 8B B4 24 ? 04 00 00 57 8B BC 24" );
			DEFINE_PATTERNS_2( LoadInsecureClient,
							   "5.22",
							   "FF 74 24 04 E8 ? ? ? ? 83 C4 ? A3 ? ? ? ? 85 C0 75",
							   "5.11",
							   "56 8B 74 24 08 56 E8 ? ? ? ? 83 C4 ? A3 ? ? ? ? 85 C0" );
			DEFINE_PATTERNS_2( MSG_WriteUsercmd,
							   "5.25",
							   "56 68 ? ? ? ? E8 ? ? ? ? 8B 74 24 0C",
							   "5.11",
							   "56 68 ? ? ? ? E8 ? ? ? ? 8B 4C 24 0C" );
			DEFINE_PATTERNS_2( SCR_UpdateScreen,
							   "5.25",
							   "83 EC ? A1 ? ? ? ? 33 C4 89 44 24 40 83 3D ? ? ? ? ? 0F 85 ? ? ? ? C7 05",
							   "5.11",
							   "83 EC ? 56 33 F6 39 35 ? ? ? ? 0F 85" );
			DEFINE_PATTERNS_2( SCR_EndLoadingPlaque,
							   "5.22",
							   "C7 05 ? ? ? ? ? ? ? ? E8 ? ? ? ? 68 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? C7 05",
							   "5.11",
							   "33 C0 A3 ? ? ? ? A3 ? ? ? ? E8 ? ? ? ? 68" );
			DEFINE_PATTERNS_3( V_VideoMode_Create,
							   "5.25",
							   "A3 ? ? ? ? 8B 4D F4 64 89 0D 00 00 00 00 59 5E 5B 8B E5 5D C3",
							   "5.22",
							   "A3 ? ? ? ? 8B 4D F4",
							   "5.11",
							   "A3 ? ? ? ? 59 C3 33 C0" );
			DEFINE_PATTERNS_3( Host_FilterTime,
							   "5.25",
							   "D9 ? D9 ? ? ? ? ? 8B 0D ? ? ? ? D8 D1",
							   "5.22",
							   "51 D9 ? D9 ? ? ? ? ? 8B 0D ? ? ? ? D8 D1",
							   "5.11",
							   "51 D9 ? 8B 0D ? ? ? ? D9 05" );
			DEFINE_PATTERNS_2( MSG_ReadByte,
							   "5.22",
							   "8B 0D ? ? ? ? 8D 51 01 3B 15 ? ? ? ? 7E ? C7 05 ? ? ? ? ? ? ? ? 83 C8 ? C3 A1 ? ? ? ? 0F B6 04 08",
							   "5.11",
							   "A1 ? ? ? ? 8D 48 01 3B 0D ? ? ? ? 7E ? C7 05 ? ? ? ? ? ? ? ? 83 C8 ? C3 8B 15 ? ? ? ? 0F B6 04 02" );
			DEFINE_PATTERNS_1( Key_Event,
							   "5.11",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 00 04 00 00 56 8B B4 24 0C 04 00 00 81 FE" );
		}

		namespace Client
		{
			DEFINE_PATTERNS_2( IN_Move,
							   "5.25",
							   "83 3D ? ? ? ? ? 75 ? 83 3D ? ? ? ? ? 74 ? FF 74 24 08",
							   "5.11",
							   "83 3D ? ? ? ? ? 56 8B 74 24 0C 75 1A" );
			DEFINE_PATTERNS_2( GetClientColor,
							   "5.23",
							   "8B 4C 24 04 85 C9 7E",
							   "5.11",
							   "8B 44 24 04 85 C0 7E" );
			DEFINE_PATTERNS_3( CHudAmmo__VidInit,
							   "5.26",
							   "81 FE ? ? ? ? 7C ? 5E B0 ? C3",
							   "5.22",
							   "81 FE ? ? ? ? 7C ? B0 ? 5E C3",
							   "5.11",
							   "81 FE ? ? ? ? 7C ? 81 3D ? ? ? ? ? ? ? ? 5E" );
			DEFINE_PATTERNS_2( __MsgFunc_ServerVer,
							   "5.23",
							   "8D 44 24 40 C6 44 24 3C 00 68 ? ? ? ? 50 0F 11 44 24 48",
							   "5.11",
							   "8D 54 24 1C 68 ? ? ? ? 52 89 44 24 24" );
		}

		namespace Server
		{
			DEFINE_PATTERNS_2( PlayerSpawns,
							   "5.26",
							   "56 8B 74 24 0C 57 8B 7C 24 0C 85 FF 0F 84 ? ? ? ? 83 3F",
							   "5.11",
							   "57 8B 7C 24 08 85 FF 0F 84 ? ? ? ? 83 3F ? 0F" );
			DEFINE_PATTERNS_3( FixPlayerStuck,
							   "5.26",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 00 03 00 00",
							   "5.25",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 FC 02 00 00",
							   "5.11",
							   "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 0C 03 00 00 56" );
			DEFINE_PATTERNS_3( CBaseEntity__FireBullets,
							   "5.26",
							   "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F1",
							   "5.25",
							   "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 89 7D F0",
							   "5.11",
							   "6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 81 EC ? ? ? ? 53 55 56 57 A1 ? ? ? ? 33 C4 50 8D 84 24 BC 00 00 00" );
			DEFINE_PATTERNS_3( UTIL_GetCircularGaussianSpread,
							   "5.26",
							   "56 8B 74 24 08 57 8B 7C 24 10 66 0F 1F 44 00 00",
							   "5.25",
							   "56 8B 74 24 08 57 8B 7C 24 10 66 0F 1F 44 00 00",
							   "5.11",
							   "55 8B EC 83 E4 ? 83 EC ? 56 8B 75 08 57 8B 7D 0C" );
			DEFINE_PATTERNS_3( FireTargets,
							   "5.26",
							   "51 53 8B 5C 24 0C 57 8B 7C 24 14",
							   "5.25",
							   "53 8B 5C 24 08 85 DB 0F 84 ? ? ? ? 80 3B ? 0F 84 ? ? ? ? 55",
							   "5.11",
							   "8B 44 24 04 56 33 F6 3B C6" );
			DEFINE_PATTERNS_3( CopyPEntityVars,
							   "5.26",
							   "E8 ? ? ? ? F3 0F 10 84 24 90 01 00 00",
							   "5.25",
							   "E8 ? ? ? ? F3 0F 10 84 24 8C 01 00 00",
							   "5.11",
							   "E8 ? ? ? ? D9 ? D9 ? ? ? ? ? ? D8 ? DF E0 DD ? F6 C4 ? 74" );
		}

		namespace GameUI
		{
			DEFINE_PATTERNS_2( CGameConsoleDialog__DPrint,
							   "5.22",
							   "55 8B EC 56 8B F1 FF B6 2C 01 00 00",
							   "5.11",
							   "56 8B F1 8B 86 2C 01 00 00 8B 8E 18 01 00 00" );
		}
	}

	namespace Cvars
	{
		cvar_t *host_framerate = NULL;
		cvar_t *fps_max = NULL;
		cvar_t *hud_draw = NULL;
	}

	namespace Offsets
	{
		namespace Server
		{
			size_t vtidx_CBaseEntity_ObjectCaps = ~0;
			
			size_t vtidx_CBasePlayer_IsAlive = ~0;
			size_t vtidx_CBasePlayer_BeginRevive = ~0;
			size_t vtidx_CBasePlayer_EndRevive = ~0;
			size_t vtidx_CBasePlayer_SpecialSpawn = ~0;
			size_t vtidx_CBasePlayer_IsConnected = ~0;
		}
	}

	namespace Pointers
	{
		namespace Engine
		{
			void *Z_Free = NULL;
			void *Mem_Free = NULL;
			void *Cvar_DirectSet = NULL;
			void *GL_Bind = NULL;
			void *Host_FilterTime = NULL;
			void *Host_IsServerActive = NULL;
			void *Key_Event = NULL;
			void *Netchan_Transmit = NULL;
			void *MSG_WriteUsercmd = NULL;
			void *SCR_UpdateScreen = NULL;
			void *SCR_BeginLoadingPlaque = NULL;
			void *SCR_EndLoadingPlaque = NULL;
		}

		namespace Client
		{
			void *IN_Move = NULL;
			void *GetClientColor = NULL;
		}

		namespace Server
		{
			void *vmt_CBasePlayer = NULL;
			void *pSurvivalModeInstance = NULL;
			void *toggle_survival_mode_Callback = NULL;
			void *PlayerSpawns = NULL;
			void *FixPlayerStuck = NULL;
			void *CBaseEntity__FireBullets = NULL;
			void *CCrossbow__PrimaryAttack = NULL;
			void *UTIL_GetCircularGaussianSpread = NULL;
			void *FireTargets = NULL;
			void *CopyPEntityVars = NULL;
		}

		namespace GameUI
		{
			void *RichText__InsertColorChange = NULL;
			void *RichText__InsertString = NULL;
		}
	}

	bool GetGameDLLModules( void )
	{
		bool bOK = true;

		std::future<bool> fLoadServerDLL;

		auto fModuleEngine = std::async( GetModuleHandle, "hw.dll" );
		auto fModuleClient = std::async( GetModuleHandle, "client.dll" );
		auto fModuleServer = std::async( GetModuleHandle, "server.dll" );
		auto fModuleFileSystem = std::async( GetModuleHandle, "filesystem_stdio.dll" );
		auto fModuleGameUI = std::async( GetModuleHandle, "GameUI.dll" );
		auto fModuleVGUI2 = std::async( GetModuleHandle, "vgui2.dll" );
		auto fModuleSDL2 = std::async( GetModuleHandle, "SDL2.dll" );
		auto fModuleOpenGL = std::async( GetModuleHandle, "opengl32.dll" );

		if ( ( Modules::Server = fModuleServer.get() ) == NULL )
		{
		#if 1
			fLoadServerDLL = std::async( [] {
				Modules::Server = GetModuleHandle( "server.dll" );
				if ( Modules::Server != NULL )
					return true;

				MsgWrapper( "<SvenInt::GameData> Preloading server binary\n" );

				void *pfnSys_InitializeGameDLL = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::Sys_InitializeGameDLL );
				if ( pfnSys_InitializeGameDLL == NULL )
				{
					Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", "Sys_InitializeGameDLL" );
					return false;
				}

				( ( void ( * )( void ) )pfnSys_InitializeGameDLL )( );

				if ( ( Modules::Server = GetModuleHandle( "server.dll" ) ) == NULL )
				{
					Warning( "<SvenInt::GameData> Failed to load server's binary\n" );
					return false;
				}

				MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", "server.dll", Modules::Server );
				return true;
			} );
		#else
			Warning2( "<SvenInt::GameData> Failed to get library \"%s\"\n", "server.dll" );
			bOK = false;
		#endif
		}
		else
		{
			MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", "server.dll", Modules::Server );
		}

		GAMEDATA_CHECK_MODULE( Engine, "hw.dll", fModuleEngine );
		GAMEDATA_CHECK_MODULE( Client, "client.dll", fModuleClient );
		GAMEDATA_CHECK_MODULE( FileSystem, "filesystem_stdio.dll", fModuleFileSystem );
		GAMEDATA_CHECK_MODULE( GameUI, "GameUI.dll", fModuleGameUI );
		GAMEDATA_CHECK_MODULE( VGUI2, "vgui2.dll", fModuleVGUI2 );
		GAMEDATA_CHECK_MODULE( SDL2, "SDL2.dll", fModuleSDL2 );
		GAMEDATA_CHECK_MODULE( OpenGL, "opengl32.dll", fModuleOpenGL );

		if ( !bOK )
			return false;

		// Store modules info
		auto fModuleInfoEngine = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::Engine ); } );
		auto fModuleInfoClient = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::Client ); } );
		auto fModuleInfoFileSystem = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::FileSystem ); } );
		auto fModuleInfoGameUI = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::GameUI ); } );
		auto fModuleInfoVGUI2 = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::VGUI2 ); } );
		auto fModuleInfoSDL2 = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::SDL2 ); } );
		auto fModuleInfoOpenGL = std::async( [] { return MemoryUtils()->RetrieveModuleInfo( Modules::OpenGL ); } );

		GAMEDATA_CHECK_MODULE_INFO( "hw.dll", fModuleInfoEngine );
		GAMEDATA_CHECK_MODULE_INFO( "client.dll", fModuleInfoClient );
		GAMEDATA_CHECK_MODULE_INFO( "filesystem_stdio.dll", fModuleInfoFileSystem );
		GAMEDATA_CHECK_MODULE_INFO( "GameUI.dll", fModuleInfoGameUI );
		GAMEDATA_CHECK_MODULE_INFO( "vgui2.dll", fModuleInfoVGUI2 );
		GAMEDATA_CHECK_MODULE_INFO( "SDL2.dll", fModuleInfoSDL2 );
		GAMEDATA_CHECK_MODULE_INFO( "opengl32.dll", fModuleInfoOpenGL );

		if ( fLoadServerDLL.valid() && !fLoadServerDLL.get() )
			return false;

		if ( !MemoryUtils()->RetrieveModuleInfo( Modules::Server ) )
		{
			Warning2( "<SvenInt::GameData> Failed to retrieve module's info of library \"%s\"\n", "server.dll" );
			bOK = false;
		}

		return bOK;
	}

	bool GetGameDLLInterfaceFactories( void )
	{
		bool bOK = true;

		auto fInterfaceFactoryEngine = std::async( GetProcAddress, (HMODULE)Modules::Engine, CREATEINTERFACE_PROCNAME );
		auto fInterfaceFactoryClient = std::async( GetProcAddress, (HMODULE)Modules::Client, CREATEINTERFACE_PROCNAME );
		auto fInterfaceFactoryServer = std::async( GetProcAddress, (HMODULE)Modules::Server, CREATEINTERFACE_PROCNAME );
		auto fInterfaceFactoryFileSystem = std::async( GetProcAddress, (HMODULE)Modules::FileSystem, CREATEINTERFACE_PROCNAME );
		auto fInterfaceFactoryGameUI = std::async( GetProcAddress, (HMODULE)Modules::GameUI, CREATEINTERFACE_PROCNAME );
		auto fInterfaceFactoryVGUI2 = std::async( GetProcAddress, (HMODULE)Modules::VGUI2, CREATEINTERFACE_PROCNAME );

		GAMEDATA_CHECK_IFACE_FACTORY( Engine, "hw.dll", fInterfaceFactoryEngine );
		GAMEDATA_CHECK_IFACE_FACTORY( Client, "client.dll", fInterfaceFactoryClient );
		GAMEDATA_CHECK_IFACE_FACTORY( Server, "server.dll", fInterfaceFactoryServer );
		GAMEDATA_CHECK_IFACE_FACTORY( FileSystem, "filesystem_stdio.dll", fInterfaceFactoryFileSystem );
		GAMEDATA_CHECK_IFACE_FACTORY( GameUI, "GameUI.dll", fInterfaceFactoryGameUI );
		GAMEDATA_CHECK_IFACE_FACTORY( VGUI2, "vgui2.dll", fInterfaceFactoryVGUI2 );

		return bOK;
	}

	bool GetGameDLLInterfaces( void )
	{
		bool bOK = true;

		// Engine
		const char szVEngineClient[] = ENGINECLIENT_INTERFACE_VERSION;
		const char szVBaseUI[] = BASEUI_INTERFACE_VERSION;
		const char szVGameUIFuncs[] = ENGINE_GAMEUIFUNCS_INTERFACE_VERSION;
		const char szVEngineSurface[] = ENGINE_SURFACE_INTERFACE_VERSION;
		const char szVEngineVGui[] = VENGINE_VGUI_VERSION;
		const char szVSurface[] = VGUI_SURFACE_INTERFACE_VERSION;
		// FileSystem
		const char szVFileSystem[] = FILESYSTEM_INTERFACE_VERSION;
		// GameUI
		const char szVGameUI[] = GAMEUI_INTERFACE_VERSION;
		const char szVGameConsole[] = GAMECONSOLE_INTERFACE_VERSION;
		const char szVRunGameEngine[] = RUNGAMEENGINE_INTERFACE_VERSION;
		// VGUI2
		const char szVGui[] = VGUI_IVGUI_INTERFACE_VERSION;
		const char szVSchemeManager[] = VGUI_SCHEME_INTERFACE_VERSION;
		const char szVLocalize[] = VGUI_LOCALIZE_INTERFACE_VERSION;
		const char szVPanel[] = VGUI_PANEL_INTERFACE_VERSION;
		const char szVInput[] = VGUI_INPUT_INTERFACE_VERSION;
		const char szVInputInternal[] = VGUI_INPUTINTERNAL_INTERFACE_VERSION;
		const char szVSystem[] = VGUI_SYSTEM_INTERFACE_VERSION;
		const char szVKeyValues[] = VGUI_KEYVALUES_INTERFACE_VERSION;

		// Engine
		auto fpEngineClient = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineClient );
		auto fpBaseUI = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVBaseUI );
		auto fpGameUIFuncs = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVGameUIFuncs );
		auto fpEngineSurface = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineSurface );
		auto fpEngineVGui = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineVGui );
		auto fpSurface = std::async( GetInterfaceIteratively, InterfaceFactories::Engine, szVSurface );
		// FileSystem
		auto fpFileSystem = std::async( GetInterfaceIteratively, InterfaceFactories::FileSystem, szVFileSystem );
		// GameUI
		auto fpGameUI = std::async( GetInterfaceIteratively, InterfaceFactories::GameUI, szVGameUI );
		auto fpGameConsole = std::async( GetInterfaceIteratively, InterfaceFactories::GameUI, szVGameConsole );
		auto fpRunGameEngine = std::async( GetInterfaceIteratively, InterfaceFactories::GameUI, szVRunGameEngine );
		// VGUI2
		auto fpVGui = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVGui );
		auto fpSchemeManager = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVSchemeManager );
		auto fpLocalize = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVLocalize );
		auto fpPanel = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVPanel );
		auto fpInput = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVInput );
		auto fpInputInternal = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVInputInternal );
		auto fpSystem = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVSystem );
		auto fpKeyValues = std::async( GetInterfaceIteratively, InterfaceFactories::VGUI2, szVKeyValues );

		// Engine
		Globals::engineclient = reinterpret_cast<CBaseEngineClient *>( fpEngineClient.get() );
		Globals::baseUI = reinterpret_cast<IBaseUI *>( fpBaseUI.get() );
		Globals::gameUIFuncs = reinterpret_cast<IGameUIFuncs *>( fpGameUIFuncs.get() );
		Globals::enginesurface = reinterpret_cast<IEngineSurface *>( fpEngineSurface.get() );
		Globals::enginevgui = reinterpret_cast<vgui::IEngineVGui *>( fpEngineVGui.get() );
		Globals::surface = reinterpret_cast<vgui::ISurface *>( fpSurface.get() );
		// FileSystem
		Globals::filesystem = reinterpret_cast<IFileSystem *>( fpFileSystem.get() );
		// GameUI
		Globals::gameUI = reinterpret_cast<IGameUI *>( fpGameUI.get() );
		Globals::gameconsole = reinterpret_cast<IGameConsole *>( fpGameConsole.get() );
		Globals::rungameengine = reinterpret_cast<IRunGameEngine *>( fpRunGameEngine.get() );
		// VGUI2
		Globals::vgui = reinterpret_cast<vgui::IVGui *>( fpVGui.get() );
		Globals::schememanager = reinterpret_cast<vgui::ISchemeManager *>( fpSchemeManager.get() );
		Globals::localize = reinterpret_cast<vgui::ILocalize *>( fpLocalize.get() );
		Globals::panel = reinterpret_cast<vgui::IPanel *>( fpPanel.get() );
		Globals::input = reinterpret_cast<vgui::IInput *>( fpInput.get() );
		Globals::inputinternal = reinterpret_cast<vgui::IInputInternal *>( fpInputInternal.get() );
		Globals::system = reinterpret_cast<vgui::ISystem *>( fpSystem.get() );
		Globals::keyvalues = reinterpret_cast<vgui::IKeyValues *>( fpKeyValues.get() );

		// Engine
		GAMEDATA_CHECK_IFACE_NOT_CRITICAL( engineclient, "SCEngineClient", szVEngineClient );
		GAMEDATA_CHECK_IFACE( baseUI, "BaseUI", szVBaseUI );
		GAMEDATA_CHECK_IFACE( gameUIFuncs, "VENGINE_GAMEUIFUNCS_VERSION", szVGameUIFuncs );
		GAMEDATA_CHECK_IFACE( enginesurface, "EngineSurface", szVEngineSurface );
		GAMEDATA_CHECK_IFACE( enginevgui, "VEngineVGui", szVEngineVGui );
		GAMEDATA_CHECK_IFACE( surface, "VGUI_Surface", szVSurface );
		// FileSystem
		GAMEDATA_CHECK_IFACE( filesystem, "VFileSystem", szVFileSystem );
		// GameUI
		GAMEDATA_CHECK_IFACE( gameUI, "GameUI", szVGameUI );
		GAMEDATA_CHECK_IFACE( gameconsole, "GameConsole", szVGameConsole );
		GAMEDATA_CHECK_IFACE( rungameengine, "RunGameEngine", szVRunGameEngine );
		// VGUI2
		GAMEDATA_CHECK_IFACE( vgui, "VGUI_ivgui", szVGui );
		GAMEDATA_CHECK_IFACE( schememanager, "VGUI_Scheme", szVSchemeManager );
		GAMEDATA_CHECK_IFACE( localize, "VGUI_Localize", szVLocalize );
		GAMEDATA_CHECK_IFACE( panel, "VGUI_Panel", szVPanel );
		GAMEDATA_CHECK_IFACE( input, "VGUI_Input", szVInput );
		GAMEDATA_CHECK_IFACE( inputinternal, "VGUI_InputInternal", szVInputInternal );
		GAMEDATA_CHECK_IFACE( system, "VGUI_System", szVSystem );
		GAMEDATA_CHECK_IFACE_NOT_CRITICAL( keyvalues, "KeyValues", szVKeyValues );

		return bOK;
	}

	bool FindGameSymbols( void )
	{
		bool bOK = true;

		// Find game version
		auto fGameVersion = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( f__MsgFunc_ServerVer );

			MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::__MsgFunc_ServerVer, f__MsgFunc_ServerVer );

			void *p__MsgFunc_ServerVer = MemoryUtils()->GetPatternFutureValue( f__MsgFunc_ServerVer, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( p__MsgFunc_ServerVer,
											"__MsgFunc_ServerVer",
											Patterns::Client::__MsgFunc_ServerVer,
											patternIndex );

			ud_t inst;
			int iMajorVer = 0, iMinorVer = 0;
			MemoryUtils()->InitDisasm( &inst, p__MsgFunc_ServerVer, 32, 48 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Ipush && inst.operand[ 0 ].type == UD_OP_IMM )
				{
					char *pszClientVersion = strdup( (const char *)inst.operand[ 0 ].lval.udword );
					if ( pszClientVersion == NULL )
						break;

					char *buffer = pszClientVersion;
					char *pszFirstSeparator = strchr( buffer, '.' );
					if ( pszFirstSeparator == NULL )
					{
						Warning( "<SvenInt::GameData> Tried to find game's version in format 'X.XX' but got this: '%s'\n", pszClientVersion );
						free( pszClientVersion );
						break;
					}

					*pszFirstSeparator = 0;
					iMajorVer = atoi( pszClientVersion );
					*pszFirstSeparator = '.';

					pszFirstSeparator += 1;
					iMinorVer = atoi( pszFirstSeparator );

					Globals::gameversion = iMajorVer * 100 + iMinorVer;

					free( pszClientVersion );
					break;
				}
			}

			if ( Globals::gameversion == 0 )
			{
				Warning( "<SvenInt::GameData> Failed to locate game's version\n" );
				return false;
			}

			MsgWrapper( "<SvenInt::GameData> Found game's version: \"%d.%.2d\"\n", iMajorVer, iMinorVer );
			return true;
		} );

		// Find console printing
		auto fConsolePrint = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fCGameConsoleDialog__DPrint );

			MemoryUtils()->FindPatternAsync( Modules::GameUI, Patterns::GameUI::CGameConsoleDialog__DPrint, fCGameConsoleDialog__DPrint );

			void *pCGameConsoleDialog__DPrint = MemoryUtils()->GetPatternFutureValue( fCGameConsoleDialog__DPrint, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( pCGameConsoleDialog__DPrint,
											"CGameConsoleDialog::DPrint",
											Patterns::GameUI::CGameConsoleDialog__DPrint,
											patternIndex );

			ud_t inst;
			bool bFoundFirstCall = false;
			int iDisassembledBytes = 0;
			uint8_t *p = (uint8_t *)pCGameConsoleDialog__DPrint;

			MemoryUtils()->InitDisasm( &inst, pCGameConsoleDialog__DPrint, 32, 128 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icall && inst.operand[ 0 ].type == UD_OP_JIMM )
				{
					if ( !bFoundFirstCall )
					{
						Pointers::GameUI::RichText__InsertColorChange = MemoryUtils()->CalcAbsoluteAddress( p );
						bFoundFirstCall = true;
					}
					else
					{
						Pointers::GameUI::RichText__InsertString = MemoryUtils()->CalcAbsoluteAddress( p );
						break;
					}
				}

				p += iDisassembledBytes;
			}

			GAMEDATA_CHECK_SYMBOL( Pointers::GameUI::RichText__InsertColorChange, "RichText::InsertColorChange" );
			GAMEDATA_CHECK_SYMBOL( Pointers::GameUI::RichText__InsertString, "RichText::InsertString" );

			return true;
		} );

		// Find 'cl_enginefunc_t *cl_enginefuncs' & 'playermove_t *g_clmove'
		auto fEngineFuncsAndPlayerMove = std::async( [] {
			void *pClientDLL_Init = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::ClientDLL_Init );
			GAMEDATA_CHECK_SYMBOL_PATTERN( pClientDLL_Init, "ClientDLL_Init" );

			ud_t inst;
			bool bFoundEngfuncs = false;
			MemoryUtils()->InitDisasm( &inst, pClientDLL_Init, 32 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Ipush && inst.operand[ 0 ].type == UD_OP_IMM )
				{
					if ( !bFoundEngfuncs )
					{
						Globals::cl_enginefuncs = reinterpret_cast<cl_enginefunc_t *>( inst.operand[ 0 ].lval.udword );
						bFoundEngfuncs = true;
					}
					else
					{
						Globals::playermove = reinterpret_cast<CBasePlayerMove *>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::cl_enginefuncs, "cl_enginefuncs" );
			GAMEDATA_CHECK_SYMBOL( Globals::playermove, "g_clmove" );

			// Find 'client_static_t cls'
			MemoryUtils()->InitDisasm( &inst, Globals::cl_enginefuncs->pNetAPI->Status, 32, 36 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icmp && inst.operand[ 0 ].type == UD_OP_MEM &&
					 inst.operand[ 0 ].size == 32 && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					Globals::cls = reinterpret_cast<client_static_t *>( inst.operand[ 0 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::cls, "cls" );

			// Find 'usermsg_t gClientUserMsgs'
			auto fClientUserMessages = std::async( [] {
				ud_t inst;
				int iDisassembledBytes = 0;
				uint8_t *pHookUserMsg = (uint8_t *)Globals::cl_enginefuncs->pfnHookUserMsg;

				MemoryUtils()->InitDisasm( &inst, pHookUserMsg, 32, 24 );
				while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
				{
					if ( inst.mnemonic == UD_Icall )
					{
						pHookUserMsg = (uint8_t *)MemoryUtils()->CalcAbsoluteAddress( pHookUserMsg );

						MemoryUtils()->InitDisasm( &inst, pHookUserMsg, 32, 48 );
						while ( MemoryUtils()->Disassemble( &inst ) )
						{
							if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
								 inst.operand[ 0 ].base == UD_R_ESI && inst.operand[ 1 ].type == UD_OP_MEM )
							{
								Globals::g_pClientUserMsgs = reinterpret_cast<usermsg_t **>( inst.operand[ 1 ].lval.udword );
								break;
							}

						}

						break;
					}

					pHookUserMsg += iDisassembledBytes;
				}

				bool bOK = true;
				GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL_STATUS( Globals::g_pClientUserMsgs, "gClientUserMsgs" );

				if ( Globals::g_pClientUserMsgs != NULL )
				{
					bool bFoundAnyCall = false;
					const usermsg_t *pUserMsgHideHUD = Globals::gameutils->FindUserMessage( "HideHUD" );
					void *pMsgFunc = pUserMsgHideHUD->function;

					for ( int i = 0; i < 2; i++ )
					{
						int iDisassembledBytes = 0;
						uint8_t *p = (uint8_t *)pUserMsgHideHUD->function;
						MemoryUtils()->InitDisasm( &inst, pMsgFunc, 32, 32 );

						while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
						{
							if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM
								 && inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
							{
								Globals::gpHideHUD = reinterpret_cast<int *>( inst.operand[ 0 ].lval.udword );
								break;
							}
							else if ( inst.mnemonic == UD_Icall && !bFoundAnyCall )
							{
								bFoundAnyCall = true;
								pMsgFunc = MemoryUtils()->CalcAbsoluteAddress( p );
							}

							p += iDisassembledBytes;
						}

						if ( Globals::gpHideHUD != NULL || !bFoundAnyCall )
							break;
					}
				}

				GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::gpHideHUD, "m_iHideHUD" );
				return true;
			} );

			// Find 'event_t *g_pEventHooks'
			auto fEventHooks = std::async( [] {
				ud_t inst;
				void *pHookEvent = Globals::cl_enginefuncs->pfnHookEvent;
				if ( *(uint8_t *)pHookEvent == 0xE9 ) // JMP opcode
					pHookEvent = MemoryUtils()->CalcAbsoluteAddress( pHookEvent );

				MemoryUtils()->InitDisasm( &inst, pHookEvent, 32, 128 );
				while ( MemoryUtils()->Disassemble( &inst ) )
				{
					if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
						 ( inst.operand[ 0 ].base == UD_R_ESI || inst.operand[ 0 ].base == UD_R_EAX ) // 5.11 -> EAX
						 && inst.operand[ 1 ].type == UD_OP_MEM )
					{
						Globals::g_pEventHooks = reinterpret_cast<event_hook_t *>( inst.operand[ 1 ].lval.udword );
						break;
					}
				}

				GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::g_pEventHooks, "g_pEventHooks" );
				return true;
			} );

			// Find 'int cmd_argc' & 'char *cmd_argv[]'
			auto fCommandArguments = std::async( [] {
				ud_t inst;

				void *pCmd_Argc = Globals::cl_enginefuncs->Cmd_Argc;
				if ( *( (unsigned char *)Globals::cl_enginefuncs->Cmd_Argc ) == 0xE9 ) // JMP opcode
					pCmd_Argc = MemoryUtils()->CalcAbsoluteAddress( Globals::cl_enginefuncs->Cmd_Argc );

				MemoryUtils()->InitDisasm( &inst, pCmd_Argc, 32, 16 );
				while ( MemoryUtils()->Disassemble( &inst ) )
				{
					if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
						 inst.operand[ 0 ].base == UD_R_EAX && inst.operand[ 1 ].type == UD_OP_MEM )
					{
						Globals::cmd_argc = reinterpret_cast<int *>( inst.operand[ 1 ].lval.udword );
						break;
					}
				}

				void *pCmd_Argv = Globals::cl_enginefuncs->Cmd_Argv;
				if ( *( (unsigned char *)Globals::cl_enginefuncs->Cmd_Argv ) == 0xE9 ) // JMP opcode
					pCmd_Argv = MemoryUtils()->CalcAbsoluteAddress( Globals::cl_enginefuncs->Cmd_Argv );

				MemoryUtils()->InitDisasm( &inst, pCmd_Argv, 32, 96 );
				while ( MemoryUtils()->Disassemble( &inst ) )
				{
					if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_EAX &&
						 inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].index == UD_R_EAX &&
						 inst.operand[ 1 ].scale == 4 && inst.operand[ 1 ].offset == 32 )
					{
						Globals::cmd_argv = reinterpret_cast<const char **>( inst.operand[ 1 ].lval.udword );
						break;
					}
				}

				return true;
			} );

			fClientUserMessages.get();
			fEventHooks.get();
			fCommandArguments.get();

			GAMEDATA_CHECK_SYMBOL( Globals::cmd_argc, "cmd_argc" );
			GAMEDATA_CHECK_SYMBOL( Globals::cmd_argv, "cmd_argv" );

			return true;
		} );

		// Find 'cldll_func_t *cl_funcs'
		auto fClientFuncs = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fLoadInsecureClient );

			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::LoadInsecureClient, fLoadInsecureClient );

			void *pLoadInsecureClient = MemoryUtils()->GetPatternFutureValue( fLoadInsecureClient, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( pLoadInsecureClient,
											"LoadInsecureClient",
											Patterns::Engine::LoadInsecureClient,
											patternIndex );

			ud_t inst;
			bool bFoundKeyInstruction = false;
			MemoryUtils()->InitDisasm( &inst, pLoadInsecureClient, 32 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM &&
					 inst.operand[ 1 ].type == UD_OP_IMM && inst.operand[ 1 ].lval.ubyte == 1 )
				{
					bFoundKeyInstruction = true;
				}
				else if ( bFoundKeyInstruction )
				{
					if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM &&
						 ( inst.operand[ 1 ].type == UD_OP_IMM || /* 5.11 -> */ inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX ) )
					{
						Globals::cl_funcs = reinterpret_cast<cldll_func_t *>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::cl_funcs, "cl_funcs" );
			return true;
		} );

		// Find 'engine_studio_api_t *enginestudio'
		auto fEngineStudio = std::async( [] {
			void *pV_EngineStudio_Init = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::V_EngineStudio_Init );
			GAMEDATA_CHECK_SYMBOL_PATTERN( pV_EngineStudio_Init, "V_EngineStudio_Init" );

			ud_t inst;
			bool bFoundFirstPush = false;
			MemoryUtils()->InitDisasm( &inst, pV_EngineStudio_Init, 32 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Ipush && inst.operand[ 0 ].type == UD_OP_IMM )
				{
					if ( !bFoundFirstPush )
					{
						Globals::enginestudio = reinterpret_cast<engine_studio_api_t *>( inst.operand[ 0 ].lval.udword );
						bFoundFirstPush = true;
					}
					else
					{
						Globals::studioapi = *reinterpret_cast<r_studio_interface_t **>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::enginestudio, "engine_studio_api" );
			GAMEDATA_CHECK_SYMBOL( Globals::studioapi, "studioapi" );
			return true;
		} );

		// Find 'IVideoMode *videomode'
		auto fVideoMode = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fV_VideoMode_Create );

			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::V_VideoMode_Create, fV_VideoMode_Create );

			void *pV_VideoMode_Create = MemoryUtils()->GetPatternFutureValue( fV_VideoMode_Create, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( pV_VideoMode_Create,
											"V_VideoMode_Create",
											Patterns::Engine::V_VideoMode_Create,
											patternIndex );

			ud_t inst;
			MemoryUtils()->InitDisasm( &inst, pV_VideoMode_Create, 32, 64 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM &&
					 inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
				{
					Globals::videomode_ptr = reinterpret_cast<IVideoMode **>( inst.operand[ 0 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::videomode_ptr, "videomode_ptr" );
			return true;
		} );

		// Find 'CStudioModelRenderer g_StudioRenderer'
		auto fStudioRenderer = std::async( [] {
			void *pHUD_GetStudioModelInterface = GetProcAddress( (HMODULE)Modules::Client, "HUD_GetStudioModelInterface" );
			GAMEDATA_CHECK_SYMBOL( pHUD_GetStudioModelInterface, "HUD_GetStudioModelInterface" );

			ud_t inst;
			bool bFoundFirstECX = false;
			MemoryUtils()->InitDisasm( &inst, pHUD_GetStudioModelInterface, 32, 128 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_ECX && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					if ( !bFoundFirstECX )
					{
						bFoundFirstECX = true;
						continue;
					}

					Globals::studiorenderer = reinterpret_cast<CStudioModelRenderer *>( inst.operand[ 1 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::studiorenderer, "g_StudioRenderer" );
			return true;
		} );
		
		// Find 'double host_frametime' & 'double realtime'
		auto fFrametime = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fHost_FilterTime );

			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Host_FilterTime, fHost_FilterTime );

			Pointers::Engine::Host_FilterTime = MemoryUtils()->GetPatternFutureValue( fHost_FilterTime, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( Pointers::Engine::Host_FilterTime,
											"Host_FilterTime",
											Patterns::Engine::Host_FilterTime,
											patternIndex );

			ud_t inst;
			bool bFoundFrametime = false;
			MemoryUtils()->InitDisasm( &inst, Pointers::Engine::Host_FilterTime, 32, 128 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Ifst && inst.operand[ 0 ].type == UD_OP_MEM && !bFoundFrametime )
				{
					Globals::host_frametime = reinterpret_cast<double *>( inst.operand[ 0 ].lval.udword );
					bFoundFrametime = true;
				}
				else if ( inst.mnemonic == UD_Ifadd && inst.operand[ 0 ].type == UD_OP_MEM && bFoundFrametime )
				{
					Globals::realtime = reinterpret_cast<double *>( inst.operand[ 0 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::host_frametime, "host_frametime" );
			GAMEDATA_CHECK_SYMBOL( Globals::realtime, "realtime" );
			return true;
		} );

		// Find 'cvar_t **cvar_vars' & 'cmd_function_t **cmd_functions'
		auto fCvar = std::async( [] {
			bool bOK = true;

			int patternIndex, patternIndex2;
			DEFINE_PATTERNS_FUTURE( fCvar_RemoveClientDLLCvars );
			DEFINE_PATTERNS_FUTURE( fCvar_DirectSet );

			auto fZ_Free = MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Z_Free );
			auto fMem_Free = MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Mem_Free );
			auto fCmd_AddCommand = MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Cmd_AddCommand );
			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Cvar_RemoveClientDLLCvars, fCvar_RemoveClientDLLCvars );
			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Cvar_DirectSet, fCvar_DirectSet );

			Pointers::Engine::Z_Free = fZ_Free.get();
			Pointers::Engine::Mem_Free = fMem_Free.get();
			void *pCmd_AddCommand = fCmd_AddCommand.get();
			void *pCvar_RemoveClientDLLCvars = MemoryUtils()->GetPatternFutureValue( fCvar_RemoveClientDLLCvars, &patternIndex );
			Pointers::Engine::Cvar_DirectSet = MemoryUtils()->GetPatternFutureValue( fCvar_DirectSet, &patternIndex2 );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Engine::Cvar_DirectSet,
												   "Cvar_DirectSet",
												   Patterns::Engine::Cvar_DirectSet,
												   patternIndex2 );

			GAMEDATA_CHECK_SYMBOL_PATTERN( pCmd_AddCommand, "Cmd_AddCommand" );
			GAMEDATA_CHECK_SYMBOL_PATTERNS( pCvar_RemoveClientDLLCvars,
											"Cvar_RemoveClientDLLCvars",
											Patterns::Engine::Cvar_RemoveClientDLLCvars,
											patternIndex );

			ud_t inst;
			MemoryUtils()->InitDisasm( &inst, pCvar_RemoveClientDLLCvars, 32, 32 );
			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_ESI && inst.operand[ 1 ].type == UD_OP_MEM )
				{
					Globals::cvar_vars = reinterpret_cast<cvar_t **>( inst.operand[ 1 ].lval.udword );
					break;
				}
			}
			
			MemoryUtils()->InitDisasm( &inst, pCmd_AddCommand, 32, 48 );
			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_ESI && inst.operand[ 1 ].type == UD_OP_MEM )
				{
					Globals::cmd_functions = reinterpret_cast<cmd_function_t **>( inst.operand[ 1 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL_STATUS( Globals::cvar_vars, "cvar_vars" );
			GAMEDATA_CHECK_SYMBOL_STATUS( Globals::cmd_functions, "cmd_functions" );
			GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( Pointers::Engine::Z_Free, "Z_Free" );
			GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( Pointers::Engine::Mem_Free, "Mem_Free" );

			return bOK;
		} );

		// Find 'extra_player_info_t g_PlayerExtraInfo'
		auto fExtraPlayerInfo = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fGetClientColor );

			MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::GetClientColor, fGetClientColor );

			Pointers::Client::GetClientColor = MemoryUtils()->GetPatternFutureValue( fGetClientColor, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Client::GetClientColor,
														 "GetClientColor",
														 Patterns::Client::GetClientColor,
														 patternIndex );

			ud_t inst;
			MemoryUtils()->InitDisasm( &inst, Pointers::Client::GetClientColor, 32, 48 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imovsx && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_EAX && inst.operand[ 1 ].type == UD_OP_MEM )
				{
					// In GetClientColor, the actual access to g_PlayerExtraInfo is shifted to offset 'extra_player_info_t::teamnumber'
					// So, just subtract that offset - 0xA
					constexpr auto offteamnumber = offsetof( extra_player_info_t, teamnumber );
					Globals::extraplayerinfo = reinterpret_cast<CBaseExtraPlayerInfo *>( inst.operand[ 1 ].lval.udword - offteamnumber );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::extraplayerinfo, "g_PlayerExtraInfo" );
			return true;
		} );

		// Find 'WeaponsResource gWR'
		auto fWeaponsResource = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fCHudAmmo__VidInit );

			MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::CHudAmmo__VidInit, fCHudAmmo__VidInit );

			void *pCHudAmmo__VidInit = MemoryUtils()->GetPatternFutureValue( fCHudAmmo__VidInit, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( pCHudAmmo__VidInit,
														 "CHudAmmo::VidInit",
														 Patterns::Client::CHudAmmo__VidInit,
														 patternIndex );

			ud_t inst;
			MemoryUtils()->InitDisasm( &inst, pCHudAmmo__VidInit, 32, 24 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icmp && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_ESI && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					Globals::weaponsresource = reinterpret_cast<CBaseWeaponsResource *>( inst.operand[ 1 ].lval.udword );
					break;
				}
			}

			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::weaponsresource, "gWR" );
			return true;
		} );

		// Find 'g_NetworkMessages'
		auto fNetworkMessages = std::async( [] {
			void *pszSvcBad = MemoryUtils()->FindString( Modules::Engine, "svc_bad" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pszSvcBad, "svc_bad" );
			
			Globals::g_NetworkMessages = (netmsg_t *)MemoryUtils()->FindAddress( Modules::Engine, pszSvcBad );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::g_NetworkMessages, "g_NetworkMessages" );

			return true;
		} );
		
		// Find 'clc_buffer'
		auto fClcBuffer = std::async( [] {
			void *pszSendRes = MemoryUtils()->FindString( Modules::Engine, "sendres" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pszSendRes, "sendres" );
			
			uint8_t *pclc_buffer = (uint8_t *)MemoryUtils()->FindAddress( Modules::Engine, pszSendRes );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pclc_buffer, "PUSH [sendres]" );
			
			pclc_buffer += sizeof( void * );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( *pclc_buffer == 0x68, "PUSH [clc_buffer]" );

			Globals::clc_buffer = *(sizebuf_t **)( pclc_buffer + 1 );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::clc_buffer, "clc_buffer" );

			return true;
		} );
		
		// Find 'sizebuf_t net_message', 'int msg_readcount' & 'qboolean msg_badread'
		auto fNetworkMessageBuffer = std::async( [] {
			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fMSG_ReadByte );

			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::MSG_ReadByte, fMSG_ReadByte );

			void *pMSG_ReadByte = MemoryUtils()->GetPatternFutureValue( fMSG_ReadByte, &patternIndex );
			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( pMSG_ReadByte,
														 "MSG_ReadByte",
														 Patterns::Engine::MSG_ReadByte,
														 patternIndex );

			ud_t inst;
			MemoryUtils()->InitDisasm( &inst, pMSG_ReadByte, 32, 32 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov )
				{
					if ( inst.operand[ 0 ].type == UD_OP_REG &&
						 ( inst.operand[ 0 ].base == UD_R_ECX || inst.operand[ 0 ].base == UD_R_EAX ) && // 5.11 -> EAX
						 inst.operand[ 1 ].type == UD_OP_MEM )
					{
						Globals::msg_readcount = reinterpret_cast<int *>( inst.operand[ 1 ].lval.udword );
					}
					else if ( inst.operand[ 0 ].type == UD_OP_MEM &&
							  inst.operand[ 1 ].type == UD_OP_IMM &&
							  inst.operand[ 1 ].lval.udword == 1 )
					{
						Globals::msg_badread = reinterpret_cast<int *>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}
				else if ( inst.mnemonic == UD_Icmp && inst.operand[ 0 ].type == UD_OP_REG &&
						  ( inst.operand[ 0 ].base == UD_R_EDX || inst.operand[ 0 ].base == UD_R_ECX ) && // 5.11 -> ECX
						  inst.operand[ 1 ].type == UD_OP_MEM )
				{
					// Subtract offset 'sizebuf_t::cursize'
					constexpr auto offcursize = offsetof( sizebuf_t, cursize );
					Globals::net_message = reinterpret_cast<sizebuf_t *>( inst.operand[ 1 ].lval.udword - offcursize );
				}
			}

			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::msg_readcount, "msg_readcount" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::msg_badread, "msg_badread" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::net_message, "net_message" );

			return true;
		} );

		// Find SCR_BeginLoadingPlaque & SCR_EndLoadingPlaque
		auto fSCRLoadingPlaque = std::async( [] {
			bool bOK = true;

			int patternIndex;
			DEFINE_PATTERNS_FUTURE( fSCR_EndLoadingPlaque );

			auto fSCR_BeginLoadingPlaque = MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::SCR_BeginLoadingPlaque );
			MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::SCR_EndLoadingPlaque, fSCR_EndLoadingPlaque );

			Pointers::Engine::SCR_BeginLoadingPlaque = fSCR_BeginLoadingPlaque.get();
			Pointers::Engine::SCR_EndLoadingPlaque = MemoryUtils()->GetPatternFutureValue( fSCR_EndLoadingPlaque, &patternIndex );

			GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( Pointers::Engine::SCR_BeginLoadingPlaque, "SCR_BeginLoadingPlaque" );
			GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Engine::SCR_EndLoadingPlaque,
												   "SCR_EndLoadingPlaque",
												   Patterns::Engine::SCR_EndLoadingPlaque,
												   patternIndex );

			return bOK;
		} );

		// Find server patterns
		auto fServerGameDataPatterns = std::async( []
		{
			bool bOK = true;

			int ndxPlayerSpawns, ndxFixPlayerStuck, ndxCBaseEntity__FireBullets;
			int ndxUTIL_GetCircularGaussianSpread, ndxFireTargets, ndxCopyPEntityVars;

			DEFINE_PATTERNS_FUTURE( fPlayerSpawns );
			DEFINE_PATTERNS_FUTURE( fFixPlayerStuck );
			DEFINE_PATTERNS_FUTURE( fCBaseEntity__FireBullets );
			DEFINE_PATTERNS_FUTURE( fUTIL_GetCircularGaussianSpread );
			DEFINE_PATTERNS_FUTURE( fFireTargets );
			DEFINE_PATTERNS_FUTURE( fCopyPEntityVars );

			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::PlayerSpawns, fPlayerSpawns );
			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::FixPlayerStuck, fFixPlayerStuck );
			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::CBaseEntity__FireBullets, fCBaseEntity__FireBullets );
			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::UTIL_GetCircularGaussianSpread, fUTIL_GetCircularGaussianSpread );
			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::FireTargets, fFireTargets );
			MemoryUtils()->FindPatternAsync( Modules::Server, Patterns::Server::CopyPEntityVars, fCopyPEntityVars );

			Pointers::Server::PlayerSpawns = MemoryUtils()->GetPatternFutureValue( fPlayerSpawns, &ndxPlayerSpawns );
			Pointers::Server::FixPlayerStuck = MemoryUtils()->GetPatternFutureValue( fFixPlayerStuck, &ndxFixPlayerStuck );
			Pointers::Server::CBaseEntity__FireBullets = MemoryUtils()->GetPatternFutureValue( fCBaseEntity__FireBullets, &ndxCBaseEntity__FireBullets );
			Pointers::Server::UTIL_GetCircularGaussianSpread = MemoryUtils()->GetPatternFutureValue( fUTIL_GetCircularGaussianSpread, &ndxUTIL_GetCircularGaussianSpread );
			Pointers::Server::FireTargets = MemoryUtils()->GetPatternFutureValue( fFireTargets, &ndxFireTargets );
			Pointers::Server::CopyPEntityVars = MemoryUtils()->GetPatternFutureValue( fCopyPEntityVars, &ndxCopyPEntityVars );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::PlayerSpawns,
														 "PlayerSpawns",
														 Patterns::Server::PlayerSpawns,
														 ndxPlayerSpawns );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::FixPlayerStuck,
														 "FixPlayerStuck",
														 Patterns::Server::FixPlayerStuck,
														 ndxFixPlayerStuck );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::CBaseEntity__FireBullets,
														 "CBaseEntity::FireBullets",
														 Patterns::Server::CBaseEntity__FireBullets,
														 ndxCBaseEntity__FireBullets );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::UTIL_GetCircularGaussianSpread,
														 "UTIL_GetCircularGaussianSpread",
														 Patterns::Server::UTIL_GetCircularGaussianSpread,
														 ndxUTIL_GetCircularGaussianSpread );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::FireTargets,
														 "FireTargets",
														 Patterns::Server::FireTargets,
														 ndxFireTargets );

			GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Server::CopyPEntityVars,
														 "CopyPEntityVars",
														 Patterns::Server::CopyPEntityVars,
														 ndxCopyPEntityVars );

			if ( Pointers::Server::CopyPEntityVars != NULL )
				Pointers::Server::CopyPEntityVars = MemoryUtils()->CalcAbsoluteAddress( Pointers::Server::CopyPEntityVars );

			return bOK;
		} );

		// Further loading cannot proceed without important symbols
		if ( !fGameVersion.get() )
			bOK = false;
		if ( !fConsolePrint.get() )
			bOK = false;
		if ( !fEngineFuncsAndPlayerMove.get() )
			bOK = false;
		if ( !fClientFuncs.get() )
			bOK = false;
		if ( !fEngineStudio.get() )
			bOK = false;
		if ( !fVideoMode.get() )
			bOK = false;
		if ( !fStudioRenderer.get() )
			bOK = false;
		if ( !fFrametime.get() )
			bOK = false;
		if ( !fCvar.get() )
			bOK = false;
		if ( !fSCRLoadingPlaque.get() )
			bOK = false;

		// Find server symbols
		auto fServerGameData = std::async( [] {
			ud_t inst;
			bool bOK = true;

			int iNewDllFunctionsVersion = NEW_DLL_FUNCTIONS_VERSION;

			void *GiveFnptrsToDll = GetProcAddress( (HMODULE)Modules::Server, "GiveFnptrsToDll" );
			APIFUNCTION GetEntityAPI = (APIFUNCTION)GetProcAddress( (HMODULE)Modules::Server, "GetEntityAPI" );
			NEW_DLL_FUNCTIONS_FN GetNewDLLFunctions = (NEW_DLL_FUNCTIONS_FN)GetProcAddress( (HMODULE)Modules::Server, "GetNewDLLFunctions" );

			GAMEDATA_CHECK_SYMBOL_STATUS( GiveFnptrsToDll, "GiveFnptrsToDll" );
			GAMEDATA_CHECK_SYMBOL_STATUS( GetEntityAPI, "GetEntityAPI" );
			GAMEDATA_CHECK_SYMBOL_STATUS( GetNewDLLFunctions, "GetNewDLLFunctions" );

			if ( !bOK )
				return false;

			if ( !GetEntityAPI( &Globals::s_dllFuncs, INTERFACE_VERSION ) )
			{
				Warning( "<SvenInt::GameData> Failed to import \"%s\"\n", "dllFuncs" );
				return false;
			}
			else
			{
				Globals::dllFuncs = &Globals::s_dllFuncs;
				MsgWrapper( "<SvenInt::GameData> Imported \"%s\" (0x%X) from the game DLL\n", "dllFuncs", Globals::dllFuncs );
			}

			if ( !GetNewDLLFunctions( &Globals::s_dllFuncs2, &iNewDllFunctionsVersion ) )
			{
				Warning( "<SvenInt::GameData> Failed to import \"%s\"\n", "dllFuncs2" );
				return false;
			}
			else
			{
				Globals::dllFuncs2 = &Globals::s_dllFuncs2;
				MsgWrapper( "<SvenInt::GameData> Imported \"%s\" (0x%X) from the game DLL\n", "dllFuncs2", Globals::dllFuncs2 );
			}

			globalvars_t **pgpGlobals = NULL;

			MemoryUtils()->InitDisasm( &inst, GiveFnptrsToDll, 32, 32 );
			while ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_EDI && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					Globals::sv_enginefuncs = reinterpret_cast<enginefuncs_t *>( inst.operand[ 1 ].lval.udword );
				}
				else if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM &&
						  inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
				{
					pgpGlobals = reinterpret_cast<globalvars_t **>( inst.operand[ 0 ].lval.udword );
					break;
				}
			}

			if ( pgpGlobals != NULL )
				Globals::gpGlobals = *pgpGlobals;
			
			GAMEDATA_CHECK_SYMBOL( Globals::sv_enginefuncs, "gpEngFuncs" );
			GAMEDATA_CHECK_SYMBOL( Globals::gpGlobals, "gpGlobals" );

			if ( Globals::cl_enginefuncs == NULL )
				return false;
			
			cmd_function_t *disconnect = Globals::cvar->FindCmd( "disconnect" );
			GAMEDATA_CHECK_SYMBOL( disconnect, "disconnect" );

			bool bFoundFirstCall = false;
			int iDisassembledBytes = 0;
			uint8_t *p = (uint8_t *)disconnect->function;

			MemoryUtils()->InitDisasm( &inst, disconnect->function, 32, 16 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icall )
				{
					if ( !bFoundFirstCall )
					{
						bFoundFirstCall = true;
					}
					else
					{
						Pointers::Engine::Host_IsServerActive = MemoryUtils()->CalcAbsoluteAddress( p );
						break;
					}
				}

				p += iDisassembledBytes;
			}

			GameData::Pointers::Server::vmt_CBasePlayer = MemoryUtils()->FindVTable( Modules::Server, "CBasePlayer" );

			GAMEDATA_CHECK_SYMBOL( Pointers::Engine::Host_IsServerActive, "Host_IsServerActive" );
			GAMEDATA_CHECK_SYMBOL( GameData::Pointers::Server::vmt_CBasePlayer, "CBasePlayer (VMT)" );

			return bOK;
		} );
		
		// Find survival mode instance & toggle command
		auto fServerSurvivalMode = std::async( [] {
			bool bOK = true;

			void *ptoggle_survival_mode = MemoryUtils()->FindString( Modules::Server, "toggle_survival_mode" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( ptoggle_survival_mode, "toggle_survival_mode" );

			void *ptoggle_survival_mode_xref = MemoryUtils()->FindAddress( Modules::Server, ptoggle_survival_mode );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( ptoggle_survival_mode_xref, "toggle_survival_mode (XREF)" );

			uint8_t *push = (uint8_t *)ptoggle_survival_mode_xref - 0x1;
			if ( *push != 0x68 ) // PUSH op-code
			{
				Warning2( "[SvenInt::GameData] Op-code PUSH [toggle_survival_mode] not found\n" );
				return false;
			}

			// Go to PUSH [toggle_survival_mode_Callback]
			push -= 5;
			if ( *push != 0x68 )
			{
				Warning2( "[SvenInt::GameData] Op-code PUSH [toggle_survival_mode_Callback] not found\n" );
				return false;
			}

			uint8_t *ptoggle_survival_mode_Callback = (uint8_t *)*(uint32_t **)( push + 1 );
			Pointers::Server::toggle_survival_mode_Callback = ptoggle_survival_mode_Callback;
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Pointers::Server::toggle_survival_mode_Callback, "toggle_survival_mode_Callback" );

			if ( *ptoggle_survival_mode_Callback == 0xE8 ) // in 5.11 function GetSurvivalModeInstance is inlined
				ptoggle_survival_mode_Callback = (uint8_t *)MemoryUtils()->CalcAbsoluteAddress( ptoggle_survival_mode_Callback );

			ud_t inst;
			bool bFoundFirstMov = false;
			int iDisassembledBytes = 0;
			uint8_t *p = ptoggle_survival_mode_Callback;
			MemoryUtils()->InitDisasm( &inst, ptoggle_survival_mode_Callback, 32, 96 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_MEM && inst.operand[ 0 ].base == 0 &&
					 inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
				{
					if ( !bFoundFirstMov )
					{
						bFoundFirstMov = true;
					}
					else
					{
						Pointers::Server::pSurvivalModeInstance = reinterpret_cast<void *>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}

				p += iDisassembledBytes;
			}

			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Pointers::Server::pSurvivalModeInstance, "pSurvivalModeInstance" );
			return bOK;
		} );
		
		// Find GL_Bind
		auto fGL_Bind = std::async( [] {
			bool bOK = true;

			if ( Globals::cl_enginefuncs == NULL )
				return false;

			ud_t inst;
			int iDisassembledBytes = 0, iCallCount = 0;
			uint8_t *p = (uint8_t *)Globals::cl_enginefuncs->pTriAPI->SpriteTexture;

			MemoryUtils()->InitDisasm( &inst, Globals::cl_enginefuncs->pTriAPI->SpriteTexture, 32, 48 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icall )
				{
					if ( ++iCallCount != 3 )
					{
						p += iDisassembledBytes;
						continue;
					}

					Pointers::Engine::GL_Bind = MemoryUtils()->CalcAbsoluteAddress( p );
					break;
				}

				p += iDisassembledBytes;
			}

			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Pointers::Engine::GL_Bind, "GL_Bind" );
			return bOK;
		} );
		
		// Guess server vtidxs based on the game version
		auto fGuessServerVtidx = std::async( [] {
			switch ( Globals::gameversion )
			{
			case 526:
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 9;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 48;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 90;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 91;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 212;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 220;

				break;
			}

			case 525:
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 9;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 47;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 88;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 89;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 210;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 218;

				break;
			}

			case 511:
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 8;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 43;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 82;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 83;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 192;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 198;

				break;
			}

			default:
				Warning2( "[SvenInt::GameData] Unable to guess indexes of virtual methods for the game DLL (game version: %d)\n", Globals::gameversion );
				break;
			}
		} );
		
		Pointers::Engine::Netchan_Transmit = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::Netchan_Transmit );
		GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( Pointers::Engine::Netchan_Transmit, "Netchan_Transmit" );
		
		int ndxMSG_WriteUsercmd;
		DEFINE_PATTERNS_FUTURE( fMSG_WriteUsercmd );
		MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::MSG_WriteUsercmd, fMSG_WriteUsercmd );
		Pointers::Engine::MSG_WriteUsercmd = MemoryUtils()->GetPatternFutureValue( fMSG_WriteUsercmd, &ndxMSG_WriteUsercmd );
		GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Engine::MSG_WriteUsercmd,
											   "MSG_WriteUsercmd",
											   Patterns::Engine::MSG_WriteUsercmd,
											   ndxMSG_WriteUsercmd );

		int ndxIN_Move;
		DEFINE_PATTERNS_FUTURE( fIN_Move );
		MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::IN_Move, fIN_Move );
		Pointers::Client::IN_Move = MemoryUtils()->GetPatternFutureValue( fIN_Move, &ndxIN_Move );
		GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Client::IN_Move,
											   "IN_Move",
											   Patterns::Client::IN_Move,
											   ndxIN_Move );

		int ndxKey_Event;
		DEFINE_PATTERNS_FUTURE( fKey_Event );
		MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Key_Event, fKey_Event );
		Pointers::Engine::Key_Event = MemoryUtils()->GetPatternFutureValue( fKey_Event, &ndxKey_Event );
		GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Engine::Key_Event,
											   "Key_Event",
											   Patterns::Engine::Key_Event,
											   ndxKey_Event );

		int ndxSCR_UpdateScreen;
		DEFINE_PATTERNS_FUTURE( fSCR_UpdateScreen );
		MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::SCR_UpdateScreen, fSCR_UpdateScreen );
		Pointers::Engine::SCR_UpdateScreen = MemoryUtils()->GetPatternFutureValue( fSCR_UpdateScreen, &ndxSCR_UpdateScreen );
		GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( Pointers::Engine::SCR_UpdateScreen,
											   "SCR_UpdateScreen",
											   Patterns::Engine::SCR_UpdateScreen,
											   ndxSCR_UpdateScreen );

		fExtraPlayerInfo.get();
		fWeaponsResource.get();
		fNetworkMessages.get();
		fClcBuffer.get();
		fNetworkMessageBuffer.get();
		fGL_Bind.get();
		fServerGameDataPatterns.get();
		fServerSurvivalMode.get();
		fGuessServerVtidx.get();

		Globals::engineclient = GetBaseEngineClient( Globals::engineclient, Globals::gameversion );
		Globals::extraplayerinfo = GetBaseExtraPlayerInfo( Globals::extraplayerinfo, Globals::gameversion );
		Globals::playermove = GetBasePlayerMove( Globals::playermove, Globals::gameversion );
		Globals::weaponsresource = GetBaseWeaponsResource( Globals::weaponsresource, Globals::gameversion );

		Globals::localplayer = LocalPlayer();
		Globals::clientweapon = ClientWeapon();
		Globals::inventory = Inventory();

		Globals::gamehooks = GameHooks();
		Globals::gameutils = GameUtils();
		Globals::hookevents = HookEvents();

		Globals::refparams = &Globals::s_refparams;
		Globals::refparams_movevars = &Globals::s_refparams_movevars;

		memset( Globals::refparams, 0, sizeof( ref_params_t ) );
		memset( Globals::refparams_movevars, 0, sizeof( movevars_t ) );

		if ( !fServerGameData.get() )
			bOK = false;

		if ( Globals::cl_enginefuncs != NULL )
		{
			Cvars::host_framerate = Globals::cl_enginefuncs->pfnGetCvarPointer( "host_framerate" );
			Cvars::fps_max = Globals::cl_enginefuncs->pfnGetCvarPointer( "fps_max" );
			Cvars::hud_draw = Globals::cl_enginefuncs->pfnGetCvarPointer( "hud_draw" );
		}

		return bOK;
	}

	void *GetInterfaceIteratively( CreateInterfaceFn interfaceFactory, const char *pszInterfaceVersion )
	{
		void *pInterface = NULL;
		char *szInterfaceVersion = const_cast<char *>( pszInterfaceVersion );

		const size_t length = strlen( szInterfaceVersion ), last_idx = length - 1, post_last_idx = length - 2;

		do
		{
			if ( pInterface = interfaceFactory( szInterfaceVersion, NULL ) )
				return pInterface;

			if ( szInterfaceVersion[ last_idx ] == '0' )
			{
				szInterfaceVersion[ last_idx ] = '9';

				if ( szInterfaceVersion[ post_last_idx ] != '0' )
					--szInterfaceVersion[ post_last_idx ];
			}
			else
			{
				--szInterfaceVersion[ last_idx ];
			}

		} while ( szInterfaceVersion[ post_last_idx ] != '0' && szInterfaceVersion[ last_idx ] != '0' );

		return pInterface;
	}
}
// SvenInt (c) Sw1ft
// gamedata.cpp

#include "stdafx.h"
#include "game/dbg.h"
#include "game/keyvalues_parser.h"

using Globals::gamedata;

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

#define GAMEDATA_CHECK_SYMBOL_PATTERN_STATUS( symbolPointer, symbolName ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" at address 0x%X\n", symbolName, symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); return false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

#define GAMEDATA_CHECK_SYMBOL_PATTERNS_STATUS( symbolPointer, symbolName, patternVar, patternIndex ) \
	if ( symbolPointer == NULL ) { Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", symbolName ); bOK = false; } \
	else { MsgWrapper( "<SvenInt::GameData> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", symbolName, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), symbolPointer ); }

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
// Dump file offsets
//-----------------------------------------------------------------------------

#if DUMP_FILE_OFFSET
static FILE *pDumpFileOffset = NULL;
static std::mutex dump_mutex;

void InitPrintDumpFileOffset( void )
{
	pDumpFileOffset = fopen( "sint_file_offsets.txt", "w" );
	if ( pDumpFileOffset != NULL )
		fprintf( pDumpFileOffset, "[RVA Offsets]\n" );
}

void ShutdownPrintDumpFileOffset( void )
{
	if ( pDumpFileOffset != NULL )
		fclose( pDumpFileOffset );
}

void PrintDumpFileOffset( const char *pszSymbolName, uint32_t ptr, module_t hModule )
{
	std::unique_lock<std::mutex> lock( dump_mutex );
	uint32_t allocBase;

#ifdef WIN32
	allocBase = (uint32_t)hModule;
#else
	moduleinfo_t modinfo;
	if ( !MemoryUtils()->RetrieveModuleInfo( hModule, &modinfo ) )
		return;
	allocBase = (uint32_t)modinfo.pBaseOfDll;
#endif

	if ( pDumpFileOffset != NULL )
		fprintf( pDumpFileOffset, "(%.8X) %s = %X\n", allocBase, pszSymbolName, ptr == 0 ? 0 : ptr - allocBase );
}
#endif

//-----------------------------------------------------------------------------
// Various gamedata (patterns, modules, interfaces, function pointers, variables, etc.)
//-----------------------------------------------------------------------------

bool CGameDataFinder::Init( void )
{
	int result_code = 0;
	m_pGamedata = KeyValuesParser::LoadFromFile( SVENINT_FOLDER_NAME "/gamedata.txt", &result_code );

	if ( result_code == KeyValuesParser::PARSE_FAILED )
	{
		Warning( "<SvenInt::GameData> Failed to parse file \"./" SVENINT_FOLDER_NAME "/gamedata.txt\". Reason: %s (line: %d)\n", KeyValuesParser::GetLastErrorMessage(), KeyValuesParser::GetLastErrorLine() );
		return false;
	}

	if ( m_pGamedata == NULL )
	{
		Warning( "<SvenInt::GameData> File \"./" SVENINT_FOLDER_NAME "/gamedata.txt\" is empty\n" );
		return false;
	}

	if ( m_pGamedata->GetName() != "GameData" )
	{
		Warning( "<SvenInt::GameData> Expected \"GameData\" as the main section in the file \"./" SVENINT_FOLDER_NAME "/gamedata.txt\"\n" );

		MemFreeInstance( m_pGamedata );
		m_pGamedata = NULL;
		return false;
	}

	auto pSettings = m_pGamedata->FindKey( "Settings" );
	if ( pSettings != NULL && !pSettings->IsPair() )
	{
		auto pPreferRVA = pSettings->FindKey( "PreferRVA" );
		if ( pPreferRVA != NULL && pPreferRVA->IsPair() )
		{
			m_bPreferRVA = !!atoi( pPreferRVA->GetValue().c_str() );
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown gamedata finder
//-----------------------------------------------------------------------------

void CGameDataFinder::Shutdown( void )
{
	MemFreeInstance( m_pGamedata );
	m_pGamedata = NULL;
}

//-----------------------------------------------------------------------------
// Tries to automatically find a given symbol (i.e. a function), first is an RVA offset,
// if it fails we do switch to FindPattern (Windows) or FindSymbol (Linux)
// RVA offset name and pattern/symbol name are supposed to be the same
//-----------------------------------------------------------------------------

void *CGameDataFinder::AutoFind( module_t hModule, const char *pszModuleName, const char *pszSymbolName, bool bNotify /* = true */ )
{
	void *ptr = NULL;
	if ( PreferRVA() )
		ptr = FindRVA( hModule, pszModuleName, pszSymbolName, bNotify );

	if ( ptr != NULL )
		return ptr;

#ifdef WIN32
	return FindPattern( hModule, pszModuleName, pszSymbolName, bNotify );
#else
	if ( SVEN_VERSION() >= SVEN_VERSION_CHECK( 5, 26, 0 ) ) // removing debug symbols is a dirty move
		return NULL;

	return FindSymbol( hModule, pszModuleName, pszSymbolName, bNotify );
#endif
}

//-----------------------------------------------------------------------------
// FindRVA
//-----------------------------------------------------------------------------

void *CGameDataFinder::FindRVA( module_t hModule, const char *pszModuleName, const char *pszRvaOffsetName, bool bNotify /* = true */ )
{
	static KeyValuesParser::KeyValues *pRVA = NULL;
	if ( pRVA == NULL )
		pRVA = m_pGamedata->FindKey( "RVA" );

	if ( pRVA == NULL || pRVA->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindRVA> Section \"%s\" is missing, unable to look up for offset \"%s\"\n", "RVA", pszRvaOffsetName );
		return NULL;
	}

	auto pModuleSection = pRVA->FindKey( pszModuleName );
	if ( pModuleSection == NULL || pModuleSection->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindRVA> Module section \"%s\" is missing, unable to look up for offset \"%s\"\n", pszModuleName, pszRvaOffsetName );
		return NULL;
	}

	auto pRvaPlatformContainer = pModuleSection->FindKey( pszRvaOffsetName );
	if ( pRvaPlatformContainer == NULL || pRvaPlatformContainer->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindRVA> RVA offset section \"%s\" is missing, unable to look up further\n", pszRvaOffsetName );
		return NULL;
	}

#ifdef WIN32
	auto pRvaPlatform = pRvaPlatformContainer->FindKey( "windows" );
#else
	auto pRvaPlatform = pRvaPlatformContainer->FindKey( "linux" );
#endif
	if ( pRvaPlatform == NULL || pRvaPlatform->IsPair() )
	{
		if ( bNotify )
		#ifdef WIN32
			Warning( "<SvenInt::FindRVA> Platform section \"windows\" is missing, unable to look up for offset \"%s\"\n", pszRvaOffsetName );
		#else
			Warning( "<SvenInt::FindRVA> Platform section \"linux\" is missing, unable to look up for offset \"%s\"\n", pszRvaOffsetName );
		#endif
		return NULL;
	}

	auto pRvaOffset = pRvaPlatform->FindKey( Globals::szGameVersion );
	if ( pRvaOffset == NULL || !pRvaOffset->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindRVA> Named RVA offset \"%s\" not found for the game version \"%s\"\n", pszRvaOffsetName, Globals::szGameVersion );
		return NULL;
	}

	const uint32_t ulRvaOffset = __svenint_hex_to_decimal_fast( (char *)pRvaOffset->GetValue().c_str() );
	//const uint32_t ulRvaOffset = strtoul( pRvaOffset->GetValue().c_str(), NULL, 16 );

	if ( ulRvaOffset == 0 )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindRVA> Named RVA offset \"%s\" not found for the game version \"%s\"\n", pszRvaOffsetName, Globals::szGameVersion );
		return NULL;
	}

	moduleinfo_t modinfo;
	if ( !MemoryUtils()->RetrieveModuleInfo( hModule, &modinfo ) )
		return NULL;

	MsgWrapper( "<SvenInt::FindRVA> Found RVA offset \"%s\" for the game version \"%s\"\n", pszRvaOffsetName, Globals::szGameVersion );
	return (uint8_t *)modinfo.pBaseOfDll + ulRvaOffset;
}

//-----------------------------------------------------------------------------
// FindPattern
//-----------------------------------------------------------------------------

void *CGameDataFinder::FindPattern( module_t hModule, const char *pszModuleName, const char *pszPatternName, bool bNotify /* = true */, uint32_t offset /* = 0 */ )
{
	static KeyValuesParser::KeyValues *pPatterns = NULL;
	if ( pPatterns == NULL )
		pPatterns = m_pGamedata->FindKey( "Patterns" );

	if ( pPatterns == NULL || pPatterns->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindPattern> Section \"%s\" is missing, unable to look up for pattern \"%s\"\n", "Patterns", pszPatternName );
		return NULL;
	}

	auto pModuleSection = pPatterns->FindKey( pszModuleName );
	if ( pModuleSection == NULL || pModuleSection->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindPattern> Module section \"%s\" is missing, unable to look up for pattern \"%s\"\n", pszModuleName, pszPatternName );
		return NULL;
	}

	auto pPatternPlatformContainer = pModuleSection->FindKey( pszPatternName );
	if ( pPatternPlatformContainer == NULL || pPatternPlatformContainer->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindPattern> Pattern section \"%s\" is missing, unable to look up further\n", pszPatternName );
		return NULL;
	}

#ifdef WIN32
	auto pPatternPlatform = pPatternPlatformContainer->FindKey( "windows" );
#else
	auto pPatternPlatform = pPatternPlatformContainer->FindKey( "linux" );
#endif
	if ( pPatternPlatform == NULL || pPatternPlatform->IsPair() )
	{
		if ( bNotify )
		#ifdef WIN32
			Warning( "<SvenInt::FindPattern> Platform section \"windows\" is missing, unable to look up for pattern \"%s\"\n", pszPatternName );
		#else
			Warning( "<SvenInt::FindPattern> Platform section \"linux\" is missing, unable to look up for pattern \"%s\"\n", pszPatternName );
		#endif
		return NULL;
	}

	auto pAny = pPatternPlatform->FindKey( "any" );
	if ( pAny != NULL && pAny->IsPair() )
	{
		uint32_t len = svenint_get_pattern_length( pAny->GetValue().c_str() );
		uint8_t *signature = (uint8_t *)calloc( len, sizeof( uint8_t ) );

		svenint_construct_pattern( signature, pAny->GetValue().c_str(), len );
		void *ptr = MemoryUtils()->FindPattern( hModule, signature, len, offset );

		free( signature );

		if ( bNotify )
		{
			if ( ptr != NULL )
			{
				MsgWrapper( "<SvenInt::FindPattern> Found signature of symbol \"%s\" at address 0x%X\n", pszPatternName, ptr );
			}
			else
			{
				Warning( "<SvenInt::FindPattern> Failed to locate signature of symbol \"%s\"\n", pszPatternName );
			}
		}

		return ptr;
	}

	uint32_t patternsLen = 0;
	for ( size_t i = 0; i < pPatternPlatform->GetList().size(); i++ )
	{
		if ( pPatternPlatform->GetList()[ i ]->IsPair() )
			patternsLen++;
	}

	if ( patternsLen == 0 )
		return NULL;
	
	std::vector<uint8_t *> signatures;
	std::vector<uint32_t> signaturesLen;
	std::vector<const char *> signaturesGameVer;

	for ( size_t i = 0; i < pPatternPlatform->GetList().size(); i++ )
	{
		auto pPatternKv = pPatternPlatform->GetList()[ i ];
		if ( !pPatternKv->IsPair() )
			continue;

		std::string &sPattern = pPatternKv->GetValue();

		uint32_t len = svenint_get_pattern_length( sPattern.c_str() );
		uint8_t *signature = (uint8_t *)calloc( len, sizeof( uint8_t ) );

		if ( signature == NULL )
			continue;

		svenint_construct_pattern( signature, sPattern.c_str(), len );

		signatures.push_back( signature );
		signaturesLen.push_back( len );
		signaturesGameVer.push_back( pPatternKv->Key().c_str() );
	}

	auto find_patterns_async = []( module_t hModule,
								   std::vector<uint8_t *> &patterns,
								   std::vector<uint32_t> &patternsLen,
								   std::vector<std::future<void *>> &futures,
								   uint32_t offset )
	{
		// Maybe 4 as minimum
		futures.reserve( 4 );

		CMemoryUtils *pMemoryUtils = MemoryUtils();

		for ( size_t i = 0; i < patterns.size(); i++ )
		{
			uint8_t *&pattern = patterns[ i ];
			uint32_t &patternLen = patternsLen[ i ];

			futures.push_back( std::async( [ pMemoryUtils, hModule, pattern, patternLen, offset ] {
				return pMemoryUtils->FindPattern( hModule, pattern, patternLen, offset );
			} ) );
		}
	};

	int ndx;
	DEFINE_PATTERNS_FUTURE( futures );
	find_patterns_async( hModule, signatures, signaturesLen, futures, offset );
	void *ptr = MemoryUtils()->GetPatternFutureValue( futures, &ndx );

	for ( uint8_t *signature : signatures )
		free( signature );

	if ( bNotify )
	{
		if ( ptr != NULL )
		{
			MsgWrapper( "<GameData::FindPattern> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", pszPatternName, signaturesGameVer[ ndx ], ptr );
		}
		else
		{
			Warning( "<SvenInt::FindPattern> Failed to locate signature of symbol \"%s\"\n", pszPatternName );
		}
	}

	return ptr;
}

//-----------------------------------------------------------------------------
// FindSymbol
//-----------------------------------------------------------------------------

void *CGameDataFinder::FindSymbol( module_t hModule, const char *pszModuleName, const char *pszSymbolName, bool bNotify /* = true */ )
{
#ifdef LINUX
	static KeyValuesParser::KeyValues *pSymbols = NULL;
	if ( pSymbols == NULL )
		pSymbols = m_pGamedata->FindKey( "Symbols" );

	if ( pSymbols == NULL || pSymbols->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindSymbol> Section \"%s\" is missing, unable to look up for symbol \"%s\"\n", "Symbols", pszSymbolName );
		return NULL;
	}

	auto pModuleSection = pSymbols->FindKey( pszModuleName );
	if ( pModuleSection == NULL || pModuleSection->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindSymbol> Module section \"%s\" is missing, unable to look up for symbol \"%s\"\n", pszModuleName, pszSymbolName );
		return NULL;
	}

	auto pSymbol = pModuleSection->FindKey( pszSymbolName );
	if ( pSymbol == NULL || !pSymbol->IsPair() )
		return NULL; // it's just missing

	void *ptr = MemoryUtils()->ResolveSymbol( hModule, pSymbol->GetValue().c_str() );

	if ( bNotify )
	{
		if ( ptr != NULL )
		{
			MsgWrapper( "<GameData::FindSymbol> Found symbol \"%s\" at address 0x%X\n", pszSymbolName, ptr );
		}
		else
		{
			Warning( "<SvenInt::FindSymbol> Failed to locate symbol \"%s\"\n", pszSymbolName );
		}
	}

	return ptr;
#endif
	return NULL;
}

//-----------------------------------------------------------------------------
// FindOffset
//-----------------------------------------------------------------------------

uint32_t CGameDataFinder::FindOffset( module_t hModule, const char *pszModuleName, const char *pszOffsetName, bool bNotify /* = true */ )
{
	static KeyValuesParser::KeyValues *pOffsets = NULL;
	if ( pOffsets == NULL )
		pOffsets = m_pGamedata->FindKey( "Offsets" );

	if ( pOffsets == NULL || pOffsets->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindOffset> Section \"Offsets\" is missing, unable to look up for offset \"%s\"\n", pszOffsetName );
		return ~0;
	}

	auto pModuleSection = pOffsets->FindKey( pszModuleName );
	if ( pModuleSection == NULL || pModuleSection->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindOffset> Module section \"%s\" is missing, unable to look up for offset \"%s\"\n", pszModuleName, pszOffsetName );
		return ~0;
	}

	auto pRvaPlatformContainer = pModuleSection->FindKey( pszOffsetName );
	if ( pRvaPlatformContainer == NULL || pRvaPlatformContainer->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindOffset> Offset section \"%s\" is missing, unable to look up further\n", pszOffsetName );
		return ~0;
	}

#ifdef WIN32
	auto pOffsetPlatform = pRvaPlatformContainer->FindKey( "windows" );
#else
	auto pOffsetPlatform = pRvaPlatformContainer->FindKey( "linux" );
#endif
	if ( pOffsetPlatform == NULL || pOffsetPlatform->IsPair() )
	{
		if ( bNotify )
		#ifdef WIN32
			Warning( "<SvenInt::FindOffset> Platform section \"windows\" is missing, unable to look up for offset \"%s\"\n", pszOffsetName );
		#else
			Warning( "<SvenInt::FindOffset> Platform section \"linux\" is missing, unable to look up for offset \"%s\"\n", pszOffsetName );
		#endif
		return ~0;
	}

	int iRadix = 10;
	auto pRadix = pOffsetPlatform->FindKey( "radix" );
	if ( pRadix != NULL && pRadix->IsPair() )
	{
		iRadix = strtoul( pRadix->GetValue().c_str(), NULL, 10 );
	}

	auto pAny = pOffsetPlatform->FindKey( "any" );
	if ( pAny != NULL && pAny->IsPair() )
	{
		return strtoul( pAny->GetValue().c_str(), NULL, iRadix );
	}

	auto pOffset = pOffsetPlatform->FindKey( Globals::szGameVersion );
	if ( pOffset == NULL || !pOffset->IsPair() )
	{
		if ( bNotify )
			Warning( "<SvenInt::FindOffset> Named offset \"%s\" is not found for the game version \"%s\"\n", pszOffsetName, Globals::szGameVersion );
		return ~0;
	}

	uint32_t ulOffset = (uint32_t)strtoul( pOffset->GetValue().c_str(), NULL, iRadix );

	if ( bNotify && ulOffset != ~0 )
	{
		MsgWrapper( "<GameData::FindOffset> Found named offset \"%s\" of value %d for the game version\"%s\"\n", pszOffsetName, ulOffset, Globals::szGameVersion );
	}

	return ulOffset;
}

//-----------------------------------------------------------------------------
// Various gamedata (patterns, modules, interfaces, function pointers, variables, etc.)
//-----------------------------------------------------------------------------

namespace Globals
{
	static CGameDataFinder s_gamedata;
	CGameDataFinder *gamedata = &s_gamedata;

	int gameversion = 0;
	int gameversion_major = 0;
	int gameversion_minor = 0;
	int gameversion_patch = 0;

	char szGameVersion[ 16 ];

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
	KeyValuesParser::KeyValues *m_pGamedata;

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
	#ifdef WIN32
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
	#endif
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
		
		namespace Client
		{
			size_t vtidx_CStudioModelRenderer__StudioSetupBones = ~0;
			size_t vtidx_CStudioModelRenderer__StudioRenderModel = ~0;
		}

		namespace VGUI2
		{
			size_t vtidx_IPanel__PaintTraverse = ~0;
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

	bool GetGameModules( void )
	{
		bool bOK = true;

		std::future<bool> fLoadServerDLL;

	#ifdef WIN32
		const char *pszEngineLibrary = "hw.dll";
		const char *pszClientLibrary = "client.dll";
		const char *pszServerLibrary = "server.dll";
		const char *pszFilesystemLibrary = "filesystem_stdio.dll";
		const char *pszGameUiLibrary = "GameUI.dll";
		const char *pszVGui2Library = "vgui2.dll";
		const char *pszSDL2Library = "SDL2.dll";
		const char *pszOpenGlLibrary = "opengl32.dll";
	#else
		const char *pszEngineLibrary = "hw.so";
		const char *pszClientLibrary = "svencoop/cl_dlls/client.so";
		const char *pszServerLibrary = "svencoop/dlls/server.so";
		const char *pszFilesystemLibrary = "filesystem_stdio.so";
		const char *pszGameUiLibrary = "svencoop/cl_dlls/gameui.so";
		const char *pszVGui2Library = "vgui2.so";
		const char *pszSDL2Library = "libSDL2-2.0.so.0";
		const char *pszOpenGlLibrary = "libGL.so.1";
	#endif

		MAKE_ASYNC( fModuleEngine, [ pszEngineLibrary ] { return MemoryUtils()->GetModule( pszEngineLibrary ); } );
		MAKE_ASYNC( fModuleClient, [ pszClientLibrary ] { return MemoryUtils()->GetModule( pszClientLibrary ); } );
		MAKE_ASYNC( fModuleServer, [ pszServerLibrary] { return MemoryUtils()->GetModule( pszServerLibrary ); } );
		MAKE_ASYNC( fModuleFileSystem, [ pszFilesystemLibrary ] { return MemoryUtils()->GetModule( pszFilesystemLibrary ); } );
		MAKE_ASYNC( fModuleGameUI, [ pszGameUiLibrary ] { return MemoryUtils()->GetModule( pszGameUiLibrary ); } );
		MAKE_ASYNC( fModuleVGUI2, [ pszVGui2Library] { return MemoryUtils()->GetModule( pszVGui2Library ); } );
		MAKE_ASYNC( fModuleSDL2, [ pszSDL2Library ] { return MemoryUtils()->GetModule( pszSDL2Library ); } );
	#ifdef WIN32
		MAKE_ASYNC( fModuleOpenGL, [ pszOpenGlLibrary ] { return MemoryUtils()->GetModule( pszOpenGlLibrary ); } );
	#endif

		GAMEDATA_CHECK_MODULE( Engine, pszEngineLibrary, fModuleEngine );
		GAMEDATA_CHECK_MODULE( Client, pszClientLibrary, fModuleClient );
		GAMEDATA_CHECK_MODULE( FileSystem, pszFilesystemLibrary, fModuleFileSystem );
		GAMEDATA_CHECK_MODULE( GameUI, pszGameUiLibrary, fModuleGameUI );
		GAMEDATA_CHECK_MODULE( VGUI2, pszVGui2Library, fModuleVGUI2 );
		GAMEDATA_CHECK_MODULE( SDL2, pszSDL2Library, fModuleSDL2 );
	#ifdef WIN32
		GAMEDATA_CHECK_MODULE( OpenGL, pszOpenGlLibrary, fModuleOpenGL );
	#else
		Modules::OpenGL = MemoryUtils()->GetModule( pszOpenGlLibrary );
		if ( Modules::OpenGL == NULL )
			Modules::OpenGL = MemoryUtils()->GetModule( "libGL.so" );

		if ( Modules::OpenGL != NULL )
		{
			MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", pszOpenGlLibrary, Modules::OpenGL );
		}
		else
		{
			Warning2( "<SvenInt::GameData> Failed to get library \"%s\"\n", pszOpenGlLibrary );
			bOK = false;
		}
	#endif
		
		if ( ( Modules::Server = fModuleServer.get() ) == NULL )
		{
		#if 1
			fLoadServerDLL = std::async( [ pszServerLibrary ] {
				Modules::Server = MemoryUtils()->GetModule( pszServerLibrary );
				if ( Modules::Server != NULL )
					return true;

				MsgWrapper( "<SvenInt::GameData> Preloading server binary\n" );

				void *pfnSys_InitializeGameDLL;

				if ( gamedata->Initialized() )
				{
					pfnSys_InitializeGameDLL = gamedata->FindPattern( Modules::Engine, "Engine", "Sys_InitializeGameDll" );
				}
				else
				{
				#ifdef WIN32
					pfnSys_InitializeGameDLL = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::Sys_InitializeGameDLL );
				#else
					pfnSys_InitializeGameDLL = NULL;
				#endif
				}

				if ( pfnSys_InitializeGameDLL == NULL )
				{
					Warning( "<SvenInt::GameData> Failed to locate signature of symbol \"%s\"\n", "Sys_InitializeGameDLL" );
					return false;
				}

				( ( void ( * )( void ) )pfnSys_InitializeGameDLL )( );

				if ( ( Modules::Server = MemoryUtils()->GetModule( pszServerLibrary ) ) == NULL )
				{
					Warning( "<SvenInt::GameData> Failed to load server's binary\n" );
					return false;
				}

				MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", pszServerLibrary, Modules::Server );
				return true;
			} );
		#else
			Warning2( "<SvenInt::GameData> Failed to get library \"%s\"\n", pszServerLibrary );
			bOK = false;
		#endif
		}
		else
		{
			MsgWrapper( "<SvenInt::GameData> Received library module \"%s\" at address 0x%X\n", pszServerLibrary, Modules::Server );
		}

		if ( !bOK )
			return false;

		// Store modules info
		MAKE_ASYNC( fModuleInfoEngine, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::Engine ); } );
		MAKE_ASYNC( fModuleInfoClient, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::Client ); } );
		MAKE_ASYNC( fModuleInfoFileSystem, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::FileSystem ); } );
		MAKE_ASYNC( fModuleInfoGameUI, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::GameUI ); } );
		MAKE_ASYNC( fModuleInfoVGUI2, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::VGUI2 ); } );
		MAKE_ASYNC( fModuleInfoSDL2, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::SDL2 ); } );
	#ifdef WIN32
		MAKE_ASYNC( fModuleInfoOpenGL, [] { return MemoryUtils()->RetrieveModuleInfo( Modules::OpenGL ); } );
	#endif

		GAMEDATA_CHECK_MODULE_INFO( pszEngineLibrary, fModuleInfoEngine );
		GAMEDATA_CHECK_MODULE_INFO( pszClientLibrary, fModuleInfoClient );
		GAMEDATA_CHECK_MODULE_INFO( pszFilesystemLibrary, fModuleInfoFileSystem );
		GAMEDATA_CHECK_MODULE_INFO( pszGameUiLibrary, fModuleInfoGameUI );
		GAMEDATA_CHECK_MODULE_INFO( pszVGui2Library, fModuleInfoVGUI2 );
		GAMEDATA_CHECK_MODULE_INFO( pszSDL2Library, fModuleInfoSDL2 );
	#ifdef WIN32
		GAMEDATA_CHECK_MODULE_INFO( pszOpenGlLibrary, fModuleInfoOpenGL );
	#endif

		if ( fLoadServerDLL.valid() && !fLoadServerDLL.get() )
			return false;

		if ( !MemoryUtils()->RetrieveModuleInfo( Modules::Server ) )
		{
			Warning2( "<SvenInt::GameData> Failed to retrieve module's info of library \"%s\"\n", pszServerLibrary );
			bOK = false;
		}

		return bOK;
	}

	bool GetGameInterfaceFactories( void )
	{
	#ifdef WIN32
		const char *pszEngineLibrary = "hw.dll";
		const char *pszClientLibrary = "client.dll";
		const char *pszServerLibrary = "server.dll";
		const char *pszFilesystemLibrary = "filesystem_stdio.dll";
		const char *pszGameUiLibrary = "GameUI.dll";
		const char *pszVGui2Library = "vgui2.dll";
	#else
		const char *pszEngineLibrary = "hw.so";
		const char *pszClientLibrary = "svencoop/cl_dlls/client.so";
		const char *pszServerLibrary = "svencoop/dlls/server.so";
		const char *pszFilesystemLibrary = "filesystem_stdio.so";
		const char *pszGameUiLibrary = "svencoop/cl_dlls/gameui.so";
		const char *pszVGui2Library = "vgui2.so";
	#endif

		bool bOK = true;

		MAKE_ASYNC( fInterfaceFactoryEngine, [] { return MemoryUtils()->GetProcAddress( Modules::Engine, CREATEINTERFACE_PROCNAME ); } );
		MAKE_ASYNC( fInterfaceFactoryClient, [] { return MemoryUtils()->GetProcAddress( Modules::Client, CREATEINTERFACE_PROCNAME ); } );
		MAKE_ASYNC( fInterfaceFactoryServer, [] { return MemoryUtils()->GetProcAddress( Modules::Server, CREATEINTERFACE_PROCNAME ); } );
		MAKE_ASYNC( fInterfaceFactoryFileSystem, [] { return MemoryUtils()->GetProcAddress( Modules::FileSystem, CREATEINTERFACE_PROCNAME ); } );
		MAKE_ASYNC( fInterfaceFactoryGameUI, [] { return MemoryUtils()->GetProcAddress( Modules::GameUI, CREATEINTERFACE_PROCNAME ); } );
		MAKE_ASYNC( fInterfaceFactoryVGUI2, [] { return MemoryUtils()->GetProcAddress( Modules::VGUI2, CREATEINTERFACE_PROCNAME ); } );

		GAMEDATA_CHECK_IFACE_FACTORY( Engine, pszEngineLibrary, fInterfaceFactoryEngine );
		GAMEDATA_CHECK_IFACE_FACTORY( Client, pszClientLibrary, fInterfaceFactoryClient );
		GAMEDATA_CHECK_IFACE_FACTORY( Server, pszServerLibrary, fInterfaceFactoryServer );
		GAMEDATA_CHECK_IFACE_FACTORY( FileSystem, pszFilesystemLibrary, fInterfaceFactoryFileSystem );
		GAMEDATA_CHECK_IFACE_FACTORY( GameUI, pszGameUiLibrary, fInterfaceFactoryGameUI );
		GAMEDATA_CHECK_IFACE_FACTORY( VGUI2, pszVGui2Library, fInterfaceFactoryVGUI2 );

		return bOK;
	}

	bool GetGameInterfaces( void )
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
		MAKE_ASYNC( fpEngineClient, GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineClient );
		MAKE_ASYNC( fpBaseUI, GetInterfaceIteratively, InterfaceFactories::Engine, szVBaseUI );
		MAKE_ASYNC( fpGameUIFuncs, GetInterfaceIteratively, InterfaceFactories::Engine, szVGameUIFuncs );
		MAKE_ASYNC( fpEngineSurface, GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineSurface );
		MAKE_ASYNC( fpEngineVGui, GetInterfaceIteratively, InterfaceFactories::Engine, szVEngineVGui );
		MAKE_ASYNC( fpSurface, GetInterfaceIteratively, InterfaceFactories::Engine, szVSurface );
		// FileSystem
		MAKE_ASYNC( fpFileSystem, GetInterfaceIteratively, InterfaceFactories::FileSystem, szVFileSystem );
		// GameUI
		MAKE_ASYNC( fpGameUI, GetInterfaceIteratively, InterfaceFactories::GameUI, szVGameUI );
		MAKE_ASYNC( fpGameConsole, GetInterfaceIteratively, InterfaceFactories::GameUI, szVGameConsole );
		MAKE_ASYNC( fpRunGameEngine, GetInterfaceIteratively, InterfaceFactories::GameUI, szVRunGameEngine );
		// VGUI2
		MAKE_ASYNC( fpVGui, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVGui );
		MAKE_ASYNC( fpSchemeManager, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVSchemeManager );
		MAKE_ASYNC( fpLocalize, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVLocalize );
		MAKE_ASYNC( fpPanel, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVPanel );
		MAKE_ASYNC( fpInput, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVInput );
		MAKE_ASYNC( fpInputInternal, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVInputInternal );
		MAKE_ASYNC( fpSystem, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVSystem );
		MAKE_ASYNC( fpKeyValues, GetInterfaceIteratively, InterfaceFactories::VGUI2, szVKeyValues );

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
		{
			void *p__MsgFunc_ServerVer;
		#ifdef LINUX
			void *pRELOC_OFFSET__MsgFunc_ServerVer;
		#endif

			if ( gamedata->Initialized() )
			{
				p__MsgFunc_ServerVer = gamedata->FindPattern( Modules::Client, "Client", "__MsgFunc_ServerVer" );
				if ( p__MsgFunc_ServerVer == NULL )
					return false;

			#ifdef LINUX
				pRELOC_OFFSET__MsgFunc_ServerVer = gamedata->FindPattern( Modules::Client, "Client", "RELOC_OFFSET__MsgFunc_ServerVer" );
				if ( pRELOC_OFFSET__MsgFunc_ServerVer == NULL )
					return false;
			#endif
			}
			else
			{
			#ifdef WIN32
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( f__MsgFunc_ServerVer );

				MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::__MsgFunc_ServerVer, f__MsgFunc_ServerVer );

				p__MsgFunc_ServerVer = MemoryUtils()->GetPatternFutureValue( f__MsgFunc_ServerVer, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS( p__MsgFunc_ServerVer,
												"__MsgFunc_ServerVer",
												Patterns::Client::__MsgFunc_ServerVer,
												patternIndex );
			#else
				return false;
			#endif
			}

			ud_t inst;

		#ifdef LINUX
			uint32_t relocOffset = (uint32_t)pRELOC_OFFSET__MsgFunc_ServerVer;
			MemoryUtils()->InitDisasm( &inst, pRELOC_OFFSET__MsgFunc_ServerVer, 32, 15 );
			if ( MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Iadd && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_EBX && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					relocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
				}
			}
		#endif

			int iMajorVer = 0, iMinorVer = 0, iPatchVer = 0;
			MemoryUtils()->InitDisasm( &inst, p__MsgFunc_ServerVer, 32, 48 );

			while ( MemoryUtils()->Disassemble( &inst ) )
			{
			#ifdef WIN32
				if ( inst.mnemonic == UD_Ipush && inst.operand[ 0 ].type == UD_OP_IMM )
			#else
				if ( relocOffset && inst.mnemonic == UD_Ilea &&
					 inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_ESI &&
					 inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].base == UD_R_EBX )
			#endif
				{
				#ifdef WIN32
					memcpy( Globals::szGameVersion, (char *)inst.operand[ 0 ].lval.udword, strlen( (char *)inst.operand[ 0 ].lval.udword ) + 1 );
				#else
					relocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
					memcpy( Globals::szGameVersion, (char *)relocOffset, strlen( (char *)relocOffset ) + 1 );
				#endif

				/* Linux
					NU PROSTO PIZDEC NAHUYA???

					CONST = 0x548E7D;
					OFFSET = 0xFFE8CE2B
					ADDR1 = 0xE5183;

				ADDR1:
					ADD EBX,CONST

				ADDR2:
					LEA ESI,[EBX + OFFSET]

					REAL_ADDRESS = OFFSET + CONST + ADDR1;
					REAL_ADDRESST = 0x4BAE2B;
				*/

					char *pszClientVersion = Globals::szGameVersion;
					char *pszFirstSeparator = strchr( pszClientVersion, '.' );

					if ( pszFirstSeparator == NULL )
					{
						Warning( "<SvenInt::GameData> Tried to find game's version in format 'X.XX' or 'X.XX.X' but got this: '%s'\n", pszClientVersion );
						break;
					}

					char *pszSecondSeparator = strchr( pszFirstSeparator + 1, '.' );

					*pszFirstSeparator = 0;
					iMajorVer = atoi( pszClientVersion );
					*pszFirstSeparator = '.';

					pszFirstSeparator += 1;
					if ( pszSecondSeparator != NULL )
						*pszSecondSeparator = 0;
					iMinorVer = atoi( pszFirstSeparator );
					if ( pszSecondSeparator != NULL )
						*pszSecondSeparator = '.';

					if ( pszSecondSeparator != NULL )
					{
						pszSecondSeparator += 1;
						iPatchVer = atoi( pszSecondSeparator );
					}

					Globals::gameversion = iMajorVer * 100 + iMinorVer;
					Globals::gameversion_major = iMajorVer;
					Globals::gameversion_minor = iMinorVer;
					Globals::gameversion_patch = iPatchVer;
					break;
				}
			}

			if ( Globals::gameversion == 0 )
			{
				Warning( "<SvenInt::GameData> Failed to locate game's version\n" );
				return false;
			}

			if ( iPatchVer != 0 )
				MsgWrapper( "<SvenInt::GameData> Found game's version: \"%d.%.2d.%d\"\n", iMajorVer, iMinorVer, iPatchVer );
			else
				MsgWrapper( "<SvenInt::GameData> Found game's version: \"%d.%.2d\"\n", iMajorVer, iMinorVer );
		}

		// Find console printing
		auto fConsolePrint = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Pointers::GameUI::RichText__InsertColorChange = gamedata->FindRVA( Modules::GameUI, "GameUI", "RichText::InsertColorChange" );
				Pointers::GameUI::RichText__InsertString = gamedata->FindRVA( Modules::GameUI, "GameUI", "RichText::InsertString" );

				if ( Pointers::GameUI::RichText__InsertColorChange == NULL )
					return false;
				if ( Pointers::GameUI::RichText__InsertString == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pCGameConsoleDialog__DPrint;

			if ( gamedata->Initialized() )
			{
				pCGameConsoleDialog__DPrint = gamedata->FindPattern( Modules::GameUI, "GameUI", "CGameConsoleDialog::DPrint" );
				if ( pCGameConsoleDialog__DPrint == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fCGameConsoleDialog__DPrint );

				MemoryUtils()->FindPatternAsync( Modules::GameUI, Patterns::GameUI::CGameConsoleDialog__DPrint, fCGameConsoleDialog__DPrint );

				pCGameConsoleDialog__DPrint = MemoryUtils()->GetPatternFutureValue( fCGameConsoleDialog__DPrint, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS( pCGameConsoleDialog__DPrint,
												"CGameConsoleDialog::DPrint",
												Patterns::GameUI::CGameConsoleDialog__DPrint,
												patternIndex );
			}

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

			GAMEDATA_DUMP_FILE_OFFSET( "RichText::InsertColorChange", Pointers::GameUI::RichText__InsertColorChange, Modules::GameUI );
			GAMEDATA_DUMP_FILE_OFFSET( "RichText::InsertString", Pointers::GameUI::RichText__InsertString, Modules::GameUI );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'cl_enginefunc_t *cl_enginefuncs' & 'playermove_t *g_clmove'
		auto fEngineFuncsAndPlayerMove = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				bool bOK = true;

				MAKE_ASYNC( fcl_enginefuncs, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cl_enginefuncs" ); } );
				MAKE_ASYNC( fplayermove, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "g_clmove" ); } );
				MAKE_ASYNC( fcls, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cls" ); } );
				MAKE_ASYNC( fg_pClientUserMsgs, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "g_pClientUserMsgs" ); } );
				MAKE_ASYNC( fg_pEventHooks, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "g_pEventHooks" ); } );
				MAKE_ASYNC( fcmd_argc, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cmd_argc" ); } );
				MAKE_ASYNC( fcmd_argv, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cmd_argv" ); } );

				Globals::cl_enginefuncs = (cl_enginefunc_t *)fcl_enginefuncs.get();
				Globals::playermove = (CBasePlayerMove *)fplayermove.get();
				Globals::cls = (client_static_t *)fcls.get();
				Globals::g_pClientUserMsgs = (usermsg_t **)fg_pClientUserMsgs.get();
				Globals::g_pEventHooks = (event_hook_t *)fg_pEventHooks.get();
				Globals::cmd_argc = (int *)fcmd_argc.get();
				Globals::cmd_argv = (const char **)fcmd_argv.get();

				if ( Globals::cl_enginefuncs == NULL )
					return false;
				if ( Globals::playermove == NULL )
					return false;
				if ( Globals::cls == NULL )
					return false;
				if ( Globals::cmd_argc == NULL )
					return false;
				if ( Globals::cmd_argv == NULL )
					return false;

				if ( Globals::g_pClientUserMsgs != NULL )
				{
					ud_t inst;
					const usermsg_t *pUserMsgHideHUD = Globals::gameutils->FindUserMessage( "HideHUD" );
				#ifdef WIN32
					bool bFoundAnyCall = false;
					void *pMsgFunc = (void *)pUserMsgHideHUD->function;

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
				#else
					bool bFoundPcThunk = false;
					int iDisassembledBytes = 0;
					uint32_t ulRelocOffset = 0;
					uint8_t *pMsgFunc = (uint8_t *)pUserMsgHideHUD->function;
					uint8_t *p = (uint8_t *)pMsgFunc;
					uint8_t *gpHud = NULL;

					MemoryUtils()->InitDisasm( &inst, pMsgFunc, 32, 60 );
					while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
					{
						if ( inst.mnemonic == UD_Icall )
						{
							if ( !bFoundPcThunk )
							{
								bFoundPcThunk = true;
							}
							else
							{
								pMsgFunc = (uint8_t *)MemoryUtils()->CalcAbsoluteAddress( p );
								break;
							}
						}

						p += iDisassembledBytes;
					}

					p = pMsgFunc;

					MemoryUtils()->InitDisasm( &inst, pMsgFunc, 32, 60 );
					while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
					{
						if ( inst.mnemonic == UD_Icall && ulRelocOffset == 0 )
						{
							ulRelocOffset = (uint32_t)( p + iDisassembledBytes );
						}
						else if ( inst.mnemonic == UD_Iadd && inst.operand[ 0 ].type == UD_OP_REG &&
								  inst.operand[ 0 ].base == UD_R_EBX && inst.operand[ 1 ].type == UD_OP_IMM )
						{
							ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
						}
						else if ( ulRelocOffset != 0 && inst.mnemonic == UD_Ilea &&
								  inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_EDX &&
								  inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].base == UD_R_EBX )
						{
							ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
							gpHud = reinterpret_cast<uint8_t *>( ulRelocOffset );
						}
						else if ( gpHud != NULL && inst.mnemonic == UD_Imov &&
								  inst.operand[ 0 ].type == UD_OP_MEM && inst.operand[ 0 ].base == UD_R_EDX &&
								  inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
						{
							Globals::gpHideHUD = reinterpret_cast<int *>( gpHud + (uint32_t)inst.operand[ 0 ].lval.udword );
							break;
						}

						p += iDisassembledBytes;
					}
				#endif
				}

				GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL_STATUS( Globals::gpHideHUD, "m_iHideHUD" );
				return true;
			}

		#ifdef WIN32
			void *pClientDLL_Init;

			if ( gamedata->Initialized() )
			{
				pClientDLL_Init = gamedata->FindPattern( Modules::Engine, "Engine", "ClientDLL_Init" );
				if ( pClientDLL_Init == NULL )
					return false;
			}
			else
			{
				pClientDLL_Init = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::ClientDLL_Init );
				GAMEDATA_CHECK_SYMBOL_PATTERN( pClientDLL_Init, "ClientDLL_Init" );
			}

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
			MemoryUtils()->InitDisasm( &inst, (void *)Globals::cl_enginefuncs->pNetAPI->Status, 32, 36 );

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
					void *pMsgFunc = (void *)pUserMsgHideHUD->function;

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
				void *pHookEvent = (void *)Globals::cl_enginefuncs->pfnHookEvent;
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

				void *pCmd_Argc = (void *)Globals::cl_enginefuncs->Cmd_Argc;
				if ( *( (unsigned char *)Globals::cl_enginefuncs->Cmd_Argc ) == 0xE9 ) // JMP opcode
					pCmd_Argc = MemoryUtils()->CalcAbsoluteAddress( (void *)Globals::cl_enginefuncs->Cmd_Argc );

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

				void *pCmd_Argv = (void *)Globals::cl_enginefuncs->Cmd_Argv;
				if ( *( (unsigned char *)Globals::cl_enginefuncs->Cmd_Argv ) == 0xE9 ) // JMP opcode
					pCmd_Argv = MemoryUtils()->CalcAbsoluteAddress( (void *)Globals::cl_enginefuncs->Cmd_Argv );

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

			GAMEDATA_DUMP_FILE_OFFSET( "cl_enginefuncs", Globals::cl_enginefuncs, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "g_clmove", Globals::playermove, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "cls", Globals::cls, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "gClientUserMsgs", Globals::g_pClientUserMsgs, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "g_pEventHooks", Globals::g_pEventHooks, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "cmd_argc", Globals::cmd_argc, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "cmd_argv", Globals::cmd_argv, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'cldll_func_t *cl_funcs'
		auto fClientFuncs = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Globals::cl_funcs = (cldll_func_t *)gamedata->FindRVA( Modules::Engine, "Engine", "cl_funcs" );
				if ( Globals::cl_funcs == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pLoadInsecureClient;

			if ( gamedata->Initialized() )
			{
				pLoadInsecureClient = gamedata->FindPattern( Modules::Engine, "Engine", "LoadInsecureClient" );
				if ( pLoadInsecureClient == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fLoadInsecureClient );

				MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::LoadInsecureClient, fLoadInsecureClient );

				pLoadInsecureClient = MemoryUtils()->GetPatternFutureValue( fLoadInsecureClient, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS( pLoadInsecureClient,
												"LoadInsecureClient",
												Patterns::Engine::LoadInsecureClient,
												patternIndex );
			}

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
			GAMEDATA_DUMP_FILE_OFFSET( "cl_funcs", Globals::cl_funcs, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'engine_studio_api_t *enginestudio'
		auto fEngineStudio = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Globals::enginestudio = (engine_studio_api_t *)gamedata->FindRVA( Modules::Engine, "Engine", "engine_studio_api" );
				Globals::studioapi = (r_studio_interface_t *)gamedata->FindRVA( Modules::Engine, "Engine", "studioapi" );

				if ( Globals::enginestudio == NULL )
					return false;
				if ( Globals::studioapi == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pV_EngineStudio_Init;

			if ( gamedata->Initialized() )
			{
				pV_EngineStudio_Init = gamedata->FindPattern( Modules::Engine, "Engine", "V_EngineStudio_Init" );
				if ( pV_EngineStudio_Init == NULL )
					return false;
			}
			else
			{
				pV_EngineStudio_Init = MemoryUtils()->FindPattern( Modules::Engine, Patterns::Engine::V_EngineStudio_Init );
				GAMEDATA_CHECK_SYMBOL_PATTERN( pV_EngineStudio_Init, "V_EngineStudio_Init" );
			}

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
						GAMEDATA_DUMP_FILE_OFFSET( "studioapi", inst.operand[ 0 ].lval.udword, Modules::Engine );
						Globals::studioapi = *reinterpret_cast<r_studio_interface_t **>( inst.operand[ 0 ].lval.udword );
						break;
					}
				}
			}

			GAMEDATA_CHECK_SYMBOL( Globals::enginestudio, "engine_studio_api" );
			GAMEDATA_CHECK_SYMBOL( Globals::studioapi, "studioapi" );

			GAMEDATA_DUMP_FILE_OFFSET( "engine_studio_api", Globals::enginestudio, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'IVideoMode *videomode'
		auto fVideoMode = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Globals::videomode_ptr = (IVideoMode **)gamedata->FindRVA( Modules::Engine, "Engine", "videomode_ptr" );
				if ( Globals::videomode_ptr == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pV_VideoMode_Create;

			if ( gamedata->Initialized() )
			{
				pV_VideoMode_Create = gamedata->FindPattern( Modules::Engine, "Engine", "V_VideoMode_Create" );
				if ( pV_VideoMode_Create == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fV_VideoMode_Create );

				MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::V_VideoMode_Create, fV_VideoMode_Create );

				pV_VideoMode_Create = MemoryUtils()->GetPatternFutureValue( fV_VideoMode_Create, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS( pV_VideoMode_Create,
												"V_VideoMode_Create",
												Patterns::Engine::V_VideoMode_Create,
												patternIndex );
			}

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
			GAMEDATA_DUMP_FILE_OFFSET( "videomode_ptr", Globals::videomode_ptr, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'CStudioModelRenderer g_StudioRenderer'
		auto fStudioRenderer = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Globals::studiorenderer = (CStudioModelRenderer *)gamedata->FindRVA( Modules::Client, "Client", "g_StudioRenderer" );
				if ( Globals::studiorenderer == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pHUD_GetStudioModelInterface = MemoryUtils()->GetProcAddress( Modules::Client, "HUD_GetStudioModelInterface" );
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
			GAMEDATA_DUMP_FILE_OFFSET( "g_StudioRenderer", Globals::studiorenderer, Modules::Client );

			return true;
		#else
			return false;
		#endif
		} );
		
		// Find 'double host_frametime' & 'double realtime'
		auto fFrametime = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				MAKE_ASYNC( fHost_FilterTime, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Host_FilterTime" ); } );
				MAKE_ASYNC( fhost_frametime, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "host_frametime" ); } );
				MAKE_ASYNC( frealtime, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "realtime" ); } );

				Pointers::Engine::Host_FilterTime = fHost_FilterTime.get();
				Globals::host_frametime = (double *)fhost_frametime.get();
				Globals::realtime = (double *)frealtime.get();

				if ( Pointers::Engine::Host_FilterTime == NULL )
					return false;
				if ( Globals::host_frametime == NULL )
					return false;
				if ( Globals::realtime == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			if ( gamedata->Initialized() )
			{
				Pointers::Engine::Host_FilterTime = gamedata->FindPattern( Modules::Engine, "Engine", "Host_FilterTime" );
				if ( Pointers::Engine::Host_FilterTime == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fHost_FilterTime );

				MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::Host_FilterTime, fHost_FilterTime );

				Pointers::Engine::Host_FilterTime = MemoryUtils()->GetPatternFutureValue( fHost_FilterTime, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS( Pointers::Engine::Host_FilterTime,
												"Host_FilterTime",
												Patterns::Engine::Host_FilterTime,
												patternIndex );
			}

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

			GAMEDATA_DUMP_FILE_OFFSET( "Host_FilterTime", Pointers::Engine::Host_FilterTime, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "host_frametime", Globals::host_frametime, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "realtime", Globals::realtime, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'cvar_t **cvar_vars' & 'cmd_function_t **cmd_functions'
		auto fCvar = std::async( [] {
			bool bOK = true;

			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				MAKE_ASYNC( fZ_Free, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Z_Free" ); } );
				MAKE_ASYNC( fMem_Free, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Mem_Free" ); } );
				MAKE_ASYNC( fCvar_DirectSet, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Cvar_DirectSet" ); } );
				MAKE_ASYNC( fcvar_vars, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cvar_vars" ); } );
				MAKE_ASYNC( fcmd_functions, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "cmd_functions" ); } );

				Pointers::Engine::Z_Free = fZ_Free.get();
				Pointers::Engine::Mem_Free = fMem_Free.get();
				Pointers::Engine::Cvar_DirectSet = fCvar_DirectSet.get();
				Globals::cvar_vars = (cvar_t **)fcvar_vars.get();
				Globals::cmd_functions = (cmd_function_t **)fcmd_functions.get();

				if ( Pointers::Engine::Z_Free == NULL )
					return false;
				if ( Pointers::Engine::Mem_Free == NULL )
					return false;
				if ( Pointers::Engine::Cvar_DirectSet == NULL )
					return false;
				if ( Globals::cvar_vars == NULL )
					return false;
				if ( Globals::cmd_functions == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pCmd_AddCommand;
			void *pCvar_RemoveClientDLLCvars;

			if ( gamedata->Initialized() )
			{
				MAKE_ASYNC( fZ_Free, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Z_Free" ); } );
				MAKE_ASYNC( fMem_Free, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Mem_Free" ); } );
				MAKE_ASYNC( fCvar_DirectSet, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Cvar_DirectSet" ); } );
				MAKE_ASYNC( fpCmd_AddCommand, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Cmd_AddCommand" ); } );
				MAKE_ASYNC( fpCvar_RemoveClientDLLCvars, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Cvar_RemoveClientDLLCvars" ); } );
				
				Pointers::Engine::Z_Free = fZ_Free.get();
				Pointers::Engine::Mem_Free = fMem_Free.get();
				Pointers::Engine::Cvar_DirectSet = fCvar_DirectSet.get();
				pCmd_AddCommand = fpCmd_AddCommand.get();
				pCvar_RemoveClientDLLCvars = fpCvar_RemoveClientDLLCvars.get();

				if ( Pointers::Engine::Z_Free == NULL )
					return false;

				if ( Pointers::Engine::Mem_Free == NULL )
					return false;
				
				if ( Pointers::Engine::Cvar_DirectSet == NULL )
					return false;

				if ( pCmd_AddCommand == NULL )
					return false;

				if ( pCvar_RemoveClientDLLCvars == NULL )
					return false;
			}
			else
			{
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
				pCmd_AddCommand = fCmd_AddCommand.get();
				pCvar_RemoveClientDLLCvars = MemoryUtils()->GetPatternFutureValue( fCvar_RemoveClientDLLCvars, &patternIndex );
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
			}

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

			GAMEDATA_DUMP_FILE_OFFSET( "cvar_vars", Globals::cvar_vars, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "cmd_functions", Globals::cmd_functions, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "Z_Free", Pointers::Engine::Z_Free, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "Mem_Free", Pointers::Engine::Mem_Free, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "Cvar_DirectSet", Pointers::Engine::Cvar_DirectSet, Modules::Engine );

			return bOK;
		#else
			return false;
		#endif
		} );

		// Find 'extra_player_info_t g_PlayerExtraInfo'
		auto fExtraPlayerInfo = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Pointers::Client::GetClientColor = gamedata->FindRVA( Modules::Client, "Client", "GetClientColor" );
				Globals::extraplayerinfo = (CBaseExtraPlayerInfo *)gamedata->FindRVA( Modules::Client, "Client", "g_ExtraPlayerInfo" );

				if ( Pointers::Client::GetClientColor == NULL )
					return false;
				if ( Globals::extraplayerinfo == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			if ( gamedata->Initialized() )
			{
				Pointers::Client::GetClientColor = gamedata->FindPattern( Modules::Client, "Client", "GetClientColor" );
				if ( Pointers::Client::GetClientColor == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fGetClientColor );

				MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::GetClientColor, fGetClientColor );

				Pointers::Client::GetClientColor = MemoryUtils()->GetPatternFutureValue( fGetClientColor, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( Pointers::Client::GetClientColor,
															 "GetClientColor",
															 Patterns::Client::GetClientColor,
															 patternIndex );
			}

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

			GAMEDATA_DUMP_FILE_OFFSET( "GetClientColor", Pointers::Client::GetClientColor, Modules::Client );
			GAMEDATA_DUMP_FILE_OFFSET( "extraplayerinfo", Globals::extraplayerinfo, Modules::Client );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'WeaponsResource gWR'
		auto fWeaponsResource = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Globals::weaponsresource = (CBaseWeaponsResource *)gamedata->FindRVA( Modules::Client, "Client", "gWR" );
				if ( Globals::weaponsresource == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pCHudAmmo__VidInit;

			if ( gamedata->Initialized() )
			{
				pCHudAmmo__VidInit = gamedata->FindPattern( Modules::Client, "Client", "CHudAmmo::VidInit" );
				if ( pCHudAmmo__VidInit == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fCHudAmmo__VidInit );

				MemoryUtils()->FindPatternAsync( Modules::Client, Patterns::Client::CHudAmmo__VidInit, fCHudAmmo__VidInit );

				pCHudAmmo__VidInit = MemoryUtils()->GetPatternFutureValue( fCHudAmmo__VidInit, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( pCHudAmmo__VidInit,
															 "CHudAmmo::VidInit",
															 Patterns::Client::CHudAmmo__VidInit,
															 patternIndex );
			}

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
			GAMEDATA_DUMP_FILE_OFFSET( "gWR", Globals::weaponsresource, Modules::Client );

			return true;
		#else
			return false;
		#endif
		} );

		// Find 'g_NetworkMessages'
		auto fNetworkMessages = std::async( [] {
			void *pszSvcBad = MemoryUtils()->FindString( Modules::Engine, "svc_bad" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pszSvcBad, "svc_bad" );
			
			void *xref = MemoryUtils()->FindAddress( Modules::Engine, pszSvcBad );

		#ifndef WIN32
			if ( xref != NULL && *( (uint8_t *)xref - 2 ) == 0x8D ) // lea
			{
				moduleinfo_t modinfo;
				if ( MemoryUtils()->RetrieveModuleInfo( Modules::Engine, &modinfo ) )
				{
					const uint32_t base = (uint32_t)modinfo.pBaseOfDll;
					const uint32_t offset = ( (uint32_t)xref - base ) + sizeof( void * );
					xref = MemoryUtils()->FindAddress( Modules::Engine, pszSvcBad, offset );
				}
				else
				{
					xref = NULL;
				}
			}
		#endif

			Globals::g_NetworkMessages = (netmsg_t *)xref;
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::g_NetworkMessages, "g_NetworkMessages" );

			return true;
		} );
		
		// Find 'clc_buffer'
		auto fClcBuffer = std::async( [] {
		#ifdef WIN32
			void *pszSendRes = MemoryUtils()->FindString( Modules::Engine, "sendres" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pszSendRes, "sendres" );
			
			uint8_t *pclc_buffer = (uint8_t *)MemoryUtils()->FindAddress( Modules::Engine, pszSendRes );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pclc_buffer, "PUSH [sendres]" );
			
			pclc_buffer += sizeof( void * );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( *pclc_buffer == 0x68, "PUSH [clc_buffer]" );

			Globals::clc_buffer = *(sizebuf_t **)( pclc_buffer + 1 );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::clc_buffer, "clc_buffer" );
		#else
		#if 0
			// I don't want to lookup for EBX's reloc offset, screw it
			void *pszConAccepted = MemoryUtils()->FindString( Modules::Engine, "Connection accepted.\n" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pszConAccepted, "Connection accepted.\\n" );

			uint8_t *pclc_buffer = (uint8_t *)MemoryUtils()->FindAddress( Modules::Engine, pszConAccepted );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( pclc_buffer, "LEA eax, (Connection accepted.\\n)[ebx]" );

			pclc_buffer += 0xC;
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( *pclc_buffer == 0x8D, "LEA esi, (clc_buffer)[ebx]" );

			Globals::clc_buffer = *(sizebuf_t **)( pclc_buffer + 2 );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL( Globals::clc_buffer, "clc_buffer" );
		#else
			Globals::clc_buffer = gamedata->FindRVA( Modules::Engine, "Engine", "clc_buffer" );
			if ( Globals::clc_buffer == NULL )
				return false;
		#endif
		#endif

			return true;
		} );
		
		// Find 'sizebuf_t net_message', 'int msg_readcount' & 'qboolean msg_badread'
		auto fNetworkMessageBuffer = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				MAKE_ASYNC( fmsg_readcount, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "msg_readcount" ); } );
				MAKE_ASYNC( fmsg_badread, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "msg_badread" ); } );
				MAKE_ASYNC( fnet_message, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "net_message" ); } );

				Globals::msg_readcount = (int *)fmsg_readcount.get();
				Globals::msg_badread = (int *)fmsg_badread.get();
				Globals::net_message = (sizebuf_t *)fnet_message.get();

				if ( Globals::msg_readcount == NULL )
					return false;
				if ( Globals::msg_badread == NULL )
					return false;
				if ( Globals::net_message == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			void *pMSG_ReadByte;

			if ( gamedata->Initialized() )
			{
				pMSG_ReadByte = gamedata->FindPattern( Modules::Engine, "Engine", "MSG_ReadByte" );
				if ( pMSG_ReadByte == NULL )
					return false;
			}
			else
			{
				int patternIndex;
				DEFINE_PATTERNS_FUTURE( fMSG_ReadByte );

				MemoryUtils()->FindPatternAsync( Modules::Engine, Patterns::Engine::MSG_ReadByte, fMSG_ReadByte );

				pMSG_ReadByte = MemoryUtils()->GetPatternFutureValue( fMSG_ReadByte, &patternIndex );
				GAMEDATA_CHECK_SYMBOL_PATTERNS_NOT_CRITICAL( pMSG_ReadByte,
															 "MSG_ReadByte",
															 Patterns::Engine::MSG_ReadByte,
															 patternIndex );
			}

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

			GAMEDATA_DUMP_FILE_OFFSET( "msg_readcount", Globals::msg_readcount, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "msg_badread", Globals::msg_badread, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "net_message", Globals::net_message, Modules::Engine );

			return true;
		#else
			return false;
		#endif
		} );

		// Find SCR_BeginLoadingPlaque & SCR_EndLoadingPlaque
		auto fSCRLoadingPlaque = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Pointers::Engine::SCR_BeginLoadingPlaque = gamedata->FindRVA( Modules::Engine, "Engine", "SCR_BeginLoadingPlaque" );
				Pointers::Engine::SCR_EndLoadingPlaque = gamedata->FindRVA( Modules::Engine, "Engine", "SCR_EndLoadingPlaque" );

				if ( Pointers::Engine::SCR_BeginLoadingPlaque == NULL )
					return false;

				if ( Pointers::Engine::SCR_EndLoadingPlaque == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			bool bOK = true;

			if ( gamedata->Initialized() )
			{
				MAKE_ASYNC( fSCR_BeginLoadingPlaque, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "SCR_BeginLoadingPlaque" ); } );
				MAKE_ASYNC( fSCR_EndLoadingPlaque, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "SCR_EndLoadingPlaque" ); } );

				Pointers::Engine::SCR_BeginLoadingPlaque = fSCR_BeginLoadingPlaque.get();
				Pointers::Engine::SCR_EndLoadingPlaque = fSCR_EndLoadingPlaque.get();

				if ( Pointers::Engine::SCR_BeginLoadingPlaque == NULL )
					return false;
				
				if ( Pointers::Engine::SCR_EndLoadingPlaque == NULL )
					return false;
			}
			else
			{
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
			}

			GAMEDATA_DUMP_FILE_OFFSET( "SCR_BeginLoadingPlaque", Pointers::Engine::SCR_BeginLoadingPlaque, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "SCR_EndLoadingPlaque", Pointers::Engine::SCR_EndLoadingPlaque, Modules::Engine );

			return bOK;
		#else
			return false;
		#endif
		} );

	#ifdef WIN32
		// Find server patterns
		auto fServerGameDataPatterns = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				MAKE_ASYNC( fPlayerSpawns, [] { return gamedata->FindRVA( Modules::Server, "Server", "PlayerSpawns" ); } );
				MAKE_ASYNC( fFixPlayerStuck, [] { return gamedata->FindRVA( Modules::Server, "Server", "FixPlayerStuck" ); } );
				MAKE_ASYNC( fFireBullets, [] { return gamedata->FindRVA( Modules::Server, "Server", "CBaseEntity::FireBullets" ); } );
				MAKE_ASYNC( fUTIL_GetCircularGaussianSpread, [] { return gamedata->FindRVA( Modules::Server, "Server", "UTIL_GetCircularGaussianSpread" ); } );
				MAKE_ASYNC( fFireTargets, [] { return gamedata->FindRVA( Modules::Server, "Server", "FireTargets" ); } );
			#ifdef WIN32
				MAKE_ASYNC( fCopyPEntityVars, [] { return gamedata->FindRVA( Modules::Server, "Server", "CopyPEntityVars" ); } );
			#endif

				Pointers::Server::PlayerSpawns = fPlayerSpawns.get();
				Pointers::Server::FixPlayerStuck = fFixPlayerStuck.get();
				Pointers::Server::CBaseEntity__FireBullets = fFireBullets.get();
				Pointers::Server::UTIL_GetCircularGaussianSpread = fUTIL_GetCircularGaussianSpread.get();
				Pointers::Server::FireTargets = fFireTargets.get();
			#ifdef WIN32
				Pointers::Server::CopyPEntityVars = fCopyPEntityVars.get();
			#endif

				if ( Pointers::Server::PlayerSpawns == NULL )
					return false;
				if ( Pointers::Server::FixPlayerStuck == NULL )
					return false;
				if ( Pointers::Server::CBaseEntity__FireBullets == NULL )
					return false;
				if ( Pointers::Server::UTIL_GetCircularGaussianSpread == NULL )
					return false;
				if ( Pointers::Server::FireTargets == NULL )
					return false;
			#ifdef WIN32
				if ( Pointers::Server::CopyPEntityVars == NULL )
					return false;
			#endif

				return true;
			}

		#ifdef WIN32
			bool bOK = true;

			if ( gamedata->Initialized() )
			{
				MAKE_ASYNC( fPlayerSpawns, [] { return gamedata->FindPattern( Modules::Server, "Server", "PlayerSpawns" ); } );
				MAKE_ASYNC( fFixPlayerStuck, [] { return gamedata->FindPattern( Modules::Server, "Server", "FixPlayerStuck" ); } );
				MAKE_ASYNC( fCBaseEntity__FireBullets, [] { return gamedata->FindPattern( Modules::Server, "Server", "CBaseEntity::FireBullets" ); } );
				MAKE_ASYNC( fUTIL_GetCircularGaussianSpread, [] { return gamedata->FindPattern( Modules::Server, "Server", "UTIL_GetCircularGaussianSpread" ); } );
				MAKE_ASYNC( fFireTargets, [] { return gamedata->FindPattern( Modules::Server, "Server", "FireTargets" ); } );
				MAKE_ASYNC( fCopyPEntityVars, [] { return gamedata->FindPattern( Modules::Server, "Server", "CopyPEntityVars" ); } );

				Pointers::Server::PlayerSpawns = fPlayerSpawns.get();
				Pointers::Server::FixPlayerStuck = fFixPlayerStuck.get();
				Pointers::Server::CBaseEntity__FireBullets = fCBaseEntity__FireBullets.get();
				Pointers::Server::UTIL_GetCircularGaussianSpread = fUTIL_GetCircularGaussianSpread.get();
				Pointers::Server::FireTargets = fFireTargets.get();
				Pointers::Server::CopyPEntityVars = fCopyPEntityVars.get();

				if ( Pointers::Server::PlayerSpawns == NULL )
					return false;
				
				if ( Pointers::Server::FixPlayerStuck == NULL )
					return false;
				
				if ( Pointers::Server::CBaseEntity__FireBullets == NULL )
					return false;
				
				if ( Pointers::Server::UTIL_GetCircularGaussianSpread == NULL )
					return false;
				
				if ( Pointers::Server::FireTargets == NULL )
					return false;
				
				if ( Pointers::Server::CopyPEntityVars == NULL )
					return false;
			}
			else
			{
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
			}

			if ( Pointers::Server::CopyPEntityVars != NULL )
				Pointers::Server::CopyPEntityVars = MemoryUtils()->CalcAbsoluteAddress( Pointers::Server::CopyPEntityVars );

			GAMEDATA_DUMP_FILE_OFFSET( "PlayerSpawns", Pointers::Server::PlayerSpawns, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "FixPlayerStuck", Pointers::Server::FixPlayerStuck, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "FireBullets", Pointers::Server::CBaseEntity__FireBullets, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "UTIL_GetCircularGaussianSpread", Pointers::Server::UTIL_GetCircularGaussianSpread, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "FireTargets", Pointers::Server::FireTargets, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "CopyPEntityVars", Pointers::Server::CopyPEntityVars, Modules::Server );

			return bOK;
		#else
			return false;
		#endif
		} );
	#endif

		// Further loading cannot proceed without important symbols
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

			void *GiveFnptrsToDll = MemoryUtils()->GetProcAddress( Modules::Server, "GiveFnptrsToDll" );
			APIFUNCTION GetEntityAPI = (APIFUNCTION)MemoryUtils()->GetProcAddress( Modules::Server, "GetEntityAPI" );
			NEW_DLL_FUNCTIONS_FN GetNewDLLFunctions = (NEW_DLL_FUNCTIONS_FN)MemoryUtils()->GetProcAddress( Modules::Server, "GetNewDLLFunctions" );

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

			int iDisassembledBytes = 0;
			uint8_t *p = (uint8_t *)GiveFnptrsToDll;

		#ifdef WIN32
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
		#else
			uint32_t relocOffset = 0;
			MemoryUtils()->InitDisasm( &inst, GiveFnptrsToDll, 32, 44 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Iadd && inst.operand[ 0 ].type == UD_OP_REG &&
					 inst.operand[ 0 ].base == UD_R_EBX && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					relocOffset = (uint32_t)p + (uint32_t)inst.operand[ 1 ].lval.udword;
				}
				else if ( inst.mnemonic == UD_Ilea &&
					 inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_EDI &&
					 inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].base == UD_R_EBX )
				{
					if ( relocOffset != 0 )
						Globals::sv_enginefuncs = reinterpret_cast<enginefuncs_t *>( relocOffset + (uint32_t)inst.operand[ 1 ].lval.udword );
				}
				else if ( inst.mnemonic == UD_Imov &&
						  inst.operand[ 0 ].type == UD_OP_MEM && inst.operand[ 0 ].base == UD_R_EBX &&
						  inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 1 ].base == UD_R_EAX )
				{
					if ( relocOffset != 0 )
						pgpGlobals = reinterpret_cast<globalvars_t **>( relocOffset + (uint32_t)inst.operand[ 0 ].lval.udword );
					break;
				}

				p += iDisassembledBytes;
			}
		#endif

			if ( pgpGlobals != NULL )
				Globals::gpGlobals = *pgpGlobals;
			
			GAMEDATA_CHECK_SYMBOL( Globals::sv_enginefuncs, "gpEngFuncs" );
			GAMEDATA_CHECK_SYMBOL( Globals::gpGlobals, "gpGlobals" );

			if ( Globals::cl_enginefuncs == NULL )
				return false;
			
			cmd_function_t *disconnect = Globals::cvar->FindCmd( "disconnect" );
			GAMEDATA_CHECK_SYMBOL( disconnect, "disconnect" );

			iDisassembledBytes = 0;
			p = (uint8_t *)disconnect->function;

		#ifdef WIN32
			bool bFoundFirstCall = false;

			MemoryUtils()->InitDisasm( &inst, (void *)disconnect->function, 32, 16 );
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
		#else
			int iCalls = 0;

			MemoryUtils()->InitDisasm( &inst, (void *)disconnect->function, 32, 25 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icall )
				{
					if ( iCalls < 2 )
					{
						iCalls++;
					}
					else
					{
						Pointers::Engine::Host_IsServerActive = MemoryUtils()->CalcAbsoluteAddress( p );
						break;
					}
				}

				p += iDisassembledBytes;
			}
		#endif

			Pointers::Server::vmt_CBasePlayer = MemoryUtils()->FindVTable( Modules::Server, "CBasePlayer" );

			GAMEDATA_CHECK_SYMBOL( Pointers::Engine::Host_IsServerActive, "Host_IsServerActive" );
			GAMEDATA_CHECK_SYMBOL_NOT_CRITICAL_STATUS( Pointers::Server::vmt_CBasePlayer, "CBasePlayer (VMT)" );

			GAMEDATA_DUMP_FILE_OFFSET( "Host_IsServerActive", Pointers::Engine::Host_IsServerActive, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "CBasePlayer (VMT)", Pointers::Server::vmt_CBasePlayer, Modules::Server );

			return true;
		} );

		// Find survival mode instance & toggle command
		auto fServerSurvivalMode = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Pointers::Server::toggle_survival_mode_Callback = gamedata->FindRVA( Modules::Server, "Server", "toggle_survival_mode_Callback" );
				Pointers::Server::pSurvivalModeInstance = gamedata->FindRVA( Modules::Server, "Server", "pSurvivalModeInstance" );

				if ( Pointers::Server::toggle_survival_mode_Callback == NULL )
					return false;
				if ( Pointers::Server::pSurvivalModeInstance == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
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

			GAMEDATA_DUMP_FILE_OFFSET( "toggle_survival_mode_Callback", Pointers::Server::toggle_survival_mode_Callback, Modules::Server );
			GAMEDATA_DUMP_FILE_OFFSET( "pSurvivalModeInstance", Pointers::Server::pSurvivalModeInstance, Modules::Server );

			return bOK;
		#else
			return false;
		#endif
		} );
		
		// Find GL_Bind
		auto fGL_Bind = std::async( [] {
			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				Pointers::Engine::GL_Bind = gamedata->FindRVA( Modules::Engine, "Engine", "GL_Bind" );
				if ( Pointers::Engine::GL_Bind == NULL )
					return false;

				return true;
			}

		#ifdef WIN32
			bool bOK = true;

			if ( Globals::cl_enginefuncs == NULL )
				return false;

			ud_t inst;
			int iDisassembledBytes = 0, iCallCount = 0;
			uint8_t *p = (uint8_t *)Globals::cl_enginefuncs->pTriAPI->SpriteTexture;

			MemoryUtils()->InitDisasm( &inst, (void *)Globals::cl_enginefuncs->pTriAPI->SpriteTexture, 32, 48 );
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
			GAMEDATA_DUMP_FILE_OFFSET( "GL_Bind", Pointers::Engine::GL_Bind, Modules::Engine );

			return bOK;
		#else
			return false;
		#endif
		} );
		
		// Guess server vtidxs based on the game version
		auto fGuessServerVtidx = std::async( [] {
			if ( gamedata->Initialized() )
			{
				bool bOK = true;

				MAKE_ASYNC( fvtidx_CBaseEntity_ObjectCaps, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBaseEntity::ObjectCaps" ); } );
				MAKE_ASYNC( fvtidx_CBasePlayer_IsAlive, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBasePlayer::IsAlive" ); } );
				MAKE_ASYNC( fvtidx_CBasePlayer_BeginRevive, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBasePlayer::BeginRevive" ); } );
				MAKE_ASYNC( fvtidx_CBasePlayer_EndRevive, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBasePlayer::EndRevive" ); } );
				MAKE_ASYNC( fvtidx_CBasePlayer_SpecialSpawn, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBasePlayer::SpecialSpawn" ); } );
				MAKE_ASYNC( fvtidx_CBasePlayer_IsConnected, [] { return gamedata->FindOffset( Modules::Server, "Server", "CBasePlayer::IsConnected" ); } );

				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = fvtidx_CBaseEntity_ObjectCaps.get();
				Offsets::Server::vtidx_CBasePlayer_IsAlive = fvtidx_CBasePlayer_IsAlive.get();
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = fvtidx_CBasePlayer_BeginRevive.get();
				Offsets::Server::vtidx_CBasePlayer_EndRevive = fvtidx_CBasePlayer_EndRevive.get();
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = fvtidx_CBasePlayer_SpecialSpawn.get();
				Offsets::Server::vtidx_CBasePlayer_IsConnected = fvtidx_CBasePlayer_IsConnected.get();

				if ( Offsets::Server::vtidx_CBaseEntity_ObjectCaps == ~0 )
					bOK = false;
				else if ( Offsets::Server::vtidx_CBasePlayer_IsAlive == ~0 )
					bOK = false;
				else if ( Offsets::Server::vtidx_CBasePlayer_BeginRevive == ~0 )
					bOK = false;
				else if ( Offsets::Server::vtidx_CBasePlayer_EndRevive == ~0 )
					bOK = false;
				else if ( Offsets::Server::vtidx_CBasePlayer_SpecialSpawn == ~0 )
					bOK = false;
				else if ( Offsets::Server::vtidx_CBasePlayer_IsConnected == ~0 )
					bOK = false;

				if ( bOK )
					return;
				else
					Warning2( "[SvenInt::GameData] Unable to guess indexes of virtual methods for the game DLL (game version: %s)\n", Globals::szGameVersion );

			#ifdef WIN32
				Warning2( "[SvenInt::GameData] Fall back using statically defined offsets\n" );
			#else
				return;
			#endif
			}

			switch ( SVEN_VERSION() )
			{
		#ifdef WIN32
			case SVEN_VERSION_CHECK( 5, 26, 0 ):
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 9;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 48;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 90;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 91;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 212;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 220;

				break;
			}

			case SVEN_VERSION_CHECK( 5, 25, 0 ):
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 9;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 47;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 88;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 89;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 210;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 218;

				break;
			}

			case SVEN_VERSION_CHECK( 5, 11, 0 ):
			{
				Offsets::Server::vtidx_CBaseEntity_ObjectCaps = 8;

				Offsets::Server::vtidx_CBasePlayer_IsAlive = 43;
				Offsets::Server::vtidx_CBasePlayer_BeginRevive = 82;
				Offsets::Server::vtidx_CBasePlayer_EndRevive = 83;
				Offsets::Server::vtidx_CBasePlayer_SpecialSpawn = 192;
				Offsets::Server::vtidx_CBasePlayer_IsConnected = 198;

				break;
			}
		#endif

			default:
				Warning2( "[SvenInt::GameData] Unable to guess indexes of virtual methods for the game DLL (game version: %s)\n", Globals::szGameVersion );
				break;
			}
		} );
		
		// Guess rest vtidxs
		auto fGuessRestVtidx = std::async( [] {
			if ( gamedata->Initialized() )
			{
				bool bOK = true;

				MAKE_ASYNC( fvtidx_CStudioModelRenderer__StudioSetupBones, [] { return gamedata->FindOffset( Modules::Client, "Client", "CStudioModelRenderer::StudioSetupBones" ); } );
				MAKE_ASYNC( fvtidx_CStudioModelRenderer__StudioRenderModel, [] { return gamedata->FindOffset( Modules::Client, "Client", "CStudioModelRenderer::StudioRenderModel" ); } );
				MAKE_ASYNC( fvtidx_IPanel__PaintTraverse, [] { return gamedata->FindOffset( Modules::VGUI2, "VGUI2", "IPanel::PaintTraverse" ); } );

				Offsets::Client::vtidx_CStudioModelRenderer__StudioSetupBones = fvtidx_CStudioModelRenderer__StudioSetupBones.get();
				Offsets::Client::vtidx_CStudioModelRenderer__StudioRenderModel = fvtidx_CStudioModelRenderer__StudioRenderModel.get();
				Offsets::VGUI2::vtidx_IPanel__PaintTraverse = fvtidx_IPanel__PaintTraverse.get();

				if ( Offsets::Client::vtidx_CStudioModelRenderer__StudioSetupBones == ~0 )
					bOK = false;
				else if ( Offsets::Client::vtidx_CStudioModelRenderer__StudioRenderModel == ~0 )
					bOK = false;
				else if ( Offsets::VGUI2::vtidx_IPanel__PaintTraverse == ~0 )
					bOK = false;

				if ( bOK )
					return true;
				else
					Warning2( "[SvenInt::GameData] Unable to guess indexes of some virtual methods\n" );

			// #ifdef WIN32
				Warning2( "[SvenInt::GameData] Fall back using statically defined offsets\n" );
			// #else
				// return false;
			// #endif
			}

		#ifdef WIN32
			Offsets::Client::vtidx_CStudioModelRenderer__StudioSetupBones = 7;
			Offsets::Client::vtidx_CStudioModelRenderer__StudioRenderModel = 20;
			Offsets::VGUI2::vtidx_IPanel__PaintTraverse = 41;
		#else
			Offsets::Client::vtidx_CStudioModelRenderer__StudioSetupBones = 8;
			Offsets::Client::vtidx_CStudioModelRenderer__StudioRenderModel = 21;
			Offsets::VGUI2::vtidx_IPanel__PaintTraverse = 42;
		#endif

			return true;
		} );
		
		// Find functions to detour
		auto fFindFunctions = std::async( [] {
			bool bOK = true;

			if ( gamedata->Initialized() && gamedata->PreferRVA() )
			{
				MAKE_ASYNC( fNetchan_Transmit, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Netchan_Transmit" ); } );
				MAKE_ASYNC( fMSG_WriteUsercmd, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "MSG_WriteUsercmd" ); } );
				MAKE_ASYNC( fKey_Event, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "Key_Event" ); } );
				MAKE_ASYNC( fSCR_UpdateScreen, [] { return gamedata->FindRVA( Modules::Engine, "Engine", "SCR_UpdateScreen" ); } );
				MAKE_ASYNC( fIN_Move, [] { return gamedata->FindRVA( Modules::Client, "Client", "IN_Move" ); } );

				Pointers::Engine::Netchan_Transmit = fNetchan_Transmit.get();
				Pointers::Engine::MSG_WriteUsercmd = fMSG_WriteUsercmd.get();
				Pointers::Engine::Key_Event = fKey_Event.get();
				Pointers::Engine::SCR_UpdateScreen = fSCR_UpdateScreen.get();
				Pointers::Client::IN_Move = fIN_Move.get();

				if ( Pointers::Engine::Netchan_Transmit == NULL )
					return false;
				if ( Pointers::Engine::MSG_WriteUsercmd == NULL )
					return false;
				if ( Pointers::Engine::Key_Event == NULL )
					return false;
				if ( Pointers::Engine::SCR_UpdateScreen == NULL )
					return false;
				if ( Pointers::Client::IN_Move == NULL )
					return false;

				return bOK;
			}

		#ifdef WIN32
			if ( gamedata->Initialized() )
			{
				MAKE_ASYNC( fNetchan_Transmit, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Netchan_Transmit" ); } );
				MAKE_ASYNC( fMSG_WriteUsercmd, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "MSG_WriteUsercmd" ); } );
				MAKE_ASYNC( fKey_Event, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "Key_Event" ); } );
				MAKE_ASYNC( fSCR_UpdateScreen, [] { return gamedata->FindPattern( Modules::Engine, "Engine", "SCR_UpdateScreen" ); } );
				MAKE_ASYNC( fIN_Move, [] { return gamedata->FindPattern( Modules::Client, "Client", "IN_Move" ); } );

				Pointers::Engine::Netchan_Transmit = fNetchan_Transmit.get();
				Pointers::Engine::MSG_WriteUsercmd = fMSG_WriteUsercmd.get();
				Pointers::Engine::Key_Event = fKey_Event.get();
				Pointers::Engine::SCR_UpdateScreen = fSCR_UpdateScreen.get();
				Pointers::Client::IN_Move = fIN_Move.get();

				if ( Pointers::Engine::Netchan_Transmit == NULL )
					return false;

				if ( Pointers::Engine::MSG_WriteUsercmd == NULL )
					return false;

				if ( Pointers::Engine::Key_Event == NULL )
					return false;
				
				if ( Pointers::Engine::SCR_UpdateScreen == NULL )
					return false;
				
				if ( Pointers::Client::IN_Move == NULL )
					return false;
			}
			else
			{
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
			}

			GAMEDATA_DUMP_FILE_OFFSET( "IN_Move", Pointers::Client::IN_Move, Modules::Client );
			GAMEDATA_DUMP_FILE_OFFSET( "MSG_WriteUsercmd", Pointers::Engine::MSG_WriteUsercmd, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "Netchan_Transmit", Pointers::Engine::Netchan_Transmit, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "Key_Event", Pointers::Engine::Key_Event, Modules::Engine );
			GAMEDATA_DUMP_FILE_OFFSET( "SCR_UpdateScreen", Pointers::Engine::SCR_UpdateScreen, Modules::Engine );
			
			return true;
		#else
			return false;
		#endif
		} );

		fExtraPlayerInfo.get();
		fWeaponsResource.get();
		fNetworkMessages.get();
		fClcBuffer.get();
		fNetworkMessageBuffer.get();
		fGL_Bind.get();
	#ifdef WIN32
		fServerGameDataPatterns.get();
	#endif
		fServerSurvivalMode.get();
		fGuessServerVtidx.get();

		if ( !fGuessRestVtidx.get() )
			return false;
		
		if ( !fFindFunctions.get() )
			return false;

		Globals::engineclient = GetBaseEngineClient( Globals::engineclient, SVEN_VERSION() );
		Globals::extraplayerinfo = GetBaseExtraPlayerInfo( Globals::extraplayerinfo, SVEN_VERSION() );
		Globals::playermove = GetBasePlayerMove( Globals::playermove, SVEN_VERSION() );
		Globals::weaponsresource = GetBaseWeaponsResource( Globals::weaponsresource, SVEN_VERSION() );

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

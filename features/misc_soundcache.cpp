// SvenInt (c) Sw1ft
// misc_soundcache.cpp

#include "stdafx.h"
#include "misc_soundcache.h"

#ifndef WIN32
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#endif

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare Hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( bool, CClient_SoundEngine__LoadSoundList, void *thisptr );
DECLARE_CLASS_HOOK( void, CClient_SoundEngine__FlushCache, void *thisptr, bool host );

//-----------------------------------------------------------------------------
// Feature gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Client
		{
			DEFINE_PATTERN( CClient_SoundEngine__LoadSoundList, "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 ? 08 00 00" );

			DEFINE_PATTERNS( CClient_SoundEngine__FlushCache,
							 "5.25",
							 "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 54 03 00 00",
							 "5.11",
							 "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 5C 04 00 00" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSoundcache, soundcache, "Misc", "Soundcache" );

static NetMsgHookFn ORIG_NetMsgHook_ResourceList = NULL;

static char szMapName[ MAX_PATH ];
static char szSoundcacheDir[ MAX_PATH ];
static char szServerSoundcacheDir[ MAX_PATH ];
static char szServerSoundcacheFolder[ MAX_PATH ];

//-----------------------------------------------------------------------------
// Netmsg ResourceList
//-----------------------------------------------------------------------------

static void HOOKED_NetMsgHook_ResourceList( void )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->NetMsgHookResourceList();

	ORIG_NetMsgHook_ResourceList();
}

//-----------------------------------------------------------------------------
// CClient_SoundEngine::LoadSoundList hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( bool, HOOKED_CClient_SoundEngine__LoadSoundList, void *thisptr )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->SoundEngineLoadSoundList();

	return ORIG_CClient_SoundEngine__LoadSoundList( ARG_THISPTR( thisptr ) );
}

//-----------------------------------------------------------------------------
// CClient_SoundEngine::FlushCache hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_CClient_SoundEngine__FlushCache, void *thisptr, bool host )
{
	if ( THIS_FEATURE_IS_ENABLED() && THIS_FEATURE()->SoundEngineFlushCache() )
		return;

	return ORIG_CClient_SoundEngine__FlushCache( ARG_THISPTR( thisptr ), host );
}

//-----------------------------------------------------------------------------
// Copy file (Linux)
//-----------------------------------------------------------------------------

#ifndef WIN32
static int CopyFile( const char *src, const char *dest )
{
    FILE *fsrc = fopen( src, "rb" );
    if ( fsrc == NULL )
		return -1;

    FILE *fdest = fopen( dest, "wb" );
    if ( fdest == NULL )
	{
        fclose( fsrc );
        return -1;
    }

    char buffer[ 4096 ];
    size_t bytes;
    while ( ( bytes = fread( buffer, 1, Q_ARRAYSIZE( buffer ), fsrc ) ) > 0 )
	{
        if ( fwrite( buffer, 1, bytes, fdest) != bytes )
		{
            fclose( fsrc );
            fclose( fdest );
            return -1;
        }
    }

    fclose( fsrc );
    fclose( fdest );

    return 0;
}
#endif

//-----------------------------------------------------------------------------
// Netmsg ResourceList event
//-----------------------------------------------------------------------------

void CSoundcache::NetMsgHookResourceList( void )
{
	m_bUseSavedSoundcache = false;

	if ( m_pMode->GetInt() != 0 )
		return;

	constexpr int localhost = 0x7F000001; // 127.0.0.1

	netadr_t addr;
	int port;

#if 0
	engineclient->GetServerAddress( &addr );
#else
	static net_status_t status;
	cl_enginefuncs->pNetAPI->Status( &status );
	memcpy( &addr, &status.remote_address, sizeof( netadr_t ) );
#endif

	port = ( addr.port << 8 ) | ( addr.port >> 8 ); // it's swapped

	if ( *(int *)addr.ip == 0 || *(int *)addr.ip == localhost )
		return;

	char mapname_buffer[ MAX_PATH ];

	char *pszMapName = mapname_buffer;
	char *pszExt = NULL;

	strncpy( mapname_buffer, cl_enginefuncs->pfnGetLevelName(), MAX_PATH );

	// maps/<mapname>.bsp to <mapname>
	while ( *pszMapName )
	{
		if ( *pszMapName == '/' )
		{
			pszMapName++;
			break;
		}

		pszMapName++;
	}

	pszExt = pszMapName;

	while ( *pszExt )
	{
		if ( *pszExt == '.' )
		{
			*pszExt = 0;
			break;
		}

		pszExt++;
	}

	strncpy( szMapName, pszMapName, MAX_PATH );

#ifdef WIN32
	snprintf( szSoundcacheDir, MAX_PATH, "maps\\soundcache\\%s.txt", szMapName );
	snprintf( szServerSoundcacheDir, MAX_PATH, "maps\\soundcache\\%hhu.%hhu.%hhu.%hhu %hu\\%s.txt", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ], port, szMapName );
	snprintf( szServerSoundcacheFolder, MAX_PATH, "maps\\soundcache\\%hhu.%hhu.%hhu.%hhu %hu", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ], port );
#else
	snprintf( szSoundcacheDir, MAX_PATH, "maps/soundcache/%s.txt", szMapName );
	snprintf( szServerSoundcacheDir, MAX_PATH, "maps/soundcache/%hhu.%hhu.%hhu.%hhu %hu/%s.txt", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ], port, szMapName );
	snprintf( szServerSoundcacheFolder, MAX_PATH, "maps/soundcache/%hhu.%hhu.%hhu.%hhu %hu", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ], port );
#endif

	// Skip download of soundcache
	if ( m_pIgnore->GetBool() )
	{
		FileHandle_t hFile = filesystem->Open( szSoundcacheDir, "a+", "GAMEDOWNLOAD" ); // dummy

		if ( hFile )
		{
			// Empty infos
			filesystem->FPrintf( hFile, "%s\n", szMapName );
			filesystem->FPrintf( hFile, "%hhu.%hhu.%hhu.%hhu\n", addr.ip[ 0 ], addr.ip[ 1 ], addr.ip[ 2 ], addr.ip[ 3 ] );
			filesystem->FPrintf( hFile, "SOUNDLIST {\n" );
			filesystem->FPrintf( hFile, "}\n" );
			filesystem->FPrintf( hFile, "SENTENCELIST {\n" );
			filesystem->FPrintf( hFile, "}\n" );
			filesystem->FPrintf( hFile, "CUSTOMMATERIALS {\n" );
			filesystem->FPrintf( hFile, "}\n" );

			filesystem->Close( hFile );
		}

		return;
	}

	// Uh we have the soundcache that wasn't deleted
	if ( filesystem->FileExists( szSoundcacheDir ) )
	{
		std::string sDirectory = UTIL_GetLongPathName();

	#ifdef WIN32
		SetFileAttributes( ( sDirectory + "\\svencoop_downloads\\" + szSoundcacheDir ).c_str(), FILE_ATTRIBUTE_NORMAL ); // WinAPI
	#else
		chmod( ( sDirectory + "/svencoop_downloads/" + szSoundcacheDir ).c_str(), 0644 );
	#endif
		//DeleteFileA(szFullSoundcacheDir);
		filesystem->RemoveFile( szSoundcacheDir, "GAMEDOWNLOAD" );
	}

	// If client already has the saved soundcache then create a dummy file so we won't download anything..
	if ( filesystem->FileExists( szServerSoundcacheDir ) )
	{
		FileHandle_t hFile = filesystem->Open( szSoundcacheDir, "a+", "GAMEDOWNLOAD" ); // dummy

		if ( hFile )
		{
			filesystem->Close( hFile );
		}

		m_bHasSoundcache = true;
	}
	else
	{
		m_bHasSoundcache = false;
	}

	m_bUseSavedSoundcache = true;
}

//-----------------------------------------------------------------------------
// CClient_SoundEngine::LoadSoundList event
//-----------------------------------------------------------------------------

void CSoundcache::SoundEngineLoadSoundList( void )
{
	if ( m_pMode->GetInt() != 0 || m_pIgnore->GetBool() || !m_bUseSavedSoundcache )
		return;

	if ( filesystem->FileExists( szSoundcacheDir ) )
	{
		std::string sDirectory = UTIL_GetLongPathName();

	#ifdef WIN32
		std::string sSoundcacheDir = sDirectory + "\\svencoop_downloads\\" + szSoundcacheDir;
		std::string sServerSoundcacheDir = sDirectory + "\\svencoop_downloads\\" + szServerSoundcacheDir;

		SetFileAttributes( sSoundcacheDir.c_str(), FILE_ATTRIBUTE_NORMAL );

		if ( !CreateDirectory( ( sDirectory + "\\svencoop_downloads\\" + szServerSoundcacheFolder ).c_str(), NULL ) && GetLastError() != ERROR_ALREADY_EXISTS )
	#else
		std::string sSoundcacheDir = sDirectory + "/svencoop_downloads/" + szSoundcacheDir;
		std::string sServerSoundcacheDir = sDirectory + "/svencoop_downloads/" + szServerSoundcacheDir;

		chmod( sSoundcacheDir.c_str(), 0644 );

		if ( mkdir( ( sDirectory + "/svencoop_downloads/" + szServerSoundcacheFolder ).c_str(), 0777 ) == -1 && errno != EEXIST )
	#endif
		{
			PrintWarning( "Failed to create directory \"..\\%s\"\n", szServerSoundcacheFolder );
			return;
		}

		if ( m_bHasSoundcache )
		{
			filesystem->RemoveFile( szSoundcacheDir, "GAMEDOWNLOAD" );

		#ifdef WIN32
			if ( CopyFile( sServerSoundcacheDir.c_str(), sSoundcacheDir.c_str(), true ) )
		#else
			if ( CopyFile( sServerSoundcacheDir.c_str(), sSoundcacheDir.c_str() ) == 0 )
		#endif
			{
				PrintMsg( "Used saved soundcache for the current map \"%s\"\n", szMapName );
			}
			else
			{
				PrintWarning( "Failed to replace soundcache for the current map \"%s\"\n", szMapName );
			}
		}
		else
		{
		#ifdef WIN32
			if ( CopyFile( sSoundcacheDir.c_str(), sServerSoundcacheDir.c_str(), true ) )
		#else
			if ( CopyFile( sSoundcacheDir.c_str(), sServerSoundcacheDir.c_str() ) == 0 )
		#endif
			{
				PrintMsg( "Saved soundcache for the current map \"%s\"\n", szMapName );
			}
			else
			{
				PrintWarning( "Failed to save soundcache for the current map \"%s\"\n", szMapName );
			}
		}
	}
	else
	{
		PrintWarning( "Unable to find soundcache for the current map \"%s\"\n", szMapName );
	}
}

//-----------------------------------------------------------------------------
// CClient_SoundEngine::FlushCache event
//-----------------------------------------------------------------------------

bool CSoundcache::SoundEngineFlushCache( void )
{
	return m_pMode->GetInt() == 0 && m_pDontFlush->GetBool();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSoundcache::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// Host_FilterTime post event
	if ( !pEvent->GetReturn<qboolean>() )
		return kHookContinue;

	if ( m_pMode->GetInt() != 1 )
		return kHookContinue;

	if ( cls->state < ca_connected || cls->state == ca_active )
		return kHookContinue;

	static int sleep_frames = 0;
#ifdef WIN32
	char szSoundcacheDirectory[ MAX_PATH ] = { 0 };
#else
	char szSoundcacheDirectory[ PATH_MAX ] = { 0 };
#endif

	if ( !*szSoundcacheDirectory )
	#ifdef WIN32
		snprintf( szSoundcacheDirectory, MAX_PATH, "%s\\svencoop_downloads\\maps\\soundcache\\", UTIL_GetLongPathName() );
	#else
		snprintf( szSoundcacheDirectory, PATH_MAX, "%s/svencoop_downloads/maps/soundcache", UTIL_GetLongPathName() );
	#endif

	sleep_frames++;

	if ( sleep_frames >= 75 )
	{
	#ifdef WIN32
		HANDLE hFile;
		WIN32_FIND_DATAA FileInformation;

		char m_szPath[ MAX_PATH ];
		char m_szFolderInitialPath[ MAX_PATH ];
		char wildCard[ MAX_PATH ] = "\\*.*";

		strcpy( m_szPath, szSoundcacheDirectory );
		strcpy( m_szFolderInitialPath, szSoundcacheDirectory );
		strcat( m_szFolderInitialPath, wildCard );

		hFile = FindFirstFile( m_szFolderInitialPath, &FileInformation );

		if ( hFile != INVALID_HANDLE_VALUE )
		{
			do
			{
				if ( FileInformation.cFileName[ 0 ] != '.' )
				{
					strcpy( m_szPath, szSoundcacheDirectory );
					strcat( m_szPath, "\\" );
					strcat( m_szPath, FileInformation.cFileName );

					if ( !( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
						 ( FileInformation.nFileSizeHigh * ( MAXDWORD + 1 ) ) + FileInformation.nFileSizeLow > 0 )
					{
						//it is a file
						SetFileAttributes( m_szPath, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY );
					}
				}
			} while ( FindNextFile( hFile, &FileInformation ) == TRUE );

			FindClose( hFile );
		}
	#else
		DIR *dir = opendir( szSoundcacheDirectory );
		if ( dir != NULL )
		{
			struct dirent *entry;
			while ( ( entry = readdir( dir ) ) != NULL )
			{
				if ( stricmp( entry->d_name, ".")  == 0 || stricmp( entry->d_name, ".." ) == 0 )
					continue;

				char szFullPath[ PATH_MAX ];
				snprintf( szFullPath, Q_ARRAYSIZE( szFullPath ), "%s/%s", szSoundcacheDirectory, entry->d_name );

				struct stat st;
				if ( stat( szFullPath, &st ) == 0 && S_ISDIR( st.st_mode ) )
					continue;

				if ( stat( szFullPath, &st ) != -1 && S_ISREG( st.st_mode ) && st.st_size == 0 )
					continue;

				chmod( szFullPath, 0444 );
			}

			closedir(dir);
		}
	#endif

		sleep_frames = 0;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSoundcache::CSoundcache( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pMode = NULL;
	m_pDontFlush = NULL;
	m_pIgnore = NULL;

	m_bHasSoundcache = false;
	m_bUseSavedSoundcache = false;

	m_pfnCClient_SoundEngine__LoadSoundList = NULL;
	m_pfnCClient_SoundEngine__FlushCache = NULL;

	m_hCClient_SoundEngine__LoadSoundList = DETOUR_INVALID_HANDLE;
	m_hCClient_SoundEngine__FlushCache = DETOUR_INVALID_HANDLE;
	m_hNetMsgHook_ResourceList = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSoundcache::OnEnable( void )
{
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSoundcache::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSoundcache::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Save for each server\0 1 - Save in a single downloads folder\0\0" );
	m_pDontFlush = Modules::menu->AddParamBool( this, "DontFlush", NULL, false );
	m_pIgnore = Modules::menu->AddParamBool( this, "Ignore", NULL, false );

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		MAKE_ASYNC( fm_pfnCClient_SoundEngine__LoadSoundList, [] { return gamedata->FindRVA( GameData::Modules::Client, "Client", "CClient_SoundEngine::LoadSoundList" ); } );
		MAKE_ASYNC( fm_pfnCClient_SoundEngine__FlushCache, [] { return gamedata->FindRVA( GameData::Modules::Client, "Client", "CClient_SoundEngine::FlushCache" ); } );

		m_pfnCClient_SoundEngine__LoadSoundList = fm_pfnCClient_SoundEngine__LoadSoundList.get();
		m_pfnCClient_SoundEngine__FlushCache = fm_pfnCClient_SoundEngine__FlushCache.get();

		if ( m_pfnCClient_SoundEngine__LoadSoundList == NULL )
			return false;
		if ( m_pfnCClient_SoundEngine__FlushCache == NULL )
			return false;
	}
	else
	{
	#ifdef WIN32
		m_pfnCClient_SoundEngine__LoadSoundList = MemoryUtils()->FindPattern( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CClient_SoundEngine__LoadSoundList );
		FEATURE_CHECK_SYMBOL_PATTERN( m_pfnCClient_SoundEngine__LoadSoundList, "CClient_SoundEngine::LoadSoundList" );

		int patternIndex;
		DEFINE_PATTERNS_FUTURE( fCClient_SoundEngine__FlushCache );
		MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CClient_SoundEngine__FlushCache, fCClient_SoundEngine__FlushCache );
		m_pfnCClient_SoundEngine__FlushCache = MemoryUtils()->GetPatternFutureValue( fCClient_SoundEngine__FlushCache, &patternIndex );
		FEATURE_CHECK_SYMBOL_PATTERNS( m_pfnCClient_SoundEngine__FlushCache,
									   "CClient_SoundEngine::FlushCache",
									   FeaturesGameData::Patterns::Client::CClient_SoundEngine__FlushCache,
									   patternIndex );
	#else
		return false;
	#endif
	}

	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnCClient_SoundEngine__LoadSoundList", m_pfnCClient_SoundEngine__LoadSoundList, GameData::Modules::Client );
	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnCClient_SoundEngine__FlushCache", m_pfnCClient_SoundEngine__FlushCache, GameData::Modules::Client );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSoundcache::PostLoad( void )
{
	m_hNetMsgHook_ResourceList = gamehooks->HookNetworkMessage( SVC_RESOURCELIST,
																HOOKED_NetMsgHook_ResourceList,
																&ORIG_NetMsgHook_ResourceList );

	m_hCClient_SoundEngine__LoadSoundList = Detours()->DetourFunction( m_pfnCClient_SoundEngine__LoadSoundList,
																	   HOOKED_CClient_SoundEngine__LoadSoundList,
																	   GET_FUNC_PTR( ORIG_CClient_SoundEngine__LoadSoundList ) );

	m_hCClient_SoundEngine__FlushCache = Detours()->DetourFunction( m_pfnCClient_SoundEngine__FlushCache,
																	HOOKED_CClient_SoundEngine__FlushCache,
																	GET_FUNC_PTR( ORIG_CClient_SoundEngine__FlushCache ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSoundcache::Unload( void )
{
	gamehooks->UnhookNetworkMessage( m_hNetMsgHook_ResourceList );
	Detours()->RemoveDetour( m_hCClient_SoundEngine__LoadSoundList );
	Detours()->RemoveDetour( m_hCClient_SoundEngine__FlushCache );
}

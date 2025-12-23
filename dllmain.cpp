// SvenInt (c) Sw1ft
// dllmain.cpp

#ifdef _WIN32
#ifndef _DEBUG
#ifdef _CPPRTTI
#error "RTTI enabled"
#endif
#endif
#else
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#endif

#include "stdafx.h"
#include "svenint.h"

#include "modules/config.h"
#include "modules/client.h"
#include "modules/hooks.h"
#include "modules/menu.h"
#include "modules/opengl.h"
#include "modules/server.h"
#include "modules/scripts.h"

#include "features/base_feature.h"

#include "game/dbg.h"
#include "utils/detours.h"
#include "utils/prof.h"

//-----------------------------------------------------------------------------
// Macro Definitions
//-----------------------------------------------------------------------------

#define SHOW_DEBUG_CONSOLE ( 1 )
#define CHECK_FOR_UPDATE ( 1 )

//-----------------------------------------------------------------------------
// Load SvenInt
//-----------------------------------------------------------------------------

bool LoadSvenInt( void )
{
#if DUMP_FILE_OFFSET
    InitPrintDumpFileOffset();
#endif

    Detours()->Init();

    Msg( "Loading SvenInt v" SVENINT_VERSION_STRING "\n" );

    PROF_MEASURE( loadTime );
    loadTime.Start();

    if ( !Globals::gamedata->Init() )
    {
        Warning( "[SvenInt] Failed to get gamedata\n" );
        Warning2( "[SvenInt] Make sure you have %s file \"./" SVENINT_FOLDER_NAME "/gamedata.txt\" (https://github.com/sw1ft747/svenint/blob/main/resource/svenint/gamedata.txt)\n", "downloaded" );
    #ifdef WIN32
        Warning2( "[SvenInt] Fall back using statically generated patterns and offsets\n" );
    #else
        return false;
    #endif
    }

    if ( !GameData::GetGameModules() )
    {
        Warning( "[SvenInt] Failed to get required game libraries\n" );
        return false;
    }

    if ( !GameData::GetGameInterfaceFactories() )
    {
        Warning( "[SvenInt] Failed to get required game interface factories\n" );
        return false;
    }

    if ( !GameData::GetGameInterfaces() )
    {
        Warning( "[SvenInt] Failed to get required game interfaces\n" );
        return false;
    }

    if ( !GameData::FindGameSymbols() )
    {
        Warning( "[SvenInt] Failed to get required game symbols\n" );
        Warning2( "[SvenInt] Make sure you have %s file \"./" SVENINT_FOLDER_NAME "/gamedata.txt\" (https://github.com/sw1ft747/svenint/blob/main/resource/svenint/gamedata.txt)\n", "updated" );
    #ifdef WIN32
        Warning2( "[SvenInt] Try your luck on setting parameter \"PreferRVA\" to 0 in the file \"./" SVENINT_FOLDER_NAME "/gamedata.txt\"\n" );
    #endif
        return false;
    }

    Globals::cvar = CVar();
    if ( !Globals::cvar->Init() )
    {
        Warning( "[SvenInt] Failed to initialize CVar system\n" );
        return false;
    }

    // Grab screen size
    Globals::gameutils->Init();

    // Load features
    bool bFeaturesOK = LoadFeatures();

    // Auto pause any detour that attaches
    Detours()->AutoPauseDetours( true );
    // Attach module detours but paused
    Modules::hooks->AttachDetours();
    // Suspend all threads
    Detours()->BeginGlobalAttach();

    Modules::config->Init();
    Modules::client->Init();
    Modules::server->Init();
    Modules::hooks->Init();
    Modules::menu->Init();
    Modules::scripts->Init();

    // Post load features & attach their detours but paused
    PostLoadFeatures();

    // Remove auto pause & unpause all attached detours
    Detours()->AutoPauseDetours( false );
    Detours()->UnpauseAllDetours();

    // Print state
    ConColorMsg( { bFeaturesOK ? 0 : 255, 255, 90, 255 }, bFeaturesOK ? "[SvenInt] Successfully loaded\n" : "[SvenInt] Loaded with limited features\n" );

    // Allow print to the game console & dump queued messages
    AllowMsgPrint( true );
    PrintQueudMessages();

    // Resume suspended threads
    Detours()->EndGlobalAttach();

    Msg( "Loaded SvenInt in %.2f s\n", (float)PROF_TIME_SEC( loadTime.Elapsed() ) );

#if DUMP_FILE_OFFSET
    ShutdownPrintDumpFileOffset();
#endif

    return true;
}

//-----------------------------------------------------------------------------
// Unload SvenInt
//-----------------------------------------------------------------------------

void UnloadSvenInt( void )
{
    Msg( "Unloading SvenInt...\n" );

    // Suspend all threads
    Detours()->BeginGlobalAttach();

    // Pause all attached detours
    Detours()->PauseAllDetours();

    // Deattach all detours
    UnloadFeatures();
    Modules::hooks->DeattachDetours();

    Modules::scripts->Shutdown();
    Modules::menu->Shutdown();
    Modules::hooks->Shutdown();
    Modules::server->Shutdown();
    Modules::client->Shutdown();
    Modules::config->Shutdown();
    Modules::opengl->Shutdown();

    Globals::cvar->Shutdown();

    Msg( "Successfully unloaded SvenInt\n" );

    // Resume suspended threads
    Detours()->EndGlobalAttach();

    Globals::gamedata->Shutdown();

    MemCheckLeaks();
}

//-----------------------------------------------------------------------------
// Debug console
//-----------------------------------------------------------------------------

#ifdef WIN32
void CreateConsole( FILE **pFile )
{
    if ( pFile == NULL || *pFile != NULL )
        return;

    AllocConsole();
    freopen_s( pFile, "CONOUT$", "w", stdout );
}

void CloseConsole( FILE **pFile )
{
    if ( pFile == NULL || *pFile == NULL )
        return;

    fclose( *pFile );
    FreeConsole();
    *pFile = NULL;
}
#endif

//-----------------------------------------------------------------------------
// Main thread
//-----------------------------------------------------------------------------

#ifdef WIN32
DWORD WINAPI MainThread( HMODULE hModule )
#else
void *MainThread( void *arg )
#endif
{
    Globals::commandline = CommandLine();
#ifdef WIN32
    Globals::commandline->CreateCmdLine( GetCommandLine() );

    // Allocate debug console
#if SHOW_DEBUG_CONSOLE
    const bool bNoConsole = Globals::commandline->HasParm( "-sint_noconsole" );
    const bool bKeepConsole = Globals::commandline->HasParm( "-sint_keepconsole" );

    if ( !bNoConsole )
        CreateConsole( (FILE **)&gpDbgConsoleFile );
#endif
#else
    int    argc;
    char **argv;

    char **get_argv( int *const argcptr );
    argv = get_argv( &argc );
    if ( argv != NULL )
    {
        Globals::commandline->CreateCmdLine( argc, argv );
        free( argv );
    }

    gpDbgConsoleFile = (void *)fopen( "svenint.log", "w" );
#endif

    // Load SvenInt
    if ( LoadSvenInt() )
    {
        void CheckForUpdate( void );

    #ifdef WIN32
        Msg( "Hold 'Right CTRL' to unload SvenInt.\n" );

    #if SHOW_DEBUG_CONSOLE
        if ( !bNoConsole && !bKeepConsole )
        {
            DevMsg( "Closing the debug console in 3 seconds...\n" );

            Sleep( 3000 );
            CloseConsole( (FILE **)&gpDbgConsoleFile );
        }
    #endif

    #if CHECK_FOR_UPDATE
        CheckForUpdate();
    #endif

        // Don't unload SvenInt until 'RCtrl' will be pressed
        while ( !GetAsyncKeyState( VK_RCONTROL ) ) { Sleep( 200 ); };

        UnloadSvenInt();
    #else
    #if CHECK_FOR_UPDATE
        CheckForUpdate();
    #endif
        fclose( (FILE *)gpDbgConsoleFile );
        gpDbgConsoleFile = NULL;

        *(bool *)arg = true;
        pthread_exit( NULL );
    #endif
    }
    else
    {
        // If possible, print all queued messages into the game's console
        // HACK: suspend all threads
        Detours()->BeginGlobalAttach();

        AllowMsgPrint( true );
        PrintQueudMessages();

        Detours()->EndGlobalAttach();

#ifdef WIN32
#if SHOW_DEBUG_CONSOLE
        if ( !bNoConsole )
        {
            // Sleep( 5000 );
            DevMsg( "Press any key to close the console\n" );
            getchar();
        }
    }

    if ( !bNoConsole )
        CloseConsole( (FILE **)&gpDbgConsoleFile );
#else
    }
#endif

    FreeLibraryAndExitThread( hModule, EXIT_SUCCESS );
    return EXIT_SUCCESS;
#else
    }

    fclose( (FILE *)gpDbgConsoleFile );
    gpDbgConsoleFile = NULL;

    *(bool *)arg = false;
    pthread_exit( NULL );
#endif
}

//-----------------------------------------------------------------------------
// Check for updates
//-----------------------------------------------------------------------------

#if CHECK_FOR_UPDATE
typedef void CURL;
typedef int CURLcode;

#define CURLE_OK 0
#define CURLOPT_URL 10002
#define CURLOPT_WRITEFUNCTION 20011
#define CURLOPT_WRITEDATA 10001
#define CURLOPT_TIMEOUT 13

typedef CURL *( *curl_easy_init_t )( void );
typedef CURLcode( *curl_easy_setopt_t )( CURL *, int, ... );
typedef CURLcode( *curl_easy_perform_t )( CURL * );
typedef const char *( *curl_easy_strerror_t )( CURLcode );
typedef void ( *curl_easy_cleanup_t )( CURL * );

struct UpdateDownloadBuffer
{
    char *memory;
    size_t size;
};

static size_t UpdateDownloadBufferWriteCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    size_t realsize = size * nmemb;
    UpdateDownloadBuffer *mem = (UpdateDownloadBuffer *)userp;

    char *ptr = (char *)MemRealloc( mem->memory, mem->size + realsize + 1 );
    if ( ptr == NULL )
        return 0;

    mem->memory = ptr;
    memcpy( &( mem->memory[ mem->size ] ), contents, realsize );
    mem->size += realsize;
    mem->memory[ mem->size ] = 0;

    return realsize;
}

static void Update_CheckVersion( const char *data )
{
    int iMajorVer = 0, iMinorVer = 0, iPatchVer = 0;

    const char *pszMajorVer = strstr( data, "SVENINT_MAJOR_VERSION" );
    const char *pszMinorVer = strstr( data, "SVENINT_MINOR_VERSION" );
    const char *pszPatchVer = strstr( data, "SVENINT_PATCH_VERSION" );

#pragma warning( push )
#pragma warning( disable : 6031 )

    if ( pszMajorVer != NULL )
        sscanf( pszMajorVer, "SVENINT_MAJOR_VERSION %d", &iMajorVer );
    if ( pszMinorVer != NULL )
        sscanf( pszMinorVer, "SVENINT_MINOR_VERSION %d", &iMinorVer );
    if ( pszPatchVer != NULL )
        sscanf( pszPatchVer, "SVENINT_PATCH_VERSION %d", &iPatchVer );

#pragma warning( pop )

    if ( SVENINT_VERSION() < SVENINT_VERSION_CHECK( iMajorVer, iMinorVer, iPatchVer ) )
    {
        Warning2( "A newer version of SvenInt is available: %d.%d.%d (Current: %d.%d.%d)\n",
                  iMajorVer, iMinorVer, iPatchVer, SVENINT_MAJOR_VERSION, SVENINT_MINOR_VERSION, SVENINT_PATCH_VERSION );
        Warning2( "Consider updating it: https://github.com/sw1ft747/svenint/releases\n" );

        /*
        if ( Globals::cls->state == ca_active )
        {
            Globals::gameutils->PrintChatText( "A newer version of SvenInt is available: %d.%d.%d (Current: %d.%d.%d)\n",
                                               iMajorVer, iMinorVer, iPatchVer, SVENINT_MAJOR_VERSION, SVENINT_MINOR_VERSION, SVENINT_PATCH_VERSION );
            Globals::gameutils->PrintChatText( "Consider updating it: https://github.com/sw1ft747/svenint/releases\n" );
        }
        */
    }
    else
    {
        Msg( "SvenInt is up to date.\n" );
    }
}

void CheckForUpdate( void )
{
    if ( Globals::commandline->HasParm( "-sint_nocheckupdates" ) )
        return;

#ifdef WIN32
    module_t hCurl = MemoryUtils()->GetModule( "libcurl.dll" );
#else
    module_t hCurl = MemoryUtils()->GetModule( "libcurl.so.4" );
#endif
    if ( hCurl == NULL )
        return;

    curl_easy_init_t curl_easy_init = (curl_easy_init_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_init" );
    curl_easy_setopt_t curl_easy_setopt = (curl_easy_setopt_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_setopt" );
    curl_easy_perform_t curl_easy_perform = (curl_easy_perform_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_perform" );
    curl_easy_strerror_t curl_easy_strerror = (curl_easy_strerror_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_strerror" );
    curl_easy_cleanup_t curl_easy_cleanup = (curl_easy_cleanup_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_cleanup" );

    if ( curl_easy_init == NULL || curl_easy_setopt == NULL ||
         curl_easy_perform == NULL || curl_easy_strerror == NULL ||
         curl_easy_cleanup == NULL )
    {
        return;
    }

    CURL *curl;
    CURLcode res;
    UpdateDownloadBuffer chunk = { NULL, 0 };
    chunk.memory = (char *)MemAlloc( 1 );

    curl = curl_easy_init();
    if ( curl != NULL )
    {
        curl_easy_setopt( curl, CURLOPT_URL, "https://raw.githubusercontent.com/sw1ft747/svenint/refs/heads/main/svenint.h" );
        curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, UpdateDownloadBufferWriteCallback );
        curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk );
        curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10L );

        res = curl_easy_perform( curl );

        if ( res == CURLE_OK )
        {
            Update_CheckVersion( chunk.memory );
        }
        else
        {
            Warning2( "Failed to check for update. Request timed out or failed: %s\n", curl_easy_strerror( res ) );
        }

        curl_easy_cleanup( curl );
        MemFree( chunk.memory );
    }
}
#endif

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

#ifdef WIN32
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE)MainThread, hModule, NULL, NULL );
        if ( hThread != NULL )
            CloseHandle( hThread );

        return TRUE;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return FALSE;
}
#else
static bool loaded = false;
static pthread_t tid;

int __attribute__((constructor)) module_load( void )
{
    pthread_create( &tid, NULL, MainThread, &loaded );
    //loaded = ( MainThread( NULL ) != NULL );

    return 0;
}

void __attribute__((destructor)) module_unload( void )
{
    pthread_join( tid, NULL );
    if ( loaded )
    {
        loaded = false;
        // UnloadSvenInt();
    }
}

//-----------------------------------------------------------------------------
// Get command line arguments
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

// https://stackoverflow.com/questions/28020711/how-to-pass-argument-to-constructor-on-library-load
char **get_argv( int *const argcptr )
{
    char **argv;
    char *data = NULL;
    size_t  size = 0;    /* Allocated to data */
    size_t  used = 0;
    size_t  argc, i;
    ssize_t bytes;
    int     fd;

    if ( argcptr )
        *argcptr = 0;

    do
    {
        fd = open( "/proc/self/cmdline", O_RDONLY | O_NOCTTY );
    } while ( fd == -1 && errno == EINTR );
    if ( fd == -1 )
        return NULL;

    while ( 1 )
    {

        if ( used >= size )
        {
            char *old_data = data;
            size = ( used | 4095 ) + 4096;
            data = (char *)realloc( data, size + 1 );
            if ( data == NULL )
            {
                free( old_data );
                close( fd );
                errno = ENOMEM;
                return NULL;
            }
        }

        do
        {
            bytes = read( fd, data + used, size - used );
        } while ( bytes == (ssize_t)-1 && errno == EINTR );
        if ( bytes < (ssize_t)0 )
        {
            free( data );
            close( fd );
            errno = EIO;
            return NULL;

        }
        else
            if ( bytes == (ssize_t)0 )
                break;

            else
                used += bytes;
    }

    if ( close( fd ) )
    {
        free( data );
        errno = EIO;
        return NULL;
    }

    /* Let's be safe and overallocate one pointer here. */
    argc = 1;
    for ( i = 0; i < used; i++ )
        if ( data[ i ] == '\0' )
            argc++;

    /* Reallocate to accommodate both pointers and data. */
    argv = (char **)realloc( data, ( argc + 1 ) * sizeof( char * ) + used + 1 );
    if ( argv == NULL )
    {
        free( data );
        errno = ENOMEM;
        return NULL;
    }
    data = (char *)( argv + argc + 1 );
    memmove( data, argv, used );

    /* In case the input lacked a trailing NUL byte. */
    data[ used ] = '\0';

    /* Assign the pointers. */
    argv[ 0 ] = data;
    argc = 0;
    for ( i = 0; i < used; i++ )
        if ( data[ i ] == '\0' )
            argv[ ++argc ] = data + i + 1;
    /* Final pointer points to past data. Make it end the array. */
    argv[ argc ] = NULL;

    if ( argcptr )
        *argcptr = (int)argc;

    return argv;
}

#endif

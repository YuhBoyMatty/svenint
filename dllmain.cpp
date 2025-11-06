// SvenInt (c) Sw1ft
// dllmain.cpp

#ifndef _DEBUG
#ifdef _CPPRTTI
#error "RTTI enabled"
#endif
#endif

#include "stdafx.h"
#include "svenint.h"
#include <Windows.h>

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

//-----------------------------------------------------------------------------
// Macro Definitions
//-----------------------------------------------------------------------------

#define SHOW_DEBUG_CONSOLE ( 1 )

//-----------------------------------------------------------------------------
// Load SvenInt
//-----------------------------------------------------------------------------

bool LoadSvenInt( void )
{
    Detours()->Init();

    Msg( "Loading SvenInt v" SVENINT_VERSION "\n" );

    if ( !GameData::GetGameDLLModules() )
    {
        Warning( "[SvenInt] Failed to get required game DLL modules\n" );
        return false;
    }

    if ( !GameData::GetGameDLLInterfaceFactories() )
    {
        Warning( "[SvenInt] Failed to get required game DLL interface factories\n" );
        return false;
    }

    if ( !GameData::GetGameDLLInterfaces() )
    {
        Warning( "[SvenInt] Failed to get required game DLL interfaces\n" );
        return false;
    }
    
    if ( !GameData::FindGameSymbols() )
    {
        Warning( "[SvenInt] Failed to get required game DLL symbols\n" );
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
}

//-----------------------------------------------------------------------------
// Debug console
//-----------------------------------------------------------------------------

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

//-----------------------------------------------------------------------------
// Main thread
//-----------------------------------------------------------------------------

DWORD WINAPI MainThread( HMODULE hModule )
{
    Globals::commandline = CommandLine();
    Globals::commandline->CreateCmdLine( GetCommandLine() );

    // Allocate debug console
#if SHOW_DEBUG_CONSOLE
    const bool bNoConsole = Globals::commandline->HasParm( "-sint_noconsole" );
    const bool bKeepConsole = Globals::commandline->HasParm( "-sint_keepconsole" );

    if ( !bNoConsole )
        CreateConsole( (FILE **)&gpDbgConsoleFile );
#endif

    // Load SvenInt
    if ( LoadSvenInt() )
    {
        Msg( "Hold 'Right CTRL' to unload SvenInt.\n" );

    #if SHOW_DEBUG_CONSOLE
        if ( !bNoConsole && !bKeepConsole )
        {
            DevMsg( "Closing the debug console in 5 seconds...\n" );

            Sleep( 5000 );
            CloseConsole( (FILE **)&gpDbgConsoleFile );
        }
    #endif

        // Don't unload SvenInt until 'RCtrl' will be pressed
        while ( !GetAsyncKeyState( VK_RCONTROL ) ) { Sleep( 200 ); };

        UnloadSvenInt();
    }
    else
    {
        // If possible, print all queued messages into the game's console
        // HACK: suspend all threads
        Detours()->BeginGlobalAttach();

        AllowMsgPrint( true );
        PrintQueudMessages();

        Detours()->EndGlobalAttach();

#if SHOW_DEBUG_CONSOLE
        if ( !bNoConsole )
            Sleep( 5000 );
    }

    if ( !bNoConsole )
        CloseConsole( (FILE **)&gpDbgConsoleFile );
#else
    }
#endif

    FreeLibraryAndExitThread( hModule, EXIT_SUCCESS );
    return EXIT_SUCCESS;
}

//-----------------------------------------------------------------------------
// Entry point
//-----------------------------------------------------------------------------

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
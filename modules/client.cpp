// SvenInt (c) Sw1ft
// client.cpp

#include "stdafx.h"
#include "client.h"
#include "features/base_feature.h"

#include <regex>

namespace Modules { static CClientModule clientModule; CClientModule *client = &clientModule; }

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_prof( "sc_prof", "1", FCVAR_EXTDLL, "Enable profiling" );
ConVar sc_disable_monster_info( "sc_disable_monster_info", "0", FCVAR_EXTDLL, "Disables HUD text message about monster's info" );
ConVar sc_disable_sprays( "sc_disable_sprays", "0", FCVAR_EXTDLL, "Disables sprays of players" );
ConVar sc_debug_new_line_height( "sc_debug_new_line_height", "20", FCVAR_EXTDLL, "Height for each line when print anything" );
ConVar sc_debug_show_weapondata( "sc_debug_show_weapondata", "0", FCVAR_EXTDLL, "Shows on the screen current weapon's data vars" );
ConVar sc_debug_show_inventory( "sc_debug_show_inventory", "0", FCVAR_EXTDLL, "Shows on the screen weapon's inventory" );
ConVar sc_debug_show_refparams( "sc_debug_show_refparams", "0", FCVAR_EXTDLL, "Shows on the screen ref params" );
ConVar sc_debug_show_movevars( "sc_debug_show_movevars", "0", FCVAR_EXTDLL, "Shows on the screen movement vars" );
ConVar sc_debug_show_clientdata( "sc_debug_show_clientdata", "0", FCVAR_EXTDLL, "Shows on the screen client's data vars" );
ConVar sc_debug_show_clientent( "sc_debug_show_clientent", "0", FCVAR_EXTDLL, "Shows on the screen client's entity vars, value is entity's index to watch for" );
ConVar sc_debug_show_entitystate( "sc_debug_show_entitystate", "0", FCVAR_EXTDLL, "Shows on the screen client's entity state vars" );
ConVar sc_debug_show_playerinfo( "sc_debug_show_playerinfo", "0", FCVAR_EXTDLL, "Shows on the screen player info, value is player's index to watch for" );
ConVar sc_debug_show_playermove( "sc_debug_show_playermove", "0", FCVAR_EXTDLL, "Shows on the screen player move vars" );
ConVar sc_debug_show_prof( "sc_debug_show_prof", "0", FCVAR_EXTDLL, "Shows on the screen profiling" );

CON_COMMAND( sc_print_cvars, "Print registered ConVars / ConCommands" )
{
	const int argCount = args.ArgC();
	bool bPrintUsage = false;

	if ( argCount > 1 )
	{
		const char *pszArgument = args[ 1 ];

		if ( !stricmp( pszArgument, "all" ) )
		{
			if ( argCount > 2 )
			{
				if ( !stricmp( args[ 2 ], "?" ) )
				{
					if ( argCount > 3 )
						Globals::cvar->PrintCvars( 0, args[ 3 ] );
					else
						ConMsg( "Usage:  sc_print_cvars all ? <prefix>\n" );
				}
			}
			else
			{
				Globals::cvar->PrintCvars( 0, NULL );
				ConMsg( "For syntax:  sc_print_cvars all ? <prefix>\n" );
			}
		}
		else if ( !stricmp( pszArgument, "cvar" ) )
		{
			if ( argCount > 2 )
			{
				if ( !stricmp( args[ 2 ], "?" ) )
				{
					if ( argCount > 3 )
						Globals::cvar->PrintCvars( 1, args[ 3 ] );
					else
						ConMsg( "Usage:  sc_print_cvars cvar ? <prefix>\n" );
				}
			}
			else
			{
				Globals::cvar->PrintCvars( 1, NULL );
				ConMsg( "For syntax:  sc_print_cvars cvar ? <prefix>\n" );
			}
		}
		else if ( !stricmp( pszArgument, "cmd" ) )
		{
			if ( argCount > 2 )
			{
				if ( !stricmp( args[ 2 ], "?" ) )
				{
					if ( argCount > 3 )
						Globals::cvar->PrintCvars( 2, args[ 3 ] );
					else
						ConMsg( "Usage:  sc_print_cvars cmd ? <prefix>\n" );
				}
			}
			else
			{
				Globals::cvar->PrintCvars( 2, NULL );
				ConMsg( "For syntax:  sc_print_cvars cmd ? <prefix>\n" );
			}
		}
		else
		{
			bPrintUsage = true;
		}
	}
	else
	{
		bPrintUsage = true;
	}

	if ( bPrintUsage )
	{
		ConMsg( "* sc_print_cvars all  - Print all registered convars/concommands\n" );
		ConMsg( "* sc_print_cvars cvar - Print only convars\n" );
		ConMsg( "* sc_print_cvars cmd  - Print only concommands\n" );
	}
}

CON_COMMAND( sc_print_features, "Print SvenInt features" )
{
	CBaseFeature::PrintFeatures();
}

CON_COMMAND( sc_dump_disasm, "Disassemble & dump into the console the given memory address" )
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Usage:  sc_dump_disasm <hex address>\n" );
		return;
	}

	ud_t inst;
	int address = strtol( args[ 1 ], NULL, 16 );

	MemoryUtils()->InitDisasm( &inst, (void *)address, 32, 15 );

	if ( MemoryUtils()->Disassemble( &inst ) )
		MemoryUtils()->DumpOperands( &inst );
}

CON_COMMAND( sc_dump_disasm_bytes, "Disassemble & dump into the console the given sequence of bytes" )
{
	if ( args.ArgC() < 2 )
	{
		Msg( "Usage:  sc_dump_disasm_bytes <any sequence of hex bytes separated by a spacebar>\n" );
		return;
	}

	int bytes = args.ArgC() - 1;
	uint8_t *bytesSequence = (uint8_t *)malloc( bytes );

	for ( int i = 1; i < args.ArgC(); i++ )
		bytesSequence[ i - 1 ] = (uint8_t)( strtoul( args[ i ], NULL, 16 ) & 0xFF );

	ud_t inst;
	MemoryUtils()->InitDisasm( &inst, bytesSequence, 32, 15 );

	if ( MemoryUtils()->Disassemble( &inst ) )
		MemoryUtils()->DumpOperands( &inst );

	free( bytesSequence );
}

CON_COMMAND( sc_dump_interfaces, "Dump interfaces from the specified module name" )
{
	if ( args.ArgC() > 1 )
	{
		InterfaceReg *pInterfaceRegs = NULL;
		module_t hModule = MemoryUtils()->GetModule( args[ 1 ] );
		if ( hModule == NULL )
		{
			Msg( "sc_dump_interfaces: no such module\n" );
			return;
		}

		void *pfnCreateInterfaceFactory = MemoryUtils()->GetProcAddress( hModule, "CreateInterface" );
		if ( pfnCreateInterfaceFactory == NULL )
		{
			Msg( "sc_dump_interfaces: CreateInterface not found\n" );
			return;
		}

		ud_t inst;
		MemoryUtils()->InitDisasm( &inst, pfnCreateInterfaceFactory, 32, 16 );
		while ( MemoryUtils()->Disassemble( &inst ) )
		{
			if ( inst.mnemonic == UD_Imov && inst.operand[ 0 ].type == UD_OP_REG &&
				 inst.operand[ 0 ].base == UD_R_ESI && inst.operand[ 1 ].type == UD_OP_MEM )
			{
				pInterfaceRegs = *reinterpret_cast<InterfaceReg **>( inst.operand[ 1 ].lval.udword );
				break;
			}
		}

		if ( pInterfaceRegs == NULL )
		{
			Msg( "sc_dump_interfaces: unable to get InterfaceReg\n" );
			return;
		}

		Msg( "List of interfaces:\n" );

		for ( int i = 1; pInterfaceRegs != NULL; i++ )
		{
			void *pInterface = pInterfaceRegs->m_CreateFn();
			const char *pszName = pInterfaceRegs->m_pName;

			Msg( "%d. %s = 0x%X\n", i, pszName, pInterface );

			pInterfaceRegs = pInterfaceRegs->m_pNext;
		}
	}
	else
	{
		ConMsg( "Usage:  sc_dump_interfaces <modulename>\n" );
	}
}

CON_COMMAND( sc_dump_netmsg, "Dump network messages" )
{
	if ( Globals::g_NetworkMessages == NULL )
		return;

	Msg( "======= Network Messages =======\n" );

	netmsg_t *pNetMsg = Globals::g_NetworkMessages;
	for ( int i = 0; stricmp( pNetMsg[ i ].name, "End of List" ) != 0; i++ )
	{
		Msg( "name = %s\n", pNetMsg[ i ].name );
		Msg( "function = 0x%X\n", pNetMsg[ i ].function );
		Msg( "index = %d\n", i );
		Msg( "================================\n" );
	}
}

CON_COMMAND( sc_dump_usermsg, "Dump network messages" )
{
	if ( Globals::g_pClientUserMsgs == NULL )
		return;

	Msg( "======= User Messages =======\n" );

	int i = 0;
	for ( usermsg_t *pUserMsg = *Globals::g_pClientUserMsgs; pUserMsg != NULL; pUserMsg = pUserMsg->next, i++ )
	{
		Msg( "msgid = %d (%d)\n", pUserMsg->msgid, i );
		Msg( "size = %d\n", pUserMsg->size );
		Msg( "name = %s\n", pUserMsg->name );
		Msg( "function = 0x%X\n", pUserMsg->function );
		Msg( "=============================\n" );
	}
}

CON_COMMAND( help, "Find help about a ConVar / ConCommand registered through SvenInt." )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszName = args[ 1 ];
		const ConCommandBase *var = Globals::cvar->FindCommandBase( pszName );

		if ( var )
			ConVar_PrintDescription( var );
		else
			ConMsg( "help:  no cvar or command named \"%s\"\n", pszName );
	}
	else
	{
		ConMsg( "Usage:  help <cvarname>\n" );
	}
}

CON_COMMAND( toggle, "Toggle between values" )
{
	const int argCount = args.ArgC();

	if ( argCount > 1 )
	{
		const char *pszCvar = args[ 1 ];
		cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszCvar );

		if ( pCvar != NULL )
		{
			if ( argCount == 2 )
			{
				bool bValue = static_cast<bool>( pCvar->value );
				Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), float( !bValue ) );
			}
			else
			{
				int i;
				for ( i = 2; i < argCount; i++ )
				{
					if ( !strcmp( pCvar->string, args[ i ] ) )
						break;
				}

				i++;

				if ( i >= argCount )
					i = 2;

				Globals::cvar->SetValue( pCvar, args[ i ] );
				//Globals::cl_enginefuncs->Cvar_Set( const_cast<char *>( pszCvar ), const_cast<char *>( args[ i ] ) );
			}
		}
	}
	else
	{
		ConMsg( "Usage:  toggle <cvarname> <value #1> <value #2> <value #3>..\n" );
	}
}

CON_COMMAND( incrementvar, "Increment a cvar" )
{
	if ( args.ArgC() >= 5 )
	{
		const char *pszCvar = args[ 1 ];
		cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszCvar );

		if ( pCvar )
		{
			float currentValue = pCvar->value;
			float startValue = std::stof( args[ 2 ] );
			float endValue = std::stof( args[ 3 ] );
			float delta = std::stof( args[ 4 ] );
			float newValue = currentValue + delta;

			if ( newValue > endValue )
				newValue = startValue;
			else if ( newValue < startValue )
				newValue = endValue;

			Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), newValue );
		}
	}
	else
	{
		ConMsg( "Usage:  incrementvar <cvarname> <minvalue> <maxvalue> <delta>\n" );
	}
}

CON_COMMAND( multvar, "Multiply a cvar" )
{
	if ( args.ArgC() >= 5 )
	{
		const char *pszCvar = args[ 1 ];
		cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszCvar );

		if ( pCvar )
		{
			float currentValue = pCvar->value;
			float startValue = std::stof( args[ 2 ] );
			float endValue = std::stof( args[ 3 ] );
			float factor = std::stof( args[ 4 ] );
			float newValue = currentValue * factor;

			if ( newValue > endValue )
				newValue = startValue;
			else if ( newValue < startValue )
				newValue = endValue;

			Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), newValue );
		}
	}
	else
	{
		ConMsg( "Usage:  multvar <cvarname> <minvalue> <maxvalue> <factor>\n" );
	}
}


CON_COMMAND( append, "Append a command into the beginning of command queue\n Similar to how \"special\" appends \"_special\"\n" )
{
	if ( args.ArgC() < 2 )
	{
		ConMsg( "Usage:  append <command>\n" );
		return;
	}

	Globals::cl_enginefuncs->pfnClientCmd( args[ 1 ] );
}

CON_COMMAND( sc_test, "Retrieve an entity's info" )
{
	if ( args.ArgC() > 1 )
	{
		int index = atoi( args[ 1 ] );

		cl_entity_s *pEntity = Globals::cl_enginefuncs->GetEntityByIndex( index );

		if ( pEntity )
		{
			Msg( "Entity Pointer: %X\n", pEntity );

			if ( pEntity->player )
			{
				Msg( "Player Info Pointer: %X\n", Globals::enginestudio->PlayerInfo( index - 1 ) );

				hud_player_info_t playerInfo;
				memset( &playerInfo, 0, sizeof( hud_player_info_s ) );

				Globals::cl_enginefuncs->pfnGetPlayerInfo( index, &playerInfo );

				if ( playerInfo.name && playerInfo.model && *playerInfo.model )
					Msg( "Model: %s\n", playerInfo.model );

				Msg( "Top Color: %d\n", playerInfo.topcolor );
				Msg( "Bottom Color: %d\n", playerInfo.bottomcolor );
			}
			else if ( pEntity->model && pEntity->model->name )
			{
				Msg( "Model: %s\n", pEntity->model->name );
			}
		}
	}
	else
	{
		ConMsg( "Usage:  sc_test <entindex>\n" );
	}
}

CON_COMMAND( sc_print_steamids, "Print Steam64 IDs of all players on the server" )
{
	for ( int i = 1; i <= Globals::cl_enginefuncs->GetMaxClients(); i++ )
	{
		player_info_t *pPlayerInfo = Globals::enginestudio->PlayerInfo( i - 1 );

		if ( pPlayerInfo != NULL && pPlayerInfo->name && pPlayerInfo->name[ 0 ] )
		{
			Msg( "%d. %s - %llu\n", i, pPlayerInfo->name, pPlayerInfo->m_nSteamID );
		}
	}
}

CON_COMMAND( sc_steamid_to_steam64id, "Converts Steam ID to Steam64 ID, apostrophes \"\" are required" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszSteamID = args[ 1 ];

		std::cmatch match;
		std::regex regex_steamid( "^STEAM_[0-5]:([01]):([0-9]+)$" );

		if ( std::regex_search( pszSteamID, match, regex_steamid ) )
		{
			uint64_t steamID = 76561197960265728; // base num

			const uint64_t v1 = atoll( match[ 1 ].str().c_str() );
			const uint64_t v2 = atoll( match[ 2 ].str().c_str() );

			steamID += v1 + v2 * 2;

			Msg( "Steam64 ID: %llu\n", steamID );
			Msg( "https://steamcommunity.com/profiles/%llu\n", steamID );
		}
		else
		{
			Msg( "Invalid SteamID, did you forget to write SteamID with apostrophes? ( \"\" )\n" );
		}
	}
	else
	{
		ConMsg( "Usage:  sc_steamid_to_steam64id <steamid>\n" );
	}
}

CON_COMMAND( sc_load_model, "Load a given modelname" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszModelPath = args[ 1 ];
		void *pModel = Globals::engineclient->LoadClientModel( pszModelPath );
		Msg( "Model pointer: %X\n", pModel );
	}
	else
	{
		ConMsg("Usage:  sc_load_model <model path starting with models/...>\n");
	}
}

CON_COMMAND( sc_find_model, "Find models with the given name" )
{
	auto strstrci = []( const char *s, const char *p ) -> char *
	{
		if ( *p == '\0' )
			return (char *)s;
		for ( ; *s; s++ )
		{
			if ( tolower( (unsigned char)*s ) == tolower( (unsigned char)*p ) )
			{
				size_t i;
				for ( i = 1;; i++ )
				{
					if ( p[ i ] == '\0' )
						return (char *)s;
					if ( tolower( (unsigned char)s[ i ] ) != tolower( (unsigned char)p[ i ] ) )
						break;
				}
			}
		}

		return NULL;
	};

	if ( args.ArgC() > 1 )
	{
		const char *pszModelNameOccur = args[ 1 ];

		int count = 0;
		FileFindHandle_t hFindHandle;

		const char *pszFilename = Globals::filesystem->FindFirst( "models/player/*", &hFindHandle );

		while ( pszFilename )
		{
			if ( Globals::filesystem->FindIsDirectory( hFindHandle ) && *pszFilename != '.' )
			{
				if ( strstrci( pszFilename, pszModelNameOccur ) )
				{
					ConMsg( "%d. %s\n", ++count, pszFilename );
				}
			}

			pszFilename = Globals::filesystem->FindNext( hFindHandle );
		}

		Globals::filesystem->FindClose( hFindHandle );

		ConMsg( "Found %d models\n", count );
	}
	else
	{
		ConMsg( "Usage:  sc_find_model <modelname>\n" );
	}
}

CON_COMMAND( sc_find_model_prefix, "Find a model that starts with given name" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszModelNamePrefix = args[ 1 ];

		int count = 0;
		FileFindHandle_t hFindHandle;

		const char *pszFilename = Globals::filesystem->FindFirst( "models/player/*", &hFindHandle );

		while ( pszFilename )
		{
			if ( Globals::filesystem->FindIsDirectory( hFindHandle ) && *pszFilename != '.' )
			{
				if ( !strnicmp( pszModelNamePrefix, pszFilename, strlen( pszModelNamePrefix ) ) )
				{
					ConMsg( "%d. %s\n", ++count, pszFilename );
				}
			}

			pszFilename = Globals::filesystem->FindNext( hFindHandle );
		}

		Globals::filesystem->FindClose( hFindHandle );

		ConMsg( "Found %d models\n", count );
	}
	else
	{
		ConMsg( "Usage:  sc_find_model_prefix <modelname>\n" );
	}
}

CON_COMMAND( sc_find_cvar, "Find a cvar" )
{
	if ( args.ArgC() > 1 )
	{
		int count = 0;
		const char *pszCvar = args[ 1 ];
		cvar_t *pCvar = *Globals::cvar_vars;
		while ( pCvar != NULL )
		{
			if ( strstr( pCvar->name, pszCvar ) )
			{
				count++;
				Msg( "%d. %s\n", count, pCvar->name );
			}

			pCvar = pCvar->next;
		}

		ConMsg( "Found %d cvars\n", count );
	}
	else
	{
		ConMsg( "Usage:  sc_find_cvar <cvarname>\n" );
	}
}

CON_COMMAND( sc_find_cmd, "Find a cmd" )
{
	if ( args.ArgC() > 1 )
	{
		int count = 0;
		const char *pszCmd = args[ 1 ];
		cmd_function_t *pCmd = *Globals::cmd_functions;
		while ( pCmd != NULL )
		{
			if ( strstr( pCmd->name, pszCmd ) )
			{
				count++;
				Msg( "%d. %s\n", count, pCmd->name );
			}

			pCmd = pCmd->next;
		}

		ConMsg( "Found %d cmds\n", count );
	}
	else
	{
		ConMsg( "Usage:  sc_find_cmd <cmdname>\n" );
	}
}

CON_COMMAND( sc_drop_empty_weapon, "Drop an empty weapon from your inventory" )
{
	for ( int i = 0; i < Inventory()->GetMaxWeaponSlots(); i++ )
	{
		for ( int j = 0; j < Inventory()->GetMaxWeaponPositions(); j++ )
		{
			WEAPON *pWeapon = Inventory()->GetWeapon( i, j );

			if ( pWeapon && !Inventory()->HasAmmo( pWeapon ) )
			{
				Inventory()->DropWeapon( pWeapon );
				return;
			}
		}
	}
}

CON_COMMAND( getpos, "Prints current origin" )
{
	Vector vecOrigin;

	if ( Globals::demoplayback )
		vecOrigin = *(Vector *)Globals::refparams->simorg;
	else
		vecOrigin = *Globals::playermove->origin();

	Warning( "setpos %.6f %.6f %.6f\n", vecOrigin.x, vecOrigin.y, vecOrigin.z );
}

CON_COMMAND( getpos_exact, "Prints current origin from view angles" )
{
	Vector vecEyes;

	if ( Globals::demoplayback )
		vecEyes = *(Vector *)Globals::refparams->simorg + Globals::refparams->viewheight;
	else
		vecEyes = *Globals::playermove->origin() + *Globals::playermove->view_ofs();

	Warning( "setpos_exact %.6f %.6f %.6f\n", vecEyes.x, vecEyes.y, vecEyes.z );
}

CON_COMMAND( getang, "Prints current view angles" )
{
	Vector va;
	Globals::cl_enginefuncs->GetViewAngles( va );

	NormalizeAngles( va );

	Warning( "setang %.6f %.6f %.6f\n", va.x, va.y, va.z );
}

CON_COMMAND( setang, "Sets view angles" )
{
	if ( args.ArgC() >= 2 )
	{
		Vector va;
		float x, y, z;

		x = (float)atof( args[ 1 ] );

		Globals::cl_enginefuncs->GetViewAngles( va );

		if ( args.ArgC() >= 3 )
		{
			y = (float)atof( args[ 2 ] );

			if ( args.ArgC() >= 4 )
			{
				z = (float)atof( args[ 3 ] );
			}
			else
			{
				z = va.z;
			}
		}
		else
		{
			y = va.y;
			z = va.z;
		}

		va.x = x;
		va.y = y;
		va.z = z;

		Globals::cl_enginefuncs->SetViewAngles( va );
	}
	else
	{
		Msg( "Usage:  setang <x> <optional: y> <optional: z>\n" );
	}
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

CClientModule::CClientModule()
{
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CClientModule::Init( void )
{
	Globals::cl_enginefuncs->pfnClientCmd( "cl_timeout 9999999;clockwindow 0\n" );

	Globals::cvar->RegisterConCommand( &sc_prof );
	Globals::cvar->RegisterConCommand( &sc_disable_monster_info );
	Globals::cvar->RegisterConCommand( &sc_disable_sprays );
	Globals::cvar->RegisterConCommand( &sc_debug_new_line_height );
	Globals::cvar->RegisterConCommand( &sc_debug_show_weapondata );
	Globals::cvar->RegisterConCommand( &sc_debug_show_inventory );
	Globals::cvar->RegisterConCommand( &sc_debug_show_refparams );
	Globals::cvar->RegisterConCommand( &sc_debug_show_movevars );
	Globals::cvar->RegisterConCommand( &sc_debug_show_clientdata );
	Globals::cvar->RegisterConCommand( &sc_debug_show_clientent );
	Globals::cvar->RegisterConCommand( &sc_debug_show_entitystate );
	Globals::cvar->RegisterConCommand( &sc_debug_show_playerinfo );
	Globals::cvar->RegisterConCommand( &sc_debug_show_playermove );
	Globals::cvar->RegisterConCommand( &sc_debug_show_prof );

	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_print_cvars ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_print_features ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_dump_disasm ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_dump_disasm_bytes ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_dump_interfaces ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_dump_netmsg ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_dump_usermsg ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( help ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( toggle ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( incrementvar ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( multvar ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( append ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_test ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_print_steamids ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_steamid_to_steam64id ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_load_model ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_find_model ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_find_model_prefix ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_find_cvar ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_find_cmd ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_drop_empty_weapon ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( getpos ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( getpos_exact ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( getang ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( setang ) );

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CClientModule::Shutdown( void )
{
	Globals::cvar->UnregisterConCommand( &sc_prof );
	Globals::cvar->UnregisterConCommand( &sc_disable_monster_info );
	Globals::cvar->UnregisterConCommand( &sc_disable_sprays );
	Globals::cvar->UnregisterConCommand( &sc_debug_new_line_height );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_weapondata );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_inventory );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_refparams );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_movevars );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_clientdata );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_clientent );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_entitystate );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_playerinfo );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_playermove );
	Globals::cvar->UnregisterConCommand( &sc_debug_show_prof );

	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_print_cvars ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_print_features ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_dump_disasm ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_dump_disasm_bytes ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_dump_interfaces ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_dump_netmsg ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_dump_usermsg ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( help ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( toggle ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( incrementvar ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( multvar ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( append ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_test ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_print_steamids ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_steamid_to_steam64id ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_load_model ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_find_model ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_find_model_prefix ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_find_cvar ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_find_cmd ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_drop_empty_weapon ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( getpos ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( getpos_exact ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( getang ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( setang ) );
}

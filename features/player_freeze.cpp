// SvenInt (c) Sw1ft
// player_freeze.cpp

#include "stdafx.h"
#include "player_freeze.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( qboolean, CALLCONV_CDECL, Netchan_CanPacket, netchan_t * );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERN( Netchan_CanPacket, "D9 ? ? ? ? ? D9 ? DA ? DF E0 F6 C4 ? 8B 44 24 04" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFreeze, freeze, "Player", "Freeze" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_freeze, "Prevents netchan from transmitting any data to a server" )
{
	if ( args.ArgC() > 1 )
	{
		bool bEnable = !!atoi( args[ 1 ] );
		if ( bEnable != THIS_FEATURE()->IsManualUse() )
		{
			THIS_FEATURE()->ToggleManualUse();
			Globals::gameutils->PrintChatText( "<SvenInt> %s is %s\n", THIS_FEATURE()->GetName(), bEnable ? "ON" : "OFF" );
		}
	}
	else
	{
		bool bEnabled = THIS_FEATURE()->ToggleManualUse();
		Globals::gameutils->PrintChatText( "<SvenInt> %s is %s\n", THIS_FEATURE()->GetName(), bEnabled ? "ON" : "OFF" );
	}
}

CON_COMMAND_CMDNAME( sc_freeze_down, "+sc_freeze", "" )
{
	if ( !THIS_FEATURE()->IsManualUse() )
		THIS_FEATURE()->ToggleManualUse();
}

CON_COMMAND_CMDNAME( sc_freeze_up, "-sc_freeze", "" )
{
	if ( THIS_FEATURE()->IsManualUse() )
		THIS_FEATURE()->ToggleManualUse();
}

//-----------------------------------------------------------------------------
// Netchan_CanPacket hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( qboolean, CALLCONV_CDECL, HOOKED_Netchan_CanPacket, netchan_t *netchan )
{
	if ( THIS_FEATURE()->IsPacketCanceled() )
		return 0;

	return ORIG_Netchan_CanPacket( netchan );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CFreeze::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_bManualUse = false;
		m_bCancelPacket = false;
		m_bCancelTransmit = false;
		m_bCancelMovCmd = false;

		if ( m_flNextCmdTime != NULL )
			*m_flNextCmdTime = 0.f;
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		if ( m_flNextCmdTime != NULL )
		{
			if ( m_bManualUse && m_pMode->GetInt() == 2 )
			{
				*m_flNextCmdTime = FLT_MAX;
			}
			else if ( m_bCancelMovCmd || *m_flNextCmdTime == FLT_MAX )
			{
				*m_flNextCmdTime = 0.f;
			}
		}

		m_bCancelPacket = false;
		m_bCancelTransmit = false;
		m_bCancelMovCmd = false;

		return kHookContinue;
	}
	else if ( pEvent->GetType() == kHUD_PostRunCmd_HookEvent )
	{
		// For proper work of revive boost info
		if ( IsPacketCanceled() || IsTransmitCanceled() || IsMovCmdCanceled() )
		{
			const Vector vecOrigin = localplayer->GetClientData()->origin;

			*playermove->origin() = vecOrigin;

			cl_enginefuncs->GetLocalPlayer()->origin = vecOrigin;
			cl_enginefuncs->GetLocalPlayer()->curstate.origin = vecOrigin;

			pEvent->GetArg<local_state_t *>( "from" )->playerstate.origin = vecOrigin;
			pEvent->GetArg<local_state_t *>( "to" )->playerstate.origin = vecOrigin;
		}
	}
	else if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		if ( IsPacketCanceled() || IsTransmitCanceled() || IsMovCmdCanceled() )
		{
			auto pparams = pEvent->GetArg<ref_params_t *>( "pparams" );

			pparams->punchangle[ 0 ] = 0.f;
			pparams->punchangle[ 1 ] = 0.f;
			pparams->punchangle[ 2 ] = 0.f;
		}
	}
	else if ( pEvent->GetHookResult() == kHookContinue && IsTransmitCanceled() ) // Netchan_Transmit event
	{
		pEvent->GetArg<int>( "lengthInBytes" ) = 0;
		pEvent->GetArg<unsigned char *>( "data" ) = NULL;
		return kHookHandled;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFreeze::CFreeze( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bManualUse = false;
	m_pMode = NULL;

	m_flNextCmdTime = NULL;

	m_bCancelPacket = false;
	m_bCancelTransmit = false;
	m_bCancelMovCmd = false;

	m_pfnNetchan_CanPacket = NULL;
	m_hNetchan_CanPacket = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CFreeze::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookCall, kHookPriorityHigh );
	hookevents->RegisterListener( this, kHUD_PostRunCmd_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->RegisterListener( this, kNetchan_Transmit_HookEvent, kHookCall, kHookPriorityHigh );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CFreeze::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent );
	hookevents->UnregisterListener( this, kHUD_PostRunCmd_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->UnregisterListener( this, kNetchan_Transmit_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFreeze::Load( void )
{
	ud_t inst;
	bool bOK = true;

	Modules::menu->BindFeature( this, false );

	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 0, " 0 - Cancel Transmit\0 1 - Cancel Packet\0 2 - Cancel Move Cmd\0\0" );

	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		MAKE_ASYNC( fm_pfnNetchan_CanPacket, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "Netchan_CanPacket" ); } );
		MAKE_ASYNC( fm_flNextCmdTime, [] { return gamedata->FindRVA( GameData::Modules::Engine, "Engine", "nextcmdtime" ); } );

		m_pfnNetchan_CanPacket = fm_pfnNetchan_CanPacket.get();
		m_flNextCmdTime = (float *)fm_flNextCmdTime.get();

		if ( m_pfnNetchan_CanPacket == NULL )
			PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Packet" );
		if ( m_flNextCmdTime == NULL )
			PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Move Cmd" );
	}
	else
	{
	#ifdef WIN32
		m_pfnNetchan_CanPacket = MemoryUtils()->FindPattern( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::Netchan_CanPacket );
		FEATURE_CHECK_SYMBOL_PATTERN_STATUS( m_pfnNetchan_CanPacket, "Netchan_CanPacket" );

		if ( !bOK )
			PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Packet" );

		bOK = true;
		void *pConnectionAccepted = MemoryUtils()->FindString( GameData::Modules::Engine, "Connection accepted by %s\n" );
		if ( pConnectionAccepted != NULL )
		{
			pConnectionAccepted = MemoryUtils()->FindAddress( GameData::Modules::Engine, pConnectionAccepted );
			if ( pConnectionAccepted != NULL && *( (uint8_t *)pConnectionAccepted - 1 ) == 0x68 )
			{
				MemoryUtils()->InitDisasm( &inst, (uint8_t *)pConnectionAccepted - 1, 32, 72 );
				while ( MemoryUtils()->Disassemble( &inst ) )
				{
					if ( inst.mnemonic != UD_Ifstp )
						continue;

					m_flNextCmdTime = reinterpret_cast<float *>( inst.operand[ 0 ].lval.udword );
					break;
				}
			}
		}

		FEATURE_CHECK_SYMBOL_STATUS( m_flNextCmdTime, "nextcmdtime" );
		if ( !bOK )
			PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Move Cmd" );
	#else
		PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Packet" );
		PrintWarning2( "Freeze method \"%s\" is not available\n", "Cancel Move Cmd" );
	#endif
	}

	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnNetchan_CanPacket", m_pfnNetchan_CanPacket, GameData::Modules::Engine );
	GAMEDATA_DUMP_FILE_OFFSET( "nextcmdtime", m_flNextCmdTime, GameData::Modules::Engine );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CFreeze::PostLoad( void )
{
	if ( m_pfnNetchan_CanPacket != NULL )
		m_hNetchan_CanPacket = Detours()->DetourFunction( m_pfnNetchan_CanPacket, HOOKED_Netchan_CanPacket, GET_FUNC_PTR( ORIG_Netchan_CanPacket ) );

	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_freeze ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_freeze_down ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_freeze_up ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CFreeze::Unload( void )
{
	Detours()->RemoveDetour( m_hNetchan_CanPacket );

	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_freeze ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_freeze_down ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_freeze_up ) );
}

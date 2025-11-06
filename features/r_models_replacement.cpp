// SvenInt (c) Sw1ft
// r_models_replacement.cpp

#include "stdafx.h"
#include "r_models_replacement.h"
#include "misc_random_generator.h"
#include "game/messagebuffer.h"
#include "utils/util.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( struct model_s *, __cdecl, SetupPlayerModel, int index );

//-----------------------------------------------------------------------------
// Hash Map Functions
//-----------------------------------------------------------------------------

static bool MM_HashMap_Compare( const uint64 &a, const uint64 &b ) { return a == b; }
static unsigned int MM_HashMap_Hash( const uint64 &a ) { return HashKey( (unsigned char *)&a, sizeof( uint64 ) ); }

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CModelsReplacement, modelsreplacement, "Render", "Models Replacement" );

static NetMsgHookFn ORIG_NetMsgHook_UpdateUserInfo = NULL;

//-----------------------------------------------------------------------------
// V_RenderView hook
//-----------------------------------------------------------------------------

void HOOKED_NetMsgHook_UpdateUserInfo( void )
{
	CNetMessageParams *params = gameutils->GetNetMessageParams();

	CMessageBuffer msg;
	msg.Init( params->buffer, params->readcount, params->badread );

	int index = msg.ReadByte();

	ORIG_NetMsgHook_UpdateUserInfo();

	THIS_FEATURE()->ResetPlayerInfo( index );
}

DECLARE_FUNC( struct model_s *, __cdecl, HOOKED_SetupPlayerModel, int index )
{
	if ( THIS_FEATURE()->IsEnabled() )
		THIS_FEATURE()->UpdatePlayerModel( index );

	return ORIG_SetupPlayerModel( index );
}

//-----------------------------------------------------------------------------
// Replace logic
//-----------------------------------------------------------------------------

void CModelsReplacement::ResetPlayersInfo( void )
{
	memset( m_PlayerModelReplacementInfo, 0, Q_ARRAYSIZE( m_PlayerModelReplacementInfo ) * sizeof( player_model_replacement_info_s ) );
}

void CModelsReplacement::ResetLocalPlayerInfo( void )
{
	m_PlayerModelReplacementInfo[ playermove->player_index() ].steamid = 0uLL;
	m_PlayerModelReplacementInfo[ playermove->player_index() ].random_model = 0;
	m_PlayerModelReplacementInfo[ playermove->player_index() ].model_replaced = false;
}

void CModelsReplacement::ResetPlayerInfo( int index )
{
	m_PlayerModelReplacementInfo[ index ].model_replaced = false;
}

void CModelsReplacement::CheckPlayerInfo( int index )
{
	uint64 steamid = gameutils->GetSteamID( index + 1 );

	if ( steamid != m_PlayerModelReplacementInfo[ index ].steamid )
	{
		m_PlayerModelReplacementInfo[ index ].steamid = steamid;
		m_PlayerModelReplacementInfo[ index ].random_model = 0;
		m_PlayerModelReplacementInfo[ index ].model_replaced = false;
	}
}

void CModelsReplacement::UpdatePlayerModel( int index )
{
	CheckPlayerInfo( index );

	uint64 *pIgnoreSteamID = NULL;
	std::string *pTargetModel = NULL;

	if ( m_pReplaceTargetedPlayers->GetBool() )
	{
		pTargetModel = m_TargetPlayers.Find( m_PlayerModelReplacementInfo[ index ].steamid );
		if ( pTargetModel != NULL )
		{
			if ( !m_PlayerModelReplacementInfo[ index ].model_replaced )
			{
				const char *pszModel = (const char *)( &m_pUserInfo[ m_ulModelOffset * index ] );

				memcpy( (char *)pszModel, ( *pTargetModel ).c_str(), ( *pTargetModel ).length() + 1 );

				m_PlayerModelReplacementInfo[ index ].random_model = 0;
				m_PlayerModelReplacementInfo[ index ].model_replaced = true;

				return;
			}
		}
	}

	if ( m_pIgnoreSpecifiedPlayers->GetBool() )
	{
		pIgnoreSteamID = m_IgnorePlayers.Find( m_PlayerModelReplacementInfo[ index ].steamid );

		if ( pIgnoreSteamID != NULL )
			return;
	}

	if ( m_pReplacePlayersWithRandom->GetBool() && !m_RandomModels.empty() )
	{
		if ( index != playermove->player_index() ||
			 ( index == playermove->player_index() && m_pReplaceOnSelf->GetBool() ) )
		{
			if ( !m_PlayerModelReplacementInfo[ index ].model_replaced &&
				 pIgnoreSteamID == NULL &&
				 pTargetModel == NULL )
			{
				int model_index;

				if ( m_PlayerModelReplacementInfo[ index ].random_model != 0 )
					model_index = m_PlayerModelReplacementInfo[ index ].random_model = ( m_PlayerModelReplacementInfo[ index ].random_model - 1 );
				else
					model_index = Features::random->RandomInt( 0, m_RandomModels.size() - 1 );

				const char *pszModel = (const char *)( &m_pUserInfo[ m_ulModelOffset * index ] );

				memcpy( (char *)pszModel, m_RandomModels[ model_index ].c_str(), m_RandomModels[ model_index ].length() + 1 );

				m_PlayerModelReplacementInfo[ index ].random_model = model_index + 1;
				m_PlayerModelReplacementInfo[ index ].model_replaced = true;

				return;
			}
		}
	}

	if ( m_pReplaceAllPlayers->GetBool() )
	{
		if ( index != playermove->player_index() ||
			 ( index == playermove->player_index() && m_pReplaceOnSelf->GetBool() ) )
		{
			if ( !m_PlayerModelReplacementInfo[ index ].model_replaced &&
				 pIgnoreSteamID == NULL &&
				 pTargetModel == NULL )
			{
				const char *pszModel = (const char *)( &m_pUserInfo[ m_ulModelOffset * index ] );

				memcpy( (char *)pszModel, m_pReplaceModel->GetCString(), m_pReplaceModel->GetSize() );

				m_PlayerModelReplacementInfo[ index ].random_model = 0;
				m_PlayerModelReplacementInfo[ index ].model_replaced = true;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Load files
//-----------------------------------------------------------------------------

void CModelsReplacement::ReloadRandomModels()
{
	char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/models_replacement/random_models.txt", "r" );

	if ( file )
	{
		int nLine = 0;

		m_RandomModels.clear();

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			m_RandomModels.push_back( std::string( buffer ) );
		}

		ResetPlayersInfo();
		fclose( file );
	}
	else
	{
		PrintWarning( "Missing file \"" SVENINT_FOLDER_NAME "/models_replacement/random_models.txt\"\n" );
	}
}

void CModelsReplacement::ReloadTargetPlayers()
{
	char szBuffer[ 512 ];
	char szParameterBuffer[ 512 ];

	FILE *file = fopen( SVENINT_FOLDER_NAME "/models_replacement/target_players.txt", "r" );

	if ( file )
	{
		int nLine = 0;

		m_TargetPlayers.Clear();

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			char *key = strtok( buffer, "=" );

			if ( !key || !*key )
			{
				Warning( "Key of a parameter is empty (line: %d)\n", nLine );
				continue;
			}

			char *value = strtok( NULL, "=" );

			if ( !value || !*value )
			{
				Warning( "Value of a parameter is empty (line: %d)\n", nLine );
				continue;
			}

			UTIL_RStrip( key );
			UTIL_RStrip( value );
			value = UTIL_LStrip( value );

			uint64 steamid = atoll( key );

			if ( steamid != 0uLL )
			{
				memcpy( szParameterBuffer, value, strlen( value ) + 1 );
				szParameterBuffer[ ( sizeof( szParameterBuffer ) / sizeof( *szParameterBuffer ) ) - 1 ] = 0;

				m_TargetPlayers.Insert( steamid, std::string( szParameterBuffer ) );
			}
		}

		ResetPlayersInfo();
		fclose( file );
	}
	else
	{
		PrintWarning( "Missing file \"" SVENINT_FOLDER_NAME "/models_replacement/target_players.txt\"\n" );
	}
}

void CModelsReplacement::ReloadIgnoredPlayers()
{
	char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/models_replacement/ignored_players.txt", "r" );

	if ( file )
	{
		int nLine = 0;

		m_IgnorePlayers.Clear();

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			uint64 steamid = atoll( buffer );

			if ( steamid != 0uLL )
			{
				m_IgnorePlayers.Insert( steamid );
			}
		}

		ResetPlayersInfo();
		fclose( file );
	}
	else
	{
		PrintWarning( "Missing file \"" SVENINT_FOLDER_NAME "/models_replacement/ignored_players.txt\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CModelsReplacement::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pResetReplacement )
	{
		ResetPlayersInfo();
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CModelsReplacement::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	ResetPlayersInfo();
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CModelsReplacement::CModelsReplacement( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ),
																							m_TargetPlayers( 15 ),
																							m_IgnorePlayers( 15, MM_HashMap_Compare, MM_HashMap_Hash )
{
	SetInitiallyDisabled();
	ResetPlayersInfo();

	m_pReplaceOnSelf = NULL;
	m_pReplaceAllPlayers = NULL;
	m_pReplacePlayersWithRandom = NULL;
	m_pReplaceTargetedPlayers = NULL;
	m_pIgnoreSpecifiedPlayers = NULL;
	m_pReplaceModel = NULL;

	m_ulModelOffset = 0;
	m_pUserInfo = NULL;

	m_hNetMsgHook_UpdateUserInfo = DETOUR_INVALID_HANDLE;
	m_hSetupPlayerModel = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CModelsReplacement::OnEnable( void )
{
	ResetPlayersInfo();

	ReloadIgnoredPlayers();
	ReloadTargetPlayers();
	ReloadRandomModels();

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CModelsReplacement::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CModelsReplacement::Load( void )
{
	ud_t inst;

	Modules::menu->BindFeature( this );

	m_pResetReplacement = Modules::menu->AddElementButton( this, this, "Reset Replacements" );

	m_pReplaceOnSelf = Modules::menu->AddParamBool( this, "ReplaceOnSelf", "Replace model on self", false );
	m_pReplaceAllPlayers = Modules::menu->AddParamBool( this, "ReplaceAllPlayers", "Replace models of all players", false );
	m_pReplacePlayersWithRandom = Modules::menu->AddParamBool( this, "ReplacePlayersWithRandom", "Replace models of players with random ones", false );
	m_pReplaceTargetedPlayers = Modules::menu->AddParamBool( this, "ReplaceTargetedPlayers", "Replace models of targeted players", false );
	m_pIgnoreSpecifiedPlayers = Modules::menu->AddParamBool( this, "IgnoreSpecifiedPlayers", "Don't replace models on specified players", false );
	m_pReplaceModel = Modules::menu->AddParamText( this, "ReplaceModel", "Replacement model for all players", "player" );

	MemoryUtils()->InitDisasm( &inst, enginestudio->SetupPlayerModel, 32, 112 );

	while( MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Ilea &&
			 inst.operand[ 0 ].type == UD_OP_REG &&
			 inst.operand[ 1 ].type == UD_OP_MEM &&
			 inst.operand[ 1 ].offset == 32 )
		{
			m_pUserInfo = reinterpret_cast<uint8_t *>( inst.operand[ 1 ].lval.udword );
			break;
		}
		else if ( inst.mnemonic == UD_Iimul &&
				  inst.operand[ 0 ].type == UD_OP_REG &&
				  ( ( inst.operand[ 1 ].type == UD_OP_REG && inst.operand[ 2 ].type == UD_OP_IMM ) || inst.operand[ 1 ].type == UD_OP_IMM ) )
		{
			m_ulModelOffset = ( inst.operand[ 1 ].type == UD_OP_IMM ? inst.operand[ 1 ].lval.udword : inst.operand[ 2 ].lval.udword );
		}
	}

	FEATURE_CHECK_SYMBOL( m_pUserInfo, "userinfo" );
	FEATURE_CHECK_SYMBOL( m_ulModelOffset, "modeloffset" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CModelsReplacement::PostLoad( void )
{
	m_hNetMsgHook_UpdateUserInfo = gamehooks->HookNetworkMessage( SVC_UPDATEUSERINFO, HOOKED_NetMsgHook_UpdateUserInfo, &ORIG_NetMsgHook_UpdateUserInfo );
	m_hSetupPlayerModel = Detours()->DetourFunction( enginestudio->SetupPlayerModel, HOOKED_SetupPlayerModel, GET_FUNC_PTR( ORIG_SetupPlayerModel ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CModelsReplacement::Unload( void )
{
	Detours()->RemoveDetour( m_hNetMsgHook_UpdateUserInfo );
	Detours()->RemoveDetour( m_hSetupPlayerModel );
}
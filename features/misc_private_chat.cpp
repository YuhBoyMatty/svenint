// SvenInt (c) Sw1ft
// misc_private_chat.cpp

#include "stdafx.h"
#include "misc_private_chat.h"
#include "game/messagebuffer.h"
#include "utils/util.h"
#include "utils/GOST_28147.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CPrivateChat, privatechat, "Misc", "Private Chat" );

static UserMsgHookFn ORIG_UserMsgHook_SayText = NULL;
static CommandCallbackFn ORIG_CmdHook_Say = NULL;
static CommandCallbackFn ORIG_CmdHook_SayTeam = NULL;

//-----------------------------------------------------------------------------
// User message hook SayText, copypasting from SourceChat, it did good really well :P
//-----------------------------------------------------------------------------

static int UserMsgHook_SayText( const char *pszName, int iSize, void *pBuffer )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
		return ORIG_UserMsgHook_SayText( pszName, iSize, pBuffer );

	CMessageBuffer message( pszName, pBuffer, iSize, true );

	int src;
	int client = message.ReadByte();
	const char *pszMessage = (char *)message.GetBuffer()->data + message.GetReadCount();
	//const char *pszMessage = message.ReadString();

	if ( *pszMessage > 0 && *pszMessage <= 3 )
	{
		src = *pszMessage;
		pszMessage = pszMessage + 1;
	}
	else
	{
		src = 0;
		client = 0;
	}

	// Message from a player
	if ( src == 2 && pszMessage[ 0 ] != '\0' )
		THIS_FEATURE()->OnReceiveMessage( client, pszMessage );

	return ORIG_UserMsgHook_SayText( pszName, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// Cmd hook 'say'
//-----------------------------------------------------------------------------

static void HOOKED_CmdHook_Say( void )
{
	if ( THIS_FEATURE_IS_ENABLED() && THIS_FEATURE()->OnSendMessage( false ) )
		return;

	ORIG_CmdHook_Say();
}

//-----------------------------------------------------------------------------
// Cmd hook 'say_team'
//-----------------------------------------------------------------------------

static void HOOKED_CmdHook_SayTeam( void )
{
	if ( THIS_FEATURE_IS_ENABLED() && THIS_FEATURE()->OnSendMessage( true ) )
		return;

	ORIG_CmdHook_SayTeam();
}

//-----------------------------------------------------------------------------
// Send message event
//-----------------------------------------------------------------------------

bool CPrivateChat::OnSendMessage( bool bTeam )
{
	if ( m_bWaitSingleFrame )
		return false;

	if ( cl_enginefuncs->Cmd_Argc() < 2 )
		return false;

	char cmd[ 256 ], buffer[ 256 ];
	const char *pszMessage = cl_enginefuncs->Cmd_Argv( 1 );
	if ( cl_enginefuncs->Cmd_Argc() > 2 )
	{
		buffer[ 0 ] = '\0';
		strcat( buffer, pszMessage );

		for ( int i = 2; i < cl_enginefuncs->Cmd_Argc(); i++ )
		{
			strcat( buffer, " " );
			strcat( buffer, cl_enginefuncs->Cmd_Argv( i ) );
		}

		pszMessage = buffer;
	}

	while ( *pszMessage == ' ' && *pszMessage != '\0' )
		++pszMessage;

	if ( *pszMessage == '\0' )
		return false;

	const bool bHasPrivateMessagePrefix = ( strnicmp( pszMessage, "/pm ", Q_ARRAYSIZE( "/pm " ) - 1 ) == 0 );
	if ( !m_pEncryptAlways->GetBool() && !bHasPrivateMessagePrefix )
		return false;

	// Actual message that should be encrypted
	if ( bHasPrivateMessagePrefix )
		pszMessage += Q_ARRAYSIZE( "/pm " ) - 1;

	//Msg( "pszMessage: %s\n", pszMessage );

	size_t msglen = get_encrypt_length( strlen( pszMessage ) );
	char *pszEncryptedMessage = (char *)calloc( msglen + 1, sizeof( char ) );
	if ( pszEncryptedMessage == NULL )
		return true;

	encrypt_message( pszMessage, m_pEncryptKey->GetCString(), pszEncryptedMessage, msglen );

	char *p = pszEncryptedMessage;

	buffer[ 0 ] = '\0';
	while ( *p != '\0' )
	{
		snprintf( cmd, Q_ARRAYSIZE( cmd ), "%02x", (uint8_t)*p );
		strcat( buffer, cmd );
		p++;
	}

	snprintf( cmd, Q_ARRAYSIZE( cmd ), "%s \"706d%s\"", bTeam ? "say_team" : "say", buffer );
	cl_enginefuncs->pfnClientCmd( cmd );

	//Msg( "cmd: %s\n", cmd );

	free( pszEncryptedMessage );
	m_bWaitSingleFrame = true;
	return true;
}

//-----------------------------------------------------------------------------
// Receive message event
//-----------------------------------------------------------------------------

void CPrivateChat::OnReceiveMessage( int client, const char *pszMessage )
{
	char *msg;
	char byte[ 3 ] = { 0 };

	bool bFoundNewlineChar = false;
	char *pszNewlineChar = const_cast<char *>( pszMessage );

	while ( *pszNewlineChar )
	{
		if ( *pszNewlineChar == '\n' )
			break;
		++pszNewlineChar;
	}

	if ( *pszNewlineChar == '\n' )
		bFoundNewlineChar = true;

	player_info_t *pPlayerInfo = enginestudio->PlayerInfo( client - 1 );
	if ( pPlayerInfo == NULL )
		return;

	if ( m_pWhitelistedPlayers->GetBool() )
	{
		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
		auto it = std::find( m_whitelist.begin(), m_whitelist.end(), gameutils->GetSteamID( client ) );

		if ( it == m_whitelist.end() && pLocal != NULL && pLocal->index != client )
			return;
	}

	size_t namelen = strlen( pPlayerInfo->name );

	if ( strncmp( pPlayerInfo->name, pszMessage, namelen ) == 0 )
		msg = (char *)pszMessage + namelen + 2;
	else
		msg = (char *)pszMessage + namelen + Q_ARRAYSIZE( "(TEAM)" ) + 2;

	//Msg( "msg (%d): %s", strlen( msg ), msg );
	if ( strlen( msg ) < 12 )
		return;

	byte[ 0 ] = msg[ 0 ];
	byte[ 1 ] = msg[ 1 ];
	bool isP = static_cast<char>( __svenint_hex_to_decimal_fast( byte ) ) == 'p';
	
	byte[ 0 ] = msg[ 2 ];
	byte[ 1 ] = msg[ 3 ];
	bool isM = static_cast<char>( __svenint_hex_to_decimal_fast( byte ) ) == 'm';

	//Msg( "isP: %d, isM: %d\n", isP, isM );
	if ( !isP || !isM )
		return;

	msg += 4;

	//Msg( "msg: %s", msg );
	if ( bFoundNewlineChar )
		*pszNewlineChar = '\0';

	size_t msglen = 8 * ( get_encrypt_length( strlen( msg ) / 2 ) / 8 );

	char *pszEncryptedMessage = (char *)calloc( msglen + 1, sizeof( char ) );
	char *pszDecryptedMessage = (char *)calloc( msglen + 2, sizeof( char ) );

	if ( pszEncryptedMessage == NULL || pszDecryptedMessage == NULL )
	{
		if ( pszEncryptedMessage != NULL )
			free( pszEncryptedMessage );

		if ( pszDecryptedMessage != NULL )
			free( pszDecryptedMessage );

		if ( bFoundNewlineChar )
			*pszNewlineChar = '\n';

		return;
	}

	int c = 0;
	for ( size_t i = 0; i <= msglen * 2; i += 2 )
	{
		byte[ 0 ] = msg[ i ];
		byte[ 1 ] = msg[ i + 1 ];

		pszEncryptedMessage[ c++ ] = static_cast<char>( __svenint_hex_to_decimal_fast( byte ) );
	}

	//Msg( "pszEncryptedMessage: %s\n", pszEncryptedMessage );
	decrypt_message( pszEncryptedMessage, m_pEncryptKey->GetCString(), pszDecryptedMessage, msglen );

	PrintMsg( "Decrypted message \"%s\" from player %s (%s)\n", pszDecryptedMessage, pPlayerInfo->name, msg - 4 );
	pszDecryptedMessage[ msglen + 1 ] = '\n';

	char result[ 256 ] = "[PM] ";
	strcat( result, pszDecryptedMessage );
	memcpy( msg - 4, result, strlen( result ) + 1 );

	free( pszEncryptedMessage );
	free( pszDecryptedMessage );
}

//-----------------------------------------------------------------------------
// LoadWhitelistFromFile
//-----------------------------------------------------------------------------

void CPrivateChat::LoadWhitelistFromFile( void )
{
	char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/private_chat_whitelist.txt", "r" );

	if ( file != NULL )
	{
		m_whitelist.clear();
		int nLine = 0;

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( (char *)szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			uint64 steamID = atoll( buffer );
			if ( steamID != 0uLL )
				m_whitelist.push_back( steamID );
		}

		fclose( file );

		if ( !m_whitelist.empty() )
			std::sort( m_whitelist.begin(), m_whitelist.end() );
	}
	else
	{
		PrintWarning( "Missing file \"./" SVENINT_FOLDER_NAME "/private_chat_whitelist.txt\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CPrivateChat::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_PostRunCmd_HookEvent )
	{
		m_bWaitSingleFrame = false;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CPrivateChat::CPrivateChat( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pEncryptAlways = NULL;
	m_pWhitelistedPlayers = NULL;
	m_pEncryptKey = NULL;

	m_bWaitSingleFrame = false;

	m_hUserMsgHook_SayText = DETOUR_INVALID_HANDLE;
	m_hCmdHook_Say = DETOUR_INVALID_HANDLE;
	m_hCmdHook_SayTeam = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CPrivateChat::OnEnable( void )
{
	LoadWhitelistFromFile();
	hookevents->RegisterListener( this, kHUD_PostRunCmd_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CPrivateChat::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_PostRunCmd_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CPrivateChat::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pEncryptAlways = Modules::menu->AddParamBool( this, "EncryptAlways", "Encrypt any message you send w/o prefix /pm", false );
	m_pWhitelistedPlayers = Modules::menu->AddParamBool( this, "WhitelistedPlayers", "Decrypt messages from whitelisted players only", true );
	m_pEncryptKey = Modules::menu->AddParamText( this, "EncryptKey", "Encrypt / decrypt key", "qbQKD31jqrH7aiXbCjubR@XM^&wNKuWh" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CPrivateChat::PostLoad( void )
{
	m_hUserMsgHook_SayText = gamehooks->HookUserMessage( "SayText", UserMsgHook_SayText, &ORIG_UserMsgHook_SayText );
	m_hCmdHook_Say = gamehooks->HookConsoleCommand( "say", HOOKED_CmdHook_Say, &ORIG_CmdHook_Say );
	m_hCmdHook_SayTeam = gamehooks->HookConsoleCommand( "say_team", HOOKED_CmdHook_SayTeam, &ORIG_CmdHook_SayTeam );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CPrivateChat::Unload( void )
{
	gamehooks->UnhookUserMessage( m_hUserMsgHook_SayText );
	gamehooks->UnhookConsoleCommand( m_hCmdHook_Say );
	gamehooks->UnhookConsoleCommand( m_hCmdHook_SayTeam );
}
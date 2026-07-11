// SvenInt (c) Sw1ft
// https://github.com/autisoid/Lightning_public/blob/master/sven_internal/msvs_generic/sven_internal/sven_internal/protocol.cpp
// misc_antislowhack.cpp

#include "stdafx.h"
#include "misc_antislowhack.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAntiSlowhack, antislowhack, "Misc", "Anti Slowhack" );

static NetMsgHookFn ORIG_NetMsgHook_StuffText = NULL;
static NetMsgHookFn ORIG_NetMsgHook_Director = NULL;

static const char *s_rgszPredefinedAllowedList[] = { "npc_findcover", "fullserverinfo", "retry", "reconnect" };

//-----------------------------------------------------------------------------
// StuffText hook
//-----------------------------------------------------------------------------

static void NetMsgHook_StuffText( void )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_NetMsgHook_StuffText();
		return;
	}

	CMessageBuffer msg;
	CNetMessageParams *params = gameutils->GetNetMessageParams();
	msg.Init( params->buffer, params->readcount, params->badread );

	char *pszCommand = msg.ReadString();
	if ( THIS_FEATURE()->CheckCommandString( pszCommand ) )
		gameutils->ApplyReadToNetMessageBuffer( &msg );
	else
		ORIG_NetMsgHook_StuffText();
}

//-----------------------------------------------------------------------------
// Director hook
//-----------------------------------------------------------------------------

static void NetMsgHook_Director( void )
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	{
		ORIG_NetMsgHook_Director();
		return;
	}

	CMessageBuffer msg;
	CNetMessageParams *params = gameutils->GetNetMessageParams();
	msg.Init( params->buffer, params->readcount, params->badread );

	int cmdtype = msg.ReadByte();
	if ( cmdtype != 10 /* DRC_CMD_STUFFTEXT */ )
	{
		ORIG_NetMsgHook_Director();
		return;
	}

	char *pszCommand = msg.ReadString();
	if ( THIS_FEATURE()->CheckCommandString( pszCommand ) )
		gameutils->ApplyReadToNetMessageBuffer( &msg );
	else
		ORIG_NetMsgHook_Director();
}

//-----------------------------------------------------------------------------
// Tokenize command
//-----------------------------------------------------------------------------

std::vector<char *> *CAntiSlowhack::Split( const char *pszCommand, char chDelimiter )
{
	if ( pszCommand == NULL )
		return NULL;

	std::vector<char *> *pVec = new std::vector<char *>;
	const char *szStart = pszCommand;

	while ( *pszCommand )
	{
		if ( *pszCommand == chDelimiter )
		{
			int iLength = pszCommand - szStart;
			char *szPart = (char *)malloc( sizeof( char * ) * ( iLength + 1 ) );
			if ( szPart != NULL )
			{
				strncpy( szPart, szStart, iLength );
				szPart[ iLength ] = '\0';
				pVec->push_back( szPart );
			}
			szStart = pszCommand + 1;
		}
		pszCommand++;
	}

	int iLength = pszCommand - szStart;
	if ( iLength > 0 )
	{
		char *szPart = (char *)malloc( sizeof( char * ) * ( iLength + 1 ) );
		if ( szPart != NULL )
		{
			strncpy( szPart, szStart, iLength );
			szPart[ iLength ] = '\0';
			pVec->push_back( szPart );
		}
	}

	return pVec;
}

//-----------------------------------------------------------------------------
// Get sanitized command
//-----------------------------------------------------------------------------

sanitized_command CAntiSlowhack::GetSanitizedCommand( const char *start, const char *end )
{
	sanitized_command cmd;
	cmd.sCmdName = "";
	cmd.sRawCmd = "";

	const char *pszTrimmedStart = start;
	const char *pszTrimmedEnd = end - 1;

	while ( pszTrimmedStart < end && *pszTrimmedStart == ' ' )
		pszTrimmedStart++;

	while ( pszTrimmedEnd >= pszTrimmedStart && *pszTrimmedEnd == ' ' )
		pszTrimmedEnd--;

	if ( pszTrimmedStart > pszTrimmedEnd )
		return cmd;

	cmd.sRawCmd = std::string( pszTrimmedStart, pszTrimmedEnd - pszTrimmedStart + 1 );

	const char *pszStart = pszTrimmedStart;
	const char *pszEnd = pszStart;

	if ( *pszStart == '"' )
	{
		pszStart++;
		pszEnd = pszStart;
		while ( pszEnd <= pszTrimmedEnd && *pszEnd != '"' )
			pszEnd++;

		if ( pszEnd <= pszTrimmedEnd )
			cmd.sCmdName = std::string( pszStart, pszEnd - pszStart );
		else
			cmd.sCmdName = "";
	}
	else
	{
		pszEnd = pszStart;
		while ( pszEnd <= pszTrimmedEnd && *pszEnd != ' ' )
			pszEnd++;
		
		cmd.sCmdName = std::string( pszStart, pszEnd - pszStart );
	}

	return cmd;
}

//-----------------------------------------------------------------------------
// Sanitize command
//-----------------------------------------------------------------------------

std::vector<sanitized_command> *CAntiSlowhack::SanitizeCommand( const char *pszCommand )
{
	if ( pszCommand == NULL || *pszCommand == '\0' )
		return NULL;

	std::vector<sanitized_command> *pResult = new std::vector<sanitized_command>;

	const char *pszStart = pszCommand;
	const char *pszPos;

	while ( true )
	{
		pszPos = pszStart;
		while ( *pszPos && *pszPos != ';' )
			pszPos++;

		if ( pszPos > pszStart )
		{
			sanitized_command cmd = GetSanitizedCommand( pszStart, pszPos );
			if ( !cmd.sCmdName.empty() )
				pResult->push_back( cmd );
		}

		if ( *pszPos == '\0' )
			break;
		pszStart = pszPos + 1;
	}

	return pResult;
}

//-----------------------------------------------------------------------------
// IsCommandInPredefinedAllowedList
//-----------------------------------------------------------------------------

bool CAntiSlowhack::IsCommandInPredefinedAllowedList( const char *pszCommand )
{
	for ( int idx = 0; idx < Q_ARRAYSIZE( s_rgszPredefinedAllowedList ); idx++ )
	{
		if ( !stricmp( pszCommand, s_rgszPredefinedAllowedList[ idx ] ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// IsCommandDangerous
//-----------------------------------------------------------------------------

bool CAntiSlowhack::IsCommandDangerous( const char *pszCommand )
{
	switch ( m_pMode->GetInt() )
	{
	case 0: /* Block everything, except gameplay important commands */
	{
		return !IsCommandInPredefinedAllowedList( pszCommand );
	}
	break;

	case 1: /* Whitelist */
	{
		if ( IsCommandInPredefinedAllowedList( pszCommand ) )
			return false;

		std::vector<char *> *pSplit = Split( m_pWhitelist->GetCString(), ',' );
		bool bResult = true;
		for ( int idx = 0; idx < (int)pSplit->size(); idx++ )
		{
			if ( !stricmp( pszCommand, pSplit->at( idx ) ) )
			{
				bResult = false; // Yeah, it is slower now.
			}
		}
		for ( int idx = 0; idx < (int)pSplit->size(); idx++ )
		{
			delete pSplit->at( idx );
		}
		delete pSplit;

		return bResult;
	}
	break;
	}

	return false;
}

//-----------------------------------------------------------------------------
// CheckCommandString
//-----------------------------------------------------------------------------

bool CAntiSlowhack::CheckCommandString( char *pszCommand )
{
	char szCmdExec[ 1024 ];
	bool bBlockedAny = false;
	
	char *pszNewline = strrchr( pszCommand, '\n' );
	if ( pszNewline != NULL && pszNewline[ 1 ] == '\0' )
		*pszNewline = '\0';

	std::vector<sanitized_command> *lpVecParts = SanitizeCommand( pszCommand );
	if ( lpVecParts == NULL )
	{
		if ( pszNewline != NULL && pszNewline[ 1 ] == '\0' )
			*pszNewline = '\n';

		return bBlockedAny;
	}

	PrintMsg( "Received sequence of console commands from the server: \"%s\"\n", pszCommand );
	
	for ( int idx = 0; idx < (int)lpVecParts->size(); idx++ )
	{
		sanitized_command &cmd = lpVecParts->at( idx );
		if ( IsCommandDangerous( cmd.sCmdName.c_str() ) )
		{
			PrintMsg( "Blocked malicious console command, which server tried to execute: \"%s\"\n", cmd.sRawCmd.c_str() );

			if ( m_pNotifyPlayers->GetBool() )
			{
				snprintf( szCmdExec, Q_ARRAYSIZE( szCmdExec ), ";say \"%s\";\n", m_pNotifyPlayersText->GetCString() );
				cl_enginefuncs->pfnClientCmd( szCmdExec );
			}

			bBlockedAny = true;
		}
		else
		{
			PrintMsg( "Allowed execution of console command: \"%s\"\n", cmd.sRawCmd.c_str() );
			snprintf( szCmdExec, Q_ARRAYSIZE( szCmdExec ), ";%s;\n", cmd.sRawCmd.c_str() );
			cl_enginefuncs->pfnClientCmd( szCmdExec );
		}
	}
	delete lpVecParts;

	if ( pszNewline != NULL && pszNewline[ 1 ] == '\0' )
		*pszNewline = '\n';

	return bBlockedAny;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAntiSlowhack::CAntiSlowhack( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pMode = NULL;
	m_pWhitelist = NULL;
	m_pNotifyPlayers = NULL;
	m_pNotifyPlayersText = NULL;

	m_hNetMsgHook_StuffText = DETOUR_INVALID_HANDLE;
	m_hNetMsgHook_Director = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAntiSlowhack::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pMode = Modules::menu->AddParamList( this, "Mode", NULL, 1, " 0 - Block everything except gameplay important cmds\0 1 - Whitelist\0\0" );
	m_pWhitelist = Modules::menu->AddParamText( this, "Whitelist", NULL, "speak,spk,play,mp3,stopsound,cl_stopsound,upload,httpstop" );
	m_pNotifyPlayers = Modules::menu->AddParamBool( this, "NotifyPlayers", NULL, false );
	m_pNotifyPlayersText = Modules::menu->AddParamText( this, "NotifyPlayersText", NULL, "Woah! Seems like the server tried to slowhack me!" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CAntiSlowhack::PostLoad( void )
{
	m_hNetMsgHook_StuffText = gamehooks->HookNetworkMessage( SVC_STUFFTEXT, NetMsgHook_StuffText, &ORIG_NetMsgHook_StuffText );
	m_hNetMsgHook_Director = gamehooks->HookNetworkMessage( SVC_DIRECTOR, NetMsgHook_Director, &ORIG_NetMsgHook_Director );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CAntiSlowhack::Unload( void )
{
	gamehooks->UnhookNetworkMessage( m_hNetMsgHook_StuffText );
	gamehooks->UnhookNetworkMessage( m_hNetMsgHook_Director );
}
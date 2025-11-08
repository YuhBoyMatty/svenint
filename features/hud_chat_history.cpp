// SvenInt (c) Sw1ft
// hud_chat_history.cpp

#include "stdafx.h"
#include "hud_chat_history.h"
#include "r_drawing.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Client
		{
			DEFINE_PATTERNS_2( CHudTextMessage__MsgFunc_TextMsg,
							   "5.25",
							   "53 55 56 57 FF 74 24 18 FF 74 24 20",
							   "5.11",
							   "8B 44 24 08 8B 4C 24 0C 53" );

			DEFINE_PATTERNS_2( m_pSoundEngine,
							   "5.25",
							   "A1 ? ? ? ? 85 C0 0F 85 ? ? ? ? 68 ? ? ? ? E8 ? ? ? ? 8B F0",
							   "5.11",
							   "A1 ? ? ? ? 85 C0 75 ? 68 ? ? ? ? E8 ? ? ? ? 83 C4 ? 85 C0 74 ? 8B C8 E8 ? ? ? ? EB ? 33 C0 A3 ? ? ? ? 8B 88 04 20 00 00" );

			DEFINE_PATTERNS_2( CClient_SoundEngine__Play2DSound,
							   "5.25",
							   "83 EC ? F3 0F 10 44 24 14",
							   "5.11",
							   "83 EC ? D9 ? 8B 44 24 10" );
		}
	}
}

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( int, CHudTextMessage__MsgFunc_TextMsg, void *, const char *, int, void * );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CChatHistory, chathistory, "HUD", "Chat History" );

static UserMsgHookFn ORIG_UserMsgHook_SayText = NULL;
//static UserMsgHookFn ORIG_UserMsgHook_TextMsg = NULL;

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
	const char *pszMessage = message.ReadString();

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

	if ( pszMessage[ 0 ] != '\0' )
		THIS_FEATURE()->OnReceiveMessage( client, pszMessage, src );

	return 0;
}

//-----------------------------------------------------------------------------
// User message hook TextMsg
//-----------------------------------------------------------------------------

#if 0
DECLARE_FUNC( int, __cdecl, UserMsgHook_TextMsg, const char *pszUserMsg, int iSize, void *pBuffer )
#else
DECLARE_CLASS_FUNC( int, HOOKED_CHudTextMessage__MsgFunc_TextMsg, void *thisptr, const char *pszUserMsg, int iSize, void *pBuffer )
#endif
{
	if ( !THIS_FEATURE_IS_ENABLED() )
	#if 0
		return ORIG_UserMsgHook_TextMsg( pszUserMsg, iSize, pBuffer );
	#else
		return ORIG_CHudTextMessage__MsgFunc_TextMsg( thisptr, pszUserMsg, iSize, pBuffer );
	#endif

	CMessageBuffer message( pszUserMsg, pBuffer, iSize, true );

	if ( message.ReadByte() == HUD_PRINTTALK )
	{
		static char buffer[ 256 ];

		const char *str;
		std::vector<std::string> formattingStrings;

		std::string msg = message.ReadString();

		size_t length = strlen( msg.c_str() ) + 1;

		// #1
		str = message.ReadString();

		if ( *str != '\0' )
			formattingStrings.push_back( str );

		// #2
		str = message.ReadString();

		if ( *str != '\0' )
			formattingStrings.push_back( str );

		// #3
		str = message.ReadString();

		if ( *str != '\0' )
			formattingStrings.push_back( str );

		// #4
		str = message.ReadString();

		if ( *str != '\0' )
			formattingStrings.push_back( str );

		switch ( formattingStrings.size() )
		{
		case 0:
			if ( length >= Q_ARRAYSIZE( buffer ) )
				length = Q_ARRAYSIZE( buffer ) - 1;

			memcpy( buffer, msg.c_str(), length );
			buffer[ length ] = '\0';
			break;

		case 1:
			snprintf( buffer, Q_ARRAYSIZE( buffer ), msg.c_str(), formattingStrings[ 0 ].c_str() );
			break;

		case 2:
			snprintf( buffer, Q_ARRAYSIZE( buffer ), msg.c_str(), formattingStrings[ 0 ].c_str(), formattingStrings[ 1 ].c_str() );
			break;

		case 3:
			snprintf( buffer, Q_ARRAYSIZE( buffer ), msg.c_str(), formattingStrings[ 0 ].c_str(), formattingStrings[ 1 ].c_str(), formattingStrings[ 2 ].c_str() );
			break;

		case 4:
			snprintf( buffer, Q_ARRAYSIZE( buffer ), msg.c_str(), formattingStrings[ 0 ].c_str(), formattingStrings[ 1 ].c_str(), formattingStrings[ 2 ].c_str(), formattingStrings[ 3 ].c_str() );
			break;
		}

		if ( buffer[ 0 ] != '\0' )
			THIS_FEATURE()->OnReceiveMessage( -1, buffer, 0 );

		return 0;
	}

#if 0
	return ORIG_UserMsgHook_TextMsg( pszUserMsg, iSize, pBuffer );
#else
	return ORIG_CHudTextMessage__MsgFunc_TextMsg( thisptr, pszUserMsg, iSize, pBuffer );
#endif
}

//-----------------------------------------------------------------------------
// Receive message event
//-----------------------------------------------------------------------------

void CChatHistory::OnReceiveMessage( int client, const char *pszMessage, int src )
{
	// Check for muted player
	if ( client > 0 )
	{
		const char *pszLevelName = cl_enginefuncs->pfnGetLevelName();
		cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();

		if ( pszLevelName && *pszLevelName && pLocal && pLocal->index != client )
		{
			// TODO
		}
	}

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

	float *pflClientColor = function_cast<float *( __cdecl * )( int )>( GameData::Pointers::Client::GetClientColor )( client );

	switch ( src )
	{
	case 0: // just message from the Server
	{
		ConColorMsg( m_pTextColor->GetColor32(), pszMessage );

		if ( bFoundNewlineChar )
			*pszNewlineChar = '\0';

		AddMessage( pszMessage, NULL, NULL );

		if ( bFoundNewlineChar )
			*pszNewlineChar = '\n';

		break;
	}

	case 1: // message from the Server's Console
	{
		const char *pszMessageSender = "<Server Console>";

		ConColorMsg( Color( pflClientColor[ 0 ], pflClientColor[ 1 ], pflClientColor[ 2 ], 1.f ), pszMessageSender );
		ConColorMsg( m_pTextColor->GetColor32(), pszMessage + strlen( pszMessageSender ) );

		if ( bFoundNewlineChar )
			*pszNewlineChar = '\0';

		AddMessage( pszMessage + strlen( pszMessageSender ), pszMessageSender, pflClientColor );

		if ( bFoundNewlineChar )
			*pszNewlineChar = '\n';

		break;
	}

	case 2: // message from the Player
	{
		player_info_t *pPlayerInfo = enginestudio->PlayerInfo( client - 1 );
		if ( pPlayerInfo == NULL )
			return;

		size_t namelen = strlen( pPlayerInfo->name );
		if ( strncmp( pPlayerInfo->name, pszMessage, namelen ) == 0 )
		{
			if ( bFoundNewlineChar )
				*pszNewlineChar = '\0';

			AddMessage( pszMessage + namelen, pPlayerInfo->name, pflClientColor );

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\n';

			ConColorMsg( Color( pflClientColor[ 0 ], pflClientColor[ 1 ], pflClientColor[ 2 ], 1.f ), pPlayerInfo->name );
			ConColorMsg( m_pTextColor->GetColor32(), pszMessage + namelen );
		}
		else
		{
			const char *msg = pszMessage + namelen + Q_ARRAYSIZE( "(TEAM)" );
			std::string sClientNameTeam = std::string( "(Team) " ) + pPlayerInfo->name;

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\0';

			AddMessage( msg, sClientNameTeam.c_str(), pflClientColor );

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\n';

			ConColorMsg( Color( pflClientColor[ 0 ], pflClientColor[ 1 ], pflClientColor[ 2 ], 1.f ), sClientNameTeam.c_str() );
			ConColorMsg( m_pTextColor->GetColor32(), msg );
		}

		break;
	}

	case 3: // action message from the Player
	{
		player_info_t *pPlayerInfo = enginestudio->PlayerInfo( client - 1 );
		if ( pPlayerInfo == NULL )
			return;

		size_t namelen = strlen( pPlayerInfo->name );
		if ( strncmp( "* (TEAM)", pszMessage, Q_ARRAYSIZE( "* (TEAM)" ) - 1 ) == 0 )
		{
			const char *msg = pszMessage + namelen + Q_ARRAYSIZE( "* (TEAM)" );
			std::string sClientNameTeam = std::string( "* (Team) " ) + pPlayerInfo->name;

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\0';

			AddMessage( msg, sClientNameTeam.c_str(), pflClientColor );

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\n';

			ConColorMsg( Color( pflClientColor[ 0 ], pflClientColor[ 1 ], pflClientColor[ 2 ], 1.f ), sClientNameTeam.c_str() );
			ConColorMsg( m_pTextColor->GetColor32(), msg );
		}
		else
		{
			const char *msg = pszMessage + namelen + Q_ARRAYSIZE( "*" );
			std::string sClientNameTeam = std::string( "* " ) + pPlayerInfo->name;

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\0';

			AddMessage( msg, sClientNameTeam.c_str(), pflClientColor );

			if ( bFoundNewlineChar )
				*pszNewlineChar = '\n';

			ConColorMsg( Color( pflClientColor[ 0 ], pflClientColor[ 1 ], pflClientColor[ 2 ], 1.f ), sClientNameTeam.c_str() );
			ConColorMsg( m_pTextColor->GetColor32(), msg );
		}

		break;
	}
	}

	// Play sound notification
	if ( *m_pSoundEngine != NULL )
	{
		m_pfnCClient_SoundEngine__Play2DSound( *m_pSoundEngine, SVENINT_FOLDER_NAME "/chat/talk.wav", 1.f );
	}
	//cl_enginefuncs->pfnPlaySoundByName( SVENINT_FOLDER_NAME "/chat/talk.wav", 1.f );
}

//-----------------------------------------------------------------------------
// Add incoming chat message to the text history
//-----------------------------------------------------------------------------

void CChatHistory::AddMessage( const char *pszMessage, const char *pszClientName, float *pflClientColor )
{
	float flReceiveTime = cl_enginefuncs->GetClientTime();
	m_textHistory.insert( m_textHistory.begin(), CChatHistoryMessage( pszMessage, pszClientName, pflClientColor, flReceiveTime ) );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CChatHistory::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		for ( CChatHistoryMessage &msg : m_textHistory )
			msg.m_flReceiveTime = -1.f;

		return kHookContinue;
	}

	if ( m_textHistory.size() > (size_t)m_pMaxHistory->GetInt() )
		m_textHistory.erase( m_textHistory.begin() + m_pMaxHistory->GetInt(), m_textHistory.end() );

	const bool bShowHistory = !!( *key_dest );
	const Color clrText = m_pTextColor->GetColor32();
	const int alpha = int( m_pTextOpacity->GetFloat() * 255.f );
	const float flTime = cl_enginefuncs->GetClientTime();

	int width, height;
	int x = int( (float)gameutils->GetScreenWidth() * m_pScreenWidthFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pScreenHeightFraction->GetFloat() );

	for ( CChatHistoryMessage &msg : m_textHistory )
	{
		if ( !bShowHistory && flTime - msg.m_flReceiveTime > m_pStayTime->GetFloat() )
			continue;

		int curx = x;

		if ( !msg.m_sClientName.empty() && msg.m_pflClientColor != NULL )
		{
			const Color clrClient( msg.m_pflClientColor[ 0 ], msg.m_pflClientColor[ 1 ], msg.m_pflClientColor[ 2 ], 1.f );

			Features::drawing->DrawStringEx( Features::drawing->GetFontChat(),
											 curx, y,
											 clrClient.r, clrClient.g, clrClient.b, alpha,
											 width, height,
											 FONT_ALIGN_LEFT,
											 msg.m_sClientName.c_str() );

			curx += width;
		}

		Features::drawing->DrawStringEx( Features::drawing->GetFontChat(),
										 curx, y,
										 clrText.r, clrText.g, clrText.b, alpha,
										 width, height,
										 FONT_ALIGN_LEFT,
										 msg.m_sMsg.c_str() );

		y += ( m_pAlignmentMode->GetInt() == 0 ? -height : height );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CChatHistory::CChatHistory( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pAlignmentMode = NULL;
	m_pMaxHistory = NULL;
	m_pStayTime = NULL;
	m_pScreenWidthFraction = NULL;
	m_pScreenHeightFraction = NULL;
	m_pTextOpacity = NULL;
	m_pTextColor = NULL;

	key_dest = NULL;
	m_pSoundEngine = NULL;
	m_pfnCHudTextMessage__MsgFunc_TextMsg = NULL;
	m_pfnCClient_SoundEngine__Play2DSound = NULL;

	m_hUserMsgHook_SayText = DETOUR_INVALID_HANDLE;
	m_hUserMsgHook_TextMsg = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CChatHistory::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CChatHistory::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CChatHistory::Load( void )
{
	ud_t inst;
	Modules::menu->BindFeature( this );

	m_pAlignmentMode = Modules::menu->AddParamList( this, "AlignmentMode", NULL, 0, " 0 - Draw from bottom to top\0 1 - Draw from top to bottom\0\0" );
	m_pMaxHistory = Modules::menu->AddParamInteger( this, "MaxHistory", NULL, 6, 1, 30 );
	m_pStayTime = Modules::menu->AddParamFloat( this, "StayTime", NULL, 5.f, 0.f, 120.f );
	m_pScreenWidthFraction = Modules::menu->AddParamFloat( this, "ScreenWidthFraction", NULL, 0.01f, 0.f, 1.f );
	m_pScreenHeightFraction = Modules::menu->AddParamFloat( this, "ScreenHeightFraction", NULL, 0.73f, 0.f, 1.f );
	m_pTextOpacity = Modules::menu->AddParamFloat( this, "TextOpacity", NULL, 1.f, 0.f, 1.f );
	m_pTextColor = Modules::menu->AddParamColorRGB( this, "TextColor", NULL, Color( 210, 200, 152, 255 ) );

	cmd_function_t *messagemode = cvar->FindCmd( "messagemode" );
	if ( messagemode == NULL )
		return false;

	MemoryUtils()->InitDisasm( &inst, messagemode->function, 32, 16 );
	if ( MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Imov &&
			 inst.operand[ 0 ].type == UD_OP_MEM &&
			 inst.operand[ 1 ].type == UD_OP_IMM &&
			 inst.operand[ 1 ].lval.udword == 1 )
		{
			key_dest = reinterpret_cast<int *>( inst.operand[ 0 ].lval.udword );
		}
	}

	FEATURE_CHECK_SYMBOL( key_dest, "key_dest" );

	int patternIndex;
	bool bOK = true;

	void *pfnSoundEngine;

	DEFINE_PATTERNS_FUTURE( fm_pSoundEngine );
	DEFINE_PATTERNS_FUTURE( fCHudTextMessage__MsgFunc_TextMsg );
	DEFINE_PATTERNS_FUTURE( fCClient_SoundEngine__Play2DSound );

	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::m_pSoundEngine, fm_pSoundEngine );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CHudTextMessage__MsgFunc_TextMsg, fCHudTextMessage__MsgFunc_TextMsg );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CClient_SoundEngine__Play2DSound, fCClient_SoundEngine__Play2DSound );

	pfnSoundEngine = MemoryUtils()->GetPatternFutureValue( fm_pSoundEngine, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( pfnSoundEngine,
										  "m_pSoundEngine",
										  FeaturesGameData::Patterns::Client::m_pSoundEngine,
										  patternIndex );
	
	m_pfnCHudTextMessage__MsgFunc_TextMsg = MemoryUtils()->GetPatternFutureValue( fCHudTextMessage__MsgFunc_TextMsg, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCHudTextMessage__MsgFunc_TextMsg,
										  "CHudTextMessage::MsgFunc_TextMsg",
										  FeaturesGameData::Patterns::Client::CHudTextMessage__MsgFunc_TextMsg,
										  patternIndex );
	
	m_pfnCClient_SoundEngine__Play2DSound = (CClient_SoundEngine__Play2DSoundFn)MemoryUtils()->GetPatternFutureValue( fCClient_SoundEngine__Play2DSound, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCClient_SoundEngine__Play2DSound,
										  "CClient_SoundEngine::Play2DSound",
										  FeaturesGameData::Patterns::Client::CClient_SoundEngine__Play2DSound,
										  patternIndex );

	if ( !bOK )
		return false;

	// Get pointer to sound engine
	m_pSoundEngine = (void **)*(uint32_t *)( (uint8_t *)pfnSoundEngine + 1 );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CChatHistory::PostLoad( void )
{
	m_hUserMsgHook_SayText = gamehooks->HookUserMessage( "SayText", UserMsgHook_SayText, &ORIG_UserMsgHook_SayText );
	//m_hUserMsgHook_TextMsg = gamehooks->HookUserMessage( "TextMsg", UserMsgHook_TextMsg, &ORIG_UserMsgHook_TextMsg );
	m_hUserMsgHook_TextMsg = Detours()->DetourFunction( m_pfnCHudTextMessage__MsgFunc_TextMsg,
														HOOKED_CHudTextMessage__MsgFunc_TextMsg,
														GET_FUNC_PTR( ORIG_CHudTextMessage__MsgFunc_TextMsg ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CChatHistory::Unload( void )
{
	gamehooks->UnhookUserMessage( m_hUserMsgHook_SayText );
	//gamehooks->UnhookUserMessage( m_hUserMsgHook_TextMsg );
	Detours()->RemoveDetour( m_hUserMsgHook_TextMsg );
}
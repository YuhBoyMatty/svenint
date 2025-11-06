// SvenInt (c) Sw1ft
// hud_chat_colors.cpp

#include "stdafx.h"
#include "hud_chat_colors.h"
#include "r_drawing.h"
#include "utils/util.h"

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
			DEFINE_PATTERNS_2( CHudBaseTextBlock__Print,
							   "5.25",
							   "55 8B EC 6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 53 56 57 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B D9 8B 0D",
							   "5.11",
							   "6A ? 68 ? ? ? ? 64 A1 ? ? ? ? 50 53 55 56 57 A1 ? ? ? ? 33 C4 50 8D 44 24 14 64 A3 ? ? ? ? 8B F9" );
		}
	}
}

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( void, CHudBaseTextBlock__Print, CHudBaseTextBlock *, const char *, int, int );
DECLARE_HOOK( float *, __cdecl, GetClientColor, int playerIndex );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CChatColors, chatcolors, "HUD", "Chat Colors" );

//-----------------------------------------------------------------------------
// CHudBaseTextBlock::Print hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_CHudBaseTextBlock__Print, CHudBaseTextBlock *thisptr, const char *pszBuf, int iBufSize, int clientIndex )
{
	THIS_FEATURE()->SaveHudBaseTextBlock( thisptr );

	ORIG_CHudBaseTextBlock__Print( thisptr, pszBuf, iBufSize, clientIndex );
}

//-----------------------------------------------------------------------------
// GetClientColor hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( float *, __cdecl, HOOKED_GetClientColor, int playerIndex )
{
	if ( THIS_FEATURE_IS_ENABLED() && playerIndex > 0 )
	{
		float *pflColor = THIS_FEATURE()->GetColor( playerIndex );
		if ( pflColor != NULL )
			return pflColor;
	}

	return ORIG_GetClientColor( playerIndex );
}

//-----------------------------------------------------------------------------
// Load players from a file
//-----------------------------------------------------------------------------

float *CChatColors::GetColor( int iPlayerIndex )
{
	int nTeamNumber = extraplayerinfo->GetTeamNumber( iPlayerIndex );
	const uint64_t steamID = gameutils->GetSteamID( iPlayerIndex );

	auto it = std::find_if( m_players.begin(), m_players.end(), [ steamID ]( const chat_colors_info_t &item )
	{
		return item.steamID == steamID;
	} );

	if ( it != m_players.end() )
	{
		switch ( it->ndxColor )
		{
		case 0:
			return GetRainbowColor();

		case 1:
			return m_pColor1->GetColor();

		case 2:
			return m_pColor2->GetColor();

		case 3:
			return m_pColor3->GetColor();

		case 4:
			return m_pColor4->GetColor();

		case 5:
			return m_pColor5->GetColor();

		case 6:
			return m_pColor6->GetColor();
		}
	}
	else if ( nTeamNumber < 1 || nTeamNumber > 4 )
	{
		return m_pDefaultColor->GetColor();
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Load players from a file
//-----------------------------------------------------------------------------

void CChatColors::LoadPlayers( void )
{
	static char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/chat_colors_players.txt", "r" );

	if ( file != NULL )
	{
		m_players.clear();
		int nLine = 0;

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			nLine++;

			char *buffer = UTIL_LStrip( (char *)szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer )
				continue;

			char *pszSteamID = strtok( buffer, ":" );
			char *pszColorNum = NULL;
			if ( pszSteamID != NULL )
				pszColorNum = strtok( NULL, ":" );

			if ( pszSteamID == NULL || pszColorNum == NULL )
				continue;

			pszSteamID = UTIL_LStrip( pszSteamID );
			UTIL_RStrip( pszSteamID );
			
			pszColorNum = UTIL_LStrip( pszColorNum );
			UTIL_RStrip( pszColorNum );

			uint64 steamID = atoll( pszSteamID );
			int teamNum = atoi( pszColorNum );
			if ( steamID != 0uLL )
				m_players.push_back( { steamID, teamNum } );
		}

		fclose( file );

		if ( !m_players.empty() )
		{
			std::sort( m_players.begin(), m_players.end(), []( const chat_colors_info_t &a, const chat_colors_info_t &b )
			{
				return a.steamID < b.steamID;
			} );
		}
	}
	else
	{
		PrintWarning( "Missing file \"./%s\"\n", SVENINT_FOLDER_NAME "/chat_colors_players.txt" );
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CChatColors::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_flRainbowUpdateTime = cl_enginefuncs->GetClientTime();
	}
	else if ( pEvent->GetType() == kHUD_Redraw_HookEvent )
	{
		if ( cl_enginefuncs->GetClientTime() < m_flRainbowUpdateTime )
			return kHookContinue;

		UTIL_HSL2RGB( m_flRainbowDelta,
					  m_pRainbowSaturation->GetFloat(),
					  m_pRainbowLightness->GetFloat(),
					  m_flRainbowColor[ 0 ], m_flRainbowColor[ 1 ], m_flRainbowColor[ 2 ] );

		m_flRainbowDelta += m_pRainbowHue->GetFloat();

		while ( m_flRainbowDelta > 1.0f )
			m_flRainbowDelta -= 1.0f;

		m_flRainbowUpdateTime = cl_enginefuncs->GetClientTime() + m_pRainbowUpdate->GetFloat();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CChatColors::CChatColors( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pDefaultColor = NULL;
	m_pColor1 = NULL;
	m_pColor2 = NULL;
	m_pColor3 = NULL;
	m_pColor4 = NULL;
	m_pColor5 = NULL;
	m_pColor6 = NULL;
	m_pRainbowUpdate = NULL;
	m_pRainbowHue = NULL;
	m_pRainbowSaturation = NULL;
	m_pRainbowLightness = NULL;

	m_pfnCHudBaseTextBlock__Print = NULL;
	m_hCHudBaseTextBlock__Print = DETOUR_INVALID_HANDLE;
	m_hGetClientColor = DETOUR_INVALID_HANDLE;

	m_pHudBaseTextBlock = NULL;

	m_flRainbowDelta = 0.0f;

	m_flRainbowColor[ 0 ] = 1.0f;
	m_flRainbowColor[ 1 ] = 0.0f;
	m_flRainbowColor[ 2 ] = 0.0f;

	m_flRainbowUpdateTime = -1.0f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CChatColors::OnEnable( void )
{
	LoadPlayers();
	m_flRainbowUpdateTime = cl_enginefuncs->GetClientTime();

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CChatColors::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CChatColors::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pDefaultColor = Modules::menu->AddParamColorRGB( this, "DefaultColor", NULL, Color( 0.6f, 0.75f, 1.f, 1.f ) );
	m_pColor1 = Modules::menu->AddParamColorRGB( this, "Color1", NULL, Color( 0.25f, 0.25f, 1.f, 1.f ) );
	m_pColor2 = Modules::menu->AddParamColorRGB( this, "Color2", NULL, Color( 1.f, 0.25f, 0.25f, 1.f ) );
	m_pColor3 = Modules::menu->AddParamColorRGB( this, "Color3", NULL, Color( 1.f, 1.f, 0.25f, 1.f ) );
	m_pColor4 = Modules::menu->AddParamColorRGB( this, "Color4", NULL, Color( 0.25f, 1.f, 0.25f, 1.f ) );
	m_pColor5 = Modules::menu->AddParamColorRGB( this, "Color5", NULL, Color( 1.f, 0.57f, 0.f, 1.f ) );
	m_pColor6 = Modules::menu->AddParamColorRGB( this, "Color6", NULL, Color( 1.f, 1.f, 1.f, 1.f ) );

	Modules::menu->AddElementSeparator( this );

	m_pRainbowUpdate = Modules::menu->AddParamFloat( this, "RainbowUpdateDelay", NULL, 0.05f, 0.f, 0.5f );
	m_pRainbowHue = Modules::menu->AddParamFloat( this, "RainbowHueDelta", NULL, 0.015f, 0.f, 0.5f );
	m_pRainbowSaturation = Modules::menu->AddParamFloat( this, "RainbowSaturation", NULL, 0.8f, 0.f, 1.f );
	m_pRainbowLightness = Modules::menu->AddParamFloat( this, "RainbowLightness", NULL, 0.5f, 0.f, 1.f );

	int patternIndex;
	DEFINE_PATTERNS_FUTURE( fCHudBaseTextBlock__Print );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CHudBaseTextBlock__Print, fCHudBaseTextBlock__Print );

	m_pfnCHudBaseTextBlock__Print = MemoryUtils()->GetPatternFutureValue( fCHudBaseTextBlock__Print, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS( m_pfnCHudBaseTextBlock__Print,
								   "CHudBaseTextBlock::Print",
								   FeaturesGameData::Patterns::Client::CHudBaseTextBlock__Print,
								   patternIndex );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CChatColors::PostLoad( void )
{
	m_hCHudBaseTextBlock__Print = Detours()->DetourFunction( m_pfnCHudBaseTextBlock__Print,
															 HOOKED_CHudBaseTextBlock__Print,
															 GET_FUNC_PTR( ORIG_CHudBaseTextBlock__Print ) );

	m_hGetClientColor = Detours()->DetourFunction( GameData::Pointers::Client::GetClientColor,
												   HOOKED_GetClientColor,
												   GET_FUNC_PTR( ORIG_GetClientColor ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CChatColors::Unload( void )
{
	// Flush the text block to prevent the game from crashing because of shared color pointers
	// Maybe just allocate color data and forget about that?
	if ( m_pHudBaseTextBlock != NULL )
		m_pHudBaseTextBlock->FlushText();

	Detours()->RemoveDetour( m_hCHudBaseTextBlock__Print );
	Detours()->RemoveDetour( m_hGetClientColor );
}
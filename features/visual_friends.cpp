// SvenInt (c) Sw1ft
// visual_friends.cpp

#include "stdafx.h"
#include "svenint.h"
#include "visual_friends.h"
#include "player_camhack.h"
#include "r_drawing.h"
#include "utils/util.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFriends, friends, "Visual", "Friends" );

//-----------------------------------------------------------------------------
// LoadFriendsFromFile
//-----------------------------------------------------------------------------

void CFriends::LoadFriendsFromFile( void )
{
	static char szBuffer[ 512 ];
	FILE *file = fopen( SVENINT_FOLDER_NAME "/friends.txt", "r" );

	if ( file != NULL )
	{
		m_friends.clear();
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
				m_friends.push_back( steamID );
		}

		fclose( file );

		if ( !m_friends.empty() )
			std::sort( m_friends.begin(), m_friends.end() );
	}
	else
	{
		PrintWarning( "Missing file \"./" SVENINT_FOLDER_NAME "/friends.txt\"\n" );
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CFriends::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// VGuiClientPanelPaint event
	if ( Features::camhack->IsEnabled() )
		return kHookContinue;

	int w, h;
	int lineOffset = 0;

	const int r = m_pColor->GetColor32().r;
	const int g = m_pColor->GetColor32().g;
	const int b = m_pColor->GetColor32().b;
	const int a = m_pColor->GetColor32().a;

	const int x = int( (float)gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() );
	const int y = int( (float)gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() );

	Features::drawing->DrawStringEx( Features::drawing->GetFontFriends(),
									 x,
									 y,
									 r, g, b, a,
									 w, h,
									 FONT_ALIGN_LEFT,
									 "Friends online:" );

	lineOffset += h;

	bool bFoundAnyFriend = false;
	for ( int i = 0; i < refparams->maxclients; i++ )
	{
		player_info_t *pPlayerInfo = enginestudio->PlayerInfo( i );
		if ( !pPlayerInfo || !pPlayerInfo->name || !pPlayerInfo->name[ 0 ] )
			continue;

		auto it = std::find( m_friends.begin(), m_friends.end(), gameutils->GetSteamID( i + 1 ) );
		if ( it != m_friends.end() )
		{
			Features::drawing->DrawStringExF( Features::drawing->GetFontFriends(),
											  x,
											  y + lineOffset,
											  r, g, b, a,
											  w, h,
											  FONT_ALIGN_LEFT,
											  "(%d) %s", i + 1, pPlayerInfo->name );

			lineOffset += h;
			bFoundAnyFriend = true;
		}
	}

	if ( !bFoundAnyFriend )
	{
		Features::drawing->DrawStringEx( Features::drawing->GetFontFriends(),
										 x,
										 y + lineOffset,
										 r, g, b, a,
										 w, h,
										 FONT_ALIGN_LEFT,
										 "No friends online" );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFriends::CFriends( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;
	m_pColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CFriends::OnEnable( void )
{
	LoadFriendsFromFile();
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CFriends::OnDisable( void )
{
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFriends::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clrFriends;
	UnpackRGBA( *(unsigned char *)&clrFriends.r,
				*(unsigned char *)&clrFriends.g,
				*(unsigned char *)&clrFriends.b,
				*(unsigned char *)&clrFriends.a,
				RGBA_DRAW_TEXT_DEFAULT );

	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.01f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.3f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGBA( this, "Color", NULL, clrFriends );

	return true;
}

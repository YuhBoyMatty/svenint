// SvenInt (c) Sw1ft
// visual_nofade.cpp

#include "stdafx.h"
#include "visual_nofade.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( int, __cdecl, V_FadeAlpha );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERNS_2( V_FadeAlpha,
							   "5.25",
							   "51 D9 ? ? ? ? ? DD ? ? ? ? ? 8A 0D",
							   "5.11",
							   "51 D9 ? ? ? ? ? 8A 0D ? ? ? ? DD 05" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CNoFade, nofade, "Visual", "No Fade" );

static UserMsgHookFn ORIG_UserMsgHook_ScreenFade = NULL;

//-----------------------------------------------------------------------------
// ScreenFade
//-----------------------------------------------------------------------------

static int UserMsgHook_ScreenFade( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		return 0;

	return ORIG_UserMsgHook_ScreenFade( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// V_FadeAlpha hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, __cdecl, HOOKED_V_FadeAlpha )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		return 0;

	return ORIG_V_FadeAlpha();
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CNoFade::CNoFade( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pfnV_FadeAlpha = NULL;
	m_hV_FadeAlpha = DETOUR_INVALID_HANDLE;
	m_hUserMsgHook_ScreenFade = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CNoFade::Load( void )
{
	Modules::menu->BindFeature( this );

	int patternIndex;

	DEFINE_PATTERNS_FUTURE( fV_FadeAlpha );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::V_FadeAlpha, fV_FadeAlpha );

	m_pfnV_FadeAlpha = MemoryUtils()->GetPatternFutureValue( fV_FadeAlpha, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS( m_pfnV_FadeAlpha,
								   "V_FadeAlpha",
								   FeaturesGameData::Patterns::Engine::V_FadeAlpha,
								   patternIndex );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CNoFade::PostLoad( void )
{
	m_hV_FadeAlpha = Detours()->DetourFunction( m_pfnV_FadeAlpha, HOOKED_V_FadeAlpha, GET_FUNC_PTR( ORIG_V_FadeAlpha ) );
	m_hUserMsgHook_ScreenFade = gamehooks->HookUserMessage( "ScreenFade", UserMsgHook_ScreenFade, &ORIG_UserMsgHook_ScreenFade );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CNoFade::Unload( void )
{
	Detours()->RemoveDetour( m_hV_FadeAlpha );
	gamehooks->UnhookUserMessage( m_hUserMsgHook_ScreenFade );
}
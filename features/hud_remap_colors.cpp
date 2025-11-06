// SvenInt (c) Sw1ft
// hud_remap_colors.cpp

#include "stdafx.h"
#include "hud_remap_colors.h"
#include "r_drawing.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( void, __cdecl, SPR_Set, HSPRITE_HL hPic, int r, int g, int b );
DECLARE_HOOK( void, __cdecl, FillRGBA, int x, int y, int width, int height, int r, int g, int b, int a );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CRemapColors, remapcolors, "HUD", "Remap Colors" );

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, __cdecl, HOOKED_SPR_Set, HSPRITE_HL hPic, int r, int g, int b )
{
	if ( THIS_FEATURE_IS_ENABLED() )
	{
		const Color clrRemap = THIS_FEATURE()->GetRemapColor();

		r = clrRemap.r;
		g = clrRemap.g;
		b = clrRemap.b;
	}

	ORIG_SPR_Set( hPic, r, g, b );
}

DECLARE_FUNC( void, __cdecl, HOOKED_FillRGBA, int x, int y, int width, int height, int r, int g, int b, int a )
{
	if ( THIS_FEATURE_IS_ENABLED() )
	{
		const Color clrRemap = THIS_FEATURE()->GetRemapColor();

		r = clrRemap.r;
		g = clrRemap.g;
		b = clrRemap.b;
	}

	ORIG_FillRGBA( x, y, width, height, r, g, b, a );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CRemapColors::CRemapColors( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pHUDColor = NULL;

	m_pfnSPR_Set = NULL;
	m_pfnFillRGBA = NULL;

	m_hSPR_Set = DETOUR_INVALID_HANDLE;
	m_hFillRGBA = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CRemapColors::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clr;
	UnpackRGB( *(unsigned char *)&clr.r,
			   *(unsigned char *)&clr.g,
			   *(unsigned char *)&clr.b,
			   RGB_YELLOWISH );

	m_pHUDColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, clr );

	m_pfnSPR_Set = cl_enginefuncs->pfnSPR_Set;
	if ( *(uint8_t *)m_pfnSPR_Set == 0xE9 ) // JMP
		m_pfnSPR_Set = MemoryUtils()->CalcAbsoluteAddress( m_pfnSPR_Set );
	
	m_pfnFillRGBA = cl_enginefuncs->pfnFillRGBA;
	if ( *(uint8_t *)m_pfnFillRGBA == 0xE9 ) // JMP
		m_pfnFillRGBA = MemoryUtils()->CalcAbsoluteAddress( m_pfnFillRGBA );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CRemapColors::PostLoad( void )
{
	m_hSPR_Set = Detours()->DetourFunction( m_pfnSPR_Set, HOOKED_SPR_Set, GET_FUNC_PTR( ORIG_SPR_Set ) );
	m_hFillRGBA = Detours()->DetourFunction( m_pfnFillRGBA, HOOKED_FillRGBA, GET_FUNC_PTR( ORIG_FillRGBA ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CRemapColors::Unload( void )
{
	Detours()->RemoveDetour( m_hSPR_Set );
	Detours()->RemoveDetour( m_hFillRGBA );
}
// SvenInt (c) Sw1ft
// menu.cpp

#include "stdafx.h"
#include "menu.h"
#include "config.h"
#include "svenint.h"

#include "utils/menu_styles.h"
#include "utils/menu_fonts.h"
#include "utils/util.h"

#include "features/base_feature.h"

#include "imgui.h"

#if IMGUI_USE_GL3
#include "imgui/backends/imgui_impl_opengl3.h"
#else
#include "imgui/backends/imgui_impl_opengl2.h"
#endif

#ifdef IMGUI_USE_SDL
#include <SDL.h>
#include "imgui/backends/imgui_impl_sdl2.h"
#else
#include "imgui/backends/imgui_impl_win32.h"
#endif

#ifndef WIN32
#include <time.h>
#include <dirent.h>
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

// ImGui's WndProc / SDL events handler
#ifdef IMGUI_USE_SDL
extern bool ImGui_ImplSDL2_ProcessEvent( const SDL_Event *event );
#else
extern LRESULT ImGui_ImplWin32_WndProcHandler( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
#endif

static bool processing_menu_elements = false;

namespace Modules { static CMenuModule menuModule; CMenuModule *menu = &menuModule; }

//-----------------------------------------------------------------------------
// Themes
//-----------------------------------------------------------------------------

static const char *font_items[] =
{
	( "SvenInt" ),
	( "L4D" )
};

static const char *theme_items[] =
{
	( "SvenInt" ),
	( "Dark" ),
	( "Light" ),
	( "Classic" ),
	( "Berserk" ),
	( "Deep Dark" ),
	( "Carbon" ),
	( "Corporate Grey" ),
	( "Grey" ),
	( "Dark Light" ),
	( "Soft Dark" ),
	( "Gold & Black" ),
	( "Monochrome" ),
	( "Pink" ),
	( "Half-Life" ),
	( "Sven-Cope" ),
	( "Custom" )
};

//-----------------------------------------------------------------------------
// Menu elements
//-----------------------------------------------------------------------------

CBaseMenuElement::CBaseMenuElement( int type, const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty )
{
	static int iMenuElementCount = 0;

	if ( pszName == NULL )
	{
		if ( pCfgProperty != NULL )
			pszName = pCfgProperty->GetName();
		else
			pszName = "";
	}

	m_type = type;
	m_bHasValue = false;
	m_pszName = pszName;
	m_pFeature = pFeature;
	m_pCfgProperty = pCfgProperty;
	m_pConVar = NULL;

	m_sInternalName = pszName;
	m_sInternalName += "##";
	m_sInternalName += pFeature->GetName();
	m_sInternalName += iMenuElementCount++;
}

void CBaseMenuElement::Revert( void )
{
	if ( m_pCfgProperty != NULL )
	{
		m_pCfgProperty->Revert();
		OnValueChanged();
	}
}

void CBaseMenuElement::OnValueChanged( void )
{
	if ( m_pConVar == NULL || !HasValue() )
		return;

	switch ( m_pCfgProperty->GetType() )
	{
	case kCfgPropertyInteger:
		m_pConVar->SetValue( m_pCfgProperty->GetInt() );
		break;

	case kCfgPropertyUInteger:
		m_pConVar->SetValue( m_pCfgProperty->GetUInt() );
		break;

	case kCfgPropertyFloat:
		m_pConVar->SetValue( m_pCfgProperty->GetFloat() );
		break;

	case kCfgPropertyBoolean:
		m_pConVar->SetValue( m_pCfgProperty->GetBool() );
		break;
		
	case kCfgPropertyCString:
		m_pConVar->SetValue( m_pCfgProperty->GetCString() );
		break;

	case kCfgPropertyColorRGB:
	case kCfgPropertyColorRGBA:
	{
		char buffer[ 24 ];
		float *pflColor = m_pCfgProperty->GetColor();

		if ( m_pCfgProperty->GetType() == kCfgPropertyColorRGBA )
		{
			snprintf( buffer,
					  Q_ARRAYSIZE( buffer ),
					  "%hhu %hhu %hhu %hhu",
					  (unsigned char)( pflColor[ 0 ] * 255.f ),
					  (unsigned char)( pflColor[ 1 ] * 255.f ),
					  (unsigned char)( pflColor[ 2 ] * 255.f ),
					  (unsigned char)( pflColor[ 3 ] * 255.f ) );

		}
		else
		{
			snprintf( buffer,
					  Q_ARRAYSIZE( buffer ),
					 "%hhu %hhu %hhu",
					  (unsigned char)( pflColor[ 0 ] * 255.f ),
					  (unsigned char)( pflColor[ 1 ] * 255.f ),
					  (unsigned char)( pflColor[ 2 ] * 255.f ) );

		}

		buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;
		m_pConVar->SetValue( buffer );
		break;
	}

	case kCfgPropertyVector:
	{
		char buffer[ 24 ];
		float *pflVector = m_pCfgProperty->GetVector();

		snprintf( buffer,
				  Q_ARRAYSIZE( buffer ),
				  "%.6f %.6f %.6f",
				  pflVector[ 0 ], pflVector[ 1 ], pflVector[ 2 ] );

		buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;
		m_pConVar->SetValue( buffer );
		break;
	}
	}
}

bool CMenuValueBool::GetBool( void )
{
	return GetCfgProperty()->GetBool();
}

int CMenuValueInteger::GetInt( void )
{
	return GetCfgProperty()->GetInt();
}

float CMenuValueFloat::GetFloat( void )
{
	return GetCfgProperty()->GetFloat();
}

Color CMenuValueColorRGB::GetColor32( void )
{
	float *pflColor = GetColor();
	return Color( pflColor[ 0 ], pflColor[ 1 ], pflColor[ 2 ], 1.f );
}

float *CMenuValueColorRGB::GetColor( void )
{
	return GetCfgProperty()->GetColor();
}

Color CMenuValueColorRGBA::GetColor32( void )
{
	float *pflColor = GetColor();
	return Color( pflColor[ 0 ], pflColor[ 1 ], pflColor[ 2 ], pflColor[ 3 ] );
}

float *CMenuValueColorRGBA::GetColor( void )
{
	return GetCfgProperty()->GetColor();
}

Vector &CMenuValueVector::GetVector( void )
{
	return *(Vector *)GetCfgProperty()->GetVector();
}

int CMenuValueList::GetInt( void )
{
	return GetCfgProperty()->GetInt();
}

const char *CMenuValueText::GetCString( void )
{
	return GetCfgProperty()->GetCString();
}

int CMenuValueText::GetSize( void )
{
	return GetCfgProperty()->GetCStringSize();
}

//-----------------------------------------------------------------------------
// CMenuFeature constructor
//-----------------------------------------------------------------------------

CMenuFeature::CMenuFeature()
{
	m_bToggleable = false;
	m_bShaderFeature = false;
	m_bPopupOpened = false;
	m_pFeature = NULL;
	m_pCfgEnabled = NULL;
	m_pCfgShowDiscoveryHint = NULL;
}

CMenuFeature::CMenuFeature( CBaseFeature *pFeature, bool bToggleable, bool bShaderFeature )
{
	m_bToggleable = bToggleable;
	m_bPopupOpened = false;
	m_bShaderFeature = bShaderFeature;
	m_pFeature = pFeature;
	m_pCfgEnabled = NULL;
	m_sInternalName = "params##";
	m_sInternalName += pFeature->GetName();

	m_pCfgShowDiscoveryHint = !bShaderFeature ? Modules::config->AddProperty( pFeature->GetName(), "ShowDiscoveryHint", true ) :
		Modules::config->AddShadersProperty( pFeature->GetName(), "ShowDiscoveryHint", true );

	if ( bToggleable )
	{
		m_pCfgEnabled = !bShaderFeature ? Modules::config->AddProperty( pFeature->GetName(), "Enable", pFeature->IsEnabled() ) :
			Modules::config->AddShadersProperty( pFeature->GetName(), "Enable", pFeature->IsEnabled() );
	}
}

//-----------------------------------------------------------------------------
// CMenuFeature constructor
//-----------------------------------------------------------------------------

bool CMenuModule::BindFeature( CBaseFeature *pFeature, bool bToggleable /* = true */ )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	if ( pszCategoryName == "Shaders" )
	{
		Warning( "[SvenInt::Menu] Tried to bind feature \"%s\" to category \"%s\" that is not allowed\n", pszFeatureName, "Shaders" );
		return false;
	}

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		category.m_features.push_back( { pFeature, bToggleable, false } );
		return true;
	}

	Warning( "[SvenInt::Menu] Tried to bind feature \"%s\" to non-existent category \"%s\"\n", pszFeatureName, pszCategoryName );
	return false;
}

bool CMenuModule::BindShaderFeature( CBaseFeature *pFeature, bool bToggleable /* = true */ )
{
	const char *pszCategoryName = "Shaders";
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		category.m_features.push_back( { pFeature, bToggleable, true } );
		return true;
	}

	Warning( "[SvenInt::Menu] Tried to bind feature \"%s\" to non-existent category \"%s\"\n", pszFeatureName, pszCategoryName );
	return false;
}

bool CMenuModule::BindConVar( CBaseMenuElement *pMenuElement, ConVar *pConvar )
{
	if ( pMenuElement == NULL )
		return false;
	
	if ( pConvar == NULL )
		return false;

	if ( !pMenuElement->HasValue() )
		return false;

	if ( pMenuElement->GetConVar() != NULL )
		return false;

	const MenuConVarBind_t findBind =
	{
		NULL,
		pConvar,
		pMenuElement
	};

	if ( !m_MenuConVarBinds.Insert( findBind ) )
		return false;

	pMenuElement->SetConVar( pConvar );
	return true;
}

CMenuValueBool *CMenuModule::AddParamBool( CBaseFeature *pFeature,
										   const char *pszCfgPropName,
										   const char *pszDescription,
										   bool bDefaultValue )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyBoolean, &bDefaultValue ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyBoolean, &bDefaultValue );

			CMenuValueBool *pValueBool = new( MemAlloc( sizeof( CMenuValueBool ) ) ) CMenuValueBool( pszDescription, pFeature, pProp );
			//CMenuValueBool *pValueBool = new CMenuValueBool( pszDescription, pFeature, pProp );

			feature.m_elements.push_back( pValueBool );
			return pValueBool;
		}
	}

	return NULL;
}

CMenuValueInteger *CMenuModule::AddParamInteger( CBaseFeature *pFeature,
												 const char *pszCfgPropName,
												 const char *pszDescription,
												 int iDefaultValue,
												 int iMin,
												 int iMax )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyInteger, &iDefaultValue ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyInteger, &iDefaultValue );

			CMenuValueInteger *pValueInt = new( MemAlloc( sizeof( CMenuValueInteger ) ) ) CMenuValueInteger( pszDescription, iMin, iMax, pFeature, pProp );
			//CMenuValueInteger *pValueInt = new CMenuValueInteger( pszDescription, iMin, iMax, pFeature, pProp );

			feature.m_elements.push_back( pValueInt );
			return pValueInt;
		}
	}

	return NULL;
}

CMenuValueFloat *CMenuModule::AddParamFloat( CBaseFeature *pFeature,
											 const char *pszCfgPropName,
											 const char *pszDescription,
											 float flDefaultValue,
											 float flMin,
											 float flMax )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyFloat, &flDefaultValue ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyFloat, &flDefaultValue );

			CMenuValueFloat *pValueFloat = new( MemAlloc( sizeof( CMenuValueFloat ) ) ) CMenuValueFloat( pszDescription, flMin, flMax, pFeature, pProp );
			//CMenuValueFloat *pValueFloat = new CMenuValueFloat( pszDescription, flMin, flMax, pFeature, pProp );

			feature.m_elements.push_back( pValueFloat );
			return pValueFloat;
		}
	}

	return NULL;
}

CMenuValueColorRGB *CMenuModule::AddParamColorRGB( CBaseFeature *pFeature,
												   const char *pszCfgPropName,
												   const char *pszDescription,
												   Color clrDefault )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			float pflColor[ 3 ];
			pflColor[ 0 ] = (float)clrDefault.r / 255.f;
			pflColor[ 1 ] = (float)clrDefault.g / 255.f;
			pflColor[ 2 ] = (float)clrDefault.b / 255.f;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyColorRGB, pflColor ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyColorRGB, pflColor );

			CMenuValueColorRGB *pValueColorRGB = new( MemAlloc( sizeof( CMenuValueColorRGB ) ) ) CMenuValueColorRGB( pszDescription, pFeature, pProp );
			//CMenuValueColorRGB *pValueColorRGB = new CMenuValueColorRGB( pszDescription, pFeature, pProp );

			feature.m_elements.push_back( pValueColorRGB );
			return pValueColorRGB;
		}
	}

	return NULL;
}

CMenuValueColorRGBA *CMenuModule::AddParamColorRGBA( CBaseFeature *pFeature,
													 const char *pszCfgPropName,
													 const char *pszDescription,
													 Color clrDefault )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			float pflColor[ 4 ];
			pflColor[ 0 ] = (float)clrDefault.r / 255.f;
			pflColor[ 1 ] = (float)clrDefault.g / 255.f;
			pflColor[ 2 ] = (float)clrDefault.b / 255.f;
			pflColor[ 3 ] = (float)clrDefault.a / 255.f;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyColorRGBA, pflColor ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyColorRGBA, pflColor );

			CMenuValueColorRGBA *pValueColorRGBA = new( MemAlloc( sizeof( CMenuValueColorRGBA ) ) ) CMenuValueColorRGBA( pszDescription, pFeature, pProp );
			//CMenuValueColorRGBA *pValueColorRGBA = new CMenuValueColorRGBA( pszDescription, pFeature, pProp );

			feature.m_elements.push_back( pValueColorRGBA );
			return pValueColorRGBA;
		}
	}

	return NULL;
}

CMenuValueVector *CMenuModule::AddParamVector( CBaseFeature *pFeature,
											   const char *pszCfgPropName,
											   const char *pszDescription,
											   Vector vecDefault,
											   float flStep,
											   float flMin,
											   float flMax )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyVector, &vecDefault ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyVector, &vecDefault );

			CMenuValueVector *pValueVector = new( MemAlloc( sizeof( CMenuValueVector ) ) ) CMenuValueVector( pszDescription, flStep, flMin, flMax, pFeature, pProp );
			//CMenuValueVector *pValueVector = new CMenuValueVector( pszDescription, flStep, flMin, flMax, pFeature, pProp );

			feature.m_elements.push_back( pValueVector );
			return pValueVector;
		}
	}

	return NULL;
}

CMenuValueList *CMenuModule::AddParamList( CBaseFeature *pFeature,
										   const char *pszCfgPropName,
										   const char *pszDescription,
										   int iDefaultValue,
										   const char *pszItems )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyInteger, &iDefaultValue ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyInteger, &iDefaultValue );

			CMenuValueList *pValueList = new( MemAlloc( sizeof( CMenuValueList ) ) ) CMenuValueList( pszDescription, pszItems, pFeature, pProp );
			//CMenuValueList *pValueList = new CMenuValueList( pszDescription, pszItems, pFeature, pProp );

			feature.m_elements.push_back( pValueList );
			return pValueList;
		}
	}

	return NULL;
}

CMenuValueText *CMenuModule::AddParamText( CBaseFeature *pFeature,
										   const char *pszCfgPropName,
										   const char *pszDescription,
										   const char *pszDefaultValue /* = NULL */ )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CConfigProperty *pProp = !feature.m_bShaderFeature ?
				Modules::config->AddProperty( pszFeatureName, pszCfgPropName, kCfgPropertyCString, (char *)pszDefaultValue ) :
				Modules::config->AddShadersProperty( pszFeatureName, pszCfgPropName, kCfgPropertyCString, (char *)pszDefaultValue );

			CMenuValueText *pValueText = new( MemAlloc( sizeof( CMenuValueText ) ) ) CMenuValueText( pszDescription, pFeature, pProp );
			//CMenuValueText *pValueText = new CMenuValueText( pszDescription, pFeature, pProp );

			feature.m_elements.push_back( pValueText );
			return pValueText;
		}
	}

	return NULL;
}

void CMenuModule::AddElementSeparator( CBaseFeature *pFeature, const char *pszTextSeparator /* = NULL */ )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CMenuElementSeparator *pSeparator = new( MemAlloc( sizeof( CMenuElementSeparator ) ) ) CMenuElementSeparator( pszTextSeparator, pFeature );
			//CMenuElementSeparator *pSeparator = new CMenuElementSeparator( pszTextSeparator, pFeature );
			feature.m_elements.push_back( pSeparator );
		}
	}
}

CMenuElementButton *CMenuModule::AddElementButton( CBaseFeature *pFeature, IMenuElementButtonCallback *pCallback, const char *pszDescription )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CMenuElementButton *pButton = new( MemAlloc( sizeof( CMenuElementButton ) ) ) CMenuElementButton( pszDescription, pCallback, pFeature );
			//CMenuElementButton *pButton = new CMenuElementButton( pszDescription, pCallback, pFeature );
			feature.m_elements.push_back( pButton );
			return pButton;
		}
	}

	return NULL;
}

void CMenuModule::AddElementResetButton( CBaseFeature *pFeature, const char *pszDescription )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CMenuElementResetButton *pResetButton = new( MemAlloc( sizeof( CMenuElementResetButton ) ) ) CMenuElementResetButton( pszDescription, pFeature );
			//CMenuElementResetButton *pResetButton = new CMenuElementResetButton( pszDescription, pFeature );
			feature.m_elements.push_back( pResetButton );
		}
	}
}

void CMenuModule::AddElementSameLine( CBaseFeature *pFeature )
{
	const char *pszCategoryName = pFeature->GetCategoryName();
	const char *pszFeatureName = pFeature->GetName();

	for ( CMenuCategory &category : m_categories )
	{
		if ( category.m_pszName != pszCategoryName )
			continue;

		for ( CMenuFeature &feature : category.m_features )
		{
			if ( feature.m_pFeature != pFeature )
				continue;

			CMenuElementSameLine *pSameLine = new( MemAlloc( sizeof( CMenuElementSameLine ) ) ) CMenuElementSameLine( NULL, pFeature );
			//CMenuElementSameLine *pSameLine = new CMenuElementSameLine( NULL, pFeature );
			feature.m_elements.push_back( pSameLine );
		}
	}
}

//-----------------------------------------------------------------------------
// WndProc hook
//-----------------------------------------------------------------------------

#ifndef IMGUI_USE_SDL
DECLARE_FUNC( LRESULT, CALLBACK, HOOKED_WndProc, HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if ( Modules::menu->WndProc( hWnd, uMsg, wParam, lParam ) )
		return 0;

	return CallWindowProc( Modules::menu->GetGameWindowProc(), hWnd, uMsg, wParam, lParam );
}
#endif

//-----------------------------------------------------------------------------
// SDL_PollEvent / WndProc event
//-----------------------------------------------------------------------------

#ifdef IMGUI_USE_SDL
bool CMenuModule::SDL_PollEvent( SDL_Event *event )
#else
bool CMenuModule::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
#endif
{
#ifdef IMGUI_USE_SDL
	if ( event->type == SDL_KEYDOWN && GetVirtualKey( event->key.keysym.sym ) == m_pCfgMenuKey->GetInt() )
#else
	if ( uMsg == WM_KEYDOWN && wParam == m_pCfgMenuKey->GetInt() )
#endif
	{
		m_bOpened = !m_bOpened;

		if ( m_bOpened )
			OnOpen();
		else
			OnClose();

	#ifdef IMGUI_USE_SDL
		SDL_WarpMouseInWindow( m_pSdlWindow, Globals::gameutils->GetScreenWidth() / 2, Globals::gameutils->GetScreenHeight() / 2 );
	#elif defined(WIN32)
		SetCursorPos( Globals::gameutils->GetScreenWidth() / 2, Globals::gameutils->GetScreenHeight() / 2 );
	#endif
		Globals::cl_funcs->IN_ClearStates();
	}

	if ( m_bOpened )
	{
	#ifdef IMGUI_USE_SDL
		ImGui_ImplSDL2_ProcessEvent( event );
	#else
		ImGui_ImplWin32_WndProcHandler( hWnd, uMsg, wParam, lParam );
	#endif
	}

	return false;
}

//-----------------------------------------------------------------------------
// SDL_GL_SwapWindow / wglSwapBuffers event
//-----------------------------------------------------------------------------

#ifdef IMGUI_USE_SDL
// DECLARE_FUNC( int, CALLCONV_CDECL, HOOKED_SDL_GL_SwapWindow, SDL_Window *window )
void CMenuModule::SDL_GL_SwapWindow( SDL_Window *window )
#else
void CMenuModule::wglSwapBuffers( HDC hdc )
#endif
{
	if ( !m_bImGuiInitialized )
	{
	#ifndef IMGUI_USE_SDL
		m_hGameWnd = WindowFromDC( hdc );
		m_hGameWndProc = (WNDPROC)SetWindowLong( m_hGameWnd, GWL_WNDPROC, (LONG)HOOKED_WndProc );
	#endif

		ImGui::CreateContext();

	#ifdef IMGUI_USE_SDL
		ImGui_ImplSDL2_InitForOpenGL( window, NULL );
	#else
		ImGui_ImplWin32_Init( m_hGameWnd );
	#endif

	#if IMGUI_USE_GL3
		ImGui_ImplOpenGL3_Init();
	#else
		ImGui_ImplOpenGL2_Init();
	#endif

		InitImGuiStyles();
		ImGui::StyleColorsDark();
		SaveCurrentStyle();

		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = NULL;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		InitFont();

		m_bImGuiInitialized = true;
	}

#ifdef IMGUI_USE_SDL
	m_pSdlWindow = window;
#endif

	if ( !m_bOpened && m_pCfgMenuHide->GetBool() )
		return;

#if IMGUI_USE_GL3
	ImGui_ImplOpenGL3_NewFrame();
#else
	ImGui_ImplOpenGL2_NewFrame();
#endif

#ifdef IMGUI_USE_SDL
	ImGui_ImplSDL2_NewFrame();
#else
	ImGui_ImplWin32_NewFrame();
#endif

	ImGui::NewFrame();

	Draw();

	ImGui::Render();

#if IMGUI_USE_GL3
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#else
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
#endif
}

//-----------------------------------------------------------------------------
// SDL_IsKeyPressed
//-----------------------------------------------------------------------------

#ifdef IMGUI_USE_SDL
bool CMenuModule::SDL_IsKeyPressed( uint32_t scancode )
{
    int numkeys;
    const Uint8 *state = SDL_GetKeyboardState( &numkeys );

    if ( scancode < (uint32_t)numkeys )
        return state[ scancode ];

    return false;
}
#endif

//-----------------------------------------------------------------------------
// GetVirtualKey
//-----------------------------------------------------------------------------

int CMenuModule::GetVirtualKey( int iWinApiOrSdl, bool bSDL /* = true */ )
{
#ifdef IMGUI_USE_SDL
	if ( bSDL )
	{
		int *winapikey = m_MapKeysSDLToWinAPI.Find( iWinApiOrSdl & ~SDLK_SCANCODE_MASK );
		if ( winapikey == NULL )
			return 0;

		return *winapikey;
	}

	int *sdlkey = m_MapKeysWinAPIToSDL.Find( iWinApiOrSdl );
	if ( sdlkey == NULL )
		return 0;

	return *sdlkey;
#else
	return 0;
#endif
}

//-----------------------------------------------------------------------------
// Freeze camera
//-----------------------------------------------------------------------------

void CMenuModule::KeepCameraFrozen( void )
{
	Globals::cl_enginefuncs->SetViewAngles( m_va );
}

//-----------------------------------------------------------------------------
// Open event
//-----------------------------------------------------------------------------

void CMenuModule::OnOpen( void )
{
	Globals::cl_enginefuncs->GetViewAngles( m_va );
	m_flOpenTime = Globals::cl_enginefuncs->GetClientTime();
}

//-----------------------------------------------------------------------------
// Close event
//-----------------------------------------------------------------------------

void CMenuModule::OnClose( void )
{
	m_flCloseTime = Globals::cl_enginefuncs->GetClientTime();
}

//-----------------------------------------------------------------------------
// OnVidInit
//-----------------------------------------------------------------------------

void CMenuModule::OnVidInit( void )
{
	m_flRainbowUpdateTime = m_flOpenTime = m_flCloseTime = -1.f;
}

//-----------------------------------------------------------------------------
// OnCvarChange
//-----------------------------------------------------------------------------

void CMenuModule::OnCvarChange( cvar_t *pCvar, const char *pszOldValue, float flOldValue )
{
	if ( processing_menu_elements )
		return;

	const MenuConVarBind_t findBind =
	{
		pCvar,
		NULL,
		NULL
	};

	MenuConVarBind_t *bind = m_MenuConVarBinds.Find( findBind );
	if ( bind == NULL )
		return;

	ConVar *pConVar = bind->convar;
	CBaseMenuElement *pMenuElement = bind->menuElement;

	if ( pMenuElement == NULL || pConVar == NULL )
		return;

	if ( !pMenuElement->HasValue() || pMenuElement->GetCfgProperty() == NULL )
		return;

	pConVar->Clamp();

	CConfigProperty *pCfgProperty = pMenuElement->GetCfgProperty();
	switch ( pCfgProperty->GetType() )
	{
	case kCfgPropertyInteger:
	{
		if ( pMenuElement->GetType() == kMenuElementList )
		{
			CMenuValueList *pValueList = static_cast<CMenuValueList *>( pMenuElement );

			int items_count = 0;
			const char *p = pValueList->GetItems();
			while ( *p )
			{
				p += strlen( p ) + 1;
				items_count++;
			}

			*pCfgProperty->GetIntRef() = Q_clamp( pConVar->GetInt(), 0, items_count );
		}
		else
		{
			CMenuValueInteger *pValueInt = static_cast<CMenuValueInteger *>( pMenuElement );
			*pCfgProperty->GetIntRef() = Q_clamp( pConVar->GetInt(), pValueInt->GetMin(), pValueInt->GetMax() );
		}
		break;
	}

	case kCfgPropertyUInteger:
		*pCfgProperty->GetUIntRef() = (unsigned int)pConVar->GetInt();
		break;

	case kCfgPropertyFloat:
	{
		CMenuValueFloat *pValueFloat = static_cast<CMenuValueFloat *>( pMenuElement );
		*pCfgProperty->GetFloatRef() = Q_clamp( pConVar->GetFloat(), pValueFloat->GetMin(), pValueFloat->GetMax() );
		break;
	}

	case kCfgPropertyBoolean:
		*pCfgProperty->GetBoolRef() = pConVar->GetBool();
		break;

	case kCfgPropertyCString:
		pCfgProperty->CopyStringFrom( pConVar->GetString() );
		break;

	case kCfgPropertyColorRGB:
	case kCfgPropertyColorRGBA:
	{
		Color clr = pConVar->GetColor();
		pCfgProperty->GetColor()[ 0 ] = (float)clr.r / 255.f;
		pCfgProperty->GetColor()[ 1 ] = (float)clr.g / 255.f;
		pCfgProperty->GetColor()[ 2 ] = (float)clr.b / 255.f;
		if ( pCfgProperty->GetType() == kCfgPropertyColorRGBA )
			pCfgProperty->GetColor()[ 3 ] = (float)clr.a / 255.f;
		break;
	}

	case kCfgPropertyVector:
	{
		float vec[ 3 ];
		int nParamsRead = sscanf( pConVar->GetString(), "%f %f %f", vec, vec + 1, vec + 2 );
		if ( nParamsRead >= 3 )
		{
			pCfgProperty->GetVector()[ 0 ] = vec[ 0 ];
			pCfgProperty->GetVector()[ 1 ] = vec[ 1 ];
			pCfgProperty->GetVector()[ 2 ] = vec[ 2 ];
		}
		else if ( nParamsRead == 2 )
		{
			pCfgProperty->GetVector()[ 0 ] = vec[ 0 ];
			pCfgProperty->GetVector()[ 1 ] = vec[ 1 ];
		}
		else if ( nParamsRead == 1 )
		{
			pCfgProperty->GetVector()[ 0 ] = vec[ 0 ];
		}
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Draw menu
//-----------------------------------------------------------------------------

void CMenuModule::Draw( void )
{
	if ( !m_bThemeLoaded )
	{
		LoadSavedStyle();
		LoadMenuTheme( m_pCfgMenuTheme->GetInt() );
		WindowStyle();

		RemoveEmptyCategories();

		m_bThemeLoaded = true;
	}

	UpdateRainbow();
	UpdateConfigs();
	UpdateShaderConfigs();

	int MainWindowFlags = ImGuiWindowFlags_HorizontalScrollbar |
		ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoTitleBar;

	ImFont *pDefaultFont = GetDefaultFont();
	const bool bRainbowMenuWasEnabled = m_pCfgMenuRainbowEnable->GetBool();
	const float flMainWidth = ImGui::GetIO().DisplaySize.x;
	const float flMainHeight = 1.f; // m_height / 60.f;

	ImGui::GetIO().MouseDrawCursor = m_bOpened && !Globals::gameUI->IsGameUIActive();
	ImGui::GetStyle().Alpha = m_pCfgMenuOpacity->GetFloat();

	if ( m_pCfgMenuRainbowEnable->GetBool() )
	{
		float alpha = m_pCfgMenuRainbowAlpha->GetFloat();
		ImVec4 clr( VectorExpand( *(Vector *)m_flRainbowColor ), alpha );
		ImGui::PushStyleColor( ImGuiCol_WindowBg, clr );
		ImGui::PushStyleColor( ImGuiCol_Separator, clr );
	}

	ImGui::SetNextWindowPos( { 0, 0 } );
	ImGui::SetNextWindowSize( { flMainWidth, flMainHeight } );
	ImGui::Begin( "Main", 0, MainWindowFlags );
	ImGui::PushFont( pDefaultFont );
	
	if ( ImGui::BeginMenuBar() )
	{
		if ( pDefaultFont == m_pFontL4D )
			ImGui::SetCursorPosY( ImGui::GetCursorPos().y + pDefaultFont->LegacySize * 0.25f );

		ImGui::TextUnformatted( "SvenInt v" SVENINT_VERSION_STRING );

		if ( m_pCfgMenuRainbowEnable->GetBool() )
		{
			float alpha = m_pCfgMenuRainbowAlpha->GetFloat();
			ImVec4 clr( VectorExpand( *(Vector *)m_flRainbowColor ), alpha );
			ImGui::PushStyleColor( ImGuiCol_Text, clr );
			ImGui::TextUnformatted( "  |" );
			ImGui::PopStyleColor();
		}
		else
		{
			ImGui::TextUnformatted( "  |" );
		}

		for ( CMenuCategory &category : m_categories )
		{
			if ( !ImGui::BeginMenu( category.m_pszName, m_bOpened ) )
			{
				bool bShowDiscoveryHint = false;

				for ( CMenuFeature &feature : category.m_features )
				{
					const bool bLoaded = feature.m_pFeature->IsLoaded();
					if ( bLoaded && feature.m_pCfgShowDiscoveryHint->GetBool() )
					{
						bShowDiscoveryHint = true;
						break;
					}
				}

				if ( bShowDiscoveryHint )
					DrawIndicatingDot( 1.f, 0.1f, 4.5f, 232, 232, 48, 255 );

				continue;
			}

			int iFeaturesToDiscover = 0;
			DrawFeatures( category, &iFeaturesToDiscover );

			ImGui::EndMenu();

			if ( iFeaturesToDiscover > 0 )
				DrawIndicatingDot( 1.f, 0.1f, 4.5f, 232, 232, 48, 255 );
		}

		DrawPrivateCategories();
		DrawMiscInfo();

		ImGui::EndMenuBar();
	}

	ImGui::PopFont();
	ImGui::End();

	if ( bRainbowMenuWasEnabled )
	{
		ImGui::PopStyleColor( 2 );
	}
}

//-----------------------------------------------------------------------------
// Draw features from the menu category
//-----------------------------------------------------------------------------

void CMenuModule::DrawFeatures( CMenuCategory &category, int *plFeaturesToDiscover )
{
	if ( category.m_features.empty() )
		return;

	constexpr ImVec4 enabledClr( 80.f / 255.f, 255.f / 255.f, 80.f / 255.f, 255.f / 255.f );
	constexpr ImVec4 disabledClr( 255.f / 255.f, 80.f / 255.f, 80.f / 255.f, 255.f / 255.f );

	for ( CMenuFeature &feature : category.m_features )
	{
		const bool bLoaded = feature.m_pFeature->IsLoaded();
		if ( !bLoaded )
		{
			ImGui::BeginDisabled();
		}
		else if ( !feature.m_bToggleable )
		{
			ImGui::PushStyleColor( ImGuiCol_ButtonHovered, ImGui::GetStyle().Colors[ ImGuiCol_Button ] );
			ImGui::PushStyleColor( ImGuiCol_ButtonActive, ImGui::GetStyle().Colors[ ImGuiCol_Button ] );
		}

		ImGui::PushStyleColor( ImGuiCol_Text, feature.m_pFeature->IsEnabled() ? enabledClr : disabledClr );
		if ( ImGui::Button( feature.m_pFeature->GetName(), ImVec2( ImGui::GetContentRegionAvail().x, 0.0 ) ) && feature.m_bToggleable )
		{
			feature.m_pFeature->Toggle();

			if ( feature.m_pCfgEnabled != NULL )
				*feature.m_pCfgEnabled = feature.m_pFeature->IsEnabled();
		}
		ImGui::PopStyleColor();

		if ( !bLoaded )
		{
			ImGui::EndDisabled();
		}
		else if ( !feature.m_bToggleable )
		{
			ImGui::PopStyleColor( 2 );
		}

		if ( ImGui::IsItemHovered() )
		{
			if ( !feature.m_elements.empty() && ImGui::IsMouseClicked( ImGuiMouseButton_Right ) )
				feature.m_bPopupOpened = true;

			*feature.m_pCfgShowDiscoveryHint->GetBoolRef() = false;
		}

		if ( bLoaded && feature.m_pCfgShowDiscoveryHint->GetBool() )
		{
			if ( plFeaturesToDiscover != NULL )
				*plFeaturesToDiscover += 1;

			DrawIndicatingDot( 1.f, 0.2f, 4.5f, 232, 232, 48, 255 );
		}

		if ( feature.m_bPopupOpened )
			ImGui::OpenPopup( feature.m_sInternalName.c_str() );

		if ( ImGui::BeginPopup( feature.m_sInternalName.c_str() ) )
		{
			DrawFeatureElements( feature );

			ImGui::EndPopup();
			feature.m_bPopupOpened = false;
		}

		if ( !feature.m_elements.empty() )
			ImGui::Separator();
	}
}

//-----------------------------------------------------------------------------
// Draw menu elements from the feature
//-----------------------------------------------------------------------------

void CMenuModule::DrawFeatureElements( CMenuFeature &feature )
{
	processing_menu_elements = true;

	for ( CBaseMenuElement *element : feature.m_elements )
	{
		switch ( element->GetType() )
		{
		case kMenuElementCheckbox:
		{
			CMenuValueBool *pValueBool = static_cast<CMenuValueBool *>( element );
			if ( ImGui::Checkbox( pValueBool->GetInternalName(), pValueBool->GetCfgProperty()->GetBoolRef() ) )
			{
				pValueBool->OnValueChanged();
			}
			break;
		}

		case kMenuElementIntegerSlider:
		{
			CMenuValueInteger *pValueInteger = static_cast<CMenuValueInteger *>( element );
			if ( ImGui::SliderInt( pValueInteger->GetInternalName(),
									 pValueInteger->GetCfgProperty()->GetIntRef(),
									 pValueInteger->GetMin(),
									 pValueInteger->GetMax() ) )
			{
				pValueInteger->OnValueChanged();
			}
			break;
		}

		case kMenuElementFloatSlider:
		{
			CMenuValueFloat *pValueFloat = static_cast<CMenuValueFloat *>( element );
			if ( ImGui::SliderFloat( pValueFloat->GetInternalName(),
									 pValueFloat->GetCfgProperty()->GetFloatRef(),
									 pValueFloat->GetMin(),
									 pValueFloat->GetMax() ) )
			{
				pValueFloat->OnValueChanged();
			}
			break;
		}

		case kMenuElementColorPickerRGB:
		{
			CMenuValueColorRGB *pValueRGB = static_cast<CMenuValueColorRGB *>( element );
			if ( ImGui::ColorEdit3( pValueRGB->GetInternalName(), pValueRGB->GetCfgProperty()->GetColor() ) )
			{
				pValueRGB->OnValueChanged();
			}
			break;
		}

		case kMenuElementColorPickerRGBA:
		{
			CMenuValueColorRGBA *pValueRGBA = static_cast<CMenuValueColorRGBA *>( element );
			if ( ImGui::ColorEdit4( pValueRGBA->GetInternalName(), pValueRGBA->GetCfgProperty()->GetColor() ) )
			{
				pValueRGBA->OnValueChanged();
			}
			break;
		}
		
		case kMenuElementVectorDrag:
		{
			CMenuValueVector *pValueVector = static_cast<CMenuValueVector *>( element );
			if ( ImGui::DragFloat3( pValueVector->GetInternalName(),
									 pValueVector->GetCfgProperty()->GetVector(),
									 pValueVector->GetStep(),
									 pValueVector->GetMin(),
									 pValueVector->GetMax() ) )
			{
				pValueVector->OnValueChanged();
			}
			break;
		}

		case kMenuElementList:
		{
			CMenuValueList *pValueList = static_cast<CMenuValueList *>( element );
			if ( ImGui::Combo( pValueList->GetInternalName(), pValueList->GetCfgProperty()->GetIntRef(), pValueList->GetItems() ) )
			{
				pValueList->OnValueChanged();
			}
			break;
		}
		
		case kMenuElementInputText:
		{
			CMenuValueText *pValueText = static_cast<CMenuValueText *>( element );
			if ( ImGui::InputText( pValueText->GetInternalName(), pValueText->GetCfgProperty()->GetCString(), pValueText->GetCfgProperty()->GetCStringSize() ) )
			{
				pValueText->OnValueChanged();
			}
			break;
		}

		case kMenuElementSeparator:
		{
			CMenuElementSeparator *pSeparator = static_cast<CMenuElementSeparator *>( element );
			if ( pSeparator->GetName() != NULL )
				ImGui::SeparatorText( pSeparator->GetName() );
			else
				ImGui::Separator();
			break;
		}

		case kMenuElementButton:
		{
			CMenuElementButton *pButton = static_cast<CMenuElementButton *>( element );
			if ( ImGui::Button( pButton->GetInternalName() ) )
			{
				pButton->GetCallback()->OnButtonPressed( pButton );
			}
			break;
		}

		case kMenuElementResetButton:
		{
			CMenuElementResetButton *pValueResetButton = static_cast<CMenuElementResetButton *>( element );
			if ( ImGui::Button( pValueResetButton->GetInternalName() ) )
			{
				if ( pValueResetButton->GetFeature()->GetCategoryName() == "Shaders" )
					Modules::config->RevertShaderSectionProperties( pValueResetButton->GetFeature()->GetName() );
				else
					Modules::config->RevertSectionProperties( pValueResetButton->GetFeature()->GetName() );

				for ( CBaseMenuElement *obj : feature.m_elements )
				{
					obj->OnValueChanged();
				}
			}
			break;
		}

		case kMenuElementSameLine:
		{
			ImGui::SameLine();
			break;
		}
		}
	}

	processing_menu_elements = false;
}

//-----------------------------------------------------------------------------
// Draw config, settings
//-----------------------------------------------------------------------------

void CMenuModule::DrawPrivateCategories( void )
{
	if ( ImGui::BeginMenu( "Config", m_bOpened ) )
	{
		DrawConfigs();
		DrawShaderConfigs();

		ImGui::EndMenu();
	}

	if ( ImGui::BeginMenu( "Settings", m_bOpened ) )
	{
		ImGui::Text( "Toggle Key" );

		ImGui::Spacing();

		if ( ImGui::Button( "Use Insert" ) )
			*m_pCfgMenuKey = VK_INSERT;

		ImGui::SameLine();

		if ( ImGui::Button( "Use Delete" ) )
			*m_pCfgMenuKey = VK_DELETE;

		ImGui::SameLine();

		if ( ImGui::Button( "Use Home" ) )
			*m_pCfgMenuKey = VK_HOME;

		ImGui::Spacing(); ImGui::Spacing();

		ImGui::Checkbox( "Hide In-active Menu", m_pCfgMenuHide->GetBoolRef() );
		
		ImGui::Spacing(); ImGui::Spacing();

		ImGui::SliderFloat( "Menu Opacity", m_pCfgMenuOpacity->GetFloatRef(), 0.1f, 1.f );

		ImGui::Spacing(); ImGui::Spacing();

		ImGui::Combo( "Font##menu", m_pCfgMenuFont->GetIntRef(), font_items, IM_ARRAYSIZE( font_items ) );

		ImGui::Spacing(); ImGui::Spacing();

		if ( ImGui::Combo( "Theme##menu", m_pCfgMenuTheme->GetIntRef(), theme_items, IM_ARRAYSIZE( theme_items ) ) )
		{
			LoadSavedStyle();
			LoadMenuTheme( m_pCfgMenuTheme->GetInt() );
			WindowStyle();
		}

		DrawCustomizableTheme();
		DrawCustomizableRainbowColor();

		ImGui::EndMenu();
	}

	if ( m_bUpdateAvailable && m_pszChangelog != NULL )
	{
		if ( ImGui::BeginMenu( "What's New?", m_bOpened ) )
		{
			ImGui::TextUnformatted( m_pszChangelog );

			ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

			if ( ImGui::Button( "GitHub Releases" ) )
			{
			#ifdef WIN32
				ShellExecute( NULL, "open", "https://github.com/sw1ft747/svenint/releases", NULL, NULL, SW_SHOWNORMAL );
			#else
				system( "xdg-open https://github.com/sw1ft747/svenint/releases &" );
			#endif
			}

			ImGui::EndMenu();
		}

		DrawIndicatingDot( 1.f, 0.1f, 4.5f, 232, 232, 48, 255 );
	}
}

//-----------------------------------------------------------------------------
// Draw fps, time, etc..
//-----------------------------------------------------------------------------

void CMenuModule::DrawMiscInfo( void )
{
	char rgszBuffer[ 128 ];
	float flLastTextWidth = 0.f, flTextWidth;

	// FPS
	flTextWidth = ImGui::CalcTextSize( "FPS" ).x;
	ImGui::SetCursorPosX( ImGui::GetWindowWidth() - flLastTextWidth - flTextWidth - ImGui::GetStyle().ItemSpacing.x );
	ImGui::TextUnformatted( "FPS" );
	flLastTextWidth = flTextWidth;

	// FPS val
	int fps = (int)ImGui::GetIO().Framerate;
	ImU32 clrFps = IM_COL32( 60, 255, 60, 255 );
	if ( fps < 30 )
		clrFps = IM_COL32( 255, 60, 60, 255 );
	else if ( fps < 60 )
		clrFps = IM_COL32( 255, 255, 60, 255 );

	snprintf( rgszBuffer, Q_ARRAYSIZE( rgszBuffer ), "%d", fps );
	flTextWidth = ImGui::CalcTextSize( rgszBuffer ).x;
	ImGui::SetCursorPosX( ImGui::GetWindowWidth() - flLastTextWidth - flTextWidth - ImGui::GetStyle().ItemSpacing.x * 1.5f );
	ImGui::PushStyleColor( ImGuiCol_Text, clrFps );
	ImGui::TextUnformatted( rgszBuffer );
	ImGui::PopStyleColor();

	flLastTextWidth = ImGui::CalcTextSize( "1000" ).x + flLastTextWidth;

	// Time
#ifdef WIN32
	SYSTEMTIME systime;
	GetLocalTime( &systime );
#else
	time_t now = time( NULL );
	struct tm *systime = localtime( &now );
#endif

	flTextWidth = ImGui::CalcTextSize( "Time: 00:00:00" ).x;
	ImGui::SetCursorPosX( ImGui::GetWindowWidth() - flLastTextWidth - flTextWidth - ImGui::GetStyle().ItemSpacing.x * 1.2f );
#ifdef WIN32
	ImGui::Text( "Time: %02d:%02d:%02d", systime.wHour, systime.wMinute, systime.wSecond );
#else
	ImGui::Text( "Time: %02d:%02d:%02d", systime->tm_hour, systime->tm_min, systime->tm_sec );
#endif

	// Consumed memory
	flLastTextWidth = flLastTextWidth + flTextWidth;
	flTextWidth = ImGui::CalcTextSize( "Memory: 00000 KB" ).x;
	ImGui::SetCursorPosX( ImGui::GetWindowWidth() - flLastTextWidth - flTextWidth - ImGui::GetStyle().ItemSpacing.x * 2.f );
	ImGui::Text( "Memory: %d KB", MemConsumed() / 1024 );
}

//-----------------------------------------------------------------------------
// Draw indicating dot on top of an menu item
//-----------------------------------------------------------------------------

void CMenuModule::DrawIndicatingDot( float flAnchorX, float flAnchorY, float flRadius, int r, int g, int b, int a )
{
	const ImVec2 vecMins = ImGui::GetItemRectMin();
	const ImVec2 vecMaxs = ImGui::GetItemRectMax();

	const float flPosX = vecMins.x + ( vecMaxs.x - vecMins.x ) * flAnchorX;
	const float flPosY = vecMins.y + ( vecMaxs.y - vecMins.y ) * flAnchorY;
	const ImVec2 vecPos = ImVec2( flPosX, flPosY );

	const float flPulse = ( sinf( (float)ImGui::GetTime() * 4.f ) * 0.5f ) + 0.5f;
	const ImU32 clrDot = IM_COL32( r, g, b, a );
	const ImU32 clrGlow = IM_COL32( r, g, b, (int)( (float)a * flPulse ) );

	ImGui::GetWindowDrawList()->AddCircleFilled( vecPos, flRadius, clrGlow );
	ImGui::GetWindowDrawList()->AddCircleFilled( vecPos, flRadius / 1.5f, clrDot );
}

//-----------------------------------------------------------------------------
// Draw customizable theme
//-----------------------------------------------------------------------------

void CMenuModule::DrawCustomizableTheme( void )
{
	if ( m_pCfgMenuTheme->GetInt() != 16 )
		return;

	ImGui::Spacing();

	if ( ImGui::BeginCombo( "##customtheme", "Customize Theme", ImGuiComboFlags_HeightLargest ) )
	{
		auto ImGui_ColorPickerU32 = []( const char *label, ImU32 *color, ImGuiColorEditFlags flags )
		{
			ImColor rgba = ImGui::ColorConvertU32ToFloat4( *view_as<ImU32 *>( color ) );

			ImGui::ColorEdit4( label, (float *)&rgba, flags );

			ImU32 out = ImGui::ColorConvertFloat4ToU32( rgba );

			*color = out;
		};

		if ( ImGui::Button( "Reset##custom_style" ) )
		{
			Modules::config->RevertSectionProperties( "CustomTheme" );
		}

		ImGui::Spacing(); ImGui::Spacing();

		ImGui_ColorPickerU32( "WindowBg Color##customtheme", m_CustomThemeVars.WindowBg->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "Border Color##customtheme", m_CustomThemeVars.Border->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "Button Color##customtheme", m_CustomThemeVars.Button->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "ButtonActive Color##customtheme", m_CustomThemeVars.ButtonActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "ButtonHovered Color##customtheme", m_CustomThemeVars.ButtonHovered->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "FrameBg Color##customtheme", m_CustomThemeVars.FrameBg->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "FrameBgActive Color##customtheme", m_CustomThemeVars.FrameBgActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "FrameBgHovered Color##customtheme", m_CustomThemeVars.FrameBgHovered->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "Text Color##customtheme", m_CustomThemeVars.Text->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "ChildBg Color##customtheme", m_CustomThemeVars.ChildBg->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "CheckMark Color##customtheme", m_CustomThemeVars.CheckMark->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "SliderGrab Color##customtheme", m_CustomThemeVars.SliderGrab->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "SliderGrabActive Color##customtheme", m_CustomThemeVars.SliderGrabActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "Header Color##customtheme", m_CustomThemeVars.Header->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "HeaderHovered Color##customtheme", m_CustomThemeVars.HeaderHovered->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "HeaderActive Color##customtheme", m_CustomThemeVars.HeaderActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "ResizeGripActive Color##customtheme", m_CustomThemeVars.ResizeGripActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "Separator Color##customtheme", m_CustomThemeVars.Separator->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "SeparatorActive Color##customtheme", m_CustomThemeVars.SeparatorActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );
		ImGui_ColorPickerU32( "TitleBgActive Color##customtheme", m_CustomThemeVars.TitleBgActive->GetUIntRef(), ImGuiColorEditFlags_AlphaBar );

		StyleColors_Custom(); // Not very optimal but should be fine

		ImGui::EndCombo();
	}
}

//-----------------------------------------------------------------------------
// Draw customizable rainbow color management
//-----------------------------------------------------------------------------

void CMenuModule::DrawCustomizableRainbowColor( void )
{
	ImGui::Spacing(); ImGui::Spacing();

	ImGui::Checkbox( "Rainbow Menu", m_pCfgMenuRainbowEnable->GetBoolRef() );

	if ( !m_pCfgMenuRainbowEnable->GetBool() )
		return;

	ImGui::Spacing();

	if ( ImGui::BeginCombo( "##rainbowmenu1", "Customize Rainbow", ImGuiComboFlags_HeightLargest ) )
	{
		if ( ImGui::Button( "Reset##rainbowmenu" ) )
		{
			Modules::config->RevertSectionProperties( "RainbowMenu" );
		}

		ImGui::Spacing(); ImGui::Spacing();

		ImGui::SliderFloat( "Speed##rainbowmenu", m_pCfgMenuRainbowUpdate->GetFloatRef(), 0.01f, 0.15f );
		ImGui::SliderFloat( "Hue Delta##rainbowmenu", m_pCfgMenuRainbowHue->GetFloatRef(), 0.005f, 0.1f );
		ImGui::SliderFloat( "Saturation##rainbowmenu", m_pCfgMenuRainbowSaturation->GetFloatRef(), 0.01f, 1.f );
		ImGui::SliderFloat( "Lightness##rainbowmenu", m_pCfgMenuRainbowLightness->GetFloatRef(), 0.01f, 1.f );
		ImGui::SliderFloat( "Alpha##rainbowmenu", m_pCfgMenuRainbowAlpha->GetFloatRef(), 0.f, 1.f );

		ImGui::EndCombo();
	}
}

//-----------------------------------------------------------------------------
// Draw configs management
//-----------------------------------------------------------------------------

void CMenuModule::DrawConfigs( void )
{
	static char s_szSelectedConfigInputText[ 128 ] = "default.ini";

	ImGui::TextUnformatted( "List of Configs" );

	ImGui::Spacing();

	if ( ImGui::BeginListBox( "##configs_list" ) )
	{
		for ( size_t i = 0; i < m_Configs.size(); i++ )
		{
			bool bSelected = ( m_sSelectedConfig.compare( m_Configs[ i ] ) == 0 );

			if ( ImGui::Selectable( m_Configs[ i ].c_str(), bSelected ) )
			{
				m_sSelectedConfig = m_Configs[ i ];

				strncpy( s_szSelectedConfigInputText, m_sSelectedConfig.c_str(), sizeof( s_szSelectedConfigInputText ) );
				s_szSelectedConfigInputText[ Q_ARRAYSIZE( s_szSelectedConfigInputText ) - 1 ] = '\0';
			}

			if ( bSelected )
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::Spacing();

	if ( ImGui::Button( "Load##cfg" ) )
		Modules::config->Load( m_sSelectedConfig.c_str() );

	ImGui::SameLine();

	if ( ImGui::Button( "Save##cfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedConfig.empty() )
		Modules::config->Save( m_sSelectedConfig.c_str() );

	ImGui::SameLine();

	if ( ImGui::Button( "New##cfg" ) )
		Modules::config->Save( "new_config.ini" );

	ImGui::SameLine();

	if ( ImGui::Button( "Delete##cfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedConfig.empty() )
	{
		std::string sDir = m_sConfigsFullPath;
		sDir += m_sSelectedConfig;

	#ifdef WIN32
		DeleteFile( sDir.c_str() );
	#else
		remove( sDir.c_str() );
	#endif

		m_sSelectedConfig.clear();
	}

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::InputText( "##renamecfg", s_szSelectedConfigInputText, IM_ARRAYSIZE( s_szSelectedConfigInputText ) );

	ImGui::SameLine();

	if ( ImGui::Button( "Rename##cfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedConfig.empty() )
	{
		std::string sNewConfigName = s_szSelectedConfigInputText;

		if ( !UTIL_IsCStringEndsWithPostfix( s_szSelectedConfigInputText, ".ini" ) )
		{
			sNewConfigName += ".ini";
		}

		std::string sCurrentName = m_sConfigsFullPath;
		std::string sNewName = m_sConfigsFullPath;

		sCurrentName += m_sSelectedConfig;
		sNewName += sNewConfigName;

	#ifdef WIN32
		MoveFile( sCurrentName.c_str(), sNewName.c_str() );
	#else
		rename( sCurrentName.c_str(), sNewName.c_str() );
	#endif

		m_sSelectedConfig.clear();
	}

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::InputText( "Autoexec Game CFG", m_pCfgAutoexecGameConfig->GetCString(), m_pCfgAutoexecGameConfig->GetCStringSize() );
}

//-----------------------------------------------------------------------------
// Draw shader configs management
//-----------------------------------------------------------------------------

void CMenuModule::DrawShaderConfigs( void )
{
	static char s_szSelectedConfigInputText[ 128 ] = { 0 };

	ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

	ImGui::TextUnformatted( "List of Shader Configs" );

	ImGui::Spacing();

	if ( ImGui::BeginListBox( "##shader_configs_list" ) )
	{
		for ( size_t i = 0; i < m_ShaderConfigs.size(); i++ )
		{
			bool bSelected = ( m_sSelectedShadersConfig.compare( m_ShaderConfigs[ i ] ) == 0 );

			if ( ImGui::Selectable( m_ShaderConfigs[ i ].c_str(), bSelected ) )
			{
				m_sSelectedShadersConfig = m_ShaderConfigs[ i ];

				strncpy( s_szSelectedConfigInputText, m_sSelectedShadersConfig.c_str(), sizeof( s_szSelectedConfigInputText ) );
				s_szSelectedConfigInputText[ Q_ARRAYSIZE( s_szSelectedConfigInputText ) - 1 ] = '\0';
			}

			if ( bSelected )
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndListBox();
	}

	ImGui::Spacing();

	if ( ImGui::Button( "Load##shadercfg" ) )
		Modules::config->LoadShader( m_sSelectedShadersConfig.c_str() );

	ImGui::SameLine();

	if ( ImGui::Button( "Save##shadercfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedShadersConfig.empty() )
		Modules::config->SaveShader( m_sSelectedShadersConfig.c_str() );

	ImGui::SameLine();

	if ( ImGui::Button( "New##shadercfg" ) )
		Modules::config->SaveShader( "new_config.ini" );

	ImGui::SameLine();

	if ( ImGui::Button( "Delete##shadercfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedShadersConfig.empty() )
	{
		std::string sDir = m_sShaderConfigsFullPath;
		sDir += m_sSelectedShadersConfig;

	#ifdef WIN32
		DeleteFile( sDir.c_str() );
	#else
		remove( sDir.c_str() );
	#endif

		m_sSelectedShadersConfig.clear();
	}

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::InputText( "##renameshadercfg", s_szSelectedConfigInputText, IM_ARRAYSIZE( s_szSelectedConfigInputText ) );

	ImGui::SameLine();

	if ( ImGui::Button( "Rename##shadercfg" ) && s_szSelectedConfigInputText[ 0 ] != '\0' && !m_sSelectedShadersConfig.empty() )
	{
		std::string sNewConfigName = s_szSelectedConfigInputText;

		if ( !UTIL_IsCStringEndsWithPostfix( s_szSelectedConfigInputText, ".ini" ) )
		{
			sNewConfigName += ".ini";
		}

		std::string sCurrentName = m_sShaderConfigsFullPath;
		std::string sNewName = m_sShaderConfigsFullPath;

		sCurrentName += m_sSelectedShadersConfig;
		sNewName += sNewConfigName;

	#ifdef WIN32
		MoveFile( sCurrentName.c_str(), sNewName.c_str() );
	#else
		rename( sCurrentName.c_str(), sNewName.c_str() );
	#endif


		m_sSelectedShadersConfig.clear();
	}

	ImGui::Spacing(); ImGui::Spacing();

	ImGui::InputText( "Autoexec Config", m_pCfgAutoexecShaderConfig->GetCString(), m_pCfgAutoexecShaderConfig->GetCStringSize() );
}

//-----------------------------------------------------------------------------
// Update rainbow color
//-----------------------------------------------------------------------------

void CMenuModule::UpdateRainbow( void )
{
	if ( !m_pCfgMenuRainbowEnable->GetBool() )
		return;

	if ( Globals::cl_enginefuncs->GetClientTime() < m_flRainbowUpdateTime )
		return;

	UTIL_HSL2RGB( m_flRainbowDelta,
				  m_pCfgMenuRainbowSaturation->GetFloat(),
				  m_pCfgMenuRainbowLightness->GetFloat(),
				  m_flRainbowColor[ 0 ], m_flRainbowColor[ 1 ], m_flRainbowColor[ 2 ] );

	m_flRainbowDelta += m_pCfgMenuRainbowHue->GetFloat();

	while ( m_flRainbowDelta > 1.0f )
		m_flRainbowDelta -= 1.0f;

	m_flRainbowUpdateTime = Globals::cl_enginefuncs->GetClientTime() + m_pCfgMenuRainbowUpdate->GetFloat();
}

//-----------------------------------------------------------------------------
// Update configs
//-----------------------------------------------------------------------------

void CMenuModule::UpdateConfigs( void )
{
	static float s_flUpdateTime = -1.f;
	if ( s_flUpdateTime > Globals::cl_enginefuncs->GetClientTime() )
		return;

	BuildConfigsFullPath();
	m_Configs.clear();
	s_flUpdateTime = Globals::cl_enginefuncs->GetClientTime() + 0.4f;

#ifdef WIN32
	HANDLE hFile;
	WIN32_FIND_DATAA FileInformation;

	char szFolderInitialPath[ MAX_PATH ] = { 0 };
	snprintf( szFolderInitialPath, Q_ARRAYSIZE( szFolderInitialPath ), "%s*.*", m_sConfigsFullPath.c_str() );

	hFile = ::FindFirstFile( szFolderInitialPath, &FileInformation );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( FileInformation.cFileName[ 0 ] == '.' )
				continue;

			if ( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				continue;

			const char *pszExtension = NULL;
			const char *buffer = FileInformation.cFileName;

			while ( *buffer )
			{
				if ( *buffer == '.' )
					pszExtension = buffer;

				buffer++;
			}

			if ( pszExtension && !stricmp( pszExtension, ".ini" ) )
			{
				m_Configs.push_back( FileInformation.cFileName );
			}
		} while ( ::FindNextFile( hFile, &FileInformation ) == TRUE );

		::FindClose( hFile );
	}
#else
    DIR *dir = opendir( m_sConfigsFullPath.c_str() );
    if ( dir != NULL )
	{
		struct dirent *entry;
		while ( ( entry = readdir( dir ) ) != NULL )
		{
			if ( stricmp( entry->d_name, ".")  == 0 || stricmp( entry->d_name, ".." ) == 0 )
				continue;

			char szFullPath[ PATH_MAX ];
			snprintf( szFullPath, Q_ARRAYSIZE( szFullPath ), "%s/%s", m_sConfigsFullPath.c_str(), entry->d_name );

			struct stat st;
			if ( stat( szFullPath, &st ) == 0 && S_ISDIR( st.st_mode ) )
				continue;

			const char *pszExtension = strrchr( entry->d_name, '.' );
			if ( pszExtension && stricmp( pszExtension, ".ini" ) == 0 )
			{
				m_Configs.push_back( entry->d_name );
			}
		}

		closedir(dir);
	}
#endif

	std::vector<std::string>::iterator it = std::find( m_Configs.begin(), m_Configs.end(), m_sSelectedConfig.c_str() );

	if ( it == m_Configs.end() )
	{
		m_sSelectedConfig.clear();
	}

	std::sort( m_Configs.begin(), m_Configs.end() );
}

//-----------------------------------------------------------------------------
// Update shader configs
//-----------------------------------------------------------------------------

void CMenuModule::UpdateShaderConfigs( void )
{
	static float s_flUpdateTime = -1.f;
	if ( s_flUpdateTime > Globals::cl_enginefuncs->GetClientTime() )
		return;

	BuildConfigsFullPath();
	m_ShaderConfigs.clear();
	s_flUpdateTime = Globals::cl_enginefuncs->GetClientTime() + 0.4f;

#ifdef WIN32
	HANDLE hFile;
	WIN32_FIND_DATAA FileInformation;

	char szFolderInitialPath[ MAX_PATH ] = { 0 };
	snprintf( szFolderInitialPath, Q_ARRAYSIZE( szFolderInitialPath ), "%s*.*", m_sShaderConfigsFullPath.c_str() );

	hFile = ::FindFirstFile( szFolderInitialPath, &FileInformation );
	if ( hFile != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( FileInformation.cFileName[ 0 ] == '.' )
				continue;

			if ( FileInformation.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				continue;

			const char *pszExtension = NULL;
			const char *buffer = FileInformation.cFileName;

			while ( *buffer )
			{
				if ( *buffer == '.' )
					pszExtension = buffer;

				buffer++;
			}

			if ( pszExtension && !stricmp( pszExtension, ".ini" ) )
			{
				m_ShaderConfigs.push_back( FileInformation.cFileName );
			}
		} while ( ::FindNextFile( hFile, &FileInformation ) == TRUE );

		::FindClose( hFile );
	}
#else
    DIR *dir = opendir( m_sShaderConfigsFullPath.c_str() );
    if ( dir != NULL )
	{
		struct dirent *entry;
		while ( ( entry = readdir( dir ) ) != NULL )
		{
			if ( stricmp( entry->d_name, ".")  == 0 || stricmp( entry->d_name, ".." ) == 0 )
				continue;

			char szFullPath[ PATH_MAX ];
			snprintf( szFullPath, Q_ARRAYSIZE( szFullPath ), "%s/%s", m_sShaderConfigsFullPath.c_str(), entry->d_name );

			struct stat st;
			if ( stat( szFullPath, &st ) == 0 && S_ISDIR( st.st_mode ) )
				continue;

			const char *pszExtension = strrchr( entry->d_name, '.' );
			if ( pszExtension && stricmp( pszExtension, ".ini" ) == 0 )
			{
				m_ShaderConfigs.push_back( entry->d_name );
			}
		}

		closedir(dir);
	}
#endif

	std::vector<std::string>::iterator it = std::find( m_ShaderConfigs.begin(), m_ShaderConfigs.end(), m_sSelectedShadersConfig.c_str() );

	if ( it == m_ShaderConfigs.end() )
	{
		m_sSelectedShadersConfig.clear();
	}

	std::sort( m_ShaderConfigs.begin(), m_ShaderConfigs.end() );
}

//-----------------------------------------------------------------------------
// BuildConfigsFullPath
//-----------------------------------------------------------------------------

void CMenuModule::BuildConfigsFullPath( void )
{
	if ( !m_sConfigsFullPath.empty() && !m_sShaderConfigsFullPath.empty() )
		return;

#ifdef WIN32
	char szLongPath[ MAX_PATH ];
	char szShortPath[ MAX_PATH ];
	char *pszPath = NULL;

	szLongPath[ 0 ] = 0;
	szShortPath[ 0 ] = 0;

	if ( GetModuleFileName( NULL, szShortPath, sizeof( szShortPath ) ) )
	{
		GetLongPathName( szShortPath, szLongPath, sizeof( szLongPath ) );
		pszPath = strrchr( szLongPath, '\\' );

		if ( pszPath[ 0 ] )
			pszPath[ 1 ] = 0;
	}
#else
	char szLongPath[ PATH_MAX ];

	szLongPath[ 0 ] = 0;

	if ( readlink( "/proc/self/exe", szLongPath, PATH_MAX ) != -1 )
	{
		char *pszPath = strrchr( szLongPath, '/' );

		if ( pszPath[ 0 ] )
			pszPath[ 1 ] = 0;
	}
#endif

	m_sConfigsFullPath = szLongPath;
	m_sShaderConfigsFullPath = szLongPath;

	m_sConfigsFullPath += SVENINT_MAKE_PATH( SVENINT_CONFIG_FOLDER );
	m_sShaderConfigsFullPath += SVENINT_MAKE_PATH( SVENINT_SHADERS_CONFIG_FOLDER );

#ifdef WIN32
	std::replace( m_sConfigsFullPath.begin(), m_sConfigsFullPath.end(), '/', '\\' );
	std::replace( m_sShaderConfigsFullPath.begin(), m_sShaderConfigsFullPath.end(), '/', '\\' );
#endif
}

//-----------------------------------------------------------------------------
// Initialize custom font
//-----------------------------------------------------------------------------

ImFont *CMenuModule::GetDefaultFont( void )
{
	return m_pCfgMenuFont->GetInt() == 0 ? m_pFont : m_pFontL4D;
}

//-----------------------------------------------------------------------------
// Initialize custom font
//-----------------------------------------------------------------------------

void CMenuModule::InitFont( void )
{
	ImGuiIO &io = ImGui::GetIO();

	ImFontConfig defaultFont;
	defaultFont.FontDataOwnedByAtlas = false;
	
	ImFontConfig l4dFont;
	l4dFont.FontDataOwnedByAtlas = false;

	m_pFont = io.Fonts->AddFontFromMemoryTTF( (void *)CoolFont, sizeof( CoolFont ), Globals::gameutils->GetScreenWidth() <= 800 ? 12.f : 18.f, &defaultFont );
	m_pFontL4D = io.Fonts->AddFontFromMemoryTTF( (void *)L4D2Font, sizeof( L4D2Font ), Globals::gameutils->GetScreenWidth() <= 800 ? 9.5f : 13.5f, &l4dFont );
}

//-----------------------------------------------------------------------------
// Restores window style
//-----------------------------------------------------------------------------

void CMenuModule::WindowStyle()
{
	ImGui::GetStyle().WindowRounding = 2;
	ImGui::GetStyle().ChildRounding = 4;
	ImGui::GetStyle().FrameRounding = 2;
	ImGui::GetStyle().GrabRounding = 2;
	ImGui::GetStyle().PopupRounding = 2;
	ImGui::GetStyle().FrameRounding = 2;
}

//-----------------------------------------------------------------------------
// OnConfigLoad
//-----------------------------------------------------------------------------

void CMenuModule::OnConfigLoad( const char *pszFilename, bool bShaderConfig )
{
	if ( !bShaderConfig )
	{
		if ( m_bImGuiInitialized )
		{
			LoadSavedStyle();
			LoadMenuTheme( m_pCfgMenuTheme->GetInt() );
			WindowStyle();
		}

		if ( m_pCfgAutoexecGameConfig->GetCString()[ 0 ] != '\0' )
		{
			char buffer[ 128 ];
			snprintf( buffer, Q_ARRAYSIZE( buffer ), "exec %s", m_pCfgAutoexecGameConfig->GetCString() );
			Globals::cl_enginefuncs->pfnClientCmd( buffer );
		}

		if ( m_pCfgAutoexecShaderConfig->GetCString()[ 0 ] != '\0' )
			Modules::config->LoadShader( m_pCfgAutoexecShaderConfig->GetCString() );
	}

	for ( CMenuCategory &category : m_categories )
	{
		for ( CMenuFeature &feature : category.m_features )
		{
			CConfigProperty *pCfgEnabled;
			CBaseFeature *pFeature = feature.m_pFeature;

			if ( !pFeature->IsLoaded() )
				continue;

			// Change binded convars
			for ( CBaseMenuElement *element : feature.m_elements )
				element->OnValueChanged();
			
			if ( !feature.m_bToggleable )
				continue;

			if ( feature.m_bShaderFeature )
				pCfgEnabled = Modules::config->FindShadersProperty( pFeature->GetName(), "Enable" );
			else
				pCfgEnabled = Modules::config->FindProperty( pFeature->GetName(), "Enable" );

			if ( pCfgEnabled == NULL )
				continue;
			
			if ( pFeature->IsEnabled() == pCfgEnabled->GetBool() )
				continue;

			// OnEnable callback is fired in the function PostLoadFeatures
			if ( !AreFeaturesLoaded() && !pFeature->IsEnabled() && pCfgEnabled->GetBool() )
			{
				pFeature->SetInitiallyEnabled();
				continue;
			}

			pFeature->Toggle();
		}
	}
}

//-----------------------------------------------------------------------------
// OnConfigSave
//-----------------------------------------------------------------------------

void CMenuModule::OnConfigSave( const char *pszFilename, bool bShaderConfig )
{
}

//-----------------------------------------------------------------------------
// Module constructor
//-----------------------------------------------------------------------------

CMenuModule::CMenuModule() : m_MenuConVarBinds( 63, m_Functor, m_Functor )
						#ifdef IMGUI_USE_SDL
							, m_MapKeysWinAPIToSDL( 15 ), m_MapKeysSDLToWinAPI( 15 )
						#endif
{
	m_bOpened = false;
	m_bThemeLoaded = false;
	m_bImGuiInitialized = false;

	m_flOpenTime = -1.f;
	m_flCloseTime = -1.f;

	m_pCfgMenuKey = NULL;
	m_pCfgMenuUnloadKey = NULL;
	m_pCfgMenuOpacity = NULL;
	m_pCfgMenuFont = NULL;
	m_pCfgMenuTheme = NULL;
	m_pCfgAutoexecGameConfig = NULL;
	m_pCfgAutoexecShaderConfig = NULL;

	m_pCfgMenuRainbowEnable = NULL;
	m_pCfgMenuRainbowUpdate = NULL;
	m_pCfgMenuRainbowHue = NULL;
	m_pCfgMenuRainbowSaturation = NULL;
	m_pCfgMenuRainbowLightness = NULL;
	m_pCfgMenuRainbowAlpha = NULL;

	memset( &m_CustomThemeVars, 0, sizeof( menu_custom_theme_vars ) );

	m_pszChangelog = NULL;
	m_bUpdateAvailable = false;

	m_pFont = NULL;
	m_pFontL4D = NULL;

#ifndef IMGUI_USE_SDL
	m_hGameWnd = NULL;
	m_hGameWndProc = NULL;
#else
	m_pSdlWindow = NULL;
#endif

	PreInit();
}

//-----------------------------------------------------------------------------
// InitKeyMappings
//-----------------------------------------------------------------------------

void CMenuModule::InitKeyMappings( void )
{
#ifdef IMGUI_USE_SDL
	m_MapKeysWinAPIToSDL.Insert( VK_F1, SDL_SCANCODE_F1 );
	m_MapKeysWinAPIToSDL.Insert( VK_F2, SDL_SCANCODE_F2 );
	m_MapKeysWinAPIToSDL.Insert( VK_F3, SDL_SCANCODE_F3 );
	m_MapKeysWinAPIToSDL.Insert( VK_F4, SDL_SCANCODE_F4 );
	m_MapKeysWinAPIToSDL.Insert( VK_F5, SDL_SCANCODE_F5 );
	m_MapKeysWinAPIToSDL.Insert( VK_F6, SDL_SCANCODE_F6 );
	m_MapKeysWinAPIToSDL.Insert( VK_F7, SDL_SCANCODE_F7 );
	m_MapKeysWinAPIToSDL.Insert( VK_F8, SDL_SCANCODE_F8 );
	m_MapKeysWinAPIToSDL.Insert( VK_F9, SDL_SCANCODE_F9 );
	m_MapKeysWinAPIToSDL.Insert( VK_F10, SDL_SCANCODE_F10 );
	m_MapKeysWinAPIToSDL.Insert( VK_F11, SDL_SCANCODE_F11 );
	m_MapKeysWinAPIToSDL.Insert( VK_F12, SDL_SCANCODE_F12 );
	m_MapKeysWinAPIToSDL.Insert( VK_SPACE, SDL_SCANCODE_SPACE );
	m_MapKeysWinAPIToSDL.Insert( VK_PAUSE, SDL_SCANCODE_PAUSE );
	m_MapKeysWinAPIToSDL.Insert( VK_INSERT, SDL_SCANCODE_INSERT );
	m_MapKeysWinAPIToSDL.Insert( VK_HOME, SDL_SCANCODE_HOME );
	m_MapKeysWinAPIToSDL.Insert( VK_PRIOR, SDL_SCANCODE_PAGEUP );
	m_MapKeysWinAPIToSDL.Insert( VK_DELETE, SDL_SCANCODE_DELETE );
	m_MapKeysWinAPIToSDL.Insert( VK_END, SDL_SCANCODE_END );
	m_MapKeysWinAPIToSDL.Insert( VK_NEXT, SDL_SCANCODE_PAGEDOWN );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD0, SDL_SCANCODE_KP_0 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD1, SDL_SCANCODE_KP_1 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD2, SDL_SCANCODE_KP_2 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD3, SDL_SCANCODE_KP_3 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD4, SDL_SCANCODE_KP_4 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD5, SDL_SCANCODE_KP_5 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD6, SDL_SCANCODE_KP_6 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD7, SDL_SCANCODE_KP_7 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD8, SDL_SCANCODE_KP_8 );
	m_MapKeysWinAPIToSDL.Insert( VK_NUMPAD9, SDL_SCANCODE_KP_9 );
	m_MapKeysWinAPIToSDL.Insert( VK_EXECUTE, SDL_SCANCODE_EXECUTE );
	m_MapKeysWinAPIToSDL.Insert( VK_SELECT, SDL_SCANCODE_SELECT );
	m_MapKeysWinAPIToSDL.Insert( VK_RCONTROL, SDL_SCANCODE_RCTRL );
	m_MapKeysWinAPIToSDL.Insert( VK_RSHIFT, SDL_SCANCODE_RSHIFT );
	m_MapKeysWinAPIToSDL.Insert( VK_RMENU, SDL_SCANCODE_RALT );

	for ( int i = 0; i < m_MapKeysWinAPIToSDL.Count(); i++ )
	{
		HashTableIterator_t it = m_MapKeysWinAPIToSDL.First( i );

		while ( m_MapKeysWinAPIToSDL.IsValidIterator( it ) )
		{
			int &winapikey = m_MapKeysWinAPIToSDL.KeyAt( i, it );
			int &sdlkey = m_MapKeysWinAPIToSDL.ValueAt( i, it );

			m_MapKeysSDLToWinAPI.Insert( sdlkey, winapikey );

			it = m_MapKeysWinAPIToSDL.Next( i, it );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Preinit
//-----------------------------------------------------------------------------

void CMenuModule::PreInit( void )
{
	BuildConfigsFullPath();

	AddCategory( "Player" );
	AddCategory( "Movement" );
	AddCategory( "Aim" );
	AddCategory( "HUD" );
	AddCategory( "Visual" );
	AddCategory( "Render" );
	AddCategory( "Exploit" );
	AddCategory( "Misc" );
	AddCategory( "Speedrun Tools" );
	AddCategory( "Shaders" );

	m_sSelectedConfig = "default.ini";
}

//-----------------------------------------------------------------------------
// Remove empty categories
//-----------------------------------------------------------------------------

void CMenuModule::RemoveEmptyCategories( void )
{
	for ( size_t i = 0; i < m_categories.size(); i++ )
	{
		if ( !m_categories[ i ].m_features.empty() )
			continue;

		m_categories.erase( m_categories.begin() + i );
		i--;
	}
}

//-----------------------------------------------------------------------------
// InitConfigProperties
//-----------------------------------------------------------------------------

void CMenuModule::InitConfigProperties( void )
{
	float defaultOpacity = 1.f;
	int defaultTheme = 0;

	m_pCfgMenuKey = Modules::config->AddProperty<int>( "Settings", "MenuButton", VK_INSERT );
	m_pCfgMenuUnloadKey = Modules::config->AddProperty<int>( "Settings", "UnloadButton", VK_RCONTROL );

	m_pCfgMenuKey->SetRadix( 16 );
	m_pCfgMenuUnloadKey->SetRadix( 16 );

	m_pCfgMenuHide = Modules::config->AddProperty( "Settings", "MenuHide", true );
	m_pCfgMenuOpacity = Modules::config->AddProperty( "Settings", "MenuOpacity", 1.f );
	m_pCfgMenuFont = Modules::config->AddProperty( "Settings", "MenuFont", 0 );
	m_pCfgMenuTheme = Modules::config->AddProperty( "Settings", "MenuTheme", 0 );
	m_pCfgAutoexecGameConfig = Modules::config->AddProperty( "Settings", "AutoexecGameConfig", "svenint.cfg" );
	m_pCfgAutoexecShaderConfig = Modules::config->AddProperty( "Settings", "AutoexecShaderConfig", (char *)NULL );

	m_pCfgMenuRainbowEnable = Modules::config->AddProperty( "RainbowMenu", "Enable", false );
	m_pCfgMenuRainbowUpdate = Modules::config->AddProperty( "RainbowMenu", "UpdateDelay", 0.05f );
	m_pCfgMenuRainbowHue = Modules::config->AddProperty( "RainbowMenu", "HueDelta", 0.005f );
	m_pCfgMenuRainbowSaturation = Modules::config->AddProperty( "RainbowMenu", "Saturation", 0.8f );
	m_pCfgMenuRainbowLightness = Modules::config->AddProperty( "RainbowMenu", "Lightness", 0.5f );
	m_pCfgMenuRainbowAlpha = Modules::config->AddProperty( "RainbowMenu", "Alpha", 1.f );

	m_CustomThemeVars.WindowBg = Modules::config->AddProperty( "CustomTheme", "WindowBg", 3864023120U );
	m_CustomThemeVars.Border = Modules::config->AddProperty( "CustomTheme", "Border", 0U );
	m_CustomThemeVars.Button = Modules::config->AddProperty( "CustomTheme", "Button", 4283054648U );
	m_CustomThemeVars.ButtonActive = Modules::config->AddProperty( "CustomTheme", "ButtonActive", 4292125797U );
	m_CustomThemeVars.ButtonHovered = Modules::config->AddProperty( "CustomTheme", "ButtonHovered", 4294097778U );
	m_CustomThemeVars.FrameBg = Modules::config->AddProperty( "CustomTheme", "FrameBg", 4283054648U );
	m_CustomThemeVars.FrameBgActive = Modules::config->AddProperty( "CustomTheme", "FrameBgActive", 4294097778U );
	m_CustomThemeVars.FrameBgHovered = Modules::config->AddProperty( "CustomTheme", "FrameBgHovered", 4289104990U );
	m_CustomThemeVars.Text = Modules::config->AddProperty( "CustomTheme", "Text", 4294967295U );
	m_CustomThemeVars.ChildBg = Modules::config->AddProperty( "CustomTheme", "ChildBg", 4281147937U );
	m_CustomThemeVars.CheckMark = Modules::config->AddProperty( "CustomTheme", "CheckMark", 4294967295U );
	m_CustomThemeVars.SliderGrab = Modules::config->AddProperty( "CustomTheme", "SliderGrab", 4292125797U );
	m_CustomThemeVars.SliderGrabActive = Modules::config->AddProperty( "CustomTheme", "SliderGrabActive", 4292125797U );
	m_CustomThemeVars.Header = Modules::config->AddProperty( "CustomTheme", "Header", 4292125797U );
	m_CustomThemeVars.HeaderHovered = Modules::config->AddProperty( "CustomTheme", "HeaderHovered", 4294097778U );
	m_CustomThemeVars.HeaderActive = Modules::config->AddProperty( "CustomTheme", "HeaderActive", 4292125797U );
	m_CustomThemeVars.ResizeGripActive = Modules::config->AddProperty( "CustomTheme", "ResizeGripActive", 4282528276U );
	m_CustomThemeVars.SeparatorActive = Modules::config->AddProperty( "CustomTheme", "SeparatorActive", 2353148666U );
	m_CustomThemeVars.TitleBgActive = Modules::config->AddProperty( "CustomTheme", "TitleBgActive", 4282528276U );
	m_CustomThemeVars.Separator = Modules::config->AddProperty( "CustomTheme", "Separator", 4286930991U );
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CMenuModule::Init( void )
{
	InitKeyMappings();
	InitConfigProperties();

	m_flRainbowColor[ 0 ] = m_flRainbowColor[ 1 ] = m_flRainbowColor[ 2 ] = 0.f;
	m_flRainbowDelta = 0.f;
	m_flRainbowUpdateTime = 0.f;

	Modules::config->RegisterListener( this );

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CMenuModule::Shutdown( void )
{
	Modules::config->UnregisterListener( this );

	if ( m_pszChangelog != NULL )
	{
		MemFree( m_pszChangelog );
	}

#ifndef IMGUI_USE_SDL
	if ( m_hGameWnd != NULL && m_hGameWndProc != NULL )
	{
		SetWindowLong( m_hGameWnd, GWL_WNDPROC, (LONG)m_hGameWndProc );
	}
#endif

	m_MenuConVarBinds.Clear();
	m_MenuConVarBinds.Purge();

	for ( CMenuCategory &category : m_categories )
	{
		for ( CMenuFeature &feature : category.m_features )
		{
			for ( CBaseMenuElement *element : feature.m_elements )
			{
				MemFree( element );
				//delete element;
			}

			feature.m_elements.clear();
		}

		category.m_features.clear();
	}

	m_categories.clear();
}

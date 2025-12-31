// SvenInt (c) Sw1ft
// r_wallhack.cpp

#include "stdafx.h"
#include "r_wallhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( void, CALLCONV_STDCALL, glBegin, GLenum );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CWallhack, wallhack, "Render", "Wallhack" );

//-----------------------------------------------------------------------------
// glBegin hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, CALLCONV_STDCALL, HOOKED_glBegin, GLenum mode ) // wh
{
	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->glBeginEvent( mode );

	ORIG_glBegin( mode );
}

//-----------------------------------------------------------------------------
// glBegin event
//-----------------------------------------------------------------------------

void CWallhack::glBeginEvent( GLenum mode )
{
	if ( m_pObjects->GetBool() )
	{
		if ( mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN ) // humans and some objects
			glDepthRange( 0, 0.25 );
		else
			glDepthRange( 0.5, 1 );
	}

	if ( m_pNegative->GetBool() )
	{
		if ( mode == GL_POLYGON )
		{
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
		}
	}

	if ( m_pLambert->GetBool() )
	{
		if ( mode == GL_POLYGON )
		{
			glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL );
		}
	}

	if ( m_pWireframe->GetBool() )
	{
		if ( mode == GL_POLYGON )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glLineWidth( m_pWireframeWidth->GetFloat() );
			glColor3f( m_pWireframeColor->GetColor()[ 0 ], m_pWireframeColor->GetColor()[ 1 ], m_pWireframeColor->GetColor()[ 2 ] );
		}
		else
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}

	if ( m_pWireframeModels->GetBool() )
	{
		if ( mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			glLineWidth( 1.0 );
		}
		else if ( !m_pWireframe->GetBool() )
		{
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CWallhack::CWallhack( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pObjects = NULL;
	m_pNegative = NULL;
	m_pLambert = NULL;
	m_pWireframe = NULL;
	m_pWireframeModels = NULL;
	m_pWireframeWidth = NULL;
	m_pWireframeColor = NULL;

	m_pfnglBegin = NULL;
	m_hglBegin = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CWallhack::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pObjects = Modules::menu->AddParamBool( this, "Objects", NULL, true );
	m_pNegative = Modules::menu->AddParamBool( this, "Negative", NULL, false );
	m_pLambert = Modules::menu->AddParamBool( this, "Lambert", NULL, false );
	m_pWireframe = Modules::menu->AddParamBool( this, "Wireframe", NULL, false );
	m_pWireframeModels = Modules::menu->AddParamBool( this, "WireframeModels", NULL, false );
	m_pWireframeWidth = Modules::menu->AddParamFloat( this, "WireframeWidth", NULL, 1.5f, 0.f, 10.f );
	m_pWireframeColor = Modules::menu->AddParamColorRGB( this, "WireframeColor", NULL, Color( 0, 255, 0, 255 ) );

	m_pfnglBegin = MemoryUtils()->GetProcAddress( GameData::Modules::OpenGL, "glBegin" );
	FEATURE_CHECK_SYMBOL( m_pfnglBegin, "glBegin" );
	
#ifdef LINUX
	if ( *(uint32_t *)m_pfnglBegin == 0xFB1E0FF3 && *( (uint8_t *)m_pfnglBegin + sizeof( uint32_t ) ) == 0xE8 )
	{
		PrintWarning( "Unable to attach detour for \"%s\"\n", "glBegin" );
		return false;
	}
#endif

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CWallhack::PostLoad( void )
{
	m_hglBegin = Detours()->DetourFunction( m_pfnglBegin, HOOKED_glBegin, GET_FUNC_PTR( ORIG_glBegin ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CWallhack::Unload( void )
{
	Detours()->RemoveDetour( m_hglBegin );
}

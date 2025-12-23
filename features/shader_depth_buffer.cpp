// SvenInt (c) Sw1ft
// shader_depth_buffer.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderDepthBuffer, depthbuffer, "Shaders", "Depth Buffer" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderDepthBuffer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderDepthBuffer ) )
		return kHookContinue;

	// Store depth buffer
	Modules::opengl->InitDepthTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hDepthBuffer ) );

	if ( !m_pShow->GetBool() )
		return kHookContinue;

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hDepthBuffer ) );

	SHADER_BIND( m_ShaderDepthBuffer );
		SHADER_UNIFORM_1f( m_ShaderDepthBuffer, znear, m_pZNear->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderDepthBuffer, zfar, m_pZFar->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderDepthBuffer, factor, m_pBrightness->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderDepthBuffer, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderDepthBuffer::CShaderDepthBuffer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pShow = NULL;
	m_pZNear = NULL;
	m_pZFar = NULL;
	m_pBrightness = NULL;

	POST_PROCESSING_RESET_VARS( m_hDepthBuffer );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderDepthBuffer::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent, kHookCall, kHookPriorityHigh );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderDepthBuffer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderDepthBuffer::Load( void )
{
	Modules::menu->BindShaderFeature( this, false /* always enabled to store the depth buffer */ );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShow = Modules::menu->AddParamBool( this, "Show", NULL, false );
	m_pZNear = Modules::menu->AddParamFloat( this, "ZNear", NULL, 48.f, 0.01f, 64.f );
	m_pZFar = Modules::menu->AddParamFloat( this, "ZFar", NULL, 4096.f, 0.01f, 4096.f );
	m_pBrightness = Modules::menu->AddParamFloat( this, "Brightness", NULL, 1.f, 0.f, 1.f );

#ifdef SINT_USE_GLEW
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
#else
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "ARB Functions" );
#endif
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	POST_PROCESSING_INIT_VARS_DEPTH( m_hDepthBuffer, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );

	if ( POST_PROCESSING_FBO( m_hDepthBuffer ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hDepthBuffer );
		PrintWarning( "Failed to generate the frame buffer\n" );
		return false;
	}
	if ( POST_PROCESSING_TEX( m_hDepthBuffer ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hDepthBuffer );
		PrintWarning( "Failed to generate the depth texture\n" );
		return false;
	}

	SHADER_BEGIN_COMPILE_FILE( m_ShaderDepthBuffer, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\depth_buffer.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderDepthBuffer, znear )
		SHADER_LOCATE_UNIFORM( m_ShaderDepthBuffer, zfar )
		SHADER_LOCATE_UNIFORM( m_ShaderDepthBuffer, factor )
		SHADER_LOCATE_UNIFORM( m_ShaderDepthBuffer, res )
	SHADER_END_COMPILE()
	else { return false; }

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderDepthBuffer::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hDepthBuffer );
}
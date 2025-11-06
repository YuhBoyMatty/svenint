// SvenInt (c) Sw1ft
// shader_dof_blur.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_dof_blur.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderDoFBlur, dofblur, "Shaders", "DoF Blur" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderDoFBlur::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderDoFBlur, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\dof_blur.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, iChannel0 )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, depthmap )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, interptype )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, znear )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, zfar )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, distance )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, bokeh )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, samples )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, radius )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, dir )
		SHADER_LOCATE_UNIFORM( m_ShaderDoFBlur, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderDoFBlur::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderDoFBlur ) )
		return kHookContinue;

	const Vector2D directions[ 3 ] =
	{
		{ 0.f, 1.f },
		{ 0.866f / Modules::opengl->GetScreenAspectRatio(), 0.5f },
		{ 0.866f / Modules::opengl->GetScreenAspectRatio(), -0.5f }
	};

	for ( int i = 0; i < 3; i++ )
	{
		Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hDoFBlur ) );

		GLint texture0, texture1;

		glEnable( GL_TEXTURE_2D );

		glActiveTexture( GL_TEXTURE0 + 0 ); // Texture unit 0
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &texture0 );
		Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hDoFBlur ) );

		glActiveTexture( GL_TEXTURE0 + 1 ); // Texture unit 1
		glGetIntegerv( GL_TEXTURE_BINDING_2D, &texture1 );
		Modules::opengl->BindTexture( Features::depthbuffer->GetDepthTexture() );

		SHADER_BIND( m_ShaderDoFBlur );
			SHADER_UNIFORM_1i( m_ShaderDoFBlur, iChannel0, 0 );
			SHADER_UNIFORM_1i( m_ShaderDoFBlur, depthmap, 1 );
			SHADER_UNIFORM_1i( m_ShaderDoFBlur, interptype, m_pType->GetInt() - 1 );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, znear, 4.f );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, zfar, m_pMaxRange->GetFloat() );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, distance, m_pMinRange->GetFloat() / m_pMaxRange->GetFloat() );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, bokeh, m_pBokehCoefficient->GetFloat() );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, samples, (float)m_pQuality->GetInt() );
			SHADER_UNIFORM_1f( m_ShaderDoFBlur, radius, m_pBlurRange->GetFloat() );
			SHADER_UNIFORM_2f( m_ShaderDoFBlur, dir, directions[ i ].x, directions[ i ].y );
			SHADER_UNIFORM_2f( m_ShaderDoFBlur, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

			glColor4ub( 255, 255, 255, 255 );
			Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
		SHADER_UNBIND();

		Modules::opengl->BindTexture( texture1 ); // unbind from texture unit 1
		glActiveTexture( GL_TEXTURE0 + 0 );
		Modules::opengl->BindTexture( texture0 ); // unbind from texture unit 0
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderDoFBlur::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderDoFBlur );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderDoFBlur::CShaderDoFBlur( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pType = NULL;
	m_pButtonRecompile = NULL;
	m_pQuality = NULL;
	m_pBokehCoefficient = NULL;
	m_pMinRange = NULL;
	m_pMaxRange = NULL;
	m_pBlurRange = NULL;

	POST_PROCESSING_RESET_VARS( m_hDoFBlur );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderDoFBlur::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderDoFBlur ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderDoFBlur::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderDoFBlur::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pType = Modules::menu->AddParamList( this, "InterpType", NULL, 1, " 0 - Linear\0 1 - Simple Spline\0 2 - Parabolic\0 3 - Parabolic Inverted\0 4 - Cubic\0\0" );
	m_pQuality = Modules::menu->AddParamInteger( this, "Quality", NULL, 20, 0, 50 );
	m_pBokehCoefficient = Modules::menu->AddParamFloat( this, "BokehCoefficient", NULL, 0.7f, 0.f, 1.f );
	m_pMinRange = Modules::menu->AddParamFloat( this, "MinRange", NULL, 1024.f, 0.01f, 4096.f );
	m_pMaxRange = Modules::menu->AddParamFloat( this, "MaxRange", NULL, 4096.f, 0.01f, 4096.f );
	m_pBlurRange = Modules::menu->AddParamFloat( this, "BlurRange", NULL, 20.f, 0.f, 150.f );

	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CShaderDoFBlur::PostLoad( void )
{
	POST_PROCESSING_INIT_VARS_COLOR( m_hDoFBlur, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderDoFBlur::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hDoFBlur );
	SHADER_FREE( m_ShaderDoFBlur );
}
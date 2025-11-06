// SvenInt (c) Sw1ft
// shader_menu_background_blur.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_menu_background_blur.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderMenuBackGroundBlur, menubgblur, "Shaders", "Menu Background Blur" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderBokeh, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\bokeh.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderBokeh, bokeh )
		SHADER_LOCATE_UNIFORM( m_ShaderBokeh, samples )
		SHADER_LOCATE_UNIFORM( m_ShaderBokeh, dir )
		SHADER_LOCATE_UNIFORM( m_ShaderBokeh, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderMenuBackGroundBlur::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderBokeh ) )
		return kHookContinue;

	float dt = 1.f;
	float flRadius = m_pBlurRadius->GetFloat();
	float flFadeIn = m_pFadeInDuration->GetFloat();
	float flFadeOut = m_pFadeOutDuration->GetFloat();
	const float flTime = cl_enginefuncs->GetClientTime();

	if ( Modules::menu->IsOpen() )
	{
		if ( flTime - Modules::menu->GetOpenTime() <= flFadeIn )
		{
			dt = ( flTime - Modules::menu->GetOpenTime() ) / flFadeIn;
			dt = 3 * dt * dt - 2 * dt * dt * dt;
		}

		flRadius *= dt;
	}
	else if ( flTime - Modules::menu->GetCloseTime() > flFadeOut )
	{
		return kHookContinue;
	}
	else
	{
		if ( flTime - Modules::menu->GetCloseTime() <= flFadeOut )
		{
			dt = ( flTime - Modules::menu->GetCloseTime() ) / flFadeOut;
			dt = 3 * dt * dt - 2 * dt * dt * dt;
		}

		flRadius *= 1.f - dt;
	}

	const Vector2D directions[ 3 ] =
	{
		{ 0.f, 1.f },
		{ 0.866f / Modules::opengl->GetScreenAspectRatio(), 0.5f},
		{ 0.866f / Modules::opengl->GetScreenAspectRatio(), -0.5f }
	};

	for ( int i = 0; i < 3; i++ )
	{
		Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hBokeh ) );

		glEnable( GL_TEXTURE_2D );
		Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hBokeh ) );

		SHADER_BIND( m_ShaderBokeh );
			SHADER_UNIFORM_1f( m_ShaderBokeh, bokeh, m_pBokehCoefficient->GetFloat() );
			SHADER_UNIFORM_1f( m_ShaderBokeh, samples, (float)m_pBlurQuality->GetInt() );
			SHADER_UNIFORM_2f( m_ShaderBokeh, dir, flRadius * directions[ i ].x, flRadius * directions[ i ].y );
			SHADER_UNIFORM_2f( m_ShaderBokeh, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

			glColor4ub( 255, 255, 255, 255 );
			Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
		SHADER_UNBIND();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderBokeh );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderMenuBackGroundBlur::CShaderMenuBackGroundBlur( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pButtonRecompile = NULL;
	m_pFadeInDuration = NULL;
	m_pFadeOutDuration = NULL;
	m_pBlurQuality = NULL;
	m_pBlurRadius = NULL;
	m_pBokehCoefficient = NULL;

	POST_PROCESSING_RESET_VARS( m_hBokeh );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderBokeh ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderMenuBackGroundBlur::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pFadeInDuration = Modules::menu->AddParamFloat( this, "FadeInDuration", NULL, 0.5f, 0.f, 5.f );
	m_pFadeOutDuration = Modules::menu->AddParamFloat( this, "FadeOutDuration", NULL, 0.4f, 0.f, 5.f );
	m_pBlurQuality = Modules::menu->AddParamInteger( this, "BlurQuality", NULL, 15, 1, 50 );
	m_pBlurRadius = Modules::menu->AddParamFloat( this, "BlurRadius", NULL, 10.f, 0.1f, 150.f );
	m_pBokehCoefficient = Modules::menu->AddParamFloat( this, "BokehCoefficient", NULL, 0.7f, 0.f, 1.f );

	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::PostLoad( void )
{
	POST_PROCESSING_INIT_VARS_COLOR( m_hBokeh, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderMenuBackGroundBlur::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hBokeh );
	SHADER_FREE( m_ShaderBokeh );
}
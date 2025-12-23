// SvenInt (c) Sw1ft
// shader_vignette.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_vignette.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderVignette, vignette, "Shaders", "Vignette" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderVignette::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderVignette, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\vignette.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderVignette, falloff )
		SHADER_LOCATE_UNIFORM( m_ShaderVignette, amount )
		SHADER_LOCATE_UNIFORM( m_ShaderVignette, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderVignette::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderVignette ) )
		return kHookContinue;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hVignette ) );

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hVignette ) );

	SHADER_BIND( m_ShaderVignette );
		SHADER_UNIFORM_1f( m_ShaderVignette, falloff, m_pFalloff->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderVignette, amount, m_pAmount->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderVignette, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderVignette::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderVignette );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderVignette::CShaderVignette( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonRecompile = NULL;
	m_pFalloff = NULL;
	m_pAmount = NULL;

	POST_PROCESSING_RESET_VARS( m_hVignette );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderVignette::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderVignette ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderVignette::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderVignette::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pFalloff = Modules::menu->AddParamFloat( this, "Falloff", NULL, 0.5f, 0.f, 1.f );
	m_pAmount = Modules::menu->AddParamFloat( this, "Amount", NULL, 0.4f, 0.f, 5.f );

#ifdef SINT_USE_GLEW
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
#else
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "ARB Functions" );
#endif
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	POST_PROCESSING_INIT_VARS_COLOR( m_hVignette, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );

	if ( POST_PROCESSING_FBO( m_hVignette ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hVignette );
		PrintWarning( "Failed to generate the frame buffer\n" );
		return false;
	}
	if ( POST_PROCESSING_TEX( m_hVignette ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hVignette );
		PrintWarning( "Failed to generate the color texture\n" );
		return false;
	}

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderVignette::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hVignette );
	SHADER_FREE( m_ShaderVignette );
}
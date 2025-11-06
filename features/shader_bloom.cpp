// SvenInt (c) Sw1ft
// shader_bloom.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_bloom.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderBloom, bloom, "Shaders", "Bloom" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderBloom::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderBloom, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\bloom.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderBloom, size )
		SHADER_LOCATE_UNIFORM( m_ShaderBloom, intensity )
		SHADER_LOCATE_UNIFORM( m_ShaderBloom, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderBloom::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderBloom ) )
		return kHookContinue;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hBloom ) );

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hBloom ) );

	SHADER_BIND( m_ShaderBloom );
		SHADER_UNIFORM_1f( m_ShaderBloom, size, m_pSize->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderBloom, intensity, m_pIntensity->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderBloom, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderBloom::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderBloom );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderBloom::CShaderBloom( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonRecompile = NULL;
	m_pSize = NULL;
	m_pIntensity = NULL;

	POST_PROCESSING_RESET_VARS( m_hBloom );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderBloom::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderBloom ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderBloom::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderBloom::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pSize = Modules::menu->AddParamFloat( this, "Size", NULL, 10.0f, 0.f, 50.f );
	m_pIntensity = Modules::menu->AddParamFloat( this, "Intensity", NULL, 0.25f, 0.f, 1.f );

	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CShaderBloom::PostLoad( void )
{
	POST_PROCESSING_INIT_VARS_COLOR( m_hBloom, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderBloom::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hBloom );
	SHADER_FREE( m_ShaderBloom );
}
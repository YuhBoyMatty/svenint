// SvenInt (c) Sw1ft
// shader_chromatic_aberration.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_chromatic_aberration.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderChromaticAberration, chromaticaberration, "Shaders", "Chromatic Aberration" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderChromaticAberration::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderChromaticAberration, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\chromatic_aberration.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderChromaticAberration, type )
		SHADER_LOCATE_UNIFORM( m_ShaderChromaticAberration, shift )
		SHADER_LOCATE_UNIFORM( m_ShaderChromaticAberration, strength )
		SHADER_LOCATE_UNIFORM( m_ShaderChromaticAberration, dir )
		SHADER_LOCATE_UNIFORM( m_ShaderChromaticAberration, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderChromaticAberration::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderChromaticAberration ) )
		return kHookContinue;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hChromaticAberration ) );

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hChromaticAberration ) );

	if ( m_pType->GetInt() != 0 && m_pShift->GetFloat() > 1.f )
		*m_pShift->GetCfgProperty()->GetFloatRef() = 1.f;

	SHADER_BIND( m_ShaderChromaticAberration );
		SHADER_UNIFORM_1i( m_ShaderChromaticAberration, type, m_pType->GetInt() );
		SHADER_UNIFORM_1f( m_ShaderChromaticAberration, shift, m_pShift->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderChromaticAberration, strength, m_pStrength->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderChromaticAberration, dir, m_pDirX->GetFloat(), m_pDirY->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderChromaticAberration, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderChromaticAberration::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderChromaticAberration );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderChromaticAberration::CShaderChromaticAberration( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonRecompile = NULL;
	m_pType = NULL;
	m_pDirX = NULL;
	m_pDirY = NULL;
	m_pShift = NULL;
	m_pStrength = NULL;

	POST_PROCESSING_RESET_VARS( m_hChromaticAberration );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderChromaticAberration::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderChromaticAberration ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderChromaticAberration::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderChromaticAberration::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pType = Modules::menu->AddParamList( this, "Type", NULL, 1, " 0 - Default\0 1 - Barrel Distortion\0 2 - Linear Barrel Distortion\0\0" );
	m_pDirX = Modules::menu->AddParamFloat( this, "DirX", NULL, 1.f, -50.f, 50.f );
	m_pDirY = Modules::menu->AddParamFloat( this, "DirY", NULL, 1.f, -50.f, 50.f );
	m_pShift = Modules::menu->AddParamFloat( this, "Shift", NULL, 0.025f, 0.f, 20.f );
	m_pStrength = Modules::menu->AddParamFloat( this, "Strength", NULL, 1.f, 0.f, 10.f );

#ifdef SINT_USE_GLEW
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
#else
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "ARB Functions" );
#endif
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	POST_PROCESSING_INIT_VARS_COLOR( m_hChromaticAberration, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );

	if ( POST_PROCESSING_FBO( m_hChromaticAberration ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hChromaticAberration );
		PrintWarning( "Failed to generate the frame buffer\n" );
		return false;
	}
	if ( POST_PROCESSING_TEX( m_hChromaticAberration ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hChromaticAberration );
		PrintWarning( "Failed to generate the color texture\n" );
		return false;
	}

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderChromaticAberration::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hChromaticAberration );
	SHADER_FREE( m_ShaderChromaticAberration );
}
// SvenInt (c) Sw1ft
// shader_color_correction.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_color_correction.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderColorCorrection, colorcorrection, "Shaders", "Color Correction" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderColorCorrection::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderColorCorrection, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\color_correction.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, iTime )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_target_gamma )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_monitor_gamma )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_hue_offset )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_saturation )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_contrast )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_luminance )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_black_level )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_bright_boost )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_R )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_G )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_B )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, ia_GRAIN_STR )
		SHADER_LOCATE_UNIFORM( m_ShaderColorCorrection, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderColorCorrection::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderColorCorrection ) )
		return kHookContinue;

	m_flGrainTime += 1.f;
	if ( m_flGrainTime > 1000.f )
		m_flGrainTime = 0.f;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hColorCorrection ) );

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hColorCorrection ) );

	SHADER_BIND( m_ShaderColorCorrection );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, iTime, m_flGrainTime );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_target_gamma, m_pTargetGamma->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_monitor_gamma, m_pMonitorGamma->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_hue_offset, m_pHueOffset->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_saturation, m_pSaturation->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_contrast, m_pContrast->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_luminance, m_pLuminance->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_black_level, m_pBlackLevel->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_bright_boost, m_pBrightBoost->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_R, m_pRedLevel->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_G, m_pGreenLevel->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_B, m_pBlueLevel->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderColorCorrection, ia_GRAIN_STR, m_pGrain->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderColorCorrection, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderColorCorrection::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderColorCorrection );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderColorCorrection::CShaderColorCorrection( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_flGrainTime = -1.f;

	m_pButtonRecompile = NULL;
	m_pTargetGamma = NULL;
	m_pMonitorGamma = NULL;
	m_pHueOffset = NULL;
	m_pSaturation = NULL;
	m_pContrast = NULL;
	m_pLuminance = NULL;
	m_pBlackLevel = NULL;
	m_pBrightBoost = NULL;
	m_pRedLevel = NULL;
	m_pGreenLevel = NULL;
	m_pBlueLevel = NULL;
	m_pGrain = NULL;

	POST_PROCESSING_RESET_VARS( m_hColorCorrection );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderColorCorrection::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderColorCorrection ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderColorCorrection::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderColorCorrection::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pTargetGamma = Modules::menu->AddParamFloat( this, "Gamma", NULL, 2.2f, 0.01f, 4.f );
	m_pMonitorGamma = Modules::menu->AddParamFloat( this, "MonitorGamma", NULL, 2.2f, 0.01f, 4.f );
	m_pHueOffset = Modules::menu->AddParamFloat( this, "HueOffset", NULL, 0.f, 0.f, 1.f );
	m_pSaturation = Modules::menu->AddParamFloat( this, "Saturation", NULL, 1.f, 0.f, 10.f );
	m_pContrast = Modules::menu->AddParamFloat( this, "Contrast", NULL, 1.f, 0.f, 10.f );
	m_pLuminance = Modules::menu->AddParamFloat( this, "Luminance", NULL, 1.f, 0.f, 10.f );
	m_pBlackLevel = Modules::menu->AddParamFloat( this, "BlackLevel", NULL, 0.f, 0.f, 1.f );
	m_pBrightBoost = Modules::menu->AddParamFloat( this, "BrightBoost", NULL, 0.f, 0.f, 1.f );
	m_pRedLevel = Modules::menu->AddParamFloat( this, "RedLevel", NULL, 1.f, 0.f, 1.f );
	m_pGreenLevel = Modules::menu->AddParamFloat( this, "GreenLevel", NULL, 1.f, 0.f, 1.f );
	m_pBlueLevel = Modules::menu->AddParamFloat( this, "BlueLevel", NULL, 1.f, 0.f, 1.f );
	m_pGrain = Modules::menu->AddParamFloat( this, "Grain", NULL, 0.f, 0.f, 512.f );

#ifdef SINT_USE_GLEW
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
#else
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "ARB Functions" );
#endif
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	POST_PROCESSING_INIT_VARS_COLOR( m_hColorCorrection, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );

	if ( POST_PROCESSING_FBO( m_hColorCorrection ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hColorCorrection );
		PrintWarning( "Failed to generate the frame buffer\n" );
		return false;
	}
	if ( POST_PROCESSING_TEX( m_hColorCorrection ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hColorCorrection );
		PrintWarning( "Failed to generate the color texture\n" );
		return false;
	}

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CShaderColorCorrection::PostLoad( void )
{
	POST_PROCESSING_INIT_VARS_COLOR( m_hColorCorrection, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderColorCorrection::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hColorCorrection );
	SHADER_FREE( m_ShaderColorCorrection );
}
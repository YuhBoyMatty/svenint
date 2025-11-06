// SvenInt (c) Sw1ft
// shader_ssao.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_ssao.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderSSAO, ssao, "Shaders", "SSAO" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderSSAO::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderSSAO, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\ssao.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, iChannel0 )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, depthmap )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, zNear )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, zFar )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, strength )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, samples )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, radius )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, aoclamp )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, noise )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, noiseamount )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, diffarea )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, gdisplace )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, mist )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, miststart )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, mistend )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, onlyAO )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, lumInfluence )
		SHADER_LOCATE_UNIFORM( m_ShaderSSAO, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderSSAO::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderSSAO ) )
		return kHookContinue;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hSSAO ) );

	GLint texture0, texture1;

	glEnable( GL_TEXTURE_2D );

	glActiveTexture( GL_TEXTURE0 + 0 ); // Texture unit 0
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &texture0 );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hSSAO ) );

	glActiveTexture( GL_TEXTURE0 + 1 ); // Texture unit 1
	glGetIntegerv( GL_TEXTURE_BINDING_2D, &texture1 );
	Modules::opengl->BindTexture( Features::depthbuffer->GetDepthTexture() );

	SHADER_BIND( m_ShaderSSAO );
		SHADER_UNIFORM_1i( m_ShaderSSAO, iChannel0, 0 );
		SHADER_UNIFORM_1i( m_ShaderSSAO, depthmap, 1 );
		SHADER_UNIFORM_1f( m_ShaderSSAO, zNear, m_pZNear->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, zFar, m_pZFar->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, strength, m_pStrength->GetFloat() );
		SHADER_UNIFORM_1i( m_ShaderSSAO, samples, m_pSamples->GetInt() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, radius, m_pRadius->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, aoclamp, m_pDepthClamp->GetFloat() );
		SHADER_UNIFORM_1i( m_ShaderSSAO, noise, m_pNoise->GetBool() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, noiseamount, m_pNoiseDitheringAmount->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, diffarea, m_pSelfShadowingReduction->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, gdisplace, m_pGaussBellCenter->GetFloat() );
		SHADER_UNIFORM_1i( m_ShaderSSAO, mist, m_pMist->GetBool() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, miststart, m_pMistStart->GetFloat() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, mistend, m_pMistEnd->GetFloat() );
		SHADER_UNIFORM_1i( m_ShaderSSAO, onlyAO, m_pAmbientOcclusion->GetBool() );
		SHADER_UNIFORM_1f( m_ShaderSSAO, lumInfluence, m_pLuminanceAffection->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderSSAO, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	Modules::opengl->BindTexture( texture1 ); // unbind from texture unit 1
	glActiveTexture( GL_TEXTURE0 + 0 );
	Modules::opengl->BindTexture( texture0 ); // unbind from texture unit 0

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderSSAO::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderSSAO );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderSSAO::CShaderSSAO( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pButtonRecompile = NULL;

	m_pAmbientOcclusion = NULL;
	m_pSamples = NULL;
	m_pZNear = NULL;
	m_pZFar = NULL;

	m_pStrength = NULL;
	m_pRadius = NULL;
	m_pDepthClamp = NULL;
	m_pSelfShadowingReduction = NULL;
	m_pGaussBellCenter = NULL;
	m_pLuminanceAffection = NULL;

	m_pNoise = NULL;
	m_pNoiseDitheringAmount = NULL;

	m_pMist = NULL;
	m_pMistStart = NULL;
	m_pMistEnd = NULL;

	POST_PROCESSING_RESET_VARS( m_hSSAO );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderSSAO::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderSSAO ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderSSAO::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderSSAO::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pAmbientOcclusion = Modules::menu->AddParamBool( this, "ShowAmbientOcclusionOnly", NULL, false );
	m_pSamples = Modules::menu->AddParamInteger( this, "Quality", NULL, 32, 1, 64 );
	m_pZNear = Modules::menu->AddParamFloat( this, "ZNear", NULL, 4.f, 0.01f, 64.f );
	m_pZFar = Modules::menu->AddParamFloat( this, "ZFar", NULL, 4096.f, 0.01f, 4096.f );
	m_pStrength = Modules::menu->AddParamFloat( this, "Strength", NULL, 1.f, 0.f, 10.f );
	m_pRadius = Modules::menu->AddParamFloat( this, "Radius", NULL, 2.5f, 0.f, 50.f );
	m_pDepthClamp = Modules::menu->AddParamFloat( this, "DepthClamp", NULL, 0.14f, 0.f, 5.f );
	m_pSelfShadowingReduction = Modules::menu->AddParamFloat( this, "SelfShadowingReduction", NULL, 0.3f, 0.f, 10.f );
	m_pGaussBellCenter = Modules::menu->AddParamFloat( this, "GaussBellCenter", NULL, 0.4f, 0.f, 10.f );
	m_pLuminanceAffection = Modules::menu->AddParamFloat( this, "LuminanceAffection", NULL, 0.7f, 0.f, 5.f );

	Modules::menu->AddElementSeparator( this );

	m_pNoise = Modules::menu->AddParamBool( this, "Noise", NULL, true );
	m_pNoiseDitheringAmount = Modules::menu->AddParamFloat( this, "NoiseDitheringAmount", NULL, 0.0002f, 0.f, 10.f );

	Modules::menu->AddElementSeparator( this );

	m_pMist = Modules::menu->AddParamBool( this, "Mist", NULL, false );
	m_pMistStart = Modules::menu->AddParamFloat( this, "MistStart", NULL, 0.f, 0.f, 2048.f );
	m_pMistEnd = Modules::menu->AddParamFloat( this, "MistEnd", NULL, 4096.0f, 0.f, 4096.f );

	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CShaderSSAO::PostLoad( void )
{
	POST_PROCESSING_INIT_VARS_COLOR( m_hSSAO, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderSSAO::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hSSAO );
}
// SvenInt (c) Sw1ft
// shader_motion_blur.cpp

#include "stdafx.h"
#include "svenint.h"
#include "shader_motion_blur.h"
#include "shader_depth_buffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CShaderMotionBlur, motionblur, "Shaders", "Motion Blur" );

//-----------------------------------------------------------------------------
// Compile shader
//-----------------------------------------------------------------------------

void CShaderMotionBlur::Compile( void )
{
	SHADER_BEGIN_COMPILE_FILE( m_ShaderRadialBlur, SVENINT_FOLDER_NAME "\\shaders\\pp_fullscreen.vsh", SVENINT_FOLDER_NAME "\\shaders\\radial_blur.fsh" )
		SHADER_LOCATE_UNIFORM( m_ShaderRadialBlur, distance )
		SHADER_LOCATE_UNIFORM( m_ShaderRadialBlur, strength )
		SHADER_LOCATE_UNIFORM( m_ShaderRadialBlur, res )
	SHADER_END_COMPILE();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CShaderMotionBlur::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_Redraw event
	if ( !SHADER_COMPILED( m_ShaderRadialBlur ) )
		return kHookContinue;

	const float flSpeed = playermove->velocity()->Length2D();
	const float flMinSpeed = m_pMinSpeed->GetFloat();
	const float flMaxSpeed = m_pMaxSpeed->GetFloat();

	if ( flSpeed < flMinSpeed )
		return kHookContinue;

	float dt = 1.f;

	if ( flSpeed < flMaxSpeed )
		dt = ( flSpeed - flMinSpeed ) / ( flMaxSpeed - flMinSpeed );

	// Spline interp
	dt = 3 * dt * dt - 2 * dt * dt * dt;

	Modules::opengl->InitColorTexPostProcessing( POST_PROCESSING_EXPAND_VARS( m_hRadialBlur ) );

	glEnable( GL_TEXTURE_2D );
	Modules::opengl->BindTexture( POST_PROCESSING_TEX( m_hRadialBlur ) );

	SHADER_BIND( m_ShaderRadialBlur );
		SHADER_UNIFORM_1f( m_ShaderRadialBlur, distance, dt );
		SHADER_UNIFORM_1f( m_ShaderRadialBlur, strength, m_pStrength->GetFloat() );
		SHADER_UNIFORM_2f( m_ShaderRadialBlur, res, Modules::opengl->GetScreenWidthf(), Modules::opengl->GetScreenHeightf() );

		glColor4ub( 255, 255, 255, 255 );
		Modules::opengl->DrawQuad( Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );
	SHADER_UNBIND();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CShaderMotionBlur::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pButtonRecompile )
	{
		SHADER_FREE( m_ShaderRadialBlur );
		Compile();
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CShaderMotionBlur::CShaderMotionBlur( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pButtonRecompile = NULL;
	m_pStrength = NULL;
	m_pMinSpeed = NULL;
	m_pMaxSpeed = NULL;

	POST_PROCESSING_RESET_VARS( m_hRadialBlur );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CShaderMotionBlur::OnEnable( void )
{
	if ( !SHADER_COMPILED( m_ShaderRadialBlur ) )
		Compile();

	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	Modules::opengl->OnShaderEnabled();
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CShaderMotionBlur::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CShaderMotionBlur::Load( void )
{
	Modules::menu->BindShaderFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );
	m_pButtonRecompile = Modules::menu->AddElementButton( this, this, "Recompile" );

	m_pStrength = Modules::menu->AddParamFloat( this, "Strength", NULL, 2.f, 0.f, 50.f );
	m_pMinSpeed = Modules::menu->AddParamFloat( this, "MinSpeed", NULL, 270.f, 0.1f, 2000.f );
	m_pMaxSpeed = Modules::menu->AddParamFloat( this, "MaxSpeed", NULL, 700.f, 0.1f, 2000.f );

#ifdef SINT_USE_GLEW
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "GLEW" );
#else
	FEATURE_REQUIRE_GAMEDATA( Modules::opengl->IsInitialized(), "ARB Functions" );
#endif
	FEATURE_REQUIRE_GAMEDATA( GameData::Pointers::Engine::GL_Bind, "GL_Bind" );

	POST_PROCESSING_INIT_VARS_COLOR( m_hRadialBlur, Modules::opengl->GetScreenWidth(), Modules::opengl->GetScreenHeight() );

	if ( POST_PROCESSING_FBO( m_hRadialBlur ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hRadialBlur );
		PrintWarning( "Failed to generate the frame buffer\n" );
		return false;
	}
	if ( POST_PROCESSING_TEX( m_hRadialBlur ) == 0 )
	{
		POST_PROCESSING_FREE_VARS( m_hRadialBlur );
		PrintWarning( "Failed to generate the color texture\n" );
		return false;
	}

	Compile();

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CShaderMotionBlur::Unload( void )
{
	POST_PROCESSING_FREE_VARS( m_hRadialBlur );
	SHADER_FREE( m_ShaderRadialBlur );
}
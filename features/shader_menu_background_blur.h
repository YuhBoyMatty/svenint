// SvenInt (c) Sw1ft
// shader_menu_background_blur.h

#ifndef SINT_FEATURE_SHADER_MENU_BACKGROUND_BLUR_H
#define SINT_FEATURE_SHADER_MENU_BACKGROUND_BLUR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Menu background blur shader feature
//-----------------------------------------------------------------------------

class CShaderMenuBackGroundBlur final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderMenuBackGroundBlur( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

private:
	void Compile( void );

private:
	CMenuElementButton *m_pButtonRecompile;
	CMenuValueFloat *m_pFadeInDuration;
	CMenuValueFloat *m_pFadeOutDuration;
	CMenuValueInteger *m_pBlurQuality;
	CMenuValueFloat *m_pBlurRadius;
	CMenuValueFloat *m_pBokehCoefficient;


	POST_PROCESSING_DEFINE_VARS( m_hBokeh );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramBokeh )
		SHADER_DEFINE_INTERNAL_NAME( "Bokeh" )
		SHADER_DEFINE_UNIFORM( bokeh )
		SHADER_DEFINE_UNIFORM( samples )
		SHADER_DEFINE_UNIFORM( dir )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramBokeh, m_ShaderBokeh );
};

EXTERN_FEATURE( CShaderMenuBackGroundBlur, menubgblur );

#endif // SINT_FEATURE_SHADER_MENU_BACKGROUND_BLUR_H
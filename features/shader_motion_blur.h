// SvenInt (c) Sw1ft
// shader_motion_blur.h

#ifndef SINT_FEATURE_SHADER_MOTION_BLUR_H
#define SINT_FEATURE_SHADER_MOTION_BLUR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Motion blur shader feature
//-----------------------------------------------------------------------------

class CShaderMotionBlur final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderMotionBlur( const char *pszCategoryName, const char *pszName );

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
	CMenuValueFloat *m_pStrength;
	CMenuValueFloat *m_pMinSpeed;
	CMenuValueFloat *m_pMaxSpeed;

	POST_PROCESSING_DEFINE_VARS( m_hRadialBlur );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramRadialBlur )
		SHADER_DEFINE_INTERNAL_NAME( "RadialBlur" )
		SHADER_DEFINE_UNIFORM( distance )
		SHADER_DEFINE_UNIFORM( strength )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramRadialBlur, m_ShaderRadialBlur );
};

EXTERN_FEATURE( CShaderMotionBlur, motionblur );

#endif // SINT_FEATURE_SHADER_MOTION_BLUR_H
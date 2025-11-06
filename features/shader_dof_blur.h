// SvenInt (c) Sw1ft
// shader_dof_blur.h

#ifndef SINT_FEATURE_SHADER_DOF_BLUR_H
#define SINT_FEATURE_SHADER_DOF_BLUR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Chromatic aberration shader feature
//-----------------------------------------------------------------------------

class CShaderDoFBlur final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderDoFBlur( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

private:
	void Compile( void );

private:
	CMenuElementButton *m_pButtonRecompile;
	CMenuValueList *m_pType;
	CMenuValueInteger *m_pQuality;
	CMenuValueFloat *m_pBokehCoefficient;
	CMenuValueFloat *m_pMinRange;
	CMenuValueFloat *m_pMaxRange;
	CMenuValueFloat *m_pBlurRange;

	POST_PROCESSING_DEFINE_VARS( m_hDoFBlur );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramDoFBlur )
		SHADER_DEFINE_INTERNAL_NAME( "DoFBlur" )
		SHADER_DEFINE_UNIFORM( iChannel0 )
		SHADER_DEFINE_UNIFORM( depthmap )
		SHADER_DEFINE_UNIFORM( interptype )
		SHADER_DEFINE_UNIFORM( znear )
		SHADER_DEFINE_UNIFORM( zfar )
		SHADER_DEFINE_UNIFORM( distance )
		SHADER_DEFINE_UNIFORM( bokeh )
		SHADER_DEFINE_UNIFORM( samples )
		SHADER_DEFINE_UNIFORM( radius )
		SHADER_DEFINE_UNIFORM( dir )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramDoFBlur, m_ShaderDoFBlur );
};

EXTERN_FEATURE( CShaderDoFBlur, dofblur );

#endif // SINT_FEATURE_SHADER_DOF_BLUR_H
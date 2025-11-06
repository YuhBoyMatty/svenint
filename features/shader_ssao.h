// SvenInt (c) Sw1ft
// shader_ssao.h

#ifndef SINT_FEATURE_SHADER_SSAO_H
#define SINT_FEATURE_SHADER_SSAO_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// SSAO shader feature
//-----------------------------------------------------------------------------

class CShaderSSAO final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderSSAO( const char *pszCategoryName, const char *pszName );

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

	CMenuValueBool *m_pAmbientOcclusion;
	CMenuValueInteger *m_pSamples;
	CMenuValueFloat *m_pZNear;
	CMenuValueFloat *m_pZFar;

	CMenuValueFloat *m_pStrength;
	CMenuValueFloat *m_pRadius;
	CMenuValueFloat *m_pDepthClamp;
	CMenuValueFloat *m_pSelfShadowingReduction;
	CMenuValueFloat *m_pGaussBellCenter;
	CMenuValueFloat *m_pLuminanceAffection;

	CMenuValueBool *m_pNoise;
	CMenuValueFloat *m_pNoiseDitheringAmount;

	CMenuValueBool *m_pMist;
	CMenuValueFloat *m_pMistStart;
	CMenuValueFloat *m_pMistEnd;

	POST_PROCESSING_DEFINE_VARS( m_hSSAO );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramSSAO )
		SHADER_DEFINE_INTERNAL_NAME( "SSAO" )
		SHADER_DEFINE_UNIFORM( iChannel0 )
		SHADER_DEFINE_UNIFORM( depthmap )
		SHADER_DEFINE_UNIFORM( zNear )
		SHADER_DEFINE_UNIFORM( zFar )
		SHADER_DEFINE_UNIFORM( strength )
		SHADER_DEFINE_UNIFORM( samples )
		SHADER_DEFINE_UNIFORM( radius )
		SHADER_DEFINE_UNIFORM( aoclamp )
		SHADER_DEFINE_UNIFORM( noise )
		SHADER_DEFINE_UNIFORM( noiseamount )
		SHADER_DEFINE_UNIFORM( diffarea )
		SHADER_DEFINE_UNIFORM( gdisplace )
		SHADER_DEFINE_UNIFORM( mist )
		SHADER_DEFINE_UNIFORM( miststart )
		SHADER_DEFINE_UNIFORM( mistend )
		SHADER_DEFINE_UNIFORM( onlyAO )
		SHADER_DEFINE_UNIFORM( lumInfluence )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramSSAO, m_ShaderSSAO );
};

EXTERN_FEATURE( CShaderSSAO, ssao );

#endif // SINT_FEATURE_SHADER_SSAO_H
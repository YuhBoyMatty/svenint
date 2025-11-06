// SvenInt (c) Sw1ft
// shader_bloom.h

#ifndef SINT_FEATURE_SHADER_BLOOM_H
#define SINT_FEATURE_SHADER_BLOOM_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Bloom shader feature
//-----------------------------------------------------------------------------

class CShaderBloom final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderBloom( const char *pszCategoryName, const char *pszName );

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
	CMenuValueFloat *m_pSize;
	CMenuValueFloat *m_pIntensity;

	POST_PROCESSING_DEFINE_VARS( m_hBloom );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramBloom )
		SHADER_DEFINE_INTERNAL_NAME( "Bloom" )
		SHADER_DEFINE_UNIFORM( size )
		SHADER_DEFINE_UNIFORM( intensity )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramBloom, m_ShaderBloom );
};

EXTERN_FEATURE( CShaderBloom, bloom );

#endif // SINT_FEATURE_SHADER_BLOOM_H
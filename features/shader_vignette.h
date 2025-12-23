// SvenInt (c) Sw1ft
// shader_vignette.h

#ifndef SINT_FEATURE_SHADER_VIGNETTE_H
#define SINT_FEATURE_SHADER_VIGNETTE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Vignette shader feature
//-----------------------------------------------------------------------------

class CShaderVignette final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderVignette( const char *pszCategoryName, const char *pszName );

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
	CMenuValueFloat *m_pFalloff;
	CMenuValueFloat *m_pAmount;

	POST_PROCESSING_DEFINE_VARS( m_hVignette );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramVignette )
		SHADER_DEFINE_INTERNAL_NAME( "Vignette" )
		SHADER_DEFINE_UNIFORM( falloff )
		SHADER_DEFINE_UNIFORM( amount )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramVignette, m_ShaderVignette );
};

EXTERN_FEATURE( CShaderVignette, vignette );

#endif // SINT_FEATURE_SHADER_VIGNETTE_H
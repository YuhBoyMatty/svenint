// SvenInt (c) Sw1ft
// shader_depth_buffer.h

#ifndef SINT_FEATURE_SHADER_DEPTHBUFFER_H
#define SINT_FEATURE_SHADER_DEPTHBUFFER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Depth buffer shader feature
//-----------------------------------------------------------------------------

class CShaderDepthBuffer final : public CBaseFeature, IHookEventListener
{
public:
	CShaderDepthBuffer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline GLuint GetDepthTexture( void ) const { return m_hDepthBufferTex; }

private:
	CMenuValueBool *m_pShow;
	CMenuValueFloat *m_pZNear;
	CMenuValueFloat *m_pZFar;
	CMenuValueFloat *m_pBrightness;

	POST_PROCESSING_DEFINE_VARS( m_hDepthBuffer );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramDepthBuffer )
		SHADER_DEFINE_INTERNAL_NAME( "DepthBuffer" )
		SHADER_DEFINE_UNIFORM( znear )
		SHADER_DEFINE_UNIFORM( zfar )
		SHADER_DEFINE_UNIFORM( factor )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramDepthBuffer, m_ShaderDepthBuffer );
};

EXTERN_FEATURE( CShaderDepthBuffer, depthbuffer );

#endif // SINT_FEATURE_SHADER_DEPTHBUFFER_H
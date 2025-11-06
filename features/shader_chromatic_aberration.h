// SvenInt (c) Sw1ft
// shader_chromatic_aberration.h

#ifndef SINT_FEATURE_SHADER_CHROMATICABERRATION_H
#define SINT_FEATURE_SHADER_CHROMATICABERRATION_H

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

class CShaderChromaticAberration final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderChromaticAberration( const char *pszCategoryName, const char *pszName );

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
	CMenuValueFloat *m_pDirX;
	CMenuValueFloat *m_pDirY;
	CMenuValueFloat *m_pShift;
	CMenuValueFloat *m_pStrength;

	POST_PROCESSING_DEFINE_VARS( m_hChromaticAberration );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramChromaticAberration )
		SHADER_DEFINE_INTERNAL_NAME( "ChromaticAberration" )
		SHADER_DEFINE_UNIFORM( type )
		SHADER_DEFINE_UNIFORM( shift )
		SHADER_DEFINE_UNIFORM( strength )
		SHADER_DEFINE_UNIFORM( dir )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramChromaticAberration, m_ShaderChromaticAberration );
};

EXTERN_FEATURE( CShaderChromaticAberration, chromaticaberration );

#endif // SINT_FEATURE_SHADER_CHROMATICABERRATION_H
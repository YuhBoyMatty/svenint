// SvenInt (c) Sw1ft
// shader_color_correction.h

#ifndef SINT_FEATURE_SHADER_COLORCORRECTION_H
#define SINT_FEATURE_SHADER_COLORCORRECTION_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "modules/opengl.h"

//-----------------------------------------------------------------------------
// Color correction shader feature
//-----------------------------------------------------------------------------

class CShaderColorCorrection final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CShaderColorCorrection( const char *pszCategoryName, const char *pszName );

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
	float m_flGrainTime;

	CMenuElementButton *m_pButtonRecompile;
	CMenuValueFloat *m_pTargetGamma;
	CMenuValueFloat *m_pMonitorGamma;
	CMenuValueFloat *m_pHueOffset;
	CMenuValueFloat *m_pSaturation;
	CMenuValueFloat *m_pContrast;
	CMenuValueFloat *m_pLuminance;
	CMenuValueFloat *m_pBlackLevel;
	CMenuValueFloat *m_pBrightBoost;
	CMenuValueFloat *m_pRedLevel;
	CMenuValueFloat *m_pGreenLevel;
	CMenuValueFloat *m_pBlueLevel;
	CMenuValueFloat *m_pGrain;

	POST_PROCESSING_DEFINE_VARS( m_hColorCorrection );

	SHADER_BEGIN_DESC_MEMBER( CShaderProgramColorCorrection )
		SHADER_DEFINE_INTERNAL_NAME( "ColorCorrection" )
		SHADER_DEFINE_UNIFORM( iTime )
		SHADER_DEFINE_UNIFORM( ia_target_gamma )
		SHADER_DEFINE_UNIFORM( ia_monitor_gamma )
		SHADER_DEFINE_UNIFORM( ia_hue_offset )
		SHADER_DEFINE_UNIFORM( ia_saturation )
		SHADER_DEFINE_UNIFORM( ia_contrast )
		SHADER_DEFINE_UNIFORM( ia_luminance )
		SHADER_DEFINE_UNIFORM( ia_black_level )
		SHADER_DEFINE_UNIFORM( ia_bright_boost )
		SHADER_DEFINE_UNIFORM( ia_R )
		SHADER_DEFINE_UNIFORM( ia_G )
		SHADER_DEFINE_UNIFORM( ia_B )
		SHADER_DEFINE_UNIFORM( ia_GRAIN_STR )
		SHADER_DEFINE_UNIFORM( res )
		SHADER_END_DESC();
	SHADER_CREATE( CShaderProgramColorCorrection, m_ShaderColorCorrection );
};

EXTERN_FEATURE( CShaderColorCorrection, colorcorrection );

#endif // SINT_FEATURE_SHADER_COLORCORRECTION_H
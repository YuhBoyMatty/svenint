// SvenInt (c) Sw1ft
// r_skybox.h

#ifndef SINT_FEATURE_SKYBOX_H
#define SINT_FEATURE_SKYBOX_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Skybox feature
//-----------------------------------------------------------------------------

class CSkybox final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback, IConfigListener
{
public:
	CSkybox( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;
	virtual void OnConfigLoad( const char *pszFilename, bool bShaderConfig ) override;
	virtual void OnConfigSave( const char *pszFilename, bool bShaderConfig ) override { };

public:
	void R_LoadSkyBoxIntEvent( const char *pszSkyboxName, int loaded );
	void Replace( const char *pszSkyboxName );
	void Reset( void );
	void SaveOriginalSkybox( const char *pszSkyboxName );

private:
	CMenuElementButton *m_pButtonChangeSkybox;
	CMenuElementButton *m_pButtonResetSkybox;
	CMenuValueList *m_pSkyboxName;

	void *m_pfnR_LoadSkyboxInt;
	DetourHandle_t m_hR_LoadSkyboxInt;

	bool m_bSkyboxReplaced;
	bool m_bLoadingSkybox;
	bool m_bSkyboxLoaded;

	char m_szSkyboxName[ 128 ] = { 0 };
	char m_szCurrentSkyboxName[ 128 ] = { 0 };
	char m_szOriginalSkyboxName[ 128 ] = { 0 };

	float m_flNextThinkTime;
};

EXTERN_FEATURE( CSkybox, skybox );

#endif // SINT_FEATURE_SKYBOX_H
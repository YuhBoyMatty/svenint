// SvenInt (c) Sw1ft
// player_flashlight.h

#ifndef SINT_FEATURE_FLASHLIGHT_H
#define SINT_FEATURE_FLASHLIGHT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Flashlight feature
//-----------------------------------------------------------------------------

class CFlashlight final : public CBaseFeature
{
public:
	CFlashlight( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	void EmitCustomFlashlight( cl_entity_t *pEntity );

private:
	CMenuValueBool *m_pSelf;
	CMenuValueFloat *m_pSelfDistance;
	CMenuValueFloat *m_pSelfFalloffDistance;
	CMenuValueFloat *m_pSelfRadius;
	CMenuValueColorRGB *m_pSelfColor;

	CMenuValueBool *m_pSelfLighting;
	CMenuValueFloat *m_pSelfLightingDistance;
	CMenuValueFloat *m_pSelfLightingRadius;
	CMenuValueColorRGB *m_pSelfLightingColor;

	CMenuValueBool *m_pPlayers;
	CMenuValueFloat *m_pPlayersDistance;
	CMenuValueFloat *m_pPlayersFalloffDistance;
	CMenuValueFloat *m_pPlayersRadius;
	CMenuValueColorRGB *m_pPlayersColor;

	CMenuValueBool *m_pPlayersLighting;
	CMenuValueFloat *m_pPlayersLightingDistance;
	CMenuValueFloat *m_pPlayersLightingRadius;
	CMenuValueColorRGB *m_pPlayersLightingColor;

	void *m_pfnCL_PlayerFlashlight;
	DetourHandle_t m_hCL_PlayerFlashlight;
};

EXTERN_FEATURE( CFlashlight, flashlight );

#endif // SINT_FEATURE_FLASHLIGHT_H
// SvenInt (c) Sw1ft
// misc_game_patches.h

#ifndef SINT_FEATURE_GAME_PATCHES_H
#define SINT_FEATURE_GAME_PATCHES_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Ignore different map versions feature
//-----------------------------------------------------------------------------

class CGamePatches final : public CBaseFeature
{
public:
	CGamePatches( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

public:
	inline DetourHandle_t GetGLStringDetour( void ) { return m_hglGetString; }
	inline bool IgnoreAltInThirdPerson( void ) const { return m_pIgnoreAltInThirdPerson->GetBool(); }

private:
	void InitTertiaryAttackPatches( void );
	bool GuessTertiaryAttackVtidx( void );

private:
	CMenuValueBool *m_pIgnoreAltInThirdPerson;
	CMenuValueBool *m_pTertiaryAttackGlitch;

	bool m_bTertiaryAttackGlitchPatchable;
	int m_vtidx_CBasePlayerWeapon_TertiaryAttack;
	int m_vtidx_CBasePlayerWeapon_TertiaryAttack_Server;

	void *m_pglClearArg1;
	void *m_p31fpsFPU;
	void *m_pfnCCamera__Process;
	void *m_pfnCL_ComputeClientInterpolationAmount;

	DetourHandle_t m_hSDL_GL_ExtensionSupported;
	DetourHandle_t m_hglGetString;
	DetourHandle_t m_hCCamera__Process;
	DetourHandle_t m_hCL_ComputeClientInterpolationAmount;

	std::vector<DetourHandle_t> m_TertiaryAttackGlitchPatches;
};

EXTERN_FEATURE( CGamePatches, gamepatches );

#endif // SINT_FEATURE_GAME_PATCHES_H
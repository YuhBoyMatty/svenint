// SvenInt (c) Sw1ft
// r_viewmodel_tweaks.h

#ifndef SINT_FEATURE_VIEWMODEL_TWEAKS_H
#define SINT_FEATURE_VIEWMODEL_TWEAKS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Viewmodel tweaks feature
//-----------------------------------------------------------------------------

class CViewModelTweaks final : public CBaseFeature, IHookEventListener
{
public:
	CViewModelTweaks( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pLeftHanded;
	CMenuValueBool *m_pSemiTransparent;
	CMenuValueBool *m_pDisableGlowShell;
	CMenuValueBool *m_pDisableIdleAnim;
	CMenuValueBool *m_pDisableEquipAnim;
	CMenuValueFloat *m_pFOV;
};

EXTERN_FEATURE( CViewModelTweaks, viewmodeltweaks );

#endif // SINT_FEATURE_VIEWMODEL_TWEAKS_H
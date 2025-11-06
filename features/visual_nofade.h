// SvenInt (c) Sw1ft
// visual_nofade.h

#ifndef SINT_FEATURE_NOFADE_H
#define SINT_FEATURE_NOFADE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// No screen fade feature
//-----------------------------------------------------------------------------

class CNoFade final : public CBaseFeature
{
public:
	CNoFade( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	void *m_pfnV_FadeAlpha;
	DetourHandle_t m_hV_FadeAlpha;

	DetourHandle_t m_hUserMsgHook_ScreenFade;
};

EXTERN_FEATURE( CNoFade, nofade );

#endif // SINT_FEATURE_NOFADE_H
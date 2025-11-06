// SvenInt (c) Sw1ft
// visual_noshake.h

#ifndef SINT_FEATURE_NOSHAKE_H
#define SINT_FEATURE_NOSHAKE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// No screen shake feature
//-----------------------------------------------------------------------------

class CNoShake final : public CBaseFeature
{
public:
	CNoShake( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	DetourHandle_t m_hUserMsgHook_ScreenShake;
};

EXTERN_FEATURE( CNoShake, noshake );

#endif // SINT_FEATURE_NOSHAKE_H
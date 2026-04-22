// SvenInt (c) Sw1ft
// visual_nomotd.h

#ifndef SINT_FEATURE_NOMOTD_H
#define SINT_FEATURE_NOMOTD_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// No MOTD feature
//-----------------------------------------------------------------------------

class CNoMOTD final : public CBaseFeature
{
public:
	CNoMOTD( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	DetourHandle_t m_hUserMsgHook_MOTD;
	DetourHandle_t m_hUserMsgHook_VGUIMenu;
};

EXTERN_FEATURE( CNoMOTD, nomotd );

#endif // SINT_FEATURE_NOMOTD_H
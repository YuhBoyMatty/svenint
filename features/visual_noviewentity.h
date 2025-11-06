// SvenInt (c) Sw1ft
// visual_noviewentity.h

#ifndef SINT_FEATURE_NOVIEWENTITY_H
#define SINT_FEATURE_NOVIEWENTITY_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Disable forced view entity feature
//-----------------------------------------------------------------------------

class CNoViewEntity final : public CBaseFeature
{
public:
	CNoViewEntity( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	DetourHandle_t m_hNetMsgHook_SetView;
};

EXTERN_FEATURE( CNoViewEntity, noviewentity );

#endif // SINT_FEATURE_NOVIEWENTITY_H
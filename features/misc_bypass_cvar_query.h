// SvenInt (c) Sw1ft
// misc_bypass_cvar_query.h

#ifndef SINT_FEATURE_BYPASS_CVAR_QUERY_H
#define SINT_FEATURE_BYPASS_CVAR_QUERY_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"

//-----------------------------------------------------------------------------
// Bypass cvar query feature
//-----------------------------------------------------------------------------

class CBypassCvarQuery final : public CBaseFeature
{
public:
	CBypassCvarQuery( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	DetourHandle_t	m_hNetMsgHook_SendCvarValue;
	DetourHandle_t	m_hNetMsgHook_SendCvarValue2;
};

EXTERN_FEATURE( CBypassCvarQuery, bypasscvarquery );

#endif // SINT_FEATURE_BYPASS_CVAR_QUERY_H
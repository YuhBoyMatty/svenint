// SvenInt (c) Sw1ft
// misc_nosleep.h

#ifndef SINT_FEATURE_NOSLEEP_H
#define SINT_FEATURE_NOSLEEP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"

//-----------------------------------------------------------------------------
// No sleep feature
//-----------------------------------------------------------------------------

class CNoSleep final : public CBaseFeature
{
public:
	CNoSleep( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

private:
	DetourHandle_t	m_hCGame__SleepUntilInput;
	void			*m_pfnCGame__SleepUntilInput;
};

EXTERN_FEATURE( CNoSleep, nosleep );

#endif // SINT_FEATURE_NOSLEEP_H
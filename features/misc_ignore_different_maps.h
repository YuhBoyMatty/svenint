// SvenInt (c) Sw1ft
// misc_ignore_different_maps.h

#ifndef SINT_FEATURE_IGNORE_DIFFERENT_MAPS_H
#define SINT_FEATURE_IGNORE_DIFFERENT_MAPS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Ignore different map versions feature
//-----------------------------------------------------------------------------

class CIgnoreDifferentMaps final : public CBaseFeature, IHookEventListener
{
public:
	CIgnoreDifferentMaps( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void CheckMapCRC( uint32_t *ulCRC, char *pszMapName );

private:
	bool m_bCheckMapCRC;

	void *m_pfnCRC_MapFile;
	DetourHandle_t m_hCRC_MapFile;
};

EXTERN_FEATURE( CIgnoreDifferentMaps, ignorediffmaps );

#endif // SINT_FEATURE_IGNORE_DIFFERENT_MAPS_H
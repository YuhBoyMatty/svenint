// SvenInt (c) Sw1ft
// misc_soundcache.h

#ifndef SINT_FEATURE_SOUNDCACHE_H
#define SINT_FEATURE_SOUNDCACHE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto save soundcache feature
//-----------------------------------------------------------------------------

class CSoundcache final : public CBaseFeature, IHookEventListener
{
public:
	CSoundcache( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void NetMsgHookResourceList( void );
	void SoundEngineLoadSoundList( void );
	bool SoundEngineFlushCache( void );

private:
	CMenuValueList *m_pMode;
	CMenuValueBool *m_pDontFlush;
	CMenuValueBool *m_pIgnore;

	bool m_bHasSoundcache;
	bool m_bUseSavedSoundcache;

	void *m_pfnCClient_SoundEngine__LoadSoundList;
	void *m_pfnCClient_SoundEngine__FlushCache;

	DetourHandle_t m_hCClient_SoundEngine__LoadSoundList;
	DetourHandle_t m_hCClient_SoundEngine__FlushCache;
	DetourHandle_t m_hNetMsgHook_ResourceList;
};

EXTERN_FEATURE( CSoundcache, soundcache );

#endif // SINT_FEATURE_SOUNDCACHE_H
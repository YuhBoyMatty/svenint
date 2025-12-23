// SvenInt (c) Sw1ft
// misc_mute_manager.h

#ifndef SINT_FEATURE_MUTE_MANAGER_H
#define SINT_FEATURE_MUTE_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "utils/hashtable.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Mute flags
//-----------------------------------------------------------------------------

#define MUTE_NONE ( 0 )
#define MUTE_VOICE ( 0x10 )
#define MUTE_CHAT ( 0x20 )
#define MUTE_ALL ( MUTE_VOICE | MUTE_CHAT )

//-----------------------------------------------------------------------------
// Mute manager feature
//-----------------------------------------------------------------------------

class CMuteManager final : public CBaseFeature
{
public:
	CMuteManager( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;

public:
	void LoadMutedPlayers( void );
	void SaveMutedPlayers( void );

	inline void ClearMutedPlayers( void ) { m_MutedPlayers.Clear(); }
	inline void RemoveMutedPlayers( void ) { m_MutedPlayers.Purge(); }

	inline uint32 *GetMutedPlayer( uint64 steamid ) { return m_MutedPlayers.Find( steamid ); }
	inline bool AddMutedPlayer( uint64 steamid, uint32 flags )
	{
		auto result = m_MutedPlayers.Insert( steamid, flags, OnPlayerFound );
		if ( m_pAutosave->GetBool() )
			SaveMutedPlayers();
		return result;
	}
	inline bool RemoveMutedPlayer( uint64 steamid, uint32 flags )
	{
		auto result = m_MutedPlayers.Remove( steamid, OnPlayerRemove, &flags );
		if ( m_pAutosave->GetBool() )
			SaveMutedPlayers();
		return result;
	}

	inline const CHashTable<uint64, uint32> &GetMutedPlayersTable( void ) const { return m_MutedPlayers; }

	inline int GetLastIndexedPlayer( void ) const { return m_nLastIndexedPlayer; }
	inline void SetLastIndexedPlayer( int ndx ) { m_nLastIndexedPlayer = ndx; }
	
	inline bool IsProcessingChat( void ) const { return m_bProcessingChat; }
	inline void SetProcessingChat( bool state ) { m_bProcessingChat = state; }

	inline bool AllCommunicationsMuted( void ) const { return m_pMuteAllCommunications->GetBool(); }

	void UpdateServerState( void *thisptr, bool bForce );

private:
	static void OnPlayerFound( uint32 *pFoundValue, uint32 *pInsertValue )
	{
		*pFoundValue |= *pInsertValue;
	}

	static bool OnPlayerRemove( uint32 *pRemoveValue, uint32 *pUserValue )
	{
		*pRemoveValue &= ~( *pUserValue );

		if ( *pRemoveValue != MUTE_NONE )
			return false;

		return true;
	}

private:
	CMenuValueBool *m_pMuteAllCommunications;
	CMenuValueBool *m_pAutosave;

	CHashTable<uint64, uint32> m_MutedPlayers;

	bool m_bProcessingChat;
	int m_nLastIndexedPlayer;
	uint32 m_BanMask;

	cvar_t *voice_clientdebug;
	cvar_t *voice_modenable;

	void *m_pfnCVoiceBanMgr__SetPlayerBan;
	void *m_pfnCVoiceBanMgr__InternalFindPlayerSquelch;
	void *m_pfnCVoiceStatus__IsPlayerBlocked;
	void *m_pfnCVoiceStatus__SetPlayerBlockedState;
	void *m_pfnCVoiceStatus__UpdateServerState;
	void *m_pfnHACK_GetPlayerUniqueID;

	DetourHandle_t m_hCVoiceBanMgr__SetPlayerBan;
	DetourHandle_t m_hCVoiceBanMgr__InternalFindPlayerSquelch;
	DetourHandle_t m_hCVoiceStatus__IsPlayerBlocked;
	DetourHandle_t m_hCVoiceStatus__SetPlayerBlockedState;
	DetourHandle_t m_hCVoiceStatus__UpdateServerState;
	DetourHandle_t m_hHACK_GetPlayerUniqueID;

	DetourHandle_t m_hUserMsgHook_SayText;
};

EXTERN_FEATURE( CMuteManager, mutemanager );

#endif // SINT_FEATURE_MUTE_MANAGER_H
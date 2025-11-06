// SvenInt (c) Sw1ft
// misc_votebot.h

#ifndef SINT_FEATURE_VOTEBOT_H
#define SINT_FEATURE_VOTEBOT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Vote bot feature
//-----------------------------------------------------------------------------

class CVoteBot final : public CBaseFeature, IHookEventListener
{
public:
	CVoteBot( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline std::vector<uint64_t> &GetTargets( void ) { return m_targets; }
	inline std::vector<uint64_t> &GetFriends( void ) { return m_friends; }

	inline uint64_t FindTarget( uint64_t steamID )
	{
		auto it = std::find( m_targets.begin(), m_targets.end(), steamID );
		if ( it != m_targets.end() )
			return *it;
		return 0uLL;
	}
	
	inline uint64_t FindFriend( uint64_t steamID )
	{
		auto it = std::find( m_friends.begin(), m_friends.end(), steamID );
		if ( it != m_friends.end() )
			return *it;
		return 0uLL;
	}

	void ReloadTargets( void );
	void ReloadFriends( void );
	void TextMsgEvent( const char *pszUserMsg, int iSize, void *pBuffer );

private:
	CMenuValueBool *m_pAutoReconnect;
	CMenuValueBool *m_pFilterFriends;
	CMenuValueBool *m_pIgnoreDeadPlayers;
	CMenuValueBool *m_pVoteInObserver;
	CMenuValueList *m_pMode;
	CMenuValueList *m_pTargetMode;
	CMenuValueList *m_pCallVoteMode;

	int m_prevstate;
	float m_flVoteCooldown;
	std::vector<uint64_t> m_targets;
	std::vector<uint64_t> m_friends;
	std::vector<int> m_voteTargets;

	DetourHandle_t m_hUserMsgHook_TextMsg;
};

EXTERN_FEATURE( CVoteBot, votebot );

#endif // SINT_FEATURE_VOTEBOT_H
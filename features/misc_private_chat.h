// SvenInt (c) Sw1ft
// misc_private_chat.h

#ifndef SINT_FEATURE_PRIVATE_CHAT_H
#define SINT_FEATURE_PRIVATE_CHAT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Private chat feature
//-----------------------------------------------------------------------------

class CPrivateChat final : public CBaseFeature, IHookEventListener
{
public:
	CPrivateChat( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void PostLoad( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	bool OnSendMessage( bool bTeam );
	void OnReceiveMessage( int client, const char *pszMessage );

private:
	void LoadWhitelistFromFile( void );

private:
	CMenuValueBool *m_pEncryptAlways;
	CMenuValueBool *m_pWhitelistedPlayers;
	CMenuValueText *m_pEncryptKey;

	bool m_bWaitSingleFrame;
	std::vector<uint64> m_whitelist;

	DetourHandle_t m_hUserMsgHook_SayText;
	DetourHandle_t m_hCmdHook_Say;
	DetourHandle_t m_hCmdHook_SayTeam;
};

EXTERN_FEATURE( CPrivateChat, privatechat );

#endif // SINT_FEATURE_PRIVATE_CHAT_H
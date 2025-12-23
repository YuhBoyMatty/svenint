// SvenInt (c) Sw1ft
// hud_chat_history.h

#ifndef SINT_FEATURE_CHAT_HISTORY_H
#define SINT_FEATURE_CHAT_HISTORY_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

FUNC_SIGNATURE( void, CALLCONV_THISCALL, CClient_SoundEngine__Play2DSoundFn, void *thisptr, const char *sound, float volume );

//-----------------------------------------------------------------------------
// Chat history message
//-----------------------------------------------------------------------------

class CChatHistoryMessage
{
public:
	CChatHistoryMessage( const char *pszMessage, const char *pszClientName, float *pflClientColor, float flTime )
	{
		m_sMsg = pszMessage;
		m_pflClientColor = pflClientColor;
		m_flReceiveTime = flTime;

		if ( pszClientName != NULL )
			m_sClientName = pszClientName;
	}

	std::string m_sMsg;
	std::string m_sClientName;
	float *m_pflClientColor;
	float m_flReceiveTime;
};

//-----------------------------------------------------------------------------
// Chat history feature
//-----------------------------------------------------------------------------

class CChatHistory final : public CBaseFeature, IHookEventListener
{
public:
	CChatHistory( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void PostLoad( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void OnReceiveMessage( int client, const char *pszMessage, int src );

private:
	void AddMessage( const char *pszMessage, const char *pszClientName, float *pflClientColor );

private:
	CMenuValueList *m_pAlignmentMode;
	CMenuValueInteger *m_pMaxHistory;
	CMenuValueFloat *m_pStayTime;
	CMenuValueFloat *m_pScreenWidthFraction;
	CMenuValueFloat *m_pScreenHeightFraction;
	CMenuValueFloat *m_pTextOpacity;
	CMenuValueColorRGB *m_pTextColor;

	bool m_bOriginalChatFlushRequired;

	int *key_dest;
	void **m_pSoundEngine;
	void *m_pfnCHudTextMessage__MsgFunc_TextMsg;
	CClient_SoundEngine__Play2DSoundFn m_pfnCClient_SoundEngine__Play2DSound;

	DetourHandle_t m_hUserMsgHook_SayText;
	DetourHandle_t m_hUserMsgHook_TextMsg;
#ifdef LINUX
	DetourHandle_t m_hUserMsgHook_TextMsg2;
#endif

	std::vector<CChatHistoryMessage> m_textHistory;
};

EXTERN_FEATURE( CChatHistory, chathistory );

#endif // SINT_FEATURE_CHAT_HISTORY_H

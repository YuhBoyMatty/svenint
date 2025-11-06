// SvenInt (c) Sw1ft
// hud_vote_popup.h

#ifndef SINT_FEATURE_VOTE_POPUP_H
#define SINT_FEATURE_VOTE_POPUP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "game/messagebuffer.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Vote types
//-----------------------------------------------------------------------------

typedef enum
{
	kVoteKill = 1,
	kVoteKick,
	kVoteBan,
	kVoteMap
} EVotePopup;

//-----------------------------------------------------------------------------
// Vote popup feature
//-----------------------------------------------------------------------------

class CVotePopup final : public CBaseFeature, IHookEventListener
{
public:
	CVotePopup( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	bool OnVoteStart( const char *pszUserMsg, int iSize, void *pBuffer );
	bool OnVoteEnd( const char *pszUserMsg, int iSize, void *pBuffer );

private:
	void Draw( void );
	void ValidateMessage( const char **pszMessage );

private:
	CMessageBuffer	m_VoteMenuBuffer;

	DetourHandle_t	m_hUserMsgHook_VoteMenu;
	DetourHandle_t	m_hUserMsgHook_EndVote;

	CMenuValueInteger	*m_pKeyYes;
	CMenuValueInteger	*m_pKeyNo;
	CMenuValueInteger	*m_pWidth;
	CMenuValueInteger	*m_pHeight;
	CMenuValueInteger	*m_pBorderWidth;
	CMenuValueInteger	*m_pBorderHeight;
	CMenuValueFloat		*m_pWidthScreenFraction;
	CMenuValueFloat		*m_pHeightScreenFraction;

	int				m_iVoteType;

	wchar_t			m_wszVoteTarget[ 128 ];
	wchar_t			m_wszVoteMessage[ 128 ];
	wchar_t			m_wszVoteYes[ 128 ];
	wchar_t			m_wszVoteNo[ 128 ];

	char			m_szVoteTarget[ 128 ];

	bool			m_bShowPopup;
	bool			m_bVoteStarted;
};

EXTERN_FEATURE( CVotePopup, votepopup );

#endif // SINT_FEATURE_VOTE_POPUP_H
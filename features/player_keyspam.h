// SvenInt (c) Sw1ft
// player_keyspam.h

#ifndef SINT_FEATURE_KEYSPAM_H
#define SINT_FEATURE_KEYSPAM_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Freeze feature
//-----------------------------------------------------------------------------

class CKeySpam final : public CBaseFeature, IHookEventListener
{
public:
	CKeySpam( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pHoldMode;
	CMenuValueBool *m_pSpamCTRL;
	CMenuValueBool *m_pSpamW;
	CMenuValueBool *m_pSpamS;
	CMenuValueBool *m_pSpamE;
	CMenuValueBool *m_pSpamQ;

	int *key_dest;
};

EXTERN_FEATURE( CKeySpam, keyspam );

#endif // SINT_FEATURE_KEYSPAM_H
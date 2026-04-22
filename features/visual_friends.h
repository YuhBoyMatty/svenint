// SvenInt (c) Sw1ft
// visual_friends.h

#ifndef SINT_FEATURE_FRIENDS_H
#define SINT_FEATURE_FRIENDS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Friends feature
//-----------------------------------------------------------------------------

class CFriends final : public CBaseFeature, IHookEventListener
{
public:
	CFriends( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	void LoadFriendsFromFile( void );

private:
	CMenuValueFloat *m_pWidthScreenFraction;
	CMenuValueFloat *m_pHeightScreenFraction;
	CMenuValueColorRGBA *m_pColor;

	std::vector<uint64> m_friends;
};

EXTERN_FEATURE( CFriends, friends );

#endif // SINT_FEATURE_FRIENDS_H
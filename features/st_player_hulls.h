// SvenInt (c) Sw1ft
// st_player_hulls.h

#ifndef SINT_FEATURE_PLAYER_HULLS_H
#define SINT_FEATURE_PLAYER_HULLS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

struct playerhull_display_info_t
{
	int		dead;

	Vector	origin;

	Vector	mins;
	Vector	maxs;

	float	time;
};

//-----------------------------------------------------------------------------
// Player hulls visualization feature
//-----------------------------------------------------------------------------

class CPlayerHulls final : public CBaseFeature, IHookEventListener
{
public:
	CPlayerHulls( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline const std::vector<playerhull_display_info_t> &GetHulls( void ) const { return m_playerHulls; }
	inline const playerhull_display_info_t &GetHull( int i ) const { return m_playerHulls[ i ]; }

	inline bool AreServerHullsDisplayed( void ) const { return m_pShowServerPlayerHulls->GetBool(); }

	void CheckPlayerHulls_Server( void );
	void DrawPlayersHullsNickname_Server( void );
	void BroadcastPlayerHull_Server( int client, int dead, const Vector &vecOrigin, bool bDuck );
	void DrawPlayerHull_Comm( int client, int dead, const Vector &vecOrigin, bool bDuck );
	void DrawPlayerHulls( void );

private:
	CMenuValueBool *m_pShowServerPlayerHulls;
	CMenuValueBool *m_pShowLocalPlayerHull;
	CMenuValueBool *m_pShowWireframe;
	CMenuValueFloat *m_pWireframeWidth;
	CMenuValueColorRGBA *m_pColor;
	CMenuValueColorRGBA *m_pDeadPlayerColor;

	std::vector<playerhull_display_info_t> m_playerHulls;
	float m_flDisplayHullsNextSend;
};

EXTERN_FEATURE( CPlayerHulls, playerhulls );

#endif // SINT_FEATURE_PLAYER_HULLS_H
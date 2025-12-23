// SvenInt (c) Sw1ft
// hud_chat_colors.h

#ifndef SINT_FEATURE_CHATCOLORS_H
#define SINT_FEATURE_CHATCOLORS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Chat colors info
//-----------------------------------------------------------------------------

struct chat_colors_info_t
{
	uint64_t steamID;
	int ndxColor;
};

//-----------------------------------------------------------------------------
// Chat colors feature
//-----------------------------------------------------------------------------

class CChatColors final : public CBaseFeature, IHookEventListener
{
public:
	CChatColors( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void PostLoad( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

	float *GetColor( int iPlayerIndex );
	inline float *GetRainbowColor( void ) { return m_flRainbowColor; }
	inline CHudBaseTextBlock *GetHudBaseTextBlock( void ) const { return m_pHudBaseTextBlock; }
	inline void SaveHudBaseTextBlock( CHudBaseTextBlock *pHudBaseTextBlock ) { m_pHudBaseTextBlock = pHudBaseTextBlock; }

private:
	void LoadPlayers( void );

private:
	CMenuValueColorRGB *m_pDefaultColor;
	CMenuValueColorRGB *m_pColor1;
	CMenuValueColorRGB *m_pColor2;
	CMenuValueColorRGB *m_pColor3;
	CMenuValueColorRGB *m_pColor4;
	CMenuValueColorRGB *m_pColor5;
	CMenuValueColorRGB *m_pColor6;
	CMenuValueFloat *m_pRainbowUpdate;
	CMenuValueFloat *m_pRainbowHue;
	CMenuValueFloat *m_pRainbowSaturation;
	CMenuValueFloat *m_pRainbowLightness;

	std::vector<chat_colors_info_t> m_players;

	void			*m_pfnCHudBaseTextBlock__Print;
	DetourHandle_t	m_hCHudBaseTextBlock__Print;
	DetourHandle_t	m_hGetClientColor;

	CHudBaseTextBlock *m_pHudBaseTextBlock;

	float m_flRainbowDelta;
	float m_flRainbowColor[ 3 ];
	float m_flRainbowUpdateTime;
};

EXTERN_FEATURE( CChatColors, chatcolors );

#endif // SINT_FEATURE_CHATCOLORS_H
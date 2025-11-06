// SvenInt (c) Sw1ft
// player_color_pulsator.h

#ifndef SINT_FEATURE_COLOR_PULSATOR_H
#define SINT_FEATURE_COLOR_PULSATOR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Color pulsator feature
//-----------------------------------------------------------------------------

class CColorPulsator final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CColorPulsator( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

private:
	inline void SyncColors( void )
	{
		m_iBottomColorOffset = m_iTopColorOffset;

		if ( m_flTopColorDelay > m_flBottomColorDelay )
			m_flBottomColorDelay = m_flTopColorDelay;

		if ( m_flBottomColorDelay > m_flTopColorDelay )
			m_flTopColorDelay = m_flBottomColorDelay;
	}

private:
	CMenuElementButton *m_pButtonSync;
	CMenuValueBool *m_pTopColor;
	CMenuValueBool *m_pBottomColor;
	CMenuValueFloat *m_pUpdateDelay;

	int m_iTopColorOffset;
	float m_flTopColorDelay;

	int m_iBottomColorOffset;
	float m_flBottomColorDelay;
};

EXTERN_FEATURE( CColorPulsator, colorpulsator );

#endif // SINT_FEATURE_COLOR_PULSATOR_H
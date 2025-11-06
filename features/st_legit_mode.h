// SvenInt (c) Sw1ft
// st_legit_mode.h

#ifndef SINT_FEATURE_SPEEDRUN_LEGIT_MODE_H
#define SINT_FEATURE_SPEEDRUN_LEGIT_MODE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Legit mode feature
//-----------------------------------------------------------------------------

class CSpeedrunLegitMode final : public CBaseFeature, IHookEventListener
{
public:
	CSpeedrunLegitMode( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void SetLegitMode( bool state );
	bool IsLegitMode( void ) const;

private:
	bool m_bLegitMode;
};

EXTERN_FEATURE( CSpeedrunLegitMode, legitmode );

#endif // SINT_FEATURE_SPEEDRUN_LEGIT_MODE_H
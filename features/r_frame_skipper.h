// SvenInt (c) Sw1ft
// r_frame_skipper.h

#ifndef SINT_FEATURE_FRAME_SKIPPER_H
#define SINT_FEATURE_FRAME_SKIPPER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Frame skipper feature
//-----------------------------------------------------------------------------

class CFrameSkipper final : public CBaseFeature, IHookEventListener
{
public:
	CFrameSkipper( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pSkipSequence;
	CMenuValueInteger *m_pCount;
};

EXTERN_FEATURE( CFrameSkipper, frameskipper );

#endif // SINT_FEATURE_FRAME_SKIPPER_H
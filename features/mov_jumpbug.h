// SvenInt (c) Sw1ft
// mov_jumpbug.h

#ifndef SINT_FEATURE_JUMPBUG_H
#define SINT_FEATURE_JUMPBUG_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Auto jumpbug feature
//-----------------------------------------------------------------------------

class CJumpbug final : public CBaseFeature, IHookEventListener
{
public:
	CJumpbug( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline bool InProcess( void ) const { return m_nJumpBugState > 0; }

private:
	void Predict( bool &bInAir, bool &bDucking, Vector &vecOrigin, Vector &vecVelocity );

private:
	CMenuValueList *m_pMode;
	CMenuValueFloat *m_pMinFallVelocity;

	int m_nJumpBugState;
};

EXTERN_FEATURE( CJumpbug, jumpbug );

#endif // SINT_FEATURE_JUMPBUG_H
// SvenInt (c) Sw1ft
// mov_edgejump.h

#ifndef SINT_FEATURE_EDGEJUMP_H
#define SINT_FEATURE_EDGEJUMP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Edge jump feature
//-----------------------------------------------------------------------------

class CEdgeJump final : public CBaseFeature, IHookEventListener
{
public:
	CEdgeJump( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void Execute( usercmd_t *cmd, float frametime );

private:
	CMenuValueFloat *m_pHeight;
};

EXTERN_FEATURE( CEdgeJump, edgejump );

#endif // SINT_FEATURE_EDGEJUMP_H
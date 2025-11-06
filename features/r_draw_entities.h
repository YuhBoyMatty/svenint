// SvenInt (c) Sw1ft
// r_draw_entities.h

#ifndef SINT_FEATURE_DRAW_ENTITIES_H
#define SINT_FEATURE_DRAW_ENTITIES_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Draw entities feature
//-----------------------------------------------------------------------------

class CDrawEntities final : public CBaseFeature, IHookEventListener
{
public:
	CDrawEntities( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline int GetMode( void ) { return m_pMode->GetInt(); }

private:
	CMenuValueList *m_pMode;
	cvar_t *r_drawentities;
};

EXTERN_FEATURE( CDrawEntities, drawentities );

#endif // SINT_FEATURE_DRAW_ENTITIES_H
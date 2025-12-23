// SvenInt (c) Sw1ft
// mov_use_key.h 

#ifndef SINT_FEATURE_USEKEY_H
#define SINT_FEATURE_USEKEY_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// IN_USE input tweaks
//-----------------------------------------------------------------------------

class CUseKey final : public CBaseFeature, IHookEventListener
{
public:
	CUseKey( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pOGStyle;
	CMenuValueBool *m_pNoSlowdown;
	CMenuValueBool *m_pCustom;
	CMenuValueFloat *m_pCustomValue;

	kbutton_t *in_speed;
	cvar_t *cl_movespeedkey;
	float m_flOldValue;
};

EXTERN_FEATURE( CUseKey, usekey );

#endif // SINT_FEATURE_USEKEY_H
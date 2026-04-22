// SvenInt (c) Sw1ft
// Took it from my private project l4dst
// visual_inputs.h

#ifndef SINT_FEATURE_INPUTS_H
#define SINT_FEATURE_INPUTS_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Inputs visualization feature
//-----------------------------------------------------------------------------

class CInputs final : public CBaseFeature, IHookEventListener
{
public:
	CInputs( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueColorRGBA *m_pColor;
	CMenuValueBool *m_pShowButtonInputs;
	CMenuValueFloat *m_pButtonInputsAnchorX;
	CMenuValueFloat *m_pButtonInputsAnchorY;
	CMenuValueBool *m_pShowMoveInputs;
	CMenuValueBool *m_pShowMoveInputsDirection;
	CMenuValueInteger *m_pMoveInputsCircleSize;
	CMenuValueInteger *m_pMoveInputsDirCircleSize;
	CMenuValueFloat *m_pMoveInputsAnchorX;
	CMenuValueFloat *m_pMoveInputsAnchorY;
};

EXTERN_FEATURE( CInputs, inputs );

#endif // SINT_FEATURE_INPUTS_H

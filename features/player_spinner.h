// SvenInt (c) Sw1ft
// player_spinner.h

#ifndef SINT_FEATURE_SPINNER_H
#define SINT_FEATURE_SPINNER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"
#include "features/mov_strafer.h"

//-----------------------------------------------------------------------------
// Spinner feature
//-----------------------------------------------------------------------------

class CSpinner final : public CBaseFeature, IHookEventListener
{
public:
	CSpinner( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	CMenuValueBool *m_pRotateDeadBody;
	CMenuValueBool *m_pRevertPitch;
	CMenuValueBool *m_pRevertYaw;

	CMenuValueBool *m_pLockPitch;
	CMenuValueFloat *m_pPitchAngle;

	CMenuValueBool *m_pLockYaw;
	CMenuValueFloat *m_pYawAngle;

	CMenuValueBool *m_pSpinPitch;
	CMenuValueFloat *m_pSpinPitchAngle;

	CMenuValueBool *m_pSpinYaw;
	CMenuValueFloat *m_pSpinYawAngle;

	Vector m_vecSpinAngles;
};

EXTERN_FEATURE( CSpinner, spinner );

#endif // SINT_FEATURE_SPINNER_H
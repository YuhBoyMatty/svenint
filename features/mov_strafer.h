// SvenInt (c) Sw1ft
// mov_strafer.h

#ifndef SINT_FEATURE_STRAFER_H
#define SINT_FEATURE_STRAFER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include "strafe/strafe.h"

//-----------------------------------------------------------------------------
// Strafer feature
//-----------------------------------------------------------------------------

class CStrafer final : public CBaseFeature, IHookEventListener
{
public:
	CStrafer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	Strafe::Frame			&GetStrafeFrame( usercmd_t *cmd, float frametime, bool bStrafe, bool bStrafeVectorial, Strafe::StrafeDir dir, Strafe::StrafeType type, float flYaw, float flPointX, float flPointY );
	inline Strafe::Frame	&GetStrafeFrame( void ) { return m_strafeFrame; }

	inline bool IsStrafed( void ) const { return m_bStrafed; }
	inline bool IsBypassEnabled( void ) const { return m_pBypassAntiStrafer->GetBool(); }

private:
	bool BypassAntiStrafer( usercmd_t *cmd, Strafe::ProcessedFrame out, float *va, const bool bLastStrafedRightOld, const bool bWasStandingOnGround );

private:
	CMenuValueBool *m_pVectorialStrafer;
	CMenuValueBool *m_pIgnoreGround;
	CMenuValueBool *m_pBypassAntiStrafer;
	CMenuValueList *m_pBypassMode;
	CMenuValueBool *m_pStrafeTowardsMovementButtons;
	CMenuValueBool *m_pStopWhenHoldingBackButton;
	CMenuValueList *m_pStrafeDir;
	CMenuValueList *m_pStrafeType;

	Strafe::Frame m_strafeFrame;

	bool m_bStrafed;
	bool m_bFlip;
	bool m_bSkipFlip;
	bool m_bLastStrafedRight;
};

EXTERN_FEATURE( CStrafer, strafer );

#endif // SINT_FEATURE_STRAFER_H
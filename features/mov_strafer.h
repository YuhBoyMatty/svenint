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
	void UpdateStrafeData( Strafe::StrafeData &strafedata, float frametime, bool bStrafe, Strafe::StrafeDir dir, Strafe::StrafeType type, float flYaw, float flPointX, float flPointY );
	inline bool IsStrafed( void ) const { return m_bStrafed; }
	inline bool IsBypassEnabled( void ) const { return m_pBypassAntiStrafer->GetBool(); }

private:
	CMenuValueBool *m_pVectorialStrafer;
	CMenuValueBool *m_pIgnoreGround;
	CMenuValueBool *m_pBypassAntiStrafer;
	CMenuValueList *m_pBypassMode;
	CMenuValueBool *m_pStrafeTowardsMovementButtons;
	CMenuValueBool *m_pStopWhenHoldingBackButton;
	CMenuValueList *m_pStrafeDir;
	CMenuValueList *m_pStrafeType;

	Strafe::StrafeData m_strafeData;

	struct cvar_s *sv_friction;
	struct cvar_s *sv_accelerate;
	struct cvar_s *sv_airaccelerate;
	struct cvar_s *sv_stopspeed;

	bool m_bStrafed;
};

EXTERN_FEATURE( CStrafer, strafer );

#endif // SINT_FEATURE_STRAFER_H
// SvenInt (c) Sw1ft
// player_freeze.h

#ifndef SINT_FEATURE_FREEZE_H
#define SINT_FEATURE_FREEZE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Freeze feature
//-----------------------------------------------------------------------------

class CFreeze final : public CBaseFeature, IHookEventListener
{
public:
	CFreeze( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline bool IsManualUse( void ) const { return m_bManualUse; }
	inline bool ToggleManualUse( void ) { m_bManualUse = !m_bManualUse; return m_bManualUse; }

	inline bool IsPacketCanceled( void ) const { return m_bCancelPacket || m_bManualUse && m_pMode->GetInt() == 1; }
	inline void CancelPacket( bool state ) { m_bCancelPacket = state; }

	inline bool IsTransmitCanceled( void ) const { return m_bCancelTransmit || m_bManualUse && m_pMode->GetInt() == 0; }
	inline void CancelTransmit( bool state ) { m_bCancelTransmit = state; }
	
	inline bool IsMovCmdCanceled( void ) const
	{
		if ( m_flNextCmdTime == NULL )
			return false;

		return m_bCancelMovCmd || m_bManualUse && m_pMode->GetInt() == 2;
	}
	inline void CancelMovCmd( bool state )
	{
		if ( m_flNextCmdTime == NULL )
			return;

		if ( state )
			*m_flNextCmdTime = FLT_MAX;
		else
			*m_flNextCmdTime = 0.f;

		m_bCancelMovCmd = state;
	}
	inline void SetNextCmdTime( float time )
	{
		if ( m_flNextCmdTime == NULL )
			return;

		*m_flNextCmdTime = time;
	}

private:
	bool m_bManualUse;
	CMenuValueList *m_pMode;

	float *m_flNextCmdTime;

	// For use by any other features
	bool m_bCancelPacket;
	bool m_bCancelTransmit;
	bool m_bCancelMovCmd;

	void *m_pfnNetchan_CanPacket;
	DetourHandle_t m_hNetchan_CanPacket;
};

EXTERN_FEATURE( CFreeze, freeze );

#endif // SINT_FEATURE_FREEZE_H
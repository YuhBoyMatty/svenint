// SvenInt (c) Sw1ft
// player_firstperson_roaming.cpp

#include "stdafx.h"
#include "player_firstperson_roaming.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFirstPersonRoaming, firstpersonroaming, "Player", "First Person Roaming" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

void CFirstPersonRoaming::GetPlayerViewAngles( Vector &vecOutput )
{
	vecOutput = m_pTarget->curstate.angles;

	// Transform this weird Pitch range [-9.887 ... 9.887] to [-89.0 ... 89.0]
	vecOutput.x *= ( 89.0f / 9.8876953125f );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CFirstPersonRoaming::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		auto pparams = pEvent->GetArg<ref_params_t *>( "pparams" );

		int iSpectatorMode = playermove->iuser1();
		int iTarget = playermove->iuser2();

		if ( iSpectatorMode != OBS_ROAMING || iTarget <= 0 )
		{
			m_iSpectatorMode = iSpectatorMode;
			m_iTarget = -1;
			m_pTarget = NULL;
			return kHookContinue;
		}

		m_pTarget = cl_enginefuncs->GetEntityByIndex( iTarget ); // target player
		if ( !m_pTarget || !m_pTarget->model ) // player is invalid
		{
			m_iSpectatorMode = OBS_ROAMING;
			m_iTarget = -1;
			m_pTarget = NULL;
			return kHookContinue;
		}

		// Spectator mode or target was changed
		if ( m_iSpectatorMode != OBS_ROAMING || iTarget != m_iTarget )
			GetPlayerViewAngles( m_vecPrevAngles );

		m_iSpectatorMode = OBS_ROAMING;
		m_iTarget = iTarget;

		// Override view
		*reinterpret_cast<Vector *>( pparams->vieworg ) = m_pTarget->origin + Vector( 0.f, 0.f, m_pTarget->curstate.usehull ? 12.5f : 28.5f /* VEC_DUCK_VIEW.z : VEC_VIEW.z */ );
		*reinterpret_cast<Vector *>( pparams->viewangles ) = m_vecPrevAngles;

		// Lerp angles
		if ( m_pLerp->GetBool() )
		{
			Vector from, to;
			Vector wth_is_that_arg_pass;

			from = m_vecPrevAngles;
			GetPlayerViewAngles( to );

			m_vecPrevAngles.x = from.x + ( to.x - from.x ) * m_pLerpValue->GetFloat();
			m_vecPrevAngles.y = NormalizeAngle( from.y + NormalizeAngle( to.y - from.y ) * m_pLerpValue->GetFloat() );
			m_vecPrevAngles.z = 0.f;
		}
		else
		{
			GetPlayerViewAngles( m_vecPrevAngles );
		}
	}
	else if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_iTarget = -1;
		m_iSpectatorMode = 0;

		m_pTarget = NULL;
	}
	else if ( studiorenderer->m_pCurrentEntity == m_pTarget ) // StudioRenderModel event
	{
		return kHookSupercede;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFirstPersonRoaming::CFirstPersonRoaming( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pLerp = NULL;
	m_pLerpValue = NULL;

	m_iTarget = -1;
	m_iSpectatorMode = 0;

	m_pTarget = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CFirstPersonRoaming::OnEnable( void )
{
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CFirstPersonRoaming::OnDisable( void )
{
	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFirstPersonRoaming::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pLerp = Modules::menu->AddParamBool( this, "Lerp", NULL, true );
	m_pLerpValue = Modules::menu->AddParamFloat( this, "LerpValue", NULL, 0.15f, 0.001f, 1.f );

	return true;
}
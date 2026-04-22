// SvenInt (c) Sw1ft
// Credits go to somebody from some Half-Life forums, I dunno.
// Snark gave me this code - xWhitey
// I took this from xWhitey's Lightning - Sw1ft
// r_bobbing.cpp

#include "stdafx.h"
#include "r_bobbing.h"
#include "player_speedhack.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CBobbing, bobbing, "Render", "Bobbing" );

//-----------------------------------------------------------------------------
// RemapVal
//-----------------------------------------------------------------------------

float CBobbing::RemapVal( float val, float A, float B, float C, float D )
{
	return C + ( D - C ) * ( val - A ) / ( B - A );
}

//-----------------------------------------------------------------------------
// V_CalcNewBob
//-----------------------------------------------------------------------------

float CBobbing::V_CalcNewBob( struct ref_params_s *pparams )
{
	constexpr float PI = (float)M_PI;
	float flCycle;

	if ( realtime == NULL )
		return 0.f;

	if ( pparams->onground == -1 || *realtime == m_flLastBobTime )
		return 0.f;
	
	const float flGameSpeed = static_cast<float>( Features::speedhack->GetGameSpeed() / 1000.0 );
	float flSpeed = ( *playermove->velocity() ).Length2D() * flGameSpeed;

	if ( !m_pBobStrengthIsUncapped->GetBool() )
	{
		const int cap = m_pBobStrengthSpeedCap->GetInt();
		flSpeed = Q_clamp( flSpeed, -cap, cap );
	}

	float bob_offset = RemapVal( flSpeed, 0, 320, 0.f, 1.f );

	m_flBobTime += ( float( *realtime ) - m_flLastBobTime ) * bob_offset;
	m_flLastBobTime = (float)*realtime;

	flCycle = m_flBobTime - (float)(int)( m_flBobTime / HL2_BOB_CYCLE_MAX ) * HL2_BOB_CYCLE_MAX;
	flCycle /= HL2_BOB_CYCLE_MAX;

	if ( flCycle < HL2_BOB_UP )
	{
		flCycle = PI * flCycle / HL2_BOB_UP;
	}
	else
	{
		flCycle = PI + PI * ( flCycle - HL2_BOB_UP ) / ( 1.f - HL2_BOB_UP );
	}

	m_flVerticalBob = flSpeed * 0.004f;
	m_flVerticalBob = m_flVerticalBob * 0.3f + m_flVerticalBob * 0.7f * sinf( flCycle );

	m_flVerticalBob = Q_clamp( m_flVerticalBob, -7.f, 4.f );

	flCycle = m_flBobTime - (float)(int)( m_flBobTime / HL2_BOB_CYCLE_MAX * 2.f ) * HL2_BOB_CYCLE_MAX * 2.f;
	flCycle /= HL2_BOB_CYCLE_MAX * 2.f;

	if ( flCycle < HL2_BOB_UP )
	{
		flCycle = PI * flCycle / HL2_BOB_UP;
	}
	else
	{
		flCycle = PI + PI * ( flCycle - HL2_BOB_UP ) / ( 1.f - HL2_BOB_UP );
	}

	m_flLateralBob = flSpeed * 0.004f;
	m_flLateralBob = m_flLateralBob * 0.3f + m_flLateralBob * 0.7f * sinf( flCycle );

	m_flLateralBob = Q_clamp( m_flLateralBob, -7.f, 4.f );

	return 0.f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CBobbing::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// V_CalcRefdef post event
	auto pparams = pEvent->GetArg<ref_params_t *>( "pparams" );
	if ( pparams == NULL )
		return kHookContinue;

	Vector forward, right;
	cl_entity_t *view = cl_enginefuncs->GetViewModel();

	AngleVectors( view->angles, &forward, &right, NULL );

	V_CalcNewBob( pparams );

	VectorMA( view->origin, m_flVerticalBob * 0.1f, forward, view->origin );
	view->origin[ 2 ] += m_flVerticalBob * 0.1f;
	VectorMA( view->origin, m_flLateralBob * 0.8f, right, view->origin );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CBobbing::CBobbing( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pBobStrengthIsUncapped = NULL;
	m_pBobStrengthSpeedCap = NULL;

	m_flLateralBob = 1.f;
	m_flVerticalBob = 1.f;
	m_flBobTime = -1.f;
	m_flLastBobTime = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CBobbing::OnEnable( void )
{
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall, kHookPriorityHigh );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CBobbing::OnDisable( void )
{
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CBobbing::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pBobStrengthIsUncapped = Modules::menu->AddParamBool( this, "BobStrengthIsUncapped", NULL, false );
	m_pBobStrengthSpeedCap = Modules::menu->AddParamInteger( this, "BobStrengthSpeedCap", NULL, 320, 320, 4096 );

	return true;
}
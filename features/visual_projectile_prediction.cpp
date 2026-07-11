// SvenInt (c) Sw1ft
// visual_projectile_prediction.cpp

#include "stdafx.h"
#include "visual_projectile_prediction.h"
#include "r_draw_context.h"
#include "utils/util.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CProjectilePrediction, projprediction, "Visual", "Projectile Prediction" );

//-----------------------------------------------------------------------------
// Predict hand grenade trajectory
//-----------------------------------------------------------------------------

void CProjectilePrediction::PredictGrenadeTrajectory( void )
{
	const float flFrametime = 1.f / GameData::Cvars::fps_max->value;
	const float flEntGravity = 1.f;
	const float flFriction = 0.8f;
	const float flBoomTime = 4.f;
	const float flSpeed = 700.f;

	float flThrowStart, tillBoom;

	if ( localplayer->GetCurrentWeaponID() != WEAPON_HANDGRENADE )
		return;

	if ( ( flThrowStart = -clientweapon->GetWeaponData()->fuser1 ) == 0.f || flThrowStart >= flBoomTime )
		return;

	Vector va, forward, right;

	cl_enginefuncs->GetViewAngles( va );

	const int oldhull = playermove->usehull();
	Vector angThrow = va; // todo: va + punch angle

	if ( angThrow.x < 0.f )
		angThrow.x = -10.f + angThrow.x * ( ( 90.f - 10.f ) / 90.f );
	else
		angThrow.x = -10.f + angThrow.x * ( ( 90.f + 10.f ) / 90.f );

	float flVel = ( 90.f - angThrow.x ) * 10.f;

	if ( flVel > flSpeed )
		flVel = flSpeed;

	cl_enginefuncs->pfnAngleVectors( angThrow, forward, right, NULL );

	tillBoom = flBoomTime - flThrowStart;

	int iBounces = 0;
	float flTime = 0.f;
	float flNextThink = 0.f;
	float flBounceThink = 0.f;

	pmtrace_t trace;
	Vector vecOrigin = localplayer->GetEyePosition() + forward * 16;
	Vector vecVelocity = localplayer->GetVelocity() + forward * flVel;

	if ( tillBoom < 0.1 )
	{
		vecVelocity.Zero();
	}
	else
	{
		flNextThink = 0.1f;
	}

	CDrawTrajectory *pTrajectoryRenderer = new CDrawTrajectory( m_pGrenadeColor->GetColor32(),
																m_pGrenadeImpactColor->GetColor32() );

	playermove->setusehull( 2 );

	// Loop
	do
	{
		// Apply gravity
		vecVelocity.z -= flEntGravity * refparams_movevars->gravity * flFrametime;

		Vector vecMove = vecVelocity * flFrametime;

		// Trace forward
		pmtrace_t trace = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_WORLD_ONLY, -1 );

		pTrajectoryRenderer->AddLine( vecOrigin, trace.endpos );

		// Save trace pos
		vecOrigin = trace.endpos;

		// Tumble think
		if ( flTime >= flNextThink )
		{
			if ( trace.inwater )
			{
				vecVelocity = vecVelocity * 0.5f;
			}

			flNextThink = flTime + 0.1f;
		}

		// Did hit a wall
		if ( ( trace.fraction != 1.f && !trace.allsolid ) || trace.startsolid )
		{
			if ( flTime >= flBounceThink )
			{
				// On ground
				if ( trace.plane.normal[ 2 ] > 0.7f )
				{
					vecVelocity *= 0.9f;

					flBounceThink = flTime + 0.1f;
				}
			}

			iBounces++;

			pTrajectoryRenderer->AddImpact( vecOrigin );

			if ( iBounces >= 4 )
				break;

			// Started in solid
			if ( trace.startsolid )
				break;
		}

		if ( trace.fraction != 1.f )
		{
			UTIL_ClipVelocity( vecVelocity, trace.plane.normal, vecVelocity, 2.f - flFriction /* reflection factor */ );

			// On ground
			if ( trace.plane.normal[ 2 ] > 0.7f )
			{
				if ( vecVelocity.z < refparams_movevars->gravity * flFrametime )
					vecVelocity.z = 0.f;

				if ( vecVelocity.LengthSqr() < 100.f )
				{
					vecVelocity.Zero();
					break;
				}
			}

		}

		flTime += flFrametime;
	} while ( flTime <= tillBoom );

	Features::drawcontext->AddDrawContext( pTrajectoryRenderer );
	playermove->setusehull( oldhull );
}

//-----------------------------------------------------------------------------
// Predict AR grenade trajectory
//-----------------------------------------------------------------------------

void CProjectilePrediction::PredictARGrenadeTrajectory( void )
{
	const float flFrametime = 1.f / GameData::Cvars::fps_max->value;
	const float flEntGravity = 0.7f;

	int iWeaponID = localplayer->GetCurrentWeaponID();

	if ( iWeaponID != WEAPON_MP5 && iWeaponID != WEAPON_M16 )
		return;

	if ( clientweapon->SecondaryAmmo() == 0 )
	{
		if ( iWeaponID == WEAPON_M16 )
		{
			if ( clientweapon->GetWeaponData()->fuser1 == 8.f )
				return;
		}
		else
		{
			return;
		}
	}

	const int oldhull = playermove->usehull();
	float flSpeed = 900.f;

	if ( iWeaponID == WEAPON_M16 )
		flSpeed = 1000.f;

	int it = 0;
	Vector va, forward, right;

	cl_enginefuncs->GetViewAngles( va );
	cl_enginefuncs->pfnAngleVectors( va, forward, right, NULL );

	pmtrace_t trace;
	Vector vecOrigin;
	Vector vecVelocity = forward * flSpeed;

	if ( localplayer->IsDucked() )
	{
		vecOrigin = localplayer->GetOrigin() + forward * 16.f + right * 6.f;
	}
	else
	{
		vecOrigin = localplayer->GetOrigin() + localplayer->GetViewOffset() * 0.5f + forward * 16.f + right * 6.f;
	}

	playermove->setusehull( 2 );

	CDrawTrajectory *pTrajectoryRenderer = new CDrawTrajectory( m_pARGrenadeColor->GetColor32(), m_pARGrenadeImpactColor->GetColor32() );

	// Loop
	do
	{
		// Apply gravity
		vecVelocity.z -= flEntGravity * refparams_movevars->gravity * flFrametime;

		Vector vecMove = vecVelocity * flFrametime;

		// Trace forward
		pmtrace_t trace = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_WORLD_ONLY, -1 );

		pTrajectoryRenderer->AddLine( vecOrigin, trace.endpos );

		// Save trace pos
		vecOrigin = trace.endpos;

		// Did hit a wall or started in solid
		if ( ( trace.fraction != 1.f && !trace.allsolid ) || trace.startsolid )
		{
			pTrajectoryRenderer->AddImpact( vecOrigin );
			break;
		}

		it++;
	} while ( it < 3000 );

	Features::drawcontext->AddDrawContext( pTrajectoryRenderer );
	playermove->setusehull( oldhull );
}

//-----------------------------------------------------------------------------
// Predict spore's trajectory
//-----------------------------------------------------------------------------

void CProjectilePrediction::PredictSporeTrajectory( void )
{
	const float flFrametime = 1.f / GameData::Cvars::fps_max->value;
	const float flEntGravity = 0.8f;

	int iWeaponID = localplayer->GetCurrentWeaponID();

	if ( iWeaponID != WEAPON_SPORE_LAUNCHER )
		return;

	const int oldhull = playermove->usehull();
	float flSpeed = m_pSporeAttackMode->GetInt() == 0 ? 700.f : 1200.f;

	int it = 0;
	Vector va, forward, right, up;

	cl_enginefuncs->GetViewAngles( va );
	cl_enginefuncs->pfnAngleVectors( va, forward, right, up );

	pmtrace_t trace;
	Vector vecOrigin;
	Vector vecVelocity = forward * flSpeed;

	if ( localplayer->IsDucked() )
	{
		vecOrigin = localplayer->GetOrigin() + right * 8.f + up * 8.f;
	}
	else
	{
		vecOrigin = localplayer->GetOrigin() + localplayer->GetViewOffset() * 0.5f + right * 8.f + up * 10.f;
	}

	playermove->setusehull( 2 );

	CDrawTrajectory *pTrajectoryRenderer = new CDrawTrajectory( m_pSporeColor->GetColor32(), m_pSporeImpactColor->GetColor32() );

	// Loop
	do
	{
		// Apply gravity
		vecVelocity.z -= flEntGravity * refparams_movevars->gravity * flFrametime;

		Vector vecMove = vecVelocity * flFrametime;

		// Trace forward
		pmtrace_t trace = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_WORLD_ONLY, -1 );

		pTrajectoryRenderer->AddLine( vecOrigin, trace.endpos );

		// Save trace pos
		vecOrigin = trace.endpos;

		// Did hit a wall or started in solid
		if ( ( trace.fraction != 1.f && !trace.allsolid ) || trace.startsolid )
		{
			pTrajectoryRenderer->AddImpact( vecOrigin );
			break;
		}

		it++;
	} while ( it < 3000 );

	Features::drawcontext->AddDrawContext( pTrajectoryRenderer );
	playermove->setusehull( oldhull );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CProjectilePrediction::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// kV_CalcRefdef event
	if ( UTIL_IsDead() )
		return kHookContinue;

	if ( m_pPredictGrenade->GetBool() )
		PredictGrenadeTrajectory();

	if ( m_pPredictARGrenade->GetBool() )
		PredictARGrenadeTrajectory();
	
	if ( m_pPredictSpore->GetBool() )
		PredictSporeTrajectory();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CProjectilePrediction::CProjectilePrediction( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pPredictGrenade = NULL;
	m_pGrenadeColor = NULL;
	m_pGrenadeImpactColor = NULL;

	m_pPredictARGrenade = NULL;
	m_pARGrenadeColor = NULL;
	m_pARGrenadeImpactColor = NULL;

	m_pPredictSpore = NULL;
	m_pSporeAttackMode = NULL;
	m_pSporeColor = NULL;
	m_pSporeImpactColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CProjectilePrediction::OnEnable( void )
{
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CProjectilePrediction::OnDisable( void )
{
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CProjectilePrediction::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Modules::menu->AddElementSeparator( this, "Hand Grenade" );

	m_pPredictGrenade = Modules::menu->AddParamBool( this, "Grenade", NULL, false );
	m_pGrenadeColor = Modules::menu->AddParamColorRGBA( this, "GrenadeColor", NULL, Color( 0, 255, 0, 255 ) );
	m_pGrenadeImpactColor = Modules::menu->AddParamColorRGBA( this, "GrenadeImpactColor", NULL, Color( 255, 0, 0, 255 ) );

	Modules::menu->AddElementSeparator( this, "AR Grenade" );

	m_pPredictARGrenade = Modules::menu->AddParamBool( this, "ARGrenade", NULL, true );
	m_pARGrenadeColor = Modules::menu->AddParamColorRGBA( this, "ARGrenadeColor", NULL, Color( 0, 255, 0, 255 ) );
	m_pARGrenadeImpactColor = Modules::menu->AddParamColorRGBA( this, "ARGrenadeImpactColor", NULL, Color( 255, 0, 0, 255 ) );
	
	Modules::menu->AddElementSeparator( this, "Spore Launcher" );

	m_pPredictSpore = Modules::menu->AddParamBool( this, "Spore", NULL, true );
	m_pSporeAttackMode = Modules::menu->AddParamList( this, "SporeAttackMode", NULL, 0, " 0 - Primary\0 1 - Secondary\0\0" );
	m_pSporeColor = Modules::menu->AddParamColorRGBA( this, "SporeColor", NULL, Color( 0, 255, 0, 255 ) );
	m_pSporeImpactColor = Modules::menu->AddParamColorRGBA( this, "SporeImpactColor", NULL, Color( 255, 0, 0, 255 ) );

	return true;
}
// SvenInt (c) Sw1ft
// st_landing_prediction.cpp

#include "stdafx.h"
#include "st_landing_prediction.h"
#include "r_draw_context.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CLandingPrediction, landprediction, "Speedrun Tools", "Landing Prediction" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CLandingPrediction::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( UTIL_IsDead() )
		return kHookContinue;

	const int oldhull = playermove->usehull();
	const float flFrametime = 1.f / GameData::Cvars::fps_max->value;

	pmtrace_t tr;
	Vector vecOrigin, vecVelocity;
	bool bDucking, bOnGround;

	int it = 0;
	int landings = 0;

	if ( demoplayback )
	{
		vecVelocity = refparams->simvel;
		vecOrigin = refparams->simorg;

		bDucking = ( refparams->viewheight[ 2 ] == VEC_DUCK_VIEW.z );
		bOnGround = false;

		// Trace forward
		tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + ( vecVelocity * flFrametime ), PM_NORMAL, -1 );

		// Did hit a wall or started in solid
		if ( tr.fraction != 1.f && !tr.allsolid && tr.plane.normal.z >= 0.7f )
		{
			bOnGround = true;
		}
		else
		{
			Vector point = vecOrigin;
			point.z -= 2.f;

			// Trace down
			tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, point, PM_NORMAL, -1 );

			if ( tr.plane.normal.z >= 0.7f )
			{
				bOnGround = true;
			}
		}
	}
	else
	{
		vecVelocity = localplayer->GetVelocity();
		vecOrigin = localplayer->GetOrigin();

		bDucking = ( localplayer->GetFlags() & FL_DUCKING );
		bOnGround = localplayer->IsOnGround();
	}

	// PM_Jump
	if ( bOnGround )
	{
		vecVelocity.z += sqrtf( 2.f * 800.f * 45.f );

		if ( demoplayback )
			UTIL_FixupGravityVelocity( vecVelocity,
									   refparams_movevars->gravity,
									   refparams_movevars->entgravity,
									   flFrametime );
		else
			UTIL_FixupGravityVelocity( vecVelocity, flFrametime );
	}

	// Set trace hull
	playermove->setusehull( bDucking ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );

	// Loop
	do
	{
		// Apply gravity
		if ( demoplayback )
			UTIL_AddCorrectGravity( vecVelocity,
									refparams_movevars->gravity,
									refparams_movevars->entgravity,
									flFrametime );
		else
			UTIL_AddCorrectGravity( vecVelocity, flFrametime );

		Vector vecMove = vecVelocity * flFrametime;

		// Trace forward
		tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, vecOrigin + vecMove, PM_NORMAL, -1 );

		// Save trace pos
		vecOrigin = tr.endpos;

		// Did hit a wall or started in solid
		if ( ( tr.fraction != 1.f && !tr.allsolid ) || tr.startsolid )
		{
			if ( m_pLandingPoint->GetBool() )
			{
				float flHeightShift = ( bDucking ? VEC_DUCK_HULL_MIN.z : VEC_HULL_MIN.z );

				DrawBox( vecOrigin + ( Vector( 0.f, 0.f, flHeightShift ) ),
						 Vector( -2, -2, 0 ),
						 Vector( 2, 2, 4 ),
						 m_pLandingPointColor->GetColor()[ 0 ],
						 m_pLandingPointColor->GetColor()[ 1 ],
						 m_pLandingPointColor->GetColor()[ 2 ],
						 m_pLandingPointColor->GetColor()[ 3 ],
						 0.f,
						 false );
			}

			if ( m_pLandingDrawHull->GetBool() )
			{
				DrawBox( vecOrigin,
						 bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN,
						 bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX,
						 m_pLandingHullColor->GetColor()[ 0 ],
						 m_pLandingHullColor->GetColor()[ 1 ],
						 m_pLandingHullColor->GetColor()[ 2 ],
						 m_pLandingHullColor->GetColor()[ 3 ],
						 m_pLandingHullWidth->GetFloat(),
						 m_pLandingDrawHullWireframe->GetBool() );
			}

			landings++;

			if ( landings >= m_pLandingMaxPoints->GetInt() || vecVelocity.Length2DSqr() == 0.f )
				break;

			bool bPredictedOnGround = false;
			Vector vecWallNormal = tr.plane.normal;

			// Did hit a wall or started in solid
			if ( tr.fraction != 1.f && !tr.allsolid && tr.plane.normal.z >= 0.7f )
			{
				bPredictedOnGround = true;
			}
			else
			{
				Vector point = vecOrigin;
				point.z -= 2.f;

				// Trace down
				tr = playermove->funcs()->PM_PlayerTrace( vecOrigin, point, PM_NORMAL, -1 );

				if ( tr.plane.normal.z >= 0.7f )
				{
					bPredictedOnGround = true;
				}
			}

			if ( bPredictedOnGround )
			{
				// PM_Jump
				vecVelocity.z = sqrtf( 2.f * 800.f * 45.f );

				if ( demoplayback )
					UTIL_FixupGravityVelocity( vecVelocity,
											   refparams_movevars->gravity,
											   refparams_movevars->entgravity,
											   flFrametime );
				else
					UTIL_FixupGravityVelocity( vecVelocity, flFrametime );

				continue;
			}
			else
			{
				UTIL_ClipVelocity( vecVelocity, vecWallNormal, vecVelocity, 1.f );
			}
		}

		if ( demoplayback )
			UTIL_FixupGravityVelocity( vecVelocity,
									   refparams_movevars->gravity,
									   refparams_movevars->entgravity,
									   flFrametime );
		else
			UTIL_FixupGravityVelocity( vecVelocity, flFrametime );

		it++;

	} while ( it < 3000 );

	playermove->setusehull( oldhull );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CLandingPrediction::CLandingPrediction( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pLandingDrawHull = NULL;
	m_pLandingDrawHullWireframe = NULL;
	m_pLandingPoint = NULL;
	m_pLandingMaxPoints = NULL;
	m_pLandingHullWidth = NULL;
	m_pLandingHullColor = NULL;
	m_pLandingPointColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CLandingPrediction::OnEnable( void )
{
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CLandingPrediction::OnDisable( void )
{
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CLandingPrediction::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pLandingDrawHull = Modules::menu->AddParamBool( this, "LandingDrawHull", NULL, true );
	m_pLandingDrawHullWireframe = Modules::menu->AddParamBool( this, "LandingDrawHullWireframe", NULL, true );
	m_pLandingPoint = Modules::menu->AddParamBool( this, "LandingPoint", NULL, true );
	m_pLandingMaxPoints = Modules::menu->AddParamInteger( this, "LandingMaxPoints", NULL, 2, 1, 10 );
	m_pLandingHullWidth = Modules::menu->AddParamFloat( this, "LandingHullWidth", NULL, 4.f, 1.f, 10.f );
	m_pLandingHullColor = Modules::menu->AddParamColorRGBA( this, "LandingHullColor", NULL, Color( 255, 255, 255, 127 ) );
	m_pLandingPointColor = Modules::menu->AddParamColorRGBA( this, "LandingPointColor", NULL, Color( 255, 255, 255, 127 ) );


	return true;
}
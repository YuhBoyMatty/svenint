// SvenInt (c) Sw1ft
// st_revive_boost_info.cpp

#include "stdafx.h"
#include "st_revive_boost_info.h"
#include "st_revive_info.h"
#include "st_player_hulls.h"
#include "st_hud.h"
#include "r_drawing.h"
#include "r_draw_context.h"
#include "misc_entity_list.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CReviveBoostInfo, reviveboostinfo, "Speedrun Tools", "Revive Boost Info" );

//-----------------------------------------------------------------------------
// Reset revive info
//-----------------------------------------------------------------------------

void CReviveBoostInfo::Reset( void )
{
	m_bShowReviveBoostInfo = false;
	m_pReviveBoostTarget = NULL;
	m_flReviveBoostDistance = -1.f;
	m_flReviveBoostAngle = 0.f;
}

//-----------------------------------------------------------------------------
// Draw revive boost info
//-----------------------------------------------------------------------------

void CReviveBoostInfo::DrawReviveBoostInfo( void )
{
	if ( UTIL_IsDead() || !( localplayer->GetCurrentWeaponID() == WEAPON_MEDKIT || m_pShowWithAnyWeapon->GetBool() ) )
		return;

	const Vector vecReviveHullMins( -16, -16, 0 );
	const Vector vecReviveHullMaxs( 16, 16, 72 );

	Vector vecRevivableTargetCenter;
	float flMinIntersection, flMaxIntersection;

	bool bDucking = ( demoplayback ? refparams->viewheight[ 2 ] == VEC_DUCK_VIEW.z : playermove->flags() & FL_DUCKING );

	CEntity *pEnts = Features::entitylist->GetList();

	Vector vecCenter = ( demoplayback ? refparams->simorg : *playermove->origin() );

	Vector vecMins = vecCenter + ( bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN );
	Vector vecMaxs = vecCenter + ( bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX );

	cl_entity_t *pTarget = NULL;

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		// Ignore invisible corpses of players
		if ( ent.m_pEntity->curstate.effects & EF_NODRAW )
			continue;

		// The only revivable targets are players in DEAD state, grenades and corpses of players
		if ( !( ent.m_bPlayer && !ent.m_bAlive ) &&
			 !( ent.m_classInfo.id == CLASS_ITEM_GRENADE && ( ent.m_bNeutral || ent.m_bEnemy ) ) &&
			 ent.m_classInfo.id != CLASS_DEAD_PLAYER )
			continue;

		vecRevivableTargetCenter = ent.m_vecOrigin;

		Vector vecRevivableTargetMins = vecRevivableTargetCenter + vecReviveHullMins;
		Vector vecRevivableTargetMaxs = vecRevivableTargetCenter + vecReviveHullMaxs;

		if ( pTarget == NULL && UTIL_IsAABBIntersectingAABB( vecMins, vecMaxs, vecRevivableTargetMins, vecRevivableTargetMaxs ) )
		{
			Vector vecDir = ( vecCenter - vecRevivableTargetCenter ).Normalize();

			if ( UTIL_IsRayIntersectingAABB( vecMins, vecMaxs, vecRevivableTargetCenter, vecDir, &flMinIntersection, &flMaxIntersection ) )
			{
				float flDistance, flDistanceToEdge;

				// Condition ( flMinIntersection < 0.f ) is met if ray has started inside AABB
				flDistance = ( flMinIntersection < 0.f ? flMaxIntersection : flMinIntersection );

				UTIL_IsRayIntersectingAABB( vecRevivableTargetMins, vecRevivableTargetMaxs, vecRevivableTargetCenter, vecDir, NULL, &flMaxIntersection );

				// Since everytime ray starts inside AABB of revivable target, we need only max distance of intersection since ( flMinIntersection < 0.f ) always true
				flDistanceToEdge = flMaxIntersection;

				// Draw boost direction
				if ( m_pDirectionType->GetInt() == 0 ) // box
				{
					float flBeamExtent = m_pDirectionBoxExtent->GetFloat();
					float flBeamLength = m_pDirectionLength->GetFloat();

					Vector vecBeamMins( 0.f, -flBeamExtent / 2, -flBeamExtent / 2 );
					Vector vecBeamMaxs( flBeamLength, flBeamExtent / 2, flBeamExtent / 2 );

					Vector vecAngles;

					VectorAngles( vecDir, vecAngles );
					vecAngles.x *= -1.f; // omg it's inverted, need to fix SDK?

					DrawBoxAngles( vecRevivableTargetCenter,
								   vecBeamMins,
								   vecBeamMaxs,
								   vecAngles,
								   m_pDirectionColor->GetColor()[ 0 ],
								   m_pDirectionColor->GetColor()[ 1 ],
								   m_pDirectionColor->GetColor()[ 2 ],
								   m_pDirectionColor->GetColor()[ 3 ],
								   m_pDirectionLineWidth->GetFloat(),
								   m_pWireframeDirectionBox->GetBool() );
				}
				else // line
				{
					Features::drawcontext->DrawLine( vecRevivableTargetCenter,
													 vecRevivableTargetCenter + vecDir * m_pDirectionLength->GetFloat(),
													 m_pDirectionColor->GetColor()[ 0 ],
													 m_pDirectionColor->GetColor()[ 1 ],
													 m_pDirectionColor->GetColor()[ 2 ],
													 m_pDirectionColor->GetColor()[ 3 ],
													 m_pDirectionLineWidth->GetFloat() );
				}

				// Predict boost
				if ( m_pTrajectory->GetBool() || m_pCollision->GetBool() )
				{
					DrawPredictedReviveBoost();
				}

				m_pReviveBoostTarget = pTarget = ent.m_pEntity;
				// Distance from center of AABB of revivable target minus distance of local player's AABB
				m_flReviveBoostDistance = ( flDistanceToEdge >= flDistance ? ( flDistanceToEdge - flDistance ) : ( flDistance - flDistanceToEdge ) );
				m_flReviveBoostAngle = vecDir.z;
			}
		}

		DrawBox( vecRevivableTargetCenter,
				 vecReviveHullMins,
				 vecReviveHullMaxs,
				 m_pHullColor->GetColor()[ 0 ],
				 m_pHullColor->GetColor()[ 1 ],
				 m_pHullColor->GetColor()[ 2 ],
				 m_pHullColor->GetColor()[ 3 ],
				 m_pWireframeHullWidth->GetFloat(),
				 m_pWireframeHull->GetBool() );
	}

	if ( pTarget == NULL )
	{
		m_pReviveBoostTarget = NULL;
		m_flReviveBoostDistance = -1.f;
	}

	m_bShowReviveBoostInfo = true;
}

//-----------------------------------------------------------------------------
// Predict revive boost
//-----------------------------------------------------------------------------

void CReviveBoostInfo::DrawPredictedReviveBoost( void )
{
	if ( !demoplayback && playermove->movevars() == NULL )
		return;

	const int oldhull = playermove->usehull();
	const float flFrametime = 1.f / GameData::Cvars::fps_max->value;

	pmtrace_t tr;
	bool bDucking, bOnGround;
	Vector vecOrigin, vecVelocity;
	CDrawTrajectory *pTrajectoryRenderer = NULL;

	int it = 0;

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
		if ( Modules::server->Host_IsServerActive() )
		{
			edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );

			if ( pPlayer == NULL )
				return;

			vecVelocity = pPlayer->v.velocity;
		}
		else
		{
			vecVelocity = *playermove->velocity();
		}

		vecOrigin = *playermove->origin();

		bDucking = ( playermove->flags() & FL_DUCKING );
		bOnGround = ( playermove->onground() != -1 );
	}

	// Set trace hull
	playermove->setusehull( bDucking ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );

	if ( m_pTrajectory->GetBool() )
	{
		Color clr = m_pTrajectoryColor->GetColor32();
		pTrajectoryRenderer = new CDrawTrajectory( clr, clr );
	}

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

		if ( m_pTrajectory->GetBool() )
			pTrajectoryRenderer->AddLine( vecOrigin, tr.endpos );

		// Save trace pos
		vecOrigin = tr.endpos;

		// Did hit a wall or started in solid
		if ( ( tr.fraction != 1.f && !tr.allsolid ) || tr.startsolid )
		{
			if ( m_pCollision->GetBool() )
			{
				DrawBox( vecOrigin,
						 bDucking ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN,
						 bDucking ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX,
						 m_pCollisionColor->GetColor()[ 0 ],
						 m_pCollisionColor->GetColor()[ 1 ],
						 m_pCollisionColor->GetColor()[ 2 ],
						 m_pCollisionColor->GetColor()[ 3 ],
						 m_pCollisionWidth->GetFloat(),
						 true );
			}

			break;
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

	if ( m_pTrajectory->GetBool() )
		Features::drawcontext->AddDrawContext( pTrajectoryRenderer );

	playermove->setusehull( oldhull );
}

//-----------------------------------------------------------------------------
// Draw revive boost info
//-----------------------------------------------------------------------------

void CReviveBoostInfo::ShowReviveBoostInfo( int r, int g, int b )
{
	if ( !m_bShowReviveBoostInfo )
		return;

	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() );

	vgui::HFont hEngineFont = Features::sthud->GetEngineFont();

	if ( m_pReviveBoostTarget != NULL )
	{
		if ( m_pReviveBoostTarget->player )
		{
			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( m_pReviveBoostTarget->index - 1 );

			Features::drawing->DrawStringExF( hEngineFont,
											  x, y, r, g, b, 255,
											  width, height,
											  FONT_ALIGN_LEFT,
											  "Revive Boost Target: %s (%d)",
											  pPlayerInfo ? pPlayerInfo->name : "N/A", m_pReviveBoostTarget->index );
		}
		else
		{
			Features::drawing->DrawStringExF( hEngineFont,
											  x, y, r, g, b, 255,
											  width, height,
											  FONT_ALIGN_LEFT,
											  "Revive Boost Target: %s (%d)",
											  m_pReviveBoostTarget->model ? m_pReviveBoostTarget->model->name : "DEADPLAYER", m_pReviveBoostTarget->index );
		}
	}
	else
	{
		Features::drawing->DrawStringEx( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Revive Boost Target: N/A" );
	}

	y += height;

	if ( m_flReviveBoostDistance >= 0.f )
	{
		float flVerticalEfficiency = fabsf( 100.f * m_flReviveBoostAngle );
		float flHorizontalEfficiency = 100.f - flVerticalEfficiency;

		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Distance: %.2f", m_flReviveBoostDistance );

		y += height;

		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Vertical Boost Efficiency: %.2f %%", flVerticalEfficiency );

		y += height;

		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Horizontal Boost Efficiency: %.2f %%", flHorizontalEfficiency );

		y += height;

		if ( Modules::server->Host_IsServerActive() )
		{
			edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );

			if ( pPlayer == NULL )
				return;

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Vertical Speed: %.2f", fabs( pPlayer->v.velocity.z ) );

			y += height;

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Horizontal Speed: %.2f", pPlayer->v.velocity.Length2D() );

			y += height;

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Overall Speed: %.2f", pPlayer->v.velocity.Length() );
		}
		else
		{
			Vector vecVelocity = ( demoplayback ? refparams->simvel : *playermove->velocity() );

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Vertical Speed: %.2f", fabs( vecVelocity.z ) );

			y += height;

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Horizontal Speed: %.2f", vecVelocity.Length2D() );

			y += height;

			Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Overall Speed: %.2f", vecVelocity.Length() );
		}
	}
	else
	{
		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Distance: N/A" );
	}

	m_bShowReviveBoostInfo = false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CReviveBoostInfo::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		DrawReviveBoostInfo();
		return kHookContinue;
	}

	Color clrHud = Features::sthud->GetColor();
	ShowReviveBoostInfo( clrHud.r, clrHud.g, clrHud.b );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CReviveBoostInfo::CReviveBoostInfo( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pShowWithAnyWeapon = NULL;
	m_pWireframeHull = NULL;
	m_pWireframeDirectionBox = NULL;

	m_pDirectionType = NULL;
	m_pDirectionLength = NULL;
	m_pDirectionBoxExtent = NULL;
	m_pDirectionLineWidth = NULL;
	m_pWireframeHullWidth = NULL;

	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;

	m_pHullColor = NULL;
	m_pDirectionColor = NULL;

	m_pTrajectory = NULL;
	m_pCollision = NULL;
	m_pCollisionWidth = NULL;
	m_pTrajectoryColor = NULL;
	m_pCollisionColor = NULL;

	Reset();
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CReviveBoostInfo::OnEnable( void )
{
	Reset();

	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CReviveBoostInfo::OnDisable( void )
{
	Reset();

	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CReviveBoostInfo::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShowWithAnyWeapon = Modules::menu->AddParamBool( this, "ShowWithAnyWeapon", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pWireframeHull = Modules::menu->AddParamBool( this, "WireframeHull", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pWireframeDirectionBox = Modules::menu->AddParamBool( this, "WireframeDirectionBox", NULL, false );

	Modules::menu->AddElementSeparator( this, "Direction Visualization" );

	m_pDirectionType = Modules::menu->AddParamList( this, "DirectionType", NULL, 0, " 0 - Box\0 1 - Line\0\0" );
	m_pDirectionLength = Modules::menu->AddParamFloat( this, "DirectionLength", NULL, 2048.f, 1.f, 4096.f );
	m_pDirectionBoxExtent = Modules::menu->AddParamFloat( this, "DirectionBoxExtent", NULL, 2.f, 0.1f, 10.f );
	m_pDirectionLineWidth = Modules::menu->AddParamFloat( this, "DirectionLineWidth", NULL, 4.f, 0.1f, 10.f );
	m_pWireframeHullWidth = Modules::menu->AddParamFloat( this, "WireframeHullWidth", NULL, 4.f, 0.1f, 10.f );

	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.009f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.31f, 0.f, 1.f );

	m_pHullColor = Modules::menu->AddParamColorRGBA( this, "HullColor", NULL, Color( 0.f, 1.f, 0.f, 0.5f ) );
	m_pDirectionColor = Modules::menu->AddParamColorRGBA( this, "DirectionColor", NULL, Color( 1.f, 0.f, 0.f, 0.5f ) );

	Modules::menu->AddElementSeparator( this, "Prediction" );

	m_pTrajectory = Modules::menu->AddParamBool( this, "Trajectory", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pCollision = Modules::menu->AddParamBool( this, "Collision", NULL, true );
	m_pCollisionWidth = Modules::menu->AddParamFloat( this, "CollisionWidth", NULL, 4.f, 0.1f, 10.f );
	m_pTrajectoryColor = Modules::menu->AddParamColorRGBA( this, "TrajectoryColor", NULL, Color( 1.f, 1.f, 0.f, 1.f ) );
	m_pCollisionColor = Modules::menu->AddParamColorRGBA( this, "CollisionColor", NULL, Color( 0.f, 1.f, 1.f, 0.5f ) );

	return true;
}
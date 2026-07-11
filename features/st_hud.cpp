// SvenInt (c) Sw1ft
// st_hud.cpp

#include "stdafx.h"
#include "st_hud.h"
#include "r_drawing.h"
#include "misc_entity_list.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare func pointer
//-----------------------------------------------------------------------------

DECLARE_FUNC_PTR( vgui::HFont, CALLCONV_CDECL, VGUI2_GetEngineFont );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpeedrunHUD, sthud, "Speedrun Tools", "HUD" );

//-----------------------------------------------------------------------------
// GetEngineFont
//-----------------------------------------------------------------------------

vgui::HFont CSpeedrunHUD::GetEngineFont( void )
{
	if ( VGUI2_GetEngineFont == NULL )
		return Features::drawing->GetFontESP();

	return VGUI2_GetEngineFont();
}

//-----------------------------------------------------------------------------
// GetColor
//-----------------------------------------------------------------------------

Color CSpeedrunHUD::GetColor( void )
{
	return m_pColor->GetColor32();
}

//-----------------------------------------------------------------------------
// Draw view angles
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowViewangles( int r, int g, int b )
{
	if ( !m_pShowViewAngles->GetBool() )
		return;
	
	Vector va;
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pViewAnglesWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pViewAnglesHeightScreenFraction->GetFloat() );

	if ( demoplayback )
		va = refparams->viewangles;
	else
		cl_enginefuncs->GetViewAngles( va );

	NormalizeAngles( va );

	Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "View angles:" );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Pitch: %.6f", va.x );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Yaw: %.6f", va.y );
}

//-----------------------------------------------------------------------------
// Draw position
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowPosition( int r, int g, int b )
{
	if ( !m_pShowPosition->GetBool() )
		return;
	
	Vector origin;
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pPositionWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pPositionHeightScreenFraction->GetFloat() );

	if ( demoplayback )
		origin = refparams->simorg;
	else
		origin = *playermove->origin();

	if ( m_pShowPositionViewOrigin->GetBool() )
	{
		if ( demoplayback )
			origin += refparams->viewheight;
		else
			origin += *playermove->view_ofs();
	}

	Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Origin:" );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X: %.6f", origin.x );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y: %.6f", origin.y );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Z: %.6f", origin.z );
}

//-----------------------------------------------------------------------------
// Draw velocity
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowVelocity( int r, int g, int b )
{
	if ( !m_pShowVelocity->GetBool() )
		return;
	
	Vector velocity;
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pVelocityWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pVelocityHeightScreenFraction->GetFloat() );

	if ( demoplayback )
		velocity = refparams->simvel;
	else
		velocity = *playermove->velocity();

	Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Velocity:" );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X: %.6f", velocity.x );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y: %.6f", velocity.y );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Z: %.6f", velocity.z );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "XY: %.6f", velocity.Length2D() );

	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "XYZ: %.6f", velocity.Length() );
}

//-----------------------------------------------------------------------------
// Draw gauss info
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowGaussBoostInfo( int r, int g, int b )
{
	static cvar_t *sk_plr_secondarygauss = NULL;
	const float flGaussFullChargeTime = 3.f;

	if ( !m_pShowGaussBoostInfo->GetBool() || localplayer->GetCurrentWeaponID() != WEAPON_GAUSS )
		return;
	
	if ( sk_plr_secondarygauss == NULL )
	{
		sk_plr_secondarygauss = cvar->FindCvar( "sk_plr_secondarygauss" );

		if ( sk_plr_secondarygauss == NULL )
			return;
	}

	float flYaw;
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pGaussBoostInfoWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pGaussBoostInfoHeightScreenFraction->GetFloat() );

	Vector velocity = ( demoplayback ? refparams->simvel : *playermove->velocity() );

	Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Gauss boost info:" );

	y += height;

	if ( velocity.Length2DSqr() > 0.f )
	{
		flYaw = atan2f( velocity.y, velocity.x ) * 180.f / static_cast<float>( M_PI );

		if ( flYaw > 180.f )
			flYaw -= 360.f;
		else if ( flYaw < -180.f )
			flYaw += 360.f;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Boost with optimal yaw: %.6f", flYaw );
	}
	else
	{
		flYaw = ( demoplayback ? refparams->viewangles[ 1 ] : playermove->angles()->y );

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Boost with optimal yaw: %.6f", flYaw );
	}

	y += height;

	weapon_data_t *pWeaponData = ClientWeapon()->GetWeaponData();

	int m_fInAttack = int( pWeaponData->fuser4 );
	float m_flStartChargeTime = fabsf( pWeaponData->fuser2 );

	if ( m_fInAttack > 0 )
	{
		float flDamage;
		float flBoost;
		float flAmmoConsumed;

		float flSecondaryGaussDamage = sk_plr_secondarygauss->value;

		if ( flSecondaryGaussDamage <= 0.f )
			flSecondaryGaussDamage = 190.f; // default sven cope value

		if ( m_fInAttack == 1 )
		{
			flDamage = flSecondaryGaussDamage * ( 0.5f / flGaussFullChargeTime );

			flAmmoConsumed = 1.f;
		}
		else
		{
			if ( m_flStartChargeTime > flGaussFullChargeTime )
			{
				flDamage = flSecondaryGaussDamage;
			}
			else
			{
				flDamage = flSecondaryGaussDamage * ( m_flStartChargeTime / flGaussFullChargeTime );
			}

			flAmmoConsumed = ( m_flStartChargeTime - 0.5f ) / 0.3f;
			flAmmoConsumed += 1.f;
		}

		flBoost = flDamage * 5;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Optimal resulting speed [back boost]: %.6f", velocity.Length2D() + flBoost );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Overdose: %.6f", 10.f - m_flStartChargeTime );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Damage: %.6f", flDamage );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Boost: %.6f", flBoost );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Ammo consumed: %.2f", flAmmoConsumed );
	}
	else
	{
		Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Optimal resulting speed [back boost]: N/A" );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Overdose: %.6f", 0.f );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Damage: %.6f", 0.f );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Boost: %.6f", 0.f );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Ammo consumed: %.2f", 0.f );
	}
}

//-----------------------------------------------------------------------------
// Draw selfgauss info
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowSelfgaussInfo( int r, int g, int b )
{
	if ( !m_pShowSelfGaussInfo->GetBool() || !Modules::server->Host_IsServerActive() || localplayer->GetCurrentWeaponID() != WEAPON_GAUSS )
		return;
	
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pSelfGaussInfoWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pSelfGaussInfoHeightScreenFraction->GetFloat() );

	TraceResult tr;
	Vector va, forward, right, up, start, end;

	int iHitGroup;
	float flThreshold;

	bool bSelfgaussable = false;

	start = localplayer->GetEyePosition();

	cl_enginefuncs->GetViewAngles( va );
	cl_enginefuncs->pfnAngleVectors( va, forward, right, up );

	end = start + forward * 8192.f;

	edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );

	if ( pPlayer == NULL )
		return;

	sv_enginefuncs->pfnTraceLine( start, end, 0, pPlayer, &tr );

	if ( tr.pHit != NULL && tr.pHit->pvPrivateData != NULL && tr.pHit->v.solid == SOLID_BSP && !tr.pHit->v.takedamage )
	{
		float theta = -DotProduct( forward, tr.vecPlaneNormal );

		if ( theta >= 0.5f )
		{
			TraceResult beamTr;

			sv_enginefuncs->pfnTraceLine( tr.vecEndPos + forward * 8, end, 0, NULL, &beamTr );

			if ( !beamTr.fAllSolid )
			{
				Vector vecBeamEndPos = beamTr.vecEndPos;

				sv_enginefuncs->pfnTraceLine( vecBeamEndPos, tr.vecEndPos, 0, NULL, &beamTr );
				sv_enginefuncs->pfnTraceLine( start, end, 0, NULL, &tr );

				bSelfgaussable = true;

				flThreshold = ( beamTr.vecEndPos - tr.vecEndPos ).Length();
				iHitGroup = tr.iHitgroup;
			}
		}
	}

	if ( bSelfgaussable )
	{
		const char *HITGROUP_STRING[] =
		{
			"Generic",
			"Head",
			"Chest",
			"Stomach",
			"Left Arm",
			"Right Arm",
			"Left Leg",
			"Right Leg"
		};

		Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Selfgauss:" );

		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Threshold: %.6f", flThreshold );

		y += height;

		if ( iHitGroup < 0 || iHitGroup >= Q_ARRAYSIZE( HITGROUP_STRING ) )
		{
			iHitGroup = 0;
		}

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Hit Group: %s", HITGROUP_STRING[ iHitGroup ] );
	}
	else
	{
		Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Cannot selfgauss" );
	}
}

//-----------------------------------------------------------------------------
// Draw entity info
//-----------------------------------------------------------------------------

void CSpeedrunHUD::ShowEntityInfo( int r, int g, int b )
{
	if ( !m_pShowEntityInfo->GetBool() )
		return;
	
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pEntityInfoWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pEntityInfoHeightScreenFraction->GetFloat() );

	Vector va, forward, start, end;

	start = ( demoplayback ? *(Vector *)refparams->simorg + refparams->viewheight : localplayer->GetEyePosition() );

	cl_enginefuncs->GetViewAngles( va );
	cl_enginefuncs->pfnAngleVectors( va, forward, NULL, NULL );

	end = start + forward * 8192.f;

	if ( Modules::server->Host_IsServerActive() )
	{
		TraceResult tr;
		edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );

		if ( pPlayer == NULL )
			return;

		sv_enginefuncs->pfnTraceLine( start, end, 0, pPlayer, &tr );

		if ( tr.pHit == NULL )
		{
			Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Entity: N/A" );
			return;
		}
	
		edict_t *pEntity = tr.pHit;
		int ent = sv_enginefuncs->pfnIndexOfEdict( pEntity );
		bool bPlayer = ( ent >= 1 && ent <= cl_enginefuncs->GetMaxClients() );

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Entity: %d", ent );
		y += height;

		if ( bPlayer )
		{
			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( ent - 1 );

			if ( pPlayerInfo != NULL )
			{
				Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, pPlayerInfo->name );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "HP: %.6f", pEntity->v.health );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Armor: %.6f", pEntity->v.armorvalue );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Model: %s", pPlayerInfo->model );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Top Color: %d", pPlayerInfo->topcolor );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Bottom Color: %d", pPlayerInfo->bottomcolor );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Steam64ID: %llu", pPlayerInfo->m_nSteamID );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Pitch: %.6f", pEntity->v.v_angle.x );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Yaw: %.6f", pEntity->v.v_angle.y );
				y += height;
			}
		}
		else if ( pEntity->v.classname != 0 )
		{
			const char *pszClassname = gpGlobals->pStringBase + pEntity->v.classname;

			Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, pszClassname );
			y += height;

			if ( pEntity->v.target != 0 )
			{
				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Target: %s", gpGlobals->pStringBase + pEntity->v.target );
				y += height;
			}

			if ( pEntity->v.targetname != 0 )
			{
				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Name: %s", gpGlobals->pStringBase + pEntity->v.targetname );
				y += height;
			}

			const char *pszModelName = ( ent == 0 ? "N/A" : ( pEntity->v.model ? gpGlobals->pStringBase + pEntity->v.model : "N/A" ) );

			Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Model: %s", pszModelName );
			y += height;

			if ( strstr( pszClassname, "func_door" ) )
			{
				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Usable: %s", pEntity->v.spawnflags & 256 ? "Yes" : "No" );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Monsters: %s open", pEntity->v.spawnflags & 512 ? "Can't" : "Can" );
				y += height;
			}

			if ( strstr( pszClassname, "func_door" ) || !strncmp( pszClassname, "func_rotating", 13 ) || !strncmp( pszClassname, "func_train", 10 ) )
			{
				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Damage: %.6f", pEntity->v.dmg );
				y += height;
			}

			Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "HP: %.6f", pEntity->v.health );
			y += height;

			Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Yaw: %.6f", pEntity->v.angles.y );
			y += height;
		}

		Vector origin;

		if ( pEntity->v.solid == SOLID_BSP || pEntity->v.movetype == MOVETYPE_PUSHSTEP )
			origin = pEntity->v.origin + ( ( pEntity->v.mins + pEntity->v.maxs ) / 2.f );
		else
			origin = pEntity->v.origin;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X: %.6f", origin.x );
		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y: %.6f", origin.y );
		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Z: %.6f", origin.z );
		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X Vel: %.6f", pEntity->v.velocity.x );
		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y Vel: %.6f", pEntity->v.velocity.y );
		y += height;

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, ( "Z Vel: %.6f" ), pEntity->v.velocity.z );

		return;
	}

	pmtrace_t tr;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( start, end, PM_NORMAL, -1, &tr );

	int ent = cl_enginefuncs->pEventAPI->EV_IndexFromTrace( &tr );

	if ( tr.fraction == 1.f )
	{
		Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Entity: N/A" );
		return;
	}
		
	if ( m_pShowEntityInfoCheckPlayers->GetBool() )
	{
		struct eligible_player_t
		{
			float dist_sqr;
			int index;
		};

		std::vector<eligible_player_t> eligible_players;

		Vector traceEnd = tr.endpos;
		CEntity *pEnts = Features::entitylist->GetList();
		int iLocalPlayer = UTIL_GetLocalPlayerIndex();

		for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
		{
			if ( i == iLocalPlayer )
				continue;

			CEntity &ent = pEnts[ i ];

			if ( !ent.m_bValid || !ent.m_bVisible || !ent.m_bAlive )
				continue;

			Vector vecMins, vecMaxs;

			vecMins = vecMaxs = ent.m_vecOrigin;

			if ( ent.m_bDucked )
			{
				vecMins += VEC_DUCK_HULL_MIN;
				vecMaxs += VEC_DUCK_HULL_MAX;
			}
			else
			{
				vecMins += VEC_HULL_MIN;
				vecMaxs += VEC_HULL_MAX;
			}

			if ( UTIL_IsLineIntersectingAABB( start, traceEnd, vecMins, vecMaxs ) )
			{
				eligible_players.push_back( { ( start - ent.m_vecOrigin ).LengthSqr(), i } );
			}
		}

		if ( !eligible_players.empty() )
		{
			if ( eligible_players.size() > 1 )
			{
				std::sort( eligible_players.begin(), eligible_players.end(), []( const eligible_player_t &a, const eligible_player_t &b )
				{
					return a.dist_sqr < b.dist_sqr;
				} );
			}

			ent = eligible_players[ 0 ].index;

			eligible_players.clear();
		}
	}

	cl_entity_t *pEntity = cl_enginefuncs->GetEntityByIndex( ent );

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Entity: %d", ent );
	y += height;

	if ( ent == 0 || pEntity->player )
	{
		if ( ent != 0 )
		{
			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( ent - 1 );

			if ( pPlayerInfo != NULL )
			{
				Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, pPlayerInfo->name );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "HP: %.6f", extraplayerinfo->GetHealth( ent ) );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Armor: %.6f", extraplayerinfo->GetArmor( ent ) );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Model: %s", pPlayerInfo->model );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Top Color: %d", pPlayerInfo->topcolor );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Bottom Color: %d", pPlayerInfo->bottomcolor );
				y += height;

				Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Steam64ID: %llu", pPlayerInfo->m_nSteamID );
				y += height;
			}
		}
		else
		{
			Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "worldspawn" );
			y += height;
		}
	}

	if ( !pEntity->player )
	{
		const char *pszModelName = ( ent == 0 ? "N/A" : ( pEntity->model ? pEntity->model->name : "N/A" ) );

		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Model: %s", pszModelName );
		y += height;
	}
	else
	{
		Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Pitch: %.6f", pEntity->curstate.angles.x * ( 89.0f / 9.8876953125f ) );
		y += height;
	}

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Yaw: %.6f", pEntity->curstate.angles.y );
	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X: %.6f", pEntity->curstate.origin.x );
	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y: %.6f", pEntity->curstate.origin.y );
	y += height;

	Features::drawing->DrawStringExF( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Z: %.6f", pEntity->curstate.origin.z );
	//y += height;

	//Features::drawing->DrawStringEx (m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "X Vel: 0.000000" );
	//y += height;

	//Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Y Vel: 0.000000" );
	//y += height;

	//Features::drawing->DrawStringEx( m_hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Z Vel: 0.000000" );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpeedrunHUD::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	m_hEngineFont = VGUI2_GetEngineFont();

	int r = int( 255.f * m_pColor->GetColor()[ 0 ] );
	int g = int( 255.f * m_pColor->GetColor()[ 1 ] );
	int b = int( 255.f * m_pColor->GetColor()[ 2 ] );

	ShowViewangles( r, g, b );
	ShowPosition( r, g, b );
	ShowVelocity( r, g, b );
	ShowGaussBoostInfo( r, g, b );
	ShowSelfgaussInfo( r, g, b );
	ShowEntityInfo( r, g, b );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpeedrunHUD::CSpeedrunHUD( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pColor = NULL;

	m_pShowViewAngles = NULL;
	m_pViewAnglesWidthScreenFraction = NULL;
	m_pViewAnglesHeightScreenFraction = NULL;

	m_pShowPosition = NULL;
	m_pShowPositionViewOrigin = NULL;
	m_pPositionWidthScreenFraction = NULL;
	m_pPositionHeightScreenFraction = NULL;

	m_pShowVelocity = NULL;
	m_pVelocityWidthScreenFraction = NULL;
	m_pVelocityHeightScreenFraction = NULL;

	m_pShowGaussBoostInfo = NULL;
	m_pGaussBoostInfoWidthScreenFraction = NULL;
	m_pGaussBoostInfoHeightScreenFraction = NULL;

	m_pShowSelfGaussInfo = NULL;
	m_pSelfGaussInfoWidthScreenFraction = NULL;
	m_pSelfGaussInfoHeightScreenFraction = NULL;

	m_pShowEntityInfo = NULL;
	m_pShowEntityInfoCheckPlayers = NULL;
	m_pEntityInfoWidthScreenFraction = NULL;
	m_pEntityInfoHeightScreenFraction = NULL;

	m_hEngineFont = 0;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpeedrunHUD::OnEnable( void )
{
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpeedrunHUD::OnDisable( void )
{
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpeedrunHUD::Load( void )
{
	bool bOK = true;

	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, Color( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f, 1.f ) );

	m_pShowViewAngles = Modules::menu->AddParamBool( this, "ShowViewAngles", NULL, false );
	m_pViewAnglesWidthScreenFraction = Modules::menu->AddParamFloat( this, "ViewAnglesWidthScreenFraction", "Width", 0.012f, 0.f, 1.f );
	m_pViewAnglesHeightScreenFraction = Modules::menu->AddParamFloat( this, "ViewAnglesHeightScreenFraction", "Height", 0.09f, 0.f, 1.f );

	m_pShowPosition = Modules::menu->AddParamBool( this, "ShowPosition", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pShowPositionViewOrigin = Modules::menu->AddParamBool( this, "ShowPositionViewOrigin", "View Origin", true );
	m_pPositionWidthScreenFraction = Modules::menu->AddParamFloat( this, "PositionWidthScreenFraction", "Width", 0.012f, 0.f, 1.f );
	m_pPositionHeightScreenFraction = Modules::menu->AddParamFloat( this, "PositionHeightScreenFraction", "Height", 0.16f, 0.f, 1.f );

	m_pShowVelocity = Modules::menu->AddParamBool( this, "ShowVelocity", NULL, false );
	m_pVelocityWidthScreenFraction = Modules::menu->AddParamFloat( this, "VelocityWidthScreenFraction", "Width", 0.012f, 0.f, 1.f );
	m_pVelocityHeightScreenFraction = Modules::menu->AddParamFloat( this, "VelocityHeightScreenFraction", "Height", 0.25f, 0.f, 1.f );

	m_pShowGaussBoostInfo = Modules::menu->AddParamBool( this, "ShowGaussBoostInfo", NULL, false );
	m_pGaussBoostInfoWidthScreenFraction = Modules::menu->AddParamFloat( this, "GaussBoostInfoWidthScreenFraction", "Width", 0.3f, 0.f, 1.f );
	m_pGaussBoostInfoHeightScreenFraction = Modules::menu->AddParamFloat( this, "GaussBoostInfoHeightScreenFraction", "Height", 0.03f, 0.f, 1.f );

	m_pShowSelfGaussInfo = Modules::menu->AddParamBool( this, "ShowSelfGaussInfo", NULL, false );
	m_pSelfGaussInfoWidthScreenFraction = Modules::menu->AddParamFloat( this, "SelfGaussInfoWidthScreenFraction", "Width", 0.575f, 0.f, 1.f );
	m_pSelfGaussInfoHeightScreenFraction = Modules::menu->AddParamFloat( this, "SelfGaussInfoHeightScreenFraction", "Height", 0.03f, 0.f, 1.f );

	m_pShowEntityInfo = Modules::menu->AddParamBool( this, "ShowEntityInfo", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pShowEntityInfoCheckPlayers = Modules::menu->AddParamBool( this, "ShowEntityInfoCheckPlayers", "Check Players", true );
	m_pEntityInfoWidthScreenFraction = Modules::menu->AddParamFloat( this, "EntityInfoWidthScreenFraction", "Width", 0.009f, 0.f, 1.f );
	m_pEntityInfoHeightScreenFraction = Modules::menu->AddParamFloat( this, "EntityInfoHeightScreenFraction", "Height", 0.47f, 0.f, 1.f );

	void *pfnDrawConsoleString = cl_enginefuncs->pfnDrawConsoleString;
#ifdef WIN32
	if ( *(uint8_t *)pfnDrawConsoleString == 0xE9 ) // JMP
		pfnDrawConsoleString = MemoryUtils()->CalcAbsoluteAddress( pfnDrawConsoleString );
#endif

	ud_t inst;
	int iDisassembledBytes = 0;
	uint8_t *p = (uint8_t *)pfnDrawConsoleString;

#ifdef WIN32
	bool bFoundPcThunk = true;
#else
	bool bFoundPcThunk = false;
#endif

	MemoryUtils()->InitDisasm( &inst, pfnDrawConsoleString, 32, 48 );
	while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Icall )
		{
			if ( !bFoundPcThunk )
			{
				bFoundPcThunk = true;
			}
			else
			{
				VGUI2_GetEngineFont = (VGUI2_GetEngineFontFn)MemoryUtils()->CalcAbsoluteAddress( p );
				break;
			}
		}

		p += iDisassembledBytes;
	}

#ifdef LINUX
	if ( VGUI2_GetEngineFont != NULL )
	{
		bFoundPcThunk = false;
		p = (uint8_t *)VGUI2_GetEngineFont;

		MemoryUtils()->InitDisasm( &inst, VGUI2_GetEngineFont, 32, 48 );
		while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
		{
			if ( inst.mnemonic == UD_Icall )
			{
				if ( !bFoundPcThunk )
				{
					bFoundPcThunk = true;
				}
				else
				{
					VGUI2_GetEngineFont = (VGUI2_GetEngineFontFn)MemoryUtils()->CalcAbsoluteAddress( p );
					break;
				}
			}

			p += iDisassembledBytes;
		}
	}
#endif

	FEATURE_CHECK_SYMBOL_STATUS( VGUI2_GetEngineFont, "VGUI2_GetEngineFont" );
	if ( !bOK )
		PrintWarning2( "Default engine font is not available, switching to SvenInt's ESP font\n" );

	return true;
}

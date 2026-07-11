// SvenInt (c) Sw1ft
// st_revive_info.cpp

#include "stdafx.h"
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

EXPOSE_FEATURE_SINGLETON( CReviveInfo, reviveinfo, "Speedrun Tools", "Revive Info" );

//-----------------------------------------------------------------------------
// Simulate Observer Body-Clipping aka Corpse-Clipping aka I don't fucking know how to name that trick properly
// 
// Gonna copypaste explanation from our HL TAS run:
// 
// Corpse Clipping / Observer Body-Clipping (Sven Co-op unique)
// Player with some velocity suicides with use of Selfsinking / Ceil-Clipping trick
// to stuck in the floor / under the ceil and save in the DEAD state all velocity we have.
// A few moments (or after suicide) before entering the OBSERVER mode,
// the server / host sets FPS to the lowest possible which causes the player's body
// clip further into the world / entity. The more speed you have and the lower FPS you set - further you clip,
// direction of your velocity is the clip direction of your body, simple relative velocity-FPS formula:
// 
// ClipDistance = Velocity * 1 / FPS.
// 
// If someone stands on ground when you're in the DEAD state,
// then you will start loosing your velocity because of ground friction (How it happens? Currently no clue LOL).
// Sinking player's body ignores any walls / entities when it's stuck in the world, thus the corpse can clip like that.
// When player is entering OBSERVER mode, their dead body is created which still stores player's velocity,
// then physics simulation runs for the body (move = velocity * framerate) causing it "move" forward,
// on the next game frame velocity of the body resets to 0 because of TraceHull function
// which detects the body to be stuck in the world / entity (TraceHull part is a theory only,
// too lazy to check it, do it on your own).
// 
// The trick's usefulness for speedruns / skips was found by BotSkipper, explored and explained by Sw1ft,
// besides TAS it can be repeated in RTA too (see the latest Uplink / hl_c07_a2 speedrun made by QEES and BotSkipper on speedrun dot com),
// the discovered trick has forced us to remake almost all maps since June just in ~3 months)
//-----------------------------------------------------------------------------

CON_COMMAND( st_obsclip, "Simulate observer clipping" )
{
	if ( args.ArgC() < 3 )
	{
		ConMsg( "Usage:  st_obsclip <speed> <fps>\n" );
		return;
	}

	//edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );
	//if ( pPlayer == NULL )
	//	return;

	Vector va, vecSimOrigin, vecMove;

	Vector vecMins = ( localplayer->IsDucking() ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN );
	Vector vecMaxs = ( localplayer->IsDucking() ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX );

	float speed = (float)atof( args[ 1 ] );
	float frametime = 1.f / (float)atof( args[ 2 ] );

	cl_enginefuncs->GetViewAngles( va );

	vecMove = static_cast<QAngle>( va ).GetForward() * speed * frametime;
	vecSimOrigin = *playermove->origin() + vecMove;

	//pPlayer->v.origin = vecSimOrigin;

	//DrawBox( vecSimOrigin, vecMins, vecMaxs, 1.f, 1.f, 0.f, 0.5f, 3.f, true );
	Features::drawcontext->DrawBox( vecSimOrigin, vecMins, vecMaxs, { 1.f, 1.f, 0.f, 0.5f }, 10.f );
}

//-----------------------------------------------------------------------------
// Incredible shit to find possible revivable spots, all thanks to me ofc
// 
// Some of explanation I did write for our team when HL TAS was in production (RU only):
// 
// 1. Вызывается функция ревайва игрока CBasePlayer::SpecialSpawn (от трупа)
// 
// 2. Происходит проверка корпуса игрока на возможность застревания( используется функция TraceHull ),
//		сама проверка производится с использованием корпуса игрока в полную высоту( то есть не в сидячем состоянии ).
//		Размеры такого корпуса : минимумы = -16, -16, -36 | максимумы 16, 16, 36
// 
// 3. Если окажется, что проверка не удалась( мы как бы застряли ), то происходит смена текущего корпуса игрока на сидячее,
//		в таком случае размеры сидячего корпуса такие : -16, -16, -18 | максимумы 16, 16, 18.
//		А вот если проверка удалась, то корпус игрока остается таким же, как в пункте 2
// 
// 4. Дальше происходит то, в чем смысл я не до конца понял:
//		происходит трассировка линии от текущего положения игрока до положения игрока плюс высота 32 юнита
//		( попроще: от origin до origin + Vector(0, 0, 32) ).
//		Конечная точка результата трассировки заменяется на текущее положение игрока ( player.origin = traceEndPos ),
//		при этом при трассировке игнорируются монстры (игроки тоже?).
//		Скорее всего этот пункт нужен для получения дополнительного, свободного места для возможного анстака.
// 
// 5. Потом вызывается так называемая функция FixPlayerCrouchStuck. В ней снова происходит проверка корпуса,
//		но в этот раз не корпуса игрока, а корпуса головы лел (я пока не знаю какие у него размеры, надо будет реверснуть)
//		P.S. Реверснул: корпус головы - это корпус игрока в сидячем положении.
//		Собственно, происходят 18 итераций подряд с увеличением высоты игрока на 1 юнит, пока трассировка корпуса не покажет, что игрок не застрял.
//		ТО ЕСТЬ!!! Если нас ресают в земле, то наша позиция, а то есть высота, увеличится на 18 юнитов (максимум)!! Думаю вы замечали это изменение, когда вас ресали.
// 
// 6. Финальная рандомная и злоебическая вещь: дальше берется X компонента от размера максимума текущего корпуса игрока
//		(то есть 16, всегда. Назовем эту компоненту maxsX), потом берется корпус игрока
//		(если мы в присяди (а мы практически всегда в присяди, если застряли, см. пункты 2-3),
//		то берутся размеры 16, 16, 18, а если не в присяди, а в полный рост, то размеры 16, 16, 36).
//		Дальше происходит итерация от hulldelta (тут начальное значение hulldelta равно maxsX)
//		до числа 48 (максимальная, так скажем, область) с шагом maxsX
//		(то есть при каждом новом шаге итерации hulldelta будет увеличиваться на maxsX, всего получается 3 итераций,
//		три размера хулла: 16, 32 и 48). Во время этой итерации будет еще одна,
//		в которой 64 раза подряд будет происходить проверка на наличие свободного пространства для телепорта игрока
//		(как только будет найдено свободное пространство хоть один раз, все два цикла итерации оборвутся),
//		собственно в этой второй итерации будет выбираться рандомный спот для проверки,
//		рандомный спот выбирается так: текущее положение плюс минус hulldelta
//		(то есть получается так, что будет выбрана рандомная координата от origin - hulldelta до origin + hulldelta)
//		и так для каждой X, Y и Z компоненты положения игрока. Итого 192 возможных позиций для нахождения свободного спота.
//-----------------------------------------------------------------------------

CON_COMMAND( st_test_revive, "Simulate player/entity revive" )
{
	if ( cls->state != ca_active )
		return;

	// settings
	float flDuration = 1e6f;
	float HULL_STEP = 4.f;
	bool DRAW_VALID_SPOTS_ONLY = false;

	if ( args.ArgC() > 1 )
		HULL_STEP = (float)atof( args[ 1 ] );

	if ( args.ArgC() > 2 )
		DRAW_VALID_SPOTS_ONLY = !!atoi( args[ 2 ] );
	
	if ( args.ArgC() > 3 )
		flDuration = (float)atof( args[ 3 ] );

	Features::drawcontext->DrawClear();

	if ( Modules::server->Host_IsServerActive() )
	{
		edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 );

		if ( Modules::server->FNullEnt( pPlayer ) || !Modules::server->IsValidEntity( pPlayer ) )
			return;

		int flags;
		TraceResult tr;
		Vector vecOrigin, vecHullMins, vecHullMaxs;

		flags = pPlayer->v.flags;
		vecOrigin = pPlayer->v.origin;

		sv_enginefuncs->pfnTraceHull( vecOrigin, vecOrigin, 0 /* dont_ignore_monsters */, 1 /* human_hull */, pPlayer, &tr );

		Msg( "sc_test_revive: Ducking = %d\n", ( flags & FL_DUCKING ) == FL_DUCKING );

		if ( flags & FL_DUCKING || tr.fStartSolid )
		{
			vecHullMins = VEC_DUCK_HULL_MIN;
			vecHullMaxs = VEC_DUCK_HULL_MAX;

			flags |= FL_DUCKING;
		}
		else
		{
			vecHullMins = VEC_HULL_MIN;
			vecHullMaxs = VEC_HULL_MAX;
		}

		Msg( "sc_test_revive: TraceHull, blocked = %d\n", tr.fStartSolid );

		Vector vecUpHead = vecOrigin + Vector( 0, 0, 32 ); // idk about name

		sv_enginefuncs->pfnTraceLine( vecUpHead, vecOrigin, 1 /* ignore_monsters */, pPlayer, &tr );

		Msg( "sc_test_revive: TraceLine up, old = (%.3f, %.3f, %.3f), new = (%.3f, %.3f, %.3f)\n", VectorExpand( vecOrigin ), VectorExpand( tr.vecEndPos ) );

		vecOrigin = tr.vecEndPos;

		// FixPlayerCrouchStuck
		for ( int i = 0; i < 18; i++ )
		{
			Msg( "sc_test_revive: FixPlayerCrouchStuck, vecOrigin.z = %.3f\n", vecOrigin.z );

			sv_enginefuncs->pfnTraceHull( vecOrigin, vecOrigin, 0 /* dont_ignore_monsters */, 3 /* head_hull */, pPlayer, &tr );

			if ( !tr.fStartSolid )
				break;

			vecOrigin.z += 1.f;
		}

		// FixPlayerStuck
		int maxsX, hulltype;
		float minX, maxX, minY, maxY, minZ, maxZ;
		Vector vecHull, vecTestOrigin;

		const int MAX_HULL_BOUND = 48;

		const Vector vecDebugBoxMins( -0.5, -0.5, -0.5 );
		const Vector vecDebugBoxMaxs( 0.5, 0.5, 0.5 );

		if ( vecHullMaxs.x > 0.f )
			maxsX = (int)vecHullMaxs.x;
		else
			maxsX = 8;

		vecHull.x = 16.f;
		vecHull.y = 16.f;

		if ( flags & FL_DUCKING )
		{
			vecHull.z = 36.f;
			hulltype = 3; // head_hull
		}
		else
		{
			vecHull.z = 72.f;
			hulltype = 1; // human_hull
		}

		if ( maxsX > MAX_HULL_BOUND )
			return;
		
		// Iterate from the largest hull, don't spend time and perfomance on small and medium ones
		for ( int i = MAX_HULL_BOUND; i <= MAX_HULL_BOUND; i += maxsX )
			//for ( int i = maxsX; i <= MAX_HULL_BOUND; i += maxsX )
		{
			Msg( "sc_test_revive: FixPlayerStuck, hull = %d\n", i );

			float hull = (float)i;

			minX = vecOrigin.x - hull;
			maxX = vecOrigin.x + hull;

			minY = vecOrigin.y - hull;
			maxY = vecOrigin.y + hull;

			minZ = vecOrigin.z - hull;
			maxZ = vecOrigin.z + hull;

			for ( float x = minX; x <= maxX; x += HULL_STEP )
			{
				for ( float y = minY; y <= maxY; y += HULL_STEP )
				{
					for ( float z = minZ; z <= maxZ; z += HULL_STEP )
					{
						vecTestOrigin.x = x;
						vecTestOrigin.y = y;
						vecTestOrigin.z = z;

						sv_enginefuncs->pfnTraceHull( vecTestOrigin, vecHull, 0 /* dont_ignore_monsters */, hulltype, pPlayer, &tr );

						// Free space
						if ( !tr.fStartSolid )
						{
							Features::drawcontext->DrawBox( vecTestOrigin, vecDebugBoxMins, vecDebugBoxMaxs, { 0, 255, 0, 127 }, 1e6 );
						}
						else if ( !DRAW_VALID_SPOTS_ONLY )
						{
							Features::drawcontext->DrawBox( vecTestOrigin, vecDebugBoxMins, vecDebugBoxMaxs, { 255, 0, 0, 127 }, 1e6 );
						}
					}
				}
			}
		}

		return;
	}

	// Client one is inconsistent but still it can give good results

	int flags;
	pmtrace_t tr;
	Vector vecOrigin, vecHullMins, vecHullMaxs;

	flags = playermove->flags();
	vecOrigin = *playermove->origin();

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_PLAYER ); // human_hull
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecOrigin, vecOrigin, PM_NORMAL, -1, &tr );

	Msg( "sc_test_revive: Ducking = %d\n", ( flags & FL_DUCKING ) == FL_DUCKING );

	if ( flags & FL_DUCKING || tr.startsolid )
	{
		vecHullMins = VEC_DUCK_HULL_MIN;
		vecHullMaxs = VEC_DUCK_HULL_MAX;

		flags |= FL_DUCKING;
	}
	else
	{
		vecHullMins = VEC_HULL_MIN;
		vecHullMaxs = VEC_HULL_MAX;
	}

	Msg( "sc_test_revive: TraceHull, blocked = %d\n", tr.startsolid );

	Vector vecUpHead = vecOrigin + Vector( 0, 0, 32 ); // idk about name

	// Inconsistent!!! No trace flag 'ignore_monsters' for the client
	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecUpHead, vecOrigin, PM_NORMAL, -1, &tr );

	Msg( "sc_test_revive: TraceLine up, old = (%.3f, %.3f, %.3f), new = (%.3f, %.3f, %.3f)\n", VectorExpand( vecOrigin ), VectorExpand( tr.endpos ) );

	vecOrigin = tr.endpos;

	// FixPlayerCrouchStuck
	for ( int i = 0; i < 18; i++ )
	{
		Msg( "sc_test_revive: FixPlayerCrouchStuck, z = %.3f\n", vecOrigin.z );

		cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_DUCKED_PLAYER ); // head_hull but it's just ducked hull of player
		cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecOrigin, vecOrigin, PM_NORMAL, -1, &tr );

		if ( !tr.startsolid )
			break;

		vecOrigin.z += 1.f;
	}

	// FixPlayerStuck
	int maxsX, hulltype;
	float minX, maxX, minY, maxY, minZ, maxZ;
	Vector vecHull, vecTestOrigin;

	const int MAX_HULL_BOUND = 48;

	const Vector vecDebugBoxMins( -0.5, -0.5, -0.5 );
	const Vector vecDebugBoxMaxs( 0.5, 0.5, 0.5 );

	if ( vecHullMaxs.x > 0.f )
		maxsX = (int)vecHullMaxs.x;
	else
		maxsX = 8;

	vecHull.x = 16.f;
	vecHull.y = 16.f;

	if ( flags & FL_DUCKING )
	{
		vecHull.z = 36.f;
		hulltype = PM_HULL_DUCKED_PLAYER; // head_hull but it's just ducked hull of player
	}
	else
	{
		vecHull.z = 72.f;
		hulltype = PM_HULL_PLAYER; // human_hull
	}

	if ( maxsX > MAX_HULL_BOUND )
		return;
	
	// Iterate from the largest hull, don't spend time and perfomance on small and medium ones
	for ( int i = MAX_HULL_BOUND; i <= MAX_HULL_BOUND; i += maxsX )
	//for ( int i = maxsX; i <= MAX_HULL_BOUND; i += maxsX )
	{
		Msg( "sc_test_revive: FixPlayerStuck, hull = %d\n", i );

		float hull = (float)i;

		minX = vecOrigin.x - hull;
		maxX = vecOrigin.x + hull;

		minY = vecOrigin.y - hull;
		maxY = vecOrigin.y + hull;

		minZ = vecOrigin.z - hull;
		maxZ = vecOrigin.z + hull;

		for ( float x = minX; x <= maxX; x += HULL_STEP )
		{
			for ( float y = minY; y <= maxY; y += HULL_STEP )
			{
				for ( float z = minZ; z <= maxZ; z += HULL_STEP )
				{
					vecTestOrigin.x = x;
					vecTestOrigin.y = y;
					vecTestOrigin.z = z;

					cl_enginefuncs->pEventAPI->EV_SetTraceHull( hulltype );
					cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecTestOrigin, vecHull, PM_NORMAL, -1, &tr );

					// Free space
					if ( !tr.startsolid )
					{
						Features::drawcontext->DrawBox( vecTestOrigin, vecDebugBoxMins, vecDebugBoxMaxs, { 0, 255, 0, 127 }, 1e6 );
					}
					else if ( !DRAW_VALID_SPOTS_ONLY )
					{
						Features::drawcontext->DrawBox( vecTestOrigin, vecDebugBoxMins, vecDebugBoxMaxs, { 255, 0, 0, 127 }, 1e6 );
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Reset revive info
//-----------------------------------------------------------------------------

void CReviveInfo::Reset( void )
{
	m_bShowReviveInfo = false;
	m_pReviveTarget = NULL;
	m_flReviveDistance = -1.f;
}

//-----------------------------------------------------------------------------
// Draw revive info
//-----------------------------------------------------------------------------

void CReviveInfo::DrawReviveInfo( void )
{
	int iWeaponID = localplayer->GetCurrentWeaponID();

	if ( UTIL_IsDead() || !( iWeaponID == WEAPON_MEDKIT || ( m_pShowWithAnyWeapon->GetBool() ) ) )
		return;
	
	pmtrace_t tr;
	Vector vecForward;

	int ignore_ent = -1;

	cl_enginefuncs->pfnAngleVectors( demoplayback ? refparams->viewangles : *playermove->angles(), vecForward, NULL, NULL );

	Vector vecSrc = ( demoplayback ? *(Vector *)refparams->simorg + refparams->viewheight : localplayer->GetEyePosition() );
	Vector vecEnd = vecSrc + vecForward * 16.f;

	// Trace line
	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_NORMAL, ignore_ent, &tr );

	if ( tr.fraction >= 1.0 )
	{
		// Trace hull
		cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_DUCKED_PLAYER ); // in server-side head_hull has the same size as hull of ducked player
		//cl_enginefuncs->pEventAPI->EV_SetTraceHull( 3 ); // PM_HULL_HEAD, need to fix SDK
		cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecSrc, vecEnd, PM_NORMAL, ignore_ent, &tr );

		if ( tr.fraction < 1.0 )
		{
			// Calculate the point of intersection of the line (or hull) and the object we hit
			// This is and approximation of the "best" intersection
			int ent = cl_enginefuncs->pEventAPI->EV_IndexFromTrace( &tr );
			cl_entity_t *pEntity = cl_enginefuncs->GetEntityByIndex( ent );

			// pEntity == NULL || pEntity->IsBSPModel();
			if ( ent == 0 || ( pEntity != NULL && ( pEntity->curstate.solid == SOLID_BSP || pEntity->curstate.movetype == MOVETYPE_PUSHSTEP ) ) )
				UTIL_FindHullIntersectionClient( vecSrc, tr, Vector( -16, -16, -18 ), Vector( 16, 16, 18 ), ignore_ent ); // Duck hull

			vecEnd = tr.endpos;	// This is the point on the actual surface (the hull could have hit space)
		}
	}

	// Find target to revive within radius
	float flDistanceToTarget;
	Vector vecOrigin, vecAbsMins, vecAbsMaxs;

	cl_entity_t *pTarget = NULL;

	const float flTime = (float)*realtime;
	const float flReviveRadius = 64.f;
	const float flReviveRadiusSqr = flReviveRadius * flReviveRadius;

	CEntity *pEnts = Features::entitylist->GetList();

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		if ( ent.m_pEntity->curstate.effects & EF_NODRAW )
			continue;

		// The only revivable targets are players in DEAD state, grenades and corpses of players
		if ( ( ent.m_bPlayer && !ent.m_bAlive ) || ( ent.m_classInfo.id == CLASS_ITEM_GRENADE && ( ent.m_bNeutral || ent.m_bEnemy ) ) || ent.m_classInfo.id == CLASS_DEAD_PLAYER )
		{
			if ( ent.m_classInfo.id != CLASS_ITEM_GRENADE )
			{
				vecAbsMins = ent.m_vecOrigin + ent.m_vecMins;
				vecAbsMaxs = ent.m_vecOrigin + ent.m_vecMaxs;
			}
			else
			{
				vecAbsMins = ent.m_pEntity->curstate.origin + Vector( -1, -1, -1 );
				vecAbsMaxs = ent.m_pEntity->curstate.origin + Vector( 1, 1, 1 );
			}

			if ( !UTIL_IsSphereIntersectingAABB( vecEnd, flReviveRadiusSqr, vecAbsMins, vecAbsMaxs, &flDistanceToTarget ) )
				continue;

			// We got a player / their dead body / grenade within radius
			pTarget = ent.m_pEntity;
			vecOrigin = ent.m_pEntity->curstate.origin;
			break;
		}
	}

	// Find targets from transmitted server hulls
	if ( pTarget == NULL && Features::playerhulls->IsEnabled() && Features::playerhulls->AreServerHullsDisplayed() )
	{
		cl_entity_t *pEntity = NULL;

		for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
		{
			const playerhull_display_info_t &display_info = Features::playerhulls->GetHull( i );

			if ( display_info.time < flTime || !display_info.dead )
				continue;
			
			pEntity = cl_enginefuncs->GetEntityByIndex( i );

			if ( pEntity == NULL )
				continue;

			vecAbsMins = display_info.origin + display_info.mins;
			vecAbsMaxs = display_info.origin + display_info.maxs;

			if ( !UTIL_IsSphereIntersectingAABB( vecEnd, flReviveRadiusSqr, vecAbsMins, vecAbsMaxs, &flDistanceToTarget ) )
				continue;

			pTarget = pEntity;
			vecOrigin = display_info.origin;
			break;
		}
	}

	// Valid target
	if ( pTarget != NULL )
	{
		float r, g, b, a;

		r = m_pNoAmmoColor->GetColor()[ 0 ];
		g = m_pNoAmmoColor->GetColor()[ 1 ];
		b = m_pNoAmmoColor->GetColor()[ 2 ];
		a = m_pNoAmmoColor->GetColor()[ 3 ];

		if ( iWeaponID == WEAPON_MEDKIT )
		{
			// Enough ammo to revive
			if ( clientweapon->PrimaryAmmo() >= 50 )
			{
				r = m_pColor->GetColor()[ 0 ];
				g = m_pColor->GetColor()[ 1 ];
				b = m_pColor->GetColor()[ 2 ];
				a = m_pColor->GetColor()[ 3 ];
			}
		}
		else
		{
			WEAPON *pWeapon = inventory->GetWeapon( WEAPON_MEDKIT );

			if ( pWeapon != NULL && inventory->GetPrimaryAmmoCount( pWeapon ) >= 50 )
			{
				r = m_pColor->GetColor()[ 0 ];
				g = m_pColor->GetColor()[ 1 ];
				b = m_pColor->GetColor()[ 2 ];
				a = m_pColor->GetColor()[ 3 ];
			}
		}

		CDrawBoxNoDepthBuffer *pDrawBoxTarget = new CDrawBoxNoDepthBuffer( vecOrigin,
																		   Vector( -2, -2, -2 ),
																		   Vector( 2, 2, 2 ),
																		   { r, g, b, a } );

		Features::drawcontext->AddDrawContext( pDrawBoxTarget );

		m_pReviveTarget = pTarget;
		m_flReviveDistance = flReviveRadius - flDistanceToTarget; // invert
	}
	else
	{
		m_pReviveTarget = NULL;
		m_flReviveDistance = -1.f;
	}

	m_bShowReviveInfo = true;
}

//-----------------------------------------------------------------------------
// Show revive info
//-----------------------------------------------------------------------------

void CReviveInfo::ShowReviveInfo( int r, int g, int b )
{
	if ( !m_bShowReviveInfo )
		return;
	
	int width, height;

	int x = int( (float)gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() );

	vgui::HFont hEngineFont = Features::sthud->GetEngineFont();

	if ( m_pReviveTarget != NULL )
	{
		if ( m_pReviveTarget->player )
		{
			player_info_t *pPlayerInfo = enginestudio->PlayerInfo( m_pReviveTarget->index - 1 );

			Features::drawing->DrawStringExF( hEngineFont,
											  x, y, r, g, b, 255,
											  width, height,
											  FONT_ALIGN_LEFT,
											  "Revive Target: %s (%d)",
											  pPlayerInfo ? pPlayerInfo->name : "N/A", m_pReviveTarget->index );
		}
		else
		{
			Features::drawing->DrawStringExF( hEngineFont,
											  x, y, r, g, b, 255,
											  width, height,
											  FONT_ALIGN_LEFT,
											  "Revive Target: %s (%d)",
											  m_pReviveTarget->model ? m_pReviveTarget->model->name : "DEADPLAYER", m_pReviveTarget->index );
		}
	}
	else
	{
		Features::drawing->DrawStringEx( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Revive Target: N/A" );
	}

	y += height;

	if ( m_flReviveDistance >= 0.f )
	{
		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Distance: %.2f", m_flReviveDistance );
	}
	else
	{
		Features::drawing->DrawStringExF( hEngineFont, x, y, r, g, b, 255, width, height, FONT_ALIGN_LEFT, "Distance: N/A" );
	}

	m_bShowReviveInfo = false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CReviveInfo::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		DrawReviveInfo();
		return kHookContinue;
	}

	Color clrHud = Features::sthud->GetColor();
	ShowReviveInfo( clrHud.r, clrHud.g, clrHud.b );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CReviveInfo::CReviveInfo( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pShowWithAnyWeapon = NULL;
	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;
	m_pColor = NULL;
	m_pNoAmmoColor = NULL;

	Reset();
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CReviveInfo::OnEnable( void )
{
	Reset();

	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookCall, kHookPriorityHigh );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent, kHookCall, kHookPriorityLow );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CReviveInfo::OnDisable( void )
{
	Reset();

	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CReviveInfo::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShowWithAnyWeapon = Modules::menu->AddParamBool( this, "ShowWithAnyWeapon", NULL, false );
	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.009f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.25f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGBA( this, "Color", NULL, Color( 0.f, 1.f, 0.f, 0.5f ) );
	m_pNoAmmoColor = Modules::menu->AddParamColorRGBA( this, "NoAmmoColor", NULL, Color( 1.f, 0.f, 0.f, 0.5f ) );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CReviveInfo::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( st_obsclip );
	FEATURE_REGISTER_CCMD( st_test_revive );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CReviveInfo::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( st_obsclip );
	FEATURE_UNREGISTER_CCMD( st_test_revive );
}
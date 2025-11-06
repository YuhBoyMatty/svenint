// SvenInt (c) Sw1ft
// st_player_hulls.cpp

#include "stdafx.h"
#include "st_player_hulls.h"
#include "r_draw_context.h"
#include "r_drawing.h"
#include "misc_entity_list.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CPlayerHulls, playerhulls, "Speedrun Tools", "Player Hulls" );

//-----------------------------------------------------------------------------
// Check existing players to display
//-----------------------------------------------------------------------------

void CPlayerHulls::CheckPlayerHulls_Server( void )
{
	using namespace GameData::Offsets::Server;
	FUNC_SIGNATURE( bool, __thiscall, CBasePlayer__IsAliveFn, CBasePlayer * );
	FUNC_SIGNATURE( bool, __thiscall, CBasePlayer__IsConnectedFn, CBasePlayer * );

	const float flTime = (float)*realtime;

	if ( m_pShowServerPlayerHulls->GetBool() &&
		 vtidx_CBasePlayer_IsAlive != ~0 &&
		 vtidx_CBasePlayer_IsConnected != ~0 )
	{
		static CBasePlayer__IsAliveFn CBasePlayer__IsAlive = NULL;
		static CBasePlayer__IsConnectedFn CBasePlayer__IsConnected = NULL;

		edict_t *pEntity = NULL;
		edict_t *pPlayerEdict = NULL;

		CBasePlayer *pPlayer = NULL;
		CBaseEntity *pDeadPlayer = NULL;

		// Check bodies of dead players
		while ( !Modules::server->FNullEnt( pEntity = sv_enginefuncs->pfnFindEntityByString( pEntity, "classname", "deadplayer" ) ) )
		{
			if ( !Modules::server->IsValidEntity( pEntity ) )
				continue;

			if ( pEntity->v.effects & EF_NODRAW )
				continue;

			const int client = (int)pEntity->v.renderamt;
			pPlayerEdict = sv_enginefuncs->pfnPEntityOfEntIndex( client );

			if ( !Modules::server->IsValidEntity( pPlayerEdict ) )
				continue;

			if ( m_flDisplayHullsNextSend <= flTime )
				BroadcastPlayerHull_Server( client, 1, pEntity->v.origin, int( pEntity->v.mins.z ) == -18 );

			DrawPlayerHull_Comm( client, 1, pEntity->v.origin, int( pEntity->v.mins.z ) == -18 );
		}

		// Now check players themselves
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			pEntity = sv_enginefuncs->pfnPEntityOfEntIndex( i );
			if ( !Modules::server->IsValidEntity( pEntity ) )
				continue;

			pPlayer = reinterpret_cast<CBasePlayer *>( pEntity->pvPrivateData );

			if ( CBasePlayer__IsAlive == NULL )
			{
				CBasePlayer__IsAlive = (CBasePlayer__IsAliveFn)MemoryUtils()->GetVirtualFunction( pPlayer, vtidx_CBasePlayer_IsAlive );
				CBasePlayer__IsConnected = (CBasePlayer__IsConnectedFn)MemoryUtils()->GetVirtualFunction( pPlayer, vtidx_CBasePlayer_IsConnected );

				AssertMsg( CBasePlayer__IsAlive && CBasePlayer__IsConnected, "CBasePlayer::IsAlive && CBasePlayer::IsConnected" );
			}

			if ( !CBasePlayer__IsConnected( pPlayer ) || !CBasePlayer__IsAlive( pPlayer ) )
				continue;

			if ( m_flDisplayHullsNextSend <= flTime )
				BroadcastPlayerHull_Server( i, 0, pEntity->v.origin, int( pEntity->v.mins.z ) == -18 );

			DrawPlayerHull_Comm( i, 0, pEntity->v.origin, int( pEntity->v.mins.z ) == -18 );
		}
	}

	if ( m_flDisplayHullsNextSend <= flTime )
		m_flDisplayHullsNextSend = flTime + 0.05f;
}

//-----------------------------------------------------------------------------
// Draws nicknames of players to display
//-----------------------------------------------------------------------------

void CPlayerHulls::DrawPlayersHullsNickname_Server( void )
{
	if ( !m_pShowServerPlayerHulls->GetBool() )
		return;

	float vecScreen[ 2 ];

	for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
	{
		playerhull_display_info_t &display_info = m_playerHulls[ i ];

		if ( /* !display_info.dead || */ display_info.time < (float)*realtime )
			continue;

		if ( !UTIL_WorldToScreen( display_info.origin, vecScreen ) )
			continue;

		player_info_t *pPlayerInfo = enginestudio->PlayerInfo( i - 1 );
		if ( pPlayerInfo == NULL )
			continue;

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
										(int)vecScreen[ 0 ], (int)vecScreen[ 1 ],
										255, 255, 255, 255,
										FONT_ALIGN_CENTER,
										pPlayerInfo->name );
	}
}

//-----------------------------------------------------------------------------
// Send player hull to everyone
//-----------------------------------------------------------------------------

void CPlayerHulls::BroadcastPlayerHull_Server( int client, int dead, const Vector &vecOrigin, bool bDuck )
{
	struct
	{
		unsigned char client : 6;
		unsigned char dead : 1;
		unsigned char duck : 1;
	} displayInfo;

	displayInfo.client = client;
	displayInfo.dead = dead;
	displayInfo.duck = bDuck;

	//sv_enginefuncs->pfnMessageBegin( MSG_PVS, SVC_SVENINT, NULL, NULL );
	sv_enginefuncs->pfnMessageBegin( MSG_BROADCAST, SVC_SVENINT, NULL, NULL );
		sv_enginefuncs->pfnWriteByte( SVENINT_COMM_DISPLAY_PLAYER_HULL );
		sv_enginefuncs->pfnWriteByte( *(unsigned char *)&displayInfo );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( vecOrigin.x ) );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( vecOrigin.y ) );
		sv_enginefuncs->pfnWriteLong( FloatToLong32( vecOrigin.z ) );
	sv_enginefuncs->pfnMessageEnd();
}

//-----------------------------------------------------------------------------
// Draw player's hull
//-----------------------------------------------------------------------------

void CPlayerHulls::DrawPlayerHull_Comm( int client, int dead, const Vector &vecOrigin, bool bDuck )
{
	if ( !m_pShowServerPlayerHulls->GetBool() || ( !dead && client == UTIL_GetLocalPlayerIndex() ) )
		return;

	playerhull_display_info_t &display_info = m_playerHulls[ client ];

	if ( bDuck )
	{
		display_info.mins = VEC_DUCK_HULL_MIN;
		display_info.maxs = VEC_DUCK_HULL_MAX;
	}
	else
	{
		display_info.mins = VEC_HULL_MIN;
		display_info.maxs = VEC_HULL_MAX;
	}

	display_info.dead = dead;
	display_info.origin = vecOrigin;
	display_info.time = (float)*realtime + 1.f;
}

//-----------------------------------------------------------------------------
// Draw player hulls
//-----------------------------------------------------------------------------

void CPlayerHulls::DrawPlayerHulls( void )
{
	if ( m_pShowServerPlayerHulls->GetBool() )
	{
		int iLocalPlayer = UTIL_GetLocalPlayerIndex();

		for ( int i = 1; i <= cl_enginefuncs->GetMaxClients(); i++ )
		{
			playerhull_display_info_t &display_info = m_playerHulls[ i ];

			if ( display_info.time < (float)*realtime )
				continue;

			if ( iLocalPlayer == i && !m_pShowLocalPlayerHull->GetBool() )
				continue;

			if ( display_info.dead )
			{
				if ( m_pDeadPlayerColor->GetColor()[ 3 ] == 0.f )
					continue;

				DrawBox( display_info.origin,
						 display_info.mins,
						 display_info.maxs,
						 m_pDeadPlayerColor->GetColor()[ 0 ],
						 m_pDeadPlayerColor->GetColor()[ 1 ],
						 m_pDeadPlayerColor->GetColor()[ 2 ],
						 m_pDeadPlayerColor->GetColor()[ 3 ],
						 m_pWireframeWidth->GetFloat(),
						 m_pShowWireframe->GetBool() );

				continue;
			}

			if ( m_pColor->GetColor()[ 3 ] == 0.f )
				continue;

			DrawBox( display_info.origin,
					 display_info.mins,
					 display_info.maxs,
					 m_pColor->GetColor()[ 0 ],
					 m_pColor->GetColor()[ 1 ],
					 m_pColor->GetColor()[ 2 ],
					 m_pColor->GetColor()[ 3 ],
					 m_pWireframeWidth->GetFloat(),
					 m_pShowWireframe->GetBool() );
		}

		return;
	}

	int iLocalPlayer = UTIL_GetLocalPlayerIndex();
	CEntity *pEnts = Features::entitylist->GetList();

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		if ( ent.m_classInfo.id != CLASS_DEAD_PLAYER && !ent.m_bPlayer )
			continue;

		if ( iLocalPlayer == i && ( !m_pShowLocalPlayerHull->GetBool() || UTIL_IsSpectating() ) )
			continue;

		if ( ent.m_bPlayer )
		{
			if ( m_pColor->GetColor()[ 3 ] == 0.f )
				continue;

			DrawBox( iLocalPlayer == i ? ( demoplayback ? refparams->simorg : *playermove->origin() ) : ent.m_pEntity->origin,
					 ent.m_vecMins,
					 ent.m_vecMaxs,
					 m_pColor->GetColor()[ 0 ],
					 m_pColor->GetColor()[ 1 ],
					 m_pColor->GetColor()[ 2 ],
					 m_pColor->GetColor()[ 3 ],
					 m_pWireframeWidth->GetFloat(),
					 m_pShowWireframe->GetBool() );

			continue;
		}

		if ( m_pDeadPlayerColor->GetColor()[ 3 ] == 0.f )
			continue;

		DrawBox( ent.m_pEntity->origin,
				 ent.m_vecMins,
				 ent.m_vecMaxs,
				 m_pDeadPlayerColor->GetColor()[ 0 ],
				 m_pDeadPlayerColor->GetColor()[ 1 ],
				 m_pDeadPlayerColor->GetColor()[ 2 ],
				 m_pDeadPlayerColor->GetColor()[ 3 ],
				 m_pWireframeWidth->GetFloat(),
				 m_pShowWireframe->GetBool() );
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CPlayerHulls::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		for ( int i = 0; i < MAX_CLIENTS + 1; i++ )
			m_playerHulls[ i ].time = -1.f;

		m_flDisplayHullsNextSend = -1.f;
	}
	else if ( pEvent->GetType() == kVGuiClientPanelPaint_HookEvent )
	{
		DrawPlayersHullsNickname_Server();
	}
	else if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		DrawPlayerHulls();
	}
	else if ( pEvent->GetType() == kHost_FilterTime_HookEvent )
	{
		if ( !pEvent->GetReturn<qboolean>() || cls->state != ca_active )
			return kHookContinue;

		CheckPlayerHulls_Server();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CPlayerHulls::CPlayerHulls( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pShowServerPlayerHulls = NULL;
	m_pShowLocalPlayerHull = NULL;
	m_pShowWireframe = NULL;
	m_pWireframeWidth = NULL;
	m_pColor = NULL;
	m_pDeadPlayerColor = NULL;

	m_playerHulls.reserve( MAX_CLIENTS + 1 );
	for ( int i = 0; i < MAX_CLIENTS + 1; i++ )
		m_playerHulls.push_back( { 0, Vector(), Vector(), Vector(), -1.f } );

	m_flDisplayHullsNextSend = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CPlayerHulls::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CPlayerHulls::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CPlayerHulls::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShowServerPlayerHulls = Modules::menu->AddParamBool( this, "ShowServerPlayerHulls", NULL, false );
	m_pShowLocalPlayerHull = Modules::menu->AddParamBool( this, "ShowLocalPlayerHull", NULL, false );
	m_pShowWireframe = Modules::menu->AddParamBool( this, "ShowWireframe", NULL, false );
	m_pWireframeWidth = Modules::menu->AddParamFloat( this, "WireframeWidth", NULL, 4.f, 1.f, 10.f );
	m_pColor = Modules::menu->AddParamColorRGBA( this, "Color", NULL, Color( 232.f / 255.f, 0.f / 255.f, 232.f / 255.f, 96.f / 255.f ) );
	m_pDeadPlayerColor = Modules::menu->AddParamColorRGBA( this, "DeadPlayerColor", NULL, Color( 189.f / 255.f, 134.f / 255.f, 240.f / 255.f, 96.f / 255.f ) );

	return true;
}
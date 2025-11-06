// SvenInt (c) Sw1ft
// visual_esp.cpp

#include "stdafx.h"
#include "svenint.h"
#include "visual_esp.h"
#include "r_drawing.h"
#include "player_camhack.h"
#include "misc_entity_list.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Client
		{
			DEFINE_PATTERNS_2( CClient_SoundEngine__PlayFMODSound,
							   "5.26",
							   "55 8B EC 83 E4 ? 81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 94 00 00 00",
							   "5.25",
							   "55 8B EC 83 E4 ? 81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 B4 00 00 00 8B 45 18" );
		}
	}
}

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

struct esp_bone_s
{
	Vector vecPoint[ MAXSTUDIOBONES ];
	int nParent[ MAXSTUDIOBONES ] = { -1 };
};

#ifdef PROCESS_PLAYER_BONES_ONLY
static esp_bone_s gBones[ MAXCLIENTS + 1 ];
#else
static esp_bone_s gBones[ MY_MAXENTS + 1 ];
#endif

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_CLASS_HOOK( void, CClient_SoundEngine__PlayFMODSound, void *thisptr, int fFlags, int entindex, float *vecOrigin, int iChannel, const char *pszSample, float flVolume, float flAttenuation, int iUnknown, int iPitch, int iSoundIndex, float flOffset );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CESP, esp, "Visual", "ESP" );

//-----------------------------------------------------------------------------
// Client sound engine hook
//-----------------------------------------------------------------------------

DECLARE_CLASS_FUNC( void, HOOKED_CClient_SoundEngine__PlayFMODSound, void *thisptr, int fFlags, int entindex, float *vecOrigin, int iChannel, const char *pszSample, float flVolume, float flAttenuation, int iUnknown, int iPitch, int iSoundIndex, float flOffset )
{
	ORIG_CClient_SoundEngine__PlayFMODSound( thisptr, fFlags, entindex, vecOrigin, iChannel, pszSample, flVolume, flAttenuation, iUnknown, iPitch, iSoundIndex, flOffset );

	// Actually, it should be in entity list but whatever
	if ( entindex > 0 && entindex <= Features::entitylist->GetMaxEntities() )
	{
		CEntity &ent = Features::entitylist->GetList()[ entindex ];
		if ( ent.m_bValid )
			return;

		if ( vecOrigin == NULL || pszSample == NULL || *pszSample == '\0' || ( vecOrigin[ 0 ] == 0.f && vecOrigin[ 1 ] == 0.f && vecOrigin[ 2 ] == 0.f ) )
			return;

		float flTime = cl_enginefuncs->GetClientTime();
		if ( flTime - ent.m_flLastEmitSoundTime > 1.5f )
			ent.m_vecPrevOrigin = *(Vector *)vecOrigin;

		ent.m_flLastEmitSoundTime = cl_enginefuncs->GetClientTime();
		ent.m_vecOrigin = *(Vector *)vecOrigin;
		ent.m_vecVelocity.Zero();
	}
}

//-----------------------------------------------------------------------------
// Draw sound ESP
//-----------------------------------------------------------------------------

void CESP::DrawSoundESP( void )
{
	if ( !m_pSoundESP->GetBool() )
		return;

	static Vector vecBottom;
	static Vector vecTop;

	float boxWidth = 0.f;
	float vecScreenBottom[ 2 ], vecScreenTop[ 2 ];

	const int width = gameutils->GetScreenWidth();
	const int height = gameutils->GetScreenHeight();

	int iLocalPlayer = UTIL_GetLocalPlayerIndex();

	cl_entity_s *pLocal = cl_enginefuncs->GetLocalPlayer();
	CEntity *pEnts = Features::entitylist->GetList();

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( ent.m_bValid )
			continue;
		
		if ( ent.m_bNeutral )
			continue;

		if ( cl_enginefuncs->GetClientTime() - ent.m_flLastEmitSoundTime > m_pSoundESPFadeOut->GetFloat() )
			continue;

		if ( iLocalPlayer == i )
			continue;

		bool bPlayer = ent.m_bPlayer;
		bool bItem = ent.m_bItem;

		if ( bItem && !m_pShowItems->GetBool() )
			continue;

		float flDistanceSqr = ( ent.m_vecOrigin - pLocal->origin ).LengthSqr();
		if ( flDistanceSqr > M_SQR( m_pDistance->GetFloat() ) )
			continue;

		float x, y, w, h;
		float top_mid_x, top_mid_y, bottom_mid_x, bottom_mid_y;

		// We do some interpolation
		ent.m_vecPrevOrigin.Lerp( ent.m_vecOrigin, UTIL_SimpleSpline( 0.1f ) );

		vecBottom = ent.m_vecPrevOrigin;
		vecTop = ent.m_vecPrevOrigin;

		if ( !bPlayer )
		{
			if ( ent.m_classInfo.id == CLASS_NONE &&
				 m_pIgnoreUnknownEnts->GetBool() ||
				 ent.m_classInfo.id == CLASS_DEAD_PLAYER )
				continue;

			// Don't process if entity isn't an ESP's target
			if ( m_pDrawTargets->GetInt() == 2 )
				continue;

			if ( !bItem )
			{
				vecTop.z += ent.m_vecMaxs.z;
				vecBottom.z -= ent.m_vecMins.z;

				float height = ( ent.m_vecMaxs.z - ent.m_vecMins.z );

				boxWidth = ( height != 0.f ) ? ( Q_max( ent.m_vecMaxs.x - ent.m_vecMins.x, ent.m_vecMaxs.y - ent.m_vecMins.y ) / height ) : 0.f;
			}
		}
		else
		{
			if ( m_pDrawTargets->GetInt() == 1 )
				continue;

			if ( ent.m_bDucked )
			{
				vecTop.z += VEC_DUCK_HULL_MAX.z;
				vecBottom.z += VEC_DUCK_HULL_MIN.z;

				boxWidth = ( VEC_DUCK_HULL_MAX.x - VEC_DUCK_HULL_MIN.x ) / ( VEC_DUCK_HULL_MAX.z - VEC_DUCK_HULL_MIN.z );
			}
			else
			{
				vecTop.z += ent.m_vecMaxs.z;
				vecBottom.z -= ent.m_vecMaxs.z;

				boxWidth = ( VEC_HULL_MAX.x - VEC_HULL_MIN.x ) / ( VEC_HULL_MAX.z - VEC_HULL_MIN.z );
			}
		}

		if ( !UTIL_WorldToScreen( vecBottom, vecScreenBottom ) || !UTIL_WorldToScreen( vecTop, vecScreenTop ) )
			continue;

		float boxHeight = vecScreenBottom[ 1 ] - vecScreenTop[ 1 ];
		boxWidth = boxHeight * boxWidth;

		top_mid_x = vecScreenTop[ 0 ];
		top_mid_y = vecScreenTop[ 1 ];

		bottom_mid_x = vecScreenBottom[ 0 ];
		bottom_mid_y = vecScreenBottom[ 1 ];

		x = vecScreenBottom[ 0 ] - ( boxWidth * 0.5f ); // rotate around the pivot/legs
		y = vecScreenBottom[ 1 ] - boxHeight;
		w = boxWidth;
		h = boxHeight;

		int iHealth = (int)ent.m_flHealth;

		bool bIsEntityFriend = ent.m_bFriend;

		int r = int( 255.f * m_pFriendColor->GetColor()[ 0 ] );
		int g = int( 255.f * m_pFriendColor->GetColor()[ 1 ] );
		int b = int( 255.f * m_pFriendColor->GetColor()[ 2 ] );

		if ( bPlayer && ent.m_bEnemy )
			bIsEntityFriend = false;

		if ( bItem )
		{
			bottom_mid_x = top_mid_x = x = x + w / 2;
			bottom_mid_y = top_mid_y = y = y + h / 2;

			if ( ent.m_bEnemy )
			{
				r = int( 255.f * m_pEnemyColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pEnemyColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pEnemyColor->GetColor()[ 2 ] );
			}
			else if ( ent.m_bNeutral )
			{
				r = int( 255.f * m_pNeutralColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pNeutralColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pNeutralColor->GetColor()[ 2 ] );
			}
			else
			{
				r = int( 255.f * m_pItemColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pItemColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pItemColor->GetColor()[ 2 ] );
			}
		}
		else if ( !bIsEntityFriend )
		{
			r = int( 255.f * m_pEnemyColor->GetColor()[ 0 ] );
			g = int( 255.f * m_pEnemyColor->GetColor()[ 1 ] );
			b = int( 255.f * m_pEnemyColor->GetColor()[ 2 ] );
		}

		if ( m_pSoundESPColor->GetColor()[ 3 ] > 0.f )
		{
			Color clr = m_pSoundESPColor->GetColor32();
			float dt = 1.f - ( ( cl_enginefuncs->GetClientTime() - ent.m_flLastEmitSoundTime ) / m_pSoundESPFadeOut->GetFloat() );
			Features::drawing->FillArea( (int)x, (int)y,
										 (int)w, (int)h,
										 clr.r, clr.g, clr.b,
										 int( (float)clr.a * UTIL_SimpleSpline( dt ) ) );
		}

		if ( bPlayer )
		{
			// Box Fill
			if ( m_pDrawBoxTargets->GetInt() != 1 && w != 0.f && h != 0.f )
			{
				DrawBox( bPlayer, bItem, iHealth, (int)x, (int)y, (int)w, (int)h, r, g, b );
			}

			// Distance
			if ( m_pShowDistance->GetBool() && m_pDrawDistanceTargets->GetInt() != 1 )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												(int)bottom_mid_x, int( bottom_mid_y - 8.f ),
												255, 255, 255, 255,
												FONT_ALIGN_CENTER,
												"%.1f", sqrtf( flDistanceSqr ) );
			}

			// General Info
			if ( m_pDrawPlayerStyle->GetInt() == 0 ) // Default
			{
				DrawPlayerInfo_Default( i, iHealth, bIsEntityFriend, top_mid_x, top_mid_y, bottom_mid_x, bottom_mid_y );
			}
			else if ( m_pDrawPlayerStyle->GetInt() == 1 ) // SAMP
			{
				DrawPlayerInfo_SAMP( i, iHealth, ent.m_bDucked, bIsEntityFriend, vecTop );
			}
			else if ( m_pDrawPlayerStyle->GetInt() == 2 ) // Left 4 Dead
			{
				DrawPlayerInfo_L4D( i, iHealth, ent.m_bDucked, bIsEntityFriend, vecTop );
			}
		}
		else
		{
			// Box Fill
			if ( m_pDrawBoxTargets->GetInt() != 2 )
			{
				DrawBox( bPlayer, bItem, iHealth, (int)x, (int)y, (int)w, (int)h, r, g, b );
			}

			// Distance
			if ( m_pShowDistance->GetBool() && m_pDrawDistanceTargets->GetInt() != 2 )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												(int)bottom_mid_x, int( bottom_mid_y - 8.f ),
												255, 255, 255, 255,
												FONT_ALIGN_CENTER,
												"%.1f", sqrtf( flDistanceSqr ) );
			}

			// General Info
			if ( m_pDrawEntityStyle->GetInt() == 0 ) // Default
			{
				DrawEntityInfo_Default( i, ent.m_classInfo, bottom_mid_x, bottom_mid_y, r, g, b );
			}
			else if ( m_pDrawEntityStyle->GetInt() == 1 ) // SAMP
			{
				DrawEntityInfo_SAMP( i, ent.m_classInfo, vecTop, r, g, b );
			}
			else if ( m_pDrawEntityStyle->GetInt() == 2 ) // Left 4 Dead
			{
				DrawEntityInfo_L4D( i, ent.m_classInfo, vecTop, r, g, b );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Draw it
//-----------------------------------------------------------------------------

void CESP::Draw( void )
{
	static Vector vecAABBPoints[ 8 ];
	static Vector2D vecScreenProj[ 8 ];

	static Vector vecBottom;
	static Vector vecTop;

	float boxWidth = 0.f;
	float vecScreenBottom[ 2 ], vecScreenTop[ 2 ];

	const int width = gameutils->GetScreenWidth();
	const int height = gameutils->GetScreenHeight();

	bool bSpectating = UTIL_IsSpectating();
	int iLocalPlayer = UTIL_GetLocalPlayerIndex();

	cl_entity_s *pLocal = cl_enginefuncs->GetLocalPlayer();
	CEntity *pEnts = Features::entitylist->GetList();

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		if ( iLocalPlayer == i )
			continue;

		cl_entity_t *pEntity = ent.m_pEntity;

		bool bPlayer = ent.m_bPlayer;
		bool bItem = ent.m_bItem;

		if ( bItem && !m_pShowItems->GetBool() )
			continue;

		if ( !ent.m_bVisible )
			continue;

		float flDistanceSqr = ( pEntity->origin - pLocal->origin ).LengthSqr();
		if ( flDistanceSqr > M_SQR( m_pDistance->GetFloat() ) )
			continue;

		float x, y, w, h;
		float top_mid_x, top_mid_y, bottom_mid_x, bottom_mid_y;

		vecBottom = pEntity->origin;
		vecTop = pEntity->origin;

		if ( !bPlayer )
		{
			if ( ent.m_classInfo.id == CLASS_NONE &&
				 m_pIgnoreUnknownEnts->GetBool() ||
				 ent.m_classInfo.id == CLASS_DEAD_PLAYER )
				continue;

			// Don't process if entity isn't an ESP's target
			if ( m_pDrawTargets->GetInt() == 2 )
				continue;

			if ( !bItem )
			{
				vecTop.z += ent.m_vecMaxs.z;
				vecBottom.z -= ent.m_vecMins.z;

				float height = ( ent.m_vecMaxs.z - ent.m_vecMins.z );

				boxWidth = ( height != 0.f ) ? ( Q_max( ent.m_vecMaxs.x - ent.m_vecMins.x, ent.m_vecMaxs.y - ent.m_vecMins.y ) / height ) : 0.f;
			}
		}
		else
		{
			if ( m_pDrawTargets->GetInt() == 1 )
				continue;

			if ( ent.m_bDucked )
			{
				vecTop.z += VEC_DUCK_HULL_MAX.z;
				vecBottom.z += VEC_DUCK_HULL_MIN.z;

				boxWidth = ( VEC_DUCK_HULL_MAX.x - VEC_DUCK_HULL_MIN.x ) / ( VEC_DUCK_HULL_MAX.z - VEC_DUCK_HULL_MIN.z );
			}
			else
			{
				vecTop.z += ent.m_vecMaxs.z;
				vecBottom.z -= ent.m_vecMaxs.z;

				boxWidth = ( VEC_HULL_MAX.x - VEC_HULL_MIN.x ) / ( VEC_HULL_MAX.z - VEC_HULL_MIN.z );
			}
		}

		if ( !UTIL_WorldToScreen( vecBottom, vecScreenBottom ) || !UTIL_WorldToScreen( vecTop, vecScreenTop ) )
			continue;

		float boxHeight = vecScreenBottom[ 1 ] - vecScreenTop[ 1 ];
		boxWidth = boxHeight * boxWidth;

		top_mid_x = vecScreenTop[ 0 ];
		top_mid_y = vecScreenTop[ 1 ];

		bottom_mid_x = vecScreenBottom[ 0 ];
		bottom_mid_y = vecScreenBottom[ 1 ];

		//x = vecScreenTop[0] - (boxWidth * 0.5f); // rotate around the head
		//y = vecScreenTop[1];
		x = vecScreenBottom[ 0 ] - ( boxWidth * 0.5f ); // rotate around the pivot/legs
		y = vecScreenBottom[ 1 ] - boxHeight;
		w = boxWidth;
		h = boxHeight;

		if ( !m_pOptimize->GetBool() && !bItem )
		{
			Vector vecMins = pEntity->origin + ent.m_vecMins;
			Vector vecMaxs = pEntity->origin + ent.m_vecMaxs;

			vecAABBPoints[ 0 ].x = vecMins.x;
			vecAABBPoints[ 0 ].y = vecMins.y;
			vecAABBPoints[ 0 ].z = vecMins.z;

			vecAABBPoints[ 1 ].x = vecMins.x;
			vecAABBPoints[ 1 ].y = vecMaxs.y;
			vecAABBPoints[ 1 ].z = vecMins.z;

			vecAABBPoints[ 2 ].x = vecMaxs.x;
			vecAABBPoints[ 2 ].y = vecMaxs.y;
			vecAABBPoints[ 2 ].z = vecMins.z;

			vecAABBPoints[ 3 ].x = vecMaxs.x;
			vecAABBPoints[ 3 ].y = vecMins.y;
			vecAABBPoints[ 3 ].z = vecMins.z;

			vecAABBPoints[ 4 ].x = vecMaxs.x;
			vecAABBPoints[ 4 ].y = vecMaxs.y;
			vecAABBPoints[ 4 ].z = vecMaxs.z;

			vecAABBPoints[ 5 ].x = vecMins.x;
			vecAABBPoints[ 5 ].y = vecMaxs.y;
			vecAABBPoints[ 5 ].z = vecMaxs.z;

			vecAABBPoints[ 6 ].x = vecMins.x;
			vecAABBPoints[ 6 ].y = vecMins.y;
			vecAABBPoints[ 6 ].z = vecMaxs.z;

			vecAABBPoints[ 7 ].x = vecMaxs.x;
			vecAABBPoints[ 7 ].y = vecMins.y;
			vecAABBPoints[ 7 ].z = vecMaxs.z;

			if ( !UTIL_WorldToScreen( vecAABBPoints[ 3 ], vecScreenProj[ 0 ] ) || !UTIL_WorldToScreen( vecAABBPoints[ 5 ], vecScreenProj[ 1 ] )
				 || !UTIL_WorldToScreen( vecAABBPoints[ 0 ], vecScreenProj[ 2 ] ) || !UTIL_WorldToScreen( vecAABBPoints[ 4 ], vecScreenProj[ 3 ] )
				 || !UTIL_WorldToScreen( vecAABBPoints[ 2 ], vecScreenProj[ 4 ] ) || !UTIL_WorldToScreen( vecAABBPoints[ 1 ], vecScreenProj[ 5 ] )
				 || !UTIL_WorldToScreen( vecAABBPoints[ 6 ], vecScreenProj[ 6 ] ) || !UTIL_WorldToScreen( vecAABBPoints[ 7 ], vecScreenProj[ 7 ] ) )
			{
				continue;
			}

			float left = vecScreenProj[ 0 ].x;
			float top = vecScreenProj[ 0 ].y;
			float right = vecScreenProj[ 0 ].x;
			float bottom = vecScreenProj[ 0 ].y;

			for ( int i = 1; i < 8; i++ )
			{
				if ( left > vecScreenProj[ i ].x )
					left = vecScreenProj[ i ].x;

				if ( bottom < vecScreenProj[ i ].y )
					bottom = vecScreenProj[ i ].y;

				if ( right < vecScreenProj[ i ].x )
					right = vecScreenProj[ i ].x;

				if ( top > vecScreenProj[ i ].y )
					top = vecScreenProj[ i ].y;
			}

			x = left;
			y = top;
			w = right - left;
			h = bottom - top;

			top_mid_y = y;
			bottom_mid_y = y + h;
		}

		if ( bPlayer && m_pShowVisiblePlayers->GetBool() )
		{
			pmtrace_t trace;

			Vector vecStart = localplayer->GetEyePosition();
			Vector vecEnd = vecBottom + ( vecTop - vecBottom ) * 0.5f;

			cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
			cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecStart, vecEnd, PM_WORLD_ONLY, -1, &trace );

			if ( trace.fraction != 1.f )
			{
				continue;
			}
		}

		int iHealth = (int)ent.m_flHealth;

		bool bIsEntityFriend = ent.m_bFriend;
		bool bIsEntityNeutral = ent.m_bNeutral;

		int r = int( 255.f * m_pFriendColor->GetColor()[ 0 ] );
		int g = int( 255.f * m_pFriendColor->GetColor()[ 1 ] );
		int b = int( 255.f * m_pFriendColor->GetColor()[ 2 ] );

		if ( bPlayer && ent.m_bEnemy )
			bIsEntityFriend = false;

		if ( bItem )
		{
			bottom_mid_x = top_mid_x = x = x + w / 2;
			bottom_mid_y = top_mid_y = y = y + h / 2;

			if ( ent.m_bEnemy )
			{
				r = int( 255.f * m_pEnemyColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pEnemyColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pEnemyColor->GetColor()[ 2 ] );
			}
			else if ( ent.m_bNeutral )
			{
				r = int( 255.f * m_pNeutralColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pNeutralColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pNeutralColor->GetColor()[ 2 ] );
			}
			else
			{
				r = int( 255.f * m_pItemColor->GetColor()[ 0 ] );
				g = int( 255.f * m_pItemColor->GetColor()[ 1 ] );
				b = int( 255.f * m_pItemColor->GetColor()[ 2 ] );
			}
		}
		else if ( bIsEntityNeutral )
		{
			r = int( 255.f * m_pNeutralColor->GetColor()[ 0 ] );
			g = int( 255.f * m_pNeutralColor->GetColor()[ 1 ] );
			b = int( 255.f * m_pNeutralColor->GetColor()[ 2 ] );
		}
		else if ( !bIsEntityFriend )
		{
			r = int( 255.f * m_pEnemyColor->GetColor()[ 0 ] );
			g = int( 255.f * m_pEnemyColor->GetColor()[ 1 ] );
			b = int( 255.f * m_pEnemyColor->GetColor()[ 2 ] );
		}

		if ( m_pDebug->GetBool() )
		{
			if ( !ent.m_bItem && ( ( bPlayer && ( iHealth > 0 || iHealth < -1 ) ) || !bPlayer ) )
			{
				for ( int j = 0; j < ent.m_pStudioHeader->numhitboxes; j++ )
				{
					float vScreen[ 2 ];

					if ( !UTIL_WorldToScreen( ent.m_rgHitboxes[ j ], vScreen ) )
						continue;

					Features::drawing->DrawStringF( Features::drawing->GetFontESP(), (int)vScreen[ 0 ], (int)vScreen[ 1 ], 255, 255, 255, 255, FONT_ALIGN_CENTER, "%d", j );
				}
			}

			Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
											(int)bottom_mid_x, int( bottom_mid_y + 20.f ),
											255, 255, 255, 255,
											FONT_ALIGN_CENTER,
											"Solid: %d | Movetype: %d", pEntity->curstate.solid, pEntity->curstate.movetype );
			
			Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
											(int)bottom_mid_x, int( bottom_mid_y + 30.f ),
											255, 255, 255, 255,
											FONT_ALIGN_CENTER,
											"Sequence: %d", pEntity->curstate.sequence );
		}

		if ( m_pSnapLines->GetBool() && !bSpectating )
		{
			Features::drawing->DrawLine( width / 2, height, (int)bottom_mid_x, (int)bottom_mid_y, r, g, b, 255 );
		}

		if ( bPlayer )
		{
			// Box Fill
			if ( m_pDrawBoxTargets->GetInt() != 1 && w != 0.f && h != 0.f )
			{
				DrawBox( bPlayer, bItem, iHealth, (int)x, (int)y, (int)w, (int)h, r, g, b );
			}

			// Distance
			if ( m_pShowDistance->GetBool() && m_pDrawDistanceTargets->GetInt() != 1 )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												(int)bottom_mid_x, int( bottom_mid_y - 8.f ),
												255, 255, 255, 255,
												FONT_ALIGN_CENTER,
												"%.1f", sqrtf( flDistanceSqr ) );
			}

			// General Info
			if ( m_pDrawPlayerStyle->GetInt() == 0 ) // Default
			{
				DrawPlayerInfo_Default( i, iHealth, bIsEntityFriend, top_mid_x, top_mid_y, bottom_mid_x, bottom_mid_y );
			}
			else if ( m_pDrawPlayerStyle->GetInt() == 1 ) // SAMP
			{
				DrawPlayerInfo_SAMP( i, iHealth, (bool)pEntity->curstate.usehull, bIsEntityFriend, vecTop );
			}
			else if ( m_pDrawPlayerStyle->GetInt() == 2 ) // Left 4 Dead
			{
				DrawPlayerInfo_L4D( i, iHealth, (bool)pEntity->curstate.usehull, bIsEntityFriend, vecTop );
			}

			if ( m_pDrawSkeletonTargets->GetInt() == 1 )
				continue;
		}
		else
		{
			// Box Fill
			if ( m_pDrawBoxTargets->GetInt() != 2 )
			{
				DrawBox( bPlayer, bItem, iHealth, (int)x, (int)y, (int)w, (int)h, r, g, b );
			}

			// Distance
			if ( m_pShowDistance->GetBool() && m_pDrawDistanceTargets->GetInt() != 2 )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												(int)bottom_mid_x, int( bottom_mid_y - 8.f ),
												255, 255, 255, 255,
												FONT_ALIGN_CENTER,
												"%.1f", sqrtf( flDistanceSqr ) );
			}

			// General Info
			if ( m_pDrawEntityStyle->GetInt() == 0 ) // Default
			{
				DrawEntityInfo_Default( i, ent.m_classInfo, bottom_mid_x, bottom_mid_y, r, g, b );
			}
			else if ( m_pDrawEntityStyle->GetInt() == 1 ) // SAMP
			{
				DrawEntityInfo_SAMP( i, ent.m_classInfo, vecTop, r, g, b );
			}
			else if ( m_pDrawEntityStyle->GetInt() == 2 ) // Left 4 Dead
			{
				DrawEntityInfo_L4D( i, ent.m_classInfo, vecTop, r, g, b );
			}

			if ( m_pDrawSkeletonTargets->GetInt() == 2 )
				continue;
		}

		if ( bItem )
			continue;

		if ( bPlayer && ( iHealth == 0 || iHealth == -1 ) )
			continue;

		DrawBones( i, ent.m_pStudioHeader );
	}
}

//-----------------------------------------------------------------------------
// Default draw style
//-----------------------------------------------------------------------------

void CESP::DrawPlayerInfo_Default( int index, int iHealth, bool bIsEntityFriend, float top_mid_x, float top_mid_y, float bottom_mid_x, float bottom_mid_y )
{
	if ( m_pShowPlayerHealth->GetBool() && iHealth != 0 )
	{
		int r, g, b;

		int iActualHealth = iHealth;

		if ( iHealth == -1 )
			iActualHealth = iHealth = 0;
		else if ( iHealth > 100 )
			iHealth = 100;

		if ( bIsEntityFriend )
		{
			r = int( 255.f * ( iHealth > 50 ? 1.f - 2.f * ( iHealth - 50 ) / 100.f : 1.f ) );
			g = int( 255.f * ( ( iHealth > 50 ? 1.f : 2.f * iHealth / 100.f ) ) );
			b = 0;
		}
		else
		{
			iActualHealth = -1;

			r = 0;
			g = 255;
			b = 255;
		}

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
										(int)top_mid_x,
										int( top_mid_y - 8.f ),
										r,
										g,
										b,
										255,
										FONT_ALIGN_CENTER,
										"%d",
										iActualHealth );
	}

	if ( m_pShowPlayerArmor->GetBool() )
	{
		float flArmor = extraplayerinfo->GetArmor( index );

		if ( flArmor > 0.f )
			Features::drawing->DrawStringF( Features::drawing->GetFontESP(), (int)top_mid_x, int( top_mid_y + 8.f ), 153, 191, 255, 255, FONT_ALIGN_CENTER, "%.1f", flArmor );
	}

	if ( m_pShowPlayerName->GetBool() || m_pShowEntityIndex->GetBool() )
	{
		static char szIndex[ 16 ];
		player_info_t *pPlayer = NULL;

		if ( m_pShowEntityIndex->GetBool() )
			snprintf( szIndex, sizeof( szIndex ), m_pShowPlayerName->GetBool() ? " (%d)" : "(%d)", index );

		int nickname_r, nickname_g, nickname_b;

		if ( bIsEntityFriend )
		{
			nickname_r = int( 255.f * m_pFriendPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pFriendPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pFriendPlayerColor->GetColor()[ 2 ] );
		}
		else
		{
			nickname_r = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 2 ] );
		}

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
										(int)bottom_mid_x, int( bottom_mid_y + 8.f ),
										nickname_r, nickname_g, nickname_b, 255,
										FONT_ALIGN_CENTER,
										"%s%s",
										m_pShowPlayerName->GetBool() ? ( pPlayer = enginestudio->PlayerInfo( index - 1 ), pPlayer->name ) : "",
										m_pShowEntityIndex->GetBool() ? szIndex : "" );
	}
}

//-----------------------------------------------------------------------------
// Draw player info in SAMP style :)
//-----------------------------------------------------------------------------

void CESP::DrawPlayerInfo_SAMP( int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop )
{
	constexpr int iBarWidth = 42;
	constexpr int iBarHeight = 4;
	constexpr int iThickness = 1;

	float vecScreen[ 2 ];

	if ( bDucking )
		vecTop.z += 12.f;
	else
		vecTop.z += 6.f;

	if ( !UTIL_WorldToScreen( vecTop, vecScreen ) )
		return;

	int offset_x = int( vecScreen[ 0 ] );
	int offset_y = int( vecScreen[ 1 ] );

	int health_offset_y = offset_y;

	if ( m_pShowPlayerArmor->GetBool() )
	{
		float flArmor = extraplayerinfo->GetArmor( index );

		if ( flArmor > 0.f )
		{
			if ( flArmor > 100.f )
				flArmor = 100.f;

			float flFraction = flArmor / 100.f;

			// Thickness
			{
				constexpr int iWidth = iBarWidth + 2 * iThickness;
				constexpr int iHeight = iBarHeight + 2 * iThickness;

				Features::drawing->FillArea( offset_x - iWidth / 2, offset_y - iHeight / 2, iWidth, iHeight, 0, 0, 0, 255 );
			}

			if ( flArmor != 100.f )
			{
				Features::drawing->FillArea( offset_x - iBarWidth / 2, offset_y - iBarHeight / 2, iBarWidth, iBarHeight, 40, 40, 40, 255 );
			}

			Features::drawing->FillArea( offset_x - iBarWidth / 2, offset_y - iBarHeight / 2, int( (float)iBarWidth * flFraction ), iBarHeight, 200, 200, 200, 255 );

			health_offset_y += ( ( iBarHeight + 2 * iThickness ) / 2 ) + 5;
		}
	}

	if ( m_pShowPlayerHealth->GetBool() )
	{
		if ( iHealth < 0 )
			iHealth = 0;
		else if ( iHealth > 100 )
			iHealth = 100;

		int y = health_offset_y;
		float flFraction = (float)iHealth / 100.f;

		// Thickness
		{
			constexpr int iWidth = iBarWidth + 2 * iThickness;
			constexpr int iHeight = iBarHeight + 2 * iThickness;

			Features::drawing->FillArea( offset_x - iWidth / 2, y - iHeight / 2, iWidth, iHeight, 0, 0, 0, 255 );
		}

		if ( iHealth != 100 )
		{
			Features::drawing->FillArea( offset_x - iBarWidth / 2, y - iBarHeight / 2, iBarWidth, iBarHeight, 76, 11, 20, 255 );
		}

		Features::drawing->FillArea( offset_x - iBarWidth / 2, y - iBarHeight / 2, int( (float)iBarWidth * flFraction ), iBarHeight, 187, 32, 40, 255 );
	}

	if ( m_pShowPlayerName->GetBool() || m_pShowEntityIndex->GetBool() )
	{
		static char szIndex[ 16 ];
		player_info_t *pPlayer = NULL;

		if ( m_pShowEntityIndex->GetBool() )
			snprintf( szIndex, sizeof( szIndex ), m_pShowPlayerName->GetBool() ? " (%d)" : "(%d)", index );

		int nickname_r, nickname_g, nickname_b;
		int y = offset_y - ( ( iBarHeight + 2 * iThickness ) / 2 ) - 14;

		if ( bIsEntityFriend )
		{
			nickname_r = int( 255.f * m_pFriendPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pFriendPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pFriendPlayerColor->GetColor()[ 2 ] );
		}
		else
		{
			nickname_r = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 2 ] );
		}

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(), offset_x, y, nickname_r, nickname_g, nickname_b, 255, FONT_ALIGN_CENTER, "%s%s",
										m_pShowPlayerName->GetBool() ? ( pPlayer = enginestudio->PlayerInfo( index - 1 ), pPlayer->name ) : "",
										m_pShowEntityIndex->GetBool() ? szIndex : "" );
	}
}

//-----------------------------------------------------------------------------
// Draw player info in L4D style
//-----------------------------------------------------------------------------

void CESP::DrawPlayerInfo_L4D( int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop )
{
	float vecScreen[ 2 ];

	if ( bDucking )
		vecTop.z += 24.f;
	else
		vecTop.z += 14.f;

	if ( !UTIL_WorldToScreen( vecTop, vecScreen ) )
		return;

	int offset_x = int( vecScreen[ 0 ] );
	int offset_y = int( vecScreen[ 1 ] + 25.f );

	if ( m_pShowPlayerName->GetBool() || m_pShowPlayerHealth->GetBool() || m_pShowPlayerArmor->GetBool() )
	{
		const char *szFormatString;
		static char szInfo[ 16 ];

		player_info_t *pPlayer;

		szFormatString = m_pShowPlayerName->GetBool() ? " (%d)" : "(%d)";

		if ( m_pShowPlayerHealth->GetBool() && m_pShowPlayerArmor->GetBool() )
		{
			if ( iHealth < 0 )
				iHealth = 0;
			else if ( iHealth > 100 )
				iHealth = 100;

			float flArmor = extraplayerinfo->GetArmor( index );

			if ( flArmor > 0.f )
				snprintf( szInfo, sizeof( szInfo ), m_pShowPlayerName->GetBool() ? " (%d) [%.1f]" : "(%d) [%.1f]", iHealth, flArmor );
			else
				snprintf( szInfo, sizeof( szInfo ), m_pShowPlayerName->GetBool() ? " (%d)" : "(%d)", iHealth );
		}
		else if ( m_pShowPlayerHealth->GetBool() )
		{
			if ( iHealth < 0 )
				iHealth = 0;
			else if ( iHealth > 100 )
				iHealth = 100;

			snprintf( szInfo, sizeof( szInfo ), m_pShowPlayerName->GetBool() ? " (%d)" : "(%d)", iHealth );
		}
		else if ( m_pShowPlayerArmor->GetBool() )
		{
			float flArmor = extraplayerinfo->GetArmor( index );

			if ( flArmor > 0.f )
				snprintf( szInfo, sizeof( szInfo ), m_pShowPlayerName->GetBool() ? " [%.1f]" : "[%.1f]", flArmor );
			else
				szInfo[ 0 ] = '\0';
		}
		else
		{
			szInfo[ 0 ] = '\0';
		}

		int nickname_r, nickname_g, nickname_b;

		int x = (int)vecScreen[ 0 ];
		int y = (int)vecScreen[ 1 ];

		if ( bIsEntityFriend )
		{
			nickname_r = int( 255.f * m_pFriendPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pFriendPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pFriendPlayerColor->GetColor()[ 2 ] );
		}
		else
		{
			nickname_r = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 0 ] );
			nickname_g = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 1 ] );
			nickname_b = int( 255.f * m_pEnemyPlayerColor->GetColor()[ 2 ] );
		}

		Features::drawing->DrawStringF( Features::drawing->GetFontESP2(),
										x, y,
										nickname_r, nickname_g, nickname_b, 255,
										FONT_ALIGN_CENTER,
										"%s%s",
										m_pShowPlayerName->GetBool() ? ( pPlayer = enginestudio->PlayerInfo( index - 1 ), pPlayer->name ) : "",
										szInfo );
	}
}

//-----------------------------------------------------------------------------
// Default draw style
//-----------------------------------------------------------------------------

void CESP::DrawEntityInfo_Default( int index, class_info_t classInfo, float bottom_mid_x, float bottom_mid_y, int r, int g, int b )
{
	if ( m_pShowEntityName->GetBool() || m_pShowEntityIndex->GetBool() )
	{
		static char szIndex[ 16 ];

		if ( m_pShowEntityIndex->GetBool() )
			snprintf( szIndex, sizeof( szIndex ), m_pShowEntityName->GetBool() ? " (%d)" : "(%d)", index );

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
										(int)bottom_mid_x, int( bottom_mid_y + 8.f ),
										r, g, b, 255,
										FONT_ALIGN_CENTER,
										"%s%s",
										m_pShowEntityName->GetBool() ? Features::entitylist->GetEntityClassname( classInfo ) : "",
										m_pShowEntityIndex->GetBool() ? szIndex : "" );
	}
}

//-----------------------------------------------------------------------------
// Draw entity info in SAMP style
//-----------------------------------------------------------------------------

void CESP::DrawEntityInfo_SAMP( int index, class_info_t classInfo, Vector vecTop, int r, int g, int b )
{
	if ( m_pShowEntityName->GetBool() || m_pShowEntityIndex->GetBool() )
	{
		float vecScreen[ 2 ];
		vecTop.z += 6.f;

		UTIL_WorldToScreen( vecTop, vecScreen );

		int x = int( vecScreen[ 0 ] );
		int y = int( vecScreen[ 1 ] );

		static char szIndex[ 16 ];

		if ( m_pShowEntityIndex->GetBool() )
			snprintf( szIndex, sizeof( szIndex ), m_pShowEntityName->GetBool() ? " (%d)" : "(%d)", index );

		Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
										x, y,
										r, g, b, 255,
										FONT_ALIGN_CENTER,
										"%s%s",
										m_pShowEntityName->GetBool() ? Features::entitylist->GetEntityClassname( classInfo ) : "",
										m_pShowEntityIndex->GetBool() ? szIndex : "" );
	}
}

//-----------------------------------------------------------------------------
// Draw entity info in L4D style
//-----------------------------------------------------------------------------

void CESP::DrawEntityInfo_L4D( int index, class_info_t classInfo, Vector vecTop, int r, int g, int b )
{
	if ( m_pShowEntityName->GetBool() || m_pShowEntityIndex->GetBool() )
	{
		float vecScreen[ 2 ];
		vecTop.z += 14.f;

		UTIL_WorldToScreen( vecTop, vecScreen );

		int x = int( vecScreen[ 0 ] );
		int y = int( vecScreen[ 1 ] );

		static char szIndex[ 16 ];

		if ( m_pShowEntityIndex->GetBool() )
			snprintf( szIndex, sizeof( szIndex ), m_pShowEntityName->GetBool() ? " (%d)" : "(%d)", index );

		Features::drawing->DrawStringF( Features::drawing->GetFontESP2(),
										x, y,
										r, g, b,
										255,
										FONT_ALIGN_CENTER,
										"%s%s",
										m_pShowEntityName->GetBool() ? Features::entitylist->GetEntityClassname( classInfo ) : "",
										m_pShowEntityIndex->GetBool() ? szIndex : "" );
	}
}

//-----------------------------------------------------------------------------
// Draw custom box
//-----------------------------------------------------------------------------

void CESP::DrawBox( bool bPlayer, bool bItem, int iHealth, int x, int y, int w, int h, int r, int g, int b )
{
	if ( ( bPlayer && ( iHealth > 0 || iHealth < -1 ) ) || ( !bPlayer && !bItem ) )
	{
		int nBox = m_pDrawBoxType->GetInt();
		bool bOutline = m_pOutlineBox->GetBool();

		if ( m_pBoxFillAlpha->GetInt() != 0 )
		{
			Features::drawing->FillArea( x, y, w, h, r, g, b, m_pBoxFillAlpha->GetInt() );
		}

		if ( nBox == 1 )
		{
			Features::drawing->Box( x, y, w, h, 1, r, g, b, 200 );

			if ( bOutline )
				Features::drawing->BoxOutline( (float)x, (float)y, (float)w, (float)h, 1, r, g, b, 200 );
		}
		else if ( nBox == 2 )
		{
			Features::drawing->DrawCoalBox( x, y, w, h, 1, r, g, b, 255 );

			if ( bOutline )
				Features::drawing->DrawOutlineCoalBox( x, y, w, h, 1, r, g, b, 255 );
		}
		else if ( nBox == 3 )
		{
			Features::drawing->BoxCorner( x, y, w, h, 1, r, g, b, 255 );

			if ( bOutline )
				Features::drawing->BoxCornerOutline( x, y, w, h, 1, r, g, b, 255 );
		}
	}
}

//-----------------------------------------------------------------------------
// Draw bones
//-----------------------------------------------------------------------------

void CESP::DrawBones( int index, studiohdr_t *pStudioHeader )
{
#ifdef PROCESS_PLAYER_BONES_ONLY
	if ( !m_pShowSkeleton->GetBool() && !m_pShowBonesName->GetBool() || !bPlayer )
#else
	if ( !m_pShowSkeleton->GetBool() && !m_pShowBonesName->GetBool() )
#endif
		return;

#pragma warning(push)
#pragma warning(disable : 6011)

	mstudiobone_t *pBone = (mstudiobone_t *)( (byte *)pStudioHeader + pStudioHeader->boneindex );

	for ( int j = 0; j < pStudioHeader->numbones; ++j )
	{
		bool bBonePoint = false;
		float vBonePoint[ 2 ];

		if ( ( bBonePoint = UTIL_WorldToScreen( gBones[ index ].vecPoint[ j ], vBonePoint ) ) && m_pShowBonesName->GetBool() )
		{
			Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
											int( vBonePoint[ 0 ] ), int( vBonePoint[ 1 ] ),
											255, 255, 255, 255,
											FONT_ALIGN_CENTER,
											"%s", pBone[ j ].name );
		}

		if ( !m_pShowSkeleton->GetBool() )
			continue;

		float vParentPoint[ 2 ];

		if ( !bBonePoint || gBones[ index ].nParent[ j ] == -1 )
			continue;

		if ( !UTIL_WorldToScreen( gBones[ index ].vecPoint[ gBones[ index ].nParent[ j ] ], vParentPoint ) )
			continue;

		Features::drawing->DrawLine( int( vBonePoint[ 0 ] ),
										int( vBonePoint[ 1 ] ),
										int( vParentPoint[ 0 ] ),
										int( vParentPoint[ 1 ] ),
										255, 255, 255, 255 );
	}

#pragma warning(pop)
}

//-----------------------------------------------------------------------------
// Store bones
//-----------------------------------------------------------------------------

void CESP::UpdateBones( int index )
{
	if ( !m_pShowSkeleton->GetBool() && !m_pShowBonesName->GetBool() )
		return;

	cl_entity_s *pLocal = cl_enginefuncs->GetLocalPlayer();
	if ( pLocal == NULL )
		return;

	cl_entity_s *pEntity = enginestudio->GetCurrentEntity();

#ifdef PROCESS_PLAYER_BONES_ONLY
	if ( !pEntity->player )
		return;
#endif

	CEntity &ent = Features::entitylist->GetList()[ index ];

	if ( index == pLocal->index )
		return;

	if ( !ent.m_bValid )
		return;

	bool bPlayer = ent.m_bPlayer;
	bool bItem = ent.m_bItem;

	if ( !ent.m_bVisible )
		return;

	if ( !bPlayer &&
		 ent.m_classInfo.id == CLASS_NONE &&
		 m_pIgnoreUnknownEnts->GetBool() ||
		 ent.m_classInfo.id == CLASS_DEAD_PLAYER )
		return;

	Vector vecBottom = pEntity->origin;
	Vector vecTop = pEntity->origin;

	float vecScreenBottom[ 2 ], vecScreenTop[ 2 ];

	if ( !bPlayer )
	{
		if ( !bItem )
		{
			vecTop.z += pEntity->curstate.maxs.z;
			vecBottom.z -= pEntity->curstate.mins.z;
		}
	}
	else
	{
		if ( ent.m_bDucked )
		{
			vecTop.z += VEC_DUCK_HULL_MAX.z;
			vecBottom.z += VEC_DUCK_HULL_MIN.z;
		}
		else
		{
			vecTop.z += pEntity->curstate.maxs.z;
			vecBottom.z -= pEntity->curstate.maxs.z;
		}
	}

	if ( !UTIL_WorldToScreen( vecBottom, vecScreenBottom ) || !UTIL_WorldToScreen( vecTop, vecScreenTop ) )
		return;

	bone_matrix3x4_t *pBoneTransform = Features::entitylist->GetBoneTransform();
	mstudiobone_t *pBone = (mstudiobone_t *)( (byte *)ent.m_pStudioHeader + ent.m_pStudioHeader->boneindex );

	for ( int i = 0; i < ent.m_pStudioHeader->numbones; ++i )
	{
		Vector vecBone( ( *pBoneTransform )[ i ][ 0 ][ 3 ], ( *pBoneTransform )[ i ][ 1 ][ 3 ], ( *pBoneTransform )[ i ][ 2 ][ 3 ] );

		//vecBone += ent.m_vecVelocity;

		gBones[ index ].vecPoint[ i ] = vecBone;
		gBones[ index ].nParent[ i ] = pBone[ i ].parent;
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CESP::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kVGuiClientPanelPaint_HookEvent )
	{
		if ( !Features::camhack->IsEnabled() )
		{
			Draw();
			DrawSoundESP();
		}
	}
	// kStudioRenderModel event
	else if ( ( studiorenderer->m_pPlayerInfo != NULL ||
			  studiorenderer->m_pPlayerInfo == NULL && !studiorenderer->m_pCurrentEntity->player ) &&
			  studiorenderer->m_pCurrentEntity->index <= MY_MAXENTS )
	{
		UpdateBones( studiorenderer->m_pCurrentEntity->index );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Press menu button event
//-----------------------------------------------------------------------------

void CESP::OnButtonPressed( CMenuElementButton *pButton )
{
	if ( pButton == m_pPlayerStyleDefault )
	{
		*m_pShowPlayerHealth->GetCfgProperty() = true;
		*m_pShowPlayerArmor->GetCfgProperty() = true;
		*m_pShowVisiblePlayers->GetCfgProperty() = false;
		*m_pShowEntityIndex->GetCfgProperty() = false;

		*m_pDrawPlayerStyle->GetCfgProperty() = 0;
		*m_pDrawDistanceTargets->GetCfgProperty() = 0;

		m_pFriendPlayerColor->GetColor()[ 0 ] = 0.f;
		m_pFriendPlayerColor->GetColor()[ 1 ] = 1.f;
		m_pFriendPlayerColor->GetColor()[ 2 ] = 0.f;
	}
	else if ( pButton == m_pPlayerStyleSAMP )
	{
		*m_pShowPlayerHealth->GetCfgProperty() = true;
		*m_pShowPlayerArmor->GetCfgProperty() = true;
		*m_pShowVisiblePlayers->GetCfgProperty() = true;
		*m_pShowEntityIndex->GetCfgProperty() = true;

		*m_pDrawPlayerStyle->GetCfgProperty() = 1;
		*m_pDrawDistanceTargets->GetCfgProperty() = 1;

		m_pFriendPlayerColor->GetColor()[ 0 ] = 1.f;
		m_pFriendPlayerColor->GetColor()[ 1 ] = 1.f;
		m_pFriendPlayerColor->GetColor()[ 2 ] = 1.f;
	}
	else if ( pButton == m_pPlayerStyleL4D )
	{
		*m_pShowPlayerHealth->GetCfgProperty() = false;
		*m_pShowPlayerArmor->GetCfgProperty() = false;
		*m_pShowVisiblePlayers->GetCfgProperty() = false;
		*m_pShowEntityIndex->GetCfgProperty() = false;

		*m_pDrawPlayerStyle->GetCfgProperty() = 2;
		*m_pDrawDistanceTargets->GetCfgProperty() = 1;

		m_pFriendPlayerColor->GetColor()[ 0 ] = 0.6f;
		m_pFriendPlayerColor->GetColor()[ 1 ] = 0.75f;
		m_pFriendPlayerColor->GetColor()[ 2 ] = 1.f;
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CESP::CESP( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pPlayerStyleDefault = NULL;
	m_pPlayerStyleSAMP = NULL;
	m_pPlayerStyleL4D = NULL;

	m_pDebug = NULL;
	m_pOptimize = NULL;
	m_pSnapLines = NULL;
	m_pOutlineBox = NULL;

	m_pShowEntityIndex = NULL;
	m_pShowPlayerHealth = NULL;
	m_pShowPlayerArmor = NULL;
	m_pShowDistance = NULL;
	m_pShowEntityName = NULL;
	m_pShowPlayerName = NULL;
	m_pShowItems = NULL;
	m_pShowVisiblePlayers = NULL;
	m_pShowSkeleton = NULL;
	m_pShowBonesName = NULL;
	m_pIgnoreUnknownEnts = NULL;

	m_pSoundESP = NULL;
	m_pSoundESPFadeOut = NULL;
	m_pSoundESPColor = NULL;

	m_pDistance = NULL;
	m_pBoxFillAlpha = NULL;

	m_pDrawBoxType = NULL;
	m_pDrawPlayerStyle = NULL;
	m_pDrawEntityStyle = NULL;
	m_pDrawTargets = NULL;
	m_pDrawBoxTargets = NULL;
	m_pDrawDistanceTargets = NULL;
	m_pDrawSkeletonTargets = NULL;

	m_pFriendPlayerColor = NULL;
	m_pEnemyPlayerColor = NULL;
	m_pFriendColor = NULL;
	m_pEnemyColor = NULL;
	m_pItemColor = NULL;
	m_pNeutralColor = NULL;

	m_pfnCClient_SoundEngine__PlayFMODSound = NULL;
	m_hCClient_SoundEngine__PlayFMODSound = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CESP::OnEnable( void )
{
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CESP::OnDisable( void )
{
	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CESP::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pPlayerStyleDefault = Modules::menu->AddElementButton( this, this, "Player Style: Default" ); Modules::menu->AddElementSameLine( this );
	m_pPlayerStyleSAMP = Modules::menu->AddElementButton( this, this, "Player Style: SAMP" ); Modules::menu->AddElementSameLine( this );
	m_pPlayerStyleL4D = Modules::menu->AddElementButton( this, this, "Player Style: L4D" );

	Modules::menu->AddElementSeparator( this );

	m_pDebug = Modules::menu->AddParamBool( this, "Debug", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pOptimize = Modules::menu->AddParamBool( this, "Optimize", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pSnapLines = Modules::menu->AddParamBool( this, "SnapLines", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pOutlineBox = Modules::menu->AddParamBool( this, "OutlineBox", NULL, true );

	Modules::menu->AddElementSeparator( this );

	m_pShowEntityIndex = Modules::menu->AddParamBool( this, "ShowEntityIndex", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowPlayerHealth = Modules::menu->AddParamBool( this, "ShowPlayerHealth", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowPlayerArmor = Modules::menu->AddParamBool( this, "ShowPlayerArmor", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowDistance = Modules::menu->AddParamBool( this, "ShowDistance", NULL, true );
	m_pShowEntityName = Modules::menu->AddParamBool( this, "ShowEntityName", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowPlayerName = Modules::menu->AddParamBool( this, "ShowPlayerName", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowItems = Modules::menu->AddParamBool( this, "ShowItems", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowVisiblePlayers = Modules::menu->AddParamBool( this, "ShowVisiblePlayers", NULL, true );
	m_pShowSkeleton = Modules::menu->AddParamBool( this, "ShowSkeleton", NULL, true ); Modules::menu->AddElementSameLine( this );
	m_pShowBonesName = Modules::menu->AddParamBool( this, "ShowBonesName", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pIgnoreUnknownEnts = Modules::menu->AddParamBool( this, "IgnoreUnknownEnts", NULL, false );

	Modules::menu->AddElementSeparator( this, "Sound-based ESP" );

	m_pSoundESP = Modules::menu->AddParamBool( this, "SoundESP", NULL, false );
	m_pSoundESPFadeOut = Modules::menu->AddParamFloat( this, "SoundESPFadeOut", NULL, 2.f, 0.1f, 10.f );
	m_pSoundESPColor = Modules::menu->AddParamColorRGBA( this, "SoundESPColor", NULL, Color( 1.f, 1.f, 1.f, 0.5f ) );

	Modules::menu->AddElementSeparator( this );

	m_pDistance = Modules::menu->AddParamFloat( this, "Distance", NULL, 8192.f, 1.f, 8192.f );
	m_pBoxFillAlpha = Modules::menu->AddParamInteger( this, "BoxFillAlpha", NULL, 0, 0, 255 );

	m_pDrawBoxType = Modules::menu->AddParamList( this, "DrawBoxType", NULL, 0, " 0 - OFF\0 1 - Default\0 2 - Coal\0 3 - Corner\0\0" );
	m_pDrawPlayerStyle = Modules::menu->AddParamList( this, "DrawPlayerStyle", NULL, 1, " 0 - Default\0 1 - SAMP\0 2 - Left 4 Dead\0\0" );
	m_pDrawEntityStyle = Modules::menu->AddParamList( this, "DrawEntityStyle", NULL, 0, " 0 - Default\0 1 - SAMP\0 2 - Left 4 Dead\0\0" );
	m_pDrawTargets = Modules::menu->AddParamList( this, "DrawTargets", NULL, 0, " 0 - Everyone\0 1 - Entities\0 2 - Players\0\0" );
	m_pDrawBoxTargets = Modules::menu->AddParamList( this, "DrawBoxTargets", NULL, 0, " 0 - Everyone\0 1 - Entities\0 2 - Players\0\0" );
	m_pDrawDistanceTargets = Modules::menu->AddParamList( this, "DrawDistanceTargets", NULL, 1, " 0 - Everyone\0 1 - Entities\0 2 - Players\0\0" );
	m_pDrawSkeletonTargets = Modules::menu->AddParamList( this, "DrawSkeletonTargets", NULL, 1, " 0 - Everyone\0 1 - Entities\0 2 - Players\0\0" );

	m_pFriendPlayerColor = Modules::menu->AddParamColorRGB( this, "FriendPlayerColor", NULL, Color( 1.f, 1.f, 1.f, 1.f ) );
	m_pEnemyPlayerColor = Modules::menu->AddParamColorRGB( this, "EnemyPlayerColor", NULL, Color( 1.f, 0.f, 0.f, 1.f ) );
	m_pFriendColor = Modules::menu->AddParamColorRGB( this, "FriendColor", NULL, Color( 0.f, 1.f, 0.f, 1.f ) );
	m_pEnemyColor = Modules::menu->AddParamColorRGB( this, "EnemyColor", NULL, Color( 1.f, 0.f, 0.f, 1.f ) );
	m_pItemColor = Modules::menu->AddParamColorRGB( this, "ItemColor", NULL, Color( 0.f, 0.53f, 1.f, 1.f ) );
	m_pNeutralColor = Modules::menu->AddParamColorRGB( this, "NeutralColor", NULL, Color( 1.f, 1.f, 0.f, 1.f ) );

	bool bOK = true;
	int patternIndex;
	DEFINE_PATTERNS_FUTURE( fCClient_SoundEngine__PlayFMODSound );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client, FeaturesGameData::Patterns::Client::CClient_SoundEngine__PlayFMODSound, fCClient_SoundEngine__PlayFMODSound );

	m_pfnCClient_SoundEngine__PlayFMODSound = MemoryUtils()->GetPatternFutureValue( fCClient_SoundEngine__PlayFMODSound, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( m_pfnCClient_SoundEngine__PlayFMODSound,
										  "CClient_SoundEngine::PlayFMODSound",
										  FeaturesGameData::Patterns::Client::CClient_SoundEngine__PlayFMODSound,
										  patternIndex );

	if ( !bOK )
		PrintWarning2( "Sound ESP is not available\n" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CESP::PostLoad( void )
{
	if ( m_pfnCClient_SoundEngine__PlayFMODSound == NULL )
		return;

	m_hCClient_SoundEngine__PlayFMODSound = Detours()->DetourFunction( m_pfnCClient_SoundEngine__PlayFMODSound,
																	   HOOKED_CClient_SoundEngine__PlayFMODSound,
																	   GET_FUNC_PTR( ORIG_CClient_SoundEngine__PlayFMODSound ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CESP::Unload( void )
{
	Detours()->RemoveDetour( m_hCClient_SoundEngine__PlayFMODSound );
}
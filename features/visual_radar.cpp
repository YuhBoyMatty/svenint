// SvenInt (c) Sw1ft
// visual_radar.cpp

#include "stdafx.h"
#include "svenint.h"
#include "visual_radar.h"
#include "visual_esp.h"
#include "player_camhack.h"
#include "r_drawing.h"
#include "misc_entity_list.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CRadar, radar, "Visual", "Radar" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CRadar::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// kVGuiClientPanelPaint event
	if ( localplayer->IsSpectating() )
		return kHookContinue;

	if ( Features::camhack->IsEnabled() )
		return kHookContinue;

	if ( m_hRadarRoundTexture == -1 )
	{
		m_hRadarRoundTexture = surface->CreateNewTextureID( true );
		surface->DrawSetTextureFile( m_hRadarRoundTexture, SVENINT_FOLDER_NAME "/tex/radar_round", true, false );
	}

	constexpr int iThickness = 1;

	Vector va;
	CEntity *pEnts = Features::entitylist->GetList();
	Vector2D vecCenter = cl_enginefuncs->GetLocalPlayer()->curstate.origin.AsVector2D();

	cl_enginefuncs->GetViewAngles( va );

	int x = (int)( (float)gameutils->GetScreenWidth() * m_pScreenWidthFraction->GetFloat() );
	int y = (int)( (float)gameutils->GetScreenHeight() * m_pScreenHeightFraction->GetFloat() );

	float angle = va.y * static_cast<float>( M_PI ) / 180.f;

	float cy = cosf( -angle + static_cast<float>( M_PI ) / 2 );
	float sy = sinf( -angle + static_cast<float>( M_PI ) / 2 );

	// Transformation from world coordinates to screen coordinates
	float world2radar[ 2 ][ 3 ] =
	{
		{ (float)( m_pSize->GetInt() / m_pViewDistance->GetFloat() ), 0.f, (float)( m_pSize->GetInt() / 2 ) },
		{ 0.f, (float)( -m_pSize->GetInt() / m_pViewDistance->GetFloat() ), (float)( m_pSize->GetInt() / 2 ) }
	};

	if ( m_pType->GetInt() == 0 ) // round
	{
		// Round background
		Features::drawing->DrawTexture( m_hRadarRoundTexture,
										x, y,
										x + m_pSize->GetInt(),
										y + m_pSize->GetInt(),
										255, 255, 255, 127 );

		// Cross
		Features::drawing->DrawLine( x + m_pSize->GetInt() / 2,
									 y, x + m_pSize->GetInt() / 2,
									 y + m_pSize->GetInt(),
									 90, 90, 90, 127 );
		Features::drawing->DrawLine( x, y + m_pSize->GetInt() / 2,
									 x + m_pSize->GetInt(),
									 y + m_pSize->GetInt() / 2,
									 90, 90, 90, 127 );

		// Draw middle point
		Features::drawing->FillArea( x + m_pSize->GetInt() / 2 - iThickness,
									 y + m_pSize->GetInt() / 2 - iThickness,
									 iThickness * 2, iThickness * 2,
									 255, 255, 255, 255 );

		for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
		{
			CEntity &ent = pEnts[ i ];

			if ( !ent.m_bValid )
				continue;

			int r = int( 255.f * Features::esp->GetFriendColor()[ 0 ] );
			int g = int( 255.f * Features::esp->GetFriendColor()[ 1 ] );
			int b = int( 255.f * Features::esp->GetFriendColor()[ 2 ] );

			if ( ent.m_bItem )
			{
				if ( ent.m_bEnemy )
				{
					r = int( 255.f * Features::esp->GetEnemyColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetEnemyColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetEnemyColor()[ 2 ] );
				}
				else if ( ent.m_bNeutral )
				{
					r = int( 255.f * Features::esp->GetNeutralColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetNeutralColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetNeutralColor()[ 2 ] );
				}
				else
				{
					r = int( 255.f * Features::esp->GetItemColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetItemColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetItemColor()[ 2 ] );
				}
			}
			else if ( ent.m_bNeutral )
			{
				r = int( 255.f * Features::esp->GetNeutralColor()[ 0 ] );
				g = int( 255.f * Features::esp->GetNeutralColor()[ 1 ] );
				b = int( 255.f * Features::esp->GetNeutralColor()[ 2 ] );
			}
			else if ( ent.m_bEnemy )
			{
				r = int( 255.f * Features::esp->GetEnemyColor()[ 0 ] );
				g = int( 255.f * Features::esp->GetEnemyColor()[ 1 ] );
				b = int( 255.f * Features::esp->GetEnemyColor()[ 2 ] );
			}

			Vector2D vecDir;
			Vector2D temp = ent.m_vecOrigin.AsVector2D() - vecCenter;

			vecDir.x = temp.x * cy - temp.y * sy;
			vecDir.y = temp.x * sy + temp.y * cy;

			// Clamp
			if ( vecDir.LengthSqr() > M_SQR( m_pViewDistance->GetFloat() / 2 ) )
			{
				vecDir = vecDir.Normalize() * ( m_pViewDistance->GetFloat() / 2 );
			}

			int point_x = int( DotProduct( vecDir, *reinterpret_cast<Vector2D *>( world2radar[ 0 ] ) ) + world2radar[ 0 ][ 2 ] );
			int point_y = int( DotProduct( vecDir, *reinterpret_cast<Vector2D *>( world2radar[ 1 ] ) ) + world2radar[ 1 ][ 2 ] );

			if ( ent.m_bPlayer )
			{
				if ( m_pShowPlayerName->GetBool() )
				{
					player_info_t *pPlayer = enginestudio->PlayerInfo( i - 1 );
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													point_x + x,
													point_y + y - 8,
													r, g, b, 255,
													FONT_ALIGN_CENTER,
													"%s", pPlayer->name );
				}
			}
			else if ( m_pShowEntityName->GetBool() )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												point_x + x,
												point_y + y - 8,
												r, g, b, 255,
												FONT_ALIGN_CENTER,
												"%s", Features::entitylist->GetEntityClassname( ent.m_classInfo ) );
			}

			Features::drawing->FillArea( point_x + x - iThickness,
										 point_y + y - iThickness,
										 iThickness * 2,
										 iThickness * 2, r, g, b, 255 );
		}
	}
	else // square
	{
		// Square background
		Features::drawing->FillArea( x, y, m_pSize->GetInt(),
									 m_pSize->GetInt(),
									 0, 0, 0, 255 / 2 );

		// Cross
		Features::drawing->DrawLine( x + m_pSize->GetInt() / 2,
									 y, x + m_pSize->GetInt() / 2,
									 y + m_pSize->GetInt(),
									 90, 90, 90, 127 );
		Features::drawing->DrawLine( x, y + m_pSize->GetInt() / 2,
									 x + m_pSize->GetInt(),
									 y + m_pSize->GetInt() / 2,
									 90, 90, 90, 127 );

		// Draw middle point
		Features::drawing->FillArea( x + m_pSize->GetInt() / 2 - iThickness,
									 y + m_pSize->GetInt() / 2 - iThickness,
									 iThickness * 2, iThickness * 2,
									 255, 255, 255, 255 );

		for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
		{
			CEntity &ent = pEnts[ i ];

			if ( !ent.m_bValid )
				continue;

			int r = int( 255.f * Features::esp->GetFriendColor()[ 0 ] );
			int g = int( 255.f * Features::esp->GetFriendColor()[ 1 ] );
			int b = int( 255.f * Features::esp->GetFriendColor()[ 2 ] );

			if ( ent.m_bItem )
			{
				if ( ent.m_bEnemy )
				{
					r = int( 255.f * Features::esp->GetEnemyColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetEnemyColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetEnemyColor()[ 2 ] );
				}
				else if ( ent.m_bNeutral )
				{
					r = int( 255.f * Features::esp->GetNeutralColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetNeutralColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetNeutralColor()[ 2 ] );
				}
				else
				{
					r = int( 255.f * Features::esp->GetItemColor()[ 0 ] );
					g = int( 255.f * Features::esp->GetItemColor()[ 1 ] );
					b = int( 255.f * Features::esp->GetItemColor()[ 2 ] );
				}
			}
			else if ( ent.m_bNeutral )
			{
				r = int( 255.f * Features::esp->GetNeutralColor()[ 0 ] );
				g = int( 255.f * Features::esp->GetNeutralColor()[ 1 ] );
				b = int( 255.f * Features::esp->GetNeutralColor()[ 2 ] );
			}
			else if ( ent.m_bEnemy )
			{
				r = int( 255.f * Features::esp->GetEnemyColor()[ 0 ] );
				g = int( 255.f * Features::esp->GetEnemyColor()[ 1 ] );
				b = int( 255.f * Features::esp->GetEnemyColor()[ 2 ] );
			}

			Vector2D vecDir;
			Vector2D temp = ent.m_vecOrigin.AsVector2D() - vecCenter;

			vecDir.x = temp.x * cy - temp.y * sy;
			vecDir.y = temp.x * sy + temp.y * cy;

			// Clamp
			if ( fabsf( vecDir.x ) > ( m_pViewDistance->GetFloat() / 2 ) || fabsf( vecDir.y ) > ( m_pViewDistance->GetFloat() / 2 ) )
			{
				float scale;

				if ( abs( vecDir.x ) > abs( vecDir.y ) )
				{
					scale = fabsf( vecDir.x ) / ( m_pViewDistance->GetFloat() / 2 );
				}
				else
				{
					scale = fabsf( vecDir.y ) / ( m_pViewDistance->GetFloat() / 2 );
				}

				vecDir.x /= scale;
				vecDir.y /= scale;
			}

			int point_x = int( DotProduct( vecDir, *reinterpret_cast<Vector2D *>( world2radar[ 0 ] ) ) + world2radar[ 0 ][ 2 ] );
			int point_y = int( DotProduct( vecDir, *reinterpret_cast<Vector2D *>( world2radar[ 1 ] ) ) + world2radar[ 1 ][ 2 ] );

			if ( ent.m_bPlayer )
			{
				if ( m_pShowPlayerName->GetBool() )
				{
					player_info_t *pPlayer = enginestudio->PlayerInfo( i - 1 );
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													point_x + x,
													point_y + y - 8,
													r, g, b, 255,
													FONT_ALIGN_CENTER,
													"%s", pPlayer->name);
				}
			}
			else if ( m_pShowEntityName->GetBool() )
			{
				Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
												point_x + x,
												point_y + y - 8,
												r, g, b, 255,
												FONT_ALIGN_CENTER,
												"%s", Features::entitylist->GetEntityClassname( ent.m_classInfo ) );
			}

			Features::drawing->FillArea( point_x + x - iThickness,
										 point_y + y - iThickness,
										 iThickness * 2,
										 iThickness * 2,
										 r, g, b, 255 );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CRadar::CRadar( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pShowPlayerName = NULL;
	m_pShowEntityName = NULL;
	m_pType = NULL;
	m_pSize = NULL;
	m_pViewDistance = NULL;
	m_pScreenWidthFraction = NULL;
	m_pScreenHeightFraction = NULL;

	m_hRadarRoundTexture = -1;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CRadar::OnEnable( void )
{
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CRadar::OnDisable( void )
{
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CRadar::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pShowPlayerName = Modules::menu->AddParamBool( this, "ShowPlayerName", NULL, true );
	m_pShowEntityName = Modules::menu->AddParamBool( this, "ShowEntityName", NULL, true );
	m_pType = Modules::menu->AddParamList( this, "Type", NULL, 0, " 0 - Round\0 1 - Square\0\0" );
	m_pSize = Modules::menu->AddParamInteger( this, "Size", NULL, 250, 1, 1000 );
	m_pViewDistance = Modules::menu->AddParamFloat( this, "ViewDistance", NULL, 2048.f, 0.f, 16384.f );
	m_pScreenWidthFraction = Modules::menu->AddParamFloat( this, "ScreenWidthFraction", NULL, 0.01f, 0.f, 1.f );
	m_pScreenHeightFraction = Modules::menu->AddParamFloat( this, "ScreenHeightFraction", NULL, 0.01f, 0.f, 1.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CRadar::PostLoad( void )
{
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CRadar::Unload( void )
{
	if ( surface->IsTextureIDValid( m_hRadarRoundTexture ) )
		surface->DeleteTextureByID( m_hRadarRoundTexture );
}
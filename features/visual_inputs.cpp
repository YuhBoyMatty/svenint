// SvenInt (c) Sw1ft
// Took it from my private project l4dst
// visual_inputs.cpp

#include "stdafx.h"
#include "svenint.h"
#include "visual_inputs.h"
#include "player_camhack.h"
#include "mov_strafer.h"
#include "r_drawing.h"
#include "utils/util.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CInputs, inputs, "Visual", "Inputs" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CInputs::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// VGuiClientPanelPaint event
	if ( refparams->cmd == NULL || Features::camhack->IsEnabled() )
		return kHookContinue;

	int width, height;

	const int r = m_pColor->GetColor32().r;
	const int g = m_pColor->GetColor32().g;
	const int b = m_pColor->GetColor32().b;
	const int a = m_pColor->GetColor32().a;

	const usercmd_t *cmd = refparams->cmd;

	if ( m_pShowMoveInputs->GetBool() )
	{
		int x = int( (float)gameutils->GetScreenWidth() * m_pMoveInputsAnchorX->GetFloat() );
		int y = int( (float)gameutils->GetScreenHeight() * m_pMoveInputsAnchorY->GetFloat() );

		Vector2D vecMove( cmd->sidemove, cmd->forwardmove );

		const int iCircleSize = m_pMoveInputsCircleSize->GetInt();
		const int iDirCircleSize = m_pMoveInputsDirCircleSize->GetInt();

		constexpr int iCircleLines = 30;
		constexpr int iDirectionCircleLines = 10;

		float circlePos[ 2 ] = { (float)x, (float)y };

		float moveinput2screen[ 2 ][ 3 ] =
		{
			{ static_cast<float>( iCircleSize ), 0.f, static_cast<float>( x ) },
			{ 0.f, static_cast<float>( -iCircleSize ), static_cast<float>( y ) }
		};

		// Draw circle of move input
		Features::drawing->DrawCircle( circlePos, iCircleLines, (float)iCircleSize, r, g, b, a );
		Features::drawing->DrawCircle( circlePos, iCircleLines, (float)iCircleSize + 1, r, g, b, a );
		Features::drawing->DrawCircle( circlePos, iCircleLines, (float)iCircleSize - 1, 0, 0, 0, a ); // Inner shadow
		Features::drawing->DrawCircle( circlePos, iCircleLines, (float)iCircleSize + 2, 0, 0, 0, a ); // Outer shadow

		// Draw forward move
		Features::drawing->DrawStringExF( Features::drawing->GetFontInputs(),
										  x,
										  y + iCircleSize + iDirCircleSize * 3,
										  r, g, b, a,
										  width, height,
										  FONT_ALIGN_CENTER,
										  "%.1f", vecMove.y );

		// Draw side move
		Features::drawing->DrawStringF( Features::drawing->GetFontInputs(),
										x,
										y + iCircleSize + iDirCircleSize * 3 + height,
										r, g, b, a,
										FONT_ALIGN_CENTER,
										"%.1f", vecMove.x );

		Vector2D vecView( 0.f, -1.f );

		// Rotate move inputs & view by yaw difference between render view angles and usercmd's (silently set) view angles
		const float flSilentAnglesDiff = NormalizeAngle( cmd->viewangles[ 1 ] - refparams->viewangles[ 1 ] );
		bool clockwise;
		float thetaRotation, cy, sy;

		if ( flSilentAnglesDiff != 0.f )
		{
			clockwise = ( flSilentAnglesDiff >= 0.f );
			thetaRotation = VEC_DEG2RAD( fabsf( NormalizeAngle( flSilentAnglesDiff ) ) );
			cy = cosf( thetaRotation );
			sy = sinf( thetaRotation );

			const float vx = vecView.x, vy = vecView.y;

			if ( clockwise )
			{
				vecView.y = vy * cy - vx * sy;
				vecView.x = vy * sy + vx * sy;
			}
			else
			{
				vecView.y = vy * cy + vx * sy;
				vecView.x = -vy * sy + vx * sy;
			}
		}

		if ( !vecMove.IsZeroFast() )
		{
			vecMove.NormalizeInPlace();

			if ( flSilentAnglesDiff != 0.f )
			{
				const float fm = vecMove.x, sm = vecMove.y;

				if ( clockwise )
				{
					vecMove.x = fm * cy - sm * sy;
					vecMove.y = fm * sy + sm * sy;
				}
				else
				{
					vecMove.x = fm * cy + sm * sy;
					vecMove.y = -fm * sy + sm * sy;
				}
			}

			x = int( vecMove.Dot( *reinterpret_cast<const Vector2D *>( moveinput2screen[ 0 ] ) ) + moveinput2screen[ 0 ][ 2 ] );
			y = int( vecMove.Dot( *reinterpret_cast<const Vector2D *>( moveinput2screen[ 1 ] ) ) + moveinput2screen[ 1 ][ 2 ] );

			circlePos[ 0 ] = (float)x;
			circlePos[ 1 ] = (float)y;

			// Draw circle of move direction input
			Features::drawing->DrawCircle( circlePos, iDirectionCircleLines, (float)iDirCircleSize, r, g, b, a );
			Features::drawing->DrawCircle( circlePos, iDirectionCircleLines, (float)iDirCircleSize + 1, r, g, b, a );
			Features::drawing->DrawCircle( circlePos, iDirectionCircleLines, (float)iDirCircleSize - 1, 0, 0, 0, a ); // Inner shadow
			Features::drawing->DrawCircle( circlePos, iDirectionCircleLines, (float)iDirCircleSize + 2, 0, 0, 0, a ); // Outer shadow
		}

		// Draw our's actual view direction, which is most of time doesn't change
		moveinput2screen[ 0 ][ 0 ] = (float)iCircleSize + int( (float)iDirCircleSize * 1.85f );
		moveinput2screen[ 1 ][ 1 ] = (float)iCircleSize + int( (float)iDirCircleSize * 1.85f );

		x = int( vecView.Dot( *reinterpret_cast<const Vector2D *>( moveinput2screen[ 0 ] ) ) + moveinput2screen[ 0 ][ 2 ] );
		y = int( vecView.Dot( *reinterpret_cast<const Vector2D *>( moveinput2screen[ 1 ] ) ) + moveinput2screen[ 1 ][ 2 ] );

		circlePos[ 0 ] = (float)x;
		circlePos[ 1 ] = (float)y;

	#ifdef LINUX
		Features::drawing->DrawCircle( circlePos, 4, 1, r, g, b, a );
		Features::drawing->DrawCircle( circlePos, 8, 2, r, g, b, a );
		Features::drawing->DrawCircle( circlePos, 10, 3, 0, 0, 0, a ); // Outer shadow
	#else
		Features::drawing->DrawCircle( circlePos, 10, 3, r, g, b, a );
		Features::drawing->DrawCircle( circlePos, 12, 4, 0, 0, 0, a ); // Outer shadow
	#endif
	}

	if ( !m_pShowButtonInputs->GetBool() )
		return kHookContinue;

	struct HudInput
	{
		const char *pszName;
		int iWeight;
	};

	static HudInput HudInputs[] =
	{
		{ "IN_ATTACK", 7 },
		{ "IN_JUMP", 1 },
		{ "IN_DUCK", 8 },
		{ "IN_FORWARD", 2 },
		{ "IN_BACK", 2 },
		{ "IN_USE", 5 },
		{ "IN_CANCEL", 2 },
		{ "IN_LEFT", 2 },
		{ "IN_RIGHT", 2 },
		{ "IN_MOVELEFT", 2 },
		{ "IN_MOVERIGHT", 2 },
		{ "IN_ATTACK2", 6 },
		{ "IN_RUN", 2 },
		{ "IN_RELOAD", 3 },
		{ "IN_ALT1", 2 },
		{ "IN_SCORE", 2 },
		{ "IN_STRAFE", 9 } // Custom one
	};

	width = 0;
	height = 0;

	int offset = 0;
	const int x = int( (float)gameutils->GetScreenWidth() * m_pButtonInputsAnchorX->GetFloat() );
	const int y = int( (float)gameutils->GetScreenHeight() * m_pButtonInputsAnchorY->GetFloat() );

	std::vector<HudInput *> CurHudInputs;

	for ( int i = 0; i < 16; i++ )
	{
		if ( !( cmd->buttons & ( 1 << i ) ) )
			continue;

		if ( HudInputs[ i ].pszName[ 0 ] == '\0' )
			continue;

		CurHudInputs.push_back( HudInputs + i );
	}

	if ( Features::strafer->IsStrafed() )
		CurHudInputs.push_back( HudInputs + 16 );

	if ( CurHudInputs.empty() )
		return kHookContinue;

	std::sort( CurHudInputs.begin(), CurHudInputs.end(), []( const HudInput *a, const HudInput *b )
	{
		return a->iWeight > b->iWeight;
	} );

	for ( const HudInput *input : CurHudInputs )
	{
		if ( offset != 0 )
			offset += 2;

		Features::drawing->DrawStringEx( Features::drawing->GetFontInputs(),
										 x,
										 y + offset,
										 r, g, b, a,
										 width, height,
										 FONT_ALIGN_CENTER,
										 input->pszName );

		offset += height;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CInputs::CInputs( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pColor = NULL;
	m_pShowButtonInputs = NULL;
	m_pButtonInputsAnchorX = NULL;
	m_pButtonInputsAnchorY = NULL;
	m_pShowMoveInputs = NULL;
	m_pShowMoveInputsDirection = NULL;
	m_pMoveInputsCircleSize = NULL;
	m_pMoveInputsDirCircleSize = NULL;
	m_pMoveInputsAnchorX = NULL;
	m_pMoveInputsAnchorY = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CInputs::OnEnable( void )
{
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CInputs::OnDisable( void )
{
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CInputs::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clrInputs;
	UnpackRGBA( *(unsigned char *)&clrInputs.r,
				*(unsigned char *)&clrInputs.g,
				*(unsigned char *)&clrInputs.b,
				*(unsigned char *)&clrInputs.a,
				RGBA_DRAW_TEXT_DEFAULT );

	m_pColor = Modules::menu->AddParamColorRGBA( this, "Color", NULL, clrInputs );
	m_pShowButtonInputs = Modules::menu->AddParamBool( this, "ShowButtonInputs", NULL, true );
	m_pButtonInputsAnchorX = Modules::menu->AddParamFloat( this, "ButtonInputsAnchorX", NULL, 0.5f, 0.f, 1.f );
	m_pButtonInputsAnchorY = Modules::menu->AddParamFloat( this, "ButtonInputsAnchorY", NULL, 0.6f, 0.f, 1.f );
	m_pShowMoveInputs = Modules::menu->AddParamBool( this, "ShowMoveInputs", NULL, true );
	m_pShowMoveInputsDirection = Modules::menu->AddParamBool( this, "ShowMoveInputsDirection", NULL, true );
	m_pMoveInputsCircleSize = Modules::menu->AddParamInteger( this, "MoveInputsCircleSize", NULL, 34, 1, 100 );
	m_pMoveInputsDirCircleSize = Modules::menu->AddParamInteger( this, "MoveInputsDirCircleSize", NULL, 10, 1, 100 );
	m_pMoveInputsAnchorX = Modules::menu->AddParamFloat( this, "MoveInputsAnchorX", NULL, 0.5f, 0.f, 1.f );
	m_pMoveInputsAnchorY = Modules::menu->AddParamFloat( this, "MoveInputsAnchorY", NULL, 0.5f, 0.f, 1.f );

	return true;
}

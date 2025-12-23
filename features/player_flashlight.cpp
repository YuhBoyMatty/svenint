// SvenInt (c) Sw1ft
// player_flashlight.cpp

#include "stdafx.h"
#include "player_flashlight.h"
#include "player_firstperson_roaming.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( void, CALLCONV_CDECL, CL_PlayerFlashlight, cl_entity_t * );

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Engine
		{
			DEFINE_PATTERN( CL_PlayerFlashlight, "81 EC ? ? ? ? A1 ? ? ? ? 33 C4 89 84 24 BC 00 00 00 A1" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CFlashlight, flashlight, "Player", "Flashlight" );

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, CALLCONV_CDECL, HOOKED_CL_PlayerFlashlight, cl_entity_t *pEntity )
{
	if ( THIS_FEATURE_IS_ENABLED() )
	{
		THIS_FEATURE()->EmitCustomFlashlight( pEntity );
		return;
	}

	ORIG_CL_PlayerFlashlight( pEntity );
}

//-----------------------------------------------------------------------------
// Emit customizable flashlight
//-----------------------------------------------------------------------------

void CFlashlight::EmitCustomFlashlight( cl_entity_t *pEntity )
{
	bool bLocalPlayer;

	float falloff;
	pmtrace_t trace;

	Vector vecStart, vecEnd;
	Vector vecForward, v_angle, view_ofs;

	// Invalid entity or flashlight not enabled
	if ( pEntity == NULL || !( pEntity->curstate.effects & ( EF_BRIGHTLIGHT | EF_DIMLIGHT ) ) )
		return;

	// Local player
	if ( bLocalPlayer = ( pEntity->index == localplayer->GetPlayerIndex() ) )
	{
		cl_enginefuncs->GetViewAngles( v_angle );

		VectorCopy( *playermove->view_ofs(), view_ofs );
		AngleVectors( v_angle, &vecForward, NULL, NULL );
	}
	else
	{
		view_ofs.x = 0.f;
		view_ofs.y = 0.f;
		view_ofs.z = pEntity->curstate.usehull ? 12.5f : 28.5f;

		if ( Features::firstpersonroaming->GetTargetPlayer() == NULL )
		{
			v_angle = pEntity->curstate.angles;
			v_angle.x *= ( 89.0f / 9.8876953125f );
		}
		else
		{
			v_angle = Features::firstpersonroaming->GetLerpViewAngles();
		}

		AngleVectors( v_angle, &vecForward, NULL, NULL );
	}

	VectorAdd( pEntity->origin, view_ofs, vecStart );

	if ( ( bLocalPlayer && m_pSelf->GetBool() ) || ( !bLocalPlayer && m_pPlayers->GetBool() ) )
	{
		VectorMA( vecStart, bLocalPlayer ? m_pSelfDistance->GetFloat() : m_pPlayersDistance->GetFloat(), vecForward, vecEnd);

		cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
		cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecStart, vecEnd, PM_STUDIO_BOX, -1, &trace );

		falloff = trace.fraction * ( bLocalPlayer ? m_pSelfDistance->GetFloat() : m_pPlayersDistance->GetFloat() );

		if ( falloff < ( bLocalPlayer ? m_pSelfFalloffDistance->GetFloat() : m_pPlayersFalloffDistance->GetFloat() ) )
			falloff = 1.f;
		else
			falloff = ( bLocalPlayer ? m_pSelfFalloffDistance->GetFloat() : m_pPlayersFalloffDistance->GetFloat() ) / falloff;

		falloff *= falloff;

		// Flashlight
		dlight_t *pDynamicLight = cl_enginefuncs->pEfxAPI->CL_AllocDlight( pEntity->index );

		VectorCopy( trace.endpos, pDynamicLight->origin );

		pDynamicLight->color.r = int( 255.f * ( bLocalPlayer ? m_pSelfColor->GetColor()[ 0 ] : m_pPlayersColor->GetColor()[ 0 ] ) * falloff );
		pDynamicLight->color.g = int( 255.f * ( bLocalPlayer ? m_pSelfColor->GetColor()[ 1 ] : m_pPlayersColor->GetColor()[ 1 ] ) * falloff );
		pDynamicLight->color.b = int( 255.f * ( bLocalPlayer ? m_pSelfColor->GetColor()[ 2 ] : m_pPlayersColor->GetColor()[ 2 ] ) * falloff );
		pDynamicLight->radius = ( bLocalPlayer ? m_pSelfRadius->GetFloat() : m_pPlayersRadius->GetFloat() );

		pDynamicLight->die = cl_enginefuncs->GetClientTime() + 0.2f;
	}

	// Lighting from flashlight
	if ( ( bLocalPlayer && m_pSelfLighting->GetBool() ) || ( !bLocalPlayer && m_pPlayersLighting->GetBool() ) )
	{
		if ( !bLocalPlayer || ( bLocalPlayer && cl_funcs->CL_IsThirdPerson() ) )
		{
			Vector test;

			dlight_t *pEntityLight = cl_enginefuncs->pEfxAPI->CL_AllocElight( pEntity->index );

			VectorMA( vecStart, bLocalPlayer ? m_pSelfDistance->GetFloat() : m_pPlayersDistance->GetFloat(), vecForward, test);

			test.z += 24.f;

			VectorCopy( test, pEntityLight->origin );

			pEntityLight->color.r = int( 255.f * ( bLocalPlayer ? m_pSelfLightingColor->GetColor()[ 0 ] : m_pPlayersLightingColor->GetColor()[ 0 ] ) );
			pEntityLight->color.g = int( 255.f * ( bLocalPlayer ? m_pSelfLightingColor->GetColor()[ 1 ] : m_pPlayersLightingColor->GetColor()[ 1 ] ) );
			pEntityLight->color.b = int( 255.f * ( bLocalPlayer ? m_pSelfLightingColor->GetColor()[ 2 ] : m_pPlayersLightingColor->GetColor()[ 2 ] ) );
			pEntityLight->radius = ( bLocalPlayer ? m_pSelfLightingRadius->GetFloat() : m_pPlayersLightingRadius->GetFloat() );

			pEntityLight->die = cl_enginefuncs->GetClientTime() + 0.2f;
		}
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CFlashlight::CFlashlight( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pSelf = NULL;
	m_pSelfDistance = NULL;
	m_pSelfFalloffDistance = NULL;
	m_pSelfRadius = NULL;
	m_pSelfColor = NULL;

	m_pSelfLighting = NULL;
	m_pSelfLightingDistance = NULL;
	m_pSelfLightingRadius = NULL;
	m_pSelfLightingColor = NULL;

	m_pPlayers = NULL;
	m_pPlayersDistance = NULL;
	m_pPlayersFalloffDistance = NULL;
	m_pPlayersRadius = NULL;
	m_pPlayersColor = NULL;

	m_pPlayersLighting = NULL;
	m_pPlayersLightingDistance = NULL;
	m_pPlayersLightingRadius = NULL;
	m_pPlayersLightingColor = NULL;

	m_pfnCL_PlayerFlashlight = NULL;
	m_hCL_PlayerFlashlight = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CFlashlight::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pSelf = Modules::menu->AddParamBool( this, "Self", NULL, true );
	m_pSelfDistance = Modules::menu->AddParamFloat( this, "SelfDistance", NULL, 2000.f, 0.f, 8192.f );
	m_pSelfFalloffDistance = Modules::menu->AddParamFloat( this, "SelfFalloffDistance", NULL, 500.f, 0.f, 8192.f );
	m_pSelfRadius = Modules::menu->AddParamFloat( this, "SelfRadius", NULL, 80.f, 1.f, 1024.f );
	m_pSelfColor = Modules::menu->AddParamColorRGB( this, "SelfColor", NULL, Color( 255, 255, 255, 255 ) );
	
	m_pSelfLighting = Modules::menu->AddParamBool( this, "SelfLighting", NULL, true );
	m_pSelfLightingDistance = Modules::menu->AddParamFloat( this, "SelfLightingDistance", NULL, 18.f, 1.f, 1024.f );
	m_pSelfLightingRadius = Modules::menu->AddParamFloat( this, "SelfLightingRadius", NULL, 128.f, 1.f, 1024.f );
	m_pSelfLightingColor = Modules::menu->AddParamColorRGB( this, "SelfLightingColor", NULL, Color( 32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 32.f / 255.f ) );

	Modules::menu->AddElementSeparator( this );

	m_pPlayers = Modules::menu->AddParamBool( this, "Players", NULL, true );
	m_pPlayersDistance = Modules::menu->AddParamFloat( this, "PlayersDistance", NULL, 2000.f, 0.f, 8192.f );
	m_pPlayersFalloffDistance = Modules::menu->AddParamFloat( this, "PlayersFalloffDistance", NULL, 500.f, 0.f, 8192.f );
	m_pPlayersRadius = Modules::menu->AddParamFloat( this, "PlayersRadius", NULL, 80.f, 1.f, 1024.f );
	m_pPlayersColor = Modules::menu->AddParamColorRGB( this, "PlayersColor", NULL, Color( 255, 255, 255, 255 ) );

	m_pPlayersLighting = Modules::menu->AddParamBool( this, "PlayersLighting", NULL, true );
	m_pPlayersLightingDistance = Modules::menu->AddParamFloat( this, "PlayersLightingDistance", NULL, 18.f, 1.f, 1024.f );
	m_pPlayersLightingRadius = Modules::menu->AddParamFloat( this, "PlayersLightingRadius", NULL, 128.f, 1.f, 1024.f );
	m_pPlayersLightingColor = Modules::menu->AddParamColorRGB( this, "PlayersLightingColor", NULL, Color( 32.f / 255.f, 32.f / 255.f, 32.f / 255.f, 32.f / 255.f ) );
	
	if ( gamedata->Initialized() && gamedata->PreferRVA() )
	{
		m_pfnCL_PlayerFlashlight = gamedata->FindRVA( GameData::Modules::Engine, "Engine", "CL_PlayerFlashlight" );
		if ( m_pfnCL_PlayerFlashlight == NULL )
			return false;
	}
	else
	{
	#ifdef WIN32
		m_pfnCL_PlayerFlashlight = MemoryUtils()->FindPattern( GameData::Modules::Engine, FeaturesGameData::Patterns::Engine::CL_PlayerFlashlight );
		FEATURE_CHECK_SYMBOL_PATTERN( m_pfnCL_PlayerFlashlight, "CL_PlayerFlashlight" );
	#else
		return false;
	#endif
	}

	GAMEDATA_DUMP_FILE_OFFSET( "m_pfnCL_PlayerFlashlight", m_pfnCL_PlayerFlashlight, GameData::Modules::Engine );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CFlashlight::PostLoad( void )
{
	m_hCL_PlayerFlashlight = Detours()->DetourFunction( m_pfnCL_PlayerFlashlight, HOOKED_CL_PlayerFlashlight, GET_FUNC_PTR( ORIG_CL_PlayerFlashlight ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CFlashlight::Unload( void )
{
	Detours()->RemoveDetour( m_hCL_PlayerFlashlight );
}

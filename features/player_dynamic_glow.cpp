// SvenInt (c) Sw1ft
// player_dynamic_glow.cpp

#include "stdafx.h"
#include "player_dynamic_glow.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define DYNAMIC_LIGHT_LIFE_TIME 0.05f

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDynamicGlow, dynamicglow, "Player", "Dynamic Glow" );

//-----------------------------------------------------------------------------
// CreateDynamicLight
//-----------------------------------------------------------------------------

void CDynamicGlow::CreateDynamicLight( int entindex, float *vOrigin, float *pColor24, float flRadius, float flDecay, float flDieTime )
{
	dlight_t *pDynamicLight = cl_enginefuncs->pEfxAPI->CL_AllocDlight( m_pAttach->GetBool() ? entindex : 0);

	pDynamicLight->color.r = int( 255.f * pColor24[ 0 ] );
	pDynamicLight->color.g = int( 255.f * pColor24[ 1 ] );
	pDynamicLight->color.b = int( 255.f * pColor24[ 2 ] );

	pDynamicLight->origin = vOrigin;
	pDynamicLight->die = flDieTime;
	pDynamicLight->radius = flRadius;
	pDynamicLight->decay = flDecay;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDynamicGlow::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_Redraw_HookEvent )
	{
		if ( !m_pSelf->GetBool() || localplayer->IsSpectating() )
			return kHookContinue;

		float flRadius = m_pSelfRadius->GetFloat();
		float flDecay = m_pSelfDecay->GetFloat();
		float flDieTime = cl_enginefuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

		CreateDynamicLight( localplayer->GetPlayerIndex(),
							localplayer->GetOrigin(),
							m_pSelfColor->GetColor(),
							flRadius, flDecay, flDieTime );

		return kHookContinue;
	}

	// kHUD_AddEntity post event
	auto type = pEvent->GetArg<int>( "type" );
	auto ent = pEvent->GetArg<cl_entity_t *>( "ent" );
	auto modelname = pEvent->GetArg<const char *>( "modelname" );

	if ( !pEvent->GetReturn<int>() || *modelname != 'm' )
		return kHookContinue;

	if ( type == ET_PLAYER )
	{
		if ( !m_pPlayers->GetBool() || ent->index == localplayer->GetPlayerIndex() )
			return kHookContinue;

		float flRadius = m_pPlayersRadius->GetFloat();
		float flDecay = m_pPlayersDecay->GetFloat();
		float flDieTime = cl_enginefuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

		CreateDynamicLight( ent->index,
							ent->curstate.origin,
							m_pPlayersColor->GetColor(),
							flRadius, flDecay, flDieTime );

		return kHookContinue;
	}

	const char *pszSlashLastOccur = strrchr( modelname, '/' );

	if ( pszSlashLastOccur )
		modelname = pszSlashLastOccur + 1;

	if ( modelname[ 0 ] == 'w' && modelname[ 1 ] == '_' ) // an item
	{
		if ( !m_pItems->GetBool() )
			return kHookContinue;

		float flRadius = m_pItemsRadius->GetFloat();
		float flDecay = m_pItemsDecay->GetFloat();
		float flDieTime = cl_enginefuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

		CreateDynamicLight( ent->index,
							ent->curstate.origin,
							m_pItemsColor->GetColor(),
							flRadius, flDecay, flDieTime );

		return kHookContinue;
	}

	if ( !m_pEntities->GetBool() ||
		 ent->curstate.solid <= SOLID_TRIGGER ||
		 ent->curstate.movetype == MOVETYPE_NONE )
		return kHookContinue;

	// an entity
	float flRadius = m_pEntitiesRadius->GetFloat();
	float flDecay = m_pEntitiesDecay->GetFloat();
	float flDieTime = cl_enginefuncs->GetClientTime() + DYNAMIC_LIGHT_LIFE_TIME;

	CreateDynamicLight( ent->index,
						ent->curstate.origin,
						m_pEntitiesColor->GetColor(),
						flRadius, flDecay, flDieTime );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDynamicGlow::CDynamicGlow( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pAttach = NULL;

	m_pSelf = NULL;
	m_pSelfRadius = NULL;
	m_pSelfDecay = NULL;
	m_pSelfColor = NULL;

	m_pPlayers = NULL;
	m_pPlayersRadius = NULL;
	m_pPlayersDecay = NULL;
	m_pPlayersColor = NULL;

	m_pEntities = NULL;
	m_pEntitiesRadius = NULL;
	m_pEntitiesDecay = NULL;
	m_pEntitiesColor = NULL;

	m_pItems = NULL;
	m_pItemsRadius = NULL;
	m_pItemsDecay = NULL;
	m_pItemsColor = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CDynamicGlow::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent );
	hookevents->RegisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CDynamicGlow::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent );
	hookevents->UnregisterListener( this, kHUD_AddEntity_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CDynamicGlow::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pAttach = Modules::menu->AddParamBool( this, "Attach", NULL, false );

	Modules::menu->AddElementSeparator( this );

	m_pSelf = Modules::menu->AddParamBool( this, "Self", NULL, true );
	m_pSelfRadius = Modules::menu->AddParamFloat( this, "SelfRadius", NULL, 125.f, 0.f, 4096.f );
	m_pSelfDecay = Modules::menu->AddParamFloat( this, "SelfDecay", NULL, 0.f, 0.f, 4096.f );
	m_pSelfColor = Modules::menu->AddParamColorRGB( this, "SelfColor", NULL, Color( 255, 255, 255, 255 ) );

	Modules::menu->AddElementSeparator( this );

	m_pPlayers = Modules::menu->AddParamBool( this, "Players", NULL, false );
	m_pPlayersRadius = Modules::menu->AddParamFloat( this, "PlayersRadius", NULL, 75.f, 0.f, 4096.f );
	m_pPlayersDecay = Modules::menu->AddParamFloat( this, "PlayersDecay", NULL, 0.f, 0.f, 4096.f );
	m_pPlayersColor = Modules::menu->AddParamColorRGB( this, "PlayersColor", NULL, Color( 0, 255, 0, 255 ) );

	Modules::menu->AddElementSeparator( this );

	m_pEntities = Modules::menu->AddParamBool( this, "Entities", NULL, false );
	m_pEntitiesRadius = Modules::menu->AddParamFloat( this, "EntitiesRadius", NULL, 75.f, 0.f, 4096.f );
	m_pEntitiesDecay = Modules::menu->AddParamFloat( this, "EntitiesDecay", NULL, 0.f, 0.f, 4096.f );
	m_pEntitiesColor = Modules::menu->AddParamColorRGB( this, "EntitiesColor", NULL, Color( 255, 0, 0, 255 ) );

	Modules::menu->AddElementSeparator( this );

	m_pItems = Modules::menu->AddParamBool( this, "Items", NULL, false );
	m_pItemsRadius = Modules::menu->AddParamFloat( this, "ItemsRadius", NULL, 50.f, 0.f, 4096.f );
	m_pItemsDecay = Modules::menu->AddParamFloat( this, "ItemsDecay", NULL, 0.f, 0.f, 4096.f );
	m_pItemsColor = Modules::menu->AddParamColorRGB( this, "ItemsColor", NULL, Color( 0, 0, 255, 255 ) );

	return true;
}
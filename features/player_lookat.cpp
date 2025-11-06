// SvenInt (c) Sw1ft
// player_lookat.cpp

#include "stdafx.h"
#include "player_lookat.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CLookAt, lookat, "Player", "Look at" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_look_at( "sc_look_at", "0", FCVAR_EXTDLL, "Look at an entity" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CLookAt::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	int index = sc_look_at.GetInt();
	if ( index <= 0 || index == localplayer->GetPlayerIndex() || localplayer->IsSpectating() )
		return kHookContinue;

	cl_entity_t *pEntity = cl_enginefuncs->GetEntityByIndex( index );
	if ( pEntity != NULL )
	{
		Vector vecDir;
		Vector vecAngles;

		Vector vecSrc = localplayer->GetEyePosition();
		Vector vecTarget = pEntity->curstate.origin + Vector( 0.f, 0.f, pEntity->curstate.usehull ? 12.5f : 28.5f );

		VectorSubtract( vecTarget, vecSrc, vecDir );

		vecAngles.x = VEC_RAD2DEG( -atan2f( vecDir.z, vecDir.Length2D() ) );
		vecAngles.y = VEC_RAD2DEG( atan2f( vecDir.y, vecDir.x ) );
		vecAngles.z = 0.f;

		NormalizeAngles( vecAngles );

		cl_enginefuncs->SetViewAngles( vecAngles );
		pEvent->GetArg<usercmd_t *>( "cmd" )->viewangles = vecAngles;

		return kHookContinue;
	}
	else
	{
		sc_look_at.SetValue( 0 );
	}
	
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CLookAt::CLookAt( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CLookAt::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CLookAt::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CLookAt::PostLoad( void )
{
	FEATURE_REGISTER_CVAR( sc_look_at );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CLookAt::Unload( void )
{
	FEATURE_UNREGISTER_CVAR( sc_look_at );
}
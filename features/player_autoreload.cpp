// SvenInt (c) Sw1ft
// player_autoreload.cpp

#include "stdafx.h"
#include "player_autoreload.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAutoReload, autoreload, "Player", "Auto Reload" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAutoReload::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	if ( clientweapon->IsReloading() )
		return kHookContinue;

	if ( clientweapon->Clip() == 0 )
	{
		// Can't reload while using laser homing
		if ( localplayer->GetCurrentWeaponID() == WEAPON_RPG &&
			 clientweapon->GetWeaponData()->iuser4 &&
			 clientweapon->GetWeaponData()->fuser1 != 0.f )
			return kHookContinue;

		pEvent->GetArg<usercmd_t *>( "cmd" )->buttons |= IN_RELOAD;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAutoReload::CAutoReload( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAutoReload::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAutoReload::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAutoReload::Load( void )
{
	Modules::menu->BindFeature( this );

	return true;
}
// SvenInt (c) Sw1ft
// player_drop_empty_weapon.cpp

#include "stdafx.h"
#include "player_drop_empty_weapon.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDropEmptyWeapon, dropemptyweapon, "Player", "Drop Empty Weapon" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_drop_empty_weapon, "Drop an empty weapon from your inventory" )
{
	THIS_FEATURE()->DropEmptyWeapon();
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

void CDropEmptyWeapon::DropEmptyWeapon( void )
{
	if ( cls->state != ca_active )
		return;

	for ( int i = 0; i < Inventory()->GetMaxWeaponSlots(); i++ )
	{
		for ( int j = 0; j < Inventory()->GetMaxWeaponPositions(); j++ )
		{
			WEAPON *pWeapon = Inventory()->GetWeapon( i, j );

			if ( pWeapon && !Inventory()->HasAmmo( pWeapon ) )
			{
				Inventory()->DropWeapon( pWeapon );
				return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDropEmptyWeapon::CDropEmptyWeapon( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CDropEmptyWeapon::OnDisable( void )
{
	DropEmptyWeapon();
	Enable();
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CDropEmptyWeapon::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CDropEmptyWeapon::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_drop_empty_weapon );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CDropEmptyWeapon::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_drop_empty_weapon );
}
// SvenInt (c) Sw1ft
// CBaseWeaponsResource.h

#ifndef SINT_CBASEWEAPONSRESOURCE_H
#define SINT_CBASEWEAPONSRESOURCE_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: abstract class CBaseWeaponsResource that provides
// compatibility with different SC versions
//-----------------------------------------------------------------------------

class CBaseWeaponsResource
{
public:
	virtual					~CBaseWeaponsResource() {}
	
	virtual void			*GetStruct( void ) { return 0; }
	virtual int				GetCompatibilityVersion( void ) { return 0; }

	virtual int				GetMaxWeaponSlots( void ) { return 0; }
	virtual int				GetMaxWeaponPositions( void ) { return 0; }
	virtual int				GetMaxAmmoTypes( void ) { return 0; }

	virtual struct WEAPON	*GetWeaponSlot( int slot, int pos ) { return 0; }
	virtual struct WEAPON	*GetFirstPos( int iSlot ) { return 0; }
	virtual struct WEAPON	*GetNextActivePos( int iSlot, int iSlotPos ) { return 0; }

	virtual bool			HasAmmo( struct WEAPON *p ) { return false; }
	virtual int				GetAmmo( int iId ) { return 0; } // typedef int AMMO
	virtual void			SetAmmo( int iId, int iCount ) { }
	virtual int				CountAmmo( int iId ) { return 0; }
};

//-----------------------------------------------------------------------------
// Get CBaseWeaponsResource based on SC version
//-----------------------------------------------------------------------------

CBaseWeaponsResource *GetBaseWeaponsResource( void *pWeaponsResource, int iGameVersion );

#endif // SINT_CBASEWEAPONSRESOURCE_H

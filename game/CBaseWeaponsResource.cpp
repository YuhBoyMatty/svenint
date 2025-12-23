// SvenInt (c) Sw1ft
// CBaseWeaponsResource.cpp

#include "stdafx.h"
#include "CBaseWeaponsResource.h"

//-----------------------------------------------------------------------------
// Dummy weapons resource
//-----------------------------------------------------------------------------

static CBaseWeaponsResource gWeaponsResourceDummy;

//-----------------------------------------------------------------------------
// CWeaponsResource >= SC 5.23
//-----------------------------------------------------------------------------

class CWeaponsResource_5_23 final : public CBaseWeaponsResource
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<WeaponsResource_5_23 *>( pStruct ); }

	virtual void			*GetStruct( void ) { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 523; }

	virtual int				GetMaxWeaponSlots( void ) { return MAX_WEAPON_SLOTS; }
	virtual int				GetMaxWeaponPositions( void ) { return MAX_WEAPON_POSITIONS_5_23; }
	virtual int				GetMaxAmmoTypes( void ) { return MAX_AMMO_TYPES; }

	virtual struct WEAPON	*GetWeaponSlot( int slot, int pos ) { return m_pStruct->GetWeaponSlot( slot, pos ); }
	virtual struct WEAPON	*GetFirstPos( int iSlot ) { return m_pStruct->GetFirstPos( iSlot ); }
	virtual struct WEAPON	*GetNextActivePos( int iSlot, int iSlotPos ) { return m_pStruct->GetNextActivePos( iSlot, iSlotPos ); }

	virtual bool			HasAmmo( struct WEAPON *p ) { return m_pStruct->HasAmmo( p ); }
	virtual int				GetAmmo( int iId ) { return m_pStruct->GetAmmo( iId ); } // typedef int AMMO
	virtual void			SetAmmo( int iId, int iCount ) { m_pStruct->SetAmmo( iId, iCount ); }
	virtual int				CountAmmo( int iId ) { return m_pStruct->CountAmmo( iId ); }

private:
	WeaponsResource_5_23 *m_pStruct;
};

static CWeaponsResource_5_23 gWeaponsResource_5_23;

//-----------------------------------------------------------------------------
// CWeaponsResource 5.22
//-----------------------------------------------------------------------------

class CWeaponsResource final : public CBaseWeaponsResource
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<WeaponsResource *>( pStruct ); }

	virtual void			*GetStruct( void ) { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 515; }

	virtual int				GetMaxWeaponSlots( void ) { return MAX_WEAPON_SLOTS; }
	virtual int				GetMaxWeaponPositions( void ) { return MAX_WEAPON_POSITIONS; }
	virtual int				GetMaxAmmoTypes( void ) { return MAX_AMMO_TYPES; }

	virtual struct WEAPON	*GetWeaponSlot( int slot, int pos ) { return m_pStruct->GetWeaponSlot( slot, pos ); }
	virtual struct WEAPON	*GetFirstPos( int iSlot ) { return m_pStruct->GetFirstPos( iSlot ); }
	virtual struct WEAPON	*GetNextActivePos( int iSlot, int iSlotPos ) { return m_pStruct->GetNextActivePos( iSlot, iSlotPos ); }

	virtual bool			HasAmmo( struct WEAPON *p ) { return m_pStruct->HasAmmo( p ); }
	virtual int				GetAmmo( int iId ) { return m_pStruct->GetAmmo( iId ); } // typedef int AMMO
	virtual void			SetAmmo( int iId, int iCount ) { m_pStruct->SetAmmo( iId, iCount ); }
	virtual int				CountAmmo( int iId ) { return m_pStruct->CountAmmo( iId ); }

private:
	WeaponsResource	*m_pStruct;
};

static CWeaponsResource gWeaponsResource;

//-----------------------------------------------------------------------------
// CWeaponsResource 5.11
//-----------------------------------------------------------------------------

class CWeaponsResource_5_11 final : public CBaseWeaponsResource
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<WeaponsResource_5_11 *>( pStruct ); }

	virtual void			*GetStruct( void ) { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 500; }

	virtual int				GetMaxWeaponSlots( void ) { return MAX_WEAPON_SLOTS_5_11; }
	virtual int				GetMaxWeaponPositions( void ) { return MAX_WEAPON_POSITIONS_5_11; }
	virtual int				GetMaxAmmoTypes( void ) { return MAX_AMMO_TYPES_5_11; }

	virtual struct WEAPON	*GetWeaponSlot( int slot, int pos ) { return m_pStruct->GetWeaponSlot( slot, pos ); }
	virtual struct WEAPON	*GetFirstPos( int iSlot ) { return m_pStruct->GetFirstPos( iSlot ); }
	virtual struct WEAPON	*GetNextActivePos( int iSlot, int iSlotPos ) { return m_pStruct->GetNextActivePos( iSlot, iSlotPos ); }

	virtual bool			HasAmmo( struct WEAPON *p ) { return m_pStruct->HasAmmo( p ); }
	virtual int				GetAmmo( int iId ) { return m_pStruct->GetAmmo( iId ); } // typedef int AMMO
	virtual void			SetAmmo( int iId, int iCount ) { m_pStruct->SetAmmo( iId, iCount ); }
	virtual int				CountAmmo( int iId ) { return m_pStruct->CountAmmo( iId ); }

private:
	WeaponsResource_5_11 *m_pStruct;
};

static CWeaponsResource_5_11 gWeaponsResource_5_11;

//-----------------------------------------------------------------------------
// Get CBaseWeaponsResource based on SC version
//-----------------------------------------------------------------------------

CBaseWeaponsResource *GetBaseWeaponsResource( void *pWeaponsResource, int iGameVersion )
{
	if ( pWeaponsResource == NULL || iGameVersion == 0 )
		return &gWeaponsResourceDummy;

	if ( iGameVersion >= SVEN_VERSION_CHECK( 5, 23, 0 ) )
	{
		gWeaponsResource_5_23.Init( pWeaponsResource );
		return &gWeaponsResource_5_23;
	}
	else if ( iGameVersion >= SVEN_VERSION_CHECK( 5, 15, 0 ) )
	{
		gWeaponsResource.Init( pWeaponsResource );
		return &gWeaponsResource;
	}

	gWeaponsResource_5_11.Init( pWeaponsResource );
	return &gWeaponsResource_5_11;
}
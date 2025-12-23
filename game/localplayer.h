// SvenInt (c) Sw1ft
// localplayer.h

#ifndef SINT_LOCAL_PLAYER_H
#define SINT_LOCAL_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "game/hlsdk_mini.h"

//-----------------------------------------------------------------------------
// CLocalPlayer
//-----------------------------------------------------------------------------

class CLocalPlayer
{
public:
	CLocalPlayer();

public:
	local_state_t		*GetLocalState( void );
	entity_state_t		*GetEntityState( void );
	clientdata_t		*GetClientData( void );

	float				Time( void );
	float				Realtime( void );
	float				Frametime( void );

	int					GetPlayerIndex( void );
	int					GetViewModelIndex( void );

	bool				IsMultiplayer( void );
	bool				IsConnected( void );
	bool				IsInGame( void );

	bool				IsDead( void );
	bool				IsDying( void );
	bool				IsSpectating( void );
	
	bool				IsOnGround( void );
	bool				IsOnLadder( void );

	bool				IsDucked( void );
	bool				IsDucking( void );

	bool				HasWeapon( void );
	bool				CanAttack( void );

	float				GetHealth( void );
	int					GetFlags( void );
	int					GetMoveType( void );
	int					GetEffects( void );
	int					GetGroundEntity( void );
	float				GetFallVelocity( void );
	float				GetMaxSpeed( void );
	float				GetClientMaxSpeed( void );

	int					GetWaterLevel( void );
	int					GetOldWaterLevel( void );
	int					GetWaterType( void );

	int					GetCurrentWeaponID( void );
	int					GetWeaponAnim( void );
	float				GetNextAttack( void );

	int					GetObserverMode( void );
	int					GetObserverTarget( void );

	unsigned int		GetRandomSeed( void );
	float				GetFOV( void );

	int					Buttons( void );
	int					ButtonLast( void );
	int					ButtonPressed( void );
	int					ButtonReleased( void );

	Vector				GetViewAngles( void );
	void				SetViewAngles( Vector &va );

	Vector				GetOrigin( void );
	const Vector		&GetOrigin( void ) const;
	
	Vector				GetEyePosition( void );
	
	Vector				GetVelocity( void );
	const Vector		&GetVelocity( void ) const;

	Vector				GetBaseVelocity( void );
	const Vector		&GetBaseVelocity( void ) const;

	Vector				GetViewOffset( void );
	const Vector		&GetViewOffset( void ) const;

	Vector				GetAngles( void );
	const Vector		&GetAngles( void ) const;

	Vector				GetForwardVector( void );
	const Vector		&GetForwardVector( void ) const;

	Vector				GetRightVector( void );
	const Vector		&GetRightVector( void ) const;

	Vector				GetUpVector( void );
	const Vector		&GetUpVector( void ) const;

public:
	void Update( local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed );
	void DrawDebugInfo( void );

	inline local_state_t *GetLocalStateStatic( void ) { return &m_LocalState; };

private:
	void DrawWeaponData( void );
	void DrawInventory( void );
	void DrawRefParams( void );
	void DrawMoveVars( void );
	void DrawClientData( void );
	void DrawClientEntity( void );
	void DrawEntityState( void );
	void DrawPlayerInfo( void );
	void DrawPlayerMove( void );
	void DrawProfile( void );

private:
	local_state_t m_LocalState;
	unsigned int m_uiRandomSeed;
	double m_dbTime;

	int m_nButtons;
	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;
};

CLocalPlayer *LocalPlayer( void );

//-----------------------------------------------------------------------------
// CClientWeapon
//-----------------------------------------------------------------------------

class CClientWeapon
{
public:
	CClientWeapon();

	inline void SetCurrentWeaponID( int id ) { m_iCurrentWeaponID = id; }
	inline void SetCurrentWeaponCustom( bool state ) { m_bCurrentWeaponCustom = state; }
	inline void SetForceReload( bool state ) { m_bForceWeaponReload = state; }
	inline bool IsForceReload( void ) const { return m_bForceWeaponReload; }

public:
	weapon_data_t	*GetWeaponDataList( void );

	weapon_data_t	*GetWeaponData( void );
	weapon_data_t	*GetWeaponData( int iWeaponID );

	inline int		GetCurrentWeaponID( void ) const { return m_iCurrentWeaponID; }
	bool			IsCustom( void );

	int				Clip( void );

	int				PrimaryAmmo( void );
	int				SecondaryAmmo( void );

	int				PrimaryAmmoType( void );
	int				SecondaryAmmoType( void );

	float			GetNextPrimaryAttack( void );
	float			GetNextSecondaryAttack( void );

	float			GetWeaponIdle( void );

	bool			CanPrimaryAttack( void );
	bool			CanSecondaryAttack( void );

	bool			IsReloading( void );
	bool			IsInZoom( void );

	void			Reload( void );
	void			Drop( void );

private:
	weapon_data_t *GetWeaponDataInternal( int iWeaponID );

private:
	int m_iCurrentWeaponID;
	bool m_bCurrentWeaponCustom;
	bool m_bForceWeaponReload;
};

CClientWeapon *ClientWeapon( void );

//-----------------------------------------------------------------------------
// CInventory
//-----------------------------------------------------------------------------

class CInventory
{
public:
	CInventory();

	int				GetMaxWeaponSlots( void );
	int				GetMaxWeaponPositions( void );

	WEAPON			*GetWeapon( int iWeaponID );
	WEAPON			*GetWeapon( int iSlot, int iPos );
	WEAPON			*GetWeapon( const char *pszWeaponName );

	WEAPON			*GetFirstPos( int iSlot );
	WEAPON			*GetNextActivePos( int iSlot, int iSlotPos );

	bool			HasAmmo( WEAPON *pWeapon );
	int				CountAmmo( int iAmmoType );

	int				GetPrimaryAmmoCount( WEAPON *pWeapon );
	int				GetSecondaryAmmoCount( WEAPON *pWeapon );

	void			SelectWeapon( WEAPON *pWeapon );
	void			SelectWeapon( const char *pszWeaponName );

	void			DropWeapon( WEAPON *pWeapon );
	void			DropWeapon( const char *pszWeaponName );
};

CInventory *Inventory( void );

#endif // SINT_LOCAL_PLAYER_H
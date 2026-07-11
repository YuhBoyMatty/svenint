// SvenInt (c) Sw1ft
// localplayer.cpp

#include "stdafx.h"
#include "localplayer.h"
#include "utils/prof.h"

#define DRAW_STRING( ... ) Globals::gameutils->DrawConsoleString( __VA_ARGS__ )

extern ConVar sc_debug_new_line_height;
extern ConVar sc_debug_show_weapondata;
extern ConVar sc_debug_show_inventory;
extern ConVar sc_debug_show_refparams;
extern ConVar sc_debug_show_movevars;
extern ConVar sc_debug_show_clientdata;
extern ConVar sc_debug_show_clientent;
extern ConVar sc_debug_show_entitystate;
extern ConVar sc_debug_show_playerinfo;
extern ConVar sc_debug_show_playermove;
extern ConVar sc_debug_show_prof;
extern ConVar sc_debug_show_netmsg_buffer;

//-----------------------------------------------------------------------------
// CLocalPlayer implementation
//-----------------------------------------------------------------------------

local_state_t *CLocalPlayer::GetLocalState( void )
{
	return &m_LocalState;
}

entity_state_t *CLocalPlayer::GetEntityState( void )
{
	return &m_LocalState.playerstate;
}

clientdata_t *CLocalPlayer::GetClientData( void )
{
	return &m_LocalState.client;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

float CLocalPlayer::Time( void )
{
	return Globals::cl_enginefuncs->GetClientTime();
}

float CLocalPlayer::Realtime( void )
{
	return static_cast<float>( *Globals::realtime );
}

float CLocalPlayer::Frametime( void )
{
	return static_cast<float>( *Globals::host_frametime );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

int CLocalPlayer::GetPlayerIndex( void )
{
	return Globals::playermove->player_index() + 1;
}

int CLocalPlayer::GetViewModelIndex( void )
{
	return m_LocalState.client.viewmodel;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CLocalPlayer::IsMultiplayer( void )
{
	return bool( Globals::playermove->multiplayer() );
}

bool CLocalPlayer::IsConnected( void )
{
	return Globals::cls->state >= ca_connected;
}

bool CLocalPlayer::IsInGame( void )
{
	return Globals::cls->state == ca_active;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CLocalPlayer::IsDead( void )
{
	return Globals::playermove->iuser1() != 0 || Globals::playermove->dead();
}

bool CLocalPlayer::IsDying( void )
{
	return Globals::playermove->dead();
}

bool CLocalPlayer::IsSpectating( void )
{
	return Globals::playermove->iuser1() != 0;
}

bool CLocalPlayer::IsOnGround( void )
{
	return Globals::playermove->onground() != -1;
}

bool CLocalPlayer::IsOnLadder( void )
{
	return Globals::playermove->movetype() == MOVETYPE_FLY;
}

bool CLocalPlayer::IsDucked( void )
{
	return Globals::playermove->flags() & FL_DUCKING;
}

bool CLocalPlayer::IsDucking( void )
{
	return Globals::playermove->bInDuck();
}

bool CLocalPlayer::HasWeapon( void )
{
	return Globals::clientweapon->GetCurrentWeaponID() != WEAPON_NONE;
}

bool CLocalPlayer::CanAttack( void )
{
	return Time() + m_LocalState.client.m_flNextAttack <= Time();
}

float CLocalPlayer::GetHealth( void )
{
	return m_LocalState.client.health;
}

int CLocalPlayer::GetFlags( void )
{
	return Globals::playermove->flags();
}

int CLocalPlayer::GetMoveType( void )
{
	return Globals::playermove->movetype();
}

int CLocalPlayer::GetEffects()
{
	return Globals::playermove->effects();
}

int CLocalPlayer::GetGroundEntity( void )
{
	int physent = Globals::playermove->onground();

	if ( physent == 0 )
		return 0;

	if ( physent > 0 && physent < Globals::playermove->GetMaxPhysEnts() )
	{
		return Globals::playermove->physents()[ physent ].info;
	}

	return -1;
}

float CLocalPlayer::GetFallVelocity( void )
{
	return Globals::playermove->flFallVelocity();
}

float CLocalPlayer::GetMaxSpeed( void )
{
	return Globals::playermove->maxspeed();
}

float CLocalPlayer::GetClientMaxSpeed( void )
{
	return Globals::playermove->clientmaxspeed();
}

int CLocalPlayer::GetWaterLevel( void )
{
	return Globals::playermove->waterlevel();
}

int CLocalPlayer::GetOldWaterLevel( void )
{
	return Globals::playermove->oldwaterlevel();
}

int CLocalPlayer::GetWaterType( void )
{
	return Globals::playermove->watertype();
}

int CLocalPlayer::GetCurrentWeaponID( void )
{
	return Globals::clientweapon->GetCurrentWeaponID();
}

int CLocalPlayer::GetWeaponAnim( void )
{
	return m_LocalState.client.weaponanim;
}

float CLocalPlayer::GetNextAttack( void )
{
	return Time() + m_LocalState.client.m_flNextAttack;
}

int CLocalPlayer::GetObserverMode( void )
{
	return Globals::playermove->iuser1();
}

int CLocalPlayer::GetObserverTarget( void )
{
	return Globals::playermove->iuser2();
}

unsigned int CLocalPlayer::GetRandomSeed( void )
{
	return m_uiRandomSeed;
}

float CLocalPlayer::GetFOV( void )
{
	return m_LocalState.client.fov;
}

//-----------------------------------------------------------------------------
// Buttons
//-----------------------------------------------------------------------------

int CLocalPlayer::Buttons( void )
{
	return m_nButtons;
}

int CLocalPlayer::ButtonLast( void )
{
	return m_afButtonLast;
}

int CLocalPlayer::ButtonPressed( void )
{
	return m_afButtonPressed;
}

int CLocalPlayer::ButtonReleased( void )
{
	return m_afButtonReleased;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

Vector CLocalPlayer::GetViewAngles( void )
{
	Vector va;
	Globals::cl_enginefuncs->GetViewAngles( va );

	return va;
}

void CLocalPlayer::SetViewAngles( Vector &va )
{
	Globals::cl_enginefuncs->SetViewAngles( va );
}

Vector CLocalPlayer::GetOrigin( void )
{
	return *Globals::playermove->origin();
}

const Vector &CLocalPlayer::GetOrigin( void ) const
{
	return *Globals::playermove->origin();
}

Vector CLocalPlayer::GetEyePosition( void )
{
	return *Globals::playermove->origin() + *Globals::playermove->view_ofs();
}

Vector CLocalPlayer::GetVelocity( void )
{
	return *Globals::playermove->velocity();
}

const Vector &CLocalPlayer::GetVelocity( void ) const
{
	return *Globals::playermove->velocity();
}

Vector CLocalPlayer::GetBaseVelocity( void )
{
	return *Globals::playermove->basevelocity();
}

const Vector &CLocalPlayer::GetBaseVelocity( void ) const
{
	return *Globals::playermove->basevelocity();
}

Vector CLocalPlayer::GetViewOffset( void )
{
	return *Globals::playermove->view_ofs();
}

const Vector &CLocalPlayer::GetViewOffset( void ) const
{
	return *Globals::playermove->view_ofs();
}

Vector CLocalPlayer::GetAngles( void )
{
	return *Globals::playermove->angles();
}

const Vector &CLocalPlayer::GetAngles( void ) const
{
	return *Globals::playermove->angles();
}

Vector CLocalPlayer::GetForwardVector( void )
{
	return *Globals::playermove->forward();
}

const Vector &CLocalPlayer::GetForwardVector( void ) const
{
	return *Globals::playermove->forward();
}

Vector CLocalPlayer::GetRightVector( void )
{
	return *Globals::playermove->right();
}

const Vector &CLocalPlayer::GetRightVector( void ) const
{
	return *Globals::playermove->right();
}

Vector CLocalPlayer::GetUpVector( void )
{
	return *Globals::playermove->up();
}

const Vector &CLocalPlayer::GetUpVector( void ) const
{
	return *Globals::playermove->up();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CLocalPlayer::CLocalPlayer()
{
	memset( &m_LocalState, 0, sizeof( local_state_s ) );
	m_uiRandomSeed = 0;
	m_dbTime = 0.0;

	m_nButtons = 0;
	m_afButtonLast = 0;
	m_afButtonPressed = 0;
	m_afButtonReleased = 0;
}

void CLocalPlayer::Update( local_state_t *from, local_state_t *to, usercmd_t *cmd, double time, unsigned int random_seed )
{
	int buttonsChanged;

	m_nButtons = cmd->buttons;
	m_afButtonLast = from->playerstate.oldbuttons;

	buttonsChanged = m_afButtonLast ^ m_nButtons;

	m_afButtonPressed = buttonsChanged & m_nButtons;
	m_afButtonReleased = buttonsChanged & ( ~m_nButtons );

	m_uiRandomSeed = random_seed;
	m_dbTime = time;

	if ( SVEN_VERSION() >= SVEN_VERSION_CHECK( 5, 22, 0 ) )
		memcpy( &m_LocalState, to, sizeof( local_state_s ) );
	else
		memcpy( &m_LocalState, to, sizeof( local_state_5_11_s ) );

	Globals::clientweapon->SetCurrentWeaponCustom( to->client.m_iId == 0 );

	if ( to->client.m_iId != 0 )
	{
		Globals::clientweapon->SetCurrentWeaponID( to->client.m_iId );
	}
}

void CLocalPlayer::DrawDebugInfo( void )
{
	DrawWeaponData();
	DrawInventory();
	DrawRefParams();
	DrawMoveVars();
	DrawClientData();
	DrawClientEntity();
	DrawEntityState();
	DrawPlayerInfo();
	DrawPlayerMove();
	DrawProfile();
	DrawNetmsgBufferUsage();
}

void CLocalPlayer::DrawWeaponData( void )
{
	if ( !sc_debug_show_weapondata.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	const int id = Globals::clientweapon->GetCurrentWeaponID();
	const WEAPON *pWeapon = Globals::inventory->GetWeapon( Globals::clientweapon->GetCurrentWeaponID() );

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "Weapon ID: %d", Globals::clientweapon->GetCurrentWeaponID() );
	y += offset; DRAW_STRING( x, y, "Custom: %d", Globals::clientweapon->IsCustom() );
	y += offset; DRAW_STRING( x, y, "Weapon Data: 0x%X", m_LocalState.weapondata + Globals::clientweapon->GetCurrentWeaponID() );
	y += offset; DRAW_STRING( x, y, "Weapon Resource Data: 0x%X", Globals::inventory->GetWeapon( Globals::clientweapon->GetCurrentWeaponID() ) );

	y += offset * 2;
	DRAW_STRING( x, y, "Clip: %d", Globals::clientweapon->Clip() );
	y += offset; DRAW_STRING( x, y, "PrimaryAmmo: %d", Globals::clientweapon->PrimaryAmmo() );
	y += offset; DRAW_STRING( x, y, "SecondaryAmmo: %d", Globals::clientweapon->SecondaryAmmo() );
	y += offset; DRAW_STRING( x, y, "PrimaryAmmoType: %d", Globals::clientweapon->PrimaryAmmoType() );
	y += offset; DRAW_STRING( x, y, "SecondaryAmmoType: %d", Globals::clientweapon->SecondaryAmmoType() );
	y += offset; DRAW_STRING( x, y, "SecondaryAmmoCount: %d", pWeapon != NULL ? pWeapon->iCount : 0 );

	y += offset * 2;
	DRAW_STRING( x, y, "CanAttack: %d", CanAttack() );
	y += offset; DRAW_STRING( x, y, "CanPrimaryAttack: %d", Globals::clientweapon->CanPrimaryAttack() );
	y += offset; DRAW_STRING( x, y, "CanSecondaryAttack: %d", Globals::clientweapon->CanSecondaryAttack() );
	y += offset; DRAW_STRING( x, y, "IsReloading: %d", Globals::clientweapon->IsReloading() );
	y += offset; DRAW_STRING( x, y, "IsInZoom: %d", Globals::clientweapon->IsInZoom() );

	y += offset * 2;
	DRAW_STRING( x, y, "Current time: %.6lf", Time() );
	y += offset; DRAW_STRING( x, y, "NextPrimaryAttack: %.6f", Globals::clientweapon->GetNextPrimaryAttack() );
	y += offset; DRAW_STRING( x, y, "NextSecondaryAttack: %.6f", Globals::clientweapon->GetNextSecondaryAttack() );
	y += offset; DRAW_STRING( x, y, "WeaponIdle: %.6f", Globals::clientweapon->GetWeaponIdle() );

	y += offset * 2;
	DRAW_STRING( x, y, "weapondata.m_iClip: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_iClip );
	y += offset; DRAW_STRING( x, y, "weapondata.m_flNextPrimaryAttack: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_flNextPrimaryAttack );
	y += offset; DRAW_STRING( x, y, "weapondata.m_flNextSecondaryAttack: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_flNextSecondaryAttack );
	y += offset; DRAW_STRING( x, y, "weapondata.m_flTimeWeaponIdle: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_flTimeWeaponIdle );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fInReload: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fInReload );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fInSpecialReload: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fInSpecialReload );
	y += offset; DRAW_STRING( x, y, "weapondata.m_flNextReload: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_flNextReload );
	y += offset; DRAW_STRING( x, y, "weapondata.m_flPumpTime: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_flPumpTime );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fReloadTime: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fReloadTime );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fAimedDamage: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fAimedDamage );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fNextAimBonus: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fNextAimBonus );
	y += offset; DRAW_STRING( x, y, "weapondata.m_fInZoom: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_fInZoom );
	y += offset; DRAW_STRING( x, y, "weapondata.m_iWeaponState: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].m_iWeaponState );
	y += offset; DRAW_STRING( x, y, "weapondata.iuser1: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].iuser1 );
	y += offset; DRAW_STRING( x, y, "weapondata.iuser2: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].iuser2 );
	y += offset; DRAW_STRING( x, y, "weapondata.iuser3: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].iuser3 );
	y += offset; DRAW_STRING( x, y, "weapondata.iuser4: %d", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].iuser4 );
	y += offset; DRAW_STRING( x, y, "weapondata.fuser1: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].fuser1 );
	y += offset; DRAW_STRING( x, y, "weapondata.fuser2: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].fuser2 );
	y += offset; DRAW_STRING( x, y, "weapondata.fuser3: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].fuser3 );
	y += offset; DRAW_STRING( x, y, "weapondata.fuser4: %.6f", m_LocalState.weapondata[ Globals::clientweapon->GetCurrentWeaponID() ].fuser4 );
}

void CLocalPlayer::DrawInventory( void )
{
	if ( !sc_debug_show_inventory.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	WEAPON *pCurWeapon = NULL;
	int iCurSlot, iCurPos, iCurAmmoType = -1, iCurAmmo2Type = -1;

	int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	const int iInventoryOffsetX = 110;
	const int iAmmoOffsetX = 100;
	const int offset = sc_debug_new_line_height.GetInt();

	DRAW_STRING( x, y, "rgSlots:" );
	y += offset;

	int initial_x = x;
	int initial_y = y;

	for ( int i = 0; i < Globals::weaponsresource->GetMaxWeaponSlots(); i++ )
	{
		DRAW_STRING( x, y, "Slot #%d", i );
		y += offset;

		for ( int j = 0; j < Globals::weaponsresource->GetMaxWeaponPositions(); j++ )
		{
			bool bRevertColor = false;
			WEAPON *pWeapon = Globals::weaponsresource->GetWeaponSlot( i, j );
			if ( pWeapon != NULL && pWeapon->iId == Globals::clientweapon->GetCurrentWeaponID() )
			{
				pCurWeapon = pWeapon;
				iCurAmmoType = pWeapon->iAmmoType;
				iCurAmmo2Type = pWeapon->iAmmo2Type;
				iCurSlot = i;
				iCurPos = j;

				Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 255.f / 255.f, 210.f / 255.f );
				bRevertColor = true;
			}

			DRAW_STRING( x, y, "%X", pWeapon );

			if ( bRevertColor )
				Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

			y += offset;
		}

		x += iInventoryOffsetX;

		y = initial_y;
	}

	x = initial_x;
	y = ( Globals::weaponsresource->GetMaxWeaponPositions() + 4 ) * offset;

	DRAW_STRING( x, y, "riAmmo:" );
	y += offset;

	for ( int i = 0; i < Globals::weaponsresource->GetMaxAmmoTypes(); i++ )
	{
		if ( i == iCurAmmoType || i == iCurAmmo2Type )
		{
			Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 255.f / 255.f, 210.f / 255.f );
			DRAW_STRING( x, y, "#%d: %d", i, Globals::weaponsresource->GetAmmo( i ) );
			Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );
		}
		else
		{
			DRAW_STRING( x, y, "#%d: %d", i, Globals::weaponsresource->GetAmmo( i ) );
		}

		x += iAmmoOffsetX;

		if ( ( i + 1 ) % 10 == 0 )
		{
			x = initial_x;
			y += offset;
		}
	}

	if ( pCurWeapon == NULL )
		return;

	x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.73f );
	y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	DRAW_STRING( x, y, "WEAPON: %X", pCurWeapon );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].szName: %s", iCurSlot, iCurPos, pCurWeapon->szName );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iAmmoType: %d", iCurSlot, iCurPos, pCurWeapon->iAmmoType );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iAmmo2Type: %d", iCurSlot, iCurPos, pCurWeapon->iAmmo2Type );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iMax1: %d", iCurSlot, iCurPos, pCurWeapon->iMax1 );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iMax2: %d", iCurSlot, iCurPos, pCurWeapon->iMax2 );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iSlot: %d", iCurSlot, iCurPos, pCurWeapon->iSlot );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iSlotPos: %d", iCurSlot, iCurPos, pCurWeapon->iSlotPos );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iFlags: %d", iCurSlot, iCurPos, pCurWeapon->iFlags );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iId: %d", iCurSlot, iCurPos, pCurWeapon->iId );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iClip: %d", iCurSlot, iCurPos, pCurWeapon->iClip );
	y += offset; DRAW_STRING( x, y, "rgSlot[ %d ][ %d ].iCount: %d", iCurSlot, iCurPos, pCurWeapon->iCount );

	y += 2 * offset; DRAW_STRING( x, y, "Weapon ID: %d", Globals::clientweapon->GetCurrentWeaponID() );
	y += offset; DRAW_STRING( x, y, "Custom: %d", Globals::clientweapon->IsCustom() );
}

void CLocalPlayer::DrawRefParams( void )
{
	if ( !sc_debug_show_refparams.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "refparams.vieworg: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->vieworg ) );
	y += offset; DRAW_STRING( x, y, "refparams.viewangles: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->viewangles ) );
	y += offset; DRAW_STRING( x, y, "refparams.forward: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->forward ) );
	y += offset; DRAW_STRING( x, y, "refparams.right: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->right ) );
	y += offset; DRAW_STRING( x, y, "refparams.up: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->up ) );
	y += offset; DRAW_STRING( x, y, "refparams.frametime: %.6f", Globals::refparams->frametime );
	y += offset; DRAW_STRING( x, y, "refparams.time: %.6f", Globals::refparams->time );
	y += offset; DRAW_STRING( x, y, "refparams.intermission: %d", Globals::refparams->intermission );
	y += offset; DRAW_STRING( x, y, "refparams.paused: %d", Globals::refparams->paused );
	y += offset; DRAW_STRING( x, y, "refparams.spectator: %d", Globals::refparams->spectator );
	y += offset; DRAW_STRING( x, y, "refparams.onground: %d", Globals::refparams->onground );
	y += offset; DRAW_STRING( x, y, "refparams.waterlevel: %d", Globals::refparams->waterlevel );
	y += offset; DRAW_STRING( x, y, "refparams.simvel: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->simvel ) );
	y += offset; DRAW_STRING( x, y, "refparams.simorg: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->simorg ) );
	y += offset; DRAW_STRING( x, y, "refparams.viewheight: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->viewheight ) );
	y += offset; DRAW_STRING( x, y, "refparams.idealpitch: %.6f", Globals::refparams->idealpitch );
	y += offset; DRAW_STRING( x, y, "refparams.cl_viewangles: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->cl_viewangles ) );
	y += offset; DRAW_STRING( x, y, "refparams.health: %d", Globals::refparams->health );
	y += offset; DRAW_STRING( x, y, "refparams.crosshairangle: %.6f %.6f %.6f", VectorExpand( *(Vector *)Globals::refparams->crosshairangle ) );
	y += offset; DRAW_STRING( x, y, "refparams.maxclients: %d", Globals::refparams->maxclients );
	y += offset; DRAW_STRING( x, y, "refparams.viewentity: %d", Globals::refparams->viewentity );
	y += offset; DRAW_STRING( x, y, "refparams.playernum: %d", Globals::refparams->playernum );
	y += offset; DRAW_STRING( x, y, "refparams.max_entities: %d", Globals::refparams->max_entities );
	y += offset; DRAW_STRING( x, y, "refparams.demoplayback: %d", Globals::demoplayback );
	y += offset; DRAW_STRING( x, y, "refparams.hardware: %d", Globals::refparams->hardware );
	y += offset; DRAW_STRING( x, y, "refparams.smoothing: %d", Globals::refparams->smoothing );
	y += offset; DRAW_STRING( x, y, "refparams.cmd: %X", Globals::refparams->cmd );
	y += offset; DRAW_STRING( x, y, "refparams.movevars: %X", Globals::refparams->movevars );
	y += offset; DRAW_STRING( x, y, "refparams.viewport[ 0 ]: %d", Globals::refparams->viewport[ 0 ] );
	y += offset; DRAW_STRING( x, y, "refparams.viewport[ 1 ]: %d", Globals::refparams->viewport[ 1 ] );
	y += offset; DRAW_STRING( x, y, "refparams.viewport[ 2 ]: %d", Globals::refparams->viewport[ 2 ] );
	y += offset; DRAW_STRING( x, y, "refparams.viewport[ 3 ]: %d", Globals::refparams->viewport[ 3 ] );
	y += offset; DRAW_STRING( x, y, "refparams.nextView: %d", Globals::refparams->nextView );
	y += offset; DRAW_STRING( x, y, "refparams.onlyClientDraw: %d", Globals::refparams->onlyClientDraw );

	if ( Globals::refparams->cmd == NULL )
		return;

	y += offset;

	y += offset; DRAW_STRING( x, y, "refparams.cmd.lerp_msec: %d", Globals::refparams->cmd->lerp_msec );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.msec: %d", Globals::refparams->cmd->msec );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.viewangles: %.6f %.6f %.6f", VectorExpand( Globals::refparams->cmd->viewangles ) );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.forwardmove: %.6f", Globals::refparams->cmd->forwardmove );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.sidemove: %.6f", Globals::refparams->cmd->sidemove );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.upmove: %.6f", Globals::refparams->cmd->upmove );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.lightlevel: %d", Globals::refparams->cmd->lightlevel );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.buttons: %d", Globals::refparams->cmd->buttons );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.impulse: %d", Globals::refparams->cmd->impulse );
	y += offset; DRAW_STRING( x, y, "refparams.cmd.weaponselect: %d", Globals::refparams->cmd->weaponselect );
}

void CLocalPlayer::DrawMoveVars( void )
{
	if ( !sc_debug_show_movevars.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.73f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	movevars_t *movevars = Globals::playermove->movevars();
	if ( movevars == NULL )
		movevars = Globals::refparams_movevars;

	DRAW_STRING( x, y, "movevars.gravity: %.6f", movevars->gravity );
	y += offset; DRAW_STRING( x, y, "movevars.stopspeed: %.6f", movevars->stopspeed );
	y += offset; DRAW_STRING( x, y, "movevars.maxspeed: %.6f", movevars->maxspeed );
	y += offset; DRAW_STRING( x, y, "movevars.spectatormaxspeed: %.6f", movevars->spectatormaxspeed );
	y += offset; DRAW_STRING( x, y, "movevars.accelerate: %.6f", movevars->accelerate );
	y += offset; DRAW_STRING( x, y, "movevars.airaccelerate: %.6f", movevars->airaccelerate );
	y += offset; DRAW_STRING( x, y, "movevars.wateraccelerate: %.6f", movevars->wateraccelerate );
	y += offset; DRAW_STRING( x, y, "movevars.friction: %.6f", movevars->friction );
	y += offset; DRAW_STRING( x, y, "movevars.edgefriction: %.6f", movevars->edgefriction );
	y += offset; DRAW_STRING( x, y, "movevars.waterfriction: %.6f", movevars->waterfriction );
	y += offset; DRAW_STRING( x, y, "movevars.entgravity: %.6f", movevars->entgravity );
	y += offset; DRAW_STRING( x, y, "movevars.bounce: %.6f", movevars->bounce );
	y += offset; DRAW_STRING( x, y, "movevars.stepsize: %.6f", movevars->stepsize );
	y += offset; DRAW_STRING( x, y, "movevars.maxvelocity: %.6f", movevars->maxvelocity );
	y += offset; DRAW_STRING( x, y, "movevars.zmax: %.6f", movevars->zmax );
	y += offset; DRAW_STRING( x, y, "movevars.waveHeight: %.6f", movevars->waveHeight );
	y += offset; DRAW_STRING( x, y, "movevars.footsteps: %d", movevars->footsteps );
	y += offset; DRAW_STRING( x, y, "movevars.skyName: %s", movevars->skyName );
	y += offset; DRAW_STRING( x, y, "movevars.rollangle: %.6f", movevars->rollangle );
	y += offset; DRAW_STRING( x, y, "movevars.rollspeed: %.6f", movevars->rollspeed );
	y += offset; DRAW_STRING( x, y, "movevars.skycolor_r: %.6f", movevars->skycolor_r );
	y += offset; DRAW_STRING( x, y, "movevars.skycolor_g: %.6f", movevars->skycolor_g );
	y += offset; DRAW_STRING( x, y, "movevars.skycolor_b: %.6f", movevars->skycolor_b );
	y += offset; DRAW_STRING( x, y, "movevars.skyvec_x: %.6f", movevars->skyvec_x );
	y += offset; DRAW_STRING( x, y, "movevars.skyvec_y: %.6f", movevars->skyvec_y );
	y += offset; DRAW_STRING( x, y, "movevars.skyvec_z: %.6f", movevars->skyvec_z );
}

void CLocalPlayer::DrawClientData( void )
{
	if ( !sc_debug_show_clientdata.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.73f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "client.origin: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.origin ) );
	y += offset; DRAW_STRING( x, y, "client.velocity: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.velocity ) );
	y += offset; DRAW_STRING( x, y, "client.viewmodel: %d", m_LocalState.client.viewmodel );
	y += offset; DRAW_STRING( x, y, "client.punchangle: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.punchangle ) );
	y += offset; DRAW_STRING( x, y, "client.flags: %d", m_LocalState.client.flags );
	y += offset; DRAW_STRING( x, y, "client.waterlevel: %d", m_LocalState.client.waterlevel );
	y += offset; DRAW_STRING( x, y, "client.watertype: %d", m_LocalState.client.watertype );
	y += offset; DRAW_STRING( x, y, "client.view_ofs: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.view_ofs ) );
	y += offset; DRAW_STRING( x, y, "client.health: %.6f", m_LocalState.client.health );
	y += offset; DRAW_STRING( x, y, "client.bInDuck: %d", m_LocalState.client.bInDuck );
	y += offset; DRAW_STRING( x, y, "client.weapons: %d", m_LocalState.client.weapons );
	y += offset; DRAW_STRING( x, y, "client.flTimeStepSound: %d", m_LocalState.client.flTimeStepSound );
	y += offset; DRAW_STRING( x, y, "client.flDuckTime: %d", m_LocalState.client.flDuckTime );
	y += offset; DRAW_STRING( x, y, "client.flSwimTime: %d", m_LocalState.client.flSwimTime );
	y += offset; DRAW_STRING( x, y, "client.waterjumptime: %d", m_LocalState.client.waterjumptime );
	y += offset; DRAW_STRING( x, y, "client.maxspeed: %.6f", m_LocalState.client.maxspeed );
	y += offset; DRAW_STRING( x, y, "client.fov: %.6f", m_LocalState.client.fov );
	y += offset; DRAW_STRING( x, y, "client.weaponanim: %d", m_LocalState.client.weaponanim );
	y += offset; DRAW_STRING( x, y, "client.m_iId: %d", m_LocalState.client.m_iId );
	y += offset; DRAW_STRING( x, y, "client.ammo_shells: %d", m_LocalState.client.ammo_shells );
	y += offset; DRAW_STRING( x, y, "client.ammo_nails: %d", m_LocalState.client.ammo_nails );
	y += offset; DRAW_STRING( x, y, "client.ammo_cells: %d", m_LocalState.client.ammo_cells );
	y += offset; DRAW_STRING( x, y, "client.ammo_rockets: %d", m_LocalState.client.ammo_rockets );
	y += offset; DRAW_STRING( x, y, "client.m_flNextAttack: %.6f", m_LocalState.client.m_flNextAttack );
	y += offset; DRAW_STRING( x, y, "client.tfstate: %d", m_LocalState.client.tfstate );
	y += offset; DRAW_STRING( x, y, "client.pushmsec: %d", m_LocalState.client.pushmsec );
	y += offset; DRAW_STRING( x, y, "client.deadflag: %d", m_LocalState.client.deadflag );
	y += offset; DRAW_STRING( x, y, "client.iuser1: %d", m_LocalState.client.iuser1 );
	y += offset; DRAW_STRING( x, y, "client.iuser2: %d", m_LocalState.client.iuser2 );
	y += offset; DRAW_STRING( x, y, "client.iuser3: %d", m_LocalState.client.iuser3 );
	y += offset; DRAW_STRING( x, y, "client.iuser4: %d", m_LocalState.client.iuser4 );
	y += offset; DRAW_STRING( x, y, "client.fuser1: %.6f", m_LocalState.client.fuser1 );
	y += offset; DRAW_STRING( x, y, "client.fuser2: %.6f", m_LocalState.client.fuser2 );
	y += offset; DRAW_STRING( x, y, "client.fuser3: %.6f", m_LocalState.client.fuser3 );
	y += offset; DRAW_STRING( x, y, "client.fuser4: %.6f", m_LocalState.client.fuser4 );
	y += offset; DRAW_STRING( x, y, "client.vuser1: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.vuser1 ) );
	y += offset; DRAW_STRING( x, y, "client.vuser2: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.vuser2 ) );
	y += offset; DRAW_STRING( x, y, "client.vuser3: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.vuser3 ) );
	y += offset; DRAW_STRING( x, y, "client.vuser4: %.6f %.6f %.6f", VectorExpand( m_LocalState.client.vuser4 ) );
}

void CLocalPlayer::DrawClientEntity( void )
{
	if ( sc_debug_show_clientent.GetInt() < 1 )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	cl_entity_t *pEntity = Globals::cl_enginefuncs->GetEntityByIndex( sc_debug_show_clientent.GetInt() );
	if ( pEntity == NULL )
	{
		pEntity = Globals::cl_enginefuncs->GetLocalPlayer();
		if ( pEntity == NULL )
			return;
	}

	int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	int curpos = pEntity->current_position;
	int prevpos = curpos - 1;
	if ( prevpos < 0 )
		prevpos = HISTORY_MAX - 1;

	DRAW_STRING( x, y, "cl_entity.index: %d", pEntity->index );
	y += offset; DRAW_STRING( x, y, "cl_entity.player: %d", pEntity->player );
	y += offset; DRAW_STRING( x, y, "cl_entity.current_position: %d", curpos );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f", curpos, pEntity->ph[ curpos ].animtime );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f %.6f %.6f", curpos, VectorExpand( pEntity->ph[ curpos ].origin ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f %.6f %.6f", curpos, VectorExpand( pEntity->ph[ curpos ].angles ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f", prevpos, pEntity->ph[ prevpos ].animtime );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f %.6f %.6f", prevpos, VectorExpand( pEntity->ph[ prevpos ].origin ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.ph[ %.2d ]: %.6f %.6f %.6f", prevpos, VectorExpand( pEntity->ph[ prevpos ].angles ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.mouth.mouthopen: %d", pEntity->mouth.mouthopen );
	y += offset; DRAW_STRING( x, y, "cl_entity.mouth.sndcount: %d", pEntity->mouth.sndcount );
	y += offset; DRAW_STRING( x, y, "cl_entity.mouth.sndavg: %d", pEntity->mouth.sndavg );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevanimtime: %.6f", pEntity->latched.prevanimtime );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.sequencetime: %.6f", pEntity->latched.sequencetime );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevseqblending[ 0 ]: %d", pEntity->latched.prevseqblending[ 0 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevseqblending[ 1 ]: %d", pEntity->latched.prevseqblending[ 1 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevorigin: %.6f %.6f %.6f", VectorExpand( pEntity->latched.prevorigin ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevangles: %.6f %.6f %.6f", VectorExpand( pEntity->latched.prevangles ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevsequence: %d", pEntity->latched.prevsequence );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevframe: %.6f", pEntity->latched.prevframe );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevcontroller[ 0 ]: %d", pEntity->latched.prevcontroller[ 0 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevcontroller[ 1 ]: %d", pEntity->latched.prevcontroller[ 1 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevcontroller[ 2 ]: %d", pEntity->latched.prevcontroller[ 2 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevcontroller[ 3 ]: %d", pEntity->latched.prevcontroller[ 3 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevblending[ 0 ]: %d", pEntity->latched.prevblending[ 0 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.latched.prevblending[ 1 ]: %d", pEntity->latched.prevblending[ 1 ] );
	y += offset; DRAW_STRING( x, y, "cl_entity.lastmove: %.6f", pEntity->lastmove );
	y += offset; DRAW_STRING( x, y, "cl_entity.origin: %.6f %.6f %.6f", VectorExpand( pEntity->origin ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.angles: %.6f %.6f %.6f", VectorExpand( pEntity->angles ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.attachment[ 0 ]: %.6f %.6f %.6f", VectorExpand( pEntity->attachment[ 0 ] ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.attachment[ 1 ]: %.6f %.6f %.6f", VectorExpand( pEntity->attachment[ 1 ] ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.attachment[ 2 ]: %.6f %.6f %.6f", VectorExpand( pEntity->attachment[ 2 ] ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.attachment[ 3 ]: %.6f %.6f %.6f", VectorExpand( pEntity->attachment[ 3 ] ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.trivial_accept: %d", pEntity->trivial_accept );
	y += offset; DRAW_STRING( x, y, "cl_entity.model: %X (%s)", pEntity->model, pEntity->model ? pEntity->model->name : NULL );
	y += offset; DRAW_STRING( x, y, "cl_entity.syncbase: %.6f", pEntity->syncbase );
	y += offset; DRAW_STRING( x, y, "cl_entity.visframe: %d", pEntity->visframe );
	y += offset; DRAW_STRING( x, y, "cl_entity.cvFloorColor: %d %d %d %d", pEntity->cvFloorColor.r, pEntity->cvFloorColor.g, pEntity->cvFloorColor.b, pEntity->cvFloorColor.a );

	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.iuser1: %d", pEntity->curstate.iuser1 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.iuser2: %d", pEntity->curstate.iuser2 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.iuser3: %d", pEntity->curstate.iuser3 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.iuser4: %d", pEntity->curstate.iuser4 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.fuser1: %.6f", pEntity->curstate.fuser1 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.fuser2: %.6f", pEntity->curstate.fuser2 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.fuser3: %.6f", pEntity->curstate.fuser3 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.fuser4: %.6f", pEntity->curstate.fuser4 );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.vuser1: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.vuser1 ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.vuser2: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.vuser2 ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.vuser3: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.vuser3 ) );
	y += offset; DRAW_STRING( x, y, "cl_entity.curstate.vuser4: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.vuser4 ) );

	x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.73f );
	y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	DRAW_STRING( x, y, "curstate.entityType: %d", pEntity->curstate.entityType );
	y += offset; DRAW_STRING( x, y, "curstate.number: %d", pEntity->curstate.number );
	y += offset; DRAW_STRING( x, y, "curstate.msg_time: %.6f", pEntity->curstate.msg_time );
	y += offset; DRAW_STRING( x, y, "curstate.messagenum: %d", pEntity->curstate.messagenum );
	y += offset; DRAW_STRING( x, y, "curstate.origin: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.origin ) );
	y += offset; DRAW_STRING( x, y, "curstate.angles: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.angles ) );
	y += offset; DRAW_STRING( x, y, "curstate.modelindex: %d", pEntity->curstate.modelindex );
	y += offset; DRAW_STRING( x, y, "curstate.sequence: %d", pEntity->curstate.sequence );
	y += offset; DRAW_STRING( x, y, "curstate.frame: %.6f", pEntity->curstate.frame );
	y += offset; DRAW_STRING( x, y, "curstate.colormap: %d", pEntity->curstate.colormap );
	y += offset; DRAW_STRING( x, y, "curstate.skin: %d", pEntity->curstate.skin );
	y += offset; DRAW_STRING( x, y, "curstate.solid: %d", pEntity->curstate.solid );
	y += offset; DRAW_STRING( x, y, "curstate.effects: %d", pEntity->curstate.effects );
	y += offset; DRAW_STRING( x, y, "curstate.scale: %.6f", pEntity->curstate.scale );
	y += offset; DRAW_STRING( x, y, "curstate.eflags: %d", pEntity->curstate.eflags );
	y += offset; DRAW_STRING( x, y, "curstate.rendermode: %d", pEntity->curstate.rendermode );
	y += offset; DRAW_STRING( x, y, "curstate.renderamt: %d", pEntity->curstate.renderamt );
	y += offset; DRAW_STRING( x, y, "curstate.rendercolor: %d %d %d", pEntity->curstate.rendercolor.r, pEntity->curstate.rendercolor.g, pEntity->curstate.rendercolor.b );
	y += offset; DRAW_STRING( x, y, "curstate.renderfx: %d", pEntity->curstate.renderfx );
	y += offset; DRAW_STRING( x, y, "curstate.movetype: %d", pEntity->curstate.movetype );
	y += offset; DRAW_STRING( x, y, "curstate.animtime: %.6f", pEntity->curstate.animtime );
	y += offset; DRAW_STRING( x, y, "curstate.framerate: %.6f", pEntity->curstate.framerate );
	y += offset; DRAW_STRING( x, y, "curstate.body: %d", pEntity->curstate.body );
	y += offset; DRAW_STRING( x, y, "curstate.controller: %d %d %d %d", pEntity->curstate.controller[ 0 ], pEntity->curstate.controller[ 1 ], pEntity->curstate.controller[ 2 ], pEntity->curstate.controller[ 3 ] );
	y += offset; DRAW_STRING( x, y, "curstate.blending: %d %d %d %d", pEntity->curstate.blending[ 0 ], pEntity->curstate.blending[ 1 ], pEntity->curstate.blending[ 2 ], pEntity->curstate.blending[ 3 ] );
	y += offset; DRAW_STRING( x, y, "curstate.velocity: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.velocity ) );
	y += offset; DRAW_STRING( x, y, "curstate.mins: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.mins ) );
	y += offset; DRAW_STRING( x, y, "curstate.maxs: %d", VectorExpand( pEntity->curstate.maxs ) );
	y += offset; DRAW_STRING( x, y, "curstate.aiment: %d", pEntity->curstate.aiment );
	y += offset; DRAW_STRING( x, y, "curstate.owner: %d", pEntity->curstate.owner );
	y += offset; DRAW_STRING( x, y, "curstate.friction: %.6f", pEntity->curstate.friction );
	y += offset; DRAW_STRING( x, y, "curstate.gravity: %.6f", pEntity->curstate.gravity );
	y += offset; DRAW_STRING( x, y, "curstate.team: %d", pEntity->curstate.team );
	y += offset; DRAW_STRING( x, y, "curstate.playerclass: %d", pEntity->curstate.playerclass );
	y += offset; DRAW_STRING( x, y, "curstate.health: %d", pEntity->curstate.health );
	y += offset; DRAW_STRING( x, y, "curstate.spectator: %d", pEntity->curstate.spectator );
	y += offset; DRAW_STRING( x, y, "curstate.weaponmodel: %d", pEntity->curstate.weaponmodel );
	y += offset; DRAW_STRING( x, y, "curstate.gaitsequence: %d", pEntity->curstate.gaitsequence );
	y += offset; DRAW_STRING( x, y, "curstate.basevelocity: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.basevelocity ) );
	y += offset; DRAW_STRING( x, y, "curstate.usehull: %d", pEntity->curstate.usehull );
	y += offset; DRAW_STRING( x, y, "curstate.oldbuttons: %d", pEntity->curstate.oldbuttons );
	y += offset; DRAW_STRING( x, y, "curstate.onground: %d", pEntity->curstate.onground );
	y += offset; DRAW_STRING( x, y, "curstate.iStepLeft: %d", pEntity->curstate.iStepLeft );
	y += offset; DRAW_STRING( x, y, "curstate.flFallVelocity: %.6f", pEntity->curstate.flFallVelocity );
	y += offset; DRAW_STRING( x, y, "curstate.fov: %.6f", pEntity->curstate.fov );
	y += offset; DRAW_STRING( x, y, "curstate.weaponanim: %d", pEntity->curstate.weaponanim );
	y += offset; DRAW_STRING( x, y, "curstate.startpos: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.startpos ) );
	y += offset; DRAW_STRING( x, y, "curstate.basevelocity: %.6f %.6f %.6f", VectorExpand( pEntity->curstate.endpos ) );
	y += offset; DRAW_STRING( x, y, "curstate.impacttime: %.6f", pEntity->curstate.impacttime );
	y += offset; DRAW_STRING( x, y, "curstate.starttime: %.6f", pEntity->curstate.starttime );

#if 0
	x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.4f );
	y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	DRAW_STRING( x, y, "prevstate.entityType: %d", pEntity->prevstate.entityType );
	y += offset; DRAW_STRING( x, y, "prevstate.number: %d", pEntity->prevstate.number );
	y += offset; DRAW_STRING( x, y, "prevstate.msg_time: %.6f", pEntity->prevstate.msg_time );
	y += offset; DRAW_STRING( x, y, "prevstate.messagenum: %d", pEntity->prevstate.messagenum );
	y += offset; DRAW_STRING( x, y, "prevstate.origin: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.origin ) );
	y += offset; DRAW_STRING( x, y, "prevstate.angles: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.angles ) );
	y += offset; DRAW_STRING( x, y, "prevstate.modelindex: %d", pEntity->prevstate.modelindex );
	y += offset; DRAW_STRING( x, y, "prevstate.sequence: %d", pEntity->prevstate.sequence );
	y += offset; DRAW_STRING( x, y, "prevstate.frame: %.6f", pEntity->prevstate.frame );
	y += offset; DRAW_STRING( x, y, "prevstate.colormap: %d", pEntity->prevstate.colormap );
	y += offset; DRAW_STRING( x, y, "prevstate.skin: %d", pEntity->prevstate.skin );
	y += offset; DRAW_STRING( x, y, "prevstate.solid: %d", pEntity->prevstate.solid );
	y += offset; DRAW_STRING( x, y, "prevstate.effects: %d", pEntity->prevstate.effects );
	y += offset; DRAW_STRING( x, y, "prevstate.scale: %.6f", pEntity->prevstate.scale );
	y += offset; DRAW_STRING( x, y, "prevstate.eflags: %d", pEntity->prevstate.eflags );
	y += offset; DRAW_STRING( x, y, "prevstate.rendermode: %d", pEntity->prevstate.rendermode );
	y += offset; DRAW_STRING( x, y, "prevstate.renderamt: %d", pEntity->prevstate.renderamt );
	y += offset; DRAW_STRING( x, y, "prevstate.rendercolor: %d %d %d", pEntity->prevstate.rendercolor.r, pEntity->prevstate.rendercolor.g, pEntity->prevstate.rendercolor.b );
	y += offset; DRAW_STRING( x, y, "prevstate.renderfx: %d", pEntity->prevstate.renderfx );
	y += offset; DRAW_STRING( x, y, "prevstate.movetype: %d", pEntity->prevstate.movetype );
	y += offset; DRAW_STRING( x, y, "prevstate.animtime: %.6f", pEntity->prevstate.animtime );
	y += offset; DRAW_STRING( x, y, "prevstate.framerate: %.6f", pEntity->prevstate.framerate );
	y += offset; DRAW_STRING( x, y, "prevstate.body: %d", pEntity->prevstate.body );
	y += offset; DRAW_STRING( x, y, "prevstate.controller: %d %d %d %d", pEntity->prevstate.controller[ 0 ], pEntity->prevstate.controller[ 1 ], pEntity->prevstate.controller[ 2 ], pEntity->prevstate.controller[ 3 ] );
	y += offset; DRAW_STRING( x, y, "prevstate.blending: %d %d %d %d", pEntity->prevstate.blending[ 0 ], pEntity->prevstate.blending[ 1 ], pEntity->prevstate.blending[ 2 ], pEntity->prevstate.blending[ 3 ] );
	y += offset; DRAW_STRING( x, y, "prevstate.velocity: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.velocity ) );
	y += offset; DRAW_STRING( x, y, "prevstate.mins: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.mins ) );
	y += offset; DRAW_STRING( x, y, "prevstate.maxs: %d", VectorExpand( pEntity->prevstate.maxs ) );
	y += offset; DRAW_STRING( x, y, "prevstate.aiment: %d", pEntity->prevstate.aiment );
	y += offset; DRAW_STRING( x, y, "prevstate.owner: %d", pEntity->prevstate.owner );
	y += offset; DRAW_STRING( x, y, "prevstate.friction: %.6f", pEntity->prevstate.friction );
	y += offset; DRAW_STRING( x, y, "prevstate.gravity: %.6f", pEntity->prevstate.gravity );
	y += offset; DRAW_STRING( x, y, "prevstate.team: %d", pEntity->prevstate.team );
	y += offset; DRAW_STRING( x, y, "prevstate.playerclass: %d", pEntity->prevstate.playerclass );
	y += offset; DRAW_STRING( x, y, "prevstate.health: %d", pEntity->prevstate.health );
	y += offset; DRAW_STRING( x, y, "prevstate.spectator: %d", pEntity->prevstate.spectator );
	y += offset; DRAW_STRING( x, y, "prevstate.weaponmodel: %d", pEntity->prevstate.weaponmodel );
	y += offset; DRAW_STRING( x, y, "prevstate.gaitsequence: %d", pEntity->prevstate.gaitsequence );
	y += offset; DRAW_STRING( x, y, "prevstate.basevelocity: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.basevelocity ) );
	y += offset; DRAW_STRING( x, y, "prevstate.usehull: %d", pEntity->prevstate.usehull );
	y += offset; DRAW_STRING( x, y, "prevstate.oldbuttons: %d", pEntity->prevstate.oldbuttons );
	y += offset; DRAW_STRING( x, y, "prevstate.onground: %d", pEntity->prevstate.onground );
	y += offset; DRAW_STRING( x, y, "prevstate.iStepLeft: %d", pEntity->prevstate.iStepLeft );
	y += offset; DRAW_STRING( x, y, "prevstate.flFallVelocity: %.6f", pEntity->prevstate.flFallVelocity );
	y += offset; DRAW_STRING( x, y, "prevstate.fov: %.6f", pEntity->prevstate.fov );
	y += offset; DRAW_STRING( x, y, "prevstate.weaponanim: %d", pEntity->prevstate.weaponanim );
	y += offset; DRAW_STRING( x, y, "prevstate.startpos: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.startpos ) );
	y += offset; DRAW_STRING( x, y, "prevstate.basevelocity: %.6f %.6f %.6f", VectorExpand( pEntity->prevstate.endpos ) );
	y += offset; DRAW_STRING( x, y, "prevstate.impacttime: %.6f", pEntity->prevstate.impacttime );
	y += offset; DRAW_STRING( x, y, "prevstate.starttime: %.6f", pEntity->prevstate.starttime );
#endif
}

void CLocalPlayer::DrawEntityState( void )
{
	if ( !sc_debug_show_entitystate.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.73f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "playerstate.entityType: %d", m_LocalState.playerstate.entityType );
	y += offset; DRAW_STRING( x, y, "playerstate.number: %d", m_LocalState.playerstate.number );
	y += offset; DRAW_STRING( x, y, "playerstate.msg_time: %.6f", m_LocalState.playerstate.msg_time );
	y += offset; DRAW_STRING( x, y, "playerstate.messagenum: %d", m_LocalState.playerstate.messagenum );
	y += offset; DRAW_STRING( x, y, "playerstate.origin: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.origin ) );
	y += offset; DRAW_STRING( x, y, "playerstate.angles: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.angles ) );
	y += offset; DRAW_STRING( x, y, "playerstate.modelindex: %d", m_LocalState.playerstate.modelindex );
	y += offset; DRAW_STRING( x, y, "playerstate.sequence: %d", m_LocalState.playerstate.sequence );
	y += offset; DRAW_STRING( x, y, "playerstate.frame: %.6f", m_LocalState.playerstate.frame );
	y += offset; DRAW_STRING( x, y, "playerstate.colormap: %d", m_LocalState.playerstate.colormap );
	y += offset; DRAW_STRING( x, y, "playerstate.skin: %d", m_LocalState.playerstate.skin );
	y += offset; DRAW_STRING( x, y, "playerstate.solid: %d", m_LocalState.playerstate.solid );
	y += offset; DRAW_STRING( x, y, "playerstate.effects: %d", m_LocalState.playerstate.effects );
	y += offset; DRAW_STRING( x, y, "playerstate.scale: %.6f", m_LocalState.playerstate.scale );
	y += offset; DRAW_STRING( x, y, "playerstate.eflags: %d", m_LocalState.playerstate.eflags );
	y += offset; DRAW_STRING( x, y, "playerstate.rendermode: %d", m_LocalState.playerstate.rendermode );
	y += offset; DRAW_STRING( x, y, "playerstate.renderamt: %d", m_LocalState.playerstate.renderamt );
	y += offset; DRAW_STRING( x, y, "playerstate.rendercolor: %d %d %d", m_LocalState.playerstate.rendercolor.r, m_LocalState.playerstate.rendercolor.g, m_LocalState.playerstate.rendercolor.b );
	y += offset; DRAW_STRING( x, y, "playerstate.renderfx: %d", m_LocalState.playerstate.renderfx );
	y += offset; DRAW_STRING( x, y, "playerstate.movetype: %d", m_LocalState.playerstate.movetype );
	y += offset; DRAW_STRING( x, y, "playerstate.animtime: %.6f", m_LocalState.playerstate.animtime );
	y += offset; DRAW_STRING( x, y, "playerstate.framerate: %.6f", m_LocalState.playerstate.framerate );
	y += offset; DRAW_STRING( x, y, "playerstate.body: %d", m_LocalState.playerstate.body );
	y += offset; DRAW_STRING( x, y, "playerstate.controller: %d %d %d %d", m_LocalState.playerstate.controller[ 0 ], m_LocalState.playerstate.controller[ 1 ], m_LocalState.playerstate.controller[ 2 ], m_LocalState.playerstate.controller[ 3 ] );
	y += offset; DRAW_STRING( x, y, "playerstate.blending: %d %d %d %d", m_LocalState.playerstate.blending[ 0 ], m_LocalState.playerstate.blending[ 1 ], m_LocalState.playerstate.blending[ 2 ], m_LocalState.playerstate.blending[ 3 ] );
	y += offset; DRAW_STRING( x, y, "playerstate.velocity: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.velocity ) );
	y += offset; DRAW_STRING( x, y, "playerstate.mins: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.mins ) );
	y += offset; DRAW_STRING( x, y, "playerstate.maxs: %d", VectorExpand( m_LocalState.playerstate.maxs ) );
	y += offset; DRAW_STRING( x, y, "playerstate.aiment: %d", m_LocalState.playerstate.aiment );
	y += offset; DRAW_STRING( x, y, "playerstate.owner: %d", m_LocalState.playerstate.owner );
	y += offset; DRAW_STRING( x, y, "playerstate.friction: %.6f", m_LocalState.playerstate.friction );
	y += offset; DRAW_STRING( x, y, "playerstate.gravity: %.6f", m_LocalState.playerstate.gravity );
	y += offset; DRAW_STRING( x, y, "playerstate.team: %d", m_LocalState.playerstate.team );
	y += offset; DRAW_STRING( x, y, "playerstate.playerclass: %d", m_LocalState.playerstate.playerclass );
	y += offset; DRAW_STRING( x, y, "playerstate.health: %d", m_LocalState.playerstate.health );
	y += offset; DRAW_STRING( x, y, "playerstate.spectator: %d", m_LocalState.playerstate.spectator );
	y += offset; DRAW_STRING( x, y, "playerstate.weaponmodel: %d", m_LocalState.playerstate.weaponmodel );
	y += offset; DRAW_STRING( x, y, "playerstate.gaitsequence: %d", m_LocalState.playerstate.gaitsequence );
	y += offset; DRAW_STRING( x, y, "playerstate.basevelocity: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.basevelocity ) );
	y += offset; DRAW_STRING( x, y, "playerstate.usehull: %d", m_LocalState.playerstate.usehull );
	y += offset; DRAW_STRING( x, y, "playerstate.oldbuttons: %d", m_LocalState.playerstate.oldbuttons );
	y += offset; DRAW_STRING( x, y, "playerstate.onground: %d", m_LocalState.playerstate.onground );
	y += offset; DRAW_STRING( x, y, "playerstate.iStepLeft: %d", m_LocalState.playerstate.iStepLeft );
	y += offset; DRAW_STRING( x, y, "playerstate.flFallVelocity: %.6f", m_LocalState.playerstate.flFallVelocity );
	y += offset; DRAW_STRING( x, y, "playerstate.fov: %.6f", m_LocalState.playerstate.fov );
	y += offset; DRAW_STRING( x, y, "playerstate.weaponanim: %d", m_LocalState.playerstate.weaponanim );
	y += offset; DRAW_STRING( x, y, "playerstate.startpos: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.startpos ) );
	y += offset; DRAW_STRING( x, y, "playerstate.basevelocity: %.6f %.6f %.6f", VectorExpand( m_LocalState.playerstate.endpos ) );
	y += offset; DRAW_STRING( x, y, "playerstate.impacttime: %.6f", m_LocalState.playerstate.impacttime );
	y += offset; DRAW_STRING( x, y, "playerstate.starttime: %.6f", m_LocalState.playerstate.starttime );
}

void CLocalPlayer::DrawPlayerInfo( void )
{
	if ( sc_debug_show_playerinfo.GetInt() < 1 || sc_debug_show_playerinfo.GetInt() > Globals::refparams->maxclients )
		return;

	if ( Globals::cls->state != ca_active )
		return;
	
	player_info_t *pPlayerInfo = Globals::enginestudio->PlayerInfo( sc_debug_show_playerinfo.GetInt() - 1 );
	if ( pPlayerInfo == NULL )
	{
		pPlayerInfo = Globals::enginestudio->PlayerInfo( GetPlayerIndex() - 1 );
		if ( pPlayerInfo == NULL )
			return;
	}

	int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.05f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "player_info.userid: %d", pPlayerInfo->userid );
	y += offset; DRAW_STRING( x, y, "player_info.userinfo:", pPlayerInfo->userinfo );

	int prevX = x;
	x += 40;
	bool bPrintPair = false;
	std::string sPair;
	std::string sUserInfo = pPlayerInfo->userinfo;
	char *p = strtok( (char *)sUserInfo.c_str(), "\\" );
	while ( p != NULL )
	{
		if ( bPrintPair )
		{
			sPair += p;
			bPrintPair = false;

			y += offset; DRAW_STRING( x, y, "%s", sPair.c_str() );
			sPair.clear();
		}
		else
		{
			sPair = p;
			sPair += " = ";
			bPrintPair = true;
		}

		p = strtok( NULL, "\\" );
	}
	x = prevX;

	y += offset; DRAW_STRING( x, y, "player_info.name: %s", pPlayerInfo->name );
	y += offset; DRAW_STRING( x, y, "player_info.spectator: %d", pPlayerInfo->spectator );
	y += offset; DRAW_STRING( x, y, "player_info.ping: %d", pPlayerInfo->ping );
	y += offset; DRAW_STRING( x, y, "player_info.packet_loss: %d", pPlayerInfo->packet_loss );
	y += offset; DRAW_STRING( x, y, "player_info.model: %s", pPlayerInfo->model );
	y += offset; DRAW_STRING( x, y, "player_info.topcolor: %d", pPlayerInfo->topcolor );
	y += offset; DRAW_STRING( x, y, "player_info.bottomcolor: %d", pPlayerInfo->bottomcolor );
	y += offset; DRAW_STRING( x, y, "player_info.renderframe: %d", pPlayerInfo->renderframe );
	y += offset; DRAW_STRING( x, y, "player_info.gaitsequence: %d", pPlayerInfo->gaitsequence );
	y += offset; DRAW_STRING( x, y, "player_info.gaitframe: %.6f", pPlayerInfo->gaitframe );
	y += offset; DRAW_STRING( x, y, "player_info.gaityaw: %.6f", pPlayerInfo->gaityaw );
	y += offset; DRAW_STRING( x, y, "player_info.prevgaitorigin: %.6f %.6f %.6f", VectorExpand( pPlayerInfo->prevgaitorigin ) );
	y += offset; DRAW_STRING( x, y, "player_info.hashedcdkey: %X", pPlayerInfo->hashedcdkey );
	y += offset; DRAW_STRING( x, y, "player_info.m_nSteamID: %llu", pPlayerInfo->m_nSteamID );
}

void CLocalPlayer::DrawPlayerMove( void )
{
	if ( !sc_debug_show_playermove.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DRAW_STRING( x, y, "playermove.player_index: %d", Globals::playermove->player_index() );
	y += offset; DRAW_STRING( x, y, "playermove.multiplayer: %d", Globals::playermove->multiplayer() );
	y += offset; DRAW_STRING( x, y, "playermove.time: %.6f", Globals::playermove->time() );
	y += offset; DRAW_STRING( x, y, "playermove.frametime: %.6f", Globals::playermove->frametime() );
	y += offset; DRAW_STRING( x, y, "playermove.forward: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->forward() ) );
	y += offset; DRAW_STRING( x, y, "playermove.right: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->right() ) );
	y += offset; DRAW_STRING( x, y, "playermove.up: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->up() ) );
	y += offset; DRAW_STRING( x, y, "playermove.origin: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->origin() ) );
	y += offset; DRAW_STRING( x, y, "playermove.angles: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->angles() ) );
	y += offset; DRAW_STRING( x, y, "playermove.oldangles: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->oldangles() ) );
	y += offset; DRAW_STRING( x, y, "playermove.velocity: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->velocity() ) );
	y += offset; DRAW_STRING( x, y, "playermove.movedir: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->movedir() ) );
	y += offset; DRAW_STRING( x, y, "playermove.basevelocity: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->basevelocity() ) );
	y += offset; DRAW_STRING( x, y, "playermove.view_ofs: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->view_ofs() ) );
	y += offset; DRAW_STRING( x, y, "playermove.flDuckTime: %.6f", Globals::playermove->flDuckTime() );
	y += offset; DRAW_STRING( x, y, "playermove.bInDuck: %d", Globals::playermove->bInDuck() );
	y += offset; DRAW_STRING( x, y, "playermove.flTimeStepSound: %d", Globals::playermove->flTimeStepSound() );
	y += offset; DRAW_STRING( x, y, "playermove.iStepLeft: %d", Globals::playermove->iStepLeft() );
	y += offset; DRAW_STRING( x, y, "playermove.flFallVelocity: %.6f", Globals::playermove->flFallVelocity() );
	y += offset; DRAW_STRING( x, y, "playermove.punchangle: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->punchangle() ) );
	y += offset; DRAW_STRING( x, y, "playermove.flSwimTime: %.6f", Globals::playermove->flSwimTime() );
	y += offset; DRAW_STRING( x, y, "playermove.flNextPrimaryAttack: %.6f", Globals::playermove->flNextPrimaryAttack() );
	y += offset; DRAW_STRING( x, y, "playermove.effects: %d", Globals::playermove->effects() );
	y += offset; DRAW_STRING( x, y, "playermove.flags: %d", Globals::playermove->flags() );
	y += offset; DRAW_STRING( x, y, "playermove.usehull: %d", Globals::playermove->usehull() );
	y += offset; DRAW_STRING( x, y, "playermove.gravity: %.6f", Globals::playermove->gravity() );
	y += offset; DRAW_STRING( x, y, "playermove.friction: %.6f", Globals::playermove->friction() );
	y += offset; DRAW_STRING( x, y, "playermove.oldbuttons: %d", Globals::playermove->oldbuttons() );
	y += offset; DRAW_STRING( x, y, "playermove.waterjumptime: %.6f", Globals::playermove->waterjumptime() );
	y += offset; DRAW_STRING( x, y, "playermove.dead: %d", Globals::playermove->dead() );
	y += offset; DRAW_STRING( x, y, "playermove.deadflag: %d", Globals::playermove->deadflag() );
	y += offset; DRAW_STRING( x, y, "playermove.spectator: %d", Globals::playermove->spectator() );
	y += offset; DRAW_STRING( x, y, "playermove.movetype: %d", Globals::playermove->movetype() );
	y += offset; DRAW_STRING( x, y, "playermove.onground: %d", Globals::playermove->onground() );
	y += offset; DRAW_STRING( x, y, "playermove.waterlevel: %d", Globals::playermove->waterlevel() );
	y += offset; DRAW_STRING( x, y, "playermove.watertype: %d", Globals::playermove->watertype() );
	y += offset; DRAW_STRING( x, y, "playermove.oldwaterlevel: %d", Globals::playermove->oldwaterlevel() );
	y += offset; DRAW_STRING( x, y, "playermove.maxspeed: %.6f", Globals::playermove->maxspeed() );
	y += offset; DRAW_STRING( x, y, "playermove.clientmaxspeed: %.6f", Globals::playermove->clientmaxspeed() );
	y += offset; DRAW_STRING( x, y, "playermove.iuser1: %d", Globals::playermove->iuser1() );
	y += offset; DRAW_STRING( x, y, "playermove.iuser2: %d", Globals::playermove->iuser2() );
	y += offset; DRAW_STRING( x, y, "playermove.iuser3: %d", Globals::playermove->iuser3() );
	y += offset; DRAW_STRING( x, y, "playermove.iuser4: %d", Globals::playermove->iuser4() );
	y += offset; DRAW_STRING( x, y, "playermove.fuser1: %.6f", Globals::playermove->fuser1() );
	y += offset; DRAW_STRING( x, y, "playermove.fuser2: %.6f", Globals::playermove->fuser2() );
	y += offset; DRAW_STRING( x, y, "playermove.fuser3: %.6f", Globals::playermove->fuser3() );
	y += offset; DRAW_STRING( x, y, "playermove.fuser4: %.6f", Globals::playermove->fuser4() );
	y += offset; DRAW_STRING( x, y, "playermove.vuser1: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->vuser1() ) );
	y += offset; DRAW_STRING( x, y, "playermove.vuser2: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->vuser2() ) );
	y += offset; DRAW_STRING( x, y, "playermove.vuser3: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->vuser3() ) );
	y += offset; DRAW_STRING( x, y, "playermove.vuser4: %.6f %.6f %.6f", VectorExpand( *Globals::playermove->vuser4() ) );
}

void CLocalPlayer::DrawNetmsgBufferUsage( void )
{
	if ( !sc_debug_show_netmsg_buffer.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.0185185f );

	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	if ( Globals::net_message != NULL )
	{
		DRAW_STRING( x, y, "net_message usage:" );
		y += offset; DRAW_STRING( x, y, "cursize: %d bytes", Globals::net_message->cursize );
		y += offset; DRAW_STRING( x, y, "maxsize: %d bytes", Globals::net_message->maxsize );
		y += offset; DRAW_STRING( x, y, "%.1f / 100 %%", ( (float)Globals::net_message->cursize / (float)Globals::net_message->maxsize ) * 100.f );

		y += 2 * offset;
	}

	if ( Globals::clc_buffer != NULL )
	{
		DRAW_STRING( x, y, "clc_buffer usage:" );
		y += offset; DRAW_STRING( x, y, "cursize: %d bytes", Globals::clc_buffer->cursize );
		y += offset; DRAW_STRING( x, y, "maxsize: %d bytes", Globals::clc_buffer->maxsize );
		y += offset; DRAW_STRING( x, y, "%.1f / 100 %%", ( (float)Globals::clc_buffer->cursize / (float)Globals::clc_buffer->maxsize ) * 100.f );
	}
}

#ifdef PROF_ENABLED
static int DrawProfilesTree( CProfileNode *pNode, int x, int y, int offset, int iDepth = 0 )
{
	if ( pNode == NULL )
		return y;

	if ( sc_debug_show_prof.GetInt() == 1 )
	{
		DRAW_STRING( x + iDepth * 150, y,
					 "%s (%.2f us, %d calls)",
					 pNode->GetName(),
					 iDepth == 0 ? pNode->GetChildrenTime() : pNode->GetTime(),
					 pNode->GetCalls() );
	}
	else if ( sc_debug_show_prof.GetInt() == 2 )
	{
		DRAW_STRING( x + iDepth * 150, y,
					 "%s (%.2f ns, %d calls)",
					 pNode->GetName(),
					 iDepth == 0 ? PROF_TIME_NANOSEC( pNode->GetChildrenTime() ) : PROF_TIME_NANOSEC( pNode->GetTime() ),
					 pNode->GetCalls() );
	}
	else if ( sc_debug_show_prof.GetInt() == 3 )
	{
		DRAW_STRING( x + iDepth * 150, y,
					 "%s (%.2f ms, %d calls)",
					 pNode->GetName(),
					 iDepth == 0 ? PROF_TIME_MILLISEC( pNode->GetChildrenTime() ) : PROF_TIME_MILLISEC( pNode->GetTime() ),
					 pNode->GetCalls() );
	}

	y += offset;

	for ( CProfileNode *pChild : pNode->GetChildren() )
		y = DrawProfilesTree( pChild, x, y, offset, iDepth + 1 );

	return y;
}
#endif

void CLocalPlayer::DrawProfile( void )
{
#ifdef PROF_ENABLED
	if ( !sc_debug_show_prof.GetBool() )
		return;

	if ( Globals::cls->state != ca_active )
		return;

	const int x = int( float( Globals::gameutils->GetScreenWidth() ) * 0.015625f );
	const int y = int( float( Globals::gameutils->GetScreenHeight() ) * 0.05f );
	const int offset = sc_debug_new_line_height.GetInt();

	Globals::gameutils->DrawSetTextColor( 180.f / 255.f, 220.f / 255.f, 255.f / 255.f );

	DrawProfilesTree( &gProfileRoot, x, y, offset + 5 );
#endif
}

static CLocalPlayer gLocalPlayer;
CLocalPlayer *LocalPlayer( void )
{
	return &gLocalPlayer;
}

//-----------------------------------------------------------------------------
// CClientWeapon implementation
//-----------------------------------------------------------------------------

CClientWeapon::CClientWeapon()
{
	m_iCurrentWeaponID = 0;
	m_bCurrentWeaponCustom = false;
	m_bForceWeaponReload = false;
}

weapon_data_t *CClientWeapon::GetWeaponDataList( void )
{
	return LocalPlayer()->GetLocalStateStatic()->weapondata;
}

weapon_data_t *CClientWeapon::GetWeaponData( void )
{
	return GetWeaponDataInternal( m_iCurrentWeaponID );
}

weapon_data_t *CClientWeapon::GetWeaponData( int iWeaponID )
{
	return GetWeaponDataInternal( iWeaponID );
}

bool CClientWeapon::IsCustom( void )
{
	return m_bCurrentWeaponCustom;
}

int CClientWeapon::Clip( void )
{
	if ( m_bCurrentWeaponCustom )
	{
		WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

		if ( !pWeapon )
			return -1;

		return pWeapon->iClip;
	}

	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1;

	return pWeaponData->m_iClip;
}

int CClientWeapon::PrimaryAmmo( void )
{
	if ( m_bCurrentWeaponCustom || m_iCurrentWeaponID == WEAPON_RPG )
	{
		WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

		if ( !pWeapon )
			return -1;

		return Globals::inventory->GetPrimaryAmmoCount( pWeapon );
	}

	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1;

	switch ( m_iCurrentWeaponID )
	{
	case WEAPON_M249:
	case WEAPON_M16:
	{
		int iAmmoType = pWeaponData->iuser1;
		int ammo = Globals::inventory->CountAmmo( iAmmoType );

		if ( !ammo )
		{
			int clip = pWeaponData->m_iClip;

			if ( clip != -1 )
				return -clip;

			return 0;
		}

		return ammo;
	}
	}

	return pWeaponData->iuser2;
}

int CClientWeapon::SecondaryAmmo( void )
{
	if ( m_bCurrentWeaponCustom || m_iCurrentWeaponID == WEAPON_MP5 )
	{
		WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

		if ( !pWeapon )
			return -1;

		return Globals::inventory->GetSecondaryAmmoCount( pWeapon );
	}

	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1;

	switch ( m_iCurrentWeaponID )
	{
	//case WEAPON_MP5:
	case WEAPON_M16:
		return pWeaponData->iuser3;
	}

	return 0;
}

int CClientWeapon::PrimaryAmmoType( void )
{
	if ( m_bCurrentWeaponCustom )
	{
		WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

		if ( !pWeapon )
			return -1;

		return pWeapon->iAmmoType;
	}

	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1;

	int iAmmoType = pWeaponData->iuser1;

	if ( iAmmoType == 255 )
		return -1;

	return iAmmoType;
}

int CClientWeapon::SecondaryAmmoType( void )
{
	WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

	if ( !pWeapon )
		return -1;

	return pWeapon->iAmmo2Type;
}

float CClientWeapon::GetNextPrimaryAttack( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1.f;

	if ( m_bCurrentWeaponCustom )
		return pWeaponData->m_flNextPrimaryAttack;

	return static_cast<float>( Globals::localplayer->Time() ) + pWeaponData->m_flNextPrimaryAttack;
}

float CClientWeapon::GetNextSecondaryAttack( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1.f;

	if ( m_bCurrentWeaponCustom )
		return pWeaponData->m_flNextSecondaryAttack;

	return static_cast<float>( Globals::localplayer->Time() ) + pWeaponData->m_flNextSecondaryAttack;
}

float CClientWeapon::GetWeaponIdle( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return -1.f;

	float flCurrentTime = static_cast<float>( Globals::localplayer->Time() );

	if ( m_bCurrentWeaponCustom )
	{
		if ( pWeaponData->m_fInReload )
			return flCurrentTime + Globals::localplayer->GetLocalStateStatic()->client.m_flNextAttack;

		return pWeaponData->m_flTimeWeaponIdle;
	}

	if ( pWeaponData->m_fInReload )
		return flCurrentTime + Globals::localplayer->GetLocalStateStatic()->client.m_flNextAttack;

	return flCurrentTime + pWeaponData->m_flTimeWeaponIdle;
}

bool CClientWeapon::CanPrimaryAttack( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return false;

	float flCurrentTime = static_cast<float>( Globals::localplayer->Time() );

	if ( m_bCurrentWeaponCustom )
		return flCurrentTime >= pWeaponData->m_flNextPrimaryAttack;

	switch ( m_iCurrentWeaponID )
	{
	case WEAPON_GAUSS:
		return pWeaponData->fuser4 == 0.f && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextPrimaryAttack );

	case WEAPON_RPG:
		return pWeaponData->fuser1 == 0.f && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextPrimaryAttack );

	case WEAPON_HANDGRENADE:
		return pWeaponData->fuser1 >= 0.f && pWeaponData->fuser2 >= 0.f && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextPrimaryAttack );

	case WEAPON_DISPLACER:
		return pWeaponData->fuser1 == 0.f;
	}

	return flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextPrimaryAttack );
}

bool CClientWeapon::CanSecondaryAttack( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return false;

	float flCurrentTime = static_cast<float>( Globals::localplayer->Time() );

	if ( m_bCurrentWeaponCustom )
		return flCurrentTime >= pWeaponData->m_flNextSecondaryAttack;

	switch ( m_iCurrentWeaponID )
	{
	case WEAPON_M16:
		return pWeaponData->fuser1 == 0.f && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextSecondaryAttack );

	case WEAPON_HANDGRENADE:
		return pWeaponData->fuser1 < 0.f && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextSecondaryAttack );

	case WEAPON_SATCHEL:
		return pWeaponData->iuser2 != 0 && flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextSecondaryAttack );

	case WEAPON_DISPLACER:
		return pWeaponData->fuser1 == 0.f;
	}

	return flCurrentTime >= ( flCurrentTime + pWeaponData->m_flNextSecondaryAttack );
}

bool CClientWeapon::IsReloading( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return true;

	if ( m_bCurrentWeaponCustom )
		return pWeaponData->m_fInReload;

	switch ( m_iCurrentWeaponID )
	{
	case WEAPON_SHOTGUN:
		return pWeaponData->fuser4 != 0.f;

	case WEAPON_SPORE_LAUNCHER:
		return pWeaponData->iuser3 != 0;
	}

	return pWeaponData->m_fInReload;
}

bool CClientWeapon::IsInZoom( void )
{
	weapon_data_t *pWeaponData = GetWeaponDataInternal( m_iCurrentWeaponID );

	if ( !pWeaponData )
		return false;

	if ( m_iCurrentWeaponID == WEAPON_DESERT_EAGLE || m_iCurrentWeaponID == WEAPON_SHOTGUN )
		return false;

	return pWeaponData->m_fInZoom;
}

void CClientWeapon::Reload( void )
{
	m_bForceWeaponReload = true;
}

void CClientWeapon::Drop( void )
{
	WEAPON *pWeapon = Globals::inventory->GetWeapon( m_iCurrentWeaponID );

	if ( pWeapon != NULL )
		Globals::inventory->DropWeapon( pWeapon );
}

weapon_data_t *CClientWeapon::GetWeaponDataInternal( int iWeaponID )
{
	if ( iWeaponID <= WEAPON_NONE )
		return NULL;

	return Globals::localplayer->GetLocalStateStatic()->weapondata + iWeaponID;
}

static CClientWeapon gClientWeapon;
CClientWeapon *ClientWeapon( void )
{
	return &gClientWeapon;
}

//-----------------------------------------------------------------------------
// CInventory implementation
//-----------------------------------------------------------------------------

CInventory::CInventory()
{
}

int CInventory::GetMaxWeaponSlots()
{
	return Globals::weaponsresource->GetMaxWeaponSlots();
}

int CInventory::GetMaxWeaponPositions()
{
	return Globals::weaponsresource->GetMaxWeaponPositions();
}

WEAPON *CInventory::GetWeapon( int iWeaponID )
{
	const int iMaxSlots = GetMaxWeaponSlots();
	const int iMaxPositions = GetMaxWeaponPositions();

	for ( int i = 0; i < iMaxSlots; i++ )
	{
		for ( int j = 0; j < iMaxPositions; j++ )
		{
			WEAPON *pWeapon = GetWeapon( i, j );
			if ( pWeapon && pWeapon->iId == iWeaponID )
			{
				return pWeapon;
			}
		}
	}

	return NULL;
}

WEAPON *CInventory::GetWeapon( int iSlot, int iPos )
{
	return Globals::weaponsresource->GetWeaponSlot( iSlot, iPos );
}

WEAPON *CInventory::GetWeapon( const char *pszWeaponName )
{
	const int iMaxSlots = Globals::weaponsresource->GetMaxWeaponSlots();
	const int iMaxPositions = Globals::weaponsresource->GetMaxWeaponPositions();

	for ( int i = 0; i < iMaxSlots; i++ )
	{
		for ( int j = 0; j < iMaxPositions; j++ )
		{
			WEAPON *pWeapon = GetWeapon( i, j );
			if ( pWeapon && !stricmp( pWeapon->szName, pszWeaponName ) )
			{
				return pWeapon;
			}
		}
	}

	return NULL;
}

WEAPON *CInventory::GetFirstPos( int iSlot )
{
	return Globals::weaponsresource->GetFirstPos( iSlot );
}

WEAPON *CInventory::GetNextActivePos( int iSlot, int iSlotPos )
{
	return Globals::weaponsresource->GetNextActivePos( iSlot, iSlotPos );
}

bool CInventory::HasAmmo( WEAPON *pWeapon )
{
	return Globals::weaponsresource->HasAmmo( pWeapon );
}

int CInventory::CountAmmo( int iAmmoType )
{
	return Globals::weaponsresource->CountAmmo( iAmmoType );
}

int CInventory::GetPrimaryAmmoCount( WEAPON *pWeapon )
{
	if ( pWeapon->iAmmoType == -1 )
		return 0;

	int ammo = CountAmmo( pWeapon->iAmmoType );

	if ( !ammo )
	{
		int clip = pWeapon->iClip;

		if ( clip != -1 )
			return -clip;

		return 0;
	}

	return ammo;
}

int CInventory::GetSecondaryAmmoCount( WEAPON *pWeapon )
{
	if ( pWeapon->iAmmo2Type == -1 )
		return 0;

	int ammo = CountAmmo( pWeapon->iAmmo2Type );

	if ( !ammo )
	{
		int count = pWeapon->iCount;

		if ( count != -1 )
			return -count;

		return 0;
	}

	return ammo;
}

void CInventory::SelectWeapon( WEAPON *pWeapon )
{
	Globals::cl_enginefuncs->pfnClientCmd( pWeapon->szName );
}

void CInventory::SelectWeapon( const char *pszWeaponName )
{
	Globals::cl_enginefuncs->pfnClientCmd( const_cast<char *>( pszWeaponName ) );
}

void CInventory::DropWeapon( WEAPON *pWeapon )
{
	char rgszCmd[ 140 ];

	snprintf( rgszCmd, Q_ARRAYSIZE( rgszCmd ), "drop %s\n", pWeapon->szName );
	rgszCmd[ Q_ARRAYSIZE( rgszCmd ) - 1 ] = '\0';

	Globals::cl_enginefuncs->pfnClientCmd( rgszCmd );
}

void CInventory::DropWeapon( const char *pszWeaponName )
{
	char rgszCmd[ 140 ];

	snprintf( rgszCmd, Q_ARRAYSIZE( rgszCmd ), "drop %s\n", pszWeaponName );
	rgszCmd[ Q_ARRAYSIZE( rgszCmd ) - 1 ] = '\0';

	Globals::cl_enginefuncs->pfnClientCmd( rgszCmd );
}

static CInventory gInventory;
CInventory *Inventory()
{
	return &gInventory;
}
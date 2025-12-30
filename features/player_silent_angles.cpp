// SvenInt (c) Sw1ft
// player_silent_angles.cpp

#include "stdafx.h"
#include "player_silent_angles.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

static EventHookFn ORIG_EventHook_FireGlock1 = NULL;
static EventHookFn ORIG_EventHook_FireGlock2 = NULL;
static EventHookFn ORIG_EventHook_FireShotGunSingle = NULL;
static EventHookFn ORIG_EventHook_FireShotGunDouble = NULL;
static EventHookFn ORIG_EventHook_FireMP5 = NULL;
static EventHookFn ORIG_EventHook_FirePython = NULL;
static EventHookFn ORIG_EventHook_FireDeagle = NULL;
static EventHookFn ORIG_EventHook_FireGauss = NULL;
static EventHookFn ORIG_EventHook_Uzi = NULL;
static EventHookFn ORIG_EventHook_UziAkimbo = NULL;
static EventHookFn ORIG_EventHook_WeaponCustom = NULL;
static EventHookFn ORIG_EventHook_Minigun = NULL;
static EventHookFn ORIG_EventHook_SniperRifle = NULL;
static EventHookFn ORIG_EventHook_M249 = NULL;
static EventHookFn ORIG_EventHook_M16 = NULL;
static EventHookFn ORIG_EventHook_FireShockRifle = NULL;
static EventHookFn ORIG_EventHook_DisplacerSpin = NULL;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSilentAngles, silentangles, "Player", "Silent Angles" );

//-----------------------------------------------------------------------------
// Event Hooks
//-----------------------------------------------------------------------------

static void HOOKED_EventHook_FireGlock1( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireGlock1( args );
}

static void HOOKED_EventHook_FireGlock2( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireGlock2( args );
}

static void HOOKED_EventHook_FireShotGunSingle( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireShotGunSingle( args );
}

static void HOOKED_EventHook_FireShotGunDouble( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireShotGunDouble( args );
}

static void HOOKED_EventHook_FireMP5( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireMP5( args );
}

static void HOOKED_EventHook_FirePython( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FirePython( args );
}

static void HOOKED_EventHook_FireDeagle( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireDeagle( args );
}

static void HOOKED_EventHook_FireGauss( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireGauss( args );
}

static void HOOKED_EventHook_Uzi( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_Uzi( args );
}

static void HOOKED_EventHook_UziAkimbo( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_UziAkimbo( args );
}

static void HOOKED_EventHook_WeaponCustom( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_WeaponCustom( args );
}

static void HOOKED_EventHook_Minigun( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_Minigun( args );
}

static void HOOKED_EventHook_SniperRifle( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_SniperRifle( args );
}

static void HOOKED_EventHook_M249( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_M249( args );
}

static void HOOKED_EventHook_M16( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_M16( args );
}

static void HOOKED_EventHook_FireShockRifle( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_FireShockRifle( args );
}

static void HOOKED_EventHook_DisplacerSpin( event_args_t *args )
{
	THIS_FEATURE()->OverrideEventWeaponAngles( args->entindex, *reinterpret_cast<Vector *>( args->angles ) );

	ORIG_EventHook_DisplacerSpin( args );
}

//-----------------------------------------------------------------------------
// Override angles of event weapon
//-----------------------------------------------------------------------------

void CSilentAngles::OverrideEventWeaponAngles( int entindex, Vector &angles )
{
	if ( !m_bOverrideVirtualVA )
		return;

	if ( entindex != localplayer->GetPlayerIndex() )
		return;

	angles = m_vecLastVirtualVA;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

void CSilentAngles::Process( usercmd_t *cmd )
{
	Vector va;
	cl_enginefuncs->GetViewAngles( va );

	m_bOverrideVirtualVA = m_bSetAngles ? ( va != m_vecAngles ) : ( va != cmd->viewangles );
	m_vecLastVirtualVA = m_bSetAngles ? m_vecAngles : cmd->viewangles;

	if ( !m_bSetAngles )
		return;

	if ( !CanSetAngles( cmd, m_fAbortFlags ) )
	{
		m_bSetAngles = false;
		m_bOverrideVirtualVA = false;
		return;
	}

	FixMoveStart( cmd );

	cmd->viewangles = m_vecAngles;
	m_flPlayerModelPitch = m_vecAngles.x / -3.f;

	FixMoveEnd( cmd );
}

//-----------------------------------------------------------------------------
// FixPlayerModelLean
//-----------------------------------------------------------------------------

void CSilentAngles::FixPlayerModelLean( void )
{
	if ( !m_bSetAngles )
		return;

	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
	if ( pLocal == NULL )
		return;

	pLocal->angles.x = m_flPlayerModelPitch;
	pLocal->curstate.angles.x = m_flPlayerModelPitch;
	pLocal->prevstate.angles.x = m_flPlayerModelPitch;
	pLocal->latched.prevangles.x = m_flPlayerModelPitch;
}

//-----------------------------------------------------------------------------
// FixMoveStart
//-----------------------------------------------------------------------------

void CSilentAngles::FixMoveStart( usercmd_t *cmd )
{
	m_flForwardMove = cmd->forwardmove;
	m_flSideMove = cmd->sidemove;
	m_flUpMove = cmd->upmove;

	if ( playermove->iuser1() == 0 )
		cl_enginefuncs->pfnAngleVectors( Vector( 0.f, cmd->viewangles.y, 0.f ), vecViewForward, vecViewRight, vecViewUp );
	else
		cl_enginefuncs->pfnAngleVectors( cmd->viewangles, vecViewForward, vecViewRight, vecViewUp );
}

//-----------------------------------------------------------------------------
// FixMoveEnd
//-----------------------------------------------------------------------------

void CSilentAngles::FixMoveEnd( usercmd_t *cmd )
{
	NormalizeAngles( cmd->viewangles );

	if ( playermove->iuser1() == 0 )
		cl_enginefuncs->pfnAngleVectors( Vector( 0.f, cmd->viewangles.y, 0.f ), vecAimForward, vecAimRight, vecAimUp );
	else
		cl_enginefuncs->pfnAngleVectors( cmd->viewangles, vecAimForward, vecAimRight, vecAimUp );

	Vector forwardmove_normalized = vecViewForward * m_flForwardMove;
	Vector sidemove_normalized = vecViewRight * m_flSideMove;
	Vector upmove_normalized = vecViewUp * m_flUpMove;

	cmd->forwardmove = DotProduct( forwardmove_normalized, vecAimForward ) + DotProduct( sidemove_normalized, vecAimForward ) + DotProduct( upmove_normalized, vecAimForward );
	cmd->sidemove = DotProduct( forwardmove_normalized, vecAimRight ) + DotProduct( sidemove_normalized, vecAimRight ) + DotProduct( upmove_normalized, vecAimRight );
	cmd->upmove = DotProduct( forwardmove_normalized, vecAimUp ) + DotProduct( sidemove_normalized, vecAimUp ) + DotProduct( upmove_normalized, vecAimUp );

	Vector move( cmd->forwardmove, cmd->sidemove, cmd->upmove );
	const float flSpeed = sqrtf( move.x * move.x + move.y * move.y );

	Vector vecMove, vecRealView( cmd->viewangles );

	VectorAngles( move, vecMove );

	const float theta = VEC_DEG2RAD( cmd->viewangles.y - vecRealView.y + vecMove.y );

	cmd->forwardmove = cosf( theta ) * flSpeed;

	if ( cmd->viewangles.x >= 90.f || cmd->viewangles.x <= -90.f )
		cmd->forwardmove *= -1.f;

	cmd->sidemove = sinf( theta ) * flSpeed;
}

//-----------------------------------------------------------------------------
// CanSetAngles
//-----------------------------------------------------------------------------

bool CSilentAngles::CanSetAngles( usercmd_t *cmd, int fAbortFlags )
{
	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_NOT_WALKING ) )
	{
		if ( localplayer->GetMoveType() == MOVETYPE_FLY )
		{
			if ( !localplayer->GetVelocity().IsZeroFast() )
				return false;
		}
		else if ( localplayer->GetMoveType() != MOVETYPE_WALK )
		{
			return false;
		}
	}

	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_IN_WATER ) && localplayer->GetWaterLevel() > WL_FEET )
		return false;

	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_USE ) && cmd->buttons & IN_USE )
		return false;
	
	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_SPRAY ) && cmd->impulse == 201 )
		return false;
	
	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_FIRING ) && UTIL_IsFiring( cmd ) )
		return false;
	
	if ( ( fAbortFlags & SILENT_ANGLES_ABORT_LONGJUMP ) && UTIL_IsBusyWithLongJump( cmd ) )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSilentAngles::CSilentAngles( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bOverrideVirtualVA = false;
	m_bSetAngles = false;
	m_fAbortFlags = -1;
	m_flPlayerModelPitch = 0.f;

	m_flForwardMove = m_flSideMove = m_flUpMove = 0.f;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CSilentAngles::PostLoad( void )
{
	m_eventhooks.push_back( gamehooks->HookEvent( "events/glock1.sc", HOOKED_EventHook_FireGlock1, &ORIG_EventHook_FireGlock1 ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/glock2.sc", HOOKED_EventHook_FireGlock2, &ORIG_EventHook_FireGlock2 ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/shotgun1.sc", HOOKED_EventHook_FireShotGunSingle, &ORIG_EventHook_FireShotGunSingle ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/shotgun2.sc", HOOKED_EventHook_FireShotGunDouble, &ORIG_EventHook_FireShotGunDouble ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/mp5.sc", HOOKED_EventHook_FireMP5, &ORIG_EventHook_FireMP5 ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/python.sc", HOOKED_EventHook_FirePython, &ORIG_EventHook_FirePython ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/deagle.sc", HOOKED_EventHook_FireDeagle, &ORIG_EventHook_FireDeagle ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/gauss.sc", HOOKED_EventHook_FireGauss, &ORIG_EventHook_FireGauss ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/uzi.sc", HOOKED_EventHook_Uzi, &ORIG_EventHook_Uzi ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/uziakimbo.sc", HOOKED_EventHook_UziAkimbo, &ORIG_EventHook_UziAkimbo ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/weapon_custom.sc", HOOKED_EventHook_WeaponCustom, &ORIG_EventHook_WeaponCustom ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/minigun.sc", HOOKED_EventHook_Minigun, &ORIG_EventHook_Minigun ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/sniperrifle.sc", HOOKED_EventHook_SniperRifle, &ORIG_EventHook_SniperRifle ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/m249.sc", HOOKED_EventHook_M249, &ORIG_EventHook_M249 ) );
	m_eventhooks.push_back( gamehooks->HookEvent( "events/m16a2.sc", HOOKED_EventHook_M16, &ORIG_EventHook_M16 ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CSilentAngles::Unload( void )
{
	for ( const DetourHandle_t &detour : m_eventhooks )
	{
		Detours()->RemoveDetour( detour );
	}
}
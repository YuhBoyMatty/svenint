// SvenInt (c) Sw1ft
// player_aim.cpp

#include "stdafx.h"
#include "modules/scripts.h"
#include "player_aim.h"
#include "player_silent_angles.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Features gamedata
//-----------------------------------------------------------------------------

namespace FeaturesGameData
{
	namespace Patterns
	{
		namespace Client
		{
			DEFINE_PATTERNS_2( V_PunchAxis,
							   "5.25",
							   "8B 44 24 04 F3 0F 10 44 24 08",
							   "5.11",
							   "D9 ? ? ? 8B 44 24 04 D9 ? ? ? ? ? ? C3" );
		}
	}
}

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAim, aim, "Player", "Aim" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_aim, "Aim-related features" );

ConVar sc_aimbot_scripts_filter_targets( "sc_aimbot_scripts_filter_targets", "0", FCVAR_EXTDLL, "Enable scripts callback to filter valid aimbot targets" );

//-----------------------------------------------------------------------------
// Execute aimbot
//-----------------------------------------------------------------------------

bool CAim::Aimbot( usercmd_t *cmd, int iMode, bool bChangeAnglesBack, bool &bAnglesChanged )
{
	constexpr int fAbortFlags = ~( SILENT_ANGLES_ABORT_IN_WATER | SILENT_ANGLES_ABORT_FIRING );

	bAnglesChanged = false;

	if ( iMode == 0 || iMode >= 2 && !Features::silentangles->CanSetAngles( cmd, fAbortFlags ) )
		return false;

	WEAPON *pWeapon;
	int iWeaponID = WEAPON_NONE, iClip;
	bool bUsingMountedGun = IsUsingMountedGun();

	// We're dead
	if ( localplayer->IsDead() )
		return false;

	if ( !bUsingMountedGun )
	{
		// Don't have any weapon
		if ( ( iWeaponID = localplayer->GetCurrentWeaponID() ) == WEAPON_NONE )
			return false;

		// Take out a weapon rn + exception for displacer
		if ( !localplayer->CanAttack() && iWeaponID != WEAPON_DISPLACER )
			return false;

		// Don't use crowbar, medkit etc..
		if ( !IsHoldingAppropriateWeapon( iWeaponID ) )
			return false;

		// Whoops.. what happened with the weapon?
		if ( ( pWeapon = inventory->GetWeapon( iWeaponID ) ) == NULL )
			return false;

		// We're empty
		if ( !inventory->HasAmmo( pWeapon ) )
			return false;

		iClip = clientweapon->Clip();

		// Going to reload
		if ( CheckReload( iWeaponID, iClip, cmd ) )
			return false;
	}

	if ( iMode == 3 ) // ragebot
	{
		if ( bUsingMountedGun )
		{
			CEntity *pTarget = FindBestTarget();
			if ( pTarget == NULL )
				return false;

			Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity;
			Vector vecDir = vecTargetPoint - localplayer->GetEyePosition();

			Vector vAngles;
			DirectionToAngles( vecDir, vAngles );

			//if ( bChangeAnglesBack )
			//	SetChangeAnglesBack();

			Features::silentangles->SetAngles( vAngles, fAbortFlags );
			cmd->buttons |= IN_ATTACK;

			bAnglesChanged = true;
			return true;
		}
		else
		{
			bool bStillFiring = IsStillFiring( iWeaponID, cmd );
			bool bCanPrimaryAttack = clientweapon->CanPrimaryAttack();
			bool bCanSecondaryAttack = clientweapon->CanSecondaryAttack();

			if ( !bStillFiring && !bCanPrimaryAttack && !bCanSecondaryAttack )
				return false;

			CEntity *pTarget = FindBestTarget();

			if ( pTarget == NULL || !IsTargetCanBeHurted( (EEntityClassID)pTarget->m_classInfo.id, iWeaponID ) )
				return false;
			
			Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity;
			Vector vecDir = vecTargetPoint - localplayer->GetEyePosition();

			if ( bStillFiring )
			{
				Vector vAngles;
				DirectionToAngles( vecDir, vAngles );

				//if ( bChangeAnglesBack )
				//	SetChangeAnglesBack();

				Features::silentangles->SetAngles( vAngles, fAbortFlags );

				bAnglesChanged = true;

				return true;
			}
			else
			{
				float flDistance = vecDir.Length();
				if ( !IsDistanceAllowsUseWeapon( iWeaponID, flDistance ) )
					return false;
				
				int fAttackButton = ConcludeAttackButton( iWeaponID, iClip, flDistance );

				if ( !( fAttackButton == IN_ATTACK && bCanPrimaryAttack ) && !( fAttackButton == IN_ATTACK2 && bCanSecondaryAttack ) )
					return false;
				
				Vector vAngles;
				DirectionToAngles( vecDir, vAngles );

				if ( bChangeAnglesBack )
					SetChangeAnglesBack();

				Features::silentangles->SetAngles( vAngles, fAbortFlags );
				cmd->buttons |= fAttackButton;

				bAnglesChanged = true;

				return true;
			}
		}
	}
	else if ( iMode < 3 ) // (silent) aimbot
	{
		if ( bUsingMountedGun && cmd->buttons & IN_ATTACK )
		{
			CEntity *pTarget = FindBestTarget();
			if ( pTarget == NULL )
				return false;
			
			Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity;
			Vector vecDir = vecTargetPoint - localplayer->GetEyePosition();

			Vector vAngles;
			DirectionToAngles( vecDir, vAngles );

			if ( iMode == 2 )
			{
				Features::silentangles->SetAngles( vAngles, fAbortFlags );
				bAnglesChanged = true;
			}
			else
			{
				if ( bChangeAnglesBack )
					SetChangeAnglesBack();

				//g_bYawChanged = true;

				cmd->viewangles = vAngles;
				cl_enginefuncs->SetViewAngles( vAngles );
			}

			return true;
		}
		else if ( IsStillFiring( iWeaponID, cmd ) || IsFiring( iWeaponID, cmd ) )
		{
			CEntity *pTarget = FindBestTarget();

			if ( pTarget == NULL || !IsTargetCanBeHurted( (EEntityClassID)pTarget->m_classInfo.id, iWeaponID ) )
				return false;
			
			Vector vecTargetPoint = m_vecTargetPoint + pTarget->m_vecVelocity;
			Vector vecDir = vecTargetPoint - localplayer->GetEyePosition();

			float flDistance = vecDir.Length();
			if ( !IsDistanceAllowsUseWeapon( iWeaponID, flDistance ) )
				return false;
			
			Vector vAngles;
			DirectionToAngles( vecDir, vAngles );

			if ( iMode == 2 )
			{
				Features::silentangles->SetAngles( vAngles, fAbortFlags );

				bAnglesChanged = true;
			}
			else
			{
				if ( bChangeAnglesBack )
					SetChangeAnglesBack();

				//g_bYawChanged = true;

				cmd->viewangles = vAngles;
				cl_enginefuncs->SetViewAngles( vAngles );
			}

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// No recoil
//-----------------------------------------------------------------------------

void CAim::NoRecoil( usercmd_t *cmd )
{
	if ( !m_pNoRecoil->GetBool() )
		return;

	if ( localplayer->IsDead() )
		return;

	if ( !localplayer->HasWeapon() )
		return;

	if ( !localplayer->CanAttack() )
		return;

	if ( clientweapon->IsReloading() )
		return;

	if ( cmd->buttons & IN_ATTACK )
	{
		if ( clientweapon->IsCustom() || ( !clientweapon->IsCustom() && clientweapon->CanPrimaryAttack() ) )
		{
			Vector vecNoRecoil = m_vecPunchAngle + m_vecEvPunchAngle;

			if ( Features::silentangles->IsSet() )
				Features::silentangles->SubtractAngles( vecNoRecoil );
			else
				cmd->viewangles -= vecNoRecoil;
		}
	}
	else if ( cmd->buttons & IN_ATTACK2 )
	{
		if ( clientweapon->IsCustom() || ( !clientweapon->IsCustom() && clientweapon->CanSecondaryAttack() ) )
		{
			Vector vecNoRecoil = m_vecPunchAngle + m_vecEvPunchAngle;

			if ( Features::silentangles->IsSet() )
				Features::silentangles->SubtractAngles( vecNoRecoil );
			else
				cmd->viewangles -= vecNoRecoil;
		}
	}
}

//-----------------------------------------------------------------------------
// SetChangeAnglesBack
//-----------------------------------------------------------------------------

void CAim::SetChangeAnglesBack( void )
{
	cl_enginefuncs->GetViewAngles( m_vecChangeAnglesTarget );
	m_bChangeAnglesBack = true;
}

//-----------------------------------------------------------------------------
// IsUsingMountedGun
//-----------------------------------------------------------------------------

bool CAim::IsUsingMountedGun( void )
{
	return gpHideHUD != NULL && ( *gpHideHUD & HIDEHUD_WEAPONS );
}

//-----------------------------------------------------------------------------
// CheckReload
//-----------------------------------------------------------------------------

bool CAim::CheckReload( int iWeaponID, int iClip, usercmd_t *cmd )
{
	if ( clientweapon->IsReloading() )
		return true;

	if ( iClip == 0 )
	{
		if ( iWeaponID == WEAPON_RPG )
		{
			// Can't reload while using laser homing
			if ( clientweapon->GetWeaponData()->iuser4 && clientweapon->GetWeaponData()->fuser1 != 0.f )
			{
				return false;
			}
		}

		cmd->buttons |= IN_RELOAD;
		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// IsHoldingAppropriateWeapon
//-----------------------------------------------------------------------------

bool CAim::IsHoldingAppropriateWeapon( int iWeaponID )
{
	switch ( iWeaponID )
	{
	case WEAPON_CROWBAR:
	case WEAPON_WRENCH:
	case WEAPON_MEDKIT:
	case WEAPON_HANDGRENADE:
	case WEAPON_TRIPMINE:
	case WEAPON_SATCHEL:
	case WEAPON_BARNACLE_GRAPPLE:
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// IsDistanceAllowsUseWeapon
//-----------------------------------------------------------------------------

bool CAim::IsDistanceAllowsUseWeapon( int iWeaponID, float flDistance )
{
	switch ( iWeaponID )
	{
	case WEAPON_RPG:
		if ( flDistance <= 340.f )
			return false;

		return true;

	case WEAPON_CROSSBOW:
		if ( flDistance <= 128.f )
			return false;

		return true;

	case WEAPON_EGON:
		if ( flDistance <= 128.f || flDistance > 2048.f )
			return false;

		return true;

	case WEAPON_DISPLACER:
		if ( flDistance <= 350.f )
			return false;

		return true;

	case WEAPON_SPORE_LAUNCHER:
		if ( flDistance <= 500.f || flDistance > 800.f )
			return false;

		return true;

	case WEAPON_SHOTGUN:
	case WEAPON_SNARK:
		if ( flDistance > 500.f )
			return false;

		return true;
	}

	return true;
}

//-----------------------------------------------------------------------------
// IsTargetCanBeHurted
//-----------------------------------------------------------------------------

bool CAim::IsTargetCanBeHurted( EEntityClassID iClassID, int iWeaponID )
{
	switch ( iClassID )
	{
	case CLASS_NPC_TENTACLE:
	case CLASS_NPC_DESTROYED_OSPREY:
	case CLASS_NPC_SPORE_AMMO:
	case CLASS_NPC_PIT_WORM:
	case CLASS_NPC_GENEWORM:
	case CLASS_NPC_MORTAR:
		return false;

	case CLASS_NPC_GARGANTUA:
		if ( iWeaponID == WEAPON_EGON || iWeaponID == WEAPON_RPG )
			return true;

		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// ConcludeAttackButton
//-----------------------------------------------------------------------------

int CAim::ConcludeAttackButton( int iWeaponID, int iClip, float flDistance )
{
	switch ( iWeaponID )
	{
	case WEAPON_SHOTGUN:
		if ( flDistance <= 256.f && iClip > 1 )
			return IN_ATTACK2;

		return IN_ATTACK;

	case WEAPON_HORNETGUN:
		return IN_ATTACK2;
	}

	return IN_ATTACK;
}

//-----------------------------------------------------------------------------
// IsFiring
//-----------------------------------------------------------------------------

bool CAim::IsFiring( int iWeaponID, usercmd_t *cmd )
{
	switch ( iWeaponID )
	{
	case WEAPON_DESERT_EAGLE:
	case WEAPON_MP5:
	case WEAPON_M16:
	case WEAPON_CROSSBOW:
	case WEAPON_RPG:
	case WEAPON_EGON:
	case WEAPON_SNIPER_RIFLE:
	case WEAPON_M249:
	case WEAPON_DISPLACER:
		if ( cmd->buttons & IN_ATTACK2 )
			return false;

		break;

		/*
			case WEAPON_GAUSS:
				if ( clientweapon->GetWeaponData()->fuser4 > 0.f )
				{
					if ( localplayer->ButtonLast() & IN_ATTACK2 )
					{
						if ( !(cmd->buttons & IN_ATTACK2) )
							return true;
					}
					else if ( localplayer->ButtonLast() & IN_ALT1 )
					{
						if ( !(cmd->buttons & IN_ALT1) )
							return true;
					}
					else if ( clientweapon->GetWeaponData()->fuser4 == 1.f )
					{
						return true;
					}

					return false;
				}
				else if ( cmd->buttons & IN_ATTACK2 )
				{
					return false;
				}

				break;
		*/
	}

	if ( cmd->buttons & ( IN_ATTACK | IN_ATTACK2 ) )
	{
		if ( cmd->buttons & IN_ATTACK )
		{
			if ( clientweapon->CanPrimaryAttack() )
				return true;
		}
		else
		{
			if ( clientweapon->CanSecondaryAttack() )
				return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// IsStillFiring
//-----------------------------------------------------------------------------

bool CAim::IsStillFiring( int iWeaponID, usercmd_t *cmd )
{
	// To hit a target, we still need to aim after firing from a weapon

	switch ( iWeaponID )
	{
	case WEAPON_M16:
		if ( clientweapon->GetWeaponData()->fuser2 != 0.f )
			return true;

		break;

	case WEAPON_GAUSS:
		if ( clientweapon->GetWeaponData()->fuser4 > 0.f )
		{
			if ( localplayer->ButtonLast() & IN_ATTACK2 )
			{
				if ( !( cmd->buttons & IN_ATTACK2 ) )
					return true;
			}
			else if ( localplayer->ButtonLast() & IN_ALT1 )
			{
				if ( !( cmd->buttons & IN_ALT1 ) )
					return true;
			}
			else if ( clientweapon->GetWeaponData()->fuser4 == 1.f )
			{
				return true;
			}

			return false;
		}
		else if ( cmd->buttons & IN_ATTACK2 )
		{
			return false;
		}

		break;

	case WEAPON_RPG:
		if ( clientweapon->GetWeaponData()->iuser4 && clientweapon->GetWeaponData()->fuser1 != 0.f )
			return true;

		break;

	case WEAPON_DISPLACER:
		if ( clientweapon->GetWeaponData()->fuser1 == 1.f )
			return true;

		break;
	}

	return false;
}

//-----------------------------------------------------------------------------
// FindBestTarget
//-----------------------------------------------------------------------------

CEntity *CAim::FindBestTarget( void )
{
	Vector va;
	Vector vForward;

	std::vector<unsigned char> vHitboxes;

	float flDistanceSqr = FLT_MAX;
	float flMaxDistanceSqr = M_SQR( m_pDistance->GetFloat() );

	CEntity *pTarget = NULL;
	CEntity *pEnts = Features::entitylist->GetList();

	cl_entity_t *pLocal = cl_enginefuncs->GetLocalPlayer();
	Vector vecEyes = localplayer->GetEyePosition();

	if ( m_pConsiderFOV->GetBool() )
	{
		cl_enginefuncs->GetViewAngles( va );
		AngleVectors( va, &vForward, NULL, NULL );
	}

	for ( register int i = 1; i <= Features::entitylist->GetMaxEntities(); i++ )
	{
		CEntity &ent = pEnts[ i ];

		if ( !ent.m_bValid )
			continue;

		if ( ent.m_classInfo.id != CLASS_NONE )
		{
			if ( !ent.m_bEnemy )
				continue;

			if ( ent.m_bItem )
				continue;

			if ( !ent.m_bAlive )
				continue;

			if ( ent.m_classInfo.id == CLASS_NPC_SNARK || ent.m_classInfo.id == CLASS_NPC_DESTROYED_OSPREY )
				continue;
		}
		else if ( !m_pIgnoreBlockingEnts->GetBool() )
		{
			continue;
		}

		float dist_sqr = ( pLocal->curstate.origin - ent.m_pEntity->curstate.origin ).LengthSqr();
		if ( dist_sqr >= flDistanceSqr )
			continue;
		
		Vector vecMins = ent.m_vecOrigin + ent.m_vecMins;
		Vector vecMaxs = ent.m_vecOrigin + ent.m_vecMaxs;

		Vector vecTargetPoint = ( vecMins + ( vecMaxs - vecMins ) * 0.6f );

		if ( m_pConsiderFOV->GetBool() )
		{
			const float angle = VEC_RAD2DEG( acosf( vForward.Dot( ( vecTargetPoint - vecEyes ).Normalize() ) ) );

			if ( angle > m_pFOV->GetFloat() )
				continue;
		}

		extra_class_info_t &extra_info = Features::entitylist->GetExtraEntityClassInfo( (EEntityClassID)ent.m_classInfo.id );

		if ( std::binary_search( extra_info.sequence_dead.begin(), extra_info.sequence_dead.end(), (unsigned char)ent.m_pEntity->curstate.sequence ) )
			continue;

		vHitboxes.clear();

		if ( m_pAimHitboxes->GetBool() && !extra_info.aimbot_hitboxes.empty() )
		{
			if ( m_pAimHead->GetBool() && std::find( vHitboxes.begin(), vHitboxes.end(), extra_info.aimbot_hitboxes[ HITBOX_HEAD ] ) == vHitboxes.end() )
				vHitboxes.push_back( extra_info.aimbot_hitboxes[ HITBOX_HEAD ] );

			if ( m_pAimNeck->GetBool() && std::find( vHitboxes.begin(), vHitboxes.end(), extra_info.aimbot_hitboxes[ HITBOX_NECK ] ) == vHitboxes.end() )
				vHitboxes.push_back( extra_info.aimbot_hitboxes[ HITBOX_NECK ] );

			if ( m_pAimChest->GetBool() && std::find( vHitboxes.begin(), vHitboxes.end(), extra_info.aimbot_hitboxes[ HITBOX_CHEST ] ) == vHitboxes.end() )
				vHitboxes.push_back( extra_info.aimbot_hitboxes[ HITBOX_CHEST ] );
		}

		// No hitboxes to aim, check the mid point then
		if ( vHitboxes.empty() )
		{
			if ( !IsCanSeeTarget( &ent, vecEyes, vecTargetPoint ) )
				continue;

			if ( sc_aimbot_scripts_filter_targets.GetBool() && !Modules::scripts->Callbacks()->OnFilterAimbotTarget( i ) )
				continue;

			pTarget = &ent;
			flDistanceSqr = dist_sqr;
			m_vecTargetPoint = vecTargetPoint;

			continue;
		}

		for ( size_t j = 0; j < vHitboxes.size(); j++ )
		{
			if ( ent.m_rgHitboxes == NULL )
				break;

			vecTargetPoint = ent.m_rgHitboxes[ vHitboxes[ j ] ];

			if ( !IsCanSeeTarget( &ent, vecEyes, vecTargetPoint ) )
				continue;
				
			if ( sc_aimbot_scripts_filter_targets.GetBool() && !Modules::scripts->Callbacks()->OnFilterAimbotTarget( i ) )
				break;

			pTarget = &ent;
			flDistanceSqr = dist_sqr;
			m_vecTargetPoint = vecTargetPoint;

			break;
		}
	}

	return pTarget;
}

//-----------------------------------------------------------------------------
// IsCanSeeTarget
//-----------------------------------------------------------------------------

bool CAim::IsCanSeeTarget( CEntity *pEntity, Vector &vecEyes, Vector &vecPoint )
{
	pmtrace_t trace;

	cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
	cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecEyes,
											   vecPoint,
											   m_pIgnoreBlockingEnts->GetBool() ? PM_WORLD_ONLY : ( m_pIgnoreGlass->GetBool() ? PM_GLASS_IGNORE : PM_NORMAL ),
											   -1,
											   &trace );

	return m_pIgnoreBlockingEnts->GetBool() ? ( trace.fraction == 1.f ) : ( cl_enginefuncs->pEventAPI->EV_IndexFromTrace( &trace ) == pEntity->m_pEntity->index );
}

//-----------------------------------------------------------------------------
// DirectionToAngles
//-----------------------------------------------------------------------------

void CAim::DirectionToAngles( Vector &vecDir, Vector &vecAngles )
{
	vecAngles.x = VEC_RAD2DEG( -atan2f( vecDir.z, vecDir.Length2D() ) );
	vecAngles.y = VEC_RAD2DEG( atan2f( vecDir.y, vecDir.x ) );
	vecAngles.z = 0.f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAim::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		auto pparams = pEvent->GetArg<ref_params_t *>( "pparams" );
		if ( bPostCall )
		{
			if ( m_pNoRecoilVisual->GetBool() )
			{
				*reinterpret_cast<Vector *>( pparams->viewangles ) -= m_vecPunchAngle + m_vecEvPunchAngle;
			}
		}
		else
		{
			m_vecPunchAngle = *reinterpret_cast<Vector *>( pparams->punchangle );

			if ( ev_punchangle != NULL )
				m_vecEvPunchAngle = *ev_punchangle;
			else
				m_vecEvPunchAngle.Zero();
		}

		return kHookContinue;
	}

	bool bAnglesChanged = false;
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	if ( m_bChangeAnglesBack )
	{
		cmd->viewangles = m_vecChangeAnglesTarget;
		cl_enginefuncs->SetViewAngles( m_vecChangeAnglesTarget );

		m_bChangeAnglesBack = false;
	}

	Aimbot( cmd, m_pAimbotMode->GetInt(), m_pAimChangeCameraBack->GetBool(), bAnglesChanged);
	NoRecoil( cmd );

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAim::CAim( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pAimbotMode = NULL;
	m_pAimChangeCameraBack = NULL;
	m_pAimHitboxes = NULL;
	m_pAimHead = NULL;
	m_pAimNeck = NULL;
	m_pAimChest = NULL;
	m_pIgnoreUnknownEnts = NULL;
	m_pIgnoreBlockingEnts = NULL;
	m_pIgnoreGlass = NULL;
	m_pConsiderFOV = NULL;
	m_pFOV = NULL;
	m_pDistance = NULL;

	m_pNoRecoil = NULL;
	m_pNoRecoilVisual = NULL;

	m_bChangeAnglesBack = false;
	ev_punchangle = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAim::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall, kHookPriorityLow );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAim::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAim::Load( void )
{
	Modules::menu->BindFeature( this );

	Modules::menu->AddElementSeparator( this, "Aimbot" );

	m_pAimbotMode = Modules::menu->AddParamList( this, "AimbotMode", "Mode", 0, " 0 - OFF\0 1 - Aimbot\0 2 - Silent Aimbot\0 3 - Ragebot\0\0" );
	m_pAimChangeCameraBack = Modules::menu->AddParamBool( this, "AimChangeCameraBack", NULL, false );
	m_pAimHitboxes = Modules::menu->AddParamBool( this, "AimHitboxes", NULL, true );
	m_pAimHead = Modules::menu->AddParamBool( this, "AimHead", "Head", true ); Modules::menu->AddElementSameLine( this );
	m_pAimNeck = Modules::menu->AddParamBool( this, "AimNeck", "Neck", true ); Modules::menu->AddElementSameLine( this );
	m_pAimChest = Modules::menu->AddParamBool( this, "AimChest", "Chest", true );
	m_pIgnoreUnknownEnts = Modules::menu->AddParamBool( this, "IgnoreUnknownEnts", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pIgnoreBlockingEnts = Modules::menu->AddParamBool( this, "IgnoreBlockingEnts", NULL, false ); Modules::menu->AddElementSameLine( this );
	m_pIgnoreGlass = Modules::menu->AddParamBool( this, "IgnoreGlass", NULL, false );
	m_pConsiderFOV = Modules::menu->AddParamBool( this, "ConsiderFOV", NULL, false );
	m_pFOV = Modules::menu->AddParamFloat( this, "FOV", NULL, 90.f, 0.f, 180.f );
	m_pDistance = Modules::menu->AddParamFloat( this, "Distance", NULL, 2048.f, 1.f, 8192.f );

	Modules::menu->AddElementSeparator( this, "Recoil" );

	m_pNoRecoil = Modules::menu->AddParamBool( this, "NoRecoil", NULL, true );
	m_pNoRecoilVisual = Modules::menu->AddParamBool( this, "NoRecoilVisual", NULL, false );

	bool bOK = true;
	int patternIndex;
	DEFINE_PATTERNS_FUTURE( fV_PunchAxis );
	MemoryUtils()->FindPatternAsync( GameData::Modules::Client,
									 FeaturesGameData::Patterns::Client::V_PunchAxis,
									 fV_PunchAxis );

	void *pfnV_PunchAxis = MemoryUtils()->GetPatternFutureValue( fV_PunchAxis, &patternIndex );
	FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( pfnV_PunchAxis,
										  "V_PunchAxis",
										  FeaturesGameData::Patterns::Client::V_PunchAxis,
										  patternIndex );

	if ( bOK )
	{
		ud_t inst;
		MemoryUtils()->InitDisasm( &inst, pfnV_PunchAxis, 32, 24 );

		while ( MemoryUtils()->Disassemble( &inst ) )
		{
			if ( ( inst.mnemonic == UD_Imovss && inst.operand[ 0 ].type == UD_OP_MEM &&
				 inst.operand[ 0 ].index == UD_R_EAX && inst.operand[ 0 ].scale == 4 &&
				 inst.operand[ 0 ].offset == 32 && inst.operand[ 1 ].type == UD_OP_REG &&
				 inst.operand[ 1 ].base == UD_R_XMM0 ) ||
				 ( inst.mnemonic == UD_Ifstp && inst.operand[ 0 ].type == UD_OP_MEM && // 5.11
				 inst.operand[ 0 ].index == UD_R_EAX && inst.operand[ 0 ].scale == 4 &&
				 inst.operand[ 0 ].offset == 32 ) )
			{
				ev_punchangle = reinterpret_cast<Vector *>( inst.operand[ 0 ].lval.udword );
				break;
			}
		}

		FEATURE_CHECK_SYMBOL_STATUS( ev_punchangle, "ev_punchangle" );
	}

	if ( !bOK )
		PrintWarning2( "No recoil is not available\n" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAim::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_aim );
	FEATURE_REGISTER_CVAR( sc_aimbot_scripts_filter_targets );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAim::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_aim );
	FEATURE_UNREGISTER_CVAR( sc_aimbot_scripts_filter_targets );
}
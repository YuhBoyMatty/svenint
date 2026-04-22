// SvenInt (c) Sw1ft
// misc_entity_list.cpp

#include "stdafx.h"
#include "misc_entity_list.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define LINK_CLASS_INFO(classID, flags) { static_cast<unsigned short>(flags & 0xFFFF), static_cast<unsigned short>(classID & 0xFFFF) }

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CEntityList, entitylist, "Misc", "Entity List" );

//-----------------------------------------------------------------------------
// Update entity list
//-----------------------------------------------------------------------------

void CEntityList::Update( void )
{
	static float vScreen[ 2 ];

	model_t *pModel = NULL;
	studiohdr_t *pStudioHeader = NULL;

	cl_entity_t *pEntity;
	cl_entity_s *pLocal = cl_enginefuncs->GetLocalPlayer();
	cl_entity_s *pViewModel = cl_enginefuncs->GetViewModel();

	bool bWasValid = true;

	for ( register int i = 1; i <= MY_MAXENTS; ++i )
	{
		bWasValid = m_ents[ i ].m_bValid;
		m_ents[ i ].m_bValid = false;

		const char *pszModelName;
		const char *pszSlashLastOccur;

		pEntity = cl_enginefuncs->GetEntityByIndex( i );

		if ( pEntity == NULL ||
			 pEntity->curstate.messagenum < pLocal->curstate.messagenum ||
			 pEntity == pViewModel )
			continue;

		if ( ( pModel = pEntity->model ) == NULL ||
			 *pModel->name != 'm' )
			continue;

		if ( ( pszSlashLastOccur = strrchr( pModel->name, '/' ) ) == NULL )
			continue;

		pszModelName = pszSlashLastOccur + 1;

		if ( !( pStudioHeader = (studiohdr_t *)enginestudio->Mod_Extradata( pModel ) ) ||
			 pStudioHeader->numhitboxes == 0 )
			continue;

		if ( pEntity->player )
		{
			float flHealth = m_ents[ i ].m_flHealth = extraplayerinfo->GetHealth( i );

			m_ents[ i ].m_bAlive = ( flHealth > 0.f || flHealth < -1.f );

			m_ents[ i ].m_classInfo.id = CLASS_PLAYER;
			m_ents[ i ].m_classInfo.flags = 0;

			if ( flHealth < -1.f )
			{
				m_ents[ i ].m_bFriend = false;
				m_ents[ i ].m_bEnemy = true;
				m_ents[ i ].m_bNeutral = false;
			}
			else
			{
				m_ents[ i ].m_bFriend = true;
				m_ents[ i ].m_bEnemy = false;
				m_ents[ i ].m_bNeutral = false;
			}
		}
		else
		{
			m_ents[ i ].m_bAlive = true;

			if ( pEntity->curstate.renderfx != (int)kRenderFxDeadPlayer )
			{
				m_ents[ i ].m_classInfo = GetEntityClassInfo( pszModelName );

				if ( IsEntityClassCorpse( m_ents[ i ].m_classInfo, pEntity->curstate.solid ) ||
					 IsEntityClassTrash( m_ents[ i ].m_classInfo ) )
					continue;
			}
			else
			{
				m_ents[ i ].m_classInfo.id = CLASS_DEAD_PLAYER;
				m_ents[ i ].m_classInfo.flags = FL_CLASS_NEUTRAL;
			}

			m_ents[ i ].m_bItem = IsEntityClassItem( m_ents[ i ].m_classInfo );
			m_ents[ i ].m_bFriend = IsEntityClassFriend( m_ents[ i ].m_classInfo );
			m_ents[ i ].m_bEnemy = IsEntityClassEnemy( m_ents[ i ].m_classInfo );
			m_ents[ i ].m_bNeutral = IsEntityClassNeutral( m_ents[ i ].m_classInfo );

			if ( m_ents[ i ].m_bItem )
			{
				if ( m_ents[ i ].m_classInfo.id == CLASS_ITEM_GRENADE &&
					 pEntity->curstate.solid == SOLID_BBOX )
				{
					if ( pEntity->curstate.sequence == 0 &&
						 pEntity->curstate.movetype == MOVETYPE_STEP )
					{
						m_ents[ i ].m_bNeutral = true;
						m_ents[ i ].m_bEnemy = false;
					}
					else
					{
						m_ents[ i ].m_bEnemy = true;
					}
				}
			}
			else if ( pEntity->curstate.solid == SOLID_NOT )
			{
				switch ( pEntity->curstate.movetype )
				{
				case MOVETYPE_STEP:
				case MOVETYPE_FLY:
				case MOVETYPE_TOSS:
					if ( m_ents[ i ].m_bFriend || m_ents[ i ].m_bEnemy )
						continue;

					break;
				}
			}
		}

		int iPrevPos = pEntity->current_position - 1;
		if ( iPrevPos < 0 )
			iPrevPos += HISTORY_MAX;

		m_ents[ i ].m_pEntity = pEntity;
		m_ents[ i ].m_pStudioHeader = pStudioHeader;

		m_ents[ i ].m_vecVelocity = pEntity->ph[ pEntity->current_position ].origin - pEntity->ph[ iPrevPos ].origin;
		//m_ents[ i ].m_frametime = pEntity->curstate.animtime - pEntity->prevstate.animtime;

		m_ents[ i ].m_bPlayer = pEntity->player;
		m_ents[ i ].m_bDucked = pEntity->curstate.usehull;
		m_ents[ i ].m_bVisible = UTIL_WorldToScreen( pEntity->curstate.origin + pEntity->curstate.mins +
													 ( ( pEntity->curstate.origin + pEntity->curstate.maxs ) - ( pEntity->curstate.origin + pEntity->curstate.mins ) ) * 0.5f,
													 vScreen );

		m_ents[ i ].m_vecPrevOrigin = !bWasValid ? pEntity->curstate.origin : m_ents[ i ].m_vecOrigin;
		m_ents[ i ].m_vecOrigin = pEntity->curstate.origin;

		if ( m_ents[ i ].m_classInfo.id == CLASS_OBJECT_CP )
		{
			m_ents[ i ].m_vecMins.Zero();
			m_ents[ i ].m_vecMaxs.Zero();
		}
		else
		{
			m_ents[ i ].m_vecMins = pEntity->curstate.mins;
			m_ents[ i ].m_vecMaxs = pEntity->curstate.maxs;
		}

		m_ents[ i ].m_bValid = true;
	}
}

//-----------------------------------------------------------------------------
// Update hitboxes
//-----------------------------------------------------------------------------

void CEntityList::UpdateHitboxes( int index )
{
	m_ents[ index ].m_pStudioHeader = studiorenderer->m_pStudioHeader;

	if ( !m_ents[ index ].m_bValid || m_ents[ index ].m_rgHitboxes == NULL )
		return;

	Vector vecHitbox;
	mstudiobbox_t *pHitbox = (mstudiobbox_t *)( (byte *)m_ents[ index ].m_pStudioHeader + m_ents[ index ].m_pStudioHeader->hitboxindex );

	for ( int i = 0; i < m_ents[ index ].m_pStudioHeader->numhitboxes; i++ )
	{
		Vector vecMid = ( pHitbox[ i ].bbmin + pHitbox[ i ].bbmax ) * 0.5f;
		VectorTransform( vecMid, ( *m_pBoneTransform )[ pHitbox[ i ].bone ], vecHitbox );

		m_ents[ index ].m_rgHitboxes[ i ] = vecHitbox + m_ents[ index ].m_vecVelocity;
	}
}

//-----------------------------------------------------------------------------
// GetEntityClassInfo
//-----------------------------------------------------------------------------

class_info_t CEntityList::GetEntityClassInfo( const char *pszModelName )
{
	auto pClassEntry = m_classTable.Find( (uint32_t)pszModelName );

	if ( !pClassEntry )
	{
		const char *pszSlashLastOccur = strrchr( pszModelName, '/' );
		const char *pszModelNameSliced = pszModelName;

		if ( pszSlashLastOccur )
			pszModelNameSliced = pszSlashLastOccur + 1;

		// Result: "model/hlclassic/scientist.mdl" -> "scientist.mdl"

		class_info_t *pClassInfo = m_modelsTable.Find( pszModelNameSliced );

		if ( pClassInfo == NULL )
		{
			m_classTable.Insert( (uint32_t)pszModelName, LINK_CLASS_INFO( CLASS_NONE, FL_CLASS_NEUTRAL ) );
			return { CLASS_NONE, FL_CLASS_NEUTRAL };
		}

		m_classTable.Insert( (uint32_t)pszModelName, *pClassInfo );
		return *pClassInfo;
	}

	return *pClassEntry;
}

//-----------------------------------------------------------------------------
// Add classes
//-----------------------------------------------------------------------------

void CEntityList::AddClasses( void )
{
	AddClassInfo( "lambda.mdl", CLASS_OBJECT_CP, FL_CLASS_NEUTRAL );
	AddClassInfo( "umbrella.mdl", CLASS_OBJECT_CP, FL_CLASS_NEUTRAL );

	// NPCs
	AddClassInfo( "gordon_scientist.mdl", CLASS_NPC_GORDON_FREEMAN, FL_CLASS_FRIEND );
	AddClassInfo( "gordon.mdl", CLASS_NPC_GORDON_FREEMAN, FL_CLASS_FRIEND );
	AddClassInfo( "scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "scientist2.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "cleansuit_scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "scientist_rosenberg.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "wheelchair_sci.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "civ_scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );
	AddClassInfo( "civ_paper_scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );
	AddClassInfo( "console_civ_scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );
	AddClassInfo( "sc2sci.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );
	AddClassInfo( "scigun.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );
	AddClassInfo( "civ_coat_scientist.mdl", CLASS_NPC_SCIENTIST, FL_CLASS_FRIEND );

	AddClassInfo( "hgruntf.mdl", CLASS_NPC_HUMAN_GRUNT, FL_CLASS_FRIEND );
	AddClassInfo( "sc2grunt.mdl", CLASS_NPC_ALIEN_GRUNT, FL_CLASS_FRIEND );
	AddClassInfo( "agruntf.mdl", CLASS_NPC_ALIEN_GRUNT, FL_CLASS_FRIEND );

	AddClassInfo( "barney.mdl", CLASS_NPC_BARNEY, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "intro_barney.mdl", CLASS_NPC_BARNEY, FL_CLASS_FRIEND );
	AddClassInfo( "hungerbarney.mdl", CLASS_NPC_BARNEY, FL_CLASS_ENEMY | FL_CLASS_CORPSE ); // TH
	AddClassInfo( "pilot.mdl", CLASS_NPC_BARNEY, FL_CLASS_FRIEND ); // TH

	AddClassInfo( "headcrab.mdl", CLASS_NPC_HEADCRAB, FL_CLASS_ENEMY );
	AddClassInfo( "baby_headcrab.mdl", CLASS_NPC_BABY_HEADCRAB, FL_CLASS_ENEMY );
	AddClassInfo( "hungercrab.mdl", CLASS_NPC_HEADCRAB, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "otis.mdl", CLASS_NPC_OTIS, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "intro_otis.mdl", CLASS_NPC_OTIS, FL_CLASS_FRIEND );
	AddClassInfo( "otisf.mdl", CLASS_NPC_OTIS, FL_CLASS_FRIEND );
	AddClassInfo( "hungerotis.mdl", CLASS_NPC_OTIS, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "zombie.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY );
	AddClassInfo( "zombie_soldier.mdl", CLASS_NPC_ZOMBIE_SOLDIER, FL_CLASS_ENEMY );
	AddClassInfo( "zombie_barney.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY );
	AddClassInfo( "hungerzombie.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "nursezombie.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "zombie2.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "zombie3.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "nurse.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "lpzombie.mdl", CLASS_NPC_ZOMBIE, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "houndeye.mdl", CLASS_NPC_HOUNDEYE, FL_CLASS_ENEMY );
	AddClassInfo( "hungerhound.mdl", CLASS_NPC_HOUNDEYE, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "bullsquid.mdl", CLASS_NPC_BULLSQUID, FL_CLASS_ENEMY );
	AddClassInfo( "barnacle.mdl", CLASS_NPC_BARNACLE, FL_CLASS_ENEMY );

	AddClassInfo( "islave.mdl", CLASS_NPC_VORTIGAUNT, FL_CLASS_ENEMY );
	AddClassInfo( "islavef.mdl", CLASS_NPC_VORTIGAUNT, FL_CLASS_FRIEND );
	AddClassInfo( "sslave.mdl", CLASS_NPC_VORTIGAUNT, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "hungerslave.mdl", CLASS_NPC_VORTIGAUNT, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "hgrunt.mdl", CLASS_NPC_HUMAN_GRUNT, FL_CLASS_ENEMY | FL_CLASS_CORPSE );
	AddClassInfo( "zgrunt.mdl", CLASS_NPC_HUMAN_GRUNT, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "hgrunt_sniper.mdl", CLASS_NPC_HUMAN_GRUNT_SNIPER, FL_CLASS_ENEMY );

	AddClassInfo( "agrunt.mdl", CLASS_NPC_ALIEN_GRUNT, FL_CLASS_ENEMY );
	AddClassInfo( "zork.mdl", CLASS_NPC_ALIEN_GRUNT, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "tentacle2.mdl", CLASS_NPC_TENTACLE, FL_CLASS_ENEMY );
	AddClassInfo( "tentacle3.mdl", CLASS_NPC_TENTACLE, FL_CLASS_ENEMY );

	AddClassInfo( "sentry.mdl", CLASS_NPC_SENTRY, FL_CLASS_ENEMY );
	AddClassInfo( "turret.mdl", CLASS_NPC_TURRET, FL_CLASS_ENEMY );
	AddClassInfo( "miniturret.mdl", CLASS_NPC_TURRET, FL_CLASS_ENEMY );
	AddClassInfo( "leech.mdl", CLASS_NPC_LEECH, FL_CLASS_ENEMY );

	AddClassInfo( "gman.mdl", CLASS_NPC_GMAN, FL_CLASS_NEUTRAL );

	AddClassInfo( "hassassin.mdl", CLASS_NPC_FEMALE_ASSASSIN, FL_CLASS_ENEMY );
	AddClassInfo( "hassassinf.mdl", CLASS_NPC_FEMALE_ASSASSIN, FL_CLASS_FRIEND );

	AddClassInfo( "w_squeak.mdl", CLASS_NPC_SNARK, FL_CLASS_ENEMY );
	AddClassInfo( "chubby.mdl", CLASS_NPC_CHUMTOAD, FL_CLASS_FRIEND );
	AddClassInfo( "chumtoad.mdl", CLASS_NPC_CHUMTOAD, FL_CLASS_FRIEND );
	AddClassInfo( "piranha.mdl", CLASS_NPC_PIRANHA, FL_CLASS_ENEMY );
	AddClassInfo( "zombierat.mdl", CLASS_NPC_SNARK, FL_CLASS_ENEMY );

	AddClassInfo( "controller.mdl", CLASS_NPC_ALIEN_CONTROLLER, FL_CLASS_ENEMY );

	AddClassInfo( "icky.mdl", CLASS_NPC_ICHTYOSAUR, FL_CLASS_ENEMY );

	AddClassInfo( "garg.mdl", CLASS_NPC_GARGANTUA, FL_CLASS_ENEMY );
	AddClassInfo( "babygarg.mdl", CLASS_NPC_BABY_GARGANTUA, FL_CLASS_ENEMY );

	AddClassInfo( "big_mom.mdl", CLASS_NPC_BIG_MOMMA, FL_CLASS_ENEMY );

	AddClassInfo( "osprey.mdl", CLASS_NPC_OSPREY, FL_CLASS_ENEMY );
	AddClassInfo( "blkop_osprey.mdl", CLASS_NPC_BLACK_OPS_OSPREY, FL_CLASS_ENEMY );
	AddClassInfo( "dead_osprey.mdl", CLASS_NPC_DESTROYED_OSPREY, FL_CLASS_ENEMY );
	AddClassInfo( "apache.mdl", CLASS_NPC_APACHE, FL_CLASS_ENEMY );
	AddClassInfo( "boss.mdl", CLASS_NPC_APACHE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "huey_apache.mdl", CLASS_NPC_APACHE, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "apache2.mdl", CLASS_NPC_APACHE, FL_CLASS_FRIEND ); // TH
	AddClassInfo( "blkop_apache.mdl", CLASS_NPC_APACHE, FL_CLASS_ENEMY );

	AddClassInfo( "nihilanth.mdl", CLASS_NPC_NIHILANTH, FL_CLASS_ENEMY );
	AddClassInfo( "aflock.mdl", CLASS_NPC_BOID, FL_CLASS_NEUTRAL );

	AddClassInfo( "player.mdl", CLASS_NPC_HEV, FL_CLASS_WORLD_ENTITY );

	AddClassInfo( "spore_ammo.mdl", CLASS_NPC_SPORE_AMMO, FL_CLASS_ENEMY );

	AddClassInfo( "hgrunt_opforf.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "hgrunt_opfor.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "hgrunt_torchf.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "hgrunt_torch.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "hgrunt_medicf.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "hgrunt_medic.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC, FL_CLASS_FRIEND | FL_CLASS_CORPSE );
	AddClassInfo( "intro_saw.mdl", CLASS_NPC_HUMAN_GRUNT_OPFOR, FL_CLASS_FRIEND | FL_CLASS_CORPSE );

	AddClassInfo( "massn.mdl", CLASS_NPC_MALE_ASSASSIN, FL_CLASS_ENEMY );
	AddClassInfo( "massnf.mdl", CLASS_NPC_MALE_SNIPER_ASSASSIN, FL_CLASS_ENEMY );
	AddClassInfo( "bgman.mdl", CLASS_NPC_AGENT, FL_CLASS_ENEMY );

	AddClassInfo( "gonome.mdl", CLASS_NPC_GONOME, FL_CLASS_ENEMY );
	AddClassInfo( "hungergonome.mdl", CLASS_NPC_GONOME, FL_CLASS_ENEMY );

	AddClassInfo( "pit_drone.mdl", CLASS_NPC_PIT_DRONE, FL_CLASS_ENEMY );
	AddClassInfo( "strooper.mdl", CLASS_NPC_SHOCK_TROOPER, FL_CLASS_ENEMY );

	AddClassInfo( "voltigore.mdl", CLASS_NPC_VOLTIGORE, FL_CLASS_ENEMY );
	AddClassInfo( "baby_voltigore.mdl", CLASS_NPC_BABY_VOLTIGORE, FL_CLASS_ENEMY );
	AddClassInfo( "pit_worm_up.mdl", CLASS_NPC_PIT_WORM, FL_CLASS_ENEMY );
	AddClassInfo( "geneworm.mdl", CLASS_NPC_GENEWORM, FL_CLASS_ENEMY );

	AddClassInfo( "w_shock_rifle.mdl", CLASS_NPC_SHOCK_RIFLE, FL_CLASS_ENEMY );
	AddClassInfo( "mortar.mdl", CLASS_NPC_MORTAR, FL_CLASS_ENEMY );

	AddClassInfo( "stukabat.mdl", CLASS_NPC_STUKABAT, FL_CLASS_ENEMY );
	AddClassInfo( "kingpin.mdl", CLASS_NPC_KINGPIN, FL_CLASS_ENEMY );
	AddClassInfo( "tor.mdl", CLASS_NPC_TOR, FL_CLASS_ENEMY );
	AddClassInfo( "torf.mdl", CLASS_NPC_TOR, FL_CLASS_FRIEND );

	AddClassInfo( "hwgrunt.mdl", CLASS_NPC_HEAVY_GRUNT, FL_CLASS_ENEMY );
	AddClassInfo( "hwgruntf.mdl", CLASS_NPC_HEAVY_GRUNT, FL_CLASS_FRIEND );

	AddClassInfo( "rgrunt.mdl", CLASS_NPC_ROBOT_GRUNT, FL_CLASS_ENEMY );
	AddClassInfo( "rgruntf.mdl", CLASS_NPC_ROBOT_GRUNT, FL_CLASS_FRIEND );

	AddClassInfo( "zombiebull.mdl", CLASS_NPC_ZOMBIE_BULL, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "thehand.mdl", CLASS_NPC_THE_HAND, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "chicken.mdl", CLASS_NPC_CHICKEN, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "sheriff.mdl", CLASS_NPC_SHERIFF, FL_CLASS_ENEMY ); // TH
	AddClassInfo( "franklin2.mdl", CLASS_NPC_CYBERFRANKLIN, FL_CLASS_ENEMY ); // TH

	AddClassInfo( "flyer.mdl", CLASS_NPC_MANTA, FL_CLASS_ENEMY );

	AddClassInfo( "barnabus.mdl", CLASS_NPC_BARNABUS, FL_CLASS_ENEMY );
	AddClassInfo( "skeleton.mdl", CLASS_NPC_SKELETON, FL_CLASS_ENEMY );

	// Wouldn't add them tbh
	AddClassInfo( "spforce.mdl", CLASS_NPC_SPECFOR_GRUNT, FL_CLASS_ENEMY );
	AddClassInfo( "barniel.mdl", CLASS_NPC_BARNIEL, FL_CLASS_FRIEND );
	AddClassInfo( "archer.mdl", CLASS_NPC_ARCHER, FL_CLASS_ENEMY );
	AddClassInfo( "panther.mdl", CLASS_NPC_PANTHEREYE, FL_CLASS_ENEMY );
	AddClassInfo( "fiona.mdl", CLASS_NPC_FIONA, FL_CLASS_NEUTRAL );
	AddClassInfo( "twitcher.mdl", CLASS_NPC_TWITCHER, FL_CLASS_ENEMY );
	AddClassInfo( "twitcher2.mdl", CLASS_NPC_TWITCHER, FL_CLASS_ENEMY );
	AddClassInfo( "twitcher3.mdl", CLASS_NPC_TWITCHER, FL_CLASS_ENEMY );
	AddClassInfo( "twitcher4.mdl", CLASS_NPC_TWITCHER, FL_CLASS_ENEMY );
	AddClassInfo( "spitter.mdl", CLASS_NPC_SPITTER, FL_CLASS_ENEMY );
	AddClassInfo( "handcrab.mdl", CLASS_NPC_HANDCRAB, FL_CLASS_ENEMY );
	AddClassInfo( "ghost.mdl", CLASS_NPC_GHOST, FL_CLASS_ENEMY );
	AddClassInfo( "screamer.mdl", CLASS_NPC_SCREAMER, FL_CLASS_ENEMY );
	AddClassInfo( "devourer.mdl", CLASS_NPC_DEVOURER, FL_CLASS_ENEMY );
	AddClassInfo( "wheelchair_new.mdl", CLASS_NPC_WHEELCHAIR, FL_CLASS_ENEMY );
	AddClassInfo( "face_new.mdl", CLASS_NPC_FACE, FL_CLASS_ENEMY );
	AddClassInfo( "hellhound.mdl", CLASS_NPC_HELLHOUND, FL_CLASS_ENEMY );
	AddClassInfo( "davidbad_cutscene.mdl", CLASS_NPC_ADDICTION, FL_CLASS_ENEMY );
	AddClassInfo( "davidbad_noaxe.mdl", CLASS_NPC_ADDICTION, FL_CLASS_ENEMY );
	AddClassInfo( "shark.mdl", CLASS_NPC_SHARK, FL_CLASS_ENEMY );

	// Items
	AddClassInfo( "w_suit.mdl", CLASS_ITEM_HEV, FL_CLASS_ITEM );
	AddClassInfo( "w_medkit.mdl", CLASS_ITEM_MEDKIT, FL_CLASS_ITEM );
	AddClassInfo( "w_pmedkit.mdl", CLASS_ITEM_MEDKIT, FL_CLASS_ITEM );
	AddClassInfo( "th_medkit.mdl", CLASS_ITEM_MEDKIT, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_battery.mdl", CLASS_ITEM_BATTERY, FL_CLASS_ITEM );
	AddClassInfo( "w_9mmclip.mdl", CLASS_ITEM_GLOCK_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_1911_mag.mdl", CLASS_ITEM_GLOCK_AMMO, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_357ammobox.mdl", CLASS_ITEM_PYTHON_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_hunger357ammobox.mdl", CLASS_ITEM_PYTHON_AMMO, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_shotbox.mdl", CLASS_ITEM_SHOTGUN_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_uzi_clip.mdl", CLASS_ITEM_UZI_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_9mmarclip.mdl", CLASS_ITEM_MP5_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_mp5_clip.mdl", CLASS_ITEM_MP5_AMMO2, FL_CLASS_ITEM );
	AddClassInfo( "w_chainammo.mdl", CLASS_ITEM_CHAIN_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_crossbow_clip.mdl", CLASS_ITEM_CROSSBOW_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_gaussammo.mdl", CLASS_ITEM_GAUSS_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_rpgammo.mdl", CLASS_ITEM_RPG_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_m40a1clip.mdl", CLASS_ITEM_SNIPER_RIFLE_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_saw_clip.mdl", CLASS_ITEM_MACHINEGUN_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_crowbar.mdl", CLASS_ITEM_CROWBAR, FL_CLASS_ITEM );
	AddClassInfo( "w_hungercrowbar.mdl", CLASS_ITEM_CROWBAR, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_pipe_wrench.mdl", CLASS_ITEM_WRENCH, FL_CLASS_ITEM );
	AddClassInfo( "w_knife.mdl", CLASS_ITEM_KNIFE, FL_CLASS_ITEM );
	AddClassInfo( "w_bgrap.mdl", CLASS_ITEM_BARNACLE_GRAPPLE, FL_CLASS_ITEM );
	AddClassInfo( "w_9mmhandgun.mdl", CLASS_ITEM_GLOCK, FL_CLASS_ITEM );
	AddClassInfo( "w_1911.mdl", CLASS_ITEM_GLOCK, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_357.mdl", CLASS_ITEM_PYTHON, FL_CLASS_ITEM );
	AddClassInfo( "w_hunger357.mdl", CLASS_ITEM_PYTHON, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_desert_eagle.mdl", CLASS_ITEM_DEAGLE, FL_CLASS_ITEM );
	AddClassInfo( "w_shotgun.mdl", CLASS_ITEM_SHOTGUN, FL_CLASS_ITEM );
	AddClassInfo( "w_uzi.mdl", CLASS_ITEM_UZI, FL_CLASS_ITEM );
	AddClassInfo( "w_2uzis.mdl", CLASS_ITEM_2UZIS, FL_CLASS_ITEM );
	AddClassInfo( "w_9mmAR.mdl", CLASS_ITEM_MP5, FL_CLASS_ITEM );
	AddClassInfo( "w_m16.mdl", CLASS_ITEM_M16, FL_CLASS_ITEM );
	AddClassInfo( "w_crossbow.mdl", CLASS_ITEM_CROSSBOW, FL_CLASS_ITEM );
	AddClassInfo( "w_gauss.mdl", CLASS_ITEM_GAUSS, FL_CLASS_ITEM );
	AddClassInfo( "w_egon.mdl", CLASS_ITEM_EGON, FL_CLASS_ITEM );
	AddClassInfo( "w_rpg.mdl", CLASS_ITEM_RPG, FL_CLASS_ITEM );
	AddClassInfo( "w_hgun.mdl", CLASS_ITEM_HORNET_GUN, FL_CLASS_ITEM );
	AddClassInfo( "w_m40a1.mdl", CLASS_ITEM_SNIPER_RIFLE, FL_CLASS_ITEM );
	AddClassInfo( "w_saw.mdl", CLASS_ITEM_MACHINEGUN, FL_CLASS_ITEM );
	AddClassInfo( "w_spore_launcher.mdl", CLASS_ITEM_SPORE_LAUNCHER, FL_CLASS_ITEM );
	AddClassInfo( "w_displacer.mdl", CLASS_ITEM_DISPLACER, FL_CLASS_ITEM );
	AddClassInfo( "w_minigun.mdl", CLASS_ITEM_MINIGUN, FL_CLASS_ITEM );
	AddClassInfo( "w_sqknest.mdl", CLASS_ITEM_SNARK_NEST, FL_CLASS_ITEM );
	AddClassInfo( "w_grenade.mdl", CLASS_ITEM_GRENADE, FL_CLASS_ITEM );
	AddClassInfo( "w_hungergrenade.mdl", CLASS_ITEM_GRENADE, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_tnt.mdl", CLASS_ITEM_GRENADE, FL_CLASS_ITEM ); // TH
	AddClassInfo( "w_satchel.mdl", CLASS_ITEM_SATCHEL, FL_CLASS_ITEM );
	AddClassInfo( "w_argrenade.mdl", CLASS_ITEM_ARGRENADE, FL_CLASS_ITEM );
	AddClassInfo( "w_tripmine.mdl", CLASS_ITEM_TRIPMINE, FL_CLASS_ITEM );
	AddClassInfo( "w_weaponbox.mdl", CLASS_ITEM_WEAPON_BOX, FL_CLASS_ITEM );
	AddClassInfo( "w_longjump.mdl", CLASS_ITEM_LONGJUMP, FL_CLASS_ITEM );
	AddClassInfo( "health_charger_body.mdl", CLASS_ITEM_HEALTH_CHARGER, FL_CLASS_ITEM );
	AddClassInfo( "hev_glass.mdl", CLASS_ITEM_HEV_CHARGER, FL_CLASS_ITEM );
	AddClassInfo( "barney_vest.mdl", CLASS_ITEM_BARNEY_VEST, FL_CLASS_ITEM );
	AddClassInfo( "barney_helmet.mdl", CLASS_ITEM_BARNEY_HELMET, FL_CLASS_ITEM );
	AddClassInfo( "suit2.mdl", CLASS_ITEM_SUIT, FL_CLASS_ITEM );

	AddClassInfo( "spore.mdl", CLASS_ITEM_SPORE, FL_CLASS_ITEM | FL_CLASS_ENEMY );
	AddClassInfo( "grenade.mdl", CLASS_ITEM_GRENADE, FL_CLASS_ITEM | FL_CLASS_ENEMY );
	AddClassInfo( "crossbow_bolt.mdl", CLASS_ITEM_CROSSBOW_BOLT, FL_CLASS_ITEM | FL_CLASS_ENEMY );
	AddClassInfo( "rpgrocket.mdl", CLASS_ITEM_RPG_ROCKET, FL_CLASS_ITEM | FL_CLASS_ENEMY );
	AddClassInfo( "HVR.mdl", CLASS_ITEM_HVR_ROCKET, FL_CLASS_ITEM | FL_CLASS_ENEMY );
	AddClassInfo( "mortarshell.mdl", CLASS_ITEM_MORTAR_SHELL, FL_CLASS_ITEM | FL_CLASS_ENEMY );

	AddClassInfo( "w_tommygun_mag.mdl", CLASS_ITEM_TOMMY_GUN_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_greasegun_mag.mdl", CLASS_ITEM_GREASE_GUN_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_m16_mag.mdl", CLASS_ITEM_M16_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_m14_mag.mdl", CLASS_ITEM_M14_AMMO, FL_CLASS_ITEM );
	AddClassInfo( "w_shovel.mdl", CLASS_ITEM_SHOVEL, FL_CLASS_ITEM );
	AddClassInfo( "w_spanner.mdl", CLASS_ITEM_SPANNER, FL_CLASS_ITEM );
	AddClassInfo( "w_dbarrel.mdl", CLASS_ITEM_DOUBLE_BARREL, FL_CLASS_ITEM );
	AddClassInfo( "w_tommygun.mdl", CLASS_ITEM_TOMMY_GUN, FL_CLASS_ITEM );
	AddClassInfo( "w_greasegun.mdl", CLASS_ITEM_GREASE_GUN, FL_CLASS_ITEM );
	AddClassInfo( "w_m14.mdl", CLASS_ITEM_M14, FL_CLASS_ITEM );
	AddClassInfo( "w_tesla.mdl", CLASS_ITEM_TESLA_GUN, FL_CLASS_ITEM );

	AddClassInfo( "w_glock18.mdl", CLASS_ITEM_CS_GLOCK, FL_CLASS_ITEM );
	AddClassInfo( "w_usp.mdl", CLASS_ITEM_CS_USP45, FL_CLASS_ITEM );
	AddClassInfo( "w_p228.mdl", CLASS_ITEM_CS_P228, FL_CLASS_ITEM );
	AddClassInfo( "w_57.mdl", CLASS_ITEM_CS_FN57, FL_CLASS_ITEM );
	AddClassInfo( "w_elite.mdl", CLASS_ITEM_CS_DUAL_BERETTAS, FL_CLASS_ITEM );
	AddClassInfo( "w_eagle.mdl", CLASS_ITEM_CS_DEAGLE, FL_CLASS_ITEM );
	AddClassInfo( "w_xm1014.mdl", CLASS_ITEM_CS_XM1014, FL_CLASS_ITEM );
	AddClassInfo( "w_m3.mdl", CLASS_ITEM_CS_M3, FL_CLASS_ITEM );
	AddClassInfo( "w_mac10.mdl", CLASS_ITEM_CS_MAC10, FL_CLASS_ITEM );
	AddClassInfo( "w_tmp.mdl", CLASS_ITEM_CS_TMP, FL_CLASS_ITEM );
	AddClassInfo( "w_mp5.mdl", CLASS_ITEM_CS_MP5, FL_CLASS_ITEM );
	AddClassInfo( "w_ump45.mdl", CLASS_ITEM_CS_UMP45, FL_CLASS_ITEM );
	AddClassInfo( "w_p90.mdl", CLASS_ITEM_CS_P90, FL_CLASS_ITEM );
	AddClassInfo( "w_famas.mdl", CLASS_ITEM_CS_FAMAS, FL_CLASS_ITEM );
	AddClassInfo( "w_galil.mdl", CLASS_ITEM_CS_GALIL, FL_CLASS_ITEM );
	AddClassInfo( "w_ak47.mdl", CLASS_ITEM_CS_AK47, FL_CLASS_ITEM );
	AddClassInfo( "w_m4a1.mdl", CLASS_ITEM_CS_M4A1, FL_CLASS_ITEM );
	AddClassInfo( "w_aug.mdl", CLASS_ITEM_CS_AUG, FL_CLASS_ITEM );
	AddClassInfo( "w_sg552.mdl", CLASS_ITEM_CS_SG552, FL_CLASS_ITEM );
	AddClassInfo( "w_scout.mdl", CLASS_ITEM_CS_SCOUT, FL_CLASS_ITEM );
	AddClassInfo( "w_awp.mdl", CLASS_ITEM_CS_AWP, FL_CLASS_ITEM );
	AddClassInfo( "w_sg550.mdl", CLASS_ITEM_CS_SG550, FL_CLASS_ITEM );
	AddClassInfo( "w_g3sg1.mdl", CLASS_ITEM_CS_G3SG1, FL_CLASS_ITEM );
	AddClassInfo( "w_m249.mdl", CLASS_ITEM_CS_M249, FL_CLASS_ITEM );
	AddClassInfo( "w_he.mdl", CLASS_ITEM_CS_HEGRENADE, FL_CLASS_ITEM );
	AddClassInfo( "w_c4.mdl", CLASS_ITEM_CS_C4, FL_CLASS_ITEM );
	AddClassInfo( "w_bp.mdl", CLASS_ITEM_CS_C4, FL_CLASS_ITEM );

	AddClassInfo( "spooky_gifts.mdl", CLASS_ITEM_SPOOKY_GIFTS, FL_CLASS_ITEM );
	AddClassInfo( "xmas_gifts.mdl", CLASS_ITEM_XMAS_GIFTS, FL_CLASS_ITEM );

	// World entites that have at least one hitbox
	AddClassInfo( "bigrat.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "w_syringebox.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "dead_islave.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "headless_zombie.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "dissected_headcrab.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tank_base.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "DecayCrystals.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "broken_tube_glass.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bs_experiment.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bs_glasstube.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "hornet.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tree.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "protozoa.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fungus.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fungus(small).mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fungus(large).mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "hair.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "rengine.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "sat_globe.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "roach.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "forklift.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "loader.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "construction.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "dead_barney.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "dead_scientist.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "ball.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "can.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "w_crossbow_clip.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "pit_drone_spike.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "crashed_osprey.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "baby_strooper.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "filecabinet.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "light.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tool_box.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "nuke_case.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "arc_xer_tree1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "chair.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tool_box_sm.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "w_flashlight.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "rip.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "base.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "base_flag.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "shell.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "stretcher.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "holo.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY ); // holo-.. Horo?
	AddClassInfo( "egg.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "plant_01.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "health_charger_both.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "EYE_SCANNER.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "obj_chair.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "obj_pipe1_straight.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "pipe_1_straight.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "pipe_1_curve.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "arc_xer_tree2.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "arc_bush.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "arc_flower.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "arc_fern.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "ouitz_tree1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "alec_tree1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "zalec_tree1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bush1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bush2.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fern1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fern2.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "uplant1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "mbarrel.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tree1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "tree2.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "shrub1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "cross.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "grave.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "grave1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "grave2.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "grave3.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "grave4.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );

	// fucking gibs
	AddClassInfo( "tech_crategibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "vgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "pit_drone_gibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "med_crategibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "mil_crategibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "strooper_gibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "concrete_gibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "chromegibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "catwalkgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "garbagegibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "metalplategibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "rockgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "office_gibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "hgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "webgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "osprey_bodygibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "woodgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "fleshgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "glassgibsw.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "metalplategibs_dark.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "metalgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bleachbones.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "bonegibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "glassgibs.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "gib_lung.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "gib_skull.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "gib_b_bone.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "ribcage.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
	AddClassInfo( "riblet1.mdl", CLASS_NONE, FL_CLASS_WORLD_ENTITY );
}

//-----------------------------------------------------------------------------
// Add class info
//-----------------------------------------------------------------------------

void CEntityList::AddClassInfo( const char *pszModelname, int id, int flags )
{
	m_modelsTable.Insert( pszModelname, LINK_CLASS_INFO( id, flags ) );
}

//-----------------------------------------------------------------------------
// Add classes
//-----------------------------------------------------------------------------

void CEntityList::AddExtraClassInfos( void )
{
	// class id | name of entity | array of 3 hitboxes for aimbot ( head, neck, chest ) | array of sequences when entity is considered as dead
	AddExtraClassInfo( CLASS_NONE, "Unknown" );

	AddExtraClassInfo( CLASS_PLAYER, "Player", { 11, 10, 8 } );
	AddExtraClassInfo( CLASS_DEAD_PLAYER, "Dead Player" );

	AddExtraClassInfo( CLASS_OBJECT_CP, "Respawn Point", { 0 } );

	AddExtraClassInfo( CLASS_NPC_GORDON_FREEMAN, "Gordon Freeman" );
	AddExtraClassInfo( CLASS_NPC_SCIENTIST, "Scientist" );
	AddExtraClassInfo( CLASS_NPC_BARNEY, "Barney" );
	AddExtraClassInfo( CLASS_NPC_OTIS, "Otis", {  }, { 25, 26, 27, 28, 29, 30 } );
	AddExtraClassInfo( CLASS_NPC_HEADCRAB, "Headcrab", { 6, 6, 6 }, { 7 } );
	AddExtraClassInfo( CLASS_NPC_BABY_HEADCRAB, "Baby Headcrab", { 0, 0, 0 }, { 7 } );
	AddExtraClassInfo( CLASS_NPC_ZOMBIE, "Zombie", { 11, 10, 8 }, { 15, 16, 17, 18, 19, 31 } );
	AddExtraClassInfo( CLASS_NPC_ZOMBIE_SOLDIER, "Zombie Soldier", { 12, 11, 9 }, { 15, 16, 17, 18, 19 } );
	AddExtraClassInfo( CLASS_NPC_BULLSQUID, "Bullsquid", { 17, 16, 0 }, { 13, 15, 16, 17 } );
	AddExtraClassInfo( CLASS_NPC_HOUNDEYE, "Houndeye", { 9, 8, 6 }, { 6, 7, 8, 9 } );
	AddExtraClassInfo( CLASS_NPC_BARNACLE, "Barnacle", {  }, { 6 } );
	AddExtraClassInfo( CLASS_NPC_VORTIGAUNT, "Vortigaunt", { 19, 10, 9 }, { 18, 19, 20, 21 } );
	AddExtraClassInfo( CLASS_NPC_HUMAN_GRUNT, "Human Grunt", { 23, 12, 11 }, { 35, 36, 37, 38, 39, 40, 78, 80 } );
	AddExtraClassInfo( CLASS_NPC_HUMAN_GRUNT_SNIPER, "Sniper", { 16, 7, 6 } );
	AddExtraClassInfo( CLASS_NPC_ALIEN_GRUNT, "Alient Grunt", { 11, 10, 8 }, { 22, 23, 24, 25, 26 } );
	AddExtraClassInfo( CLASS_NPC_TENTACLE, "Tentacle" );
	AddExtraClassInfo( CLASS_NPC_SENTRY, "Sentry", { 2, 2, 2 }, { 0, 5 } );
	AddExtraClassInfo( CLASS_NPC_TURRET, "Turret", { 1, 1, 2 }, { 0, 5 } );
	AddExtraClassInfo( CLASS_NPC_LEECH, "Leech", {  }, { 3, 6, 7 } );
	AddExtraClassInfo( CLASS_NPC_GMAN, "G-Man" );
	AddExtraClassInfo( CLASS_NPC_FEMALE_ASSASSIN, "Female Assassin", { 13, 12, 11 }, { 10, 11, 12 } );
	AddExtraClassInfo( CLASS_NPC_MALE_ASSASSIN, "Male Assassin", { 14, 7, 7 }, { 32, 33, 34, 35, 36, 37, 50 } );
	AddExtraClassInfo( CLASS_NPC_MALE_SNIPER_ASSASSIN, "Male Sniper Assassin" );
	AddExtraClassInfo( CLASS_NPC_AGENT, "Agent", { 11, 10, 8 }, { 55 } );
	AddExtraClassInfo( CLASS_NPC_SNARK, "Snark" );
	AddExtraClassInfo( CLASS_NPC_CHUMTOAD, "Chumtoad" );
	AddExtraClassInfo( CLASS_NPC_ALIEN_CONTROLLER, "Alien Controller", { 22, 9, 8 }, { 8, 18 } );
	AddExtraClassInfo( CLASS_NPC_ICHTYOSAUR, "Ichtyosaur", { 15, 3, 2 }, { 3, 4, 7 } );
	AddExtraClassInfo( CLASS_NPC_GARGANTUA, "Gargantua", { 18, 16, 0 }, { 14 } );
	AddExtraClassInfo( CLASS_NPC_BABY_GARGANTUA, "Baby Gargantua", {  }, { 14 } );
	AddExtraClassInfo( CLASS_NPC_BIG_MOMMA, "Big Momma", { 12, 12, 12 }, { 4, 19 } );
	AddExtraClassInfo( CLASS_NPC_OSPREY, "Osprey", { 6, 6, 2 } );
	AddExtraClassInfo( CLASS_NPC_BLACK_OPS_OSPREY, "Black Ops Osprey", { 6, 6, 2 } );
	AddExtraClassInfo( CLASS_NPC_DESTROYED_OSPREY, "Destroyed Osprey" );
	AddExtraClassInfo( CLASS_NPC_APACHE, "Apache", { 2, 2, 0 } );
	AddExtraClassInfo( CLASS_NPC_NIHILANTH, "Nihilanth", { 3, 3, 3 }, { 12 } );
	AddExtraClassInfo( CLASS_NPC_BOID, "Boid" );
	AddExtraClassInfo( CLASS_NPC_HEV, "H.E.V." );
	AddExtraClassInfo( CLASS_NPC_SPORE_AMMO, "Spore Ammo" );
	AddExtraClassInfo( CLASS_NPC_HUMAN_GRUNT_OPFOR, "Human Grunt" );
	AddExtraClassInfo( CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH, "Torch | Human Grunt" );
	AddExtraClassInfo( CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC, "Medic | Human Grunt" );
	AddExtraClassInfo( CLASS_NPC_GONOME, "Gonome", { 15, 13 /* 14 */, 12 }, { 11, 12, 13, 14, 15 } );
	AddExtraClassInfo( CLASS_NPC_PIT_DRONE, "Pit Drone", { 2, 1, 0 }, { 15, 16, 17 } );
	AddExtraClassInfo( CLASS_NPC_SHOCK_TROOPER, "Shock Trooper", { 3, 2, 1 }, { 27, 28, 29, 30, 31, 32 } );
	AddExtraClassInfo( CLASS_NPC_VOLTIGORE, "Voltigore", { 19, 18, 29 }, { 13, 14, 15 } );
	AddExtraClassInfo( CLASS_NPC_BABY_VOLTIGORE, "Baby Voltigore", { 17, 24, 14 } );
	AddExtraClassInfo( CLASS_NPC_PIT_WORM, "Pit Worm" );
	AddExtraClassInfo( CLASS_NPC_GENEWORM, "Geneworm" );
	AddExtraClassInfo( CLASS_NPC_SHOCK_RIFLE, "Shock Rifle", { 1, 0, 13 }, { 7 } );
	AddExtraClassInfo( CLASS_NPC_MORTAR, "Mortar" );
	AddExtraClassInfo( CLASS_NPC_STUKABAT, "Stukabat", {  }, { 5, 6, 14 } );
	AddExtraClassInfo( CLASS_NPC_KINGPIN, "Kingpin", {  }, { 5 } );
	AddExtraClassInfo( CLASS_NPC_TOR, "Xen Commander" );
	AddExtraClassInfo( CLASS_NPC_HEAVY_GRUNT, "Heavy Weapons Grunt", {  }, { 11, 12, 13 } );
	AddExtraClassInfo( CLASS_NPC_ROBOT_GRUNT, "Robot Grunt", {  }, { 35, 36, 37, 38, 39, 40, 53 } );
	AddExtraClassInfo( CLASS_NPC_ZOMBIE_BULL, "Zombie Bull", {  }, { 35, 36, 37, 38, 39, 40, 53 } );
	AddExtraClassInfo( CLASS_NPC_THE_HAND, "The Hand" );
	AddExtraClassInfo( CLASS_NPC_CHICKEN, "Chicken", { 5, 4, 3 }, { 11 } );
	AddExtraClassInfo( CLASS_NPC_SHERIFF, "Sheriff", { 5, 4, 3 }, { 11 } );
	AddExtraClassInfo( CLASS_NPC_CYBERFRANKLIN, "Cyber Franklin", { 20, 22, 10 }, { 11 } );
	AddExtraClassInfo( CLASS_NPC_MANTA, "Manta" );
	AddExtraClassInfo( CLASS_NPC_BARNABUS, "Barnabus", { 11, 10, 9 }, { 11 } );
	AddExtraClassInfo( CLASS_NPC_SKELETON, "Skeleton" );
	AddExtraClassInfo( CLASS_NPC_PIRANHA, "Piranha" );

	// Wouldn't add them tbh
	AddExtraClassInfo( CLASS_NPC_SPECFOR_GRUNT, "Special Forces Grunt" );
	AddExtraClassInfo( CLASS_NPC_BARNIEL, "Barniel" );
	AddExtraClassInfo( CLASS_NPC_ARCHER, "Archer" );
	AddExtraClassInfo( CLASS_NPC_PANTHEREYE, "Panthereye" );
	AddExtraClassInfo( CLASS_NPC_FIONA, "Fiona" );
	AddExtraClassInfo( CLASS_NPC_TWITCHER, "Twitcher" );
	AddExtraClassInfo( CLASS_NPC_SPITTER, "Spitter" );
	AddExtraClassInfo( CLASS_NPC_HANDCRAB, "Handcrab" );
	AddExtraClassInfo( CLASS_NPC_GHOST, "Ghost" );
	AddExtraClassInfo( CLASS_NPC_SCREAMER, "Screamer" );
	AddExtraClassInfo( CLASS_NPC_DEVOURER, "Devourer" );
	AddExtraClassInfo( CLASS_NPC_WHEELCHAIR, "Wheelchair" );
	AddExtraClassInfo( CLASS_NPC_FACE, "Face" );
	AddExtraClassInfo( CLASS_NPC_HELLHOUND, "Hellhound" );
	AddExtraClassInfo( CLASS_NPC_ADDICTION, "Addiction" );
	AddExtraClassInfo( CLASS_NPC_SHARK, "Shark" );

	// Items
	AddExtraClassInfo( CLASS_ITEM_HEV, "H.E.V." );
	AddExtraClassInfo( CLASS_ITEM_MEDKIT, "Medkit" );
	AddExtraClassInfo( CLASS_ITEM_BATTERY, "Suit Battery" );
	AddExtraClassInfo( CLASS_ITEM_GLOCK_AMMO, "Glock Ammo" );
	AddExtraClassInfo( CLASS_ITEM_PYTHON_AMMO, ".357 Ammo" );
	AddExtraClassInfo( CLASS_ITEM_SHOTGUN_AMMO, "Shotgun Ammo" );
	AddExtraClassInfo( CLASS_ITEM_UZI_AMMO, "UZI Ammo" );
	AddExtraClassInfo( CLASS_ITEM_MP5_AMMO, "MP5 Ammo" );
	AddExtraClassInfo( CLASS_ITEM_MP5_AMMO2, "MP5 Ammo" );
	AddExtraClassInfo( CLASS_ITEM_CHAIN_AMMO, "Chain Ammo" );
	AddExtraClassInfo( CLASS_ITEM_CROSSBOW_AMMO, "Crossbow Ammo" );
	AddExtraClassInfo( CLASS_ITEM_GAUSS_AMMO, "Gauss Ammo" );
	AddExtraClassInfo( CLASS_ITEM_RPG_AMMO, "RPG Ammo" );
	AddExtraClassInfo( CLASS_ITEM_SNIPER_RIFLE_AMMO, "Sniper Rifle Ammo" );
	AddExtraClassInfo( CLASS_ITEM_MACHINEGUN_AMMO, "Machine Gun Ammo" );
	AddExtraClassInfo( CLASS_ITEM_CROWBAR, "Crowbar" );
	AddExtraClassInfo( CLASS_ITEM_WRENCH, "Wrench" );
	AddExtraClassInfo( CLASS_ITEM_KNIFE, "Knife" );
	AddExtraClassInfo( CLASS_ITEM_BARNACLE_GRAPPLE, "Barnacle Grapple" );
	AddExtraClassInfo( CLASS_ITEM_GLOCK, "Glock" );
	AddExtraClassInfo( CLASS_ITEM_PYTHON, ".357" );
	AddExtraClassInfo( CLASS_ITEM_DEAGLE, "Deagle" );
	AddExtraClassInfo( CLASS_ITEM_SHOTGUN, "Shotgun" );
	AddExtraClassInfo( CLASS_ITEM_UZI, "Uzi" );
	AddExtraClassInfo( CLASS_ITEM_2UZIS, "Double Uzis" );
	AddExtraClassInfo( CLASS_ITEM_MP5, "MP5" );
	AddExtraClassInfo( CLASS_ITEM_M16, "M16" );
	AddExtraClassInfo( CLASS_ITEM_CROSSBOW, "Crossbow" );
	AddExtraClassInfo( CLASS_ITEM_GAUSS, "Gauss" );
	AddExtraClassInfo( CLASS_ITEM_EGON, "Gluon Gun" );
	AddExtraClassInfo( CLASS_ITEM_RPG, "RPG" );
	AddExtraClassInfo( CLASS_ITEM_HORNET_GUN, "Hornet Gun" );
	AddExtraClassInfo( CLASS_ITEM_SNIPER_RIFLE, "Sniper Rifle" );
	AddExtraClassInfo( CLASS_ITEM_MACHINEGUN, "Machine Gun" );
	AddExtraClassInfo( CLASS_ITEM_SPORE_LAUNCHER, "Spore Launcher" );
	AddExtraClassInfo( CLASS_ITEM_DISPLACER, "Displacer" );
	AddExtraClassInfo( CLASS_ITEM_MINIGUN, "Minigun" );
	AddExtraClassInfo( CLASS_ITEM_SNARK_NEST, "Snark Nest" );
	AddExtraClassInfo( CLASS_ITEM_GRENADE, "Grenade" );
	AddExtraClassInfo( CLASS_ITEM_SATCHEL, "Satchel Charge" );
	AddExtraClassInfo( CLASS_ITEM_ARGRENADE, "AR Grenade" );
	AddExtraClassInfo( CLASS_ITEM_TRIPMINE, "Tripmine" );
	AddExtraClassInfo( CLASS_ITEM_WEAPON_BOX, "Weapon Box" );
	AddExtraClassInfo( CLASS_ITEM_LONGJUMP, "Longjump" );
	AddExtraClassInfo( CLASS_ITEM_HEALTH_CHARGER, "Health Charger" );
	AddExtraClassInfo( CLASS_ITEM_HEV_CHARGER, "H.E.V. Charger" );
	AddExtraClassInfo( CLASS_ITEM_BARNEY_VEST, "Barney Vest" );
	AddExtraClassInfo( CLASS_ITEM_BARNEY_HELMET, "Barney Helmet" );
	AddExtraClassInfo( CLASS_ITEM_SUIT, "Suit" );
	AddExtraClassInfo( CLASS_ITEM_SPORE, "Spore" );
	AddExtraClassInfo( CLASS_ITEM_CROSSBOW_BOLT, "Crossbow Bolt" );
	AddExtraClassInfo( CLASS_ITEM_RPG_ROCKET, "RPG Rocket" );
	AddExtraClassInfo( CLASS_ITEM_HVR_ROCKET, "HVR Rocket" );
	AddExtraClassInfo( CLASS_ITEM_MORTAR_SHELL, "Mortar Shell" );

	// Custom Weapons/Items
	AddExtraClassInfo( CLASS_ITEM_TOMMY_GUN_AMMO, "Tommy Gun Ammo" );
	AddExtraClassInfo( CLASS_ITEM_GREASE_GUN_AMMO, "Grease Gun Ammo" );
	AddExtraClassInfo( CLASS_ITEM_M16_AMMO, "M16 Ammo" );
	AddExtraClassInfo( CLASS_ITEM_M14_AMMO, "M14 Ammo" );
	AddExtraClassInfo( CLASS_ITEM_SHOVEL, "Shovel" );
	AddExtraClassInfo( CLASS_ITEM_SPANNER, "Spanner" );
	AddExtraClassInfo( CLASS_ITEM_DOUBLE_BARREL, "Double Barrel" );
	AddExtraClassInfo( CLASS_ITEM_TOMMY_GUN, "Tommy Gun" );
	AddExtraClassInfo( CLASS_ITEM_GREASE_GUN, "Grease Gun" );
	AddExtraClassInfo( CLASS_ITEM_M14, "M14" );
	AddExtraClassInfo( CLASS_ITEM_TESLA_GUN, "Tesla Gun" );

	AddExtraClassInfo( CLASS_ITEM_CS_GLOCK, "Glock 18" );
	AddExtraClassInfo( CLASS_ITEM_CS_USP45, "USP45" );
	AddExtraClassInfo( CLASS_ITEM_CS_P228, "P228" );
	AddExtraClassInfo( CLASS_ITEM_CS_FN57, "FN 57" );
	AddExtraClassInfo( CLASS_ITEM_CS_DUAL_BERETTAS, "Dual Berettas" );
	AddExtraClassInfo( CLASS_ITEM_CS_DEAGLE, "Desert Eagle" );
	AddExtraClassInfo( CLASS_ITEM_CS_XM1014, "XM1014" );
	AddExtraClassInfo( CLASS_ITEM_CS_M3, "M3" );
	AddExtraClassInfo( CLASS_ITEM_CS_MAC10, "MAC-10" );
	AddExtraClassInfo( CLASS_ITEM_CS_TMP, "TMP" );
	AddExtraClassInfo( CLASS_ITEM_CS_MP5, "MP5" );
	AddExtraClassInfo( CLASS_ITEM_CS_UMP45, "UMP45" );
	AddExtraClassInfo( CLASS_ITEM_CS_P90, "FN P90" );
	AddExtraClassInfo( CLASS_ITEM_CS_FAMAS, "Famas" );
	AddExtraClassInfo( CLASS_ITEM_CS_GALIL, "Galil" );
	AddExtraClassInfo( CLASS_ITEM_CS_AK47, "AK47" );
	AddExtraClassInfo( CLASS_ITEM_CS_M4A1, "M4A1" );
	AddExtraClassInfo( CLASS_ITEM_CS_AUG, "AUG" );
	AddExtraClassInfo( CLASS_ITEM_CS_SG552, "SG552" );
	AddExtraClassInfo( CLASS_ITEM_CS_SCOUT, "Scout" );
	AddExtraClassInfo( CLASS_ITEM_CS_AWP, "AWP" );
	AddExtraClassInfo( CLASS_ITEM_CS_SG550, "SG550" );
	AddExtraClassInfo( CLASS_ITEM_CS_G3SG1, "G3SG1" );
	AddExtraClassInfo( CLASS_ITEM_CS_M249, "M249" );
	AddExtraClassInfo( CLASS_ITEM_CS_HEGRENADE, "HE" );
	AddExtraClassInfo( CLASS_ITEM_CS_C4, "C4" );

	AddExtraClassInfo( CLASS_ITEM_SPOOKY_GIFTS, "Spooky Gifts" );
	AddExtraClassInfo( CLASS_ITEM_XMAS_GIFTS, "Xmas Gifts" );
}

//-----------------------------------------------------------------------------
// Add class info
//-----------------------------------------------------------------------------

void CEntityList::AddExtraClassInfo( int id, const char *_name, std::vector<unsigned char> _aimbot_hitboxes, std::vector<unsigned char> _sequence_dead )
{
	m_extraClassInfo[ id ].name = _name;
	m_extraClassInfo[ id ].aimbot_hitboxes = _aimbot_hitboxes;
	m_extraClassInfo[ id ].sequence_dead = _sequence_dead;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CEntityList::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		for ( int i = 0; i <= MY_MAXENTS; i++ )
		{
			m_ents[ i ].m_bValid = false;
			m_ents[ i ].m_bPlayer = false;
			m_ents[ i ].m_bItem = false;
			m_ents[ i ].m_bAlive = false;
			m_ents[ i ].m_bDucked = false;
			m_ents[ i ].m_bVisible = false;
			m_ents[ i ].m_bFriend = false;
			m_ents[ i ].m_bEnemy = false;
			m_ents[ i ].m_bNeutral = true;
			m_ents[ i ].m_flLastEmitSoundTime = -1.f;
		}
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		Update();
	}
	// StudioRenderModel event
	else if ( ( studiorenderer->m_pPlayerInfo != NULL ||
			  studiorenderer->m_pPlayerInfo == NULL && !studiorenderer->m_pCurrentEntity->player ) && // skip player's viewmodel
			  studiorenderer->m_pCurrentEntity->index <= MY_MAXENTS )
	{
		UpdateHitboxes( studiorenderer->m_pCurrentEntity->index );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CEntityList::CEntityList( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ),
m_classTable( 1023 ),
m_modelsTable( 1023 )
{
	memset( m_ents, 0, Q_ARRAYSIZE( m_ents ) );

	m_pBoneTransform = NULL;

	m_extraClassInfo.resize( CLASS_LAST );

	AddClasses();
	AddExtraClassInfos();
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CEntityList::Load( void )
{
	for ( int i = 0; i <= MY_MAXENTS; i++ )
		m_ents[ i ].m_rgHitboxes = (Vector *)MemCalloc( MAXSTUDIOBONES, sizeof( Vector ), "m_rgHitboxes" );

	m_pBoneTransform = (bone_matrix3x4_t *)enginestudio->StudioGetLightTransform();

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent, kHookCall, kHookPriorityHigh );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookCall, kHookPriorityHigh );
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent, kHookCall, kHookPriorityHigh );

	return true;
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CEntityList::Unload( void )
{
	for ( int i = 0; i <= MY_MAXENTS; i++ )
	{
		if ( m_ents[ i ].m_rgHitboxes != NULL )
			MemFree( m_ents[ i ].m_rgHitboxes );
	}
}

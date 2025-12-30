// SvenInt (c) Sw1ft
// misc_entity_list.h

#ifndef SINT_FEATURE_ENTITY_LIST_H
#define SINT_FEATURE_ENTITY_LIST_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "utils/hashdict.h"
#include "utils/hashtable.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define MY_MAXENTS 2048

//-----------------------------------------------------------------------------
// Bone matrix
//-----------------------------------------------------------------------------

typedef float bone_matrix3x4_t[ MAXSTUDIOBONES ][ 3 ][ 4 ];

//-----------------------------------------------------------------------------
// Class flags
//-----------------------------------------------------------------------------

// 16 flags only
#define FL_CLASS_FRIEND (1 << 0)
#define FL_CLASS_ENEMY (1 << 1)
#define FL_CLASS_ITEM (1 << 2)
#define FL_CLASS_NEUTRAL (1 << 3)
#define FL_CLASS_WORLD_ENTITY (1 << 4)
#define FL_CLASS_CORPSE (1 << 5)
//#define FL_CLASS_UNDEFINED_7 (1 << 6)
//#define FL_CLASS_UNDEFINED_8 (1 << 7)
//#define FL_CLASS_UNDEFINED_9 (1 << 8)
//#define FL_CLASS_UNDEFINED_10 (1 << 9)
//#define FL_CLASS_UNDEFINED_11 (1 << 10)
//#define FL_CLASS_UNDEFINED_12 (1 << 11)
//#define FL_CLASS_UNDEFINED_13 (1 << 12)
//#define FL_CLASS_UNDEFINED_14 (1 << 13)
//#define FL_CLASS_UNDEFINED_15 (1 << 14)
//#define FL_CLASS_UNDEFINED_16 (1 << 15)

//-----------------------------------------------------------------------------
// Class ID
//-----------------------------------------------------------------------------

typedef enum
{
	CLASS_NONE = 0,

	CLASS_PLAYER,
	CLASS_DEAD_PLAYER,

	CLASS_OBJECT_CP,

	// NPCs
	CLASS_NPC_GORDON_FREEMAN,
	CLASS_NPC_SCIENTIST,
	CLASS_NPC_BARNEY,
	CLASS_NPC_OTIS,
	CLASS_NPC_HEADCRAB,
	CLASS_NPC_BABY_HEADCRAB,
	CLASS_NPC_ZOMBIE,
	CLASS_NPC_ZOMBIE_SOLDIER,
	CLASS_NPC_BULLSQUID,
	CLASS_NPC_HOUNDEYE,
	CLASS_NPC_BARNACLE,
	CLASS_NPC_VORTIGAUNT,
	CLASS_NPC_HUMAN_GRUNT,
	CLASS_NPC_HUMAN_GRUNT_SNIPER,
	CLASS_NPC_ALIEN_GRUNT,
	CLASS_NPC_TENTACLE,
	CLASS_NPC_SENTRY,
	CLASS_NPC_TURRET,
	CLASS_NPC_LEECH,
	CLASS_NPC_GMAN,
	CLASS_NPC_FEMALE_ASSASSIN,
	CLASS_NPC_MALE_ASSASSIN,
	CLASS_NPC_MALE_SNIPER_ASSASSIN,
	CLASS_NPC_AGENT,
	CLASS_NPC_SNARK,
	CLASS_NPC_CHUMTOAD,
	CLASS_NPC_ALIEN_CONTROLLER,
	CLASS_NPC_ICHTYOSAUR,
	CLASS_NPC_GARGANTUA,
	CLASS_NPC_BABY_GARGANTUA,
	CLASS_NPC_BIG_MOMMA,
	CLASS_NPC_OSPREY,
	CLASS_NPC_BLACK_OPS_OSPREY,
	CLASS_NPC_DESTROYED_OSPREY,
	CLASS_NPC_APACHE,
	CLASS_NPC_NIHILANTH,
	CLASS_NPC_BOID,
	CLASS_NPC_HEV,
	CLASS_NPC_SPORE_AMMO,
	CLASS_NPC_HUMAN_GRUNT_OPFOR,
	CLASS_NPC_HUMAN_GRUNT_OPFOR_TORCH,
	CLASS_NPC_HUMAN_GRUNT_OPFOR_MEDIC,
	CLASS_NPC_GONOME,
	CLASS_NPC_PIT_DRONE,
	CLASS_NPC_SHOCK_TROOPER,
	CLASS_NPC_VOLTIGORE,
	CLASS_NPC_BABY_VOLTIGORE,
	CLASS_NPC_PIT_WORM,
	CLASS_NPC_GENEWORM,
	CLASS_NPC_SHOCK_RIFLE,
	CLASS_NPC_MORTAR,
	CLASS_NPC_STUKABAT,
	CLASS_NPC_KINGPIN,
	CLASS_NPC_TOR,
	CLASS_NPC_HEAVY_GRUNT,
	CLASS_NPC_ROBOT_GRUNT,
	CLASS_NPC_ZOMBIE_BULL,
	CLASS_NPC_THE_HAND,
	CLASS_NPC_CHICKEN,
	CLASS_NPC_SHERIFF,
	CLASS_NPC_CYBERFRANKLIN,
	CLASS_NPC_MANTA,
	CLASS_NPC_BARNABUS, // wtf is this?
	CLASS_NPC_SKELETON,
	CLASS_NPC_PIRANHA,

	// Wouldn't add them tbh
	CLASS_NPC_SPECFOR_GRUNT,
	CLASS_NPC_BARNIEL,
	CLASS_NPC_ARCHER,
	CLASS_NPC_PANTHEREYE,
	CLASS_NPC_FIONA,
	CLASS_NPC_TWITCHER,
	CLASS_NPC_SPITTER,
	CLASS_NPC_HANDCRAB,
	CLASS_NPC_GHOST,
	CLASS_NPC_SCREAMER,
	CLASS_NPC_DEVOURER,
	CLASS_NPC_WHEELCHAIR,
	CLASS_NPC_FACE,
	CLASS_NPC_HELLHOUND,
	CLASS_NPC_ADDICTION,
	CLASS_NPC_SHARK,

	// Items
	CLASS_ITEM_HEV,
	CLASS_ITEM_MEDKIT,
	CLASS_ITEM_BATTERY,
	CLASS_ITEM_GLOCK_AMMO,
	CLASS_ITEM_PYTHON_AMMO,
	CLASS_ITEM_SHOTGUN_AMMO,
	CLASS_ITEM_UZI_AMMO,
	CLASS_ITEM_MP5_AMMO,
	CLASS_ITEM_MP5_AMMO2,
	CLASS_ITEM_CHAIN_AMMO,
	CLASS_ITEM_CROSSBOW_AMMO,
	CLASS_ITEM_GAUSS_AMMO,
	CLASS_ITEM_RPG_AMMO,
	CLASS_ITEM_SNIPER_RIFLE_AMMO,
	CLASS_ITEM_MACHINEGUN_AMMO,
	CLASS_ITEM_CROWBAR,
	CLASS_ITEM_WRENCH,
	CLASS_ITEM_KNIFE,
	CLASS_ITEM_BARNACLE_GRAPPLE,
	CLASS_ITEM_GLOCK,
	CLASS_ITEM_PYTHON,
	CLASS_ITEM_DEAGLE,
	CLASS_ITEM_SHOTGUN,
	CLASS_ITEM_UZI,
	CLASS_ITEM_2UZIS,
	CLASS_ITEM_MP5,
	CLASS_ITEM_M16,
	CLASS_ITEM_CROSSBOW,
	CLASS_ITEM_GAUSS,
	CLASS_ITEM_EGON,
	CLASS_ITEM_RPG,
	CLASS_ITEM_HORNET_GUN,
	CLASS_ITEM_SNIPER_RIFLE,
	CLASS_ITEM_MACHINEGUN,
	CLASS_ITEM_SPORE_LAUNCHER,
	CLASS_ITEM_DISPLACER,
	CLASS_ITEM_MINIGUN,
	CLASS_ITEM_SNARK_NEST,
	CLASS_ITEM_GRENADE,
	CLASS_ITEM_SATCHEL,
	CLASS_ITEM_ARGRENADE,
	CLASS_ITEM_TRIPMINE,
	CLASS_ITEM_WEAPON_BOX,
	CLASS_ITEM_LONGJUMP,
	CLASS_ITEM_HEALTH_CHARGER,
	CLASS_ITEM_HEV_CHARGER,
	CLASS_ITEM_BARNEY_VEST,
	CLASS_ITEM_BARNEY_HELMET,
	CLASS_ITEM_SUIT,
	CLASS_ITEM_SPORE,
	CLASS_ITEM_CROSSBOW_BOLT,
	CLASS_ITEM_RPG_ROCKET,
	CLASS_ITEM_HVR_ROCKET,
	CLASS_ITEM_MORTAR_SHELL,

	// Custom Weapons/Items
	CLASS_ITEM_TOMMY_GUN_AMMO,
	CLASS_ITEM_GREASE_GUN_AMMO,
	CLASS_ITEM_M16_AMMO,
	CLASS_ITEM_M14_AMMO,
	CLASS_ITEM_SHOVEL,
	CLASS_ITEM_SPANNER,
	CLASS_ITEM_DOUBLE_BARREL,
	CLASS_ITEM_TOMMY_GUN,
	CLASS_ITEM_GREASE_GUN,
	CLASS_ITEM_M14,
	CLASS_ITEM_TESLA_GUN,

	CLASS_ITEM_CS_GLOCK,
	CLASS_ITEM_CS_USP45,
	CLASS_ITEM_CS_P228,
	CLASS_ITEM_CS_FN57,
	CLASS_ITEM_CS_DUAL_BERETTAS,
	CLASS_ITEM_CS_DEAGLE,
	CLASS_ITEM_CS_XM1014,
	CLASS_ITEM_CS_M3,
	CLASS_ITEM_CS_MAC10,
	CLASS_ITEM_CS_TMP,
	CLASS_ITEM_CS_MP5,
	CLASS_ITEM_CS_UMP45,
	CLASS_ITEM_CS_P90,
	CLASS_ITEM_CS_FAMAS,
	CLASS_ITEM_CS_GALIL,
	CLASS_ITEM_CS_AK47,
	CLASS_ITEM_CS_M4A1,
	CLASS_ITEM_CS_AUG,
	CLASS_ITEM_CS_SG552,
	CLASS_ITEM_CS_SCOUT,
	CLASS_ITEM_CS_AWP,
	CLASS_ITEM_CS_SG550,
	CLASS_ITEM_CS_G3SG1,
	CLASS_ITEM_CS_M249,
	CLASS_ITEM_CS_HEGRENADE,
	CLASS_ITEM_CS_C4,

	CLASS_ITEM_SPOOKY_GIFTS,
	CLASS_ITEM_XMAS_GIFTS,

	CLASS_LAST
} EEntityClassID;

//-----------------------------------------------------------------------------
// Class structures
//-----------------------------------------------------------------------------

// Access to indexes of hitboxes
#define HITBOX_HEAD		0
#define HITBOX_NECK		1
#define HITBOX_CHEST	2

typedef struct class_info_s
{
	unsigned short flags;
	unsigned short id;
} class_info_t;

typedef struct extra_class_info_s
{
	extra_class_info_s( const char *_name = NULL,
						std::vector<unsigned char> _aimbot_hitboxes = {},
						std::vector<unsigned char> _sequence_dead = {} )
	{
		name = _name;
		aimbot_hitboxes = _aimbot_hitboxes;
		sequence_dead = _sequence_dead;
	}

	const char *name;
	std::vector<unsigned char> aimbot_hitboxes; // Head, neck, chest
	std::vector<unsigned char> sequence_dead;
} extra_class_info_t;

//-----------------------------------------------------------------------------
// Entity Class
//-----------------------------------------------------------------------------

class CEntity
{
public:
	bool m_bValid : 1;
	bool m_bPlayer : 1;
	bool m_bItem : 1;
	bool m_bAlive : 1;
	bool m_bDucked : 1;
	bool m_bVisible : 1;
	bool m_bFriend : 1;
	bool m_bEnemy : 1;
	bool m_bNeutral : 1;

	cl_entity_s *m_pEntity;
	studiohdr_t *m_pStudioHeader;

	Vector m_vecOrigin;
	Vector m_vecPrevOrigin;
	Vector m_vecVelocity;

	Vector m_vecMins;
	Vector m_vecMaxs;

	float m_flLastEmitSoundTime;
	float m_flHealth;

	class_info_t m_classInfo;

	Vector *m_rgHitboxes;
};

//-----------------------------------------------------------------------------
// Entity list feature
//-----------------------------------------------------------------------------

class CEntityList final : public CBaseFeature, IHookEventListener
{
public:
	CEntityList( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline CEntity *GetList( void ) { return m_ents; }
	inline int GetMaxEntities( void ) { return MY_MAXENTS; }
	inline bone_matrix3x4_t *GetBoneTransform( void ) { return m_pBoneTransform; }

	void Update( void );
	void UpdateHitboxes( int index );

public:
	inline const char *GetEntityClassname( class_info_t &classInfo ) { return m_extraClassInfo[ classInfo.id ].name; }
	inline bool IsEntityClassFriend( class_info_t &classInfo ) { return classInfo.flags & FL_CLASS_FRIEND; }
	inline bool IsEntityClassEnemy( class_info_t &classInfo ) { return classInfo.flags & FL_CLASS_ENEMY; }
	inline bool IsEntityClassNeutral( class_info_t &classInfo ) { return classInfo.flags & FL_CLASS_NEUTRAL; }
	inline bool IsEntityClassItem( class_info_t &classInfo ) { return classInfo.flags & FL_CLASS_ITEM; }
	inline bool IsEntityClassCorpse( class_info_t &classInfo, int iSolid ) { return iSolid == SOLID_BBOX && classInfo.flags & FL_CLASS_CORPSE; }
	inline bool IsEntityClassTrash( class_info_t &classInfo ) { return classInfo.flags & FL_CLASS_WORLD_ENTITY; }
	inline extra_class_info_t &GetExtraEntityClassInfo( EEntityClassID id ) { return m_extraClassInfo[ id ]; }

	class_info_t GetEntityClassInfo( const char *pszModelname );

private:
	void AddClasses( void );
	void AddClassInfo( const char *pszModelname, int id, int flags );

	void AddExtraClassInfos( void );
	void AddExtraClassInfo( int id, const char *_name = NULL, std::vector<unsigned char> _aimbot_hitboxes = {}, std::vector<unsigned char> _sequence_dead = {} );

private:
	CEntity m_ents[ MY_MAXENTS + 1 ];
	bone_matrix3x4_t *m_pBoneTransform;

	CHashTable<uint32_t, class_info_t> m_classTable;
	CHashDict<class_info_t, true, false> m_modelsTable;

	std::vector<extra_class_info_t> m_extraClassInfo;
};

EXTERN_FEATURE( CEntityList, entitylist );

#endif // SINT_FEATURE_ENTITY_LIST_H

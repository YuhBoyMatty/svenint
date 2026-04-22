/**
 * Copyright - id Software, ?.
 * Copyright - Valve Software, ?.
 * Copyright - xWhitey, 2022-2025.
 * Copyright - Sw1ft, 2025.
 * hlsdk_mini.hpp - Lightweight version of HLSDK, including some undocumented engine structs.
 */

#ifdef HLSDK_MINI_HPP_RECURSE_GUARD
#error Recursive header files inclusion detected in hlsdk_mini.hpp
#else //HLSDK_MINI_HPP_RECURSE_GUARD

#define HLSDK_MINI_HPP_RECURSE_GUARD

#ifndef HLSDK_MINI_HPP_GUARD
#define HLSDK_MINI_HPP_GUARD

#ifdef _WIN32
#pragma once
#else
#include <stdint.h>
#endif

#ifndef MAX_PATH
#define MAX_PATH 260
#endif //MAX_PATH

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif //!M_PI

#include "game/mathlib.h"

#define MAX_PHYSINFO_STRING 256

//Still handing the case when no bool type?
#ifndef __cplusplus
typedef enum
{
	false,
	true
} qboolean;
#else
typedef int qboolean;
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef Q_ARRAYSIZE
#define Q_ARRAYSIZE( arr ) sizeof( arr ) / sizeof( arr[ 0 ] )
#endif

#ifndef Q_min
#define Q_min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef Q_max
#define Q_max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef Q_clamp
#define Q_clamp(val, minval, maxval) (((val) > (maxval)) ? (maxval) : (((val) < (minval)) ? (minval) : (val)))
#endif

//Copyright goes to Sw1ft747.
enum WeaponId
{
	WEAPON_NONE = 0, // WEAPON_NONE can be 0 and -1
	WEAPON_CUSTOM = WEAPON_NONE, // I doubt that it's right, but still..

	WEAPON_CROWBAR,
	WEAPON_GLOCK,
	WEAPON_PYTHON,
	WEAPON_MP5,
	WEAPON_CHAINGUN,
	WEAPON_CROSSBOW,
	WEAPON_SHOTGUN,
	WEAPON_RPG,
	WEAPON_GAUSS,
	WEAPON_EGON,
	WEAPON_HORNETGUN,
	WEAPON_HANDGRENADE,
	WEAPON_TRIPMINE,
	WEAPON_SATCHEL,
	WEAPON_SNARK,

	WEAPON_UNDEFINED_16,

	WEAPON_UZI,
	WEAPON_MEDKIT,
	WEAPON_CROWBAR_ELECTRIC,
	WEAPON_WRENCH,
	WEAPON_MINIGUN,
	WEAPON_BARNACLE_GRAPPLE,
	WEAPON_SNIPER_RIFLE,
	WEAPON_M249,
	WEAPON_M16,
	WEAPON_SPORE_LAUNCHER,
	WEAPON_DESERT_EAGLE,
	WEAPON_SHOCK_RIFLE,
	WEAPON_DISPLACER,

	WEAPON_UNDEFINED_30,

	//WEAPON_SUIT = 31 // is it even needed if Sven has custom weapons?
};

#define DMG_GENERIC 0			 // generic damage was done
#define DMG_CRUSH (1 << 0)		 // crushed by falling or moving object
#define DMG_BULLET (1 << 1)		 // shot
#define DMG_SLASH (1 << 2)		 // cut, clawed, stabbed
#define DMG_BURN (1 << 3)		 // heat burned
#define DMG_FREEZE (1 << 4)		 // frozen
#define DMG_FALL (1 << 5)		 // fell too far
#define DMG_BLAST (1 << 6)		 // explosive blast damage
#define DMG_CLUB (1 << 7)		 // crowbar, punch, headbutt
#define DMG_SHOCK (1 << 8)		 // electric shock
#define DMG_SONIC (1 << 9)		 // sound pulse shockwave
#define DMG_ENERGYBEAM (1 << 10) // laser or other high energy beam
#define DMG_NEVERGIB (1 << 12)	 // with this bit OR'd in, no damage type will be able to gib victims upon death
#define DMG_ALWAYSGIB (1 << 13)	 // with this bit OR'd in, any damage type can be made to gib victims upon death.

// time-based damage
//mask off TF-specific stuff too
#define DMG_TIMEBASED (~(0xff003fff)) // mask for time-based damage

#define DMG_DROWN (1 << 14) // Drowning
#define DMG_FIRSTTIMEBASED DMG_DROWN

#define DMG_PARALYZE (1 << 15)	   // slows affected creature down
#define DMG_NERVEGAS (1 << 16)	   // nerve toxins, very bad
#define DMG_POISON (1 << 17)	   // blood poisioning
#define DMG_RADIATION (1 << 18)	   // radiation exposure
#define DMG_DROWNRECOVER (1 << 19) // drowning recovery
#define DMG_ACID (1 << 20)		   // toxic chemicals or acid burns
#define DMG_SLOWBURN (1 << 21)	   // in an oven
#define DMG_SLOWFREEZE (1 << 22)   // in a subzero freezer
#define DMG_MORTAR (1 << 23)	   // Hit by air raid (done to distinguish grenade from mortar)

//TF ADDITIONS
#define DMG_IGNITE (1 << 24)	   // Players hit by this begin to burn
#define DMG_RADIUS_MAX (1 << 25)   // Radius damage with this flag doesn't decrease over distance
#define DMG_RADIUS_QUAKE (1 << 26) // Radius damage is done like Quake. 1/2 damage at 1/2 radius.
#define DMG_IGNOREARMOR (1 << 27)  // Damage ignores target's armor
#define DMG_AIMED (1 << 28)		   // Does Hit location damage
#define DMG_WALLPIERCING (1 << 29) // Blast Damages ents through walls

#define DMG_CALTROP (1 << 30)
#define DMG_HALLUC (1 << 31)

// TF Healing Additions for TakeHealth
#define DMG_IGNORE_MAXHEALTH DMG_IGNITE
// TF Redefines since we never use the originals
#define DMG_NAIL DMG_SLASH
#define DMG_NOT_SELF DMG_FREEZE


#define DMG_TRANQ DMG_MORTAR
#define DMG_CONCUSS DMG_SONIC

// these are the damage types that are allowed to gib corpses
#define DMG_GIB_CORPSE (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_SONIC | DMG_CLUB)

// these are the damage types that have client hud art
#define DMG_SHOWNHUD (DMG_POISON | DMG_ACID | DMG_FREEZE | DMG_SLOWFREEZE | DMG_DROWN | DMG_BURN | DMG_SLOWBURN | DMG_NERVEGAS | DMG_RADIATION | DMG_SHOCK)

/* CVAR FLAGS - START */
#define FCVAR_ARCHIVE        (1<<0)    // set to cause it to be saved to vars.rc
#define FCVAR_USERINFO        (1<<1)    // changes the client's info string
#define FCVAR_SERVER        (1<<2)    // notifies players when changed
#define	FCVAR_EXTDLL        (1<<3)    // defined by external DLL
#define FCVAR_CLIENTDLL     (1<<4)  // defined by the client dll
#define FCVAR_PROTECTED     (1<<5)  // It's a server cvar, but we don't send the data since it's a password, etc.  Sends 1 if it's not bland/zero, 0 otherwise as value
#define FCVAR_SPONLY        (1<<6)  // This cvar cannot be changed by clients connected to a multiplayer server.
#define FCVAR_PRINTABLEONLY (1<<7)  // This cvar's string cannot contain unprintable characters ( e.g., used for player name etc ).
#define FCVAR_UNLOGGED        (1<<8)  // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log
#define FCVAR_NOEXTRAWHITEPACE    (1<<9)  // strip trailing/leading white space from this cvar
/* CVAR FLAGS - END */

typedef struct cmd_function_s
{
	struct cmd_function_s *next;
	char *name;
	void ( *function )( void );
	int flags;
} cmd_function_t;

#if !defined( CACHE_USER ) && !defined( QUAKEDEF_H )
#define CACHE_USER
typedef struct cache_user_s
{
	void *data;
} cache_user_t;
#endif

typedef struct
{
	unsigned char r, g, b;
} color24;

typedef struct
{
	unsigned r, g, b, a;
} colorVec;

typedef struct entity_state_s entity_state_t;

struct entity_state_s
{
	// Fields which are filled in by routines outside of delta compression
	int			entityType;
	// Index into cl_entities array for this entity.
	int			number;
	float		msg_time;

	// Message number last time the player/entity state was updated.
	int			messagenum;

	// Fields which can be transitted and reconstructed over the network stream
	vec3_t		origin;
	vec3_t		angles;

	int			modelindex;
	int			sequence;
	float		frame;
	int			colormap;
	short		skin;
	short		solid;
	int			effects;
	float		scale;

	unsigned char		eflags;

	// Render information
	int			rendermode;
	int			renderamt;
	color24		rendercolor;
	int			renderfx;

	int			movetype;
	float		animtime;
	float		framerate;
	int			body;
	unsigned char		controller[ 4 ];
	unsigned char		blending[ 4 ];
	vec3_t		velocity;

	// Send bbox down to client for use during prediction.
	vec3_t		mins;
	vec3_t		maxs;

	int			aiment;
	// If owned by a player, the index of that player ( for projectiles ).
	int			owner;

	// Friction, for prediction.
	float		friction;
	// Gravity multiplier
	float		gravity;

	// PLAYER SPECIFIC
	int			team;
	int			playerclass;
	int			health;
	qboolean	spectator;
	int         weaponmodel;
	int			gaitsequence;
	// If standing on conveyor, e.g.
	vec3_t		basevelocity;
	// Use the crouched hull, or the regular player hull.
	int			usehull;
	// Latched buttons last time state updated.
	int			oldbuttons;
	// -1 = in air, else pmove entity number
	int			onground;
	int			iStepLeft;
	// How fast we are falling
	float		flFallVelocity;

	float		fov;
	int			weaponanim;

	// Parametric movement overrides
	vec3_t				startpos;
	vec3_t				endpos;
	float				impacttime;
	float				starttime;

	// For mods
	int			iuser1;
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
	vec3_t		vuser1;
	vec3_t		vuser2;
	vec3_t		vuser3;
	vec3_t		vuser4;
};

// edict->flags
#define FL_FLY           (1 << 0) // Changes the SV_Movestep() behavior to not need to be on ground
#define FL_SWIM          (1 << 1) // Changes the SV_Movestep() behavior to not need to be on ground (but stay in water)
#define FL_CONVEYOR      (1 << 2)
#define FL_CLIENT        (1 << 3)
#define FL_INWATER       (1 << 4)
#define FL_MONSTER       (1 << 5)
#define FL_GODMODE       (1 << 6)
#define FL_NOTARGET      (1 << 7)
#define FL_SKIPLOCALHOST (1 << 8) // Don't send entity to local host, it's predicting this entity itself
#define FL_ONGROUND      (1 << 9) // At rest / on the ground
#define FL_PARTIALGROUND (1 << 10) // not all corners are valid
#define FL_WATERJUMP     (1 << 11) // player jumping out of water
#define FL_FROZEN        (1 << 12) // Player is frozen for 3rd person camera
#define FL_FAKECLIENT    (1 << 13) // JAC: fake client, simulated server side; don't send network messages to them
#define FL_DUCKING       (1 << 14) // Player flag -- Player is fully crouched
#define FL_FLOAT         (1 << 15) // Apply floating force to this entity when in water
#define FL_GRAPHED       (1 << 16) // worldgraph has this ent listed as something that blocks a connection

// UNDONE: Do we need these?
#define FL_IMMUNE_WATER (1 << 17)
#define FL_IMMUNE_SLIME (1 << 18)
#define FL_IMMUNE_LAVA  (1 << 19)

#define FL_PROXY        (1 << 20) // This is a spectator proxy
#define FL_ALWAYSTHINK  (1 << 21) // Brush model flag -- call think every frame regardless of nextthink - ltime (for constantly changing velocity/path)
#define FL_BASEVELOCITY (1 << 22) // Base velocity has been applied this frame (used to convert base velocity into momentum)
#define FL_MONSTERCLIP  (1 << 23) // Only collide in with monsters who have FL_MONSTERCLIP set
#define FL_ONTRAIN      (1 << 24) // Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
#define FL_WORLDBRUSH   (1 << 25) // Not moveable/removeable brush entity (really part of the world, but represented as an entity for transparency or something)
#define FL_SPECTATOR    (1 << 26) // This client is a spectator, don't run touch functions, etc.
#define FL_CUSTOMENTITY (1 << 29) // This is a custom entity
#define FL_KILLME       (1 << 30) // This entity is marked for death -- This allows the engine to kill ents at the appropriate time
#define FL_DORMANT      (1 << 31) // Entity is dormant, no updates to client

// Goes into globalvars_t.trace_flags
#define FTRACE_SIMPLEBOX (1 << 0) // Traceline with a simple box

// walkmove modes
#define WALKMOVE_NORMAL    0 // normal walkmove
#define WALKMOVE_WORLDONLY 1 // doesn't hit ANY entities, no matter what the solid type
#define WALKMOVE_CHECKONLY 2 // move, but don't touch triggers

// edict->movetype values
#define MOVETYPE_NONE 0 // never moves
//#define	MOVETYPE_ANGLENOCLIP	1
//#define	MOVETYPE_ANGLECLIP		2
#define MOVETYPE_WALK          3 // Player only - moving on the ground
#define MOVETYPE_STEP          4 // gravity, special edge handling -- monsters use this
#define MOVETYPE_FLY           5 // No gravity, but still collides with stuff
#define MOVETYPE_TOSS          6 // gravity/collisions
#define MOVETYPE_PUSH          7 // no clip to world, push and crush
#define MOVETYPE_NOCLIP        8 // No gravity, no collisions, still do velocity/avelocity
#define MOVETYPE_FLYMISSILE    9 // extra size to monsters
#define MOVETYPE_BOUNCE        10 // Just like Toss, but reflect velocity when contacting surfaces
#define MOVETYPE_BOUNCEMISSILE 11 // bounce w/o gravity
#define MOVETYPE_FOLLOW        12 // track movement of aiment
#define MOVETYPE_PUSHSTEP      13 // BSP model that needs physics/world collisions (uses nearest hull for world collision)

// Water level
#define WL_NOT_IN_WATER 0
#define WL_FEET 1
#define WL_WAIST 2
#define WL_EYES 3

// Entity types
#define ET_NORMAL 0
#define ET_PLAYER 1
#define ET_TEMPENTITY 2
#define ET_BEAM 3
// BMODEL or SPRITE that was split across BSP nodes
#define ET_FRAGMENTED 4

// edict->solid values
// NOTE: Some movetypes will cause collisions independent of SOLID_NOT/SOLID_TRIGGER when the entity moves
// SOLID only effects OTHER entities colliding with this one when they move - UGH!
#define SOLID_NOT      0 // no interaction with other objects
#define SOLID_TRIGGER  1 // touch on edge, but not blocking
#define SOLID_BBOX     2 // touch on edge, block
#define SOLID_SLIDEBOX 3 // touch on edge, but not an onground
#define SOLID_BSP      4 // bsp clip, touch on edge, block

// edict->deadflag values
#define DEAD_NO          0 // alive
#define DEAD_DYING       1 // playing death animation or still falling off of a ledge waiting to hit ground
#define DEAD_DEAD        2 // dead. lying still.
#define DEAD_RESPAWNABLE 3
#define DEAD_DISCARDBODY 4

#define DAMAGE_NO  0
#define DAMAGE_YES 1
#define DAMAGE_AIM 2

// entity effects
#define EF_BRIGHTFIELD 1 // swirling cloud of particles
#define EF_MUZZLEFLASH 2 // single frame ELIGHT on entity attachment 0
#define EF_BRIGHTLIGHT 4 // DLIGHT centered at entity origin
#define EF_DIMLIGHT    8 // player flashlight
#define EF_INVLIGHT    16 // get lighting from ceiling
#define EF_NOINTERP    32 // don't interpolate the next frame
#define EF_LIGHT       64 // rocket flare glow sprite
#define EF_NODRAW      128 // don't draw entity
#define EF_NIGHTVISION 256 // player nightvision
#define EF_SNIPERLASER 512 // sniper laser effect
#define EF_FIBERCAMERA 1024 // fiber camera

// Must be wrong? effects in original half life
//#define EF_NIGHTVISION 256	// player nightvision
//#define EF_SNIPERLASER 512	// sniper laser effect
//#define EF_FIBERCAMERA 1024	// fiber camera

// Must be correct? effects in sven cope
#define EF_NOANIMTEXTURES 256		// Never animate a texture on given brush.
#define EF_FRAMEANIMTEXTURES 512	// If the brush has a "frame" value, the corresponding animating texture index is used.
#define EF_SPRITE_CUSTOM_VP 1024	// If the entity is a sprite it should override the viewport draw mode with the type specified in key "sequence".


// entity flags
#define EFLAG_SLERP 1 // do studio interpolation of this entity

//
// temp entity events
//
#define TE_BEAMPOINTS 0 // beam effect between two points
// coord coord coord (start position)
// coord coord coord (end position)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMENTPOINT 1 // beam effect between point and entity
// short (start entity)
// coord coord coord (end position)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_GUNSHOT 2 // particle effect plus ricochet sound
// coord coord coord (position)

#define TE_EXPLOSION 3 // additive sprite, 2 dynamic lights, flickering particles, explosion sound, move vertically 8 pps
// coord coord coord (position)
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)
// byte (flags)
//
// The Explosion effect has some flags to control performance/aesthetic features:
#define TE_EXPLFLAG_NONE        0 // all flags clear makes default Half-Life explosion
#define TE_EXPLFLAG_NOADDITIVE  1 // sprite will be drawn opaque (ensure that the sprite you send is a non-additive sprite)
#define TE_EXPLFLAG_NODLIGHTS   2 // do not render dynamic lights
#define TE_EXPLFLAG_NOSOUND     4 // do not play client explosion sound
#define TE_EXPLFLAG_NOPARTICLES 8 // do not draw particles

#define TE_TAREXPLOSION 4 // Quake1 "tarbaby" explosion with sound
// coord coord coord (position)

#define TE_SMOKE 5 // alphablend sprite, move vertically 30 pps
// coord coord coord (position)
// short (sprite index)
// byte (scale in 0.1's)
// byte (framerate)

#define TE_TRACER 6 // tracer effect from point to point
// coord, coord, coord (start)
// coord, coord, coord (end)

#define TE_LIGHTNING 7 // TE_BEAMPOINTS with simplified parameters
// coord, coord, coord (start)
// coord, coord, coord (end)
// byte (life in 0.1's)
// byte (width in 0.1's)
// byte (amplitude in 0.01's)
// short (sprite model index)

#define TE_BEAMENTS 8
// short (start entity)
// short (end entity)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_SPARKS 9 // 8 random tracers with gravity, ricochet sprite
// coord coord coord (position)

#define TE_LAVASPLASH 10 // Quake1 lava splash
// coord coord coord (position)

#define TE_TELEPORT 11 // Quake1 teleport splash
// coord coord coord (position)

#define TE_EXPLOSION2 12 // Quake1 colormaped (base palette) particle explosion with sound
// coord coord coord (position)
// byte (starting color)
// byte (num colors)

#define TE_BSPDECAL 13 // Decal from the .BSP file
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// short (texture index of precached decal texture name)
// short (entity index)
// [optional - only included if previous short is non-zero (not the world)] short (index of model of above entity)

#define TE_IMPLOSION 14 // tracers moving toward a point
// coord, coord, coord (position)
// byte (radius)
// byte (count)
// byte (life in 0.1's)

#define TE_SPRITETRAIL 15 // line of moving glow sprites with gravity, fadeout, and collisions
// coord, coord, coord (start)
// coord, coord, coord (end)
// short (sprite index)
// byte (count)
// byte (life in 0.1's)
// byte (scale in 0.1's)
// byte (velocity along vector in 10's)
// byte (randomness of velocity in 10's)

#define TE_BEAM 16 // obsolete

#define TE_SPRITE 17 // additive sprite, plays 1 cycle
// coord, coord, coord (position)
// short (sprite index)
// byte (scale in 0.1's)
// byte (brightness)

#define TE_BEAMSPRITE 18 // A beam with a sprite at the end
// coord, coord, coord (start position)
// coord, coord, coord (end position)
// short (beam sprite index)
// short (end sprite index)

#define TE_BEAMTORUS 19 // screen aligned beam ring, expands to max radius over lifetime
// coord coord coord (center position)
// coord coord coord (axis and radius)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMDISK 20 // disk that expands to max radius over lifetime
// coord coord coord (center position)
// coord coord coord (axis and radius)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMCYLINDER 21 // cylinder that expands to max radius over lifetime
// coord coord coord (center position)
// coord coord coord (axis and radius)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_BEAMFOLLOW 22 // create a line of decaying beam segments until entity stops moving
// short (entity:attachment to follow)
// short (sprite index)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte,byte,byte (color)
// byte (brightness)

#define TE_GLOWSPRITE 23
// coord, coord, coord (pos) short (model index) byte (scale / 10)

#define TE_BEAMRING 24 // connect a beam ring to two entities
// short (start entity)
// short (end entity)
// short (sprite index)
// byte (starting frame)
// byte (frame rate in 0.1's)
// byte (life in 0.1's)
// byte (line width in 0.1's)
// byte (noise amplitude in 0.01's)
// byte,byte,byte (color)
// byte (brightness)
// byte (scroll speed in 0.1's)

#define TE_STREAK_SPLASH 25 // oriented shower of tracers
// coord coord coord (start position)
// coord coord coord (direction vector)
// byte (color)
// short (count)
// short (base speed)
// short (ramdon velocity)

#define TE_BEAMHOSE 26 // obsolete

#define TE_DLIGHT 27 // dynamic light, effect world, minor entity effect
// coord, coord, coord (pos)
// byte (radius in 10's)
// byte byte byte (color)
// byte (brightness)
// byte (life in 10's)
// byte (decay rate in 10's)

#define TE_ELIGHT 28 // point entity light, no world effect
// short (entity:attachment to follow)
// coord coord coord (initial position)
// coord (radius)
// byte byte byte (color)
// byte (life in 0.1's)
// coord (decay rate)

#define TE_TEXTMESSAGE 29
// short 1.2.13 x (-1 = center)
// short 1.2.13 y (-1 = center)
// byte Effect 0 = fade in/fade out
// 1 is flickery credits
// 2 is write out (training room)

// 4 bytes r,g,b,a color1	(text color)
// 4 bytes r,g,b,a color2	(effect color)
// ushort 8.8 fadein time
// ushort 8.8  fadeout time
// ushort 8.8 hold time
// optional ushort 8.8 fxtime	(time the highlight lags behing the leading text in effect 2)
// string text message		(512 chars max sz string)
#define TE_LINE 30
// coord, coord, coord		startpos
// coord, coord, coord		endpos
// short life in 0.1 s
// 3 bytes r, g, b

#define TE_BOX 31
// coord, coord, coord		boxmins
// coord, coord, coord		boxmaxs
// short life in 0.1 s
// 3 bytes r, g, b

#define TE_KILLBEAM 99 // kill all beams attached to entity
// short (entity)

#define TE_LARGEFUNNEL 100
// coord coord coord (funnel position)
// short (sprite index)
// short (flags)

#define TE_BLOODSTREAM 101 // particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

#define TE_SHOWLINE 102 // line of particles every 5 units, dies in 30 seconds
// coord coord coord (start position)
// coord coord coord (end position)

#define TE_BLOOD 103 // particle spray
// coord coord coord (start position)
// coord coord coord (spray vector)
// byte (color)
// byte (speed)

#define TE_DECAL 104 // Decal applied to a brush entity (not the world)
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)
// short (entity index)

#define TE_FIZZ 105 // create alpha sprites inside of entity, float upwards
// short (entity)
// short (sprite index)
// byte (density)

#define TE_MODEL 106 // create a moving model that bounces and makes a sound when it hits
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// angle (initial yaw)
// short (model index)
// byte (bounce sound type)
// byte (life in 0.1's)

#define TE_EXPLODEMODEL 107 // spherical shower of models, picks from set
// coord, coord, coord (origin)
// coord (velocity)
// short (model index)
// short (count)
// byte (life in 0.1's)

#define TE_BREAKMODEL 108 // box of models or sprites
// coord, coord, coord (position)
// coord, coord, coord (size)
// coord, coord, coord (velocity)
// byte (random velocity in 10's)
// short (sprite or model index)
// byte (count)
// byte (life in 0.1 secs)
// byte (flags)

#define TE_GUNSHOTDECAL 109 // decal and ricochet sound
// coord, coord, coord (position)
// short (entity index???)
// byte (decal???)

#define TE_SPRITE_SPRAY 110 // spay of alpha sprites
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (sprite index)
// byte (count)
// byte (speed)
// byte (noise)

#define TE_ARMOR_RICOCHET 111 // quick spark sprite, client ricochet sound.
// coord, coord, coord (position)
// byte (scale in 0.1's)

#define TE_PLAYERDECAL 112 // ???
// byte (playerindex)
// coord, coord, coord (position)
// short (entity???)
// byte (decal number???)
// [optional] short (model index???)

#define TE_BUBBLES 113 // create alpha sprites inside of box, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

#define TE_BUBBLETRAIL 114 // create alpha sprites along a line, float upwards
// coord, coord, coord (min start position)
// coord, coord, coord (max start position)
// coord (float height)
// short (model index)
// byte (count)
// coord (speed)

#define TE_BLOODSPRITE 115 // spray of opaque sprite1's that fall, single sprite2 for 1..2 secs (this is a high-priority tent)
// coord, coord, coord (position)
// short (sprite1 index)
// short (sprite2 index)
// byte (color)
// byte (scale)

#define TE_WORLDDECAL 116 // Decal applied to the world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name)

#define TE_WORLDDECALHIGH 117 // Decal (with texture index > 256) applied to world brush
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)

#define TE_DECALHIGH 118 // Same as TE_DECAL, but the texture index was greater than 256
// coord, coord, coord (x,y,z), decal position (center of texture in world)
// byte (texture index of precached decal texture name - 256)
// short (entity index)

#define TE_PROJECTILE 119 // Makes a projectile (like a nail) (this is a high-priority tent)
// coord, coord, coord (position)
// coord, coord, coord (velocity)
// short (modelindex)
// byte (life)
// byte (owner)  projectile won't collide with owner (if owner == 0, projectile will hit any client).

#define TE_SPRAY 120 // Throws a shower of sprites or models
// coord, coord, coord (position)
// coord, coord, coord (direction)
// short (modelindex)
// byte (count)
// byte (speed)
// byte (noise)
// byte (rendermode)

#define TE_PLAYERSPRITES 121 // sprites emit from a player's bounding box (ONLY use for players!)
// byte (playernum)
// short (sprite modelindex)
// byte (count)
// byte (variance) (0 = no variance in size) (10 = 10% variance in size)

#define TE_PARTICLEBURST 122 // very similar to lavasplash.
// coord (origin)
// short (radius)
// byte (particle color)
// byte (duration * 10) (will be randomized a bit)

#define TE_FIREFIELD 123 // makes a field of fire.
// coord (origin)
// short (radius) (fire is made in a square around origin. -radius, -radius to radius, radius)
// short (modelindex)
// byte (count)
// byte (flags)
// byte (duration (in seconds) * 10) (will be randomized a bit)
//
// to keep network traffic low, this message has associated flags that fit into a byte:
#define TEFIRE_FLAG_ALLFLOAT  1 // all sprites will drift upwards as they animate
#define TEFIRE_FLAG_SOMEFLOAT 2 // some of the sprites will drift upwards. (50% chance)
#define TEFIRE_FLAG_LOOP      4 // if set, sprite plays at 15 fps, otherwise plays at whatever rate stretches the animation over the sprite's duration.
#define TEFIRE_FLAG_ALPHA     8 // if set, sprite is rendered alpha blended at 50% else, opaque
#define TEFIRE_FLAG_PLANAR    16 // if set, all fire sprites have same initial Z instead of randomly filling a cube.
#define TEFIRE_FLAG_ADDITIVE  32 // if set, sprite is rendered non-opaque with additive

#define TE_PLAYERATTACHMENT 124 // attaches a TENT to a player (this is a high-priority tent)
// byte (entity index of player)
// coord (vertical offset) ( attachment origin.z = player origin.z + vertical offset )
// short (model index)
// short (life * 10 );

#define TE_KILLPLAYERATTACHMENTS 125 // will expire all TENTS attached to a player.
// byte (entity index of player)

#define TE_MULTIGUNSHOT 126 // much more compact shotgun message
// This message is used to make a client approximate a 'spray' of gunfire.
// Any weapon that fires more than one bullet per frame and fires in a bit of a spread is
// a good candidate for MULTIGUNSHOT use. (shotguns)
//
// NOTE: This effect makes the client do traces for each bullet, these client traces ignore
//		 entities that have studio models.Traces are 4096 long.
//
// coord (origin)
// coord (origin)
// coord (origin)
// coord (direction)
// coord (direction)
// coord (direction)
// coord (x noise * 100)
// coord (y noise * 100)
// byte (count)
// byte (bullethole decal texture index)

#define TE_USERTRACER 127 // larger message than the standard tracer, but allows some customization.
// coord (origin)
// coord (origin)
// coord (origin)
// coord (velocity)
// coord (velocity)
// coord (velocity)
// byte ( life * 10 )
// byte ( color ) this is an index into an array of color vectors in the engine. (0 - )
// byte ( length * 10 )

#define MSG_BROADCAST      0 // unreliable to all
#define MSG_ONE            1 // reliable to one (msg_entity)
#define MSG_ALL            2 // reliable to all
#define MSG_INIT           3 // write to the init string
#define MSG_PVS            4 // Ents in PVS of org
#define MSG_PAS            5 // Ents in PAS of org
#define MSG_PVS_R          6 // Reliable to PVS
#define MSG_PAS_R          7 // Reliable to PAS
#define MSG_ONE_UNRELIABLE 8 // Send to one client, but don't put in reliable stream, put in unreliable datagram ( could be dropped )
#define MSG_SPEC           9 // Sends to all spectator proxies

// contents of a spot in the world
#define CONTENTS_EMPTY -1
#define CONTENTS_SOLID -2
#define CONTENTS_WATER -3
#define CONTENTS_SLIME -4
#define CONTENTS_LAVA  -5
#define CONTENTS_SKY   -6
// These additional contents constants are defined in bspfile.h
#define	CONTENTS_ORIGIN		-7		// removed at csg time
#define	CONTENTS_CLIP		-8		// changed to contents_solid
#define	CONTENTS_CURRENT_0		-9
#define	CONTENTS_CURRENT_90		-10
#define	CONTENTS_CURRENT_180	-11
#define	CONTENTS_CURRENT_270	-12
#define	CONTENTS_CURRENT_UP		-13
#define	CONTENTS_CURRENT_DOWN	-14
#define CONTENTS_TRANSLUCENT	-15

#define CONTENTS_LADDER -16

#define CONTENT_FLYFIELD         -17
#define CONTENT_GRAVITY_FLYFIELD -18
#define CONTENT_FOG              -19

#define CONTENT_EMPTY -1
#define CONTENT_SOLID -2
#define CONTENT_WATER -3
#define CONTENT_SLIME -4
#define CONTENT_LAVA  -5
#define CONTENT_SKY   -6

// channels
#define CHAN_AUTO              0
#define CHAN_WEAPON            1
#define CHAN_VOICE             2
#define CHAN_ITEM              3
#define CHAN_BODY              4
#define CHAN_STREAM            5 // allocate stream channel from the static or dynamic area
#define CHAN_STATIC            6 // allocate channel from the static area
#define CHAN_NETWORKVOICE_BASE 7 // voice data coming across the network
#define CHAN_NETWORKVOICE_END  500 // network voice data reserves slots (CHAN_NETWORKVOICE_BASE through CHAN_NETWORKVOICE_END).
#define CHAN_BOT               501 // channel used for bot chatter.

// attenuation values
#define ATTN_NONE   0
#define ATTN_NORM   (float)0.8
#define ATTN_IDLE   (float)2
#define ATTN_STATIC (float)1.25

// pitch values
#define PITCH_NORM 100 // non-pitch shifted
#define PITCH_LOW  95 // other values are possible - 0-255, where 255 is very high
#define PITCH_HIGH 120

// volume values
#define VOL_NORM 1.0

// plats
#define PLAT_LOW_TRIGGER 1

// Trains
#define SF_TRAIN_WAIT_RETRIGGER 1
#define SF_TRAIN_START_ON       4 // Train is initially moving
#define SF_TRAIN_PASSABLE       8 // Train is not solid -- used to make water trains

// Break Model Defines

#define BREAK_TYPEMASK 0x4F
#define BREAK_GLASS    0x01
#define BREAK_METAL    0x02
#define BREAK_FLESH    0x04
#define BREAK_WOOD     0x08

#define BREAK_SMOKE    0x10
#define BREAK_TRANS    0x20
#define BREAK_CONCRETE 0x40
#define BREAK_2        0x80

// Colliding temp entity sounds

#define BOUNCE_GLASS     BREAK_GLASS
#define BOUNCE_METAL     BREAK_METAL
#define BOUNCE_FLESH     BREAK_FLESH
#define BOUNCE_WOOD      BREAK_WOOD
#define BOUNCE_SHRAP     0x10
#define BOUNCE_SHELL     0x20
#define BOUNCE_CONCRETE  BREAK_CONCRETE
#define BOUNCE_SHOTSHELL 0x80

// Temp entity bounce sound types
#define TE_BOUNCE_NULL      0
#define TE_BOUNCE_SHELL     1
#define TE_BOUNCE_SHOTSHELL 2

// Rendering constants
enum
{
	kRenderNormal, // src
	kRenderTransColor, // c*a+dest*(1-a)
	kRenderTransTexture, // src*a+dest*(1-a)
	kRenderGlow, // src*a+dest -- No Z buffer checks
	kRenderTransAlpha, // src*srca+dest*(1-srca)
	kRenderTransAdd, // src*a+dest
};

enum
{
	kRenderFxNone = 0,
	kRenderFxPulseSlow,
	kRenderFxPulseFast,
	kRenderFxPulseSlowWide,
	kRenderFxPulseFastWide,
	kRenderFxFadeSlow,
	kRenderFxFadeFast,
	kRenderFxSolidSlow,
	kRenderFxSolidFast,
	kRenderFxStrobeSlow,
	kRenderFxStrobeFast,
	kRenderFxStrobeFaster,
	kRenderFxFlickerSlow,
	kRenderFxFlickerFast,
	kRenderFxNoDissipation,
	kRenderFxDistort, // Distort/scale/translate flicker
	kRenderFxHologram, // kRenderFxDistort + distance fade
	kRenderFxDeadPlayer, // kRenderAmt is the player index
	kRenderFxExplode, // Scale up really big!
	kRenderFxGlowShell, // Glowing Shell
	kRenderFxClampMinScale, // Keep this sprite from getting very small (SPRITES only!)
	kRenderFxLightMultiplier, //CTM !!!CZERO added to tell the studiorender that the value in iuser2 is a lightmultiplier
};

//
// these are the key numbers that should be passed to Key_Event
//
#define	K_TAB			9
#define	K_ENTER			13
#define	K_ESCAPE		27
#define	K_SPACE			32

// normal keys should be passed as lowercased ascii

#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131

#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

#define K_KP_HOME		160
#define K_KP_UPARROW	161
#define K_KP_PGUP		162
#define	K_KP_LEFTARROW	163
#define K_KP_5			164
#define K_KP_RIGHTARROW	165
#define K_KP_END		166
#define K_KP_DOWNARROW	167
#define K_KP_PGDN		168
#define	K_KP_ENTER		169
#define K_KP_INS   		170
#define	K_KP_DEL		171
#define K_KP_SLASH		172
#define K_KP_MINUS		173
#define K_KP_PLUS		174
#define K_CAPSLOCK		175
#define K_KP_MUL		176
#define K_WIN			177


//
// joystick buttons
//
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

//
// aux keys are for multi-buttoned joysticks to generate so they can use
// the normal binding process
//
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238
#define K_MWHEELDOWN	239
#define K_MWHEELUP		240

#define K_PAUSE			255

//
// mouse buttons generate virtual keys
//
#define	K_MOUSE1		241
#define	K_MOUSE2		242
#define	K_MOUSE3		243
#define K_MOUSE4		244
#define K_MOUSE5		245

typedef unsigned char byte;
typedef unsigned short word;
#define _DEF_BYTE_

#define PROTOCOL_VERSION		48

// Server-to-client network messages
#define SVC_BAD					0 // immediately crash client when received
#define SVC_NOP					1 // does nothing
#define SVC_DISCONNECT			2 // disconnects a player with given reason
#define SVC_EVENT				3 // an event, defined by the game library, has recently occurred on the server
#define SVC_VERSION				4 // disconnects the client and sends a message to the console if client's protocol doesn't match the server's protocol version
#define SVC_SETVIEW				5 // attaches a player's view to an entity
#define SVC_SOUND				6 // plays a sound file on the client
#define SVC_TIME				7 // notifies clients about the current server time
#define SVC_PRINT				8 // sends a message to the client's console
#define SVC_STUFFTEXT			9 // executes command on player
#define SVC_SETANGLE			10 // update immediately the client's view angles
#define SVC_SERVERINFO			11 // contains information about the server
#define SVC_LIGHTSTYLE			12 // setup light animation tables. 'a' is total darkness, 'z' is maxbright
#define SVC_UPDATEUSERINFO		13 // contains information about a particular client
#define SVC_DELTADESCRIPTION	14 // synchronizes client delta descriptions with server ones
#define SVC_CLIENTDATA			15 // contains information about the client state at the time of last server frame
#define SVC_STOPSOUND			16 // stops an ambient sound
#define SVC_PINGS				17 // contains ping and loss values for a number of players
#define SVC_PARTICLE			18 // shows a particle effect
#define SVC_WEAPONANIMEX		19
#define SVC_SPAWNSTATIC			20 // marks an entity as "static", so that it can be freed from server memory
#define SVC_EVENT_RELIABLE		21 // similar to SVC_EVENT, but no queuing takes place, and the message can only hold one event.
#define SVC_SPAWNBASELINE		22 // creates a baseline for future referencing
#define SVC_TEMPENTITY			23 // creates a temp entity.
#define SVC_SETPAUSE			24 // puts client to a pause
#define SVC_SIGNONNUM			25 // called just after client_putinserver. Signals the client that the server has marked it as "active"
#define SVC_CENTERPRINT			26 // sends a centered message
#define SVC_KILLEDMONSTER		27
#define SVC_EVENTLIST			28
#define SVC_SPAWNSTATICSOUND	29 // start playback of a sound, loaded into the static portion of the channel array
#define SVC_INTERMISSION		30 // shows the intermission camera view
#define SVC_FINALE				31 // shows the intermission camera view, and writes-out text passed in first parameter
#define SVC_CDTRACK				32 // plays a Half-Life music
#define SVC_RESTORE				33 // maintains a global transition table for the saved game
#define SVC_CUTSCENE			34 // shows the intermission camera view, and writes-out text passed in first parameter
#define SVC_WEAPONANIM			35 // plays a weapon sequence
#define SVC_DECALNAME			36 // allows to set, into the client's decals array and at specific position index (0->511), a decal name
#define SVC_ROOMTYPE			37 // sets client room_type cvar to provided value
#define SVC_ADDANGLE			38 // add an angle on the yaw axis of the current client's view angle
#define SVC_NEWUSERMSG			39 // registers a new user message on the client
#define SVC_PACKETENTITIES		40 // contains information about the entity states, like origin, angles and such
#define SVC_DELTAPACKETENTITIES	41 // contains information about the entity states, like origin, angles and such
#define SVC_CHOKE				42 // notify the client that some outgoing datagrams were not transmitted due to exceeding bandwidth rate limits
#define SVC_RESOURCELIST		43 // contains all the resources provided by the server for clients to download. Consistency info can also be included
#define SVC_NEWMOVEVARS			44 // updates client's movevars
#define SVC_RESOURCEREQUEST		45 // allows the client to send its own resource list (CLC_RESOURCELIST)
#define SVC_CUSTOMIZATION		46 // notifies the client that a new customization is avaliable for download
#define SVC_CROSSHAIRANGLE		47 // adjusts the weapon's crosshair angle
#define SVC_SOUNDFADE			48 // updates client side sound fade
#define SVC_FILETXFERFAILED		49 // sends a message to the client's console telling what file has failed to be transfered
#define SVC_HLTV				50 // tells client about current spectator mode
#define SVC_DIRECTOR			51
#define SVC_VOICEINIT			52 // sends sv_voicecodec and sv_voicequality cvars to client
#define SVC_VOICEDATA			53 // contains compressed voice data
#define SVC_SENDEXTRAINFO		54 // sends some extra information regarding the server
#define SVC_TIMESCALE			55
#define SVC_RESOURCELOCATION	56 // sends sv_downloadurl to client
#define SVC_SENDCVARVALUE		57 // request a cvar value from a connected client
#define SVC_SENDCVARVALUE2		58 // request a cvar value from a connected client
#define SVC_LASTMSG				58 // start user messages at this point

// Client-to-server network messages
#define CLC_BAD					0
#define CLC_NOP					1
#define CLC_MOVE				2
#define CLC_STRINGCMD			3
#define CLC_DELTA				4
#define CLC_RESOURCELIST		5
#define CLC_TMOVE				6
#define CLC_FILECONSISTENCY		7
#define CLC_VOICEDATA			8 // CL_AddVoiceToDatagram
#define CLC_HLTV				9
#define CLC_REQUESTCVARVALUE	10 // CL_Send_CvarValue
#define CLC_REQUESTCVARVALUE2	11 // CL_Send_CvarValue2

// Client move cmd bits
#define	CM_ANGLE1				( 1 << 0 )
#define	CM_ANGLE3				( 1 << 1 )
#define	CM_FORWARD				( 1 << 2 )
#define	CM_SIDE					( 1 << 3 )
#define	CM_UP					( 1 << 4 )
#define	CM_BUTTONS				( 1 << 5 )
#define	CM_IMPULSE				( 1 << 6 )
#define	CM_ANGLE2				( 1 << 7 )

const Vector VEC_HULL_MIN( -16, -16, -36 );
const Vector VEC_HULL_MAX( 16, 16, 36 );
const Vector VEC_HUMAN_HULL_MIN( -16, -16, 0 );
const Vector VEC_HUMAN_HULL_MAX( 16, 16, 72 );
const Vector VEC_HUMAN_HULL_DUCK( 16, 16, 36 );

const Vector VEC_VIEW( 0, 0, 28 );

const Vector VEC_DUCK_HULL_MIN( -16, -16, -18 );
const Vector VEC_DUCK_HULL_MAX( 16, 16, 18 );
const Vector VEC_DUCK_VIEW( 0, 0, 12 );

const Vector VEC_DEAD_VIEW( 0, 0, -8 );

#ifdef _WIN32
typedef __int16 int16;
typedef unsigned __int16 uint16;
typedef __int32 int32;
typedef unsigned __int32 uint32;
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
#endif

typedef struct kbutton_s
{
	int down[ 2 ]; // key nums holding it down
	int state;	 // low bit is down state
} kbutton_t;

typedef struct
{
	Vector normal;
	float dist;
} plane_t;

typedef struct usercmd_s
{
	short	lerp_msec;      // Interpolation time on client
	byte	msec;           // Duration in ms of command
	vec3_t	viewangles;     // Command view angles.

	// intended velocities
	float	forwardmove;    // Forward velocity.
	float	sidemove;       // Sideways velocity.
	float	upmove;         // Upward velocity.
	byte	lightlevel;     // Light level at spot where we are standing.
	unsigned short  buttons;  // Attack buttons
	byte    impulse;          // Impulse command issued.
	byte	weaponselect;	// Current weapon id

	// Experimental player impact stuff.
	int		impact_index;
	vec3_t	impact_position;
} usercmd_t;

// physent_t
typedef struct physent_s
{
	char			name[ 32 ];             // Name of model, or "player" or "world".
	int				player;
	vec3_t			origin;               // Model's origin in world coordinates.
	struct model_s *model;		          // only for bsp models
	struct model_s *studiomodel;         // SOLID_BBOX, but studio clip intersections.
	vec3_t			mins, maxs;	          // only for non-bsp models
	int				info;		          // For client or server to use to identify (index into edicts or cl_entities)
	vec3_t			angles;               // rotated entities need this info for hull testing to work.

	int				solid;				  // Triggers and func_door type WATER brushes are SOLID_NOT
	int				skin;                 // BSP Contents for such things like fun_door water brushes.
	int				rendermode;			  // So we can ignore glass

	// Complex collision detection.
	float			frame;
	int				sequence;
	byte			controller[ 4 ];
	byte			blending[ 2 ];

	int				movetype;
	int				takedamage;
	int				blooddecal;
	int				team;
	int				classnumber;

	// For mods
	int				iuser1;
	int				iuser2;
	int				iuser3;
	int				iuser4;
	float			fuser1;
	float			fuser2;
	float			fuser3;
	float			fuser4;
	vec3_t			vuser1;
	vec3_t			vuser2;
	vec3_t			vuser3;
	vec3_t			vuser4;
} physent_t;

#define	MAX_PHYSENTS 600 		  // Must have room for all entities in the world.
#define	MAX_PHYSENTS_5_26 1024		// SC 5.26
#define MAX_MOVEENTS 64
#define	MAX_CLIP_PLANES	5

#define PM_NORMAL			0x00000000
#define PM_STUDIO_IGNORE	0x00000001		// Skip studio models
#define PM_STUDIO_BOX		0x00000002		// Use boxes for non-complex studio models (even in traceline)
#define PM_GLASS_IGNORE		0x00000004		// Ignore entities with non-normal rendermode
#define PM_WORLD_ONLY		0x00000008		// Only trace against the world

// Values for flags parameter of PM_TraceLine
#define PM_TRACELINE_ANYVISIBLE		0
#define PM_TRACELINE_PHYSENTSONLY	1

// Values for type of trace
#define PM_HULL_PLAYER 0
#define PM_HULL_DUCKED_PLAYER 1
#define PM_HULL_POINT 2

// Materials
#define CBTEXTURENAMEMAX 13 // only load first n chars of name

#define CHAR_TEX_CONCRETE 'C' // texture types
#define CHAR_TEX_METAL 'M'
#define CHAR_TEX_DIRT 'D'
#define CHAR_TEX_VENT 'V'
#define CHAR_TEX_GRATE 'G'
#define CHAR_TEX_TILE 'T'
#define CHAR_TEX_SLOSH 'S'
#define CHAR_TEX_WOOD 'W'
#define CHAR_TEX_COMPUTER 'P'
#define CHAR_TEX_GLASS 'Y'
#define CHAR_TEX_FLESH 'F'
#define CHAR_TEX_SNOW 'O' // it was 'N'. @Sw1ft: did I change it?

// Spectator Movement modes (stored in pev->iuser1, so the physics code can get at them)
#define OBS_NONE 0
#define OBS_CHASE_LOCKED 1
#define OBS_CHASE_FREE 2
#define OBS_ROAMING 3
#define OBS_IN_EYE 4
#define OBS_MAP_FREE 5
#define OBS_MAP_CHASE 6

typedef struct
{
	vec3_t	normal;
	float	dist;
} pmplane_t;

#define IN_ATTACK	(1 << 0)
#define IN_JUMP		(1 << 1)
#define IN_DUCK		(1 << 2)
#define IN_FORWARD	(1 << 3)
#define IN_BACK		(1 << 4)
#define IN_USE		(1 << 5)
#define IN_CANCEL	(1 << 6)
#define IN_LEFT		(1 << 7)
#define IN_RIGHT	(1 << 8)
#define IN_MOVELEFT	(1 << 9)
#define IN_MOVERIGHT (1 << 10)
#define IN_ATTACK2	(1 << 11)
#define IN_RUN      (1 << 12)
#define IN_RELOAD	(1 << 13)
#define IN_ALT1		(1 << 14)
#define IN_SCORE	(1 << 15)   // Used by client.dll for when scoreboard is held down

typedef struct pmtrace_s pmtrace_t;

struct pmtrace_s
{
	qboolean	allsolid;	      // if true, plane is not valid
	qboolean	startsolid;	      // if true, the initial point was in a solid area
	qboolean	inopen, inwater;  // End point is in empty space or in water
	float		fraction;		  // time completed, 1.0 = didn't hit anything
	vec3_t		endpos;			  // final position
	pmplane_t	plane;		      // surface normal at impact
	int			ent;			  // entity at impact
	vec3_t      deltavelocity;    // Change in player's velocity caused by impact.  
	// Only run on server.
	int         hitgroup;
};

typedef struct playermove_s playermove_t;
typedef struct playermove_5_26_s playermove_5_26_t;

typedef struct playermove_funcs_s
{
	const char *( *PM_Info_ValueForKey ) ( const char *s, const char *key );
	void			( *PM_Particle )( float *origin, int color, float life, int zpos, int zvel );
	int				( *PM_TestPlayerPosition ) ( float *pos, pmtrace_t *ptrace );
	void			( *Con_NPrintf )( int idx, char *fmt, ... );
	void			( *Con_DPrintf )( char *fmt, ... );
	void			( *Con_Printf )( char *fmt, ... );
	double			( *Sys_FloatTime )( void );
	void			( *PM_StuckTouch )( int hitent, pmtrace_t *ptraceresult );
	int				( *PM_PointContents ) ( float *p, int *truecontents );
	int				( *PM_TruePointContents ) ( float *p );
	int				( *PM_HullPointContents ) ( struct hull_s *hull, int num, float *p );
	pmtrace_t( *PM_PlayerTrace ) ( float *start, float *end, int traceFlags, int ignore_pe );
	struct pmtrace_s *( *PM_TraceLine )( float *start, float *end, int flags, int usehulll, int ignore_pe );
	int32( *RandomLong )( int32 lLow, int32 lHigh );
	float			( *RandomFloat )( float flLow, float flHigh );
	int				( *PM_GetModelType )( struct model_s *mod );
	void			( *PM_GetModelBounds )( struct model_s *mod, float *mins, float *maxs );
	void *( *PM_HullForBsp )( physent_t *pe, float *offset );
	float			( *PM_TraceModel )( physent_t *pEnt, float *start, float *end, pmtrace_t *trace );
	int				( *COM_FileSize )( char *filename );
	byte *( *COM_LoadFile ) ( char *path, int usehunk, int *pLength );
	void			( *COM_FreeFile ) ( void *buffer );
	char *( *memfgets )( byte *pMemFile, int fileSize, int *pFilePos, char *pBuffer, int bufferSize );
	qboolean		runfuncs;
	void			( *PM_PlaySound ) ( int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );
	const char *( *PM_TraceTexture ) ( int ground, float *vstart, float *vend );
	void			( *PM_PlaybackEventFull ) ( int flags, int clientindex, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

	pmtrace_t( *PM_PlayerTraceEx )( float *start, float *end, int traceFlags, int ( *pfnIgnore )( physent_t *pe ) );
	int ( *PM_TestPlayerPositionEx )( float *pos, pmtrace_t *ptrace, int ( *pfnIgnore )( physent_t *pe ) );
	struct pmtrace_s *( *PM_TraceLineEx )( float *start, float *end, int flags, int usehulll, int ( *pfnIgnore )( physent_t *pe ) );

	// @Sw1ft: Sven Co-op specific
	void ( *Con_DnPrintf )( int pos, const char *pszMessage, ... );
} playermove_funcs_t;

struct playermove_s
{
	int				player_index;  // So we don't try to run the PM_CheckStuck nudging too quickly.
	qboolean		server;        // For debugging, are we running physics code on server side?

	qboolean		multiplayer;   // 1 == multiplayer server
	float			time;          // realtime on host, for reckoning duck timing
	float			frametime;	   // Duration of this frame

	vec3_t			forward, right, up; // Vectors for angles
	// player state
	vec3_t			origin;        // Movement origin.
	vec3_t			angles;        // Movement view angles.
	vec3_t			oldangles;     // Angles before movement view angles were looked at.
	vec3_t			velocity;      // Current movement direction.
	vec3_t			movedir;       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	vec3_t			basevelocity;  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	vec3_t			view_ofs;      // Our eye position.
	float			flDuckTime;    // Time we started duck
	qboolean		bInDuck;       // In process of ducking or ducked already?

	// For walking/falling
	int				flTimeStepSound;  // Next time we can play a step sound
	int				iStepLeft;

	float			flFallVelocity;
	vec3_t			punchangle;

	float			flSwimTime;

	float			flNextPrimaryAttack;

	int				effects;		// MUZZLE FLASH, e.g.

	int				flags;         // FL_ONGROUND, FL_DUCKING, etc.
	int				usehull;       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	float			gravity;       // Our current gravity and friction.
	float			friction;
	int				oldbuttons;    // Buttons last usercmd
	float			waterjumptime; // Amount of time left in jumping out of water cycle.
	qboolean		dead;          // Are we a dead player?
	int				deadflag;
	int				spectator;     // Should we use spectator physics model?
	int				movetype;      // Our movement type, NOCLIP, WALK, FLY

	int				onground;
	int				waterlevel;
	int				watertype;
	int				oldwaterlevel;

	char			sztexturename[ 256 ];
	char			chtexturetype;

	float			maxspeed;
	float			clientmaxspeed; // Player specific maxspeed

	// For mods
	int				iuser1;
	int				iuser2;
	int				iuser3;
	int				iuser4;
	float			fuser1;
	float			fuser2;
	float			fuser3;
	float			fuser4;
	vec3_t			vuser1;
	vec3_t			vuser2;
	vec3_t			vuser3;
	vec3_t			vuser4;
	// world state
	// Number of entities to clip against.
	int				numphysent;
	physent_t		physents[ MAX_PHYSENTS ];
	// Number of momvement entities (ladders)
	int				nummoveent;
	// just a list of ladders
	physent_t		moveents[ MAX_MOVEENTS ];

	// All things being rendered, for tracing against things you don't actually collide with
	int				numvisent;
	physent_t		visents[ MAX_PHYSENTS ];

	// input to run through physics.
	usercmd_t		cmd;

	// Trace results for objects we collided with.
	int				numtouch;
	pmtrace_t		touchindex[ MAX_PHYSENTS ];

	char			physinfo[ MAX_PHYSINFO_STRING ]; // Physics info string

	struct movevars_s *movevars;
	vec3_t player_mins[ 4 ];
	vec3_t player_maxs[ 4 ];

	playermove_funcs_t funcs;
};

struct playermove_5_26_s
{
	int				player_index;  // So we don't try to run the PM_CheckStuck nudging too quickly.
	qboolean		server;        // For debugging, are we running physics code on server side?

	qboolean		multiplayer;   // 1 == multiplayer server
	float			time;          // realtime on host, for reckoning duck timing
	float			frametime;	   // Duration of this frame

	vec3_t			forward, right, up; // Vectors for angles
	// player state
	vec3_t			origin;        // Movement origin.
	vec3_t			angles;        // Movement view angles.
	vec3_t			oldangles;     // Angles before movement view angles were looked at.
	vec3_t			velocity;      // Current movement direction.
	vec3_t			movedir;       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	vec3_t			basevelocity;  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	vec3_t			view_ofs;      // Our eye position.
	float			flDuckTime;    // Time we started duck
	qboolean		bInDuck;       // In process of ducking or ducked already?

	// For walking/falling
	int				flTimeStepSound;  // Next time we can play a step sound
	int				iStepLeft;

	float			flFallVelocity;
	vec3_t			punchangle;

	float			flSwimTime;

	float			flNextPrimaryAttack;

	int				effects;		// MUZZLE FLASH, e.g.

	int				flags;         // FL_ONGROUND, FL_DUCKING, etc.
	int				usehull;       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	float			gravity;       // Our current gravity and friction.
	float			friction;
	int				oldbuttons;    // Buttons last usercmd
	float			waterjumptime; // Amount of time left in jumping out of water cycle.
	qboolean		dead;          // Are we a dead player?
	int				deadflag;
	int				spectator;     // Should we use spectator physics model?
	int				movetype;      // Our movement type, NOCLIP, WALK, FLY

	int				onground;
	int				waterlevel;
	int				watertype;
	int				oldwaterlevel;

	char			sztexturename[ 256 ];
	char			chtexturetype;

	float			maxspeed;
	float			clientmaxspeed; // Player specific maxspeed

	// For mods
	int				iuser1;
	int				iuser2;
	int				iuser3;
	int				iuser4;
	float			fuser1;
	float			fuser2;
	float			fuser3;
	float			fuser4;
	vec3_t			vuser1;
	vec3_t			vuser2;
	vec3_t			vuser3;
	vec3_t			vuser4;
	// world state
	// Number of entities to clip against.
	int				numphysent;
	physent_t		physents[ MAX_PHYSENTS_5_26 ];
	// Number of momvement entities (ladders)
	int				nummoveent;
	// just a list of ladders
	physent_t		moveents[ MAX_MOVEENTS ];

	// All things being rendered, for tracing against things you don't actually collide with
	int				numvisent;
	physent_t		visents[ MAX_PHYSENTS_5_26 ];

	// input to run through physics.
	usercmd_t		cmd;

	// Trace results for objects we collided with.
	int				numtouch;
	pmtrace_t		touchindex[ MAX_PHYSENTS_5_26 ];

	char			physinfo[ MAX_PHYSINFO_STRING ]; // Physics info string

	struct movevars_s *movevars;
	vec3_t player_mins[ 4 ];
	vec3_t player_maxs[ 4 ];

	playermove_funcs_t funcs;
};

// FUNC
typedef int( *pfnUserMsgHook )	( const char *pszName, int iSize, void *pbuf );

typedef struct usermsg_s
{
	int msgid;
	int size;
	char name[ 16 ];
	struct usermsg_s *next;
	pfnUserMsgHook function;
} usermsg_t;

typedef struct rect_s
{
	int				left, right, top, bottom;
} wrect_t;

typedef struct cvar_s
{
	char *name;
	char *string;
	int        flags;
	float    value;
	struct cvar_s *next;
} cvar_t;

typedef struct SCREENINFO_s
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[ 256 ];
} SCREENINFO;

typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
	float	fov;	// field of view
} client_data_t;

typedef int HSPRITE_HL;	// handle to a graphic

typedef struct client_sprite_s
{
	char szName[ 64 ];
	char szSprite[ 64 ];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct client_textmessage_s
{
	int		effect;
	unsigned char	r1, g1, b1, a1;		// 2 colors for effects
	unsigned char	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pName;
	const char *pMessage;
} client_textmessage_t;

typedef struct hud_player_info_s
{
	char *name;
	short ping;
	unsigned char thisplayer;  // TRUE if this is the calling player

	// stuff that's unused at the moment,  but should be done
	unsigned char spectator;
	unsigned char packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

	unsigned long long m_nSteamID;
} hud_player_info_t;

// movevars_t                  // Physics variables.
typedef struct movevars_s movevars_t;

struct movevars_s
{
	float	gravity;           // Gravity for map
	float	stopspeed;         // Deceleration when not moving
	float	maxspeed;          // Max allowed speed
	float	spectatormaxspeed;
	float	accelerate;        // Acceleration factor
	float	airaccelerate;     // Same for when in open air
	float	wateraccelerate;   // Same for when in water
	float	friction;
	float   edgefriction;	   // Extra friction near dropofs 
	float	waterfriction;     // Less in water
	float	entgravity;        // 1.0
	float   bounce;            // Wall bounce value. 1.0
	float   stepsize;          // sv_stepsize;
	float   maxvelocity;       // maximum server velocity.
	float	zmax;			   // Max z-buffer range (for GL)
	float	waveHeight;		   // Water wave height (for GL)
	qboolean footsteps;        // Play footstep sounds
	char	skyName[ 32 ];	   // Name of the sky map
	float	rollangle;
	float	rollspeed;
	float	skycolor_r;			// Sky color
	float	skycolor_g;			// 
	float	skycolor_b;			//
	float	skyvec_x;			// Sky vector
	float	skyvec_y;			// 
	float	skyvec_z;			// 
};

typedef struct ref_params_s
{
	// Output
	float	vieworg[ 3 ];
	float	viewangles[ 3 ];

	float	forward[ 3 ];
	float	right[ 3 ];
	float   up[ 3 ];

	// Client frametime;
	float	frametime;
	// Client time
	float	time;

	// Misc
	int		intermission;
	int		paused;
	int		spectator;
	int		onground;
	int		waterlevel;

	float	simvel[ 3 ];
	float	simorg[ 3 ];

	float	viewheight[ 3 ];
	float	idealpitch;

	float	cl_viewangles[ 3 ];

	int		health;
	float	crosshairangle[ 3 ];
	float	viewsize;

	float	punchangle[ 3 ];
	int		maxclients;
	int		viewentity;
	int		playernum;
	int		max_entities;
	int		demoplayback;
	int		hardware;

	int		smoothing;

	// Last issued usercmd
	struct usercmd_s *cmd;

	// Movevars
	struct movevars_s *movevars;

	int		viewport[ 4 ];		// the viewport coordinates x ,y , width, height

	int		nextView;			// the renderer calls ClientDLL_CalcRefdef() and Renderview
	// so long in cycles until this value is 0 (multiple views)
	int		onlyClientDraw;		// if !=0 nothing is drawn by the engine except clientDraw functions
} ref_params_t;

typedef struct link_s
{
	struct link_s *prev, *next;
} link_t;

typedef struct edict_s edict_t;
typedef int	string_t;

typedef struct
{
	float time;
	float frametime;
	float force_retouch;
	string_t mapname;
	string_t startspot;
	float deathmatch;
	float coop;
	float teamplay;
	float serverflags;
	float found_secrets;
	Vector v_forward;
	Vector v_up;
	Vector v_right;
	float trace_allsolid;
	float trace_startsolid;
	float trace_fraction;
	Vector trace_endpos;
	Vector trace_plane_normal;
	float trace_plane_dist;
	edict_t *trace_ent;
	float trace_inopen;
	float trace_inwater;
	int trace_hitgroup;
	int trace_flags;
	int msg_entity;
	int cdAudioTrack;
	int maxClients;
	int maxEntities;
	const char *pStringBase;

	void *pSaveData;
	Vector vecLandmarkOffset;
} globalvars_t;

typedef struct entvars_s
{
	string_t	classname;
	string_t	globalname;

	vec3_t		origin;
	vec3_t		oldorigin;
	vec3_t		velocity;
	vec3_t		basevelocity;
	vec3_t      clbasevelocity;  // Base velocity that was passed in to server physics so 
	//  client can predict conveyors correctly.  Server zeroes it, so we need to store here, too.
	vec3_t		movedir;

	vec3_t		angles;			// Model angles
	vec3_t		avelocity;		// angle velocity (degrees per second)
	vec3_t		punchangle;		// auto-decaying view angle adjustment
	vec3_t		v_angle;		// Viewing angle (player only)

	// For parametric entities
	vec3_t		endpos;
	vec3_t		startpos;
	float		impacttime;
	float		starttime;

	int			fixangle;		// 0:nothing, 1:force view angles, 2:add avelocity
	float		idealpitch;
	float		pitch_speed;
	float		ideal_yaw;
	float		yaw_speed;

	int			modelindex;
	string_t	model;

	int			viewmodel;		// player's viewmodel
	int			weaponmodel;	// what other players see

	vec3_t		absmin;		// BB max translated to world coord
	vec3_t		absmax;		// BB max translated to world coord
	vec3_t		mins;		// local BB min
	vec3_t		maxs;		// local BB max
	vec3_t		size;		// maxs - mins

	float		ltime;
	float		nextthink;

	int			movetype;
	int			solid;

	int			skin;
	int			body;			// sub-model selection for studiomodels
	int 		effects;

	float		gravity;		// % of "normal" gravity
	float		friction;		// inverse elasticity of MOVETYPE_BOUNCE

	int			light_level;

	int			sequence;		// animation sequence
	int			gaitsequence;	// movement animation sequence for player (0 for none)
	float		frame;			// % playback position in animation sequences (0..255)
	float		animtime;		// world time when frame was set
	float		framerate;		// animation playback rate (-8x to 8x)
	unsigned char		controller[ 4 ];	// bone controller setting (0..255)
	unsigned char		blending[ 2 ];	// blending amount between sub-sequences (0..255)

	float		scale;			// sprite rendering scale (0..255)

	int			rendermode;
	float		renderamt;
	vec3_t		rendercolor;
	int			renderfx;

	float		health;
	float		frags;
	int			weapons;  // bit mask for available weapons
	float		takedamage;

	int			deadflag;
	vec3_t		view_ofs;	// eye position

	int			button;
	int			impulse;

	edict_t *chain;			// Entity pointer when linked into a linked list
	edict_t *dmg_inflictor;
	edict_t *enemy;
	edict_t *aiment;		// entity pointer when MOVETYPE_FOLLOW
	edict_t *owner;
	edict_t *groundentity;

	int			spawnflags;
	int			flags;

	int			colormap;		// lowbyte topcolor, highbyte bottomcolor
	int			team;

	float		max_health;
	float		teleport_time;
	float		armortype;
	float		armorvalue;
	int			waterlevel;
	int			watertype;

	string_t	target;
	string_t	targetname;
	string_t	netname;
	string_t	message;

	float		dmg_take;
	float		dmg_save;
	float		dmg;
	float		dmgtime;

	string_t	noise;
	string_t	noise1;
	string_t	noise2;
	string_t	noise3;

	float		speed;
	float		air_finished;
	float		pain_finished;
	float		radsuit_finished;

	edict_t *pContainingEntity;

	int			playerclass;
	float		maxspeed;

	float		fov;
	int			weaponanim;

	int			pushmsec;

	int			bInDuck;
	int			flTimeStepSound;
	int			flSwimTime;
	int			flDuckTime;
	int			iStepLeft;
	float		flFallVelocity;

	int			gamestate;

	int			oldbuttons;

	int			groupinfo;

	// For mods
	int			iuser1;
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
	vec3_t		vuser1;
	vec3_t		vuser2;
	vec3_t		vuser3;
	vec3_t		vuser4;
	edict_t *euser1;
	edict_t *euser2;
	edict_t *euser3;
	edict_t *euser4;
} entvars_t;

typedef struct clientdata_s
{
	vec3_t				origin;
	vec3_t				velocity;

	int					viewmodel;
	vec3_t				punchangle;
	int					flags;
	int					waterlevel;
	int					watertype;
	vec3_t				view_ofs;
	float				health;

	int					bInDuck;

	int					weapons; // remove?

	int					flTimeStepSound;
	int					flDuckTime;
	int					flSwimTime;
	int					waterjumptime;

	float				maxspeed;

	float				fov;
	int					weaponanim;

	int					m_iId;
	int					ammo_shells;
	int					ammo_nails;
	int					ammo_cells;
	int					ammo_rockets;
	float				m_flNextAttack;

	int					tfstate;

	int					pushmsec;

	int					deadflag;

	char				physinfo[ MAX_PHYSINFO_STRING ];

	// For mods
	int					iuser1;
	int					iuser2;
	int					iuser3;
	int					iuser4;
	float				fuser1;
	float				fuser2;
	float				fuser3;
	float				fuser4;
	vec3_t				vuser1;
	vec3_t				vuser2;
	vec3_t				vuser3;
	vec3_t				vuser4;
} clientdata_t;

// Info about weapons player might have in his/her possession
typedef struct weapon_data_s
{
	int			m_iId;
	int			m_iClip;

	float		m_flNextPrimaryAttack;
	float		m_flNextSecondaryAttack;
	float		m_flTimeWeaponIdle;

	int			m_fInReload;
	int			m_fInSpecialReload;
	float		m_flNextReload;
	float		m_flPumpTime;
	float		m_fReloadTime;

	float		m_fAimedDamage;
	float		m_fNextAimBonus;
	int			m_fInZoom;
	int			m_iWeaponState;

	int			iuser1;
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
} weapon_data_t;

// @Sw1ft: Sven Co-op's specific client-side weapon info storage & HUD
#define MAX_WEAPON_SLOTS			10	 // hud item selection slots (5 for original Half-Life)
#define MAX_WEAPON_SLOTS_5_11		7 // SC 5.15 <

#define MAX_WEAPON_POSITIONS_5_23	25 // this is the max number of items in each bucket
#define MAX_WEAPON_POSITIONS		20 // SC 5.23 <
#define MAX_WEAPON_POSITIONS_5_11	( 20 - 1 ) // SC 5.15 <

#define MAX_AMMO_TYPES				64 // 32 in HL
#define MAX_AMMO_TYPES_5_11			32 // SC 5.15 <

#define MAX_WEAPON_NAME 128
#define WEAPON_FLAGS_SELECTONEMPTY 1
#define WEAPON_IS_ONTARGET 0x40

struct Point
{
	int x = 0;
	int y = 0;
};

struct Rect
{
	int left = 0;
	int right = 0;
	int top = 0;
	int bottom = 0;
};

struct WEAPON
{
	char szName[ MAX_WEAPON_NAME ];
	int iAmmoType;
	int iAmmo2Type;
	int iMax1;
	int iMax2;
	int iSlot;
	int iSlotPos;
	int iFlags;
	int iId;
	int iClip;

	int iCount; // # of items in plist

	HSPRITE_HL hActive;
	Rect rcActive;
	HSPRITE_HL hInactive;
	Rect rcInactive;
	HSPRITE_HL hAmmo;
	Rect rcAmmo;
	HSPRITE_HL hAmmo2;
	Rect rcAmmo2;
	HSPRITE_HL hCrosshair;
	Rect rcCrosshair;
	HSPRITE_HL hAutoaim;
	Rect rcAutoaim;
	HSPRITE_HL hZoomedCrosshair;
	Rect rcZoomedCrosshair;
	HSPRITE_HL hZoomedAutoaim;
	Rect rcZoomedAutoaim;
};

typedef int AMMO;

class WeaponsResource
{
private:
	WEAPON *rgSlots[ MAX_WEAPON_SLOTS + 1 ][ MAX_WEAPON_POSITIONS + 1 ];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	int	riAmmo[ MAX_AMMO_TYPES ];							// count of each ammo type

public:
	inline WEAPON *GetWeaponSlot( int slot, int pos ) { return rgSlots[ slot ][ pos ]; }
	inline WEAPON *GetFirstPos( int iSlot )
	{
		WEAPON *pret = NULL;

		for ( int i = 0; i < MAX_WEAPON_POSITIONS; i++ )
		{
			if ( rgSlots[ iSlot ][ i ] && HasAmmo( rgSlots[ iSlot ][ i ] ) )
			{
				pret = rgSlots[ iSlot ][ i ];
				break;
			}
		}

		return pret;
	}
	inline WEAPON *GetNextActivePos( int iSlot, int iSlotPos )
	{
		if ( iSlotPos >= MAX_WEAPON_POSITIONS || iSlot >= MAX_WEAPON_SLOTS )
			return NULL;

		WEAPON *p = this->rgSlots[ iSlot ][ iSlotPos + 1 ];

		if ( !p || !this->HasAmmo( p ) )
			return GetNextActivePos( iSlot, iSlotPos + 1 );

		return p;
	}

	inline bool HasAmmo( WEAPON *p )
	{
		if ( !p )
			return false;

		// weapons with no max ammo can always be selected
		if ( p->iMax1 == -1 )
			return true;

		return ( p->iAmmoType == -1 ) || p->iClip > 0 || CountAmmo( p->iAmmoType ) || CountAmmo( p->iAmmo2Type ) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
	}

	///// AMMO /////
	inline AMMO GetAmmo( int iId ) { return riAmmo[ iId ]; }
	inline void SetAmmo( int iId, int iCount ) { riAmmo[ iId ] = iCount; }
	inline int CountAmmo( int iId )
	{
		if ( iId < 0 )
			return 0;

		return riAmmo[ iId ];
	}
};

class WeaponsResource_5_23
{
private:
	WEAPON *rgSlots[ MAX_WEAPON_SLOTS + 1 ][ MAX_WEAPON_POSITIONS_5_23 + 1 ];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	int	riAmmo[ MAX_AMMO_TYPES ];							// count of each ammo type

public:
	inline WEAPON *GetWeaponSlot( int slot, int pos ) { return rgSlots[ slot ][ pos ]; }
	inline WEAPON *GetFirstPos( int iSlot )
	{
		WEAPON *pret = NULL;

		for ( int i = 0; i < MAX_WEAPON_POSITIONS_5_23; i++ )
		{
			if ( rgSlots[ iSlot ][ i ] && HasAmmo( rgSlots[ iSlot ][ i ] ) )
			{
				pret = rgSlots[ iSlot ][ i ];
				break;
			}
		}

		return pret;
	}
	inline WEAPON *GetNextActivePos( int iSlot, int iSlotPos )
	{
		if ( iSlotPos >= MAX_WEAPON_POSITIONS_5_23 || iSlot >= MAX_WEAPON_SLOTS )
			return NULL;

		WEAPON *p = this->rgSlots[ iSlot ][ iSlotPos + 1 ];

		if ( !p || !this->HasAmmo( p ) )
			return GetNextActivePos( iSlot, iSlotPos + 1 );

		return p;
	}

	inline bool HasAmmo( WEAPON *p )
	{
		if ( !p )
			return false;

		// weapons with no max ammo can always be selected
		if ( p->iMax1 == -1 )
			return true;

		return ( p->iAmmoType == -1 ) || p->iClip > 0 || CountAmmo( p->iAmmoType ) || CountAmmo( p->iAmmo2Type ) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
	}

	///// AMMO /////
	inline AMMO GetAmmo( int iId ) { return riAmmo[ iId ]; }
	inline void SetAmmo( int iId, int iCount ) { riAmmo[ iId ] = iCount; }
	inline int CountAmmo( int iId )
	{
		if ( iId < 0 )
			return 0;

		return riAmmo[ iId ];
	}
};

class WeaponsResource_5_11
{
private:
	WEAPON *rgSlots[ MAX_WEAPON_SLOTS_5_11 + 1 ][ MAX_WEAPON_POSITIONS_5_11 + 1 ];	// The slots currently in use by weapons.  The value is a pointer to the weapon;  if it's NULL, no weapon is there
	int	riAmmo[ MAX_AMMO_TYPES_5_11 ];							// count of each ammo type

public:
	inline WEAPON *GetWeaponSlot( int slot, int pos ) { return rgSlots[ slot ][ pos ]; }
	inline WEAPON *GetFirstPos( int iSlot )
	{
		WEAPON *pret = NULL;

		for ( int i = 0; i < MAX_WEAPON_POSITIONS_5_11; i++ )
		{
			if ( rgSlots[ iSlot ][ i ] && HasAmmo( rgSlots[ iSlot ][ i ] ) )
			{
				pret = rgSlots[ iSlot ][ i ];
				break;
			}
		}

		return pret;
	}
	inline WEAPON *GetNextActivePos( int iSlot, int iSlotPos )
	{
		if ( iSlotPos >= MAX_WEAPON_POSITIONS_5_11 || iSlot >= MAX_WEAPON_SLOTS_5_11 )
			return NULL;

		WEAPON *p = this->rgSlots[ iSlot ][ iSlotPos + 1 ];

		if ( !p || !this->HasAmmo( p ) )
			return GetNextActivePos( iSlot, iSlotPos + 1 );

		return p;
	}

	inline bool HasAmmo( WEAPON *p )
	{
		if ( !p )
			return false;

		// weapons with no max ammo can always be selected
		if ( p->iMax1 == -1 )
			return true;

		return ( p->iAmmoType == -1 ) || p->iClip > 0 || CountAmmo( p->iAmmoType ) || CountAmmo( p->iAmmo2Type ) || ( p->iFlags & WEAPON_FLAGS_SELECTONEMPTY );
	}

	///// AMMO /////
	inline AMMO GetAmmo( int iId ) { return riAmmo[ iId ]; }
	inline void SetAmmo( int iId, int iCount ) { riAmmo[ iId ] = iCount; }
	inline int CountAmmo( int iId )
	{
		if ( iId < 0 )
			return 0;

		return riAmmo[ iId ];
	}
};

#define MAX_PLAYERS_HUD 32 // 64 for HL
#define MAX_TEAMS		5 // 64 for HL
#define MAX_TEAM_NAME	64 // 16 for HL
#define MAX_TEAM_NAME_5_11 16

struct extra_player_info_t // sizeof 88 * 33 = 2904
{
	float frags;
	int deaths;
	short playerclass;
	short teamnumber;
	char teamname[ MAX_TEAM_NAME ];
	float health;
	float armor;

	// Sven Co-op specific
	char pad_1[ 4 ];
};

struct extra_player_info_5_26_t // sizeof 92 * 33 = 3036
{
	float frags;
	int deaths;
	short playerclass;
	short teamnumber;
	char teamname[ MAX_TEAM_NAME ];
	float health;
	float armor;

	// Sven Co-op specific
	char pad_1[ 4 ];
	char pad_2[ 4 ];
};

// FIXME: reverse it properly
struct extra_player_info_5_11_t
{
	float frags;
	int deaths;
	short playerclass;
	short teamnumber;
	char teamname[ MAX_TEAM_NAME_5_11 ];
	float health;
	float armor;

	// Sven Co-op specific
	char pad_1[ 4 ];

	/*
	short frags;
	short deaths;
	short playerclass;
	//short health; // UNUSED currently, spectator UI would like this
	bool dead; // UNUSED currently, spectator UI would like this
	short teamnumber;
	char teamname[ MAX_TEAM_NAME_5_11 ];
	float health;
	float armor;
	char pad_1[ 4 ];
	*/
};

struct team_info_t
{
	char name[ MAX_TEAM_NAME ];

	// Sven Co-op specific
	char pad_1[ 14 ];

	short frags;
	short deaths;
	short ping;
	short packetloss;
	bool ownteam;
	short players;
	bool already_drawn;
	bool scores_overriden;
	int teamnumber;

	// Sven Co-op specific
	char pad_2[ 8 ];
};

struct team_info_5_11_t
{
	char name[ MAX_TEAM_NAME_5_11 ];

	// Sven Co-op specific
	char pad_1[ 14 ];

	short frags;
	short deaths;
	short ping;
	short packetloss;
	bool ownteam;
	short players;
	bool already_drawn;
	bool scores_overriden;
	int teamnumber;

	// Sven Co-op specific
	char pad_2[ 8 ];
};

// Entity dictionary
#define	MAX_ENT_LEAFS	48

struct edict_s
{
	qboolean	free;
	int			serialnumber;
	link_t		area;				// linked to a division node or leaf

	int			headnode;			// -1 to use normal leaf check
	int			num_leafs;
	short		leafnums[ MAX_ENT_LEAFS ];

	float		freetime;			// sv.time when the object was freed

	void *pvPrivateData;		// Alloced and freed by engine, used by DLLs

	entvars_t	v;					// C exported fields from progs

	// other fields from progs come immediately after
};

typedef struct trace_s
{
	qboolean allsolid; // if true, plane is not valid
	qboolean startsolid; // if true, the initial point was in a solid area
	qboolean inopen, inwater;
	float fraction; // time completed, 1.0 = didn't hit anything
	Vector endpos; // final position
	plane_t plane; // surface normal at impact
	edict_t *ent; // entity the surface is on
	int hitgroup; // 0 == generic, non zero is specific body part
} trace_t;

// @Sw1ft: Sven Co-op specific, 32 for HL
#define MAX_WEAPONS 256 // SC 5.22 >=
#define MAX_WEAPONS_5_11 64 // SC 5.22 <

typedef struct local_state_s
{
	entity_state_t playerstate;
	clientdata_t   client;
	weapon_data_t  weapondata[ MAX_WEAPONS ];
} local_state_t;

typedef struct local_state_5_11_s
{
	entity_state_t playerstate;
	clientdata_t   client;
	weapon_data_t  weapondata[ MAX_WEAPONS_5_11 ];
} local_state_5_11_t;

typedef struct efrag_s
{
	struct mleaf_s *leaf;
	struct efrag_s *leafnext;
	struct cl_entity_s *entity;
	struct efrag_s *entnext;
} efrag_t;

typedef struct
{
	unsigned char					mouthopen;		// 0 = mouth closed, 255 = mouth agape
	unsigned char					sndcount;		// counter for running average
	int						sndavg;			// running average
} mouth_t;

typedef struct
{
	float					prevanimtime;
	float					sequencetime;
	unsigned char					prevseqblending[ 2 ];
	vec3_t					prevorigin;
	vec3_t					prevangles;

	int						prevsequence;
	float					prevframe;

	unsigned char					prevcontroller[ 4 ];
	unsigned char					prevblending[ 2 ];
} latchedvars_t;

typedef struct
{
	// Time stamp for this movement
	float					animtime;

	vec3_t					origin;
	vec3_t					angles;
} position_history_t;

#define HISTORY_MAX		64  // Must be power of 2

typedef struct cl_entity_s cl_entity_t;

struct cl_entity_s
{
	int						index;      // Index into cl_entities ( should match actual slot, but not necessarily )

	qboolean				player;     // True if this entity is a "player"

	entity_state_t			baseline;   // The original state from which to delta during an uncompressed message
	entity_state_t			prevstate;  // The state information from the penultimate message received from the server
	entity_state_t			curstate;   // The state information from the last message received from server

	int						current_position;  // Last received history update index
	position_history_t		ph[ HISTORY_MAX ];   // History of position and angle updates for this player

	mouth_t					mouth;			// For synchronizing mouth movements.

	latchedvars_t			latched;		// Variables used by studio model rendering routines

	// Information based on interplocation, extrapolation, prediction, or just copied from last msg received.
	//
	float					lastmove;

	// Actual render position and angles
	vec3_t					origin;
	vec3_t					angles;

	// Attachment points
	vec3_t					attachment[ 4 ];

	// Other entity local information
	int						trivial_accept;

	struct model_s *model;			// cl.model_precache[ curstate.modelindes ];  all visible entities have a model
	struct efrag_s *efrag;			// linked list of efrags
	struct mnode_s *topnode;		// for bmodels, first world node that splits bmodel, or NULL if not split

	float					syncbase;		// for client-side animations -- used by obsolete alias animation system, remove?
	int						visframe;		// last frame this entity was found in an active leaf
	colorVec				cvFloorColor;
};

//--------------------------------------------------------------------------
// sequenceDefaultBits_e
//	
// Enumerated list of possible modifiers for a command.  This enumeration
// is used in a bitarray controlling what modifiers are specified for a command.
//---------------------------------------------------------------------------
enum sequenceModifierBits
{
	SEQUENCE_MODIFIER_EFFECT_BIT = ( 1 << 1 ),
	SEQUENCE_MODIFIER_POSITION_BIT = ( 1 << 2 ),
	SEQUENCE_MODIFIER_COLOR_BIT = ( 1 << 3 ),
	SEQUENCE_MODIFIER_COLOR2_BIT = ( 1 << 4 ),
	SEQUENCE_MODIFIER_FADEIN_BIT = ( 1 << 5 ),
	SEQUENCE_MODIFIER_FADEOUT_BIT = ( 1 << 6 ),
	SEQUENCE_MODIFIER_HOLDTIME_BIT = ( 1 << 7 ),
	SEQUENCE_MODIFIER_FXTIME_BIT = ( 1 << 8 ),
	SEQUENCE_MODIFIER_SPEAKER_BIT = ( 1 << 9 ),
	SEQUENCE_MODIFIER_LISTENER_BIT = ( 1 << 10 ),
	SEQUENCE_MODIFIER_TEXTCHANNEL_BIT = ( 1 << 11 ),
};
typedef enum sequenceModifierBits sequenceModifierBits_e;


//---------------------------------------------------------------------------
// sequenceCommandEnum_e
// 
// Enumerated sequence command types.
//---------------------------------------------------------------------------
enum sequenceCommandEnum_
{
	SEQUENCE_COMMAND_ERROR = -1,
	SEQUENCE_COMMAND_PAUSE = 0,
	SEQUENCE_COMMAND_FIRETARGETS,
	SEQUENCE_COMMAND_KILLTARGETS,
	SEQUENCE_COMMAND_TEXT,
	SEQUENCE_COMMAND_SOUND,
	SEQUENCE_COMMAND_GOSUB,
	SEQUENCE_COMMAND_SENTENCE,
	SEQUENCE_COMMAND_REPEAT,
	SEQUENCE_COMMAND_SETDEFAULTS,
	SEQUENCE_COMMAND_MODIFIER,
	SEQUENCE_COMMAND_POSTMODIFIER,
	SEQUENCE_COMMAND_NOOP,

	SEQUENCE_MODIFIER_EFFECT,
	SEQUENCE_MODIFIER_POSITION,
	SEQUENCE_MODIFIER_COLOR,
	SEQUENCE_MODIFIER_COLOR2,
	SEQUENCE_MODIFIER_FADEIN,
	SEQUENCE_MODIFIER_FADEOUT,
	SEQUENCE_MODIFIER_HOLDTIME,
	SEQUENCE_MODIFIER_FXTIME,
	SEQUENCE_MODIFIER_SPEAKER,
	SEQUENCE_MODIFIER_LISTENER,
	SEQUENCE_MODIFIER_TEXTCHANNEL,
};
typedef enum sequenceCommandEnum_ sequenceCommandEnum_e;


//---------------------------------------------------------------------------
// sequenceCommandType_e
// 
// Typeerated sequence command types.
//---------------------------------------------------------------------------
enum sequenceCommandType_
{
	SEQUENCE_TYPE_COMMAND,
	SEQUENCE_TYPE_MODIFIER,
};
typedef enum sequenceCommandType_ sequenceCommandType_e;


//---------------------------------------------------------------------------
// sequenceCommandMapping_s
// 
// A mapping of a command enumerated-value to its name.
//---------------------------------------------------------------------------
typedef struct sequenceCommandMapping_ sequenceCommandMapping_s;
struct sequenceCommandMapping_
{
	sequenceCommandEnum_e	commandEnum;
	const char *commandName;
	sequenceCommandType_e	commandType;
};


//---------------------------------------------------------------------------
// sequenceCommandLine_s
// 
// Structure representing a single command (usually 1 line) from a
//	.SEQ file entry.
//---------------------------------------------------------------------------
typedef struct sequenceCommandLine_ sequenceCommandLine_s;
struct sequenceCommandLine_
{
	int						commandType;		// Specifies the type of command
	client_textmessage_t	clientMessage;		// Text HUD message struct
	char *speakerName;		// Targetname of speaking entity
	char *listenerName;		// Targetname of entity being spoken to
	char *soundFileName;		// Name of sound file to play
	char *sentenceName;		// Name of sentences.txt to play
	char *fireTargetNames;	// List of targetnames to fire
	char *killTargetNames;	// List of targetnames to remove
	float					delay;				// Seconds 'till next command
	int						repeatCount;		// If nonzero, reset execution pointer to top of block (N times, -1 = infinite)
	int						textChannel;		// Display channel on which text message is sent
	int						modifierBitField;	// Bit field to specify what clientmessage fields are valid
	sequenceCommandLine_s *nextCommandLine;	// Next command (linked list)
};


//---------------------------------------------------------------------------
// sequenceEntry_s
// 
// Structure representing a single command (usually 1 line) from a
//	.SEQ file entry.
//---------------------------------------------------------------------------
typedef struct sequenceEntry_ sequenceEntry_s;
struct sequenceEntry_
{
	char *fileName;		// Name of sequence file without .SEQ extension
	char *entryName;		// Name of entry label in file
	sequenceCommandLine_s *firstCommand;	// Linked list of commands in entry
	sequenceEntry_s *nextEntry;		// Next loaded entry
	qboolean				isGlobal;		// Is entry retained over level transitions?
};



//---------------------------------------------------------------------------
// sentenceEntry_s
// Structure representing a single sentence of a group from a .SEQ
// file entry.  Sentences are identical to entries in sentences.txt, but
// can be unique per level and are loaded/unloaded with the level.
//---------------------------------------------------------------------------
typedef struct sentenceEntry_ sentenceEntry_s;
struct sentenceEntry_
{
	char *data;			// sentence data (ie "We have hostiles" )
	sentenceEntry_s *nextEntry;		// Next loaded entry
	qboolean				isGlobal;		// Is entry retained over level transitions?
	unsigned int			index;			// this entry's position in the file.
};

//--------------------------------------------------------------------------
// sentenceGroupEntry_s
// Structure representing a group of sentences found in a .SEQ file.
// A sentence group is defined by all sentences with the same name, ignoring
// the number at the end of the sentence name.  Groups enable a sentence
// to be picked at random across a group.
//--------------------------------------------------------------------------
typedef struct sentenceGroupEntry_ sentenceGroupEntry_s;
struct sentenceGroupEntry_
{
	char *groupName;		// name of the group (ie CT_ALERT )
	unsigned int			numSentences;	// number of sentences in group
	sentenceEntry_s *firstSentence;	// head of linked list of sentences in group
	sentenceGroupEntry_s *nextEntry;		// next loaded group
};

#define	MAX_ALIAS_NAME	32

typedef struct cmdalias_s
{
	struct cmdalias_s *next;
	char	name[ MAX_ALIAS_NAME ];
	char *value;
} cmdalias_t;

/*
==============================================================================
STUDIO MODELS
Studio models are position independent, so the cache manager can move them.
==============================================================================
*/


#define MAXSTUDIOTRIANGLES	20000	// TODO: tune this
#define MAXSTUDIOVERTS		2048	// TODO: tune this
#define MAXSTUDIOSEQUENCES	2048	// total animation sequences -- KSH incremented
#define MAXSTUDIOSKINS		100		// total textures
#define MAXSTUDIOSRCBONES	512		// bones allowed at source movement
#define MAXSTUDIOBONES		128		// total bones actually used
#define MAXSTUDIOMODELS		32		// sub-models per model
#define MAXSTUDIOBODYPARTS	32
#define MAXSTUDIOGROUPS		16
#define MAXSTUDIOANIMATIONS	2048		
#define MAXSTUDIOMESHES		256
#define MAXSTUDIOEVENTS		1024
#define MAXSTUDIOPIVOTS		256
#define MAXSTUDIOCONTROLLERS 8

typedef struct
{
	int					id;
	int					version;

	char				name[ 64 ];
	int					length;

	vec3_t				eyeposition;	// ideal eye position
	vec3_t				min;			// ideal movement hull size
	vec3_t				max;

	vec3_t				bbmin;			// clipping bounding box
	vec3_t				bbmax;

	int					flags;

	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxes;			// complex bounding boxes
	int					hitboxindex;

	int					numseq;				// animation sequences
	int					seqindex;

	int					numseqgroups;		// demand loaded sequences
	int					seqgroupindex;

	int					numtextures;		// raw textures
	int					textureindex;
	int					texturedataindex;

	int					numskinref;			// replaceable textures
	int					numskinfamilies;
	int					skinindex;

	int					numbodyparts;
	int					bodypartindex;

	int					numattachments;		// queryable attachable points
	int					attachmentindex;

	int					soundtable;
	int					soundindex;
	int					soundgroups;
	int					soundgroupindex;

	int					numtransitions;		// animation node to animation node transition graph
	int					transitionindex;
} studiohdr_t;

// header for demand loaded sequence group data
typedef struct
{
	int					id;
	int					version;

	char				name[ 64 ];
	int					length;
} studioseqhdr_t;

// bones
typedef struct
{
	char				name[ 32 ];	// bone name for symbolic links
	int		 			parent;		// parent bone
	int					flags;		// ??
	int					bonecontroller[ 6 ];	// bone controller index, -1 == none
	float				value[ 6 ];	// default DoF values
	float				scale[ 6 ];   // scale for delta DoF values
} mstudiobone_t;


// bone controllers
typedef struct
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					index;	// 0-3 user set controller, 4 mouth
} mstudiobonecontroller_t;

// intersection boxes
typedef struct
{
	int					bone;
	int					group;			// intersection group
	vec3_t				bbmin;		// bounding box
	vec3_t				bbmax;
} mstudiobbox_t;

//
// demand loaded sequence groups
//
typedef struct
{
	char				label[ 32 ];	// textual name
	char				name[ 64 ];	// file name
	int					unused1;    // was "cache"  - index pointer
	int					unused2;    // was "data" -  hack for group 0
} mstudioseqgroup_t;

// sequence descriptions
typedef struct
{
	char				label[ 32 ];	// sequence label

	float				fps;		// frames per second	
	int					flags;		// looping/non-looping flags

	int					activity;
	int					actweight;

	int					numevents;
	int					eventindex;

	int					numframes;	// number of frames per sequence

	int					numpivots;	// number of foot pivots
	int					pivotindex;

	int					motiontype;
	int					motionbone;
	vec3_t				linearmovement;
	int					automoveposindex;
	int					automoveangleindex;

	vec3_t				bbmin;		// per sequence bounding box
	vec3_t				bbmax;

	int					numblends;
	int					animindex;		// mstudioanim_t pointer relative to start of sequence group data
	// [blend][bone][X, Y, Z, XR, YR, ZR]

	int					blendtype[ 2 ];	// X, Y, Z, XR, YR, ZR
	float				blendstart[ 2 ];	// starting value
	float				blendend[ 2 ];	// ending value
	int					blendparent;

	int					seqgroup;		// sequence group for demand loading

	int					entrynode;		// transition node at entry
	int					exitnode;		// transition node at exit
	int					nodeflags;		// transition rules

	int					nextseq;		// auto advancing sequences
} mstudioseqdesc_t;

// events
#define STUDIO_EVENT_SPECIFIC 0
#define STUDIO_EVENT_SCRIPTED 1000
#define STUDIO_EVENT_SHARED 2000
#define STUDIO_EVENT_CLIENT 5000

#define AE_TYPE_SERVER			( 1 << 0 )
#define AE_TYPE_SCRIPTED		( 1 << 1 )
#define AE_TYPE_SHARED			( 1 << 2 )
#define AE_TYPE_WEAPON			( 1 << 3 )
#define AE_TYPE_CLIENT			( 1 << 4 )
#define AE_TYPE_FACEPOSER		( 1 << 5 )

#define AE_NOT_AVAILABLE		-1

enum StudioAnimEvent
{
	// Uh.. are AE_ (server) events valid for GoldSrc?
	AE_INVALID = -1, // So we have something more succint to check for than '-1'
	AE_EMPTY,
	AE_NPC_LEFTFOOT, // NPC_EVENT_LEFTFOOT 2050
	AE_NPC_RIGHTFOOT, // NPC_EVENT_RIGHTFOOT 2051
	AE_NPC_BODYDROP_LIGHT, // NPC_EVENT_BODYDROP_LIGHT 2001
	AE_NPC_BODYDROP_HEAVY, // NPC_EVENT_BODYDROP_HEAVY 2002
	AE_NPC_SWISHSOUND, // NPC_EVENT_SWISHSOUND 2010
	AE_NPC_180TURN, // NPC_EVENT_180TURN 2020
	AE_NPC_ITEM_PICKUP, // NPC_EVENT_ITEM_PICKUP 2040
	AE_NPC_WEAPON_DROP, // NPC_EVENT_WEAPON_DROP 2041
	AE_NPC_WEAPON_SET_SEQUENCE_NAME, // NPC_EVENT_WEAPON_SET_SEQUENCE_NAME 2042
	AE_NPC_WEAPON_SET_SEQUENCE_NUMBER, // NPC_EVENT_WEAPON_SET_SEQUENCE_NUMBER 2043
	AE_NPC_WEAPON_SET_ACTIVITY, // NPC_EVENT_WEAPON_SET_ACTIVITY 2044
	AE_NPC_HOLSTER,
	AE_NPC_DRAW,
	AE_CL_PLAYSOUND,// CL_EVENT_SOUND 5004 // Emit a sound
	AE_CL_STOPSOUND,
	AE_SV_PLAYSOUND,
	AE_START_SCRIPTED_EFFECT,
	AE_STOP_SCRIPTED_EFFECT,
	AE_CLIENT_EFFECT_ATTACH,
	AE_MUZZLEFLASH, // Muzzle flash from weapons held by the player
	AE_NPC_MUZZLEFLASH, // Muzzle flash from weapons held by NPCs
	AE_THUMPER_THUMP, //Thumper Thump!
	AE_AMMOCRATE_PICKUP_AMMO, //Ammo crate pick up ammo!
	AE_NPC_RAGDOLL,

	// Scripted sequence events
	SCRIPT_EVENT_DEAD = 1000, // 1000 character is now dead
	SCRIPT_EVENT_NOINTERRUPT, // 1001 does not allow interrupt
	SCRIPT_EVENT_CANINTERRUPT, // 1002 will allow interrupt
	SCRIPT_EVENT_FIREEVENT, // 1003 Fires OnScriptEventXX output in the script entity, where XX is the event number from the options data.
	SCRIPT_EVENT_SOUND, // 1004 Play named wave file (on CHAN_BODY)
	SCRIPT_EVENT_SENTENCE, // 1005 Play named sentence
	SCRIPT_EVENT_INAIR, // 1006 Leave the character in air at the end of the sequence (don't find the floor)
	SCRIPT_EVENT_ENDANIMATION, // 1007 Set the animation by name after the sequence completes
	SCRIPT_EVENT_SOUND_VOICE, // 1008 Play named wave file (on CHAN_VOICE)
	SCRIPT_EVENT_SENTENCE_RND1, // 1009 Play sentence group 25% of the time
	SCRIPT_EVENT_NOT_DEAD, // 1010 Bring back to life (for life/death sequences)
	SCRIPT_EVENT_EMPHASIS, // 1011 Emphasis point for gestures
	SCRIPT_EVENT_BODYGROUPON = 1020, // 1020 Turn a bodygroup on
	SCRIPT_EVENT_BODYGROUPOFF, // Turn a bodygroup off
	SCRIPT_EVENT_BODYGROUPTEMP, // Turn a bodygroup on until this sequence ends
	SCRIPT_EVENT_FIRE_INPUT = 1100, // Fires named input on the event handler

	// NPC/Monster events
	NPC_EVENT_BODYDROP_LIGHT = 2001,
	NPC_EVENT_BODYDROP_HEAVY,
	NPC_EVENT_SWISHSOUND = 2010,
	NPC_EVENT_180TURN = 2020,
	NPC_EVENT_ITEM_PICKUP = 2040,
	NPC_EVENT_WEAPON_DROP,
	NPC_EVENT_WEAPON_SET_SEQUENCE_NAME,
	NPC_EVENT_WEAPON_SET_SEQUENCE_NUMBER,
	NPC_EVENT_WEAPON_SET_ACTIVITY,
	NPC_EVENT_LEFTFOOT = 2050,
	NPC_EVENT_RIGHTFOOT,
	NPC_EVENT_OPEN_DOOR,

	// Weapon events
	EVENT_WEAPON_MELEE_HIT = 3001,
	EVENT_WEAPON_SMG1,
	EVENT_WEAPON_MELEE_SWISH,
	EVENT_WEAPON_SHOTGUN_FIRE,
	EVENT_WEAPON_THROW,
	EVENT_WEAPON_AR1,
	EVENT_WEAPON_AR2,
	EVENT_WEAPON_HMG1,
	EVENT_WEAPON_SMG2,
	EVENT_WEAPON_MISSILE_FIRE,
	EVENT_WEAPON_SNIPER_RIFLE_FIRE,
	EVENT_WEAPON_AR2_GRENADE,
	EVENT_WEAPON_THROW2,
	EVENT_WEAPON_PISTOL_FIRE,
	EVENT_WEAPON_RELOAD,
	EVENT_WEAPON_THROW3,
	EVENT_WEAPON_RELOAD_SOUND, // Use this + EVENT_WEAPON_RELOAD_FILL_CLIP to prevent shooting during the reload animation
	EVENT_WEAPON_RELOAD_FILL_CLIP,
	EVENT_WEAPON_SMG1_BURST1 = 3101, // first round in a 3-round burst
	EVENT_WEAPON_SMG1_BURSTN, // 2, 3 rounds
	EVENT_WEAPON_AR2_ALTFIRE,
	EVENT_WEAPON_SEQUENCE_FINISHED = 3900,
	EVENT_WEAPON_LAST = 3999, //Must be the last weapon event

	// Client-side events
	CL_EVENT_MUZZLEFLASH0 = 5001, // Muzzleflash on attachment 0
	CL_EVENT_MUZZLEFLASH1 = 5011, // Muzzleflash on attachment 1
	CL_EVENT_MUZZLEFLASH2 = 5021, // Muzzleflash on attachment 2
	CL_EVENT_MUZZLEFLASH3 = 5031, // Muzzleflash on attachment 3
	CL_EVENT_SPARK0 = 5002, // Spark on attachment 0
	CL_EVENT_NPC_MUZZLEFLASH0, // Muzzleflash on attachment 0 for third person views
	CL_EVENT_NPC_MUZZLEFLASH1 = 5013, // Muzzleflash on attachment 1 for third person views
	CL_EVENT_NPC_MUZZLEFLASH2 = 5023, // Muzzleflash on attachment 2 for third person views
	CL_EVENT_NPC_MUZZLEFLASH3 = 5033, // Muzzleflash on attachment 3 for third person views
	CL_EVENT_SOUND = 5004, // Emit a sound // NOTE THIS MUST MATCH THE DEFINE AT CBaseEntity::PrecacheModel on the server!!!!!
	CL_EVENT_EJECTBRASS1 = 6001, // Eject a brass shell from attachment 1
	CL_EVENT_DISPATCHEFFECT0 = 9001, // Hook into a DispatchEffect on attachment 0
	CL_EVENT_DISPATCHEFFECT1 = 9011, // Hook into a DispatchEffect on attachment 1
	CL_EVENT_DISPATCHEFFECT2 = 9021, // Hook into a DispatchEffect on attachment 2
	CL_EVENT_DISPATCHEFFECT3 = 9031, // Hook into a DispatchEffect on attachment 3
	CL_EVENT_DISPATCHEFFECT4 = 9041, // Hook into a DispatchEffect on attachment 4
	CL_EVENT_DISPATCHEFFECT5 = 9051, // Hook into a DispatchEffect on attachment 5
	CL_EVENT_DISPATCHEFFECT6 = 9061, // Hook into a DispatchEffect on attachment 6
	CL_EVENT_DISPATCHEFFECT7 = 9071, // Hook into a DispatchEffect on attachment 7
	CL_EVENT_DISPATCHEFFECT8 = 9081, // Hook into a DispatchEffect on attachment 8
	CL_EVENT_DISPATCHEFFECT9 = 9091, // Hook into a DispatchEffect on attachment 9
	CL_EVENT_FOOTSTEP_LEFT = 6004,
	CL_EVENT_FOOTSTEP_RIGHT,
	CL_EVENT_MFOOTSTEP_LEFT, // Footstep sounds based on material underfoot.
	CL_EVENT_MFOOTSTEP_RIGHT,
	CL_EVENT_MFOOTSTEP_LEFT_LOUD, // Loud material impact sounds from feet attachments
	CL_EVENT_MFOOTSTEP_RIGHT_LOUD,
	CL_EVENT_SPRITEGROUP_CREATE = 6002, // c_env_spritegroup
	CL_EVENT_SPRITEGROUP_DESTROY
};

typedef struct
{
	int 				frame;
	int					event;
	int					type;
	char				options[ 64 ];
} mstudioevent_t;

// pivots
typedef struct
{
	vec3_t				org;	// pivot point
	int					start;
	int					end;
} mstudiopivot_t;

// attachment
typedef struct
{
	char				name[ 32 ];
	int					type;
	int					bone;
	vec3_t				org;	// attachment point
	vec3_t				vectors[ 3 ];
} mstudioattachment_t;

typedef struct
{
	unsigned short	offset[ 6 ];
} mstudioanim_t;

// animation frames
typedef union
{
	struct
	{
		byte	valid;
		byte	total;
	} num;
	short		value;
} mstudioanimvalue_t;



// body part index
typedef struct
{
	char				name[ 64 ];
	int					nummodels;
	int					base;
	int					modelindex; // index into models array
} mstudiobodyparts_t;



// skin info
typedef struct
{
	char					name[ 64 ];
	int						flags;
	int						width;
	int						height;
	int						index;
} mstudiotexture_t;


// skin families
// short	index[skinfamilies][skinref]

// studio models
typedef struct
{
	char				name[ 64 ];

	int					type;

	float				boundingradius;

	int					nummesh;
	int					meshindex;

	int					numverts;		// number of unique vertices
	int					vertinfoindex;	// vertex bone info
	int					vertindex;		// vertex vec3_t
	int					numnorms;		// number of unique surface normals
	int					norminfoindex;	// normal bone info
	int					normindex;		// normal vec3_t

	int					numgroups;		// deformation groups
	int					groupindex;
} mstudiomodel_t;


// vec3_t	boundingbox[model][bone][2];	// complex intersection info


// meshes
typedef struct
{
	int					numtris;
	int					triindex;
	int					skinref;
	int					numnorms;		// per mesh normals
	int					normindex;		// normal vec3_t
} mstudiomesh_t;

// triangles
#if 0
typedef struct
{
	short				vertindex;		// index into vertex array
	short				normindex;		// index into normal array
	short				s, t;			// s,t position on skin
} mstudiotrivert_t;
#endif

// lighting options
#define STUDIO_NF_FLATSHADE		0x0001
#define STUDIO_NF_CHROME		0x0002
#define STUDIO_NF_FULLBRIGHT	0x0004
#define STUDIO_NF_NOMIPS        0x0008
#define STUDIO_NF_ALPHA         0x0010
#define STUDIO_NF_ADDITIVE      0x0020
#define STUDIO_NF_MASKED        0x0040

// motion flags
#define STUDIO_X		0x0001
#define STUDIO_Y		0x0002	
#define STUDIO_Z		0x0004
#define STUDIO_XR		0x0008
#define STUDIO_YR		0x0010
#define STUDIO_ZR		0x0020
#define STUDIO_LX		0x0040
#define STUDIO_LY		0x0080
#define STUDIO_LZ		0x0100
#define STUDIO_AX		0x0200
#define STUDIO_AY		0x0400
#define STUDIO_AZ		0x0800
#define STUDIO_AXR		0x1000
#define STUDIO_AYR		0x2000
#define STUDIO_AZR		0x4000
#define STUDIO_TYPES	0x7FFF
#define STUDIO_RLOOP	0x8000	// controller that wraps shortest distance

// sequence flags
#define STUDIO_LOOPING	0x0001

// bone flags
#define STUDIO_HAS_NORMALS	0x0001
#define STUDIO_HAS_VERTICES 0x0002
#define STUDIO_HAS_BBOX		0x0004
#define STUDIO_HAS_CHROME	0x0008	// if any of the textures have chrome on them

#define RAD_TO_STUDIO		(32768.0/M_PI)
#define STUDIO_TO_RAD		(M_PI/32768.0)

#define STUDIO_INTERFACE_VERSION 1

typedef struct engine_studio_api_s
{
	// Allocate number*size bytes and zero it
	void *( *Mem_Calloc )				( int number, size_t size );
	// Check to see if pointer is in the cache
	void *( *Cache_Check )				( struct cache_user_s *c );
	// Load file into cache ( can be swapped out on demand )
	void			( *LoadCacheFile )				( char *path, struct cache_user_s *cu );
	// Retrieve model pointer for the named model
	struct model_s *( *Mod_ForName )				( const char *name, int crash_if_missing );
	// Retrieve pointer to studio model data block from a model
	void *( *Mod_Extradata )				( struct model_s *mod );
	// Retrieve indexed model from client side model precache list
	struct model_s *( *GetModelByIndex )			( int index );
	// Get entity that is set for rendering
	struct cl_entity_s *( *GetCurrentEntity )		( void );
	// Get referenced player_info_t
	struct player_info_s *( *PlayerInfo )			( int index );
	// Get most recently received player state data from network system
	struct entity_state_s *( *GetPlayerState )		( int index );
	// Get viewentity
	struct cl_entity_s *( *GetViewEntity )			( void );
	// Get current frame count, and last two timestampes on client
	void			( *GetTimes )					( int *framecount, double *current, double *old );
	// Get a pointer to a cvar by name
	struct cvar_s *( *GetCvar )					( const char *name );
	// Get current render origin and view vectors ( up, right and vpn )
	void			( *GetViewInfo )				( float *origin, float *upv, float *rightv, float *vpnv );
	// Get sprite model used for applying chrome effect
	struct model_s *( *GetChromeSprite )			( void );
	// Get model counters so we can incement instrumentation
	void			( *GetModelCounters )			( int **s, int **a );
	// Get software scaling coefficients
	void			( *GetAliasScale )				( float *x, float *y );

	// Get bone, light, alias, and rotation matrices
	float ****( *StudioGetBoneTransform ) ( void );
	float ****( *StudioGetLightTransform )( void );
	float ***( *StudioGetAliasTransform ) ( void );
	float ***( *StudioGetRotationMatrix ) ( void );

	// Set up body part, and get submodel pointers
	void			( *StudioSetupModel )			( int bodypart, void **ppbodypart, void **ppsubmodel );
	// Check if entity's bbox is in the view frustum
	int				( *StudioCheckBBox )			( void );
	// Apply lighting effects to model
	void			( *StudioDynamicLight )			( struct cl_entity_s *ent, struct alight_s *plight );
	void			( *StudioEntityLight )			( struct alight_s *plight );
	void			( *StudioSetupLighting )		( struct alight_s *plighting );

	// Draw mesh vertices
	void			( *StudioDrawPoints )			( void );

	// Draw hulls around bones
	void			( *StudioDrawHulls )			( void );
	// Draw bbox around studio models
	void			( *StudioDrawAbsBBox )			( void );
	// Draws bones
	void			( *StudioDrawBones )			( void );
	// Loads in appropriate texture for model
	void			( *StudioSetupSkin )			( void *ptexturehdr, int index );
	// Sets up for remapped colors
	void			( *StudioSetRemapColors )		( int top, int bottom );
	// Set's player model and returns model pointer
	struct model_s *( *SetupPlayerModel )			( int index );
	// Fires any events embedded in animation
	void			( *StudioClientEvents )			( void );
	// Retrieve/set forced render effects flags
	int				( *GetForceFaceFlags )			( void );
	void			( *SetForceFaceFlags )			( int flags );
	// Tell engine the value of the studio model header
	void			( *StudioSetHeader )			( void *header );
	// Tell engine which model_t * is being renderered
	void			( *SetRenderModel )				( struct model_s *model );

	// Final state setup and restore for rendering
	void			( *SetupRenderer )				( int rendermode );
	void			( *RestoreRenderer )			( void );

	// Set render origin for applying chrome effect
	void			( *SetChromeOrigin )			( void );

	// True if using D3D/OpenGL
	int				( *IsHardware )					( void );

	// Only called by hardware interface
	void			( *GL_StudioDrawShadow )		( void );
	void			( *GL_SetRenderMode )			( int mode );

	void			( *StudioSetRenderamt )			( int iRenderamt ); 	//!!!CZERO added for rendering glass on viewmodels
	void			( *StudioSetCullState )			( int iCull );
	void			( *StudioRenderShadow )			( int iSprite, float *p1, float *p2, float *p3, float *p4 );

	// FIXME: clarify purpose
	void ( *StudioBBox )( void );
} engine_studio_api_t;

typedef struct server_studio_api_s
{
	// Allocate number*size bytes and zero it
	void *( *Mem_Calloc )				( int number, size_t size );
	// Check to see if pointer is in the cache
	void *( *Cache_Check )				( struct cache_user_s *c );
	// Load file into cache ( can be swapped out on demand )
	void			( *LoadCacheFile )				( char *path, struct cache_user_s *cu );
	// Retrieve pointer to studio model data block from a model
	void *( *Mod_Extradata )				( struct model_s *mod );
} server_studio_api_t;

#define STUDIO_RENDER 1
#define STUDIO_EVENTS 2

#define MAX_CLIENTS			32
#define	MAX_EDICTS			8192 // @Sw1ft: Sven Co-op specific
//#define	MAX_EDICTS			900

#define MAX_MODEL_NAME		64
#define MAX_MAP_HULLS		4
#define	MIPLEVELS			4
#define	NUM_AMBIENTS		4		// automatic ambient sounds
#define	MAXLIGHTMAPS		4
#define	PLANE_ANYZ			5

#define ALIAS_Z_CLIP_PLANE	5

// flags in finalvert_t.flags
#define ALIAS_LEFT_CLIP				0x0001
#define ALIAS_TOP_CLIP				0x0002
#define ALIAS_RIGHT_CLIP			0x0004
#define ALIAS_BOTTOM_CLIP			0x0008
#define ALIAS_Z_CLIP				0x0010
#define ALIAS_ONSEAM				0x0020
#define ALIAS_XY_CLIP_MASK			0x000F

#define	ZISCALE	((float)0x8000)

#define CACHE_SIZE	32		// used to align key data structures

typedef enum
{
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_studio
} modtype_t;

// must match definition in modelgen.h
#ifndef SYNCTYPE_T
#define SYNCTYPE_T

typedef enum
{
	ST_SYNC = 0,
	ST_RAND
} synctype_t;

#endif

typedef struct
{
	float		mins[ 3 ], maxs[ 3 ];
	float		origin[ 3 ];
	int			headnode[ MAX_MAP_HULLS ];
	int			visleafs;		// not including the solid leaf 0
	int			firstface, numfaces;
} dmodel_t;

// plane_t structure
typedef struct mplane_s
{
	vec3_t	normal;			// surface normal
	float	dist;			// closest appoach to origin
	byte	type;			// for texture axis selection and fast side tests
	byte	signbits;		// signx + signy<<1 + signz<<1
	byte	pad[ 2 ];
} mplane_t;

typedef struct
{
	vec3_t		position;
} mvertex_t;

typedef struct
{
	unsigned short	v[ 2 ];
	unsigned int	cachededgeoffset;
} medge_t;

typedef struct texture_s
{
	char		name[ 16 ];
	unsigned	width, height;
	int			anim_total;				// total tenths in sequence ( 0 = no)
	int			anim_min, anim_max;		// time for this frame min <=time< max
	struct texture_s *anim_next;		// in the animation sequence
	struct texture_s *alternate_anims;	// bmodels in frame 1 use these
	unsigned	offsets[ MIPLEVELS ];		// four mip maps stored
	unsigned	paloffset;
} texture_t;

typedef struct
{
	float		vecs[ 2 ][ 4 ];		// [s/t] unit vectors in world space. 
	// [i][3] is the s/t offset relative to the origin.
	// s or t = dot(3Dpoint,vecs[i])+vecs[i][3]
	float		mipadjust;		// ?? mipmap limits for very small surfaces
	texture_t *texture;
	int			flags;			// sky or slime, no lightmap or 256 subdivision
} mtexinfo_t;

typedef struct mnode_s
{
	// common with leaf
	int			contents;		// 0, to differentiate from leafs
	int			visframe;		// node needs to be traversed if current

	float		minmaxs[ 6 ];		// for bounding box culling

	struct mnode_s *parent;

	// node specific
	mplane_t *plane;
	struct mnode_s *children[ 2 ];

	unsigned short		firstsurface;
	unsigned short		numsurfaces;
} mnode_t;

typedef struct msurface_s	msurface_t;
typedef struct decal_s		decal_t;

// JAY: Compress this as much as possible
struct decal_s
{
	decal_t *pnext;			// linked list for each surface
	msurface_t *psurface;		// Surface id for persistence / unlinking
	short		dx;				// Offsets into surface texture (in texture coordinates, so we don't need floats)
	short		dy;
	short		texture;		// Decal texture
	byte		scale;			// Pixel scale
	byte		flags;			// Decal flags

	short		entityIndex;	// Entity this is attached to
};

typedef struct mleaf_s
{
	// common with node
	int			contents;		// wil be a negative contents number
	int			visframe;		// node needs to be traversed if current

	short		minmaxs[ 6 ];		// for bounding box culling

	struct mnode_s *parent;

	// leaf specific
	byte *compressed_vis;
	struct efrag_s *efrags;

	msurface_t **firstmarksurface;
	int			nummarksurfaces;
	int			key;			// BSP sequence number for leaf's contents
	byte		ambient_sound_level[ NUM_AMBIENTS ];
} mleaf_t;

/*struct msurface_s
{
	int			visframe;		// should be drawn when node is crossed

	int			dlightframe;	// last frame the surface was checked by an animated light
	int			dlightbits;		// dynamically generated. Indicates if the surface illumination
								// is modified by an animated light.

	mplane_t	*plane;			// pointer to shared plane
	int			flags;			// see SURF_ #defines

	int			firstedge;	// look up in model->surfedges[], negative numbers
	int			numedges;	// are backwards edges

// surface generation data
	struct surfcache_s	*cachespots[MIPLEVELS];

	short		texturemins[2]; // smallest s/t position on the surface.
	short		extents[2];		// ?? s/t texture size, 1..256 for all non-sky surfaces

	mtexinfo_t	*texinfo;

// lighting info
	byte		styles[MAXLIGHTMAPS]; // index into d_lightstylevalue[] for animated lights
									  // no one surface can be effected by more than 4
									  // animated lights.
	color24		*samples;

	decal_t		*pdecals;
};*/

#define VERTEXSIZE 7

typedef struct glpoly_s
{
	struct glpoly_s *next;
	struct glpoly_s *chain;
	int numverts;
	int flags;
	float verts[ 4 ][ VERTEXSIZE ];
}glpoly_t;

typedef struct msurface_s
{
	int visframe;
	mplane_t *plane;
	int flags;
	int firstedge;
	int numedges;
	short texturemins[ 2 ];
	short extents[ 2 ];
	int light_s, light_t;
	glpoly_t *polys;
	struct msurface_s *texturechain;
	mtexinfo_t *texinfo;
	int dlightframe;
	int dlightbits;
	int lightmaptexturenum;
	byte styles[ MAXLIGHTMAPS ];
	int cached_light[ MAXLIGHTMAPS ];
	qboolean cached_dlight;
	byte *samples;
	struct decal_s *pdecals;
}msurface_t;

typedef struct
{
	int			planenum;
	short		children[ 2 ];	// negative numbers are contents
} dclipnode_t;

typedef struct hull_s
{
	dclipnode_t *clipnodes;
	mplane_t *planes;
	int			firstclipnode;
	int			lastclipnode;
	vec3_t		clip_mins;
	vec3_t		clip_maxs;
} hull_t;

#if !defined( CACHE_USER ) && !defined( QUAKEDEF_H )
#define CACHE_USER
typedef struct cache_user_s
{
	void *data;
} cache_user_t;
#endif

typedef struct model_s
{
	char		name[ MAX_MODEL_NAME ];
	qboolean	needload;		// bmodels and sprites don't cache normally

	modtype_t	type;
	int			numframes;
	synctype_t	synctype;

	int			flags;

	//
	// volume occupied by the model
	//		
	vec3_t		mins, maxs;
	float		radius;

	//
	// brush model
	//
	int			firstmodelsurface, nummodelsurfaces;

	int			numsubmodels;
	dmodel_t *submodels;

	int			numplanes;
	mplane_t *planes;

	int			numleafs;		// number of visible leafs, not counting 0
	struct mleaf_s *leafs;

	int			numvertexes;
	mvertex_t *vertexes;

	int			numedges;
	medge_t *edges;

	int			numnodes;
	mnode_t *nodes;

	int			numtexinfo;
	mtexinfo_t *texinfo;

	int			numsurfaces;
	msurface_t *surfaces;

	int			numsurfedges;
	int *surfedges;

	int			numclipnodes;
	dclipnode_t *clipnodes;

	int			nummarksurfaces;
	msurface_t **marksurfaces;

	hull_t		hulls[ MAX_MAP_HULLS ];

	int			numtextures;
	texture_t **textures;

	byte *visdata;

	color24 *lightdata;

	char *entities;

	//
	// additional model data
	//
	cache_user_t	cache;		// only access through Mod_Extradata

} model_t;

typedef vec_t vec4_t[ 4 ];

typedef struct alight_s
{
	int			ambientlight;	// clip at 128
	int			shadelight;		// clip at 192 - ambientlight
	vec3_t		color;
	float *plightvec;
} alight_t;

typedef struct auxvert_s
{
	float	fv[ 3 ];		// viewspace x, y
} auxvert_t;

/* BSP File Structs
* 
*/

#define MAX_MAP_MODELS 400
#define MAX_MAP_BRUSHES 4096
#define MAX_MAP_ENTITIES 1024
#define MAX_MAP_ENTSTRING (128 * 1024)

#define MAX_MAP_PLANES 32767
#define MAX_MAP_NODES 32767		// because negative shorts are contents
#define MAX_MAP_CLIPNODES 32767 //
#define MAX_MAP_LEAFS 8192
#define MAX_MAP_VERTS 65535
#define MAX_MAP_FACES 65535
#define MAX_MAP_MARKSURFACES 65535
#define MAX_MAP_TEXINFO 8192
#define MAX_MAP_EDGES 256000
#define MAX_MAP_SURFEDGES 512000
#define MAX_MAP_TEXTURES 512
#define MAX_MAP_MIPTEX 0x200000
#define MAX_MAP_LIGHTING 0x200000
#define MAX_MAP_VISIBILITY 0x200000

#define MAX_MAP_PORTALS 65536

// key / value pair sizes

#define MAX_KEY 32
#define MAX_VALUE 1024

//=============================================================================


#define BSPVERSION 30
#define TOOLVERSION 2


typedef struct
{
	int fileofs, filelen;
} lump_t;

#define LUMP_ENTITIES 0
#define LUMP_PLANES 1
#define LUMP_TEXTURES 2
#define LUMP_VERTEXES 3
#define LUMP_VISIBILITY 4
#define LUMP_NODES 5
#define LUMP_TEXINFO 6
#define LUMP_FACES 7
#define LUMP_LIGHTING 8
#define LUMP_CLIPNODES 9
#define LUMP_LEAFS 10
#define LUMP_MARKSURFACES 11
#define LUMP_EDGES 12
#define LUMP_SURFEDGES 13
#define LUMP_MODELS 14

#define HEADER_LUMPS 15

typedef struct
{
	int version;
	lump_t lumps[ HEADER_LUMPS ];
} dheader_t;

typedef struct
{
	int nummiptex;
	int dataofs[ 4 ]; // [nummiptex]
} dmiptexlump_t;

typedef struct miptex_s
{
	char name[ 16 ];
	unsigned width, height;
	unsigned offsets[ MIPLEVELS ]; // four mip maps stored
} miptex_t;


typedef struct
{
	float point[ 3 ];
} dvertex_t;


// 0-2 are axial planes
#define PLANE_X 0
#define PLANE_Y 1
#define PLANE_Z 2

// 3-5 are non-axial planes snapped to the nearest
#define PLANE_ANYX 3
#define PLANE_ANYY 4
#define PLANE_ANYZ 5

typedef struct
{
	float normal[ 3 ];
	float dist;
	int type; // PLANE_X - PLANE_ANYZ ?remove? trivial to regenerate
} dplane_t;

// !!! if this is changed, it must be changed in asm_i386.h too !!!
typedef struct
{
	int planenum;
	short children[ 2 ]; // negative numbers are -(leafs+1), not nodes
	short mins[ 3 ];	   // for sphere culling
	short maxs[ 3 ];
	unsigned short firstface;
	unsigned short numfaces; // counting both sides
} dnode_t;

/*
typedef struct texinfo_s
{
	float vecs[ 2 ][ 4 ]; // [s/t][xyz offset]
	int miptex;
	int flags;
} texinfo_t;
*/
#define TEX_SPECIAL 1 // sky or slime, no lightmap or 256 subdivision

// note that edge 0 is never used, because negative edge nums are used for
// counterclockwise use of the edge in a face
typedef struct
{
	unsigned short v[ 2 ]; // vertex numbers
} dedge_t;

#define MAXLIGHTMAPS 4
typedef struct
{
	short planenum;
	short side;

	int firstedge; // we must support > 64k edges
	short numedges;
	short texinfo;

	// lighting info
	byte styles[ MAXLIGHTMAPS ];
	int lightofs; // start of [numstyles*surfsize] samples
} dface_t;

#define AMBIENT_WATER 0
#define AMBIENT_SKY 1
#define AMBIENT_SLIME 2
#define AMBIENT_LAVA 3

// leaf 0 is the generic CONTENTS_SOLID leaf, used for all solid areas
// all other leafs need visibility info
typedef struct
{
	int contents;
	int visofs; // -1 = no visibility info

	short mins[ 3 ]; // for frustum culling
	short maxs[ 3 ];

	unsigned short firstmarksurface;
	unsigned short nummarksurfaces;

	byte ambient_level[ NUM_AMBIENTS ];
} dleaf_t;

// client blending
typedef struct r_studio_interface_s
{
	int				version;
	int				( *StudioDrawModel )			( int flags );
	int				( *StudioDrawPlayer )			( int flags, struct entity_state_s *pplayer );
} r_studio_interface_t;

extern r_studio_interface_t *pStudioAPI;

// server blending
#define SV_BLENDING_INTERFACE_VERSION 1

typedef struct sv_blending_interface_s
{
	int				version;

	void			( *SV_StudioSetupBones )		( struct model_s *pModel,
													  float				frame,
													  int				sequence,
													  const vec3_t		angles,
													  const	vec3_t		origin,
													  const	byte *pcontroller,
													  const	byte *pblending,
													  int				iBone,
													  const edict_t *pEdict );
} sv_blending_interface_t;

// ********************************************************
// Functions exported by the engine
// ********************************************************

// Function type declarations for engine exports
typedef HSPRITE_HL( *pfnEngSrc_pfnSPR_Load_t )			( const char *szPicName );
typedef int							( *pfnEngSrc_pfnSPR_Frames_t )			( HSPRITE_HL hPic );
typedef int							( *pfnEngSrc_pfnSPR_Height_t )			( HSPRITE_HL hPic, int frame );
typedef int							( *pfnEngSrc_pfnSPR_Width_t )			( HSPRITE_HL hPic, int frame );
typedef void						( *pfnEngSrc_pfnSPR_Set_t )				( HSPRITE_HL hPic, int r, int g, int b );
typedef void						( *pfnEngSrc_pfnSPR_Draw_t )			( int frame, int x, int y, const struct rect_s *prc );
typedef void						( *pfnEngSrc_pfnSPR_DrawHoles_t )		( int frame, int x, int y, const struct rect_s *prc );
typedef void						( *pfnEngSrc_pfnSPR_DrawAdditive_t )	( int frame, int x, int y, const struct rect_s *prc );
typedef void						( *pfnEngSrc_pfnSPR_EnableScissor_t )	( int x, int y, int width, int height );
typedef void						( *pfnEngSrc_pfnSPR_DisableScissor_t )	( void );
typedef struct client_sprite_s *( *pfnEngSrc_pfnSPR_GetList_t )			( char *psz, int *piCount );
typedef void						( *pfnEngSrc_pfnFillRGBA_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int							( *pfnEngSrc_pfnGetScreenInfo_t ) 		( struct SCREENINFO_s *pscrinfo );
typedef void						( *pfnEngSrc_pfnSetCrosshair_t )		( HSPRITE_HL hspr, wrect_t rc, int r, int g, int b );
typedef struct cvar_s *( *pfnEngSrc_pfnRegisterVariable_t )	( char *szName, char *szValue, int flags );
typedef float						( *pfnEngSrc_pfnGetCvarFloat_t )		( char *szName );
typedef char *( *pfnEngSrc_pfnGetCvarString_t )		( char *szName );
typedef int							( *pfnEngSrc_pfnAddCommand_t )			( const char *cmd_name, void ( *pfnEngSrc_function )( void ) );
typedef int							( *pfnEngSrc_pfnHookUserMsg_t )			( const char *szMsgName, pfnUserMsgHook pfn );
typedef int							( *pfnEngSrc_pfnServerCmd_t )			( const char *szCmdString );
typedef int							( *pfnEngSrc_pfnClientCmd_t )			( const char *szCmdString );
typedef void						( *pfnEngSrc_pfnPrimeMusicStream_t )	( char *szFilename, int looping );
typedef void						( *pfnEngSrc_pfnGetPlayerInfo_t )		( int ent_num, struct hud_player_info_s *pinfo );
typedef void						( *pfnEngSrc_pfnPlaySoundByName_t )		( const char *szSound, float volume );
typedef void						( *pfnEngSrc_pfnPlaySoundByNameAtPitch_t )	( char *szSound, float volume, int pitch );
typedef void						( *pfnEngSrc_pfnPlaySoundVoiceByName_t )		( char *szSound, float volume, int pitch );
typedef void						( *pfnEngSrc_pfnPlaySoundByIndex_t )	( int iSound, float volume );
typedef void						( *pfnEngSrc_pfnAngleVectors_t )		( const float *vecAngles, float *forward, float *right, float *up );
typedef struct client_textmessage_s *( *pfnEngSrc_pfnTextMessageGet_t )		( const char *pName );
typedef int							( *pfnEngSrc_pfnDrawCharacter_t )		( int x, int y, int number, int r, int g, int b );
typedef int							( *pfnEngSrc_pfnDrawConsoleString_t )	( int x, int y, char *string );
typedef void						( *pfnEngSrc_pfnDrawSetTextColor_t )	( float r, float g, float b );
typedef void						( *pfnEngSrc_pfnDrawConsoleStringLen_t )( const char *string, int *length, int *height );
typedef void						( *pfnEngSrc_pfnConsolePrint_t )		( const char *string );
typedef void						( *pfnEngSrc_pfnCenterPrint_t )			( const char *string );
typedef int							( *pfnEngSrc_GetWindowCenterX_t )		( void );
typedef int							( *pfnEngSrc_GetWindowCenterY_t )		( void );
typedef void						( *pfnEngSrc_GetViewAngles_t )			( float * );
typedef void						( *pfnEngSrc_SetViewAngles_t )			( float * );
typedef int							( *pfnEngSrc_GetMaxClients_t )			( void );
typedef void						( *pfnEngSrc_Cvar_SetValue_t )			( char *cvar, float value );
typedef int       					( *pfnEngSrc_Cmd_Argc_t )					( void );
typedef char *( *pfnEngSrc_Cmd_Argv_t )				( int arg );
typedef void						( *pfnEngSrc_Con_Printf_t )				( char *fmt, ... );
typedef void						( *pfnEngSrc_Con_DPrintf_t )			( char *fmt, ... );
typedef void						( *pfnEngSrc_Con_NPrintf_t )			( int pos, char *fmt, ... );
typedef void						( *pfnEngSrc_Con_NXPrintf_t )			( struct con_nprint_s *info, char *fmt, ... );
typedef const char *( *pfnEngSrc_PhysInfo_ValueForKey_t )	( const char *key );
typedef const char *( *pfnEngSrc_ServerInfo_ValueForKey_t )( const char *key );
typedef float						( *pfnEngSrc_GetClientMaxspeed_t )		( void );
typedef int							( *pfnEngSrc_CheckParm_t )				( char *parm, char **ppnext );
typedef void						( *pfnEngSrc_Key_Event_t )				( int key, int down );
typedef void						( *pfnEngSrc_GetMousePosition_t )		( int *mx, int *my );
typedef int							( *pfnEngSrc_IsNoClipping_t )			( void );
typedef struct cl_entity_s *( *pfnEngSrc_GetLocalPlayer_t )		( void );
typedef struct cl_entity_s *( *pfnEngSrc_GetViewModel_t )			( void );
typedef struct cl_entity_s *( *pfnEngSrc_GetEntityByIndex_t )		( int idx );
typedef float						( *pfnEngSrc_GetClientTime_t )			( void );
typedef void						( *pfnEngSrc_V_CalcShake_t )			( void );
typedef void						( *pfnEngSrc_V_ApplyShake_t )			( float *origin, float *angles, float factor );
typedef int							( *pfnEngSrc_PM_PointContents_t )		( float *point, int *truecontents );
typedef int							( *pfnEngSrc_PM_WaterEntity_t )			( float *p );
typedef struct pmtrace_s *( *pfnEngSrc_PM_TraceLine_t )			( float *start, float *end, int flags, int usehull, int ignore_pe );
typedef struct model_s *( *pfnEngSrc_CL_LoadModel_t )			( const char *modelname, int *index );
typedef int							( *pfnEngSrc_CL_CreateVisibleEntity_t )	( int type, struct cl_entity_s *ent );
typedef const struct model_s *( *pfnEngSrc_GetSpritePointer_t )		( HSPRITE_HL HSPRITE_HL );
typedef void						( *pfnEngSrc_pfnPlaySoundByNameAtLocation_t )	( char *szSound, float volume, float *origin );
typedef unsigned short				( *pfnEngSrc_pfnPrecacheEvent_t )		( int type, const char *psz );
typedef void						( *pfnEngSrc_pfnPlaybackEvent_t )		( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
typedef void						( *pfnEngSrc_pfnWeaponAnim_t )			( int iAnim, int body );
typedef float						( *pfnEngSrc_pfnRandomFloat_t )			( float flLow, float flHigh );
typedef int32( *pfnEngSrc_pfnRandomLong_t )			( int32 lLow, int32 lHigh );
typedef void						( *pfnEngSrc_pfnHookEvent_t )			( char *name, void ( *pfnEvent )( struct event_args_s *args ) );
typedef int							( *pfnEngSrc_Con_IsVisible_t )			( );
typedef const char *( *pfnEngSrc_pfnGetGameDirectory_t )	( void );
typedef struct cvar_s *( *pfnEngSrc_pfnGetCvarPointer_t )		( const char *szName );
typedef const char *( *pfnEngSrc_Key_LookupBinding_t )		( const char *pBinding );
typedef const char *( *pfnEngSrc_pfnGetLevelName_t )		( void );
typedef void						( *pfnEngSrc_pfnGetScreenFade_t )		( struct screenfade_s *fade );
typedef void						( *pfnEngSrc_pfnSetScreenFade_t )		( struct screenfade_s *fade );
typedef void *( *pfnEngSrc_VGui_GetPanel_t )         ( );
typedef void                        ( *pfnEngSrc_VGui_ViewportPaintBackground_t ) ( int extents[ 4 ] );
typedef byte *( *pfnEngSrc_COM_LoadFile_t )				( char *path, int usehunk, int *pLength );
typedef char *( *pfnEngSrc_COM_ParseFile_t )			( char *data, char *token );
typedef void						( *pfnEngSrc_COM_FreeFile_t )				( void *buffer );
typedef struct triangleapi_s *pTriAPI;
typedef struct efx_api_s *pEfxAPI;
typedef struct event_api_s *pEventAPI;
typedef struct demo_api_s *pDemoAPI;
typedef struct net_api_s *pNetAPI;
typedef struct IVoiceTweak_s *pVoiceTweak;
typedef int							( *pfnEngSrc_IsSpectateOnly_t ) ( void );
typedef struct model_s *( *pfnEngSrc_LoadMapSprite_t )			( const char *filename );
typedef void						( *pfnEngSrc_COM_AddAppDirectoryToSearchPath_t ) ( const char *pszBaseDir, const char *appName );
typedef int							( *pfnEngSrc_COM_ExpandFilename_t )				 ( const char *fileName, char *nameOutBuffer, int nameOutBufferSize );
typedef const char *( *pfnEngSrc_PlayerInfo_ValueForKey_t )( int playerNum, const char *key );
typedef void						( *pfnEngSrc_PlayerInfo_SetValueForKey_t )( const char *key, const char *value );
typedef qboolean( *pfnEngSrc_GetPlayerUniqueID_t )( int iPlayer, char playerID[ 16 ] );
typedef int							( *pfnEngSrc_GetTrackerIDForPlayer_t )( int playerSlot );
typedef int							( *pfnEngSrc_GetPlayerForTrackerID_t )( int trackerID );
typedef int							( *pfnEngSrc_pfnServerCmdUnreliable_t )( char *szCmdString );
typedef void						( *pfnEngSrc_GetMousePos_t )( struct tagPOINT *ppt );
typedef void						( *pfnEngSrc_SetMousePos_t )( int x, int y );
typedef void						( *pfnEngSrc_SetMouseEnable_t )( qboolean fEnable );
typedef struct cvar_s *( *pfnEngSrc_GetFirstCVarPtr_t )( );
typedef unsigned int				( *pfnEngSrc_GetFirstCmdFunctionHandle_t )( );
typedef unsigned int				( *pfnEngSrc_GetNextCmdFunctionHandle_t )( unsigned int cmdhandle );
typedef const char *( *pfnEngSrc_GetCmdFunctionName_t )( unsigned int cmdhandle );
typedef float						( *pfnEngSrc_GetClientOldTime_t )( );
typedef float						( *pfnEngSrc_GetServerGravityValue_t )( );
typedef struct model_s *( *pfnEngSrc_GetModelByIndex_t )( int index );
typedef void						( *pfnEngSrc_pfnSetFilterMode_t )( int mode );
typedef void						( *pfnEngSrc_pfnSetFilterColor_t )( float r, float g, float b );
typedef void						( *pfnEngSrc_pfnSetFilterBrightness_t )( float brightness );
typedef sequenceEntry_s *( *pfnEngSrc_pfnSequenceGet_t )( const char *fileName, const char *entryName );
typedef void						( *pfnEngSrc_pfnSPR_DrawGeneric_t )( int frame, int x, int y, const struct rect_s *prc, int src, int dest, int w, int h );
typedef sentenceEntry_s *( *pfnEngSrc_pfnSequencePickSentence_t )( const char *sentenceName, int pickMethod, int *entryPicked );
// draw a complete string
typedef int							( *pfnEngSrc_pfnDrawString_t )		( int x, int y, const char *str, int r, int g, int b );
typedef int							( *pfnEngSrc_pfnDrawStringReverse_t )		( int x, int y, const char *str, int r, int g, int b );
typedef const char *( *pfnEngSrc_LocalPlayerInfo_ValueForKey_t )( const char *key );
typedef int							( *pfnEngSrc_pfnVGUI2DrawCharacter_t )		( int x, int y, int ch, unsigned int font );
typedef int							( *pfnEngSrc_pfnVGUI2DrawCharacterAdd_t )	( int x, int y, int ch, int r, int g, int b, unsigned int font );
typedef unsigned int		( *pfnEngSrc_COM_GetApproxWavePlayLength ) ( const char *filename );
typedef void *( *pfnEngSrc_pfnGetCareerUI_t )( );
typedef void						( *pfnEngSrc_Cvar_Set_t )			( char *cvar, char *value );
typedef int							( *pfnEngSrc_pfnIsPlayingCareerMatch_t )( );
typedef double						( *pfnEngSrc_GetAbsoluteTime_t ) ( void );
typedef void						( *pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t )( int *buffer, int bufferLength );
typedef void						( *pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t )( int *buffer, int bufferLength );
typedef void						( *pfnEngSrc_pfnResetTutorMessageDecayData_t )( );
typedef void						( *pfnEngSrc_pfnFillRGBABlend_t )			( int x, int y, int width, int height, int r, int g, int b, int a );
typedef int						( *pfnEngSrc_pfnGetAppID_t )			( void );
typedef cmdalias_t *( *pfnEngSrc_pfnGetAliases_t )		( void );
typedef void					( *pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t ) ( int *x, int *y );

typedef enum
{
	TRI_FRONT = 0,
	TRI_NONE = 1,
} TRICULLSTYLE;

#define TRI_API_VERSION 1

#define TRI_TRIANGLES 0
#define TRI_TRIANGLE_FAN 1
#define TRI_QUADS 2
#define TRI_POLYGON 3
#define TRI_LINES 4
#define TRI_TRIANGLE_STRIP 5
#define TRI_QUAD_STRIP 6

typedef struct triangleapi_s
{
	int	version;

	void	( *RenderMode )( int mode );
	void	( *Begin )( int primitiveCode );
	void	( *End )( void );

	void	( *Color4f )( float r, float g, float b, float a );
	void	( *Color4ub )( unsigned char r, unsigned char g, unsigned char b, unsigned char a );
	void	( *TexCoord2f )( float u, float v );
	void	( *Vertex3fv )( const float *worldPnt );
	void	( *Vertex3f )( float x, float y, float z );
	void	( *Brightness )( float brightness );
	void	( *CullFace )( TRICULLSTYLE style );
	int	( *SpriteTexture )( struct model_s *pSpriteModel, int frame );
	int	( *WorldToScreen )( const float *world, float *screen );  // Returns 1 if it's z clipped
	void	( *Fog )( float flFogColor[ 3 ], float flStart, float flEnd, int bOn ); //Works just like GL_FOG, flFogColor is r/g/b.
	void	( *ScreenToWorld )( const float *screen, float *world );
	void	( *GetMatrix )( const int pname, float *matrix );
	int	( *BoxInPVS )( float *mins, float *maxs );
	void	( *LightAtPoint )( float *pos, float *value );
	void	( *Color4fRendermode )( float r, float g, float b, float a, int rendermode );
	void	( *FogParams )( float flDensity, int iFogSkybox );
} triangleapi_t;

typedef struct demo_api_s
{
	int ( *IsRecording )( void );
	int ( *IsPlayingback )( void );
	int ( *IsTimeDemo )( void );
	void ( *WriteClientDLLMessage )( int size, unsigned char *buffer ); // WriteBuffer
} demo_api_t;

// These provide access to the voice controls.
typedef enum
{
	MicrophoneVolume = 0, // values 0-1.
	OtherSpeakerScale,	  // values 0-1. Scales how loud other players are.
	MicBoost,			  // 20 db gain to voice input
} VoiceTweakControl;

typedef struct IVoiceTweak_s
{
	// These turn voice tweak mode on and off. While in voice tweak mode, the user's voice is echoed back
	// without sending to the server.
	int ( *StartVoiceTweakMode )( ); // Returns 0 on error.
	void ( *EndVoiceTweakMode )( );

	// Get/set control values.
	void ( *SetControlFloat )( VoiceTweakControl iControl, float value );
	float ( *GetControlFloat )( VoiceTweakControl iControl );

	int ( *GetSpeakingVolume )( );
} IVoiceTweak;

// Particles
typedef enum
{
	pt_static,
	pt_grav,
	pt_slowgrav,
	pt_fire,
	pt_explode,
	pt_explode2,
	pt_blob,
	pt_blob2,
	pt_vox_slowgrav,
	pt_vox_grav,
	pt_clientcustom // Must have callback function specified
} ptype_t;

// !!! if this is changed, it must be changed in d_ifacea.h too !!!
typedef struct particle_s
{
	// driver-usable fields
	Vector org;
	short color;
	short packedColor;
	// drivers never touch the following fields
	struct particle_s *next;
	Vector vel;
	float ramp;
	float die;
	ptype_t type;
	void ( *deathfunc )( struct particle_s *particle );

	// for pt_clientcusttom, we'll call this function each frame
	void ( *callback )( struct particle_s *particle, float frametime );

	// For deathfunc, etc.
	unsigned char context;
} particle_t;

// Beam
#define FBEAM_STARTENTITY 0x00000001
#define FBEAM_ENDENTITY 0x00000002
#define FBEAM_FADEIN 0x00000004
#define FBEAM_FADEOUT 0x00000008
#define FBEAM_SINENOISE 0x00000010
#define FBEAM_SOLID 0x00000020
#define FBEAM_SHADEIN 0x00000040
#define FBEAM_SHADEOUT 0x00000080
#define FBEAM_STARTVISIBLE 0x10000000 // Has this client actually seen this beam's start entity yet?
#define FBEAM_ENDVISIBLE 0x20000000	  // Has this client actually seen this beam's end entity yet?
#define FBEAM_ISACTIVE 0x40000000
#define FBEAM_FOREVER 0x80000000

typedef struct beam_s BEAM;
struct beam_s
{
	BEAM *next;
	int type;
	int flags;
	Vector source;
	Vector target;
	Vector delta;
	float t; // 0 .. 1 over lifetime of beam
	float freq;
	float die;
	float width;
	float amplitude;
	float r, g, b;
	float brightness;
	float speed;
	float frameRate;
	float frame;
	int segments;
	int startEntity;
	int endEntity;
	int modelIndex;
	int frameCount;
	struct model_s *pFollowModel;
	struct particle_s *particles;
};

/* ENGINE STRUCTS GOING FROM HERE - xWhitey */
struct sfx_t
{
	char name[ 64 ];
	cache_user_t cache;
	int servercount;
};

struct channel_t
{
	sfx_t *sfx;
	int leftvol;
	int rightvol;
	int end;
	int pos;
	int looping;
	int entnum;
	int entchannel;
	vec_t origin[ 3 ];
	vec_t dist_mult;
	int master_vol;
	int isentence;
	int iword;
	int pitch;
};

#define SIZEBUF_CHECK_OVERFLOW	0
#define SIZEBUF_ALLOW_OVERFLOW	1
#define SIZEBUF_OVERFLOWED		2

typedef struct sizebuf_s
{
	const char *buffername;
	unsigned short flags;
	unsigned char *data;
	int maxsize;
	int cursize;
} sizebuf_t;

#define MAX_DATAGRAM 4000
#define MAX_DEMOS 32
#define MAX_LATENT 32
#define MAX_MSGLEN 3990
#define FRAGMENT_MAX_SIZE 1400
#define MAX_FRAGMENTS 25000

typedef struct flowstats_s
{
	// Size of message sent/received
	int size;
	// Time that message was sent/received
	double time;
} flowstats_t;

typedef struct flow_s
{
	// Data for last MAX_LATENT messages
	flowstats_t stats[ MAX_LATENT ];
	// Current message position
	int current;
	// Time when we should recompute k/sec data
	double nextcompute;
	// Average data
	float kbytespersec;
	float avgkbytespersec;
} flow_t;

enum
{
	FRAG_NORMAL_STREAM = 0,
	FRAG_FILE_STREAM,

	MAX_STREAMS
};

typedef enum cactive_e
{
	ca_dedicated,
	ca_disconnected,
	ca_connecting,
	ca_connected,
	ca_uninitialized,
	ca_active,
} cactive_t;

typedef struct downloadtime_s
{
	qboolean bUsed;
	float fTime;
	int nBytesRemaining;
} downloadtime_t;

typedef struct incomingtransfer_s
{
	qboolean doneregistering;
	int percent;
	qboolean downloadrequested;
	downloadtime_t rgStats[ 8 ];
	int nCurStat;
	int nTotalSize;
	int nTotalToTransfer;
	int nRemainingToTransfer;
	float fLastStatusUpdate;
	qboolean custom;
} incomingtransfer_t;

#define MAKE_FRAGID(id, count)	(((id & 0xffff) << 16) | (count & 0xffff))
#define FRAG_GETID(fragid)		((fragid >> 16) & 0xffff)
#define FRAG_GETCOUNT(fragid)	(fragid & 0xffff)

// Generic fragment structure
typedef struct fragbuf_s
{
	// Next buffer in chain
	fragbuf_s *next;
	// Id of this buffer
	int bufferid;
	// Message buffer where raw data is stored
	sizebuf_t frag_message;
	// The actual data sits here
	byte frag_message_buf[ FRAGMENT_MAX_SIZE ];
	// Is this a file buffer?
	qboolean isfile;
	// Is this file buffer from memory ( custom decal, etc. ).
	qboolean isbuffer;
	qboolean iscompressed;
	// Name of the file to save out on remote host
	char filename[ MAX_PATH ];
	// Offset in file from which to read data
	int foffset;
	// Size of data to read at that offset
	int size;
} fragbuf_t;

// Waiting list of fragbuf chains
typedef struct fragbufwaiting_s
{
	// Next chain in waiting list
	fragbufwaiting_s *next;
	// Number of buffers in this chain
	int fragbufcount;
	// The actual buffers
	fragbuf_t *fragbufs;
} fragbufwaiting_t;

enum
{
	FLOW_OUTGOING = 0,
	FLOW_INCOMING,

	MAX_FLOWS
};

typedef enum netsrc_s
{
	NS_CLIENT = 0,
	NS_SERVER,
	NS_MULTICAST,	// xxxMO
	NS_MAX
} netsrc_t;

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX,
} netadrtype_t;

typedef struct netadr_s
{
	netadrtype_t	type;
	unsigned char	ip[ 4 ];
	unsigned char	ipx[ 26 ]; // @Sw1ft: Sven Co-op specific. 10 for HL
	unsigned short	port;
} netadr_t;

// Network Connection Channel
typedef struct netchan_s
{
	// NS_SERVER or NS_CLIENT, depending on channel.
	netsrc_t sock;

	// Address this channel is talking to.
	netadr_t remote_address;

	int player_slot;
	// For timeouts.  Time last message was received.
	float last_received;
	// Time when channel was connected.
	float connect_time;

	// Bandwidth choke
	// Bytes per second
	double rate;
	// If realtime > cleartime, free to send next packet
	double cleartime;

	// Sequencing variables
	//
	// Increasing count of sequence numbers
	int incoming_sequence;
	// # of last outgoing message that has been ack'd.
	int incoming_acknowledged;
	// Toggles T/F as reliable messages are received.
	int incoming_reliable_acknowledged;
	// single bit, maintained local
	int incoming_reliable_sequence;
	// Message we are sending to remote
	int outgoing_sequence;
	// Whether the message contains reliable payload, single bit
	int reliable_sequence;
	// Outgoing sequence number of last send that had reliable data
	int last_reliable_sequence;

	void *connection_status;
	int ( *pfnNetchan_Blocksize )( void * );

	// Staging and holding areas
	sizebuf_t message;
	byte message_buf[ MAX_MSGLEN ];

	// Reliable message buffer. We keep adding to it until reliable is acknowledged. Then we clear it.
	int reliable_length;
	byte reliable_buf[ MAX_MSGLEN ];

	// Waiting list of buffered fragments to go onto queue. Multiple outgoing buffers can be queued in succession.
	fragbufwaiting_t *waitlist[ MAX_STREAMS ];

	// Is reliable waiting buf a fragment?
	int reliable_fragment[ MAX_STREAMS ];
	// Buffer id for each waiting fragment
	unsigned int reliable_fragid[ MAX_STREAMS ];

	// The current fragment being set
	fragbuf_t *fragbufs[ MAX_STREAMS ];
	// The total number of fragments in this stream
	int fragbufcount[ MAX_STREAMS ];

	// Position in outgoing buffer where frag data starts
	short int frag_startpos[ MAX_STREAMS ];
	// Length of frag data in the buffer
	short int frag_length[ MAX_STREAMS ];

	// Incoming fragments are stored here
	fragbuf_t *incomingbufs[ MAX_STREAMS ];
	// Set to true when incoming data is ready
	qboolean incomingready[ MAX_STREAMS ];

	// Only referenced by the FRAG_FILE_STREAM component
	// Name of file being downloaded
	char incomingfilename[ MAX_PATH ];

	void *tempbuffer;
	int tempbuffersize;

	// Incoming and outgoing flow metrics
	flow_t flow[ MAX_FLOWS ];
} netchan_t;

// netapi
#define NETAPI_REQUEST_SERVERLIST (0) // Doesn't need a remote address
#define NETAPI_REQUEST_PING (1)
#define NETAPI_REQUEST_RULES (2)
#define NETAPI_REQUEST_PLAYERS (3)
#define NETAPI_REQUEST_DETAILS (4)

// Set this flag for things like broadcast requests, etc. where the engine should not
//  kill the request hook after receiving the first response
#define FNETAPI_MULTIPLE_RESPONSE (1 << 0)

typedef void ( *net_api_response_func_t )( struct net_response_s *response );

#define NET_SUCCESS (0)
#define NET_ERROR_TIMEOUT (1 << 0)
#define NET_ERROR_PROTO_UNSUPPORTED (1 << 1)
#define NET_ERROR_UNDEFINED (1 << 2)

typedef struct net_adrlist_s
{
	struct net_adrlist_s *next;
	netadr_t remote_address;
} net_adrlist_t;

typedef struct net_response_s
{
	// NET_SUCCESS or an error code
	int error;

	// Context ID
	int context;
	// Type
	int type;

	// Server that is responding to the request
	netadr_t remote_address;

	// Response RTT ping time
	double ping;
	// Key/Value pair string ( separated by backlash \ characters )
	// WARNING:  You must copy this buffer in the callback function, because it is freed
	//  by the engine right after the call!!!!
	// ALSO:  For NETAPI_REQUEST_SERVERLIST requests, this will be a pointer to a linked list of net_adrlist_t's
	void *response;
} net_response_t;

typedef struct net_status_s
{
	// Connected to remote server?  1 == yes, 0 otherwise
	int connected;
	// Client's IP address
	netadr_t local_address;
	// Address of remote server
	netadr_t remote_address;
	// Packet Loss ( as a percentage )
	int packet_loss;
	// Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	double latency;
	// Connection time, in seconds
	double connection_time;
	// Rate setting ( for incoming data )
	double rate;
} net_status_t;

typedef struct net_api_s
{
	// APIs
	void ( *InitNetworking )( void );
	void ( *Status )( struct net_status_s *status );
	void ( *SendRequest )( int context, int request, int flags, double timeout, struct netadr_s *remote_address, net_api_response_func_t response );
	void ( *CancelRequest )( int context );
	void ( *CancelAllRequests )( void );
	char *( *AddressToString )( struct netadr_s *a );
	int ( *CompareAdrress )( struct netadr_s *a, struct netadr_s *b );
	int ( *StringToAdrress )( char *s, struct netadr_s *a );
	const char *( *ValueForKey )( const char *s, const char *key );
	void ( *RemoveKey )( char *s, const char *key );
	void ( *SetValueForKey )( char *s, const char *key, const char *value, int maxsize );
} net_api_t;

typedef struct soundfade_s
{
	int nStartPercent;
	int nClientSoundFadePercent;
	double soundFadeStartTime;
	int soundFadeOutTime;
	int soundFadeHoldTime;
	int soundFadeInTime;
} soundfade_t;

typedef struct client_static_s
{
	cactive_t state;
	netchan_t netchan;
	sizebuf_t datagram;
	byte datagram_buf[ MAX_DATAGRAM ];
	double connect_time;
	int connect_retry;
	int challenge;
	byte authprotocol;
	int userid;
	char trueaddress[ 32 ];
	float slist_time;
	int signon;
	char servername[ MAX_PATH ];
	char mapstring[ 64 ];
	char spawnparms[ 2048 ];
	char userinfo[ 256 ];
	float nextcmdtime;
	int lastoutgoingcommand;
	int demonum;
	char demos[ MAX_DEMOS ][ 16 ];
	qboolean demorecording;
	qboolean demoplayback;
	qboolean timedemo;
	float demostarttime;
	int demostartframe;
	int forcetrack;
	void *demofile;
	void *demoheader;
	qboolean demowaiting;
	qboolean demoappending;
	char demofilename[ MAX_PATH ];
	int demoframecount;
	int td_lastframe;
	int td_startframe;
	float td_starttime;
	incomingtransfer_t dl;
	float packet_loss;
	double packet_loss_recalc_time;
	int playerbits;
	soundfade_t soundfade;
	char physinfo[ MAX_PHYSINFO_STRING ];
	unsigned char md5_clientdll[ 16 ];
	netadr_t game_stream;
	netadr_t connect_stream;
	qboolean passive;
	qboolean spectator;
	qboolean director;
	qboolean fSecureClient;
	qboolean isVAC2Secure;
	unsigned long long GameServerSteamID;
	int build_num;
} client_static_t;

#define MAX_QPATH 64    // Must match value in quakedefs.h
#define MAX_RESOURCE_LIST	1280

/////////////////
// Customization
// passed to pfnPlayerCustomization
// For automatic downloading.
typedef enum
{
	t_sound = 0,
	t_skin,
	t_model,
	t_decal,
	t_generic,
	t_eventscript,
	t_world,		// Fake type for world, is really t_model
	rt_unk,

	rt_max
} resourcetype_t;


typedef struct
{
	int				size;
} _resourceinfo_t;

typedef struct resourceinfo_s
{
	_resourceinfo_t info[ rt_max ];
} resourceinfo_t;

typedef struct resource_s
{
	char              szFileName[ MAX_QPATH ]; // File name to download/precache.
	resourcetype_t    type;                // t_sound, t_skin, t_model, t_decal.
	int               nIndex;              // For t_decals
	int               nDownloadSize;       // Size in Bytes if this must be downloaded.
	unsigned char     ucFlags;

	// For handling client to client resource propagation
	unsigned char     rgucMD5_hash[ 16 ];    // To determine if we already have it.
	unsigned char     playernum;           // Which player index this resource is associated with, if it's a custom resource.

	unsigned char	  rguc_reserved[ 32 ]; // For future expansion
	struct resource_s *pNext;              // Next in chain.
	struct resource_s *pPrev;
} resource_t;

typedef struct screenfade_s
{
	float		fadeSpeed;		// How fast to fade (tics / second) (+ fade in, - fade out)
	float		fadeEnd;		// When the fading hits maximum
	float		fadeTotalEnd;	// Total End Time of the fade (used for FFADE_OUT)
	float		fadeReset;		// When to reset to not fading (for fadeout and hold)
	byte		fader, fadeg, fadeb, fadealpha;	// Fade color
	int			fadeFlags;		// Fading flags
} screenfade_t;

typedef struct runcmd_s
{
	usercmd_t cmd;
	float senttime;
	float receivedtime;
	float frame_lerp;
	qboolean processedfuncs;
	qboolean heldback;
	int sendsize;
} runcmd_t;

typedef struct packet_entities_s
{
	int num_entities;
	unsigned char flags[ 32 ];
	entity_state_t *entities;
} packet_entities_t;

typedef struct frame_s
{
	double receivedtime;
	double latency;
	qboolean invalid;
	qboolean choked;
	entity_state_t playerstate[ 32 ];
	double time;
	clientdata_t clientdata;
	weapon_data_t weapondata[ 64 ];
	packet_entities_t packet_entities;
	unsigned short clientbytes;
	unsigned short playerinfobytes;
	unsigned short packetentitybytes;
	unsigned short tentitybytes;
	unsigned short soundbytes;
	unsigned short eventbytes;
	unsigned short usrbytes;
	unsigned short voicebytes;
	unsigned short msgbytes;
} frame_t;

typedef struct event_s
{
	unsigned short index;
	const char *filename;
	int filesize;
	const char *pszScript;
} event_t;

#define MAX_EVENTS			256
#define MAX_PACKET_ENTITIES		256	// 256 visible entities per frame
#define MAX_MODEL_INDEX_BITS		9	// sent as a short
#define MAX_MODELS			(1<<MAX_MODEL_INDEX_BITS)
#define MAX_SOUND_INDEX_BITS		9
#define MAX_SOUNDS			(1<<MAX_SOUND_INDEX_BITS)
#define MAX_SOUNDS_HASHLOOKUP_SIZE	(MAX_SOUNDS * 2 - 1)

#define MAX_CONSISTENCY_LIST 512

typedef struct consistency_s
{
	char *filename;
	int issound;
	int orig_index;
	int value;
	int check_type;
	float mins[ 3 ];
	float maxs[ 3 ];
} consistency_t;

typedef struct customization_s
{
	qboolean bInUse;     // Is this customization in use;
	resource_t resource; // The resource_t for this customization
	qboolean bTranslated; // Has the raw data been translated into a useable format?  
	//  (e.g., raw decal .wad make into texture_t *)
	int        nUserData1; // Customization specific data
	int        nUserData2; // Customization specific data
	void *pInfo;          // Buffer that holds the data structure that references the data (e.g., the cachewad_t)
	void *pBuffer;       // Buffer that holds the data for the customization (the raw .wad data)
	struct customization_s *pNext; // Next in chain
} customization_t;

#define	MAX_INFO_STRING			256
#define	MAX_SCOREBOARDNAME	32
typedef struct player_info_s
{
	// User id on server
	int		userid;

	// User info string
	char	userinfo[ MAX_INFO_STRING ];

	// Name
	char	name[ MAX_SCOREBOARDNAME ];

	// Spectator or not, unused
	int		spectator;

	int		ping;
	int		packet_loss;

	// skin information
	char	model[ MAX_QPATH ];
	int		topcolor;
	int		bottomcolor;

	// last frame rendered
	int		renderframe;

	// Gait frame estimation
	int		gaitsequence;
	float	gaitframe;
	float	gaityaw;
	Vector	prevgaitorigin;

	customization_t customdata;

	char hashedcdkey[ 16 ];
	unsigned long long m_nSteamID;
} player_info_t;

typedef struct dlight_s
{
	Vector	origin;
	float	radius;
	color24	color;
	float	die;				// stop lighting after this time
	float	decay;				// drop this each second
	float	minlight;			// don't add when contributing less
	int		key;
	qboolean	dark;			// subtracts light instead of adding
} dlight_t;

// 16 simultaneous events, max
#define MAX_EVENT_QUEUE 64

#define DEFAULT_EVENT_RESENDS 1


typedef struct event_args_s
{
	int		flags;

	// Transmitted
	int		entindex;

	float	origin[ 3 ];
	float	angles[ 3 ];
	float	velocity[ 3 ];

	int		ducking;

	float	fparam1;
	float	fparam2;

	int		iparam1;
	int		iparam2;

	int		bparam1;
	int		bparam2;
} event_args_t;

typedef struct event_info_s event_info_t;

struct event_info_s
{
	unsigned short index;			  // 0 implies not in use

	short packet_index;      // Use data from state info for entity in delta_packet .  -1 implies separate info based on event
	// parameter signature
	short entity_index;      // The edict this event is associated with

	float fire_time;        // if non-zero, the time when the event should be fired ( fixed up on the client )

	event_args_t args;

	// CLIENT ONLY	
	int	  flags;			// Reliable or not, etc.

};

typedef struct event_state_s event_state_t;

struct event_state_s
{
	struct event_info_s ei[ MAX_EVENT_QUEUE ];
};

typedef void ( *pfnEventHook )( event_args_t *args );
typedef struct event_hook_s
{
	struct event_hook_s *next;
	char *name;
	pfnEventHook function;
} event_hook_t;

typedef void ( *pfnNetMsgHook )( void );
typedef struct netmsg_s
{
	char *name;
	pfnNetMsgHook function;
	int index;
} netmsg_t;

typedef struct client_state_s
{
	int max_edicts;
	resource_t resourcesonhand;
	resource_t resourcesneeded;
	resource_t resourcelist[ MAX_RESOURCE_LIST ];
	int num_resources;
	qboolean need_force_consistency_response;
	char serverinfo[ 512 ];
	int servercount;
	int validsequence;
	int parsecount;
	int parsecountmod;
	int stats[ 32 ];
	int weapons;
	usercmd_t cmd;
	Vector viewangles;
	Vector punchangle;
	Vector crosshairangle;
	Vector simorg;
	Vector simvel;
	Vector simangles;
	Vector predicted_origins[ 64 ];
	Vector prediction_error;
	float idealpitch;
	Vector viewheight;
	screenfade_t sf;
	qboolean paused;
	int onground;
	int moving;
	int waterlevel;
	int usehull;
	float maxspeed;
	int pushmsec;
	int light_level;
	int intermission;
	double mtime[ 2 ];
	double time;
	double oldtime;
	frame_t frames[ 64 ];
	runcmd_t commands[ 64 ];
	local_state_t predicted_frames[ 64 ];
	int delta_sequence;
	int playernum;
	event_t event_precache[ MAX_EVENTS ];
	model_t *model_precache[ MAX_MODELS ];
	int model_precache_count;
	sfx_t *sound_precache[ MAX_SOUNDS ];
	consistency_t consistency_list[ MAX_CONSISTENCY_LIST ];
	int num_consistency;
	int highentity;
	char levelname[ 40 ];
	int maxclients;
	int gametype;
	int viewentity;
	model_t *worldmodel;
	efrag_t *free_efrags;
	int num_entities;
	int num_statics;
	cl_entity_t viewent;
	int cdtrack;
	int looptrack;
	unsigned int serverCRC;
	unsigned char clientdllmd5[ 16 ];
	float weaponstarttime;
	int weaponsequence;
	int fPrecaching;
	dlight_t *pLight;
	player_info_t players[ 32 ];
	entity_state_t instanced_baseline[ 64 ];
	int instanced_baseline_number;
	unsigned int mapCRC;
	event_state_t events;
	char downloadUrl[ 128 ];
} client_state_t;


typedef struct vmode_s
{
	int width;
	int height;
	int bpp; // bits per pixel
} vmode_t;

class IVideoMode
{
public:
	virtual const char *GetName( void ) = 0;

	virtual	int Init( void ) = 0;
	virtual void Shutdown( void ) = 0;

	virtual void AddMode( int width, int height, int bpp ) = 0;

	virtual vmode_t *GetCurrentMode() = 0;
	virtual vmode_t *GetMode( int num ) = 0;

	virtual int GetModeCount( void ) = 0;

	virtual bool IsWindowedMode( void ) = 0;

	virtual int GetInitialized( void ) = 0;
	virtual void SetInitialized( bool initialized ) = 0;

	virtual void UpdateWindowPosition( void ) = 0;

	// (=
	virtual void FlipScreen( void ) = 0;

	virtual void RestoreVideo( void ) = 0;
	virtual void ReleaseVideo( void ) = 0;

	virtual void unk( void ) = 0;

	virtual int MaxBitsPerPixel( void ) = 0;

	virtual void ReleaseFullScreen( void ) = 0;
	virtual void ChangeDisplaySettingsToFullscreen( void ) = 0;
};

/* ENGINE STRUCTS END */


typedef struct
{
	double				dummy;

	double				m_clTime;			// Client clock
	double				m_clOldTime;		// Old Client clock
	qboolean			m_fDoInterp;		// Do interpolation?
	qboolean			m_fGaitEstimation;	// Do gait estimation?
	int					m_nFrameCount; // Current render frame #

	// Cvars that studio model code needs to reference
	cvar_t *m_pCvarHiModels;				// Use high quality models?
	cvar_t *m_pCvarDeveloper;				// Developer debug output desired?
	cvar_t *m_pCvarDrawEntities; // Draw entities bone hit boxes, etc?

	cl_entity_t *m_pCurrentEntity;		// The entity which we are currently rendering.
	model_t *m_pRenderModel;			// The model for the entity being rendered
	player_info_t *m_pPlayerInfo;			// Player info for current player, if drawing a player

	int					m_nPlayerIndex;			// The index of the player being drawn
	float				m_flGaitMovement;		// The player's gait movement

	studiohdr_t *m_pStudioHeader;	// Pointer to header block for studio model data
	mstudiobodyparts_t *m_pBodyPart;		// Pointers to current body part and submodel
	mstudiomodel_t *m_pSubModel;

	// Palette substition for top and bottom of model
	int					m_nTopColor;
	int					m_nBottomColor;

	model_t *m_pChromeSprite; // Sprite model used for drawing studio model chrome

	// Caching
	int					m_nCachedBones;							// Number of bones in bone cache
	char				m_nCachedBoneNames[ MAXSTUDIOBONES ][ 32 ];	// Names of cached bones

	// Cached bone & light transformation matrices
	float				m_rgCachedBoneTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float				m_rgCachedLightTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];

	// Software renderer scale factors
	float				m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	Vector				m_vUp;
	Vector				m_vRight;
	Vector				m_vNormal;

	Vector				m_vRenderOrigin;

	// Model render counters(from engine)
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float( *m_protationmatrix )[ 3 ][ 4 ]; // Model to world transformation
	float( *m_paliastransform )[ 3 ][ 4 ]; // Model to view transformation

	// Concatenated bone and light transforms
	float( *m_pbonetransform )[ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float( *m_plighttransform )[ MAXSTUDIOBONES ][ 3 ][ 4 ];
} StudioModelRenderer_td, *StudioModelRenderer_d;

typedef struct StudioModelRenderer_s
{
	// Construction/Destruction
	void				( *CStudioModelRenderer )			( void );

	// Initialization
	void				( *Init )							( void );

	// Public Interfaces
	int					( *StudioDrawModel )				( int flags );
	int					( *StudioDrawPlayer )				( int flags, entity_state_s *pplayer );

	// Look up animation data for sequence
	mstudioanim_t *( *StudioGetAnim )				( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc );

	// Interpolate model position and angles and set up matrices
	void				( *StudioSetUpTransform )			( int trivial_accept );

	// Set up model bone positions
	void				( *StudioSetupBones )				( void );

	// Find final attachment points
	void				( *StudioCalcAttachments )		( void );

	// Save bone matrices and names
	void				( *StudioSaveBones )				( void );

	// Merge cached bones with current bones for model
	void				( *StudioMergeBones )				( model_t *m_pSubModel );

	// Determine interpolation fraction
	float				( *StudioEstimateInterpolant )	( void );

	// Determine current frame for rendering
	float				( *StudioEstimateFrame )			( mstudioseqdesc_t *pseqdesc );

	// Apply special effects to transform matrix
	void				( *StudioFxTransform )			( cl_entity_t *ent, float transform[ 3 ][ 4 ] );

	// Spherical interpolation of bones
	void				( *StudioSlerpBones )				( vec4_t q1[], float pos1[][ 3 ], vec4_t q2[], float pos2[][ 3 ], float s );

	// Compute bone adjustments(bone controllers)
	void				( *StudioCalcBoneAdj )			( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen );

	// Get bone quaternions
	void				( *StudioCalcBoneQuaterion )		( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q );

	// Get bone positions
	void				( *StudioCalcBonePosition )		( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos );

	// Compute rotations
	void				( *StudioCalcRotations )			( float pos[][ 3 ], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	// Send bones and verts to renderer
	void				( *StudioRenderModel )			( void );

	// Finalize rendering
	void				( *StudioRenderFinal )			( void );

	// GL&D3D vs. Software renderer finishing functions
	void				( *StudioRenderFinal_Software )	( void );
	void				( *StudioRenderFinal_Hardware )	( void );

	// Player specific data
	// Determine pitch and blending amounts for players
	void				( *StudioPlayerBlend )			( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );

	// Estimate gait frame for player
	void				( *StudioEstimateGait )			( entity_state_t *pplayer );

	// Process movement of player
	void				( *StudioProcessGait )			( entity_state_t *pplayer );

	int					( *_StudioDrawPlayer )			( int flags, entity_state_s *pplayer );

	int					( *CalculateYawBlend )			( entity_state_s *pplayer );

	void				( *CalculatePitchBlend )			( entity_state_t *pplayer );
} StudioModelRenderer_t;

class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer( void );
	virtual ~CStudioModelRenderer( void ); // 0

	// Initialization
	virtual void Init( void ); // 1

public:
	// Public Interfaces
	virtual int StudioDrawModel( int flags ); // 2
	virtual int StudioDrawPlayer( int flags, struct entity_state_s *pplayer ); // 3

public:
	// Local interfaces
	//
	virtual int StudioDrawMonster( int flags, struct cl_entity_s *pmonster ); // 4

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc ); // 5

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpTransform( int trivial_accept ); // 6

	// Set up model bone positions
	virtual void StudioSetupBones( void ); // 7

	// Find final attachment points
	virtual void StudioCalcAttachments( void ); // 8

	// Save bone matrices and names
	virtual void StudioSaveBones( void ); // 9

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones( model_t *m_pSubModel ); // 10

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant( void ); // 11

	// Determine current frame for rendering
	virtual float StudioEstimateFrame( mstudioseqdesc_t *pseqdesc ); // 12

	// Determine current gait frame for rendering
	virtual float StudioEstimateGaitFrame( mstudioseqdesc_t *pseqdesc ); // 13

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[ 3 ][ 4 ] ); // 14

	// Spherical interpolation of bones
	virtual void StudioSlerpBones( vec4_t q1[], float pos1[][ 3 ], vec4_t q2[], float pos2[][ 3 ], float s ); // 15

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen ); // 16

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q ); // 17

	// Get bone positions
	virtual void StudioCalcBonePosition( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos ); // 18

	// Compute rotations
	virtual void StudioCalcRotations( float pos[][ 3 ], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f ); // 19

	// Send bones and verts to renderer
	virtual void StudioRenderModel( void ); // 20

	// Finalize rendering
	virtual void StudioRenderFinal( void ); // 21

	// GL&D3D vs. Software renderer finishing functions
	virtual void StudioRenderFinal_Software( void ); // 22
	virtual void StudioRenderFinal_Hardware( void ); // 23

	// Player/Monster specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch ); // 24

	// Determine pitch and blending amounts for monsters
	virtual void StudioMonsterBlend( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch ); // 25

	// Estimate gait frame for player
	virtual void StudioEstimateGait( entity_state_t *pplayer ); // 26

	// Estimate gait frame for monster
	virtual void StudioEstimateMonsterGait( cl_entity_s *pmonster ); // 27

	// Process movement of player
	virtual void StudioProcessGait( entity_state_t *pplayer ); // 28

	// Process movement of monster
	virtual void StudioProcessMonsterGait( cl_entity_s *pmonster ); // 29

public:
	// Client clock
	double			m_clTime;
	// Old Client clock
	double			m_clOldTime;

	// Do interpolation?
	int				m_fDoInterp;
	// Do gait estimation?
	int				m_fGaitEstimation;

	// Current render frame #
	int				m_nFrameCount;

	// Cvars that studio model code needs to reference
	//
	// Use high quality models?
	cvar_t *m_pCvarHiModels;

	// FIXME: purpose?
	cvar_t *m_pCvarIndex1;
	cvar_t *m_pCvarIndex2;
	cvar_t *m_pCvarIndex3;
	cvar_t *m_pCvarIndex4;

	// Developer debug output desired?
	cvar_t *m_pCvarDeveloper;
	// Draw entities bone hit boxes, etc?
	cvar_t *m_pCvarDrawEntities;

	// The entity which we are currently rendering.
	cl_entity_t *m_pCurrentEntity;

	// The model for the entity being rendered
	model_t *m_pRenderModel;

	// Player info for current player, if drawing a player
	player_info_t *m_pPlayerInfo;

	// Never used
	char			unknown[ 4 ];

	// The index of the player being drawn
	int				m_nPlayerIndex;

	// The player's gait movement
	float			m_flGaitMovement;

	// Pointer to header block for studio model data
	studiohdr_t *m_pStudioHeader;

	// Pointers to current body part and submodel
	mstudiobodyparts_t *m_pBodyPart;
	mstudiomodel_t *m_pSubModel;

	// Palette substition for top and bottom of model
	int				m_nTopColor;
	int				m_nBottomColor;

	//
	// Sprite model used for drawing studio model chrome
	model_t *m_pChromeSprite;

	// Caching
	// Number of bones in bone cache
	int				m_nCachedBones;
	// Names of cached bones
	char			m_nCachedBoneNames[ MAXSTUDIOBONES ][ 32 ];
	// Cached bone & light transformation matrices
	float			m_rgCachedBoneTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float			m_rgCachedLightTransform[ MAXSTUDIOBONES ][ 3 ][ 4 ];

	// Software renderer scale factors
	float			m_fSoftwareXScale, m_fSoftwareYScale;

	// Current view vectors and render origin
	float			m_vUp[ 3 ];
	float			m_vRight[ 3 ];
	float			m_vNormal[ 3 ];

	float			m_vRenderOrigin[ 3 ];

	// Model render counters ( from engine )
	int *m_pStudioModelCount;
	int *m_pModelsDrawn;

	// Matrices
	// Model to world transformation
	float( *m_protationmatrix )[ 3 ][ 4 ];
	// Model to view transformation
	float( *m_paliastransform )[ 3 ][ 4 ];

	// Concatenated bone and light transforms
	float( *m_pbonetransform )[ MAXSTUDIOBONES ][ 3 ][ 4 ];
	float( *m_plighttransform )[ MAXSTUDIOBONES ][ 3 ][ 4 ];
};

// Temporary entity array
#define TENTPRIORITY_LOW 0
#define TENTPRIORITY_HIGH 1

// TEMPENTITY flags
#define FTENT_NONE 0x00000000
#define FTENT_SINEWAVE 0x00000001
#define FTENT_GRAVITY 0x00000002
#define FTENT_ROTATE 0x00000004
#define FTENT_SLOWGRAVITY 0x00000008
#define FTENT_SMOKETRAIL 0x00000010
#define FTENT_COLLIDEWORLD 0x00000020
#define FTENT_FLICKER 0x00000040
#define FTENT_FADEOUT 0x00000080
#define FTENT_SPRANIMATE 0x00000100
#define FTENT_HITSOUND 0x00000200
#define FTENT_SPIRAL 0x00000400
#define FTENT_SPRCYCLE 0x00000800
#define FTENT_COLLIDEALL 0x00001000		// will collide with world and slideboxes
#define FTENT_PERSIST 0x00002000		// tent is not removed when unable to draw
#define FTENT_COLLIDEKILL 0x00004000	// tent is removed upon collision with anything
#define FTENT_PLYRATTACHMENT 0x00008000 // tent is attached to a player (owner)
#define FTENT_SPRANIMATELOOP 0x00010000 // animating sprite doesn't die when last frame is displayed
#define FTENT_SPARKSHOWER 0x00020000
#define FTENT_NOMODEL 0x00040000	  // Doesn't have a model, never try to draw ( it just triggers other things )
#define FTENT_CLIENTCUSTOM 0x00080000 // Must specify callback.  Callback function is responsible for killing tempent and updating fields ( unless other flags specify how to do things )

typedef struct tempent_s
{
	int flags;
	float die;
	float frameMax;
	float x;
	float y;
	float z;
	float fadeSpeed;
	float bounceFactor;
	int hitSound;
	void ( *hitcallback )( struct tempent_s *ent, struct pmtrace_s *ptr );
	void ( *callback )( struct tempent_s *ent, float frametime, float currenttime );
	struct tempent_s *next;
	int priority;
	short clientIndex; // if attached, this is the index of the client to stick to
	// if COLLIDEALL, this is the index of the client to ignore
	// TENTS with FTENT_PLYRATTACHMENT MUST set the clientindex!

	Vector tentOffset; // if attached, client origin + tentOffset = tent origin.
	cl_entity_t entity;

	// baseline.origin		- velocity
	// baseline.renderamt	- starting fadeout intensity
	// baseline.angles		- angle velocity
} TEMPENTITY;

typedef struct efx_api_s efx_api_t;

struct efx_api_s
{
	particle_t *( *R_AllocParticle )( void ( *callback )( struct particle_s *particle, float frametime ) );
	void ( *R_BlobExplosion )( float *org );
	void ( *R_Blood )( float *org, float *dir, int pcolor, int speed );
	void ( *R_BloodSprite )( float *org, int colorindex, int modelIndex, int modelIndex2, float size );
	void ( *R_BloodStream )( float *org, float *dir, int pcolor, int speed );
	void ( *R_BreakModel )( float *pos, float *size, float *dir, float random, float life, int count, int modelIndex, char flags );
	void ( *R_Bubbles )( float *mins, float *maxs, float height, int modelIndex, int count, float speed );
	void ( *R_BubbleTrail )( float *start, float *end, float height, int modelIndex, int count, float speed );
	void ( *R_BulletImpactParticles )( float *pos );
	void ( *R_EntityParticles )( struct cl_entity_s *ent );
	void ( *R_Explosion )( float *pos, int model, float scale, float framerate, int flags );
	void ( *R_FizzEffect )( struct cl_entity_s *pent, int modelIndex, int density );
	void ( *R_FireField )( float *org, int radius, int modelIndex, int count, int flags, float life );
	void ( *R_FlickerParticles )( float *org );
	void ( *R_FunnelSprite )( float *org, int modelIndex, int reverse );
	void ( *R_Implosion )( float *end, float radius, int count, float life );
	void ( *R_LargeFunnel )( float *org, int reverse );
	void ( *R_LavaSplash )( float *org );
	void ( *R_MultiGunshot )( float *org, float *dir, float *noise, int count, int decalCount, int *decalIndices );
	void ( *R_MuzzleFlash )( float *pos1, int type );
	void ( *R_ParticleBox )( float *mins, float *maxs, unsigned char r, unsigned char g, unsigned char b, float life );
	void ( *R_ParticleBurst )( float *pos, int size, int color, float life );
	void ( *R_ParticleExplosion )( float *org );
	void ( *R_ParticleExplosion2 )( float *org, int colorStart, int colorLength );
	void ( *R_ParticleLine )( float *start, float *end, unsigned char r, unsigned char g, unsigned char b, float life );
	void ( *R_PlayerSprites )( int client, int modelIndex, int count, int size );
	void ( *R_Projectile )( float *origin, float *velocity, int modelIndex, int life, int owner, void ( *hitcallback )( struct tempent_s *ent, struct pmtrace_s *ptr ) );
	void ( *R_RicochetSound )( float *pos );
	void ( *R_RicochetSprite )( float *pos, struct model_s *pmodel, float duration, float scale );
	void ( *R_RocketFlare )( float *pos );
	void ( *R_RocketTrail )( float *start, float *end, int type );
	void ( *R_RunParticleEffect )( float *org, float *dir, int color, int count );
	void ( *R_ShowLine )( float *start, float *end );
	void ( *R_SparkEffect )( float *pos, int count, int velocityMin, int velocityMax );
	void ( *R_SparkShower )( float *pos );
	void ( *R_SparkStreaks )( float *pos, int count, int velocityMin, int velocityMax );
	void ( *R_Spray )( float *pos, float *dir, int modelIndex, int count, int speed, int spread, int rendermode );
	void ( *R_Sprite_Explode )( TEMPENTITY *pTemp, float scale, int flags );
	void ( *R_Sprite_Smoke )( TEMPENTITY *pTemp, float scale );
	void ( *R_Sprite_Spray )( float *pos, float *dir, int modelIndex, int count, int speed, int iRand );
	void ( *R_Sprite_Trail )( int type, float *start, float *end, int modelIndex, int count, float life, float size, float amplitude, int renderamt, float speed );
	void ( *R_Sprite_WallPuff )( TEMPENTITY *pTemp, float scale );
	void ( *R_StreakSplash )( float *pos, float *dir, int color, int count, float speed, int velocityMin, int velocityMax );
	void ( *R_TracerEffect )( float *start, float *end );
	void ( *R_UserTracerParticle )( float *org, float *vel, float life, int colorIndex, float length, unsigned char deathcontext, void ( *deathfunc )( struct particle_s *particle ) );
	particle_t *( *R_TracerParticles )( float *org, float *vel, float life );
	void ( *R_TeleportSplash )( float *org );
	void ( *R_TempSphereModel )( float *pos, float speed, float life, int count, int modelIndex );
	TEMPENTITY *( *R_TempModel )( float *pos, float *dir, float *angles, float life, int modelIndex, int soundtype );
	TEMPENTITY *( *R_DefaultSprite )( float *pos, int spriteIndex, float framerate );
	TEMPENTITY *( *R_TempSprite )( float *pos, const float *dir, float scale, int modelIndex, int rendermode, int renderfx, float a, float life, int flags );
	int ( *Draw_DecalIndex )( int id );
	int ( *Draw_DecalIndexFromName )( char *name );
	void ( *R_DecalShoot )( int textureIndex, int entity, int modelIndex, float *position, int flags );
	void ( *R_AttachTentToPlayer )( int client, int modelIndex, float zoffset, float life );
	void ( *R_KillAttachedTents )( int client );
	BEAM *( *R_BeamCirclePoints )( int type, float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM *( *R_BeamEntPoint )( int startEnt, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM *( *R_BeamEnts )( int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM *( *R_BeamFollow )( int startEnt, int modelIndex, float life, float width, float r, float g, float b, float brightness );
	void ( *R_BeamKill )( int deadEntity );
	BEAM *( *R_BeamLightning )( float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed );
	BEAM *( *R_BeamPoints )( float *start, float *end, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	BEAM *( *R_BeamRing )( int startEnt, int endEnt, int modelIndex, float life, float width, float amplitude, float brightness, float speed, int startFrame, float framerate, float r, float g, float b );
	dlight_t *( *CL_AllocDlight )( int key );
	dlight_t *( *CL_AllocElight )( int key );
	TEMPENTITY *( *CL_TempEntAlloc )( float *org, struct model_s *model );
	TEMPENTITY *( *CL_TempEntAllocNoModel )( float *org );
	TEMPENTITY *( *CL_TempEntAllocHigh )( float *org, struct model_s *model );
	TEMPENTITY *( *CL_TentEntAllocCustom )( float *origin, struct model_s *model, int high, void ( *callback )( struct tempent_s *ent, float frametime, float currenttime ) );
	void ( *R_GetPackedColor )( short *packed, short color );
	short ( *R_LookupColor )( unsigned char r, unsigned char g, unsigned char b );
	void ( *R_DecalRemoveNonPermanent )( int textureIndex ); //textureIndex points to the decal index in the array, not the actual texture index.  R_DecalRemoveAll
	void ( *R_FireCustomDecal )( int textureIndex, int entity, int modelIndex, float *position, int flags, float scale );
};

// event api
#define EVENT_API_VERSION 1

typedef struct event_api_s
{
	int        version;
	void    ( *EV_PlaySound ) ( int ent, float *origin, int channel, const char *sample, float volume, float attenuation, int fFlags, int pitch );
	void    ( *EV_StopSound ) ( int ent, int channel, const char *sample );
	int        ( *EV_FindModelIndex )( const char *pmodel );
	int        ( *EV_IsLocal ) ( int playernum );
	int        ( *EV_LocalPlayerDucking ) ( void );
	void    ( *EV_LocalPlayerViewheight ) ( float * );
	void    ( *EV_LocalPlayerBounds ) ( int hull, float *mins, float *maxs );
	int        ( *EV_IndexFromTrace ) ( struct pmtrace_s *pTrace );
	struct physent_s *( *EV_GetPhysent ) ( int idx );
	void    ( *EV_SetUpPlayerPrediction ) ( int dopred, int bIncludeLocalClient );
	void    ( *EV_PushPMStates ) ( void );
	void    ( *EV_PopPMStates ) ( void );
	void    ( *EV_SetSolidPlayers ) ( int playernum );
	void    ( *EV_SetTraceHull ) ( int hull );
	void    ( *EV_PlayerTrace ) ( float *start, float *end, int traceFlags, int ignore_pe, struct pmtrace_s *tr );
	void    ( *EV_WeaponAnimation ) ( int sequence, int body );
	unsigned short ( *EV_PrecacheEvent ) ( int type, const char *psz );
	void    ( *EV_PlaybackEvent ) ( int flags, const struct edict_s *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
	const char *( *EV_TraceTexture ) ( int ground, float *vstart, float *vend );
	void    ( *EV_StopAllSounds ) ( int entnum, int entchannel );
	void    ( *EV_KillEvents ) ( int entnum, const char *eventname );
} event_api_t;

// Pointers to the exported engine functions themselves
typedef struct cl_enginefuncs_s
{
	pfnEngSrc_pfnSPR_Load_t					pfnSPR_Load;
	pfnEngSrc_pfnSPR_Frames_t				pfnSPR_Frames;
	pfnEngSrc_pfnSPR_Height_t				pfnSPR_Height;
	pfnEngSrc_pfnSPR_Width_t				pfnSPR_Width;
	pfnEngSrc_pfnSPR_Set_t					pfnSPR_Set;
	pfnEngSrc_pfnSPR_Draw_t					pfnSPR_Draw;
	pfnEngSrc_pfnSPR_DrawHoles_t			pfnSPR_DrawHoles;
	pfnEngSrc_pfnSPR_DrawAdditive_t			pfnSPR_DrawAdditive;
	pfnEngSrc_pfnSPR_EnableScissor_t		pfnSPR_EnableScissor;
	pfnEngSrc_pfnSPR_DisableScissor_t		pfnSPR_DisableScissor;
	pfnEngSrc_pfnSPR_GetList_t				pfnSPR_GetList;
	pfnEngSrc_pfnFillRGBA_t					pfnFillRGBA;
	pfnEngSrc_pfnGetScreenInfo_t			pfnGetScreenInfo;
	pfnEngSrc_pfnSetCrosshair_t				pfnSetCrosshair;
	pfnEngSrc_pfnRegisterVariable_t			pfnRegisterVariable;
	pfnEngSrc_pfnGetCvarFloat_t				pfnGetCvarFloat;
	pfnEngSrc_pfnGetCvarString_t			pfnGetCvarString;
	pfnEngSrc_pfnAddCommand_t				pfnAddCommand;
	pfnEngSrc_pfnHookUserMsg_t				pfnHookUserMsg;
	pfnEngSrc_pfnServerCmd_t				pfnServerCmd;
	pfnEngSrc_pfnClientCmd_t				pfnClientCmd;
	pfnEngSrc_pfnGetPlayerInfo_t			pfnGetPlayerInfo;
	pfnEngSrc_pfnPlaySoundByName_t			pfnPlaySoundByName;
	pfnEngSrc_pfnPlaySoundByIndex_t			pfnPlaySoundByIndex;
	pfnEngSrc_pfnAngleVectors_t				pfnAngleVectors;
	pfnEngSrc_pfnTextMessageGet_t			pfnTextMessageGet;
	pfnEngSrc_pfnDrawCharacter_t			pfnDrawCharacter;
	pfnEngSrc_pfnDrawConsoleString_t		pfnDrawConsoleString;
	pfnEngSrc_pfnDrawSetTextColor_t			pfnDrawSetTextColor;
	pfnEngSrc_pfnDrawConsoleStringLen_t		pfnDrawConsoleStringLen;
	pfnEngSrc_pfnConsolePrint_t				pfnConsolePrint;
	pfnEngSrc_pfnCenterPrint_t				pfnCenterPrint;
	pfnEngSrc_GetWindowCenterX_t			GetWindowCenterX;
	pfnEngSrc_GetWindowCenterY_t			GetWindowCenterY;
	pfnEngSrc_GetViewAngles_t				GetViewAngles;
	pfnEngSrc_SetViewAngles_t				SetViewAngles;
	pfnEngSrc_GetMaxClients_t				GetMaxClients;
	pfnEngSrc_Cvar_SetValue_t				Cvar_SetValue;
	pfnEngSrc_Cmd_Argc_t					Cmd_Argc;
	pfnEngSrc_Cmd_Argv_t					Cmd_Argv;
	pfnEngSrc_Con_Printf_t					Con_Printf;
	pfnEngSrc_Con_DPrintf_t					Con_DPrintf;
	pfnEngSrc_Con_NPrintf_t					Con_NPrintf;
	pfnEngSrc_Con_NXPrintf_t				Con_NXPrintf;
	pfnEngSrc_PhysInfo_ValueForKey_t		PhysInfo_ValueForKey;
	pfnEngSrc_ServerInfo_ValueForKey_t		ServerInfo_ValueForKey;
	pfnEngSrc_GetClientMaxspeed_t			GetClientMaxspeed;
	pfnEngSrc_CheckParm_t					CheckParm;
	pfnEngSrc_Key_Event_t					Key_Event;
	pfnEngSrc_GetMousePosition_t			GetMousePosition;
	pfnEngSrc_IsNoClipping_t				IsNoClipping;
	pfnEngSrc_GetLocalPlayer_t				GetLocalPlayer;
	pfnEngSrc_GetViewModel_t				GetViewModel;
	pfnEngSrc_GetEntityByIndex_t			GetEntityByIndex;
	pfnEngSrc_GetClientTime_t				GetClientTime;
	pfnEngSrc_V_CalcShake_t					V_CalcShake;
	pfnEngSrc_V_ApplyShake_t				V_ApplyShake;
	pfnEngSrc_PM_PointContents_t			PM_PointContents;
	pfnEngSrc_PM_WaterEntity_t				PM_WaterEntity;
	pfnEngSrc_PM_TraceLine_t				PM_TraceLine;
	pfnEngSrc_CL_LoadModel_t				CL_LoadModel;
	pfnEngSrc_CL_CreateVisibleEntity_t		CL_CreateVisibleEntity;
	pfnEngSrc_GetSpritePointer_t			GetSpritePointer;
	pfnEngSrc_pfnPlaySoundByNameAtLocation_t	pfnPlaySoundByNameAtLocation;
	pfnEngSrc_pfnPrecacheEvent_t			pfnPrecacheEvent;
	pfnEngSrc_pfnPlaybackEvent_t			pfnPlaybackEvent;
	pfnEngSrc_pfnWeaponAnim_t				pfnWeaponAnim;
	pfnEngSrc_pfnRandomFloat_t				pfnRandomFloat;
	pfnEngSrc_pfnRandomLong_t				pfnRandomLong;
	pfnEngSrc_pfnHookEvent_t				pfnHookEvent;
	pfnEngSrc_Con_IsVisible_t				Con_IsVisible;
	pfnEngSrc_pfnGetGameDirectory_t			pfnGetGameDirectory;
	pfnEngSrc_pfnGetCvarPointer_t			pfnGetCvarPointer;
	pfnEngSrc_Key_LookupBinding_t			Key_LookupBinding;
	pfnEngSrc_pfnGetLevelName_t				pfnGetLevelName;
	pfnEngSrc_pfnGetScreenFade_t			pfnGetScreenFade;
	pfnEngSrc_pfnSetScreenFade_t			pfnSetScreenFade;
	pfnEngSrc_VGui_GetPanel_t				VGui_GetPanel;
	pfnEngSrc_VGui_ViewportPaintBackground_t	VGui_ViewportPaintBackground;
	pfnEngSrc_COM_LoadFile_t				COM_LoadFile;
	pfnEngSrc_COM_ParseFile_t				COM_ParseFile;
	pfnEngSrc_COM_FreeFile_t				COM_FreeFile;
	struct triangleapi_s *pTriAPI;
	struct efx_api_s *pEfxAPI;
	struct event_api_s *pEventAPI;
	struct demo_api_s *pDemoAPI;
	struct net_api_s *pNetAPI;
	struct IVoiceTweak_s *pVoiceTweak;
	pfnEngSrc_IsSpectateOnly_t				IsSpectateOnly;
	pfnEngSrc_LoadMapSprite_t				LoadMapSprite;
	pfnEngSrc_COM_AddAppDirectoryToSearchPath_t		COM_AddAppDirectoryToSearchPath;
	pfnEngSrc_COM_ExpandFilename_t			COM_ExpandFilename;
	pfnEngSrc_PlayerInfo_ValueForKey_t		PlayerInfo_ValueForKey;
	pfnEngSrc_PlayerInfo_SetValueForKey_t	PlayerInfo_SetValueForKey;
	pfnEngSrc_GetPlayerUniqueID_t			GetPlayerUniqueID;
	pfnEngSrc_GetTrackerIDForPlayer_t		GetTrackerIDForPlayer;
	pfnEngSrc_GetPlayerForTrackerID_t		GetPlayerForTrackerID;
	pfnEngSrc_pfnServerCmdUnreliable_t		pfnServerCmdUnreliable;
	pfnEngSrc_GetMousePos_t					pfnGetMousePos;
	pfnEngSrc_SetMousePos_t					pfnSetMousePos;
	pfnEngSrc_SetMouseEnable_t				pfnSetMouseEnable;
	pfnEngSrc_GetFirstCVarPtr_t				GetFirstCvarPtr;
	pfnEngSrc_GetFirstCmdFunctionHandle_t	GetFirstCmdFunctionHandle;
	pfnEngSrc_GetNextCmdFunctionHandle_t	GetNextCmdFunctionHandle;
	pfnEngSrc_GetCmdFunctionName_t			GetCmdFunctionName;
	pfnEngSrc_GetClientOldTime_t			hudGetClientOldTime;
	pfnEngSrc_GetServerGravityValue_t		hudGetServerGravityValue;
	pfnEngSrc_GetModelByIndex_t				hudGetModelByIndex;
	pfnEngSrc_pfnSetFilterMode_t			pfnSetFilterMode;
	pfnEngSrc_pfnSetFilterColor_t			pfnSetFilterColor;
	pfnEngSrc_pfnSetFilterBrightness_t		pfnSetFilterBrightness;
	pfnEngSrc_pfnSequenceGet_t				pfnSequenceGet;
	pfnEngSrc_pfnSPR_DrawGeneric_t			pfnSPR_DrawGeneric;
	pfnEngSrc_pfnSequencePickSentence_t		pfnSequencePickSentence;
	pfnEngSrc_pfnDrawString_t				pfnDrawString;
	pfnEngSrc_pfnDrawStringReverse_t				pfnDrawStringReverse;
	pfnEngSrc_LocalPlayerInfo_ValueForKey_t		LocalPlayerInfo_ValueForKey;
	pfnEngSrc_pfnVGUI2DrawCharacter_t		pfnVGUI2DrawCharacter;
	pfnEngSrc_pfnVGUI2DrawCharacterAdd_t	pfnVGUI2DrawCharacterAdd;
	pfnEngSrc_COM_GetApproxWavePlayLength	COM_GetApproxWavePlayLength;
	pfnEngSrc_pfnGetCareerUI_t				pfnGetCareerUI;
	pfnEngSrc_Cvar_Set_t					Cvar_Set;
	pfnEngSrc_pfnIsPlayingCareerMatch_t		pfnIsCareerMatch;
	pfnEngSrc_pfnPlaySoundVoiceByName_t	pfnPlaySoundVoiceByName;
	pfnEngSrc_pfnPrimeMusicStream_t		pfnPrimeMusicStream;
	pfnEngSrc_GetAbsoluteTime_t				GetAbsoluteTime;
	pfnEngSrc_pfnProcessTutorMessageDecayBuffer_t		pfnProcessTutorMessageDecayBuffer;
	pfnEngSrc_pfnConstructTutorMessageDecayBuffer_t		pfnConstructTutorMessageDecayBuffer;
	pfnEngSrc_pfnResetTutorMessageDecayData_t		pfnResetTutorMessageDecayData;
	pfnEngSrc_pfnPlaySoundByNameAtPitch_t	pfnPlaySoundByNameAtPitch;
	pfnEngSrc_pfnFillRGBABlend_t					pfnFillRGBABlend;
	pfnEngSrc_pfnGetAppID_t					pfnGetAppID;
	pfnEngSrc_pfnGetAliases_t				pfnGetAliasList;
	pfnEngSrc_pfnVguiWrap2_GetMouseDelta_t pfnVguiWrap2_GetMouseDelta;
} cl_enginefunc_t;

typedef struct
{
	int ( *Initialize )( cl_enginefunc_t *pEnginefunc, int iVersion );
	void ( *HUD_Init )( void );
	int ( *HUD_VidInit )( void );
	int ( *HUD_Redraw )( float time, int intermission );
	int ( *HUD_UpdateClientData )( client_data_t *pcldata, float flTime );
	void ( *HUD_Reset )( void );
	void ( *HUD_PlayerMove )( struct playermove_s *ppmove, int server );
	void ( *HUD_PlayerMoveInit )( struct playermove_s *ppmove );
	char ( *HUD_PlayerMoveTexture )( const char *name );
	void ( *IN_ActivateMouse )( void );
	void ( *IN_DeactivateMouse )( void );
	void ( *IN_MouseEvent )( int mstate );
	void ( *IN_ClearStates )( void );
	void ( *IN_Accumulate )( void );
	void ( *CL_CreateMove )( float frametime, struct usercmd_s *cmd, int active );
	int ( *CL_IsThirdPerson )( void );
	void ( *CL_CameraOffset )( float *ofs );
	struct kbutton_s *( *KB_Find )( const char *name );
	void ( *CAM_Think )( void );
	void ( *V_CalcRefdef )( struct ref_params_s *pparams );
	int ( *HUD_AddEntity )( int type, struct cl_entity_s *ent, const char *modelname );
	void ( *HUD_CreateEntities )( void );
	void ( *HUD_DrawNormalTriangles )( void );
	void ( *HUD_DrawTransparentTriangles )( void );
	void ( *HUD_StudioEvent )( const struct mstudioevent_s *event, const struct cl_entity_s *entity );
	void ( *HUD_PostRunCmd )( struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed );
	void ( *HUD_Shutdown )( void );
	void ( *HUD_TxferLocalOverrides )( struct entity_state_s *state, const struct clientdata_s *client );
	void ( *HUD_ProcessPlayerState )( struct entity_state_s *dst, const struct entity_state_s *src );
	void ( *HUD_TxferPredictionData )( struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd );
	void ( *Demo_ReadBuffer )( int size, unsigned const char *buffer );
	int ( *HUD_ConnectionlessPacket )( struct netadr_s *net_from, const char *args, const char *response_buffer, int *response_buffer_size );
	int ( *HUD_GetHullBounds )( int hullnumber, float *mins, float *maxs );
	void ( *HUD_Frame )( double time );
	int ( *HUD_Key_Event )( int down, int keynum, const char *pszCurrentBinding );
	void ( *HUD_TempEntUpdate )( double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int ( *Callback_AddVisibleEntity )( struct cl_entity_s *pEntity ), void ( *Callback_TempEntPlaySound )( struct tempent_s *pTemp, float damp ) );
	struct cl_entity_s *( *HUD_GetUserEntity )( int index );
	void ( *HUD_VoiceStatus )( int entindex, qboolean bTalking );
	void ( *HUD_DirectorMessage )( unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags );
	int ( *HUD_GetStudioModelInterface )( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
	void ( *HUD_ChatInputPosition )( int *x, int *y );
	int ( *HUD_GetPlayerTeam )( int iplayer );
	void ( *ClientFactory )( void );

	// From Sven Cope, never called
	void ( *HUD_OnClientDisconnect )( void );
} cldll_func_t;

// eiface
#define INTERFACE_VERSION 140

//
// Defines entity interface between engine and DLLs.
// This header file included by engine files and DLL files.
//
// Before including this header, DLLs must:
//		include progdefs.h
// This is conveniently done for them in extdll.h
//

typedef enum
{
	at_notice,
	at_console,	  // same as at_notice, but forces a ConPrintf, not a message box
	at_aiconsole, // same as at_console, but only shown if developer level is 2!
	at_warning,
	at_error,
	at_logged // Server print to console ( only in multiplayer games ).
} ALERT_TYPE;

// 4-22-98  JOHN: added for use in pfnClientPrintf
typedef enum
{
	print_console,
	print_center,
	print_chat,
} PRINT_TYPE;

// For integrity checking of content on clients
typedef enum
{
	force_exactfile,					// File on client must exactly match server's file
	force_model_samebounds,				// For model files only, the geometry must fit in the same bbox
	force_model_specifybounds,			// For model files only, the geometry must fit in the specified bbox
	force_model_specifybounds_if_avail, // For Steam model files only, the geometry must fit in the specified bbox (if the file is available)
} FORCE_TYPE;

// Returned by TraceLine
typedef struct
{
	int fAllSolid;	 // if true, plane is not valid
	int fStartSolid; // if true, the initial point was in a solid area
	int fInOpen;
	int fInWater;
	float flFraction; // time completed, 1.0 = didn't hit anything
	Vector vecEndPos; // final position
	float flPlaneDist;
	Vector vecPlaneNormal; // surface normal at impact
	edict_t *pHit;		   // entity the surface is on
	int iHitgroup;		   // 0 == generic, non zero is specific body part
} TraceResult;

// CD audio status
typedef struct
{
	int fPlaying;	 // is sound playing right now?
	int fWasPlaying; // if not, CD is paused if WasPlaying is true.
	int fInitialized;
	int fEnabled;
	int fPlayLooping;
	float cdvolume;
	//byte 	remap[100];
	int fCDRom;
	int fPlayTrack;
} CDStatus;

// MD5 Hash
typedef struct
{
	unsigned int buf[ 4 ];
	unsigned int bits[ 2 ];
	unsigned char in[ 64 ];
} MD5Context_t;

typedef uint32 CRC32_t;

// Engine hands this to DLLs for functionality callbacks
typedef struct enginefuncs_s
{
	int ( *pfnPrecacheModel )( const char *s );
	int ( *pfnPrecacheSound )( const char *s );
	void ( *pfnSetModel )( edict_t *e, const char *m );
	int ( *pfnModelIndex )( const char *m );
	int ( *pfnModelFrames )( int modelIndex );
	void ( *pfnSetSize )( edict_t *e, const float *rgflMin, const float *rgflMax );
	void ( *pfnChangeLevel )( const char *s1, const char *s2 );
	void ( *pfnGetSpawnParms )( edict_t *ent );
	void ( *pfnSaveSpawnParms )( edict_t *ent );
	float ( *pfnVecToYaw )( const float *rgflVector );
	void ( *pfnVecToAngles )( const float *rgflVectorIn, float *rgflVectorOut );
	void ( *pfnMoveToOrigin )( edict_t *ent, const float *pflGoal, float dist, int iMoveType );
	void ( *pfnChangeYaw )( edict_t *ent );
	void ( *pfnChangePitch )( edict_t *ent );
	edict_t *( *pfnFindEntityByString )( edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue );
	int ( *pfnGetEntityIllum )( edict_t *pEnt );
	edict_t *( *pfnFindEntityInSphere )( edict_t *pEdictStartSearchAfter, const float *org, float rad );
	edict_t *( *pfnFindClientInPVS )( edict_t *pEdict );
	edict_t *( *pfnEntitiesInPVS )( edict_t *pplayer );
	void ( *pfnMakeVectors )( const float *rgflVector );
	void ( *pfnAngleVectors )( const float *rgflVector, float *forward, float *right, float *up );
	edict_t *( *pfnCreateEntity )( );
	void ( *pfnRemoveEntity )( edict_t *e );
	edict_t *( *pfnCreateNamedEntity )( int className );
	void ( *pfnMakeStatic )( edict_t *ent );
	int ( *pfnEntIsOnFloor )( edict_t *e );
	int ( *pfnDropToFloor )( edict_t *e );
	int ( *pfnWalkMove )( edict_t *ent, float yaw, float dist, int iMode );
	void ( *pfnSetOrigin )( edict_t *e, const float *rgflOrigin );
	void ( *pfnEmitSound )( edict_t *entity, int channel, const char *sample, /*int*/ float volume, float attenuation, int fFlags, int pitch );
	void ( *pfnEmitAmbientSound )( edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch );
	void ( *pfnTraceLine )( const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr );
	void ( *pfnTraceToss )( edict_t *pent, edict_t *pentToIgnore, TraceResult *ptr );
	int ( *pfnTraceMonsterHull )( edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr );
	void ( *pfnTraceHull )( const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr );
	void ( *pfnTraceModel )( const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr );
	const char *( *pfnTraceTexture )( edict_t *pTextureEntity, const float *v1, const float *v2 );
	void ( *pfnTraceSphere )( const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr );
	void ( *pfnGetAimVector )( edict_t *ent, float speed, float *rgflReturn );
	void ( *pfnServerCommand )( const char *str );
	void ( *pfnServerExecute )( );
	void ( *pfnClientCommand )( edict_t *pEdict, const char *szFmt, ... );
	void ( *pfnParticleEffect )( const float *org, const float *dir, float color, float count );
	void ( *pfnLightStyle )( int style, const char *val );
	int ( *pfnDecalIndex )( const char *name );
	int ( *pfnPointContents )( const float *rgflVector );
	void ( *pfnMessageBegin )( int msg_dest, int msg_type, const float *pOrigin, edict_t *ed );
	void ( *pfnMessageEnd )( );
	void ( *pfnWriteByte )( int iValue );
	void ( *pfnWriteChar )( int iValue );
	void ( *pfnWriteShort )( int iValue );
	void ( *pfnWriteLong )( int iValue );
	void ( *pfnWriteAngle )( float flValue );
	void ( *pfnWriteCoord )( float flValue );
	void ( *pfnWriteString )( const char *sz );
	void ( *pfnWriteEntity )( int iValue );
	void ( *pfnCVarRegister )( cvar_t *pCvar );
	float ( *pfnCVarGetFloat )( const char *szVarName );
	const char *( *pfnCVarGetString )( const char *szVarName );
	void ( *pfnCVarSetFloat )( const char *szVarName, float flValue );
	void ( *pfnCVarSetString )( const char *szVarName, const char *szValue );
	void ( *pfnAlertMessage )( ALERT_TYPE atype, const char *szFmt, ... );
	void ( *pfnEngineFprintf )( void *pfile, const char *szFmt, ... );
	void *( *pfnPvAllocEntPrivateData )( edict_t *pEdict, int32 cb );
	void *( *pfnPvEntPrivateData )( edict_t *pEdict );
	void ( *pfnFreeEntPrivateData )( edict_t *pEdict );
	const char *( *pfnSzFromIndex )( int iString );
	int ( *pfnAllocString )( const char *szValue );
	struct entvars_s *( *pfnGetVarsOfEnt )( edict_t *pEdict );
	edict_t *( *pfnPEntityOfEntOffset )( int iEntOffset );
	int ( *pfnEntOffsetOfPEntity )( const edict_t *pEdict );
	int ( *pfnIndexOfEdict )( const edict_t *pEdict );
	edict_t *( *pfnPEntityOfEntIndex )( int iEntIndex );
	edict_t *( *pfnFindEntityByVars )( struct entvars_s *pvars );
	void *( *pfnGetModelPtr )( edict_t *pEdict );
	int ( *pfnRegUserMsg )( const char *pszName, int iSize );
	void ( *pfnAnimationAutomove )( const edict_t *pEdict, float flTime );
	void ( *pfnGetBonePosition )( const edict_t *pEdict, int iBone, float *rgflOrigin, float *rgflAngles );
	uint32( *pfnFunctionFromName )( const char *pName );
	const char *( *pfnNameForFunction )( uint32 function );
	void ( *pfnClientPrintf )( edict_t *pEdict, PRINT_TYPE ptype, const char *szMsg ); // JOHN: engine callbacks so game DLL can print messages to individual clients
	void ( *pfnServerPrint )( const char *szMsg );
	const char *( *pfnCmd_Args )( );		  // these 3 added
	const char *( *pfnCmd_Argv )( int argc ); // so game DLL can easily
	int ( *pfnCmd_Argc )( );				  // access client 'cmd' strings
	void ( *pfnGetAttachment )( const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles );
	void ( *pfnCRC32_Init )( CRC32_t *pulCRC );
	void ( *pfnCRC32_ProcessBuffer )( CRC32_t *pulCRC, void *p, int len );
	void ( *pfnCRC32_ProcessByte )( CRC32_t *pulCRC, unsigned char ch );
	CRC32_t( *pfnCRC32_Final )( CRC32_t pulCRC );
	int32( *pfnRandomLong )( int32 lLow, int32 lHigh );
	float ( *pfnRandomFloat )( float flLow, float flHigh );
	void ( *pfnSetView )( const edict_t *pClient, const edict_t *pViewent );
	float ( *pfnTime )( );
	void ( *pfnCrosshairAngle )( const edict_t *pClient, float pitch, float yaw );
	byte *( *pfnLoadFileForMe )( const char *filename, int *pLength );
	void ( *pfnFreeFile )( void *buffer );
	void ( *pfnEndSection )( const char *pszSectionName ); // trigger_endsection
	int ( *pfnCompareFileTime )( const char *filename1, const char *filename2, int *iCompare );
	void ( *pfnGetGameDir )( char *szGetGameDir );
	void ( *pfnCvar_RegisterVariable )( cvar_t *variable );
	void ( *pfnFadeClientVolume )( const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds );
	void ( *pfnSetClientMaxspeed )( const edict_t *pEdict, float fNewMaxspeed );
	edict_t *( *pfnCreateFakeClient )( const char *netname ); // returns NULL if fake client can't be created
	void ( *pfnRunPlayerMove )( edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec );
	int ( *pfnNumberOfEntities )( );
	char *( *pfnGetInfoKeyBuffer )( edict_t *e ); // passing in NULL gets the serverinfo
	char *( *pfnInfoKeyValue )( char *infobuffer, const char *key );
	void ( *pfnSetKeyValue )( char *infobuffer, const char *key, const char *value );
	void ( *pfnSetClientKeyValue )( int clientIndex, char *infobuffer, const char *key, const char *value );
	int ( *pfnIsMapValid )( const char *filename );
	void ( *pfnStaticDecal )( const float *origin, int decalIndex, int entityIndex, int modelIndex );
	int ( *pfnPrecacheGeneric )( const char *s );
	int ( *pfnGetPlayerUserId )( edict_t *e ); // returns the server assigned userid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients
	void ( *pfnBuildSoundMsg )( edict_t *entity, int channel, const char *sample, /*int*/ float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed );
	int ( *pfnIsDedicatedServer )( ); // is this a dedicated server?
	cvar_t *( *pfnCVarGetPointer )( const char *szVarName );
	unsigned int ( *pfnGetPlayerWONId )( edict_t *e ); // returns the server assigned WONid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients

	// YWB 8/1/99 TFF Physics additions
	void ( *pfnInfo_RemoveKey )( char *s, const char *key );
	const char *( *pfnGetPhysicsKeyValue )( const edict_t *pClient, const char *key );
	void ( *pfnSetPhysicsKeyValue )( const edict_t *pClient, const char *key, const char *value );
	const char *( *pfnGetPhysicsInfoString )( const edict_t *pClient );
	unsigned short ( *pfnPrecacheEvent )( int type, const char *psz );
	void ( *pfnPlaybackEvent )( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, const float *origin, const float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

	unsigned char *( *pfnSetFatPVS )( float *org );
	unsigned char *( *pfnSetFatPAS )( float *org );

	int ( *pfnCheckVisibility )( const edict_t *entity, unsigned char *pset );

	void ( *pfnDeltaSetField )( struct delta_s *pFields, const char *fieldname );
	void ( *pfnDeltaUnsetField )( struct delta_s *pFields, const char *fieldname );
	void ( *pfnDeltaAddEncoder )( const char *name, void ( *conditionalencode )( struct delta_s *pFields, const unsigned char *from, const unsigned char *to ) );
	int ( *pfnGetCurrentPlayer )( );
	int ( *pfnCanSkipPlayer )( const edict_t *player );
	int ( *pfnDeltaFindField )( struct delta_s *pFields, const char *fieldname );
	void ( *pfnDeltaSetFieldByIndex )( struct delta_s *pFields, int fieldNumber );
	void ( *pfnDeltaUnsetFieldByIndex )( struct delta_s *pFields, int fieldNumber );

	void ( *pfnSetGroupMask )( int mask, int op );

	int ( *pfnCreateInstancedBaseline )( int classname, struct entity_state_s *baseline );
	void ( *pfnCvar_DirectSet )( struct cvar_s *var, const char *value );

	// Forces the client and server to be running with the same version of the specified file
	//  ( e.g., a player model ).
	// Calling this has no effect in single player
	void ( *pfnForceUnmodified )( FORCE_TYPE type, float *mins, float *maxs, const char *filename );

	void ( *pfnGetPlayerStats )( const edict_t *pClient, int *ping, int *packet_loss );

	void ( *pfnAddServerCommand )( const char *cmd_name, void ( *function )( ) );

	// For voice communications, set which clients hear eachother.
	// NOTE: these functions take player entity indices (starting at 1).
	qboolean( *pfnVoice_GetClientListening )( int iReceiver, int iSender );
	qboolean( *pfnVoice_SetClientListening )( int iReceiver, int iSender, qboolean bListen );

	const char *( *pfnGetPlayerAuthId )( edict_t *e );

	// PSV: Added for CZ training map
	//	const char *(*pfnKeyNameForBinding)		( const char* pBinding );

	sequenceEntry_s *( *pfnSequenceGet )( const char *fileName, const char *entryName );
	sentenceEntry_s *( *pfnSequencePickSentence )( const char *groupName, int pickMethod, int *picked );

	// LH: Give access to filesize via filesystem
	int ( *pfnGetFileSize )( const char *filename );

	unsigned int ( *pfnGetApproxWavePlayLen )( const char *filepath );
	// MDC: Added for CZ career-mode
	int ( *pfnIsCareerMatch )( );

	// BGC: return the number of characters of the localized string referenced by using "label"
	int ( *pfnGetLocalizedStringLength )( const char *label );

	// BGC: added to facilitate persistent storage of tutor message decay values for
	// different career game profiles.  Also needs to persist regardless of mp.dll being
	// destroyed and recreated.
	void ( *pfnRegisterTutorMessageShown )( int mid );
	int ( *pfnGetTimesTutorMessageShown )( int mid );
	void ( *ProcessTutorMessageDecayBuffer )( int *buffer, int bufferLength );
	void ( *ConstructTutorMessageDecayBuffer )( int *buffer, int bufferLength );
	void ( *ResetTutorMessageDecayData )( );

	void ( *pfnQueryClientCvarValue )( const edict_t *player, const char *cvarName );
	void ( *pfnQueryClientCvarValue2 )( const edict_t *player, const char *cvarName, int requestID );
	int ( *pfnCheckParm )( const char *pchCmdLineToken, const char **ppnext );
	edict_t *( *pfnPEntityOfEntIndexAllEntities )( int iEntIndex );
} enginefuncs_t;


// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.  INTERFACE VERSION IS FROZEN AT 138

// Passed to pfnKeyValue
typedef struct KeyValueData_s
{
	const char *szClassName; // in: entity classname
	const char *szKeyName;	 // in: name of key
	const char *szValue;	 // in: value of key
	int32 fHandled;			 // out: DLL sets to true if key-value pair was understood
} KeyValueData;


typedef struct
{
	char mapName[ 32 ];
	char landmarkName[ 32 ];
	edict_t *pentLandmark;
	Vector vecLandmarkOrigin;
} LEVELLIST;
#define MAX_LEVEL_CONNECTIONS 16 // These are encoded in the lower 16bits of ENTITYTABLE->flags

typedef struct
{
	int id;		   // Ordinal ID of this entity (used for entity <--> pointer conversions)
	edict_t *pent; // Pointer to the in-game entity

	int location;		// Offset from the base data of this entity
	int size;			// Byte size of this entity's data
	int flags;			// This could be a short -- bit mask of transitions that this entity is in the PVS of
	string_t classname; // entity class name

} ENTITYTABLE;

#define FENTTABLE_PLAYER 0x80000000
#define FENTTABLE_REMOVED 0x40000000
#define FENTTABLE_MOVEABLE 0x20000000
#define FENTTABLE_GLOBAL 0x10000000

typedef struct saverestore_s
{
	char *pBaseData;							// Start of all entity save data
	char *pCurrentData;							// Current buffer pointer for sequential access
	int size;									// Current data size
	int bufferSize;								// Total space for data
	int tokenSize;								// Size of the linear list of tokens
	int tokenCount;								// Number of elements in the pTokens table
	char **pTokens;								// Hash table of entity strings (sparse)
	int currentIndex;							// Holds a global entity table ID
	int tableCount;								// Number of elements in the entity table
	int connectionCount;						// Number of elements in the levelList[]
	ENTITYTABLE *pTable;						// Array of ENTITYTABLE elements (1 for each entity)
	LEVELLIST levelList[ MAX_LEVEL_CONNECTIONS ]; // List of connections from this level

	// smooth transition
	int fUseLandmark;
	char szLandmarkName[ 20 ];  // landmark we'll spawn near in next level
	Vector vecLandmarkOffset; // for landmark transitions
	float time;
	char szCurrentMapName[ 32 ]; // To check global entities

} SAVERESTOREDATA;

typedef enum _fieldtypes
{
	FIELD_FLOAT = 0,	   // Any floating point value
	FIELD_STRING,		   // A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,		   // An entity offset (EOFFSET)
	FIELD_CLASSPTR,		   // CBaseEntity *
	FIELD_EHANDLE,		   // Entity handle
	FIELD_EVARS,		   // EVARS *
	FIELD_EDICT,		   // edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,		   // Any vector
	FIELD_POSITION_VECTOR, // A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,		   // Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,		   // Any integer or enum
	FIELD_FUNCTION,		   // A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,		   // boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,		   // 2 byte integer
	FIELD_CHARACTER,	   // a byte
	FIELD_TIME,			   // a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,	   // Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,	   // Engine string that is a sound name (needs precache)

	FIELD_ENGINETYPECOUNT, //All preceding types are known to the engine and must remain.

	FIELD_INT64 = FIELD_ENGINETYPECOUNT, //64 bit integer

	FIELD_TYPECOUNT, // MUST BE LAST
} FIELDTYPE;

#define _FIELD(type, name, fieldtype, count, flags)                            \
	{                                                                          \
		fieldtype, #name, static_cast<int>(offsetof(type, name)), count, flags \
	}
#define DEFINE_FIELD(type, name, fieldtype) _FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type, name, fieldtype, count) _FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name, fieldtype) _FIELD(entvars_t, name, fieldtype, 1, 0)
#define DEFINE_ENTITY_GLOBAL_FIELD(name, fieldtype) _FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL)
#define DEFINE_GLOBAL_FIELD(type, name, fieldtype) _FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL)


#define FTYPEDESC_GLOBAL 0x0001 // This field is masked for global entity save/restore

typedef struct
{
	FIELDTYPE fieldType;
	const char *fieldName;
	int fieldOffset;
	short fieldSize;
	short flags;
} TYPEDESCRIPTION;

typedef struct
{
	// Initialize/shutdown the game (one-time call after loading of game .dll )
	void ( *pfnGameInit )( );
	int ( *pfnSpawn )( edict_t *pent );
	void ( *pfnThink )( edict_t *pent );
	void ( *pfnUse )( edict_t *pentUsed, edict_t *pentOther );
	void ( *pfnTouch )( edict_t *pentTouched, edict_t *pentOther );
	void ( *pfnBlocked )( edict_t *pentBlocked, edict_t *pentOther );
	void ( *pfnKeyValue )( edict_t *pentKeyvalue, KeyValueData *pkvd );
	void ( *pfnSave )( edict_t *pent, SAVERESTOREDATA *pSaveData );
	int ( *pfnRestore )( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
	void ( *pfnSetAbsBox )( edict_t *pent );

	void ( *pfnSaveWriteFields )( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );
	void ( *pfnSaveReadFields )( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );

	void ( *pfnSaveGlobalState )( SAVERESTOREDATA * );
	void ( *pfnRestoreGlobalState )( SAVERESTOREDATA * );
	void ( *pfnResetGlobalState )( );

	qboolean( *pfnClientConnect )( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );

	void ( *pfnClientDisconnect )( edict_t *pEntity );
	void ( *pfnClientKill )( edict_t *pEntity );
	void ( *pfnClientPutInServer )( edict_t *pEntity );
	void ( *pfnClientCommand )( edict_t *pEntity );
	void ( *pfnClientUserInfoChanged )( edict_t *pEntity, char *infobuffer );

	void ( *pfnServerActivate )( edict_t *pEdictList, int edictCount, int clientMax );
	void ( *pfnServerDeactivate )( );

	void ( *pfnPlayerPreThink )( edict_t *pEntity );
	void ( *pfnPlayerPostThink )( edict_t *pEntity );

	void ( *pfnStartFrame )( );
	void ( *pfnParmsNewLevel )( );
	void ( *pfnParmsChangeLevel )( );

	// Returns string describing current .dll.  E.g., TeamFotrress 2, Half-Life
	const char *( *pfnGetGameDescription )( );

	// Notify dll about a player customization.
	void ( *pfnPlayerCustomization )( edict_t *pEntity, customization_t *pCustom );

	// Spectator funcs
	void ( *pfnSpectatorConnect )( edict_t *pEntity );
	void ( *pfnSpectatorDisconnect )( edict_t *pEntity );
	void ( *pfnSpectatorThink )( edict_t *pEntity );

	// Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	void ( *pfnSys_Error )( const char *error_string );

	void ( *pfnPM_Move )( struct playermove_s *ppmove, qboolean server );
	void ( *pfnPM_Init )( struct playermove_s *ppmove );
	char ( *pfnPM_FindTextureType )( char *name );
	void ( *pfnSetupVisibility )( struct edict_s *pViewEntity, struct edict_s *pClient, unsigned char **pvs, unsigned char **pas );
	void ( *pfnUpdateClientData )( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd );
	int ( *pfnAddToFullPack )( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet );
	void ( *pfnCreateBaseline )( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, Vector *player_mins, Vector *player_maxs );
	void ( *pfnRegisterEncoders )( );
	int ( *pfnGetWeaponData )( struct edict_s *player, struct weapon_data_s *info );

	void ( *pfnCmdStart )( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
	void ( *pfnCmdEnd )( const edict_t *player );

	// Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	//  size of the response_buffer, so you must zero it out if you choose not to respond.
	int ( *pfnConnectionlessPacket )( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );

	// Enumerates player hulls.  Returns 0 if the hull number doesn't exist, 1 otherwise
	int ( *pfnGetHullBounds )( int hullnumber, float *mins, float *maxs );

	// Create baselines for certain "unplaced" items.
	void ( *pfnCreateInstancedBaselines )( );

	// One of the pfnForceUnmodified files failed the consistency check for the specified player
	// Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
	int ( *pfnInconsistentFile )( const struct edict_s *player, const char *filename, char *disconnect_message );

	// The game .dll should return 1 if lag compensation should be allowed ( could also just set
	//  the sv_unlag cvar.
	// Most games right now should return 0, until client-side weapon prediction code is written
	//  and tested for them.
	int ( *pfnAllowLagCompensation )( );
} DLL_FUNCTIONS;

extern DLL_FUNCTIONS gEntityInterface;

// Current version.
#define NEW_DLL_FUNCTIONS_VERSION 1

typedef struct
{
	// Called right before the object's memory is freed.
	// Calls its destructor.
	void ( *pfnOnFreeEntPrivateData )( edict_t *pEnt );
	void ( *pfnGameShutdown )( );
	int ( *pfnShouldCollide )( edict_t *pentTouched, edict_t *pentOther );
	void ( *pfnCvarValue )( const edict_t *pEnt, const char *value );
	void ( *pfnCvarValue2 )( const edict_t *pEnt, int requestID, const char *cvarName, const char *value );
} NEW_DLL_FUNCTIONS;
typedef int ( *NEW_DLL_FUNCTIONS_FN )( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

// Pointers will be null if the game DLL doesn't support this API.
extern NEW_DLL_FUNCTIONS gNewDLLFunctions;

typedef int ( *APIFUNCTION )( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
typedef int ( *APIFUNCTION2 )( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

/* Client HUD goes here
*/

#define HUD_PRINTNOTIFY 1
#define HUD_PRINTCONSOLE 2
#define HUD_PRINTTALK 3
#define HUD_PRINTCENTER 4

inline void UnpackRGB( unsigned char &r, unsigned char &g, unsigned char &b, unsigned long ulRGB )
{
	r = (unsigned char)( ( ulRGB & 0xFF0000 ) >> 16 );
	g = (unsigned char)( ( ulRGB & 0xFF00 ) >> 8 );
	b = (unsigned char)( ulRGB & 0xFF );
}

inline void UnpackRGBA( unsigned char &r, unsigned char &g, unsigned char &b, unsigned char &a, unsigned long ulRGBA )
{
	r = (unsigned char)( ( ulRGBA & 0xFF000000 ) >> 24 );
	g = (unsigned char)( ( ulRGBA & 0xFF0000 ) >> 16 );
	b = (unsigned char)( ( ulRGBA & 0xFF00 ) >> 8 );
	a = (unsigned char)( ulRGBA & 0xFF );
}

#define RGBA_DRAW_TEXT_DEFAULT 0xB4DCFFFF

#define RGB_SVENISH 0x006482C8  //100,130,200 sven copium
#define RGB_YELLOWISH 0x00FFA000 //255,160,0
#define RGB_REDISH 0x00FF1010 //255,16,16
#define RGB_GREENISH 0x0000A000 //0,160,0
#define RGB_BLUEISH 0x005F5FFF //95,95,255

#define DHN_DRAWZERO 1
#define DHN_2DIGITS 2
#define DHN_3DIGITS 4
#define MIN_ALPHA 100

#define HUDELEM_ACTIVE 1

typedef struct
{
	int x, y;
} POSITION;

#define HUD_ACTIVE 1
#define HUD_INTERMISSION 2

#define HIDEHUD_WEAPONS (1 << 0)
#define HIDEHUD_FLASHLIGHT (1 << 1)
#define HIDEHUD_ALL (1 << 2)
#define HIDEHUD_HEALTH (1 << 3)

#define MAX_PLAYER_NAME_LENGTH 32

#define MAX_MOTD_LENGTH 1536

//-----------------------------------------------------

class CHudBase
{
public:
	virtual ~CHudBase() {}
	virtual bool Init() { return false; }
	virtual bool VidInit() { return false; }
	virtual bool Draw( float flTime ) { return false; }
	virtual void Think() {}
	virtual void Reset() {}
	virtual void InitHUDData() {} // called every time a server is connected to

public:
	POSITION m_pos;
	int m_type;
	int m_iFlags; // active, moving,
};

struct HUDLIST
{
	CHudBase *p;
	HUDLIST *pNext;
};

//
//-----------------------------------------------------
//

class CHudSpectator;
class CHudAmmo;
class CHudAmmoSecondary;
class CHudHealth;
class CHudGeiger;
class CHudTrain;
class CHudStatusBar;
class CHudDeathNotice;
class CHudMenu;
class CHudBattery;
class CHudFlashlight;
class CHudTextMessage;
class CHudMessage;
class CHudStatusIcons;

class CHudBaseTextBlock : public CHudBase
{
public:
	bool Init() override;
	bool VidInit() override;
	bool Draw( float flTime ) override;
	// CHudBase::Think - not overriden
	// CHudBase::Reset - not overriden
	void InitHUDData() override;
	virtual void FlushText();
};

class CHudSayText : public CHudBaseTextBlock
{
public:
	bool Init() override;
	// CHudBaseTextBlock::VidInit() - not overriden
	bool Draw( float flTime ) override;
	// CHudBase::Think - not overriden
	// CHudBase::Reset - not overriden
	// CHudBaseTextBlock::InitHUDData - not overriden
	// CHudBaseTextBlock::FlushText - not overriden
	virtual float GetScrollTime();
	// @messageType: 0 - client, 1 - server. Should be more? I didn't go further lol
	virtual void PrintText( int messageType, const char *pszBuf, int iBufSize, int clientIndex );
	virtual void UpdatePos();
};

class CHud
{
private:
	HUDLIST *m_pHudList;
};

//-----------------------------------------------------

#ifndef INSET_OFF
#define INSET_OFF 0
#define INSET_CHASE_FREE 1
#define INSET_IN_EYE 2
#define INSET_MAP_FREE 3
#define INSET_MAP_CHASE 4
#endif

#define MAX_SPEC_HUD_MESSAGES 8

#define OVERVIEW_TILE_SIZE 128 // don't change this
#define OVERVIEW_MAX_LAYERS 1

//-----------------------------------------------------

#define DMG_IMAGE_LIFE 2 // seconds that image is up

#define DMG_IMAGE_POISON 0
#define DMG_IMAGE_ACID 1
#define DMG_IMAGE_COLD 2
#define DMG_IMAGE_DROWN 3
#define DMG_IMAGE_BURN 4
#define DMG_IMAGE_NERVE 5
#define DMG_IMAGE_RAD 6
#define DMG_IMAGE_SHOCK 7
//tf defines
#define DMG_IMAGE_CALTROP 8
#define DMG_IMAGE_TRANQ 9
#define DMG_IMAGE_CONCUSS 10
#define DMG_IMAGE_HALLUC 11
#define NUM_DMG_TYPES 12

#endif //HLSDK_MINI_HPP_GUARD

#undef HLSDK_MINI_HPP_RECURSE_GUARD
#endif //HLSDK_MINI_HPP_RECURSE_GUARD

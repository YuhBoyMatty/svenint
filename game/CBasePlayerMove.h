// SvenInt (c) Sw1ft
// CBasePlayerMove.h

#ifndef SINT_CBASEPLAYERMOVE_H
#define SINT_CBASEPLAYERMOVE_H

#ifdef _WIN32
#pragma once
#endif

#include "game/hlsdk_mini.h"

//-----------------------------------------------------------------------------
// Purpose: abstract class CBasePlayerMove that provides
// compatibility with different SC versions
//-----------------------------------------------------------------------------

class CBasePlayerMove
{
public:
	virtual					~CBasePlayerMove() {}

	virtual void			*GetStruct( void ) { return 0; }
	virtual int				GetCompatibilityVersion( void ) { return 0; }
	virtual int				GetMaxPhysEnts( void ) { return MAX_PHYSENTS; }

	virtual int				player_index( void ) { return 0; } // So we don't try to run the PM_CheckStuck nudging too quickly.
	virtual qboolean		server( void ) { return 0; }        // For debugging, are we running physics code on server side?

	virtual qboolean		multiplayer( void ) { return 0; }   // 1 == multiplayer server
	virtual float			time( void ) { return 0.f; }          // realtime on host, for reckoning duck timing
	virtual float			frametime( void ) { return 0.f; }	   // Duration of this frame
	
	// Vectors for angles
	virtual vec3_t			*forward( void ) { return 0; }
	virtual vec3_t			*right( void ) { return 0; }
	virtual vec3_t			*up( void ) { return 0; }
	// player state
	virtual vec3_t			*origin( void ) { return 0; }        // Movement origin.
	virtual vec3_t			*angles( void ) { return 0; }        // Movement view angles.
	virtual vec3_t			*oldangles( void ) { return 0; }     // Angles before movement view angles were looked at.
	virtual vec3_t			*velocity( void ) { return 0; }      // Current movement direction.
	virtual vec3_t			*movedir( void ) { return 0; }       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	virtual vec3_t			*basevelocity( void ) { return 0; }  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	virtual vec3_t			*view_ofs( void ) { return 0; }      // Our eye position.
	virtual float			flDuckTime( void ) { return 0.f; }    // Time we started duck
	virtual qboolean		bInDuck( void ) { return 0; }       // In process of ducking or ducked already?

	// For walking/falling
	virtual int				flTimeStepSound( void ) { return 0; }  // Next time we can play a step sound
	virtual int				iStepLeft( void ) { return 0; }

	virtual float			flFallVelocity( void ) { return 0.f; }
	virtual vec3_t			*punchangle( void ) { return 0; }

	virtual float			flSwimTime( void ) { return 0.f; }

	virtual float			flNextPrimaryAttack( void ) { return 0.f; }

	virtual int				effects( void ) { return 0; }		// MUZZLE FLASH, e.g.

	virtual int				flags( void ) { return 0; }         // FL_ONGROUND, FL_DUCKING, etc.
	virtual int				usehull( void ) { return 0; }       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	virtual void			setusehull( int hull ) { }       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	virtual float			gravity( void ) { return 0.f; }       // Our current gravity and friction.
	virtual float			friction( void ) { return 0.f; }
	virtual int				oldbuttons( void ) { return 0; }    // Buttons last usercmd
	virtual float			waterjumptime( void ) { return 0.f; } // Amount of time left in jumping out of water cycle.
	virtual qboolean		dead( void ) { return 0; }          // Are we a dead player?
	virtual int				deadflag( void ) { return 0; }
	virtual int				spectator( void ) { return 0; }     // Should we use spectator physics model?
	virtual int				movetype( void ) { return 0; }      // Our movement type, NOCLIP, WALK, FLY

	virtual int				onground( void ) { return 0; }
	virtual int				waterlevel( void ) { return 0; }
	virtual int				watertype( void ) { return 0; }
	virtual int				oldwaterlevel( void ) { return 0; }

	virtual char			*sztexturename( void ) { return 0; } // sztexturename[ 256 ]
	virtual char			chtexturetype( void ) { return 0; }

	virtual float			maxspeed( void ) { return 0.f; }
	virtual float			clientmaxspeed( void ) { return 0.f; } // Player specific maxspeed

	// For mods
	virtual int				iuser1( void ) { return 0; }
	virtual int				iuser2( void ) { return 0; }
	virtual int				iuser3( void ) { return 0; }
	virtual int				iuser4( void ) { return 0; }
	virtual float			fuser1( void ) { return 0.f; }
	virtual float			fuser2( void ) { return 0.f; }
	virtual float			fuser3( void ) { return 0.f; }
	virtual float			fuser4( void ) { return 0.f; }
	virtual vec3_t			*vuser1( void ) { return 0; }
	virtual vec3_t			*vuser2( void ) { return 0; }
	virtual vec3_t			*vuser3( void ) { return 0; }
	virtual vec3_t			*vuser4( void ) { return 0; }

	virtual int				numphysent( void ) { return 0; }
	virtual physent_t		*physents( void ) { return 0; } // physents[ MAX_PHYSENTS ]
	// Number of momvement entities (ladders)
	virtual int				nummoveent( void ) { return 0; }
	// just a list of ladders
	virtual physent_t		*moveents( void ) { return 0; } // moveents[ MAX_MOVEENTS ]

	// All things being rendered, for tracing against things you don't actually collide with
	virtual int				numvisent( void ) { return 0; }
	virtual physent_t		*visents( void ) { return 0; } // visents[ MAX_PHYSENTS ]

	// input to run through physics.
	virtual usercmd_t		*cmd( void ) { return 0; }

	// Trace results for objects we collided with.
	virtual int				numtouch( void ) { return 0; }
	virtual pmtrace_t		*touchindex( void ) { return 0; } // touchindex[ MAX_PHYSENTS ]

	virtual char			*physinfo( void ) { return 0; } // physinfo[ MAX_PHYSINFO_STRING ]

	virtual struct movevars_s *movevars( void ) { return 0; }
	virtual vec3_t			*player_mins( void ) { return 0; } // player_mins[ 4 ]
	virtual vec3_t			*player_maxs( void ) { return 0; } // player_maxs[ 4 ]

	virtual playermove_funcs_t *funcs( void ) { return 0; }
};

//-----------------------------------------------------------------------------
// Get CBasePlayerMove based on SC version
//-----------------------------------------------------------------------------

CBasePlayerMove *GetBasePlayerMove( void *pPlayerMove, int iGameVersion );

#endif // SINT_CBASEPLAYERMOVE_H

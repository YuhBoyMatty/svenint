// SvenInt (c) Sw1ft
// CBasePlayerMove.cpp

#include "stdafx.h"
#include "CBasePlayerMove.h"

//-----------------------------------------------------------------------------
// Dummy player move
//-----------------------------------------------------------------------------

static CBasePlayerMove gPlayerMoveDummy;

//-----------------------------------------------------------------------------
// CPlayerMove < SC 5.26
//-----------------------------------------------------------------------------

class CPlayerMove final : public CBasePlayerMove
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<playermove_t *>( pStruct ); }

	virtual void			*GetStruct( void ) override { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 500; }

	virtual int				player_index( void ) { return m_pStruct->player_index; } // So we don't try to run the PM_CheckStuck nudging too quickly.
	virtual qboolean		server( void ) { return m_pStruct->server; }        // For debugging, are we running physics code on server side?

	virtual qboolean		multiplayer( void ) { return m_pStruct->multiplayer; }   // 1 == multiplayer server
	virtual float			time( void ) { return m_pStruct->time; }          // realtime on host, for reckoning duck timing
	virtual float			frametime( void ) { return m_pStruct->frametime; }	   // Duration of this frame
	
	// Vectors for angles
	virtual vec3_t			*forward( void ) { return &m_pStruct->forward; }
	virtual vec3_t			*right( void ) { return &m_pStruct->right; }
	virtual vec3_t			*up( void ) { return &m_pStruct->up; }
	// player state
	virtual vec3_t			*origin( void ) { return &m_pStruct->origin; }        // Movement origin.
	virtual vec3_t			*angles( void ) { return &m_pStruct->angles; }        // Movement view angles.
	virtual vec3_t			*oldangles( void ) { return &m_pStruct->oldangles; }     // Angles before movement view angles were looked at.
	virtual vec3_t			*velocity( void ) { return &m_pStruct->velocity; }      // Current movement direction.
	virtual vec3_t			*movedir( void ) { return &m_pStruct->movedir; }       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	virtual vec3_t			*basevelocity( void ) { return &m_pStruct->basevelocity; }  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	virtual vec3_t			*view_ofs( void ) { return &m_pStruct->view_ofs; }      // Our eye position.
	virtual float			flDuckTime( void ) { return m_pStruct->flDuckTime; }    // Time we started duck
	virtual qboolean		bInDuck( void ) { return m_pStruct->bInDuck; }       // In process of ducking or ducked already?

	// For walking/falling
	virtual int				flTimeStepSound( void ) { return m_pStruct->flTimeStepSound; }  // Next time we can play a step sound
	virtual int				iStepLeft( void ) { return m_pStruct->iStepLeft; }

	virtual float			flFallVelocity( void ) { return m_pStruct->flFallVelocity; }
	virtual vec3_t			*punchangle( void ) { return &m_pStruct->punchangle; }

	virtual float			flSwimTime( void ) { return m_pStruct->flSwimTime; }

	virtual float			flNextPrimaryAttack( void ) { return m_pStruct->flNextPrimaryAttack; }

	virtual int				effects( void ) { return m_pStruct->effects; }		// MUZZLE FLASH, e.g.

	virtual int				flags( void ) { return m_pStruct->flags; }         // FL_ONGROUND, FL_DUCKING, etc.
	virtual int				usehull( void ) { return m_pStruct->usehull; }       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	virtual void			setusehull( int hull ) { m_pStruct->usehull = hull; }
	virtual float			gravity( void ) { return m_pStruct->gravity; }       // Our current gravity and friction.
	virtual float			friction( void ) { return m_pStruct->friction; }
	virtual int				oldbuttons( void ) { return m_pStruct->oldbuttons; }    // Buttons last usercmd
	virtual float			waterjumptime( void ) { return m_pStruct->waterjumptime; } // Amount of time left in jumping out of water cycle.
	virtual qboolean		dead( void ) { return m_pStruct->dead; }          // Are we a dead player?
	virtual int				deadflag( void ) { return m_pStruct->deadflag; }
	virtual int				spectator( void ) { return m_pStruct->spectator; }     // Should we use spectator physics model?
	virtual int				movetype( void ) { return m_pStruct->movetype; }      // Our movement type, NOCLIP, WALK, FLY

	virtual int				onground( void ) { return m_pStruct->onground; }
	virtual int				waterlevel( void ) { return m_pStruct->waterlevel; }
	virtual int				watertype( void ) { return m_pStruct->watertype; }
	virtual int				oldwaterlevel( void ) { return m_pStruct->oldwaterlevel; }

	virtual char			*sztexturename( void ) { return m_pStruct->sztexturename; } // sztexturename[ 256 ]
	virtual char			chtexturetype( void ) { return m_pStruct->chtexturetype; }

	virtual float			maxspeed( void ) { return m_pStruct->maxspeed; }
	virtual float			clientmaxspeed( void ) { return m_pStruct->clientmaxspeed; } // Player specific maxspeed

	// For mods
	virtual int				iuser1( void ) { return m_pStruct->iuser1; }
	virtual int				iuser2( void ) { return m_pStruct->iuser2; }
	virtual int				iuser3( void ) { return m_pStruct->iuser3; }
	virtual int				iuser4( void ) { return m_pStruct->iuser4; }
	virtual float			fuser1( void ) { return m_pStruct->fuser1; }
	virtual float			fuser2( void ) { return m_pStruct->fuser2; }
	virtual float			fuser3( void ) { return m_pStruct->fuser3; }
	virtual float			fuser4( void ) { return m_pStruct->fuser4; }
	virtual vec3_t			*vuser1( void ) { return &m_pStruct->vuser1; }
	virtual vec3_t			*vuser2( void ) { return &m_pStruct->vuser2; }
	virtual vec3_t			*vuser3( void ) { return &m_pStruct->vuser3; }
	virtual vec3_t			*vuser4( void ) { return &m_pStruct->vuser4; }

	virtual int				numphysent( void ) { return m_pStruct->numphysent; }
	virtual physent_t		*physents( void ) { return m_pStruct->physents; } // physents[ MAX_PHYSENTS ]
	// Number of momvement entities (ladders)
	virtual int				nummoveent( void ) { return m_pStruct->nummoveent; }
	// just a list of ladders
	virtual physent_t		*moveents( void ) { return m_pStruct->moveents; } // moveents[ MAX_MOVEENTS ]

	// All things being rendered, for tracing against things you don't actually collide with
	virtual int				numvisent( void ) { return m_pStruct->numvisent; }
	virtual physent_t		*visents( void ) { return m_pStruct->visents; } // visents[ MAX_PHYSENTS ]

	// input to run through physics.
	virtual usercmd_t		*cmd( void ) { return &m_pStruct->cmd; }

	// Trace results for objects we collided with.
	virtual int				numtouch( void ) { return m_pStruct->bInDuck; }
	virtual pmtrace_t		*touchindex( void ) { return m_pStruct->touchindex; } // touchindex[ MAX_PHYSENTS ]

	virtual char			*physinfo( void ) { return m_pStruct->physinfo; } // physinfo[ MAX_PHYSINFO_STRING ]

	virtual struct movevars_s *movevars( void ) { return m_pStruct->movevars; }
	virtual vec3_t			*player_mins( void ) { return m_pStruct->player_mins; } // player_mins[ 4 ]
	virtual vec3_t			*player_maxs( void ) { return m_pStruct->player_maxs; } // player_maxs[ 4 ]

	virtual playermove_funcs_t *funcs( void ) { return &m_pStruct->funcs; }

private:
	playermove_t *m_pStruct;
};

static CPlayerMove gPlayerMove;

//-----------------------------------------------------------------------------
// CPlayerMove_5_26
//-----------------------------------------------------------------------------

class CPlayerMove_5_26 final : public CBasePlayerMove
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<playermove_5_26_t *>( pStruct ); }

	virtual void			*GetStruct( void ) override { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 526; }
	virtual int				GetMaxPhysEnts( void ) override { return MAX_PHYSENTS_5_26; }

	virtual int				player_index( void ) { return m_pStruct->player_index; } // So we don't try to run the PM_CheckStuck nudging too quickly.
	virtual qboolean		server( void ) { return m_pStruct->server; }        // For debugging, are we running physics code on server side?

	virtual qboolean		multiplayer( void ) { return m_pStruct->multiplayer; }   // 1 == multiplayer server
	virtual float			time( void ) { return m_pStruct->time; }          // realtime on host, for reckoning duck timing
	virtual float			frametime( void ) { return m_pStruct->frametime; }	   // Duration of this frame
	
	// Vectors for angles
	virtual vec3_t			*forward( void ) { return &m_pStruct->forward; }
	virtual vec3_t			*right( void ) { return &m_pStruct->right; }
	virtual vec3_t			*up( void ) { return &m_pStruct->up; }
	// player state
	virtual vec3_t			*origin( void ) { return &m_pStruct->origin; }        // Movement origin.
	virtual vec3_t			*angles( void ) { return &m_pStruct->angles; }        // Movement view angles.
	virtual vec3_t			*oldangles( void ) { return &m_pStruct->oldangles; }     // Angles before movement view angles were looked at.
	virtual vec3_t			*velocity( void ) { return &m_pStruct->velocity; }      // Current movement direction.
	virtual vec3_t			*movedir( void ) { return &m_pStruct->movedir; }       // For waterjumping, a forced forward velocity so we can fly over lip of ledge.
	virtual vec3_t			*basevelocity( void ) { return &m_pStruct->basevelocity; }  // Velocity of the conveyor we are standing, e.g.

	// For ducking/dead
	virtual vec3_t			*view_ofs( void ) { return &m_pStruct->view_ofs; }      // Our eye position.
	virtual float			flDuckTime( void ) { return m_pStruct->flDuckTime; }    // Time we started duck
	virtual qboolean		bInDuck( void ) { return m_pStruct->bInDuck; }       // In process of ducking or ducked already?

	// For walking/falling
	virtual int				flTimeStepSound( void ) { return m_pStruct->flTimeStepSound; }  // Next time we can play a step sound
	virtual int				iStepLeft( void ) { return m_pStruct->iStepLeft; }

	virtual float			flFallVelocity( void ) { return m_pStruct->flFallVelocity; }
	virtual vec3_t			*punchangle( void ) { return &m_pStruct->punchangle; }

	virtual float			flSwimTime( void ) { return m_pStruct->flSwimTime; }

	virtual float			flNextPrimaryAttack( void ) { return m_pStruct->flNextPrimaryAttack; }

	virtual int				effects( void ) { return m_pStruct->effects; }		// MUZZLE FLASH, e.g.

	virtual int				flags( void ) { return m_pStruct->flags; }         // FL_ONGROUND, FL_DUCKING, etc.
	virtual int				usehull( void ) { return m_pStruct->usehull; }       // 0 = regular player hull, 1 = ducked player hull, 2 = point hull
	virtual void			setusehull( int hull ) { m_pStruct->usehull = hull; }
	virtual float			gravity( void ) { return m_pStruct->gravity; }       // Our current gravity and friction.
	virtual float			friction( void ) { return m_pStruct->friction; }
	virtual int				oldbuttons( void ) { return m_pStruct->oldbuttons; }    // Buttons last usercmd
	virtual float			waterjumptime( void ) { return m_pStruct->waterjumptime; } // Amount of time left in jumping out of water cycle.
	virtual qboolean		dead( void ) { return m_pStruct->dead; }          // Are we a dead player?
	virtual int				deadflag( void ) { return m_pStruct->deadflag; }
	virtual int				spectator( void ) { return m_pStruct->spectator; }     // Should we use spectator physics model?
	virtual int				movetype( void ) { return m_pStruct->movetype; }      // Our movement type, NOCLIP, WALK, FLY

	virtual int				onground( void ) { return m_pStruct->onground; }
	virtual int				waterlevel( void ) { return m_pStruct->waterlevel; }
	virtual int				watertype( void ) { return m_pStruct->watertype; }
	virtual int				oldwaterlevel( void ) { return m_pStruct->oldwaterlevel; }

	virtual char			*sztexturename( void ) { return m_pStruct->sztexturename; } // sztexturename[ 256 ]
	virtual char			chtexturetype( void ) { return m_pStruct->chtexturetype; }

	virtual float			maxspeed( void ) { return m_pStruct->maxspeed; }
	virtual float			clientmaxspeed( void ) { return m_pStruct->clientmaxspeed; } // Player specific maxspeed

	// For mods
	virtual int				iuser1( void ) { return m_pStruct->iuser1; }
	virtual int				iuser2( void ) { return m_pStruct->iuser2; }
	virtual int				iuser3( void ) { return m_pStruct->iuser3; }
	virtual int				iuser4( void ) { return m_pStruct->iuser4; }
	virtual float			fuser1( void ) { return m_pStruct->fuser1; }
	virtual float			fuser2( void ) { return m_pStruct->fuser2; }
	virtual float			fuser3( void ) { return m_pStruct->fuser3; }
	virtual float			fuser4( void ) { return m_pStruct->fuser4; }
	virtual vec3_t			*vuser1( void ) { return &m_pStruct->vuser1; }
	virtual vec3_t			*vuser2( void ) { return &m_pStruct->vuser2; }
	virtual vec3_t			*vuser3( void ) { return &m_pStruct->vuser3; }
	virtual vec3_t			*vuser4( void ) { return &m_pStruct->vuser4; }

	virtual int				numphysent( void ) { return m_pStruct->numphysent; }
	virtual physent_t		*physents( void ) { return m_pStruct->physents; } // physents[ MAX_PHYSENTS ]
	// Number of momvement entities (ladders)
	virtual int				nummoveent( void ) { return m_pStruct->nummoveent; }
	// just a list of ladders
	virtual physent_t		*moveents( void ) { return m_pStruct->moveents; } // moveents[ MAX_MOVEENTS ]

	// All things being rendered, for tracing against things you don't actually collide with
	virtual int				numvisent( void ) { return m_pStruct->numvisent; }
	virtual physent_t		*visents( void ) { return m_pStruct->visents; } // visents[ MAX_PHYSENTS ]

	// input to run through physics.
	virtual usercmd_t		*cmd( void ) { return &m_pStruct->cmd; }

	// Trace results for objects we collided with.
	virtual int				numtouch( void ) { return m_pStruct->bInDuck; }
	virtual pmtrace_t		*touchindex( void ) { return m_pStruct->touchindex; } // touchindex[ MAX_PHYSENTS ]

	virtual char			*physinfo( void ) { return m_pStruct->physinfo; } // physinfo[ MAX_PHYSINFO_STRING ]

	virtual struct movevars_s *movevars( void ) { return m_pStruct->movevars; }
	virtual vec3_t			*player_mins( void ) { return m_pStruct->player_mins; } // player_mins[ 4 ]
	virtual vec3_t			*player_maxs( void ) { return m_pStruct->player_maxs; } // player_maxs[ 4 ]

	virtual playermove_funcs_t *funcs( void ) { return &m_pStruct->funcs; }

private:
	playermove_5_26_t *m_pStruct;
};

static CPlayerMove_5_26 gPlayerMove_5_26;

//-----------------------------------------------------------------------------
// Get CBasePlayerMove based on SC version
//-----------------------------------------------------------------------------

CBasePlayerMove *GetBasePlayerMove( void *pPlayerMove, int iGameVersion )
{
	if ( pPlayerMove == NULL || iGameVersion == 0 )
		return &gPlayerMoveDummy;

	if ( iGameVersion >= SVEN_VERSION_CHECK( 5, 26, 0 ) )
	{
		gPlayerMove_5_26.Init( pPlayerMove );
		return &gPlayerMove_5_26;
	}
	
	gPlayerMove.Init( pPlayerMove );
	return &gPlayerMove;
}
// SvenInt (c) Sw1ft
// server.h

#ifndef SINT_SERVER_MODULE_H
#define SINT_SERVER_MODULE_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// SvenInt user message
//-----------------------------------------------------------------------------

//#define SVC_SVENINT							( 146 ) // SvenInt user message
extern int SVC_SVENINT; // SvenInt user message

#define SVENINT_COMM_SETCVAR				( 0 )
#define SVENINT_COMM_EXECUTE				( 1 )
#define SVENINT_COMM_TIMER					( 2 )
#define SVENINT_COMM_TIMESCALE				( 3 )
#define SVENINT_COMM_DISPLAY_PLAYER_HULL	( 4 )
#define SVENINT_COMM_SCRIPTS				( 5 )

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class CBaseEntity;
class CBasePlayer;

//-----------------------------------------------------------------------------
// Server module
//-----------------------------------------------------------------------------

class CServerModule
{
public:
	CServerModule();

	bool			Init( void );
	void			Shutdown( void );

public:
	bool			Host_IsServerActive( void );

	bool			FNullEnt( struct edict_s *pEntity );
	bool			IsValidEntity( struct edict_s *pEntity );

	bool			IsSurvivalModeEnabled( void );
	bool			EnableSurvivalMode( void );
	bool			DisableSurvivalMode( void );

public:
	// Callbacks
	void OnServerInfo( void );
	void OnClientPutInServer( struct edict_s *pPlayer );

private:
	void			*GetSurvivalModeInstance( void );
};

namespace Modules { extern CServerModule *server; }

#endif // SINT_SERVER_MODULE_H

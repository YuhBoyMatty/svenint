// SvenInt (c) Sw1ft
// cvar.h

#ifndef SINT_CVAR_H
#define SINT_CVAR_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include "color.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class ConVar;
class ConCommand;
class ConCommandBase;

//-----------------------------------------------------------------------------
// DLL's identifier
//-----------------------------------------------------------------------------

typedef int CVarDLLIdentifier_t;

//-----------------------------------------------------------------------------
// ConCommandBase hash
//-----------------------------------------------------------------------------

class CConCommandHash
{
	friend class CCvar;

public:
	~CConCommandHash( void );

	void Init( void );

	unsigned int Hash( ConCommandBase *pCommand );
	unsigned int Hash( const char *pszCommand );

	ConCommandBase *Find( ConCommandBase *pCommand );
	ConCommandBase *Find( const char *pszName );

	bool Insert( ConCommandBase *pCommand );

	bool Remove( ConCommandBase *pCommand );
	bool Remove( const char *pszCommand );

	void RemoveAll( void );

	int Size( void );

protected:
	typedef std::vector<ConCommandBase *> datapool_t;

	enum
	{
		NUM_BUCKETS = 511,
		NUM_BUCKETS_MASK = NUM_BUCKETS
	};

	datapool_t m_Buckets[ NUM_BUCKETS ];
};

//-----------------------------------------------------------------------------
// Purpose: interface to ConVars/ConCommands (mostly for them) and engine's cvars/cmds
//-----------------------------------------------------------------------------

class CCvar
{
public:
	CCvar( void );
	~CCvar( void );

	bool				Init( void );
	void				Shutdown( void );

	void				PrintCvars( int mode, const char *pszPrefix );

	CVarDLLIdentifier_t AllocateDLLIdentifier( void );

	bool				RegisterConCommand( ConCommandBase *pCommandBase );
	bool				UnregisterConCommand( ConCommandBase *pCommandBase );
	void				UnregisterConCommands( CVarDLLIdentifier_t id );
	void				UnregisterConCommands( void );

	const char			*GetCommandLineValue( const char *pszVariableName );

	struct cvar_s		*FindCvar( const char *pszName );
	struct cmd_function_s *FindCmd( const char *pszName );
	ConCommandBase		*FindCommandBase( const char *pszName );
	ConVar				*FindVar( const char *pszName );
	ConCommand			*FindCommand( const char *pszName );

	void				RevertFlaggedConVars( int nFlag );

	int					ArgC( void ) const;
	const char			**ArgV( void ) const;
	const char			*Arg( int nIndex ) const;

	void				SetValue( struct cvar_s *pCvar, const char *value );
	void				SetValue( struct cvar_s *pCvar, float value );
	void				SetValue( struct cvar_s *pCvar, int value );
	void				SetValue( struct cvar_s *pCvar, bool value );
	void				SetValue( struct cvar_s *pCvar, Color value );

	void				SetValue( const char *pszCvar, const char *value );
	void				SetValue( const char *pszCvar, float value );
	void				SetValue( const char *pszCvar, int value );
	void				SetValue( const char *pszCvar, bool value );
	void				SetValue( const char *pszCvar, Color value );

	const char			*GetStringFromCvar( struct cvar_s *pCvar );
	float				GetFloatFromCvar( struct cvar_s *pCvar );
	int					GetIntFromCvar( struct cvar_s *pCvar );
	bool				GetBoolFromCvar( struct cvar_s *pCvar );
	Color				GetColorFromCvar( struct cvar_s *pCvar );

	const char			*GetStringFromCvar( const char *pszName );
	float				GetFloatFromCvar( const char *pszName );
	int					GetIntFromCvar( const char *pszName );
	bool				GetBoolFromCvar( const char *pszName );
	Color				GetColorFromCvar( const char *pszName );

public:
	bool						IsInitialized( void );

private:
	bool						m_bInitialized;
	mutable CConCommandHash		m_CommandHash;
	CVarDLLIdentifier_t			m_nNextDLLIdentifier;
};

CCvar *CVar();

#endif // SINT_CVAR_H
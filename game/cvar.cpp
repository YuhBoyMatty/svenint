// SvenInt (c) Sw1ft
// cvar.cpp

#include "stdafx.h"
#include "cvar.h"

#include "utils/generichash.h"
#include "game/dbg.h"
#include "game/convar.h"

#include <string>
#include <algorithm>

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

//#ifdef _CPPRTTI
//#define CVAR_USE_DYNAMIC_CAST 1
//#else
#define CVAR_USE_DYNAMIC_CAST 0
//#endif

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef void ( CALLCONV_CDECL *Z_FreeFn )( void * );
typedef void ( CALLCONV_CDECL *Mem_FreeFn )( void * );
typedef void ( CALLCONV_CDECL *Cvar_DirectSetFn )( cvar_t *, const char * );

//-----------------------------------------------------------------------------
// Export
//-----------------------------------------------------------------------------

static CCvar gCvar;
CCvar *CVar()
{
	return &gCvar;
}

//-----------------------------------------------------------------------------
// CCvar implementation
//-----------------------------------------------------------------------------

CCvar::CCvar( void )
{
	m_bInitialized = false;
	m_nNextDLLIdentifier = 0;
}

CCvar::~CCvar( void )
{
	Shutdown();
}

bool CCvar::IsInitialized( void )
{
	return m_bInitialized;
}

bool CCvar::Init( void )
{
	if ( !m_bInitialized &&
		 Globals::cmd_argc != NULL &&
		 Globals::cmd_argv != NULL &&
		 Globals::cvar_vars != NULL &&
		 Globals::cmd_functions != NULL &&
		 Globals::cl_enginefuncs != NULL &&
		 GameData::Pointers::Engine::Z_Free != NULL &&
		 GameData::Pointers::Engine::Cvar_DirectSet != NULL )
	{
		m_bInitialized = true;
	}

	return m_bInitialized;
}

void CCvar::Shutdown( void )
{
	if ( !IsInitialized() )
		return;

	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
			ConCommandBase *pCommandBase = bucket[ j ];

			if ( pCommandBase->IsRegistered() )
			{
				bool bFound = false;

				if ( pCommandBase->IsCommand() )
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConCommand *pCommand = dynamic_cast<ConCommand *>( pCommandBase );

					if ( !pCommand )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConVar, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConCommand *pCommand = reinterpret_cast<ConCommand *>( pCommandBase );
				#endif

					cmd_function_t *pPrev = NULL;
					cmd_function_t *pCmd = *Globals::cmd_functions;

					while ( pCmd != NULL )
					{
						if ( !stricmp( pCmd->name, pCommand->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCmd->next;
							}
							else
							{
							#pragma warning(push)
							#pragma warning(disable : 6001)

								*Globals::cmd_functions = pCmd->next;

							#pragma warning(pop)
							}

							bFound = true;
							break;
						}

						pPrev = pCmd;
						pCmd = pCmd->next;
					}

					if ( bFound && pCmd != NULL )
						function_cast<Mem_FreeFn>( GameData::Pointers::Engine::Mem_Free )( (void *)pCmd );

					pCommand->m_pCommand = NULL;
				}
				else
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConVar *pConVar = dynamic_cast<ConVar *>( pCommandBase );

					if ( !pConVar )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConCommand, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConVar *pConVar = reinterpret_cast<ConVar *>( pCommandBase );
				#endif

					cvar_t *pPrev = NULL;
					cvar_t *pCvar = *Globals::cvar_vars;

					while ( pCvar != NULL )
					{
						if ( !stricmp( pCvar->name, pConVar->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCvar->next;
							}
							else
							{
								*Globals::cvar_vars = pCvar->next;
							}

							bFound = true;
							break;
						}

						pPrev = pCvar;
						pCvar = pCvar->next;
					}

					if ( bFound && pCvar != NULL )
					{
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar->string );
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar );
					}

					pConVar->m_pCvar = NULL;
				}
			}

			pCommandBase->m_bRegistered = false;
		}
	}

	m_nNextDLLIdentifier = 0;
	m_CommandHash.RemoveAll();

	m_bInitialized = false;
}

void CCvar::PrintCvars( int mode, const char *pszPrefix )
{
	if ( !IsInitialized() )
		return;

	int iCount = 0;
	int iPrefixLength = 0;

	if ( pszPrefix )
		iPrefixLength = strlen( pszPrefix );

	ConMsg( "----------------------------------\n" );

#if CVAR_USE_DYNAMIC_CAST
	// TODO: use binary tree to sort the cvars
	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		const CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
			ConCommandBase *pCommandBase = bucket[ j ];

			if ( pszPrefix && strncmp( pszPrefix, pCommandBase->GetName(), iPrefixLength ) )
				continue;

			switch ( mode )
			{
			case 0:
				ConMsg( "%s\n", pCommandBase->GetName() );
				iCount++;
				break;

			case 1:
				if ( !pCommandBase->IsCommand() )
				{
					ConMsg( "%s\n", pCommandBase->GetName() );
					iCount++;
				}
				break;

			case 2:
				if ( pCommandBase->IsCommand() )
				{
					ConMsg( "%s\n", pCommandBase->GetName() );
					iCount++;
				}
				break;
			}
		}
	}
#else
	std::vector<std::string> commands;

	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		const CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
			ConCommandBase *pCommandBase = bucket[ j ];

			if ( pszPrefix && strncmp( pszPrefix, pCommandBase->GetName(), iPrefixLength ) )
				continue;

			switch ( mode )
			{
			case 0:
				commands.push_back( pCommandBase->GetName() );
				iCount++;
				break;

			case 1:
				if ( !pCommandBase->IsCommand() )
				{
					commands.push_back( pCommandBase->GetName() );
					iCount++;
				}
				break;

			case 2:
				if ( pCommandBase->IsCommand() )
				{
					commands.push_back( pCommandBase->GetName() );
					iCount++;
				}
				break;
			}
		}
	}

	std::sort( commands.begin(), commands.end() );

	for ( std::string &command : commands )
	{
		ConMsg( "%s\n", command.c_str() );
	}

	commands.clear();
#endif

	ConMsg( "----------------------------------\n" );

	switch ( mode )
	{
	case 0:
		ConMsg( "%d Total CVar%s/Command%s\n", iCount, iCount == 1 ? "" : "s", iCount == 1 ? "" : "s" );
		break;

	case 1:
		ConMsg( "%d Total CVar%s\n", iCount, iCount == 1 ? "" : "s" );
		break;

	case 2:
		ConMsg( "%d Total Command%s\n", iCount, iCount == 1 ? "" : "s" );
		break;
	}
}

CVarDLLIdentifier_t CCvar::AllocateDLLIdentifier()
{
	return m_nNextDLLIdentifier++;
}

//-----------------------------------------------------------------------------
// Register/unregister
//-----------------------------------------------------------------------------

bool CCvar::RegisterConCommand( ConCommandBase *pCommandBase )
{
	if ( !IsInitialized() )
		return false;

	if ( pCommandBase->IsRegistered() )
		return false;

	bool bRegisteredInGame = ( FindCvar( pCommandBase->GetName() ) != NULL || FindCmd( pCommandBase->GetName() ) != NULL );

	if ( !bRegisteredInGame && m_CommandHash.Insert( pCommandBase ) )
	{
		if ( pCommandBase->IsCommand() )
		{
		#if CVAR_USE_DYNAMIC_CAST
			ConCommand *pCommand = dynamic_cast<ConCommand *>( pCommandBase );

			if ( !pCommand )
			{
				Warning( "[SvenInt::Cvar] Can't cast to ConCommand, invalid ConCommandBase\n" );
				m_CommandHash.Remove( pCommandBase );
				return false;
			}
		#else
			ConCommand *pCommand = reinterpret_cast<ConCommand *>( pCommandBase );
		#endif

			Globals::cl_enginefuncs->pfnAddCommand( const_cast<char *>( pCommand->GetName() ), pCommand->m_pfnCallback );
			pCommand->m_pCommand = FindCmd( pCommand->GetName() );

			if ( pCommand->m_pCommand != NULL )
			{
				pCommand->m_pCommand->flags = pCommand->GetFlags();
			}

			DevMsg( "<SvenInt::Cvar> Registered ConCommand \"%s\"\n", pCommand->GetName() );
		}
		else
		{
		#if CVAR_USE_DYNAMIC_CAST
			ConVar *pConVar = dynamic_cast<ConVar *>( pCommandBase );

			if ( !pConVar )
			{
				Warning( "[SvenInt::Cvar] Can't cast to ConVar, invalid ConCommandBase\n" );
				m_CommandHash.Remove( pCommandBase );
				return false;
			}
		#else
			ConVar *pConVar = reinterpret_cast<ConVar *>( pCommandBase );
		#endif

			pConVar->m_pCvar = Globals::cl_enginefuncs->pfnRegisterVariable( const_cast<char *>( pConVar->GetName() ),
																			 const_cast<char *>( pConVar->GetDefault() ),
																			 pConVar->GetFlags() );

			DevMsg( "<SvenInt::Cvar> Registered ConVar \"%s\"\n", pConVar->GetName() );
		}

		pCommandBase->m_bRegistered = true;
	}
	else
	{
		Warning( "[SvenInt::Cvar] Console %s \"%s\" is already %s!\n",
				 pCommandBase->IsCommand() ? "command" : "variable",
				 pCommandBase->GetName(),
				 bRegisteredInGame ? "defined by the game" : "registered" );
	}

	return pCommandBase->m_bRegistered;
}

bool CCvar::UnregisterConCommand( ConCommandBase *pCommandBase )
{
	if ( !IsInitialized() )
		return false;

	if ( !pCommandBase->IsRegistered() )
		return false;

	bool bFound = false;

	if ( pCommandBase->IsCommand() )
	{
	#if CVAR_USE_DYNAMIC_CAST
		ConCommand *pCommand = dynamic_cast<ConCommand *>( pCommandBase );

		if ( !pCommand )
		{
			Warning( "[SvenInt::Cvar] Can't cast to ConVar, invalid ConCommandBase\n" );
			return false;
		}
	#else
		ConCommand *pCommand = reinterpret_cast<ConCommand *>( pCommandBase );
	#endif

		cmd_function_t *pPrev = NULL;
		cmd_function_t *pCmd = *Globals::cmd_functions;

		while ( pCmd != NULL )
		{
			if ( !stricmp( pCmd->name, pCommand->GetName() ) )
			{
				if ( pPrev != NULL )
				{
					pPrev->next = pCmd->next;
				}
				else
				{
					*Globals::cmd_functions = pCmd->next;
				}

				bFound = true;
				break;
			}

			pPrev = pCmd;
			pCmd = pCmd->next;
		}

		if ( bFound && pCmd != NULL )
			function_cast<Mem_FreeFn>( GameData::Pointers::Engine::Mem_Free )( (void *)pCmd );

		pCommand->m_pCommand = NULL;
		DevMsg( "<SvenInt::Cvar> Unregistered ConCommand \"%s\"\n", pCommand->GetName() );
	}
	else
	{
	#if CVAR_USE_DYNAMIC_CAST
		ConVar *pConVar = dynamic_cast<ConVar *>( pCommandBase );

		if ( !pConVar )
		{
			Warning( "[SvenInt::Cvar] Can't cast to ConCommand, invalid ConCommandBase\n" );
			return false;
		}
	#else
		ConVar *pConVar = reinterpret_cast<ConVar *>( pCommandBase );
	#endif

		cvar_t *pPrev = NULL;
		cvar_t *pCvar = *Globals::cvar_vars;

		while ( pCvar != NULL )
		{
			if ( !stricmp( pCvar->name, pConVar->GetName() ) )
			{
				if ( pPrev != NULL )
				{
					pPrev->next = pCvar->next;
				}
				else
				{
					*Globals::cvar_vars = pCvar->next;
				}

				bFound = true;
				break;
			}

			pPrev = pCvar;
			pCvar = pCvar->next;
		}

		if ( bFound && pCvar != NULL )
		{
			function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar->string );
			function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar );
		}

		pConVar->m_pCvar = NULL;
		DevMsg( "<SvenInt::Cvar> Unregistered ConVar \"%s\"\n", pConVar->GetName() );
	}

	m_CommandHash.Remove( pCommandBase );
	pCommandBase->m_bRegistered = false;

	return true;
}

void CCvar::UnregisterConCommands( CVarDLLIdentifier_t id )
{
	if ( !IsInitialized() )
		return;

	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
			ConCommandBase *pCommandBase = bucket[ j ];
			if ( pCommandBase->GetDLLIdentifier() != id )
				continue;

			if ( pCommandBase->IsRegistered() )
			{
				bool bFound = false;

				if ( pCommandBase->IsCommand() )
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConCommand *pCommand = dynamic_cast<ConCommand *>( pCommandBase );

					if ( !pCommand )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConVar, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConCommand *pCommand = reinterpret_cast<ConCommand *>( pCommandBase );
				#endif

					cmd_function_t *pPrev = NULL;
					cmd_function_t *pCmd = *Globals::cmd_functions;

					while ( pCmd != NULL )
					{
						if ( !stricmp( pCmd->name, pCommand->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCmd->next;
							}
							else
							{
							#pragma warning(push)
							#pragma warning(disable : 6001)

								*Globals::cmd_functions = pCmd->next;

							#pragma warning(pop)
							}

							bFound = true;
							break;
						}

						pPrev = pCmd;
						pCmd = pCmd->next;
					}

					if ( bFound && pCmd != NULL )
						function_cast<Mem_FreeFn>( GameData::Pointers::Engine::Mem_Free )( (void *)pCmd );

					pCommand->m_pCommand = NULL;
				}
				else
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConVar *pConVar = dynamic_cast<ConVar *>( pCommandBase );

					if ( !pConVar )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConCommand, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConVar *pConVar = reinterpret_cast<ConVar *>( pCommandBase );
				#endif

					cvar_t *pPrev = NULL;
					cvar_t *pCvar = *Globals::cvar_vars;

					while ( pCvar != NULL )
					{
						if ( !stricmp( pCvar->name, pConVar->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCvar->next;
							}
							else
							{
								*Globals::cvar_vars = pCvar->next;
							}

							bFound = true;
							break;
						}

						pPrev = pCvar;
						pCvar = pCvar->next;
					}

					if ( bFound && pCvar != NULL )
					{
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar->string );
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar );
					}

					pConVar->m_pCvar = NULL;
				}
			}

			pCommandBase->m_bRegistered = false;

			bucket.erase( bucket.begin() + j );
			j--;
		}
	}
}

void CCvar::UnregisterConCommands( void )
{
	if ( !IsInitialized() )
		return;

	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
			ConCommandBase *pCommandBase = bucket[ j ];

			if ( pCommandBase->IsRegistered() )
			{
				bool bFound = false;

				if ( pCommandBase->IsCommand() )
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConCommand *pCommand = dynamic_cast<ConCommand *>( pCommandBase );

					if ( !pCommand )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConVar, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConCommand *pCommand = reinterpret_cast<ConCommand *>( pCommandBase );
				#endif

					cmd_function_t *pPrev = NULL;
					cmd_function_t *pCmd = *Globals::cmd_functions;

					while ( pCmd != NULL )
					{
						if ( !stricmp( pCmd->name, pCommand->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCmd->next;
							}
							else
							{
							#pragma warning(push)
							#pragma warning(disable : 6001)

								*Globals::cmd_functions = pCmd->next;

							#pragma warning(pop)
							}

							bFound = true;
							break;
						}

						pPrev = pCmd;
						pCmd = pCmd->next;
					}

					if ( bFound && pCmd != NULL )
						function_cast<Mem_FreeFn>( GameData::Pointers::Engine::Mem_Free )( (void *)pCmd );

					pCommand->m_pCommand = NULL;
				}
				else
				{
				#if CVAR_USE_DYNAMIC_CAST
					ConVar *pConVar = dynamic_cast<ConVar *>( pCommandBase );

					if ( !pConVar )
					{
						Warning( "[SvenInt::Cvar] Can't cast to ConCommand, invalid ConCommandBase\n" );
						continue;
					}
				#else
					ConVar *pConVar = reinterpret_cast<ConVar *>( pCommandBase );
				#endif

					cvar_t *pPrev = NULL;
					cvar_t *pCvar = *Globals::cvar_vars;

					while ( pCvar != NULL )
					{
						if ( !stricmp( pCvar->name, pConVar->GetName() ) )
						{
							if ( pPrev != NULL )
							{
								pPrev->next = pCvar->next;
							}
							else
							{
								*Globals::cvar_vars = pCvar->next;
							}

							bFound = true;
							break;
						}

						pPrev = pCvar;
						pCvar = pCvar->next;
					}

					if ( bFound && pCvar != NULL )
					{
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar->string );
						function_cast<Z_FreeFn>( GameData::Pointers::Engine::Z_Free )( (void *)pCvar );
					}

					pConVar->m_pCvar = NULL;
				}
			}

			pCommandBase->m_bRegistered = false;

			bucket.erase( bucket.begin() + j );
			j--;
		}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

const char *CCvar::GetCommandLineValue( const char *pszVariableName )
{
	int nLen = strlen( pszVariableName );
	char *pSearch = (char *)calloc( nLen + 2, sizeof( char ) );

	if ( pSearch == NULL )
		return NULL;

	pSearch[ 0 ] = '+';
	memcpy( &pSearch[ 1 ], pszVariableName, nLen + 1 );

	const char *pszValue = Globals::commandline->ParmValue( pSearch );

	free( (void *)pSearch );
	return pszValue;
}

//-----------------------------------------------------------------------------
// Find cvar
//-----------------------------------------------------------------------------

cvar_t *CCvar::FindCvar( const char *pszName )
{
	return Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );
}

cmd_function_t *CCvar::FindCmd( const char *pszName )
{
	cmd_function_t *pCmd = *Globals::cmd_functions;

	while ( pCmd != NULL )
	{
		if ( !stricmp( pCmd->name, pszName ) )
		{
			return pCmd;
		}

		pCmd = pCmd->next;
	}

	return NULL;
}

ConCommandBase *CCvar::FindCommandBase( const char *pszName )
{
	return m_CommandHash.Find( pszName );
}

ConVar *CCvar::FindVar( const char *pszName )
{
	ConCommandBase *pCommandBase = m_CommandHash.Find( pszName );
	if ( pCommandBase == NULL )
		return NULL;

	if ( !pCommandBase->IsCommand() )
	#if CVAR_USE_DYNAMIC_CAST
		return dynamic_cast<ConVar *>( pCommandBase );
	#else
		return reinterpret_cast<ConVar *>( pCommandBase );
	#endif

	return NULL;
}

ConCommand *CCvar::FindCommand( const char *pszName )
{
	ConCommandBase *pCommandBase = m_CommandHash.Find( pszName );
	if ( pCommandBase == NULL )
		return NULL;

	if ( pCommandBase->IsCommand() )
	#if CVAR_USE_DYNAMIC_CAST
		return dynamic_cast<ConCommand *>( pCommandBase );
	#else
		return reinterpret_cast<ConCommand *>( pCommandBase );
	#endif

	return NULL;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CCvar::RevertFlaggedConVars( int nFlag )
{
	for ( int i = 0; i < m_CommandHash.Size(); i++ )
	{
		CConCommandHash::datapool_t &bucket = m_CommandHash.m_Buckets[ i ];

		for ( size_t j = 0; j < bucket.size(); j++ )
		{
		#if CVAR_USE_DYNAMIC_CAST
			ConVar *pConVar = dynamic_cast<ConVar *>( bucket[ j ] );
		#else
			ConVar *pConVar = reinterpret_cast<ConVar *>( bucket[ j ] );
		#endif

			if ( pConVar != NULL && pConVar->IsFlagSet( nFlag ) )
			{
				pConVar->Revert();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Commands args
//-----------------------------------------------------------------------------

int CCvar::ArgC( void ) const
{
	return *Globals::cmd_argc;
}

const char **CCvar::ArgV( void ) const
{
	return Globals::cmd_argv;
}

const char *CCvar::Arg( int nIndex ) const
{
	int args = *Globals::cmd_argc;

	Assert( nIndex >= 0 && nIndex < args );

	return Globals::cmd_argv[ nIndex ];
}

//-----------------------------------------------------------------------------
// Set value
//-----------------------------------------------------------------------------

void CCvar::SetValue( cvar_t *pCvar, const char *value )
{
	if ( pCvar != NULL )
		function_cast<Cvar_DirectSetFn>( GameData::Pointers::Engine::Cvar_DirectSet )( pCvar, value );
}

void CCvar::SetValue( cvar_t *pCvar, float value )
{
	if ( pCvar == NULL )
		return;

	char buffer[ 32 ];

	float eps = fabsf( value - int( value ) );

	if ( eps >= 0.000001 )
	{
		snprintf( buffer, Q_ARRAYSIZE( buffer ), "%f", value );
	}
	else
	{
		snprintf( buffer, Q_ARRAYSIZE( buffer ), "%d", int( value ) );
	}

	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	function_cast<Cvar_DirectSetFn>( GameData::Pointers::Engine::Cvar_DirectSet )( pCvar, buffer );
}

void CCvar::SetValue( cvar_t *pCvar, int value )
{
	if ( pCvar == NULL )
		return;

	char buffer[ 32 ];

	snprintf( buffer, Q_ARRAYSIZE( buffer ), "%d", value );
	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	function_cast<Cvar_DirectSetFn>( GameData::Pointers::Engine::Cvar_DirectSet )( pCvar, buffer );
}

void CCvar::SetValue( cvar_t *pCvar, bool value )
{
	if ( pCvar == NULL )
		return;

	char buffer[ 32 ];

	snprintf( buffer, Q_ARRAYSIZE( buffer ), "%d", int( value ) );
	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	function_cast<Cvar_DirectSetFn>( GameData::Pointers::Engine::Cvar_DirectSet )( pCvar, buffer );
}

void CCvar::SetValue( cvar_t *pCvar, Color value )
{
	if ( pCvar == NULL )
		return;

	char buffer[ 24 ];

	snprintf( buffer, Q_ARRAYSIZE( buffer ), "%hhu %hhu %hhu %hhu", value.r, value.g, value.b, value.a );
	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	SetValue( pCvar, buffer );
}

void CCvar::SetValue( const char *pszCvar, const char *value )
{
	Globals::cl_enginefuncs->Cvar_Set( const_cast<char *>( pszCvar ), const_cast<char *>( value ) );
}

void CCvar::SetValue( const char *pszCvar, float value )
{
	Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), value );
}

void CCvar::SetValue( const char *pszCvar, int value )
{
	Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), float( value ) );
}

void CCvar::SetValue( const char *pszCvar, bool value )
{
	Globals::cl_enginefuncs->Cvar_SetValue( const_cast<char *>( pszCvar ), !!int( value ) );
}

void CCvar::SetValue( const char *pszCvar, Color value )
{
	char buffer[ 24 ];

	snprintf( buffer, Q_ARRAYSIZE( buffer ), "%hhu %hhu %hhu %hhu", value.r, value.g, value.b, value.a );
	buffer[ Q_ARRAYSIZE( buffer ) - 1 ] = 0;

	Globals::cl_enginefuncs->Cvar_Set( const_cast<char *>( pszCvar ), buffer );
}

//-----------------------------------------------------------------------------
// Get value
//-----------------------------------------------------------------------------

const char *CCvar::GetStringFromCvar( cvar_t *pCvar )
{
	return pCvar->string;
}

float CCvar::GetFloatFromCvar( cvar_t *pCvar )
{
	return pCvar->value;
}

int CCvar::GetIntFromCvar( cvar_t *pCvar )
{
	return int( pCvar->value );
}

bool CCvar::GetBoolFromCvar( cvar_t *pCvar )
{
	return !!int( pCvar->value );
}

Color CCvar::GetColorFromCvar( cvar_t *pCvar )
{
	const char *pszColor = pCvar->string;

	unsigned char nRGBA[ 4 ];
	int nParamsRead = sscanf( pszColor, "%hhu %hhu %hhu %hhu", nRGBA, nRGBA + 1, nRGBA + 2, nRGBA + 3 );

	if ( nParamsRead >= 3 )
	{
		return {
			nRGBA[ 0 ], // red
			nRGBA[ 1 ], // green
			nRGBA[ 2 ], // blue
			( nParamsRead == 3 ) ? (unsigned char)255 : nRGBA[ 3 ]  // alpha
		};
	}
	else if ( nParamsRead == 2 )
	{
		return {
			nRGBA[ 0 ],
			nRGBA[ 1 ],
			(unsigned char)255,
			(unsigned char)255
		};
	}
	else if ( nParamsRead == 1 )
	{
		return {
			nRGBA[ 0 ],
			(unsigned char)255,
			(unsigned char)255,
			(unsigned char)255
		};
	}

	return { 255, 255, 255, 255 };
}

const char *CCvar::GetStringFromCvar( const char *pszName )
{
	cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );

	if ( pCvar != NULL )
		return GetStringFromCvar( pCvar );

	return "";
}

float CCvar::GetFloatFromCvar( const char *pszName )
{
	cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );

	if ( pCvar != NULL )
		return GetFloatFromCvar( pCvar );

	return 0.f;
}

int CCvar::GetIntFromCvar( const char *pszName )
{
	cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );

	if ( pCvar != NULL )
		return GetIntFromCvar( pCvar );

	return 0;
}

bool CCvar::GetBoolFromCvar( const char *pszName )
{
	cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );

	if ( pCvar != NULL )
		return GetBoolFromCvar( pCvar );

	return false;
}

Color CCvar::GetColorFromCvar( const char *pszName )
{
	cvar_t *pCvar = Globals::cl_enginefuncs->pfnGetCvarPointer( pszName );

	if ( pCvar != NULL )
		return GetColorFromCvar( pCvar );

	return { 255, 255, 255, 255 };
}

//-----------------------------------------------------------------------------
// CConCommandHash
//-----------------------------------------------------------------------------

CConCommandHash::~CConCommandHash( void )
{
	RemoveAll();
}

void CConCommandHash::Init( void )
{
	for ( size_t i = 0; i < NUM_BUCKETS; i++ )
	{
		m_Buckets[ i ].clear();
	}
}

unsigned int CConCommandHash::Hash( const char *pszCommand )
{
	return HashStringCaseless( pszCommand );
}

unsigned int CConCommandHash::Hash( ConCommandBase *pCommand )
{
	return HashStringCaseless( pCommand->GetName() );
}

ConCommandBase *CConCommandHash::Find( ConCommandBase *pCommand )
{
	return Find( pCommand->GetName() );
}

ConCommandBase *CConCommandHash::Find( const char *pszName )
{
	int index = Hash( pszName ) % NUM_BUCKETS_MASK;

	const datapool_t &bucket = m_Buckets[ index ];

	for ( size_t i = 0; i < bucket.size(); i++ )
	{
		if ( !stricmp( bucket[ i ]->GetName(), pszName ) )
		{
			return bucket[ i ];
		}
	}

	return NULL;
}

bool CConCommandHash::Insert( ConCommandBase *pCommand )
{
	int index = Hash( pCommand ) % NUM_BUCKETS_MASK;
	datapool_t &bucket = m_Buckets[ index ];

	for ( size_t i = 0; i < bucket.size(); i++ )
	{
		if ( !stricmp( bucket[ i ]->GetName(), pCommand->GetName() ) )
		{
			return false;
		}
	}

	bucket.push_back( pCommand );
	return true;
}

bool CConCommandHash::Remove( ConCommandBase *pCommand )
{
	return Remove( pCommand->GetName() );
}

bool CConCommandHash::Remove( const char *pszCommand )
{
	int index = Hash( pszCommand ) % NUM_BUCKETS_MASK;
	datapool_t &bucket = m_Buckets[ index ];

	for ( size_t i = 0; i < bucket.size(); i++ )
	{
		if ( !stricmp( bucket[ i ]->GetName(), pszCommand ) )
		{
			bucket.erase( bucket.begin() + i );
			return true;
		}
	}

	return false;
}

void CConCommandHash::RemoveAll( void )
{
	for ( int i = 0; i < NUM_BUCKETS; i++ )
	{
		if ( !m_Buckets[ i ].empty() )
		{
			m_Buckets[ i ].clear();
		}
	}
}

int CConCommandHash::Size( void )
{
	return NUM_BUCKETS;
}

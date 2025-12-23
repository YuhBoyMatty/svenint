// SvenInt (c) Sw1ft
// misc_message_spammer.cpp

#include "stdafx.h"
#include "misc_message_spammer.h"
#include "utils/util.h"

#include <regex>

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CMessageSpammer, messagespammer, "Misc", "Message Spammer" );

//-----------------------------------------------------------------------------
// Utility
//-----------------------------------------------------------------------------

static void ms_strip_extension( const char **pszString )
{
	char *pszExtension = NULL;
	char *str = (char *)*pszString;

	while ( *str )
	{
		if ( *str == '.' )
			pszExtension = str;

		str++;
	}

	if ( pszExtension )
	{
		*pszExtension = 0;
	}
}

//-----------------------------------------------------------------------------
// ConCommands & ConVars
//-----------------------------------------------------------------------------

CON_COMMAND( sc_ms_add, "Add spam task from file ./" SVENINT_FOLDER_NAME "/message_spammer/<taskname>.txt" )
{
	if ( args.ArgC() >= 2 )
	{
		const char *pszTask = args[ 1 ];

		ms_strip_extension( &pszTask );
		THIS_FEATURE()->AddTask( pszTask );
	}
	else
	{
		ConMsg( "Usage:  sc_ms_add <taskname>\n" );
	}
}

CON_COMMAND( sc_ms_remove, "Remove spam task by name" )
{
	if ( args.ArgC() >= 2 )
	{
		const char *pszTask = args[ 1 ];

		ms_strip_extension( &pszTask );
		THIS_FEATURE()->RemoveTask( pszTask );
	}
	else
	{
		ConMsg( "Usage:  sc_ms_remove <filename>\n" );
	}
}

CON_COMMAND( sc_ms_reload, " Reload spam task by name" )
{
	if ( args.ArgC() >= 2 )
	{
		const char *pszTask = args[ 1 ];

		ms_strip_extension( &pszTask );
		THIS_FEATURE()->ReloadTask( pszTask );
	}
	else
	{
		ConMsg( "Usage:  sc_ms_reload <filename>\n" );
	}
}

CON_COMMAND( sc_ms_keywords, "sc_ms_keywords - Prints all keywords" )
{
	Msg( "[Message Spammer] Keywords:\n" );
	Msg( "loop | must be defined at the beginning\n" );
	Msg( "send [message] | send a given message to the game chat\n" );
	Msg( "sleep [delay] | pause a running task for a given delay\n" );
}

CON_COMMAND( sc_ms_print, "sc_ms_print - Print all spam tasks" )
{
	THIS_FEATURE()->PrintTasks();
}

ConVar sc_ms_debug( "sc_ms_debug", "0", FCVAR_CLIENTDLL, "sc_ms_debug [0/1] - Enable debugging for Message Spammer" );

//-----------------------------------------------------------------------------
// CSpamTask
//-----------------------------------------------------------------------------

CSpamTask::CSpamTask( const char *pszName )
{
	m_pszName = (const char *)MemStrdup( pszName );
	m_bLoop = false;
	m_iOperatorBegin = 0;
}

CSpamTask::~CSpamTask()
{
	if ( m_pszName )
		MemFree( (void *)m_pszName );

	for ( size_t i = 0; i < m_operators.size(); ++i )
	{
		ISpamOperator *pOperator = m_operators[ i ];
		MemFreeInstance( pOperator );
		//delete pOperator;
	}

	m_operators.clear();
}

bool CSpamTask::Run( void )
{
	size_t i;

	if ( IsFinished() )
		return false;

	for ( i = m_iOperatorBegin; i < m_operators.size(); ++i )
	{
		ISpamOperator *pOperator = m_operators[ i ];

		m_iOperatorBegin = i;

		if ( IsWaiting() )
			return true;

		pOperator->Run( m_spamInfo );
	}

	if ( i == m_operators.size() )
	{
		if ( m_bLoop )
			m_iOperatorBegin = 0; // go through the operators list again
		else
			m_iOperatorBegin = m_operators.size();
	}

	return true;
}

bool CSpamTask::IsWaiting( void )
{
	return cl_enginefuncs->GetAbsoluteTime() < m_spamInfo.flNextRunTime;
}

bool CSpamTask::IsFinished( void )
{
	return ( !m_bLoop && m_operators.size() == m_iOperatorBegin ) || m_operators.size() == 0;
}

bool CSpamTask::IsLooped( void )
{
	return m_bLoop;
}

void CSpamTask::SetLoop( bool bLoop )
{
	m_bLoop = bLoop;
}

void CSpamTask::ResetWaiting( void )
{
	m_spamInfo.flNextRunTime = 0.f;
}

void CSpamTask::AddOperator( ISpamOperator *pOperator )
{
	m_operators.push_back( pOperator );
}

//-----------------------------------------------------------------------------
// CSpamOperatorSend
//-----------------------------------------------------------------------------

CSpamOperatorSend::CSpamOperatorSend()
{
	m_pszMessage = NULL;
}

CSpamOperatorSend::~CSpamOperatorSend()
{
	if ( m_pszMessage )
		MemFree( (void *)m_pszMessage );
}

void CSpamOperatorSend::Run( CSpamInfo &spamInfo )
{
	char command_buffer[ 512 ];

	snprintf( command_buffer, sizeof( command_buffer ), "say %s", m_pszMessage );
	cl_enginefuncs->pfnClientCmd( command_buffer );
}

void CSpamOperatorSend::SetOperand( const char *pszMessage )
{
	m_pszMessage = (const char *)MemStrdup( pszMessage );
}

//-----------------------------------------------------------------------------
// CSpamOperatorSleep
//-----------------------------------------------------------------------------

CSpamOperatorSleep::CSpamOperatorSleep()
{
	m_flSleepDelay = 0.f;
}

void CSpamOperatorSleep::Run( CSpamInfo &spamInfo )
{
	spamInfo.flNextRunTime = (float)cl_enginefuncs->GetAbsoluteTime() + m_flSleepDelay;
}

void CSpamOperatorSleep::SetOperand( float flSleepDelay )
{
	m_flSleepDelay = flSleepDelay;
}

//-----------------------------------------------------------------------------
// Message tasks management
//-----------------------------------------------------------------------------

void CMessageSpammer::PrintTasks( void )
{
	Msg( "[Message Spammer] Spam tasks list:\n" );

	for ( size_t i = 0; i < m_tasks.size(); ++i )
	{
		CSpamTask *pTask = m_tasks[ i ];

		Msg( "%d: %s (loop: %d, waiting: %d)\n", i, pTask->GetName(), pTask->IsLooped(), pTask->IsWaiting() );
	}
}

bool CMessageSpammer::AddTask( const char *pszTaskName )
{
	if ( GetTask( pszTaskName ) )
		return ReloadTask( pszTaskName );

	char szBuffer[ 512 ];

	snprintf( szBuffer, sizeof( szBuffer ), SVENINT_FOLDER_NAME "/message_spammer/%s.txt", pszTaskName );

	FILE *file = fopen( szBuffer, "r" );

	if ( file )
	{
		int nLine = 0;

		bool bLoopVarFound = false;
		bool bParsingOperators = false;
		bool bDebug = sc_ms_debug.GetBool();

		std::regex regex_loop( "^loop[\n]{0,1}$" );
		std::regex regex_send( "^send (.+)[\n]{0,1}$" );
		std::regex regex_sleep( "^sleep ([0-9.]+)[\n]{0,1}$" );

		CSpamTask *pTask = MemAllocInstance( (CSpamTask *)NULL, pszTaskName );

		if ( bDebug )
			Msg( "< Parsing task: %s >\n", pszTaskName );

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			std::cmatch match;
			nLine++;

			char *buffer = UTIL_LStrip( szBuffer );
			UTIL_RemoveComment( buffer );
			UTIL_RStrip( buffer );

			if ( !*buffer ) // empty
				continue;

			if ( !bParsingOperators && !bLoopVarFound )
			{
				if ( std::regex_search( buffer, match, regex_loop ) )
				{
					if ( bDebug )
						Msg( "[%d] Found action | loop\n", nLine );

					bLoopVarFound = true;
					continue;
				}
			}

			if ( std::regex_search( buffer, match, regex_send ) )
			{
				if ( bDebug )
					Msg( "[%d] Found action | send %s\n", nLine, match[ 1 ].str().c_str() );

				CSpamOperatorSend *pOperator = MemAllocInstance( (CSpamOperatorSend *)NULL );

				pOperator->SetOperand( match[ 1 ].str().c_str() );
				pTask->AddOperator( reinterpret_cast<ISpamOperator *>( pOperator ) );

				bParsingOperators = true;
			}
			else if ( std::regex_search( buffer, match, regex_sleep ) )
			{
				if ( bDebug )
					Msg( "[%d] Found action | sleep %s\n", nLine, match[ 1 ].str().c_str() );

				CSpamOperatorSleep *pOperator = MemAllocInstance( (CSpamOperatorSleep *)NULL );

				pOperator->SetOperand( strtof( match[ 1 ].str().c_str(), NULL ) );
				pTask->AddOperator( reinterpret_cast<ISpamOperator *>( pOperator ) );

				bParsingOperators = true;
			}
			else
			{
				Msg( "[MS] Unrecognized expression '%s' at line %d\n", buffer, nLine );
			}
		}

		if ( bDebug )
			Msg( "< Parsing finished >\n" );

		pTask->SetLoop( bLoopVarFound );
		m_tasks.push_back( pTask );

		Msg( "[Message Spammer] Spam task %s successfully parsed\n", pszTaskName );

		fclose( file );
		return true;
	}
	else
	{
		Msg( "[Message Spammer] Failed to open file called %s.txt\n", pszTaskName );
	}

	return false;
}

bool CMessageSpammer::ReloadTask( const char *pszTaskName )
{
	bool bReloaded = RemoveTask( pszTaskName ) && AddTask( pszTaskName );

	if ( bReloaded )
		Msg( "[Message Spammer] Spam task %s has been reloaded\n", pszTaskName );
	else
		Msg( "[Message Spammer] Failed to reload spam task %s\n", pszTaskName );

	return bReloaded;
}

bool CMessageSpammer::RemoveTask( const char *pszTaskName )
{
	for ( size_t i = 0; i < m_tasks.size(); ++i )
	{
		CSpamTask *pTask = m_tasks[ i ];
		const char *pszName = pTask->GetName();

		if ( pszName && !strcmp( pszName, pszTaskName ) )
		{
			m_tasks.erase( m_tasks.begin() + i );

			MemFreeInstance( pTask );
			//delete pTask;

			Msg( "[Message Spammer] Spam task %s has been removed\n", pszTaskName );

			return true;
		}
	}

	return false;
}

CSpamTask *CMessageSpammer::GetTask( const char *pszTaskName )
{
	for ( size_t i = 0; i < m_tasks.size(); ++i )
	{
		CSpamTask *pTask = m_tasks[ i ];
		const char *pszName = pTask->GetName();

		if ( pszName && !strcmp( pszName, pszTaskName ) )
			return pTask;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CMessageSpammer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// CL_CreateMove post event
	for ( size_t i = 0; i < m_tasks.size(); ++i )
	{
		CSpamTask *pTask = m_tasks[ i ];

		if ( !pTask->Run() )
		{
			m_tasks.erase( m_tasks.begin() + i );
			MemFreeInstance( pTask );
			//delete pTask;
			--i;
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CMessageSpammer::CMessageSpammer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CMessageSpammer::Load( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CMessageSpammer::PostLoad( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );

	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_ms_add ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_ms_remove ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_ms_reload ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_ms_keywords ) );
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_ms_print ) );
	cvar->RegisterConCommand( &sc_ms_debug );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CMessageSpammer::Unload( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );

	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_ms_add ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_ms_remove ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_ms_reload ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_ms_keywords ) );
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_ms_print ) );
	cvar->UnregisterConCommand( &sc_ms_debug );
}

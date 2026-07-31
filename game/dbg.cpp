// SvenInt (c) Sw1ft
// dbg.cpp

#include "stdafx.h"
#include "dbg.h"

#include "game/gameui/gameconsole.h"

#include <stdarg.h>
#include <stdio.h>
#include <mutex>

#ifdef GetMessage
#undef GetMessage
#endif

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#ifdef WIN32
#define COLOR_CAST( clr ) clr
#else
#define COLOR_CAST( clr ) *(uint32_t *)&clr
#endif

#define CONSOLE_PRINT_MESSAGE_LENGTH ( 8192 )

//-----------------------------------------------------------------------------
// Function signatures
//-----------------------------------------------------------------------------

#ifdef WIN32
FUNC_SIGNATURE( void, CALLCONV_THISCALL, RichText__InsertColorChangeFn, void *thisptr, Color clr );
#else // I really really really cannot understand but I don't care at all, this is cursed, it has no meaning, what's the purpose? How is that possible? The God only knows.
FUNC_SIGNATURE( void, CALLCONV_THISCALL, RichText__InsertColorChangeFn, void *thisptr, uint32_t clr );
#endif
FUNC_SIGNATURE( void, CALLCONV_THISCALL, RichText__InsertStringFn, void *thisptr, const char *msg );

//-----------------------------------------------------------------------------
// Colors
//-----------------------------------------------------------------------------

static const Color s_ConsoleDefaultPrintColor = { 240, 240, 240, 250 };
static const Color s_MsgPrintColor = { 232, 232, 232, 255 };
static const Color s_WarningPrintColor = { 255, 90, 90, 255 };
static const Color s_Warning2PrintColor = { 255, 255, 90, 255 };

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

void *gpDbgConsoleFile = NULL;
static std::mutex print_mutex;

//-----------------------------------------------------------------------------
// Auto lock access to the called function when doing multithreading
//-----------------------------------------------------------------------------

#define AUTO_LOCK( mutex ) CMutexAutoLock __##mutex##__autolock(mutex)

class CMutexAutoLock
{
public:
	CMutexAutoLock( std::mutex &mutex )
	{
		m_mutex = &mutex;
		m_mutex->lock();
	}

	~CMutexAutoLock()
	{
		m_mutex->unlock();
	}

private:
	std::mutex *m_mutex;
};

//-----------------------------------------------------------------------------
// Game console printing
//-----------------------------------------------------------------------------

enum
{
	kPrint = 0,
	kColorPrint,
	kDevPrint,
	kDevColorPrint
};

class CQueuedMessage
{
public:
	CQueuedMessage( int iType, const char *pszMessage )
	{
		m_type = iType;
		m_str = pszMessage;
	}
	
	CQueuedMessage( int iType, const Color &clr, const char *pszMessage )
	{
		m_type = iType;
		m_clr = clr;
		m_str = pszMessage;
	}

	~CQueuedMessage( void )
	{
		m_str.clear();
	}

	int			GetType( void ) const { return m_type; }
	const Color &GetColor( void ) const { return m_clr; }
	const char	*GetMessage( void ) const { return m_str.c_str(); }

private:
	int			m_type;
	Color		m_clr;
	std::string m_str;
};

class CConsolePrinting
{
public:
	CConsolePrinting()
	{
		m_bCanPrint = false;
		m_bAllowPrint = false;
		m_pGameConsole = NULL;
		m_pfnRichText__InsertColorChange = NULL;
		m_pfnRichText__InsertString = NULL;
		m_pDeveloper = NULL;
	}

	inline bool IsPrintAllowed( void ) const { return m_bAllowPrint; }
	inline void AllowPrint( bool state ) { m_bAllowPrint = state; }

	bool CanPrint( void )
	{
		if ( !m_bCanPrint &&
			 Globals::gameconsole != NULL &&
			 GameData::Pointers::GameUI::RichText__InsertColorChange != NULL &&
			 GameData::Pointers::GameUI::RichText__InsertString != NULL )
		{
			m_bCanPrint = true;
			m_pGameConsole = view_as<CGameConsole *>( Globals::gameconsole );
			m_pfnRichText__InsertColorChange = function_cast<RichText__InsertColorChangeFn>( GameData::Pointers::GameUI::RichText__InsertColorChange );
			m_pfnRichText__InsertString = function_cast<RichText__InsertStringFn>( GameData::Pointers::GameUI::RichText__InsertString );
		}

		return m_bCanPrint;
	}

	bool IsDeveloperMode( void )
	{
		if ( m_pDeveloper == NULL )
		{
			if ( Globals::cl_enginefuncs == NULL )
				return false;

			m_pDeveloper = Globals::cl_enginefuncs->pfnGetCvarPointer( "developer" );
			if ( m_pDeveloper == NULL )
				return false;
		}

		return !!m_pDeveloper->value;
	}

	bool PrintQueuedMessages( void )
	{
		if ( m_QueuedMessages.empty() )
			return false;

		for ( const CQueuedMessage &msg : m_QueuedMessages )
		{
			switch ( msg.GetType() )
			{
			case kPrint:
				Print( msg.GetMessage() );
				break;

			case kColorPrint:
				ColorPrint( msg.GetColor(), msg.GetMessage() );
				break;

			case kDevPrint:
				DPrint( msg.GetMessage() );
				break;

			case kDevColorPrint:
				DColorPrint( msg.GetColor(), msg.GetMessage() );
				break;
			}
		}

		m_QueuedMessages.clear();
		return true;
	}

	inline void AddQueuedMessage( int iType, const char *pszMessage )
	{
		if ( pszMessage == NULL || *pszMessage == '\0' )
			return;

		m_QueuedMessages.push_back( CQueuedMessage( iType, pszMessage ) );
	}
	
	inline void AddQueuedMessage( int iType, const Color &clr, const char *pszMessage )
	{
		if ( pszMessage == NULL || *pszMessage == '\0' )
			return;

		m_QueuedMessages.push_back( CQueuedMessage( iType, clr, pszMessage ) );
	}

	void Print( const char *pszMessage )
	{
		if ( !CanPrint() || !m_pGameConsole->IsInitialized() )
			return;

		CGameConsoleDialog *pGameConsoleDialog = m_pGameConsole->GetGameConsoleDialog();

		m_pfnRichText__InsertColorChange( pGameConsoleDialog->m_pRichText, COLOR_CAST( s_ConsoleDefaultPrintColor ) );
		m_pfnRichText__InsertString( pGameConsoleDialog->m_pRichText, pszMessage );
	}

	void ColorPrint( const Color &clr, const char *pszMessage )
	{
		if ( !CanPrint() || !m_pGameConsole->IsInitialized() )
			return;

		CGameConsoleDialog *pGameConsoleDialog = m_pGameConsole->GetGameConsoleDialog();

		m_pfnRichText__InsertColorChange( pGameConsoleDialog->m_pRichText, COLOR_CAST( clr ) );
		m_pfnRichText__InsertString( pGameConsoleDialog->m_pRichText, pszMessage );
	}

	void DPrint( const char *pszMessage )
	{
		if ( !CanPrint() || !IsDeveloperMode() || !m_pGameConsole->IsInitialized() )
			return;

		CGameConsoleDialog *pGameConsoleDialog = m_pGameConsole->GetGameConsoleDialog();

		m_pfnRichText__InsertColorChange( pGameConsoleDialog->m_pRichText, COLOR_CAST( s_ConsoleDefaultPrintColor ) );
		m_pfnRichText__InsertString( pGameConsoleDialog->m_pRichText, pszMessage );
	}
	
	void DColorPrint( const Color &clr, const char *pszMessage )
	{
		if ( !CanPrint() || !IsDeveloperMode() || !m_pGameConsole->IsInitialized() )
			return;

		CGameConsoleDialog *pGameConsoleDialog = m_pGameConsole->GetGameConsoleDialog();

		m_pfnRichText__InsertColorChange( pGameConsoleDialog->m_pRichText, COLOR_CAST( clr ) );
		m_pfnRichText__InsertString( pGameConsoleDialog->m_pRichText, pszMessage );
	}

private:
	bool							m_bCanPrint;
	bool							m_bAllowPrint;
	CGameConsole					*m_pGameConsole;
	RichText__InsertColorChangeFn	m_pfnRichText__InsertColorChange;
	RichText__InsertStringFn		m_pfnRichText__InsertString;
	cvar_t							*m_pDeveloper;
	std::vector<CQueuedMessage>		m_QueuedMessages;
};

static CConsolePrinting gConsolePrinting;

//-----------------------------------------------------------------------------
// Printing functions
//-----------------------------------------------------------------------------

static bool HasFormatSpecifiers( const char *fmt )
{
	if ( fmt == NULL )
		return false;

	for ( const char *p = fmt; *p; ++p )
	{
		if ( *p == '%' )
		{
			if ( p[ 1 ] == '%' )
			{
				++p;
				continue;
			}

			if ( p[ 1 ] && ( isalpha( p[ 1 ] ) || p[ 1 ] == '.' || p[ 1 ] == '*' || p[ 1 ] == '-' || p[ 1 ] == '+' || p[ 1 ] == '0' ) )
			{
				return true;
			}
		}
	}

	return false;
}

void AllowMsgPrint( bool state )
{
	gConsolePrinting.AllowPrint( state );
}

bool IsMsgPrintAllowed( void )
{
	return gConsolePrinting.IsPrintAllowed();
}

bool PrintQueudMessages( void )
{
	return gConsolePrinting.PrintQueuedMessages();
}

void Msg( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.Print( rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kPrint, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, rgszFormattedMsg );

#ifdef LINUX
	printf( "%s", rgszFormattedMsg );
#endif
}

void Warning( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.ColorPrint( s_WarningPrintColor, rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kColorPrint, s_WarningPrintColor, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "WARNING: %s", rgszFormattedMsg );

#ifdef LINUX
	printf( "WARNING: %s", rgszFormattedMsg );
#endif
}

void Warning2( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.ColorPrint( s_Warning2PrintColor, rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kColorPrint, s_Warning2PrintColor, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "WARNING: %s", rgszFormattedMsg );

#ifdef LINUX
	printf( "WARNING: %s", rgszFormattedMsg );
#endif
}

void DevMsg( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.DPrint( rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kDevPrint, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "DEV: %s", rgszFormattedMsg );

#ifdef LINUX
	printf( "DEV: %s", rgszFormattedMsg );
#endif
}

void DevWarning( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.DColorPrint( s_WarningPrintColor, rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kDevColorPrint, s_WarningPrintColor, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "DEV WARNING: %s", rgszFormattedMsg );

#ifdef LINUX
	printf( "DEV WARNING: %s", rgszFormattedMsg );
#endif
}

void ConColorMsg( const Color &clr, const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.ColorPrint( clr, rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kColorPrint, clr, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, rgszFormattedMsg );

#ifdef LINUX
	printf( "%s", rgszFormattedMsg );
#endif
}

void ConMsg( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.Print( rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kPrint, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, rgszFormattedMsg );

#ifdef LINUX
	printf( "%s", rgszFormattedMsg );
#endif
}

void ConDMsg( const char *pszMessageFormat, ... )
{
	AUTO_LOCK( print_mutex );

	char rgszFormattedMsg[ CONSOLE_PRINT_MESSAGE_LENGTH ];

	va_list args;
	va_start( args, pszMessageFormat );
	vsnprintf( rgszFormattedMsg, Q_ARRAYSIZE( rgszFormattedMsg ), pszMessageFormat, args );
	va_end( args );

	rgszFormattedMsg[ Q_ARRAYSIZE( rgszFormattedMsg ) - 1 ] = 0;

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.DPrint( rgszFormattedMsg );
	else
		gConsolePrinting.AddQueuedMessage( kDevPrint, rgszFormattedMsg );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "DEV: %s", rgszFormattedMsg );

#ifdef LINUX
	printf( "DEV: %s", rgszFormattedMsg );
#endif
}

void ConColorMsgNoFormat( const Color &clr, const char *pszMessage )
{
	AUTO_LOCK( print_mutex );

	if ( gConsolePrinting.IsPrintAllowed() )
		gConsolePrinting.ColorPrint( clr, pszMessage );
	else
		gConsolePrinting.AddQueuedMessage( kColorPrint, clr, pszMessage );

	if ( gpDbgConsoleFile != NULL )
		fprintf( (FILE *)gpDbgConsoleFile, "%s", pszMessage);

#ifdef LINUX
	printf( "%s", pszMessage );
#endif
}

// SvenInt (c) Sw1ft
// dbg.h

#ifndef SINT_DBG_H
#define SINT_DBG_H

#ifdef _WIN32
#pragma once
#endif

#include <assert.h>
#include "game/color.h"

//-----------------------------------------------------------------------------
// Macro definition
//-----------------------------------------------------------------------------

#define __AssertMsg( exp, msg, bFatal ) \
	if ( !static_cast<bool>( exp ) ) \
	{ \
		ConColorMsg( { 255, 20, 20, 255 }, "[SvenInt] Assertion Failed\n%s (%d): %s\n", __FILE__, __LINE__, msg ); \
		if ( bFatal ) \
		{ \
			assert( exp ); \
		} \
	}

#ifdef DEBUG

#define Assert( exp ) __AssertMsg( exp, #exp, true )
#define AssertMsg( exp, msg ) __AssertMsg( exp, msg, true )

#else

#define Assert( exp ) __AssertMsg( exp, #exp, false )
#define AssertMsg( exp, msg ) __AssertMsg( exp, msg, false )

#endif

//-----------------------------------------------------------------------------
// Debug console
//-----------------------------------------------------------------------------

extern void *gpDbgConsoleFile;

//-----------------------------------------------------------------------------
// Debug functions
//-----------------------------------------------------------------------------

void AllowMsgPrint( bool state );
bool IsMsgPrintAllowed( void );
bool PrintQueudMessages( void );

void Msg( const char *pszMessageFormat, ... );
void Warning( const char *pszMessageFormat, ... );
void Warning2( const char *pszMessageFormat, ... );
void DevMsg( const char *pszMessageFormat, ... );
void DevWarning( const char *pszMessageFormat, ... );
void ConColorMsg( const Color &clr, const char *pszMessageFormat, ... );
void ConMsg( const char *pszMessageFormat, ... );
void ConDMsg( const char *pszMessageFormat, ... );

void ConColorMsgNoFormat( const Color & clr, const char *pszMessage );

#endif // !SINT_DBG_H
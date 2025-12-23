// SvenInt (c) Sw1ft
// util.h

#ifndef SINT_UTIL_H
#define SINT_UTIL_H

#ifdef _WIN32
#pragma once
#endif

#include <string.h>

#define PARSE_COMMENT_PREFIX ";#"
#define PARSE_STRIP_CHARS (" \t\n")

#define PARSE_STRIP_CHARS_LEN (sizeof(PARSE_STRIP_CHARS) - 1)
#define PARSE_COMMENT_PREFIX_LEN (sizeof(PARSE_COMMENT_PREFIX) - 1)

inline int UTIL_ContainsChars( char ch, const char *chars, size_t length )
{
	for ( size_t i = 0; i < length; ++i )
	{
		if ( chars[ i ] == ch )
			return 1;
	}

	return 0;
}

inline char *UTIL_LStrip( char *str )
{
	while ( *str && UTIL_ContainsChars( *str, PARSE_STRIP_CHARS, PARSE_STRIP_CHARS_LEN ) )
		++str;

	return str;
}

inline void UTIL_RStrip( char *str )
{
	char *end = str + strlen( str ) - 1;

	if ( end < str )
		return;

	while ( end >= str && UTIL_ContainsChars( *end, PARSE_STRIP_CHARS, PARSE_STRIP_CHARS_LEN ) )
	{
		*end = '\0';
		--end;
	}
}

inline void UTIL_RemoveComment( char *str )
{
	while ( *str && !UTIL_ContainsChars( *str, PARSE_COMMENT_PREFIX, PARSE_COMMENT_PREFIX_LEN ) )
		++str;

	if ( *str )
		*str = '\0';
}

inline void UTIL_ReplaceUnicodeChars( char *pszInput, char chReplace = ' ' )
{
	if ( pszInput == NULL )
		return;

	for ( size_t i = 0; i < strlen( pszInput ); ++i )
	{
		if ( ( static_cast<signed char>( pszInput[ i ] ) ) < 0 )
		{
			pszInput[ i ] = chReplace;
		}
	}
}

inline void UTIL_ReplaceUnicodeChars( const char *pszInput, char *pszOutput, char chReplace = ' ' )
{
	size_t i = 0;

	if ( pszInput == NULL || pszOutput == NULL )
		return;

	for ( i = 0; pszInput[ i ] != '\0'; i++ )
	{
		if ( (signed char)pszInput[ i ] < 0 )
		{
			pszOutput[ i ] = chReplace;
		}
		else
		{
			pszOutput[ i ] = pszInput[ i ];
		}
	}

	pszOutput[ i ] = '\0';
}

inline float UTIL_Hue2RGB( float p, float q, float t )
{
	if ( t < 0.f )
		t += 1.f;

	if ( t > 1.f )
		t -= 1.f;

	if ( t < 1.f / 6.f )
		return p + ( q - p ) * 6.f * t;

	if ( t < 1.f / 2.f )
		return q;

	if ( t < 2.f / 3.f )
		return p + ( q - p ) * ( ( 2.f / 3.f ) - t ) * 6.f;

	return p;
}

inline void UTIL_HSL2RGB( float h, float s, float l, float &r, float &g, float &b )
{
	if ( s == 0.f )
	{
		r = g = b = l;
		return;
	}

	float q = l < 0.5f ? l * ( 1.f + s ) : l + s - l * s;
	float p = 2.f * l - q;

	r = UTIL_Hue2RGB( p, q, h + ( 1.f / 3.f ) );
	g = UTIL_Hue2RGB( p, q, h );
	b = UTIL_Hue2RGB( p, q, h - ( 1.f / 3.f ) );
}

inline bool UTIL_IsCStringEndsWithPostfix( const char *pszString, const char *pszPostfix )
{
	if ( !pszString || !pszPostfix )
		return false;

	size_t nLength = strlen( pszString );
	size_t nPostfixLength = strlen( pszPostfix );

	if ( nPostfixLength > nLength )
		return false;

	return strncmp( pszString + nLength - nPostfixLength, pszPostfix, nPostfixLength ) == 0;
}

#endif // SINT_UTIL_H

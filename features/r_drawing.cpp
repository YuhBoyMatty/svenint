// SvenInt (c) Sw1ft
// r_drawing.cpp

#include "stdafx.h"
#include "r_drawing.h"
#include "utils/util.h"
#ifdef LINUX
#include "features/visual_ignore_unicode.h"
#endif

#include <stdarg.h>

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDrawing, drawing, "Render", "Drawing" );

//-----------------------------------------------------------------------------
// Sprite handlers
//-----------------------------------------------------------------------------

int CDrawing::DrawDigit( int digit, int x, int y, int r, int g, int b )
{
	if ( digit >= 0 && digit <= 9 )
	{
		cl_enginefuncs->pfnSPR_Set( m_NumberSprites[ digit ], r, g, b );
		cl_enginefuncs->pfnSPR_DrawAdditive( 0, x, y, &m_NumberSpriteRects[ digit ] );
	}

	return m_iNumberWidth;
}

int CDrawing::DrawDigit( int digit, int x, int y, int r, int g, int b, FontAlignFlags_t alignment )
{
	ApplyTextAlignment( alignment, x, y, m_iNumberWidth, m_iNumberHeight );
	return DrawDigit( digit, x, y, r, g, b );
}

int CDrawing::DrawNumber( int number, int x, int y, int r, int g, int b, FontAlignFlags_t alignment, int fieldMinWidth /* = 1 */ )
{
	bool bNegative = false;

	if ( number < 0 )
	{
		if ( number == -2147483648 )
		{
			number = 0;
		}
		else
		{
			number = abs( number );
			bNegative = true;
		}
	}

	int i;
	int c = 0;
	int digits[ 10 ] = { 0 };

	for ( i = 0; i < 10; ++i )
	{
		if ( !number )
			break;

		digits[ i ] = number % 10;
		number /= 10;

		c++;
	}

	ApplyTextAlignment( alignment, x, y, ( fieldMinWidth >= c ? fieldMinWidth : c ) * m_iNumberWidth, m_iNumberHeight );

	if ( bNegative )
	{
		DrawLine( x - m_iNumberWidth, y + m_iNumberHeight / 2, x, y + m_iNumberHeight / 2, r, g, b, 255 );
	}

	for ( ; fieldMinWidth > 10; --fieldMinWidth )
	{
		DrawDigit( 0, x, y, r, g, b );
		x += m_iNumberWidth;
	}

	if ( fieldMinWidth > i )
		i = fieldMinWidth;

	for ( int j = i; j > 0; --j )
	{
		DrawDigit( digits[ j - 1 ], x, y, r, g, b );
		x += m_iNumberWidth;
	}

	return x;
}

int CDrawing::GetNumberSpriteWidth()
{
	return m_iNumberWidth;
}

int CDrawing::GetNumberSpriteHeight()
{
	return m_iNumberHeight;
}

//-----------------------------------------------------------------------------
// Figures
//-----------------------------------------------------------------------------

void CDrawing::DrawCircle3D( Vector &position, float points, float radius, int r, int g, int b, int a )
{
	surface->DrawSetColor( r, g, b, a );
	surface->DrawSetTextColor( r, g, b, a );

	float step = (float)M_PI * 2.0f / points;

	for ( float a = 0; a < ( M_PI * 2.0f ); a += step )
	{
		Vector start( radius * cosf( a ) + position.x, radius * sinf( a ) + position.y, position.z );
		Vector end( radius * cosf( a + step ) + position.x, radius * sinf( a + step ) + position.y, position.z );

		Vector start2d, end2d;
		if ( !UTIL_WorldToScreen( start, start2d ) || !UTIL_WorldToScreen( end, end2d ) )
			return;

		surface->DrawLine( (int)start2d.x, (int)start2d.y, (int)end2d.x, (int)end2d.y );
	}
}

void CDrawing::DrawCircle( float position[ 2 ], float points, float radius, int r, int g, int b, int a )
{
	surface->DrawSetColor( r, g, b, a );
	surface->DrawSetTextColor( r, g, b, a );

	float step = (float)M_PI * 2.0f / points;
	float start[ 2 ], end[ 2 ];

	for ( float x = 0; x < ( M_PI * 2.0f ); x += step )
	{
		start[ 0 ] = radius * cosf( x ) + position[ 0 ];
		start[ 1 ] = radius * sinf( x ) + position[ 1 ];
		end[ 0 ] = radius * cosf( x + step ) + position[ 0 ];
		end[ 1 ] = radius * sinf( x + step ) + position[ 1 ];
		surface->DrawLine( (int)start[ 0 ], (int)start[ 1 ], (int)end[ 0 ], (int)end[ 1 ] );
	}
}

void CDrawing::DrawPlayerBox( int x, int y, int w, int h, int r, int g, int b, int a, bool outline )
{
	int x0 = x - w;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	surface->DrawSetColor( r, g, b, a );
	surface->DrawOutlinedRect( x0, y0, x1, y1 );

	if ( outline )
	{
		surface->DrawSetColor( 0, 0, 0, a );
		surface->DrawOutlinedRect( x0 - 1, y0 - 1, x1 + 1, y1 + 1 );
		surface->DrawOutlinedRect( x0 + 1, y0 + 1, x1 - 1, y1 - 1 );
	}
}

void CDrawing::DrawRect( int x, int y, int w, int h, int r, int g, int b, int a )
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	surface->DrawSetColor( r, g, b, a );
	surface->DrawFilledRect( x0, y0, x1, y1 );
}

void CDrawing::DrawOutlinedRect( int x, int y, int w, int h, int r, int g, int b, int a )
{
	int x0 = x;
	int y0 = y;
	int x1 = x + w;
	int y1 = y + h;

	//if (x1 < x0)
	//	swap(x1, x0);

	//if (y1 < y0)
	//	swap(y1, y0);

	surface->DrawSetColor( r, g, b, a );
	surface->DrawOutlinedRect( x0, y0, x1, y1 );
}

void CDrawing::DrawLine( int x0, int y0, int x1, int y1, int r, int g, int b, int a )
{
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetColor( r, g, b, a );
	surface->DrawLine( x0, y0, x1, y1 );
}

void CDrawing::FillArea( int x, int y, int w, int h, int r, int g, int b, int a )
{
	glPushMatrix();

	glLoadIdentity();

	glDisable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );

	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glColor4ub( r, g, b, a );

	glBegin( GL_QUADS );
		glVertex2i( x, y );
		glVertex2i( x + w, y );
		glVertex2i( x + w, y + h );
		glVertex2i( x, y + h );
	glEnd();

	glDisable( GL_BLEND );
	glEnable( GL_TEXTURE_2D );

	glPopMatrix();
}

void CDrawing::DrawBox( int x, int y, int w, int h, int r, int g, int b, int a )
{
	surface->DrawSetColor( r, g, b, a );
	surface->DrawOutlinedRect( x - w, y - h, x + w, y + h );
}

void CDrawing::Box( int x, int y, int w, int h, int lw, int r, int g, int b, int a )
{
	FillArea( x, y, w, lw, r, g, b, a );
	FillArea( x + w - lw, y + lw, lw, h - lw, r, g, b, a );
	FillArea( x, y + lw, lw, h - lw, r, g, b, a );
	FillArea( x + lw, y + h - lw, w - lw * 2, lw, r, g, b, a );
}

void CDrawing::BoxOutline( float x, float y, float w, float h, float lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	Box( (int)x, (int)y, (int)w, (int)h, (int)lw, r, g, b, a );
	Box( (int)x - 1, (int)y - 1, (int)w + 2, (int)h + 2, 1, 0, 0, 0, a );
	Box( (int)x + (int)lw, (int)y + (int)lw, (int)w - (int)lw * 2, (int)h - (int)lw * 2, 1, 0, 0, 0, a );
}

void CDrawing::DrawCoalBox( int x, int y, int w, int h, int lw, int r, int g, int b, int a )
{
	int iw = w / 4;
	int ih = h / 1;

	DrawLine( x, y, x + iw, y, r, g, b, a );			// left
	DrawLine( x + w - iw, y, x + w, y, r, g, b, a );			// right
	DrawLine( x, y, x, y + ih, r, g, b, a );				// top left
	DrawLine( x + w, y, x + w, y + ih, r, g, b, a );	// top right
	// bottom
	DrawLine( x, y + h, x + iw, y + h, r, g, b, a );			// left
	DrawLine( x + w - iw, y + h, x + w, y + h, r, g, b, a );	// right
	DrawLine( x, y + h - ih, x, y + h, r, g, b, a );			// bottom left
	DrawLine( x + w, y + h - ih, x + w, y + h, r, g, b, a );	// bottom right
}

void CDrawing::DrawOutlineCoalBox( int x, int y, int w, int h, int  lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	int iw = w / 4;
	int ih = h / 4;

	DrawCoalBox( x - 1, y - 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x + 1, y + 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x - 1, y - 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x + 1, y + 1, w, h, lw, 0, 0, 0, a );

	DrawCoalBox( x - 1, y - 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x + 1, y + 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x - 1, y - 1, w, h, lw, 0, 0, 0, a );
	DrawCoalBox( x + 1, y + 1, w, h, lw, 0, 0, 0, a );


	DrawCoalBox( x, y, w, h, iw, r, g, b, a );
}

void CDrawing::BoxCorner( int x, int y, int w, int h, int lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	FillArea( x, y, w / 4, lw, r, g, b, a );
	FillArea( x + w - w / 4, y, w / 4, lw, r, g, b, a );
	FillArea( x, y + lw, lw, h / 4 - lw, r, g, b, a );
	FillArea( x, y + h - h / 4, lw, h / 4, r, g, b, a );
	FillArea( x + w - lw, y + lw, lw, h / 4 - lw, r, g, b, a );
	FillArea( x + w - lw, y + h - h / 4, lw, h / 4, r, g, b, a );
	FillArea( x + lw, y + h - lw, w / 4 - lw, lw, r, g, b, a );
	FillArea( x + w - w / 4, y + h - lw, w / 4 - lw, lw, r, g, b, a );
}

void CDrawing::BoxCornerOutline( int x, int y, int w, int h, int lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a )
{
	BoxCorner( x - 1, y + 1, w, h, lw, 0, 0, 0, a );
	BoxCorner( x - 1, y - 1, w, h, lw, 0, 0, 0, a );
	BoxCorner( x + 1, y + 1, w, h, lw, 0, 0, 0, a );
	BoxCorner( x + 1, y - 1, w, h, lw, 0, 0, 0, a );

	BoxCorner( x, y, w, h, lw, r, g, b, a );
}

//-----------------------------------------------------------------------------
// Draw a texture
//-----------------------------------------------------------------------------

void CDrawing::DrawTexture( int id, int x0, int y0, int x1, int y1, int r, int g, int b, int a )
{
	//enginesurface->DrawSetTexture( id );
	//enginesurface->DrawSetColor( r, g, b, a );
	//enginesurface->DrawTexturedRect( x0, y0, x1, y1 );
	surface->DrawSetColor( r, g, b, a );
	surface->DrawSetTexture( id );
	surface->DrawTexturedRect( x0, y0, x1, y1 );
}

//-----------------------------------------------------------------------------
// Draw a formatted string
//-----------------------------------------------------------------------------

void CDrawing::DrawStringF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ... )
{
	char s_szBuffer[ 4096 ];
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

	va_list va_alist;
	va_start( va_alist, pszString );
	vsnprintf( s_szBuffer, sizeof( s_szBuffer ), pszString, va_alist );
	va_end( va_alist );
#ifdef WIN32
	MultiByteToWideChar( CP_UTF8, 0, s_szBuffer, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
		UTIL_ReplaceUnicodeChars( s_szBuffer );
	localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
#endif

	int width, height;
	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawStringExF( vgui::HFont font, int x, int y, int r, int g, int b, int a, int &width, int &height, FontAlignFlags_t alignment, const char *pszString, ... )
{
	char s_szBuffer[ 4096 ];
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

	va_list va_alist;
	va_start( va_alist, pszString );
	vsnprintf( s_szBuffer, sizeof( s_szBuffer ), pszString, va_alist );
	va_end( va_alist );
#ifdef WIN32
	MultiByteToWideChar( CP_UTF8, 0, s_szBuffer, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
		UTIL_ReplaceUnicodeChars( s_szBuffer );
	localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
#endif

	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawStringACPF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ... )
{
	char s_szBuffer[ 4096 ];
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

	va_list va_alist;
	va_start( va_alist, pszString );
	vsnprintf( s_szBuffer, sizeof( s_szBuffer ), pszString, va_alist );
	va_end( va_alist );
#ifdef WIN32
	MultiByteToWideChar( CP_ACP, 0, s_szBuffer, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
		UTIL_ReplaceUnicodeChars( s_szBuffer );
	localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
#endif

	int width, height;
	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawWideStringF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString, ... )
{
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

	va_list va_alist;
	va_start( va_alist, pwszString );
	vswprintf( s_wszBuffer, sizeof( s_wszBuffer ) / sizeof( wchar_t ), pwszString, va_alist );
	va_end( va_alist );

	int width, height;
	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

//-----------------------------------------------------------------------------
// Draw a string
//-----------------------------------------------------------------------------

void CDrawing::DrawString( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString )
{
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

#ifdef WIN32
	MultiByteToWideChar( CP_UTF8, 0, pszString, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
	{
		char s_szBuffer[ 4096 ];

		UTIL_ReplaceUnicodeChars( pszString, s_szBuffer );
		localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
	else
	{
		localize->ConvertANSIToUnicode( pszString, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
#endif

	int width, height;
	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawStringEx( vgui::HFont font, int x, int y, int r, int g, int b, int a, int &width, int &height, FontAlignFlags_t alignment, const char *pszString )
{
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

#ifdef WIN32
	MultiByteToWideChar( CP_UTF8, 0, pszString, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
	{
		char s_szBuffer[ 4096 ];

		UTIL_ReplaceUnicodeChars( pszString, s_szBuffer );
		localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
	else
	{
		localize->ConvertANSIToUnicode( pszString, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
#endif

	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawStringACP( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString )
{
	wchar_t s_wszBuffer[ 4096 ];

	enginesurface->ResetViewPort();

#ifdef WIN32
	MultiByteToWideChar( CP_ACP, 0, pszString, 256, s_wszBuffer, 256 );
#else
	if ( Features::ignoreunicode->IsEnabled() )
	{
		char s_szBuffer[ 4096 ];

		UTIL_ReplaceUnicodeChars( pszString, s_szBuffer );
		localize->ConvertANSIToUnicode( s_szBuffer, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
	else
	{
		localize->ConvertANSIToUnicode( pszString, s_wszBuffer, Q_ARRAYSIZE( s_wszBuffer ) );
	}
#endif

	int width, height;
	surface->GetTextSize( font, s_wszBuffer, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( s_wszBuffer, wcslen( s_wszBuffer ) );

	surface->DrawFlushText();
}

void CDrawing::DrawWideString( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString )
{
	enginesurface->ResetViewPort();

	int width, height;
	surface->GetTextSize( font, pwszString, width, height );

	ApplyTextAlignment( alignment, x, y, width, height );

	surface->DrawSetTextFont( font );
	surface->DrawSetTextColor( r, g, b, a );
	surface->DrawSetTextPos( x, y - height / 2 );
	surface->DrawPrintText( pwszString, wcslen( pwszString ) );

	surface->DrawFlushText();
}

//-----------------------------------------------------------------------------
// Misc. draw methods
//-----------------------------------------------------------------------------

void CDrawing::DrawCrosshair( int x, int y, int r, int g, int b, int a, int iSize /* = 10 */, int iGap /* = 4 */, int iThickness /* = 2 */ )
{
	int thickness = iThickness - 1;

	// Left
	FillArea( x - iGap - iSize + 1, y - ( thickness / 2 ), iSize, iThickness, r, g, b, a );

	// Right
	FillArea( x + iGap, y - ( thickness / 2 ), iSize, iThickness, r, g, b, a );

	// Up
	FillArea( x - ( thickness / 2 ), y - iGap - iSize + 1, iThickness, iSize, r, g, b, a );

	// Down
	FillArea( x - ( thickness / 2 ), y + iGap, iThickness, iSize, r, g, b, a );
}

void CDrawing::DrawDot( int x, int y, int r, int g, int b, int a, int iThickness /* = 2 */ )
{
	int thickness = iThickness - 1;

	FillArea( x - ( thickness / 2 ), y - ( thickness / 2 ), iThickness, iThickness, r, g, b, a );
}

void CDrawing::DrawCrosshairShadow( int x, int y, int r, int g, int b, int a, int iSize /* = 10 */, int iGap /* = 4 */, int iThickness /* = 2 */, int iShadowThickness /* = 1 */ )
{
	int thickness = iThickness - 1;

	// Left
	FillArea( x - iGap - iSize + 1 - iShadowThickness + ( iShadowThickness / 2 ),
			  y - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  iSize + iShadowThickness,
			  iThickness + iShadowThickness,
			  r, g, b, a );

	// Right
	FillArea( x + iGap - iShadowThickness + ( iShadowThickness / 2 ),
			  y - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  iSize + iShadowThickness,
			  iThickness + iShadowThickness,
			  r, g, b, a );

	// Up
	FillArea( x - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  y - iGap - iSize + 1 - iShadowThickness + ( iShadowThickness / 2 ),
			  iThickness + iShadowThickness,
			  iSize + iShadowThickness,
			  r, g, b, a );

	// Down
	FillArea( x - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  y + iGap - iShadowThickness + ( iShadowThickness / 2 ),
			  iThickness + iShadowThickness,
			  iSize + iShadowThickness,
			  r, g, b, a );
}

void CDrawing::DrawDotShadow( int x, int y, int r, int g, int b, int a, int iThickness /* = 2 */, int iShadowThickness /* = 1 */ )
{
	int thickness = iThickness - 1;

	FillArea( x - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  y - ( thickness / 2 ) - iShadowThickness + ( iShadowThickness / 2 ),
			  iThickness + iShadowThickness,
			  iThickness + iShadowThickness,
			  r, g, b, a );
}

//-----------------------------------------------------------------------------
// Init sprites
//-----------------------------------------------------------------------------

void CDrawing::InitSprites( void )
{
	char szSpritePath[ MAX_PATH ];
	int iSpriteResolution = ( gameutils->GetScreenWidth() < 640 ) ? 320 : 640;

	if ( m_pSpriteList != NULL )
	{
		for ( int i = 0; i < 10; i++ )
		{
			client_sprite_t *pSprite = m_NumberSpritePointers[ i ];

			snprintf( szSpritePath, sizeof( szSpritePath ), "sprites/%s.spr", pSprite->szSprite );
			m_NumberSprites[ i ] = cl_enginefuncs->pfnSPR_Load( szSpritePath );
		}

		return;
	}

	m_pSpriteList = cl_enginefuncs->pfnSPR_GetList( (char *)( "sprites/hud.txt" ), &m_iSpriteCount );
	if ( m_pSpriteList != NULL )
	{
		for ( int i = 0; i < m_iSpriteCount; i++ )
		{
			client_sprite_t *pSprite = m_pSpriteList + i;

			char num = *( pSprite->szName + 7 );

			if ( pSprite->iRes == iSpriteResolution &&
				 strstr( pSprite->szName, "number_" ) == pSprite->szName &&
				 *( pSprite->szName + 8 ) == '\0'
				 && num >= '0' && num <= '9' )
			{
				int digit = int( num - '0' );

				m_NumberSpritePointers[ digit ] = pSprite;
				m_NumberSpriteRects[ digit ] = pSprite->rc;

				snprintf( szSpritePath, sizeof( szSpritePath ), "sprites/%s.spr", pSprite->szSprite );
				m_NumberSprites[ digit ] = cl_enginefuncs->pfnSPR_Load( szSpritePath );

				if ( !digit )
				{
					m_iNumberWidth = pSprite->rc.right - pSprite->rc.left;
					m_iNumberHeight = pSprite->rc.bottom - pSprite->rc.top;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDrawing::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// HUD_VidInit event
	InitSprites();

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDrawing::CDrawing( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_hFontESP = 0;
	m_hFontESP2 = 0;
	m_hFontFriends = 0;
	m_hFontVotePopup = 0;
	m_hFontChat = 0;
	m_hFontInputs = 0;

	m_iNumberWidth = 0;
	m_iNumberHeight = 0;

	m_iSpriteCount = 0;
	m_pSpriteList = NULL;

	memset( m_NumberSprites, 0, sizeof( HSPRITE_HL ) * 10 );
	memset( m_NumberSpriteRects, 0, sizeof( Rect ) * 10 );
	memset( m_NumberSpritePointers, 0, Q_ARRAYSIZE( m_NumberSpritePointers ) );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CDrawing::Load( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CDrawing::PostLoad( void )
{
	int iWidth = gameutils->GetScreenWidth();

	m_hFontESP = surface->CreateFont();
	m_hFontESP2 = surface->CreateFont();
	m_hFontFriends = surface->CreateFont();
	m_hFontVotePopup = surface->CreateFont();
	m_hFontChat = surface->CreateFont();
	m_hFontInputs = surface->CreateFont();

	surface->AddGlyphSetToFont( m_hFontESP, "Verdana", iWidth <= 800 ? 8 : 12, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
	surface->AddGlyphSetToFont( m_hFontESP2, "Verdana", iWidth <= 800 ? 22 : 26, FW_SEMIBOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
	surface->AddGlyphSetToFont( m_hFontFriends, "Verdana", iWidth <= 800 ? 16 : 20, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
	surface->AddGlyphSetToFont( m_hFontVotePopup, "Lucida-Console", iWidth <= 800 ? 16 : 20, FW_EXTRABOLD, NULL, NULL, vgui::ISurface::FONTFLAG_NONE, 0, 0 );
	surface->AddGlyphSetToFont( m_hFontChat, "Tahoma", iWidth <= 800 ? 16 : 20, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
#ifdef LINUX
	surface->AddGlyphSetToFont( m_hFontInputs, "Verdana", iWidth <= 800 ? 8 : 12, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
#else
	surface->AddGlyphSetToFont( m_hFontInputs, "Verdana", iWidth <= 800 ? 10 : 14, FW_BOLD, NULL, NULL, vgui::ISurface::FONTFLAG_DROPSHADOW, 0, 0 );
#endif

	InitSprites();

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CDrawing::Unload( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
}

// SvenInt (c) Sw1ft
// r_drawing.h

#ifndef SINT_FEATURE_DRAWING_H
#define SINT_FEATURE_DRAWING_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Font alignment
//-----------------------------------------------------------------------------

typedef enum
{
	FONT_ALIGN_LEFT = 0,
	FONT_ALIGN_RIGHT = ( 1 << 0 ),
	FONT_ALIGN_CENTER = ( 1 << 1 ),
	FONT_ALIGN_LEFT_BOT = ( 1 << 2 ),
	FONT_ALIGN_LEFT_TOP = ( 1 << 3 )
} FontAlignFlags_t;

//-----------------------------------------------------------------------------
// Drawing feature
//-----------------------------------------------------------------------------

class CDrawing final : public CBaseFeature, IHookEventListener
{
public:
	CDrawing( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline vgui::HFont GetFontESP( void ) { return m_hFontESP; }
	inline vgui::HFont GetFontESP2( void ) { return m_hFontESP2; }
	inline vgui::HFont GetFontFriends( void ) { return m_hFontFriends; }
	inline vgui::HFont GetFontVotePopup( void ) { return m_hFontVotePopup; }
	inline vgui::HFont GetFontChat( void ) { return m_hFontChat; }
	inline vgui::HFont GetFontInputs( void ) { return m_hFontInputs; }

public:
	void FillArea( int x, int y, int w, int h, int r, int g, int b, int a );

	void DrawRect( int x, int y, int w, int h, int r, int g, int b, int a );
	void DrawOutlinedRect( int x, int y, int w, int h, int r, int g, int b, int a );

	void DrawCircle( float position[ 2 ], float points, float radius, int r, int g, int b, int a );
	void DrawCircle3D( Vector &position, float points, float radius, int r, int g, int b, int a );
	void DrawLine( int x0, int y0, int x1, int y1, int r, int g, int b, int a );

	void BoxCorner( int x, int y, int w, int h, int lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void BoxCornerOutline( int x, int y, int w, int h, int lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a );

	void DrawCoalBox( int x, int y, int w, int h, int lw, int r, int g, int b, int a );
	void DrawOutlineCoalBox( int x, int y, int w, int h, int  lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void DrawPlayerBox( int x, int y, int w, int h, int r, int g, int b, int a, bool outline );
	void DrawBox( int x, int y, int w, int h, int r, int g, int b, int a );
	void BoxOutline( float x, float y, float w, float h, float lw, uint8_t r, uint8_t g, uint8_t b, uint8_t a );
	void Box( int x, int y, int w, int h, int lw, int r, int g, int b, int a );

	int DrawDigit( int digit, int x, int y, int r, int g, int b );
	int DrawDigit( int digit, int x, int y, int r, int g, int b, FontAlignFlags_t alignment );
	int DrawNumber( int number, int x, int y, int r, int g, int b, FontAlignFlags_t alignment, int fieldMinWidth = 1 );

	void DrawTexture( int id, int x0, int y0, int x1, int y1, int r = 255, int g = 255, int b = 255, int a = 255 );

	void DrawStringF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ... );
	void DrawStringExF( vgui::HFont font, int x, int y, int r, int g, int b, int a, int &width, int &height, FontAlignFlags_t alignment, const char *pszString, ... );
	void DrawStringACPF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString, ... );
	void DrawWideStringF( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwzString, ... );

	void DrawString( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString );
	void DrawStringEx( vgui::HFont font, int x, int y, int r, int g, int b, int a, int &width, int &height, FontAlignFlags_t alignment, const char *pszString );
	void DrawStringACP( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const char *pszString );
	void DrawWideString( vgui::HFont font, int x, int y, int r, int g, int b, int a, FontAlignFlags_t alignment, const wchar_t *pwszString );

	void DrawCrosshair( int x, int y, int r, int g, int b, int a, int iSize = 10, int iGap = 4, int iThickness = 2 );
	void DrawDot( int x, int y, int r, int g, int b, int a, int iThickness = 2 );
	void DrawCrosshairShadow( int x, int y, int r, int g, int b, int a, int iSize = 10, int Giap = 4, int iThickness = 2, int iShadowThickness = 1 );
	void DrawDotShadow( int x, int y, int r, int g, int b, int a, int iThickness = 2, int iShadowThickness = 1 );

public:
	int GetNumberSpriteWidth( void );
	int GetNumberSpriteHeight( void );

private:
	void InitSprites( void );
	void ApplyTextAlignment( FontAlignFlags_t alignment, int &x, int &y, int textWidth, int textHeight );

private:
	vgui::HFont m_hFontESP;
	vgui::HFont m_hFontESP2;
	vgui::HFont m_hFontFriends;
	vgui::HFont m_hFontVotePopup;
	vgui::HFont m_hFontChat;
	vgui::HFont m_hFontInputs;

	int m_iNumberWidth;
	int m_iNumberHeight;

	int m_iSpriteCount;
	client_sprite_t *m_pSpriteList;

	HSPRITE_HL m_NumberSprites[ 10 ];
	wrect_t m_NumberSpriteRects[ 10 ];
	client_sprite_t *m_NumberSpritePointers[ 10 ];
};

inline void CDrawing::ApplyTextAlignment( FontAlignFlags_t alignment, int &x, int &y, int textWidth, int textHeight )
{
	if ( alignment & FONT_ALIGN_RIGHT )
		x -= textWidth;

	if ( alignment & FONT_ALIGN_CENTER )
		x -= textWidth / 2;

	if ( alignment & FONT_ALIGN_LEFT_BOT )
		y += textHeight / 2;

	if ( alignment & FONT_ALIGN_LEFT_TOP )
		y -= textHeight / 2;
}

EXTERN_FEATURE( CDrawing, drawing );

#endif // SINT_FEATURE_DRAWING_H

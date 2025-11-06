// SvenInt (c) Sw1ft
// gameconsoledialog.h

#ifndef SINT_GAMECONSOLE_DIALOG_H
#define SINT_GAMECONSOLE_DIALOG_H

#ifdef _WIN32
#pragma once
#endif

class Color;

class CGameConsoleDialog
{
public:
#ifdef WIN32
	char pad_1[ 280 ];
#else
	char pad_1[ 276 ];
#endif

	void *m_pRichText;

	char pad_2[ 12 ];

	Color *m_pDefaultTextColor;
	Color *m_pDevTextColor;
};

#endif // SINT_GAMECONSOLE_DIALOG_H
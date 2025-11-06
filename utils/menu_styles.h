// SvenInt (c) Sw1ft
// menu_styles.h

#ifndef SINT_MENU_STYLES_H
#define SINT_MENU_STYLES_H

#ifdef _WIN32
#pragma once
#endif

void LoadMenuTheme( int i );

void SaveCurrentStyle( void );
void LoadSavedStyle( void );

void InitImGuiStyles( void );
void StyleColors_Custom( void );

#endif // SINT_MENU_STYLES_H
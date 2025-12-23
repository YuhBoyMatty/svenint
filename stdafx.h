// SvenInt (c) Sw1ft
// stdafx.h

#ifndef STDAFX_H
#define STDAFX_H

#ifdef _WIN32
#pragma once
#endif

#ifndef _WIN32
#ifndef LINUX
#define LINUX
#endif
#endif

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef SINT_USE_GLEW
#include <GL/glew.h>
#else
#ifdef _WIN32
#include <gl/GL.h>
#else
#include <GL/gl.h>
#endif
#endif

#include "svenint.h"
#include "game/mathlib.h"
#include "game/hlsdk_mini.h"
#include "utils/memalloc.h"
#include "gamedata.h"

#ifndef _WIN32
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#ifndef _wtoi
#define _wtoi(str) wcstol(str, 0, 10)
#endif
#ifdef __STDC_LIB_EXT1__
constexpr bool can_have_strcpy_s = true;
#else
constexpr bool can_have_strcpy_s = false;
#endif

#include <algorithm>
#endif

#endif // !STDAFX_H

#pragma hdrstop

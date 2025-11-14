// SvenInt (c) Sw1ft
// svenint.h

#ifndef SVENINT_H
#define SVENINT_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Macro Definitions
//-----------------------------------------------------------------------------

#define SVENINT_FOLDER "svenint/"
#define SVENINT_FOLDER_NAME "svenint"
#define SVENINT_RESOURCE_FOLDER "svencoop/svenint/"
#define SVENINT_CONFIG_FOLDER "config/"
#define SVENINT_SHADERS_CONFIG_FOLDER "config/shaders/"

#define SVENINT_MAKE_PATH( path ) SVENINT_FOLDER path
#define SVENINT_RESOURCE_MAKE_PATH( path ) SVENINT_RESOURCE_FOLDER path

#define _SVENINT_DO_STRING(v) #v
#define _SVENINT_STRING(v) _SVENINT_DO_STRING(v)

#define SVENINT_MAJOR_VERSION 3
#define SVENINT_MINOR_VERSION 1
#define SVENINT_PATCH_VERSION 1

#define SVENINT_MAJOR_VERSION_STRING _SVENINT_STRING( SVENINT_MAJOR_VERSION )
#define SVENINT_MINOR_VERSION_STRING _SVENINT_STRING( SVENINT_MINOR_VERSION )
#define SVENINT_PATCH_VERSION_STRING _SVENINT_STRING( SVENINT_PATCH_VERSION )

#define SVENINT_VERSION SVENINT_MAJOR_VERSION_STRING "." SVENINT_MINOR_VERSION_STRING "." SVENINT_PATCH_VERSION_STRING

#endif // SVENINT_H
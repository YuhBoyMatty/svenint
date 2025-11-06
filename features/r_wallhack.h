// SvenInt (c) Sw1ft
// r_wallhack.h

#ifndef SINT_FEATURE_WALLHACK_H
#define SINT_FEATURE_WALLHACK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Viewmodel tweaks feature
//-----------------------------------------------------------------------------

class CWallhack final : public CBaseFeature
{
public:
	CWallhack( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	inline bool WallhackObjects( void ) { return m_pObjects->GetBool(); }
	void glBeginEvent( GLenum mode );

private:
	CMenuValueBool *m_pObjects;
	CMenuValueBool *m_pNegative;
	CMenuValueBool *m_pLambert;
	CMenuValueBool *m_pWireframe;
	CMenuValueBool *m_pWireframeModels;
	CMenuValueFloat *m_pWireframeWidth;
	CMenuValueColorRGB *m_pWireframeColor;

	void *m_pfnglBegin;
	DetourHandle_t m_hglBegin;
};

EXTERN_FEATURE( CWallhack, wallhack );

#endif // SINT_FEATURE_WALLHACK_H
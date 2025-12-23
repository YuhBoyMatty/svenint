// SvenInt (c) Sw1ft
// visual_ignore_unicode.cpp

#include "stdafx.h"
#include "visual_ignore_unicode.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CIgnoreUnicode, ignoreunicode, "Visual", "Ignore Unicode" );

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CIgnoreUnicode::CIgnoreUnicode( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CIgnoreUnicode::Load( void )
{
	Modules::menu->BindFeature( this );
	return true;
}
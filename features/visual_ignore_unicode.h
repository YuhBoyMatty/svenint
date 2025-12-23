// SvenInt (c) Sw1ft
// visual_ignore_unicode.h

#ifndef SINT_FEATURE_IGNORE_UNICODE_H
#define SINT_FEATURE_IGNORE_UNICODE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Ignore unicode/utf feature
//-----------------------------------------------------------------------------

class CIgnoreUnicode final : public CBaseFeature
{
public:
	CIgnoreUnicode( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
};

EXTERN_FEATURE( CIgnoreUnicode, ignoreunicode );

#endif // SINT_FEATURE_IGNORE_UNICODE_H
// SvenInt (c) Sw1ft
// player_drop_empty_weapon.h

#ifndef SINT_FEATURE_DROP_EMPTY_WEAPON_H
#define SINT_FEATURE_DROP_EMPTY_WEAPON_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Drop empty weapon feature
//-----------------------------------------------------------------------------

class CDropEmptyWeapon final : public CBaseFeature
{
public:
	CDropEmptyWeapon( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void OnDisable( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	void DropEmptyWeapon( void );
};

EXTERN_FEATURE( CDropEmptyWeapon, dropemptyweapon );

#endif // SINT_FEATURE_DROP_EMPTY_WEAPON_H
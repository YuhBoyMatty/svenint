// SvenInt (c) Sw1ft
// misc_random_generator.h

#ifndef SINT_FEATURE_RANDOM_GENERATOR_H
#define SINT_FEATURE_RANDOM_GENERATOR_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#ifndef NTAB
#define NTAB 32
#endif

//-----------------------------------------------------------------------------
// Random generator feature
//-----------------------------------------------------------------------------

class CRandomGenerator final : public CBaseFeature
{
public:
	CRandomGenerator( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;

public:
	void			Init( void );

	void			SetSeed( int iSeed );

	float			RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f );
	int				RandomInt( int iMinVal, int iMaxVal );
	float			RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f );

private:
	int GenerateRandomNumber( void );

	int m_idum;
	int m_iy;
	int m_iv[ NTAB ];
};

EXTERN_FEATURE( CRandomGenerator, random );

#endif // SINT_FEATURE_RANDOM_GENERATOR_H
// SvenInt (c) Sw1ft
// misc_random_generator.cpp

#include "stdafx.h"
#include "misc_random_generator.h"

#include <time.h>
#include <math.h>

using namespace Globals;

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define IA 16807
#define IM 2147483647
#define IQ 127773
#define IR 2836
#define NDIV (1+(IM-1)/NTAB)
#define MAX_RANDOM_RANGE 0x7FFFFFFFUL

#define AM (1.0/IM)
#define EPS 1.2e-7
#define RNMX (1.0-EPS)

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CRandomGenerator, random, "Misc", "Random Generator" );

//-----------------------------------------------------------------------------
// CRandomGenerator
//-----------------------------------------------------------------------------

void CRandomGenerator::Init( void )
{
	//SetSeed(0);
	SetSeed( static_cast<int>( time( 0 ) ) );
}

void CRandomGenerator::SetSeed( int iSeed )
{
	m_idum = ( ( iSeed < 0 ) ? iSeed : -iSeed );
	m_iy = 0;
}

int CRandomGenerator::GenerateRandomNumber()
{
	int j;
	int k;

	if ( m_idum <= 0 || !m_iy )
	{
		if ( -( m_idum ) < 1 )
			m_idum = 1;
		else
			m_idum = -( m_idum );

		for ( j = NTAB + 7; j >= 0; j-- )
		{
			k = ( m_idum ) / IQ;
			m_idum = IA * ( m_idum - k * IQ ) - IR * k;
			if ( m_idum < 0 )
				m_idum += IM;
			if ( j < NTAB )
				m_iv[ j ] = m_idum;
		}
		m_iy = m_iv[ 0 ];
	}
	k = ( m_idum ) / IQ;
	m_idum = IA * ( m_idum - k * IQ ) - IR * k;
	if ( m_idum < 0 )
		m_idum += IM;
	j = m_iy / NDIV;

	m_iy = m_iv[ j ];
	m_iv[ j ] = m_idum;

	return m_iy;
}

#pragma warning(push)
#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4305)

float CRandomGenerator::RandomFloat( float flLow, float flHigh )
{
	float fl = AM * GenerateRandomNumber();
	if ( fl > RNMX )
	{
		fl = RNMX;
	}
	return ( fl * ( flHigh - flLow ) ) + flLow;
}

float CRandomGenerator::RandomFloatExp( float flMinVal, float flMaxVal, float flExponent )
{
	float fl = AM * GenerateRandomNumber();
	if ( fl > RNMX )
	{
		fl = RNMX;
	}
	if ( flExponent != 1.0f )
	{
		fl = powf( fl, flExponent );
	}
	return ( fl * ( flMaxVal - flMinVal ) ) + flMinVal;
}

#pragma warning(pop)
#pragma warning(pop)

int CRandomGenerator::RandomInt( int iLow, int iHigh )
{
	unsigned int maxAcceptable;
	unsigned int x = iHigh - iLow + 1;
	unsigned int n;
	if ( x <= 1 || MAX_RANDOM_RANGE < x - 1 )
	{
		return iLow;
	}

	maxAcceptable = MAX_RANDOM_RANGE - ( ( MAX_RANDOM_RANGE + 1 ) % x );
	do
	{
		n = GenerateRandomNumber();
	} while ( n > maxAcceptable );

	return iLow + ( n % x );
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CRandomGenerator::CRandomGenerator( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	Init();
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CRandomGenerator::Load( void )
{
	return true;
}
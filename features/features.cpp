// SvenInt (c) Sw1ft
// features.cpp

#include <stdarg.h>

#include "stdafx.h"
#include "base_feature.h"
#include "game/dbg.h"

CBaseFeature *CBaseFeature::s_pBaseFeaturesList = NULL;

static CBaseFeature *s_pLastLoaded = NULL;
static bool s_bFeaturesLoaded = false;
static bool s_bShaderFeaturesLoaded = false;
static char szFormattedMsg[ 2048 ];

//-----------------------------------------------------------------------------
// Implement control options for features
//-----------------------------------------------------------------------------

bool AreFeaturesLoaded()
{
	return s_bFeaturesLoaded;
}

bool LoadFeatures()
{
	if ( s_bFeaturesLoaded )
		return false;

	bool bAllFeaturesLoaded = true;
	s_bFeaturesLoaded = true;

	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		pNext = pCur->m_pNext;
		if ( pCur->GetCategoryName() != "Shaders" )
		{
			if ( pCur->Load() )
			{
				pCur->SetLoaded( true );

				Msg( "[SvenInt] Loaded feature \"%s\"\n", pCur->GetName() );
			}
			else
			{
				pCur->SetLoaded( false );
				pCur->SetInitiallyDisabled();
				bAllFeaturesLoaded = false;

				Warning( "[SvenInt] Failed to load feature \"%s\"\n", pCur->GetName() );
			}
		}

		if ( pNext == NULL )
		{
			s_pLastLoaded = pCur;
		}

		pCur = pNext;
	}

	return bAllFeaturesLoaded;
}

void PostLoadFeatures()
{
	if ( !s_bFeaturesLoaded )
		return;

	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		pNext = pCur->m_pNext;

		if ( pCur->GetCategoryName() != "Shaders" && pCur->IsLoaded() )
		{
			pCur->PostLoad();
			if ( pCur->IsEnabled() )
				pCur->OnEnable();
		}

		pCur = pNext;
	}
}

void UnloadFeatures()
{
	if ( !s_bFeaturesLoaded )
		return;

	s_bFeaturesLoaded = false;

	CBaseFeature *pCur, *pPrev;
	pCur = s_pLastLoaded;

	while ( pCur != NULL )
	{
		pPrev = pCur->m_pPrev;

		if ( pCur->IsLoaded() )
		{
			pCur->Unload();
		}

		pCur->SetLoaded( false );
		pCur = pPrev;
	}
}

//-----------------------------------------------------------------------------
// Shader features
//-----------------------------------------------------------------------------

bool AreShaderFeaturesLoaded()
{
	return s_bShaderFeaturesLoaded;
}

bool LoadShaderFeatures()
{
	if ( s_bShaderFeaturesLoaded )
		return false;

	bool bAllFeaturesLoaded = true;
	s_bShaderFeaturesLoaded = true;

	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		pNext = pCur->m_pNext;
		if ( pCur->GetCategoryName() == "Shaders" )
		{
			if ( pCur->Load() )
			{
				pCur->SetLoaded( true );

				//Msg( "[SvenInt] Loaded feature \"%s\"\n", pCur->GetName() );
			}
			else
			{
				pCur->SetLoaded( false );
				pCur->SetInitiallyDisabled();
				bAllFeaturesLoaded = false;

				Warning( "[SvenInt] Failed to load feature \"%s\"\n", pCur->GetName() );
			}
		}

		if ( pNext == NULL )
		{
			s_pLastLoaded = pCur;
		}

		pCur = pNext;
	}

	return bAllFeaturesLoaded;
}

void PostLoadShaderFeatures()
{
	if ( !s_bShaderFeaturesLoaded )
		return;

	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		pNext = pCur->m_pNext;

		if ( pCur->GetCategoryName() == "Shaders" && pCur->IsLoaded() )
		{
			pCur->PostLoad();
			if ( pCur->IsEnabled() )
				pCur->OnEnable();
		}

		pCur = pNext;
	}
}

void DisableShaderFeatures()
{
	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		pNext = pCur->m_pNext;

		if ( pCur->GetCategoryName() == "Shaders" )
		{
			pCur->SetLoaded( false );
			pCur->SetInitiallyDisabled();
			s_bShaderFeaturesLoaded = false;
		}

		pCur = pNext;
	}
}

//-----------------------------------------------------------------------------
// CBaseFeature
//-----------------------------------------------------------------------------

CBaseFeature::CBaseFeature()
{
	static const char *empty_string = "";

	m_bLoaded = false;
	m_bEnabled = true;

	m_pszCategoryName = empty_string;
	m_pszName = empty_string;

	m_pNext = NULL;
	m_pPrev = NULL;

	m_pNext = s_pBaseFeaturesList;

	if ( s_pBaseFeaturesList )
	{
		s_pBaseFeaturesList->m_pPrev = this;
	}

	s_pBaseFeaturesList = this;
}

CBaseFeature::CBaseFeature( const char *pszCategoryName, const char *pszName ) : CBaseFeature()
{
	SetFeatureInfo( pszCategoryName, pszName );
}

void CBaseFeature::Enable( void )
{
	if ( m_bEnabled )
		return;

	m_bEnabled = true;
	OnEnable();
}

void CBaseFeature::Disable( void )
{
	if ( !m_bEnabled )
		return;

	m_bEnabled = false;
	OnDisable();
}

bool CBaseFeature::Toggle( void )
{
	m_bEnabled = !m_bEnabled;

	if ( m_bEnabled )
		OnEnable();
	else
		OnDisable();

	return m_bEnabled;
}

const char *CBaseFeature::GetCategoryName( void )
{
	return m_pszCategoryName;
}

const char *CBaseFeature::GetName( void )
{
	return m_pszName;
}

void CBaseFeature::SetFeatureInfo( const char *pszCategoryName, const char *pszName )
{
	m_pszCategoryName = pszCategoryName;
	m_pszName = pszName;
}

void CBaseFeature::PrintMsg( const char *pszMessage, ... )
{
	va_list args;
	va_start( args, pszMessage );
	vsnprintf( szFormattedMsg, Q_ARRAYSIZE( szFormattedMsg ), pszMessage, args );
	va_end( args );

	Msg( "<%s> %s", GetName(), szFormattedMsg );
}

void CBaseFeature::PrintWarning( const char *pszMessage, ... )
{
	va_list args;
	va_start( args, pszMessage );
	vsnprintf( szFormattedMsg, Q_ARRAYSIZE( szFormattedMsg ), pszMessage, args );
	va_end( args );

	Warning( "<%s> %s", GetName(), szFormattedMsg );
}

void CBaseFeature::PrintWarning2( const char *pszMessage, ... )
{
	va_list args;
	va_start( args, pszMessage );
	vsnprintf( szFormattedMsg, Q_ARRAYSIZE( szFormattedMsg ), pszMessage, args );
	va_end( args );

	Warning2( "<%s> %s", GetName(), szFormattedMsg );
}

void CBaseFeature::PrintFeatures( void )
{
	Color feature_name_color( 80, 186, 255, 255 );

	Msg( "--------- List of SvenInt Features ---------\n" );

	int i = 0;
	bool loaded, enabled;
	CBaseFeature *pCur, *pNext;
	pCur = CBaseFeature::s_pBaseFeaturesList;

	while ( pCur != NULL )
	{
		i++;
		pNext = pCur->m_pNext;

		loaded = pCur->IsLoaded();
		enabled = pCur->IsEnabled();

	#if 0
		Msg( "%d. ", i );
	#else
		Msg( "[%s] ", pCur->GetCategoryName() );
	#endif
		ConColorMsg( feature_name_color, pCur->GetName() );
		Msg( " is " );

		if ( !loaded )
		{
			Warning( "failed to load\n" );
		}
		else if ( enabled )
		{
			ConColorMsg( { 90, 255, 90, 255 }, "running\n" );
		}
		else
		{
			ConColorMsg( { 255, 255, 127, 255 }, "not active\n" );
		}

		pCur = pNext;
	}

	Msg( "--------------------------------------------\n" );
}

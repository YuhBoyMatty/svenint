// SvenInt (c) Sw1ft
// prof.cpp

#include "stdafx.h"
#include "prof.h"

/*
#include <stack>

void IterateProfileNodes( CProfileNode *pStartNode )
{
    std::stack<CProfileNode *> nodes;
    nodes.push( pStartNode );

    while ( !nodes.empty() )
    {
        CProfileNode *pNode = nodes.top();
        nodes.pop();

        // ...

        for ( CProfileNode *pChild : pNode->GetChildren() )
            nodes.push( pChild );
    }
}

void PrintProfilesTree( CProfileNode *pNode, int iDepth = 0 )
{
	if ( pNode == NULL )
		return;

	for ( int i = 0; i < iDepth; i++ )
        Msg( "\t" );

	Msg( "%s\n", pNode->GetName() );

	for ( CProfileNode *pChild : pNode->GetChildren() )
		PrintProfilesTree( pChild, iDepth + 1 );
}
*/

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

#ifdef PROF_ENABLED
CProfileNode gProfileRoot( "SvenInt", true );
CProfileNode *CProfileNode::sm_pCurrentNode = NULL;
#endif

//-----------------------------------------------------------------------------
// Start measuring
//-----------------------------------------------------------------------------

void CProfileMeasure::Start( void )
{
#ifdef WIN32
    QueryPerformanceFrequency( &m_freq );
    QueryPerformanceCounter( &m_start );
#else
    clock_gettime( CLOCK_MONOTONIC, &m_start );
#endif
}

//-----------------------------------------------------------------------------
// Stop measuring
//-----------------------------------------------------------------------------

double CProfileMeasure::Elapsed( void )
{
#ifdef WIN32
    if ( !QueryPerformanceCounter( &m_end ) )
        return -1.0;

    const double sec = (double)( m_end.QuadPart - m_start.QuadPart ) / (double)m_freq.QuadPart;
    return sec * 1000000.0; // microsec
#else
    if ( clock_gettime( CLOCK_MONOTONIC, &m_end ) != 0 )
        return -1.0;

    long long ns = ( m_end.tv_sec - m_start.tv_sec ) * 1000000000LL + ( m_end.tv_nsec - m_start.tv_nsec );
    return (double)ns / 1000.0; // microsec
#endif
}

//-----------------------------------------------------------------------------
// CProfileNodeMeasure
//-----------------------------------------------------------------------------

#ifdef PROF_ENABLED
CProfileNodeMeasure::CProfileNodeMeasure( CProfileNode *pNode )
{
    extern ConVar sc_prof;
    m_pNode = NULL;

    if ( !sc_prof.GetBool() || pNode->IsMeasuring() )
        return;

    pNode->SetParent( CProfileNode::sm_pCurrentNode );
    CProfileNode::sm_pCurrentNode->AddChild( pNode );
    CProfileNode::sm_pCurrentNode = pNode;

    m_pNode = pNode;
    m_pNode->SetMeasuring( true );

    m_measure.Start();
}
#endif

//-----------------------------------------------------------------------------
// ~CProfileNodeMeasure
//-----------------------------------------------------------------------------

#ifdef PROF_ENABLED
CProfileNodeMeasure::~CProfileNodeMeasure()
{
    if ( m_pNode == NULL || !m_pNode->IsMeasuring() )
        return;

    m_pNode->SaveMeasure( m_measure.Elapsed() );
    m_pNode->SetMeasuring( false );
    CProfileNode::sm_pCurrentNode = m_pNode->GetParent();
}
#endif

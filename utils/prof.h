// SvenInt (c) Sw1ft
// prof.h

#ifndef SINT_PROF_H
#define SINT_PROF_H

#ifdef _WIN32
#pragma once
#endif

#define PROF_MEASURE( varName ) CProfileMeasure varName
#define PROF_MEASURE_START( varName ) varName.Start()
#define PROF_MEASURE_ELAPSED( varName ) varName.Elapsed()

#define PROF_TIME_SEC( time_us ) ( time_us / 1000000.0 )
#define PROF_TIME_MILLISEC( time_us ) ( time_us / 1000.0 )
#define PROF_TIME_NANOSEC( time_us ) ( time_us * 1000.0 )

//-----------------------------------------------------------------------------
// A simple measuring clock
//-----------------------------------------------------------------------------

class CProfileMeasure
{
public:
	void Start( void );
	double Elapsed( void );

private:
#ifdef WIN32
	LARGE_INTEGER m_freq;
	LARGE_INTEGER m_start;
	LARGE_INTEGER m_end;
#else
	timespec m_start;
	timespec m_end;
#endif
};

#define PROF_ENABLED
#ifdef PROF_ENABLED

class CProfileNode;
extern CProfileNode gProfileRoot;

#define PROF( name ) static CProfileNode __prof_node__( name, false ); \
	CProfileNodeMeasure __prof_node_measure__( &( __prof_node__ ) )
#ifdef WIN32
#define PROF_AUTO_NAME() PROF( __FUNCTION__ )
#else
#define PROF_AUTO_NAME() PROF( __PRETTY_FUNCTION__ )
#endif

#define PROF_SCOPE_BEGIN( name ) do { PROF( name )
#define PROF_SCOPE_END() } while ( 0 )

//-----------------------------------------------------------------------------
// CProfileNode
//-----------------------------------------------------------------------------

class CProfileNode
{
	friend class CProfileNodeMeasure;

public:
	CProfileNode( const char *pszName, bool bRoot = false )
	{
		if ( bRoot )
			sm_pCurrentNode = this;

		m_pParent = NULL;
		m_pszName = pszName;
		m_dblLastMeasureInterval = -1.0;
		m_nCalls = 0;
		m_bMeasuring = false;
	}

	~CProfileNode()
	{
	}

	inline void AddChild( CProfileNode *pNode )
	{
		auto it = std::find( m_children.begin(), m_children.end(), pNode );
		if ( it != m_children.end() )
			return;

		m_children.push_back( pNode );
	}
	inline std::vector<CProfileNode *> &GetChildren( void )
	{
		return m_children;
	}

	inline void SetParent( CProfileNode *pNode ) { m_pParent = pNode; }
	inline CProfileNode *GetParent( void ) const { return m_pParent; }

	inline void SaveMeasure( double interval )
	{
		m_dblLastMeasureInterval = interval;
		m_nCalls++;
		m_bMeasuring = false;
	}

	inline double GetTime( void ) const { return m_dblLastMeasureInterval; }
	inline unsigned int GetCalls( void ) const { return m_nCalls; }
	inline const char *GetName( void ) const { return m_pszName; }

	inline void SetMeasuring( bool state ) { m_bMeasuring = state; }
	inline bool IsMeasuring( void ) const { return m_bMeasuring; }

	inline double GetChildrenTime( void ) const
	{
        double dblTime = 0.0;

        for ( CProfileNode *pNode : m_children )
            dblTime += pNode->GetTime();

        return dblTime;
    }
	inline double GetChildrenCalls( void ) const
	{
        unsigned int nCalls = 0;

        for ( CProfileNode *pNode : m_children )
            nCalls += pNode->GetCalls();

        return nCalls;
    }

protected:
	static CProfileNode *sm_pCurrentNode;
	CProfileNode *m_pParent;

private:
	std::vector<CProfileNode *> m_children;

	const char *m_pszName;
	double m_dblLastMeasureInterval;
	unsigned int m_nCalls;
	bool m_bMeasuring;
};

//-----------------------------------------------------------------------------
// CProfileNodeMeasure
//-----------------------------------------------------------------------------

class CProfileNodeMeasure
{
public:
	CProfileNodeMeasure( CProfileNode *pNode );
	~CProfileNodeMeasure();

private:
	CProfileMeasure m_measure;
	CProfileNode *m_pNode;
};

#else

#define PROF( name ) (void)0
#define PROF_AUTO_NAME() (void)0

#define PROF_SCOPE_BEGIN( name ) do {
#define PROF_SCOPE_END() } while ( 0 )

#endif

#endif // SINT_PROF_H

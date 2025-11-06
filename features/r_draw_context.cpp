// SvenInt (c) Sw1ft
// r_draw_context.cpp

#include "stdafx.h"
#include "r_draw_context.h"

#include <gl/GL.h>

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDrawContext, drawcontext, "Render", "Draw Context" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_debug_draw_point, "Visualize point" )
{
	if ( args.ArgC() >= 10 )
	{
		Vector vecPoint( atof( args[ 1 ] ), atof( args[ 2 ] ), atof( args[ 3 ] ) );

		unsigned char r = atoi( args[ 4 ] );
		unsigned char g = atoi( args[ 5 ] );
		unsigned char b = atoi( args[ 6 ] );
		unsigned char a = atoi( args[ 7 ] );

		float size = (float)atof( args[ 8 ] );
		float duration = (float)atof( args[ 9 ] );

		THIS_FEATURE()->DrawPoint( vecPoint, { r, g, b, a }, size, duration );
	}
	else
	{
		ConMsg( "Usage: sc_debug_draw_line <x> <y> <z> <red> <green> <blue> <alpha> <size> <duration>\n" );
	}
}

CON_COMMAND( sc_debug_draw_line, "Visualize line" )
{
	if ( args.ArgC() >= 13 )
	{
		Vector vecStart( atof( args[ 1 ] ), atof( args[ 2 ] ), atof( args[ 3 ] ) );
		Vector vecEnd( atof( args[ 4 ] ), atof( args[ 5 ] ), atof( args[ 6 ] ) );

		unsigned char r = atoi( args[ 7 ] );
		unsigned char g = atoi( args[ 8 ] );
		unsigned char b = atoi( args[ 9 ] );
		unsigned char a = atoi( args[ 10 ] );

		float width = (float)atof( args[ 11 ] );
		float duration = (float)atof( args[ 12 ] );

		THIS_FEATURE()->DrawLine( vecStart, vecEnd, { r, g, b, a }, width, duration );
	}
	else
	{
		ConMsg( "Usage: sc_debug_draw_line <start x> <start y> <start z> <end x> <end y> <end z> <red> <green> <blue> <alpha> <width> <duration>\n" );
	}
}

CON_COMMAND( sc_debug_draw_box, "Visualize box" )
{
	if ( args.ArgC() >= 16 )
	{
		Vector vOrigin( atof( args[ 1 ] ), atof( args[ 2 ] ), atof( args[ 3 ] ) );
		Vector vMins( atof( args[ 4 ] ), atof( args[ 5 ] ), atof( args[ 6 ] ) );
		Vector vMaxs( atof( args[ 7 ] ), atof( args[ 8 ] ), atof( args[ 9 ] ) );

		//unsigned char r = atoi( args[ 10 ] );
		//unsigned char g = atoi( args[ 11 ] );
		//unsigned char b = atoi( args[ 12 ] );
		//unsigned char a = atoi( args[ 13 ] );

		bool wireframe = !!atoi( args[ 14 ] );
		float duration = (float)atof( args[ 15 ] );

		DrawBox( vOrigin,
				 vMins,
				 vMaxs,
				 (float)atof( args[ 10 ] ) / 255.f,
				 (float)atof( args[ 11 ] ) / 255.f,
				 (float)atof( args[ 12 ] ) / 255.f,
				 (float)atof( args[ 13 ] ) / 255.f,
				 4.f,
				 wireframe,
				 duration );

		//THIS_FEATURE()->DrawBox( vOrigin, vMins, vMaxs, { r, g, b, a }, duration );
	}
	else
	{
		ConMsg( "Usage: sc_debug_draw_box <x> <y> <z> <mins x> <mins y> <mins z> <maxs x> <maxs y> <maxs z> <r> <g> <b> <a> <wireframe: 0, 1> <duration>\n" );
	}
}

CON_COMMAND( sc_debug_draw_box_angles, "Visualize rotated box" )
{
	if ( args.ArgC() >= 18 )
	{
		Vector vOrigin( atof( args[ 1 ] ), atof( args[ 2 ] ), atof( args[ 3 ] ) );
		Vector vMins( atof( args[ 4 ] ), atof( args[ 5 ] ), atof( args[ 6 ] ) );
		Vector vMaxs( atof( args[ 7 ] ), atof( args[ 8 ] ), atof( args[ 9 ] ) );
		Vector vAngles( atof( args[ 10 ] ), atof( args[ 11 ] ), atof( args[ 12 ] ) );

		//unsigned char r = atoi( args[ 13 ] );
		//unsigned char g = atoi( args[ 14 ] );
		//unsigned char b = atoi( args[ 15 ] );
		//unsigned char a = atoi( args[ 16 ] );

		bool wireframe = !!atoi( args[ 17 ] );
		float duration = (float)atof( args[ 18 ] );

		DrawBoxAngles( vOrigin,
					   vMins,
					   vMaxs,
					   vAngles,
					   (float)atof( args[ 13 ] ) / 255.f,
					   (float)atof( args[ 14 ] ) / 255.f,
					   (float)atof( args[ 15 ] ) / 255.f,
					   (float)atof( args[ 16 ] ) / 255.f,
					   4.f,
					   wireframe,
					   duration );

		//THIS_FEATURE()->DrawBoxAngles( vOrigin, vMins, vMaxs, vAngles, { r, g, b, a }, duration );
	}
	else
	{
		ConMsg( "Usage: sc_debug_draw_box_angles <x> <y> <z> <mins x> <mins y> <mins z> <maxs x> <maxs y> <maxs z> <pitch> <yaw> <roll> <r> <g> <b> <a> <wireframe: 0, 1> <duration>\n" );
	}
}

CON_COMMAND( sc_debug_draw_clear, "Clear all draws" )
{
	THIS_FEATURE()->DrawClear();
}

//-----------------------------------------------------------------------------
// Draw contexts
//-----------------------------------------------------------------------------

// Draw box, no depth buffer
CDrawBoxNoDepthBuffer::CDrawBoxNoDepthBuffer( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color ) : m_color( color )
{
	if ( vOrigin.x == 0.f && vOrigin.y == 0.f && vOrigin.z == 0.f )
	{
		m_vecDrawOrigin = vMins + ( vMaxs - vMins ) * 0.5f;
	}
	else
	{
		m_vecDrawOrigin = vOrigin;
	}

	m_vecOrigin = vOrigin;
	m_vecMins = vMins;
	m_vecMaxs = vMaxs;
}

void CDrawBoxNoDepthBuffer::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	Vector vecPoints[ 8 ];

	Vector vecMins = m_vecMins;
	Vector vecMaxs = m_vecMaxs;

	VectorAdd( vecMins, m_vecOrigin, vecMins );
	VectorAdd( vecMaxs, m_vecOrigin, vecMaxs );

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
		glEnd();
	}

	// Bottom
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 1 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 3 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
	glEnd();

	// Top
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 5 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 7 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glEnable( GL_DEPTH_TEST );
}

// Draw wireframe box
CWireframeBox::CWireframeBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color, float width, bool bIgnoreDepthBuffer ) : m_color( color )
{
	if ( vOrigin.x == 0.f && vOrigin.y == 0.f && vOrigin.z == 0.f )
	{
		m_vecDrawOrigin = vMins + ( vMaxs - vMins ) * 0.5f;
	}
	else
	{
		m_vecDrawOrigin = vOrigin;
	}

	m_vecOrigin = vOrigin;
	m_vecMins = vMins;
	m_vecMaxs = vMaxs;

	m_flWidth = width;
	m_bIgnoreDepthBuffer = bIgnoreDepthBuffer;
}

void CWireframeBox::Draw()
{
	glEnable( GL_BLEND );

	if ( m_bIgnoreDepthBuffer )
		glDisable( GL_DEPTH_TEST );

	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	Vector vecPoints[ 8 ];

	Vector vecMins = m_vecMins;
	Vector vecMaxs = m_vecMaxs;

	VectorAdd( vecMins, m_vecOrigin, vecMins );
	VectorAdd( vecMaxs, m_vecOrigin, vecMaxs );

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );
	glLineWidth( m_flWidth );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_LINES );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );

			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );

			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );

			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
		glEnd();
	}

	// Bottom & Top
	glBegin( GL_LINES );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );

		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	/*
	// Turn on wireframe mode
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
		glEnd();
	}

	// Bottom
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 1 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 3 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
	glEnd();

	// Top
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 5 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 7 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	// Turn off wireframe mode
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	*/

	glLineWidth( 1.f );

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );

	if ( m_bIgnoreDepthBuffer )
		glEnable( GL_DEPTH_TEST );
}

// Draw rotated wireframe box
CWireframeBoxAngles::CWireframeBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color, float width, bool bIgnoreDepthBuffer ) : m_color( color )
{
	if ( vOrigin.x == 0.f && vOrigin.y == 0.f && vOrigin.z == 0.f )
	{
		m_vecDrawOrigin = vMins + ( vMaxs - vMins ) * 0.5f;
	}
	else
	{
		m_vecDrawOrigin = vOrigin;
	}

	m_vecOrigin = vOrigin;
	m_vecAngles = vAngles;
	m_vecMins = vMins;
	m_vecMaxs = vMaxs;

	m_flWidth = width;
	m_bIgnoreDepthBuffer = bIgnoreDepthBuffer;
}

void CWireframeBoxAngles::Draw()
{
	glEnable( GL_BLEND );

	if ( m_bIgnoreDepthBuffer )
		glDisable( GL_DEPTH_TEST );

	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	Vector vecPoints[ 8 ];

	Vector vecMins = m_vecMins;
	Vector vecMaxs = m_vecMaxs;

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	// Transform
	Vector temp;
	float localSpaceToWorld[ 3 ][ 4 ];

	AngleMatrix( m_vecAngles, localSpaceToWorld );

	localSpaceToWorld[ 0 ][ 3 ] = m_vecOrigin[ 0 ];
	localSpaceToWorld[ 1 ][ 3 ] = m_vecOrigin[ 1 ];
	localSpaceToWorld[ 2 ][ 3 ] = m_vecOrigin[ 2 ];

	for ( int i = 0; i < 8; i++ )
	{
		VectorTransform( vecPoints[ i ], localSpaceToWorld, temp );

		vecPoints[ i ] = temp;
	}

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );
	glLineWidth( m_flWidth );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_LINES );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );

			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );

			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );

			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
		glEnd();
	}

	// Bottom & Top
	glBegin( GL_LINES );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );

		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	glLineWidth( 1.f );

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );

	if ( m_bIgnoreDepthBuffer )
		glEnable( GL_DEPTH_TEST );
}

// Draw linear trajectory
CDrawTrajectory::CDrawTrajectory( const Color &lineColor, const Color &impactColor, float flWidth )
{
	m_lineColor = lineColor;
	m_impactColor = impactColor;
	m_flWidth = flWidth;
}

CDrawTrajectory::~CDrawTrajectory()
{
	m_trajectoryLines.clear();
	m_impacts.clear();
}

void CDrawTrajectory::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	if ( !m_trajectoryLines.empty() )
	{
		glColor4ub( m_lineColor.r, m_lineColor.g, m_lineColor.b, m_lineColor.a );
		glLineWidth( m_flWidth );

		glBegin( GL_LINES );

		for ( const Vector &point : m_trajectoryLines )
		{
			glVertex3f( VectorExpand( point ) );
		}

		glEnd();

		glLineWidth( 1.f );
	}

	if ( !m_impacts.empty() )
	{
		Vector vecPoints[ 8 ];

		glColor4ub( m_impactColor.r, m_impactColor.g, m_impactColor.b, m_impactColor.a );

		for ( const Vector &impact : m_impacts )
		{
			Vector vecMins( -2, -2, -2 );
			Vector vecMaxs( 2, 2, 2 );

			VectorAdd( vecMins, impact, vecMins );
			VectorAdd( vecMaxs, impact, vecMaxs );

			// Build points of box
			vecPoints[ 0 ].x = vecMins.x;
			vecPoints[ 0 ].y = vecMins.y;
			vecPoints[ 0 ].z = vecMins.z;

			vecPoints[ 1 ].x = vecMins.x;
			vecPoints[ 1 ].y = vecMaxs.y;
			vecPoints[ 1 ].z = vecMins.z;

			vecPoints[ 2 ].x = vecMaxs.x;
			vecPoints[ 2 ].y = vecMaxs.y;
			vecPoints[ 2 ].z = vecMins.z;

			vecPoints[ 3 ].x = vecMaxs.x;
			vecPoints[ 3 ].y = vecMins.y;
			vecPoints[ 3 ].z = vecMins.z;

			vecPoints[ 4 ].x = vecMins.x;
			vecPoints[ 4 ].y = vecMins.y;
			vecPoints[ 4 ].z = vecMaxs.z;

			vecPoints[ 5 ].x = vecMins.x;
			vecPoints[ 5 ].y = vecMaxs.y;
			vecPoints[ 5 ].z = vecMaxs.z;

			vecPoints[ 6 ].x = vecMaxs.x;
			vecPoints[ 6 ].y = vecMaxs.y;
			vecPoints[ 6 ].z = vecMaxs.z;

			vecPoints[ 7 ].x = vecMaxs.x;
			vecPoints[ 7 ].y = vecMins.y;
			vecPoints[ 7 ].z = vecMaxs.z;

			for ( int i = 0; i < 4; i++ )
			{
				int j = ( i + 1 ) % 4;

				glBegin( GL_TRIANGLE_STRIP );
					glVertex3f( VectorExpand( vecPoints[ i ] ) );
					glVertex3f( VectorExpand( vecPoints[ j ] ) );
					glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
					glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
				glEnd();
			}

			// Bottom
			glBegin( GL_TRIANGLE_STRIP );
				glVertex3f( VectorExpand( vecPoints[ 2 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 1 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 3 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
			glEnd();

			// Top
			glBegin( GL_TRIANGLE_STRIP );
				glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 5 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 7 ] ) );
				glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
			glEnd();
		}
	}

	glEnable( GL_TEXTURE_2D );

	glEnable( GL_DEPTH_TEST );
	glDisable( GL_BLEND );
}

bool CDrawTrajectory::ShouldStopDraw()
{
	return false;
}

const Vector &CDrawTrajectory::GetDrawOrigin() const
{
	return g_vecZero;
}

void CDrawTrajectory::AddLine( const Vector &start, const Vector &end )
{
	m_trajectoryLines.push_back( start );
	m_trajectoryLines.push_back( end );
}

void CDrawTrajectory::AddImpact( const Vector &impact )
{
	m_impacts.push_back( impact );
}

// Utilities
void DrawBox( const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs, float r, float g, float b, float alpha, float width, bool wireframe, float duration /* = 0.f */ )
{
	if ( wireframe )
	{
		CWireframeBox *pWireframeBox = new CWireframeBox( vecOrigin, vecMins, vecMaxs, Color( r, g, b, alpha ), width, false );

		THIS_FEATURE()->AddDrawContext( pWireframeBox, duration );
	}
	else
	{
		THIS_FEATURE()->DrawBox( vecOrigin,
								 vecMins,
								 vecMaxs,
								 r,
								 g,
								 b,
								 alpha,
								 duration );
	}
}

void DrawBoxAngles( const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs, const Vector &vecAngles, float r, float g, float b, float alpha, float width, bool wireframe, float duration /* = 0.f */ )
{
	if ( wireframe )
	{
		CWireframeBoxAngles *pWireframeBoxAngles = new CWireframeBoxAngles( vecOrigin, vecMins, vecMaxs, vecAngles, Color( r, g, b, alpha ), width, false );

		THIS_FEATURE()->AddDrawContext( pWireframeBoxAngles, duration );
	}
	else
	{
		THIS_FEATURE()->DrawBoxAngles( vecOrigin, vecMins, vecMaxs, vecAngles, r, g, b, alpha, duration );
	}
}

// Draw point
class CDrawPoint : public IDrawContext
{
public:
	CDrawPoint( const Vector &vPoint, const Color &color, float size );
	virtual ~CDrawPoint() {}

	virtual void Draw() override;
	virtual bool ShouldStopDraw() override;

	virtual const Vector &GetDrawOrigin() const override;

private:
	Vector m_vecOrigin;

	Color m_color;
	float m_flSize;
};

CDrawPoint::CDrawPoint( const Vector &vPoint, const Color &color, float size ) : m_color( color )
{
	m_vecOrigin = vPoint;
	m_flSize = size;
}

void CDrawPoint::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );
	glPointSize( m_flSize );

	glBegin( GL_POINTS );
		glVertex3f( VectorExpand( m_vecOrigin ) );
	glEnd();

	glPointSize( 1.f );

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
}

bool CDrawPoint::ShouldStopDraw()
{
	return false;
}

const Vector &CDrawPoint::GetDrawOrigin() const
{
	return m_vecOrigin;
}

// Draw line
class CDrawLine : public IDrawContext
{
public:
	CDrawLine( const Vector &vStart, const Vector &vEnd, const Color &color, float width );
	virtual ~CDrawLine() {}

	virtual void Draw() override;
	virtual bool ShouldStopDraw() override;

	virtual const Vector &GetDrawOrigin() const override;

private:
	Vector m_vecOrigin;

	Vector m_vecStart;
	Vector m_vecEnd;

	Color m_color;
	float m_flWidth;
};

CDrawLine::CDrawLine( const Vector &vStart, const Vector &vEnd, const Color &color, float width ) : m_color( color )
{
	m_vecOrigin = vStart + ( vEnd - vStart ) * 0.5f;
	m_vecStart = vStart;
	m_vecEnd = vEnd;
	m_flWidth = width;
}

void CDrawLine::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );
	glLineWidth( m_flWidth );

	glBegin( GL_LINES );
		glVertex3f( VectorExpand( m_vecStart ) );
		glVertex3f( VectorExpand( m_vecEnd ) );
	glEnd();

	glLineWidth( 1.f );

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
}

bool CDrawLine::ShouldStopDraw()
{
	return false;
}

const Vector &CDrawLine::GetDrawOrigin() const
{
	return m_vecOrigin;
}

// Draw box
class CDrawBox : public IDrawContext
{
public:
	CDrawBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color );
	virtual ~CDrawBox() {}

	virtual void Draw() override;
	virtual bool ShouldStopDraw() override;

	virtual const Vector &GetDrawOrigin() const override;

private:
	Vector m_vecDrawOrigin;
	Vector m_vecOrigin;

	Vector m_vecMins;
	Vector m_vecMaxs;

	Color m_color;
};

CDrawBox::CDrawBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color ) : m_color( color )
{
	if ( vOrigin.x == 0.f && vOrigin.y == 0.f && vOrigin.z == 0.f )
	{
		m_vecDrawOrigin = vMins + ( vMaxs - vMins ) * 0.5f;
	}
	else
	{
		m_vecDrawOrigin = vOrigin;
	}

	m_vecOrigin = vOrigin;
	m_vecMins = vMins;
	m_vecMaxs = vMaxs;
}

void CDrawBox::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	Vector vecPoints[ 8 ];

	Vector vecMins = m_vecMins;
	Vector vecMaxs = m_vecMaxs;

	VectorAdd( vecMins, m_vecOrigin, vecMins );
	VectorAdd( vecMaxs, m_vecOrigin, vecMaxs );

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
		glEnd();
	}

	// Bottom
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 1 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 3 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
	glEnd();

	// Top
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 5 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 7 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
}

bool CDrawBox::ShouldStopDraw()
{
	return false;
}

const Vector &CDrawBox::GetDrawOrigin() const
{
	return m_vecDrawOrigin;
}

// Draw box angles
class CDrawBoxAngles : public IDrawContext
{
public:
	CDrawBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color );
	virtual ~CDrawBoxAngles() {}

	virtual void Draw() override;
	virtual bool ShouldStopDraw() override;

	virtual const Vector &GetDrawOrigin() const override;

private:
	Vector m_vecDrawOrigin;

	Vector m_vecOrigin;
	Vector m_vecAngles;

	Vector m_vecMins;
	Vector m_vecMaxs;

	Color m_color;
};

CDrawBoxAngles::CDrawBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color ) : m_color( color )
{
	if ( vOrigin.x == 0.f && vOrigin.y == 0.f && vOrigin.z == 0.f )
	{
		m_vecDrawOrigin = vMins + ( vMaxs - vMins ) * 0.5f;
	}
	else
	{
		m_vecDrawOrigin = vOrigin;
	}

	m_vecOrigin = vOrigin;
	m_vecAngles = vAngles;
	m_vecMins = vMins;
	m_vecMaxs = vMaxs;
}

void CDrawBoxAngles::Draw()
{
	glEnable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	Vector vecPoints[ 8 ];

	Vector vecMins = m_vecMins;
	Vector vecMaxs = m_vecMaxs;

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	Vector temp;
	float localSpaceToWorld[ 3 ][ 4 ];

	AngleMatrix( m_vecAngles, localSpaceToWorld );

	localSpaceToWorld[ 0 ][ 3 ] = m_vecOrigin[ 0 ];
	localSpaceToWorld[ 1 ][ 3 ] = m_vecOrigin[ 1 ];
	localSpaceToWorld[ 2 ][ 3 ] = m_vecOrigin[ 2 ];

	for ( int i = 0; i < 8; i++ )
	{
		VectorTransform( vecPoints[ i ], localSpaceToWorld, temp );

		vecPoints[ i ] = temp;
	}

	glColor4ub( m_color.r, m_color.g, m_color.b, m_color.a );

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		glBegin( GL_TRIANGLE_STRIP );
			glVertex3f( VectorExpand( vecPoints[ i ] ) );
			glVertex3f( VectorExpand( vecPoints[ j ] ) );
			glVertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
			glVertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
		glEnd();
	}

	// Bottom
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 2 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 1 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 3 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 0 ] ) );
	glEnd();

	// Top
	glBegin( GL_TRIANGLE_STRIP );
		glVertex3f( VectorExpand( vecPoints[ 4 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 5 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 7 ] ) );
		glVertex3f( VectorExpand( vecPoints[ 6 ] ) );
	glEnd();

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
}

bool CDrawBoxAngles::ShouldStopDraw()
{
	return false;
}

const Vector &CDrawBoxAngles::GetDrawOrigin() const
{
	return m_vecDrawOrigin;
}

void CDrawContext::AddDrawContext( IDrawContext *pContext, float duration )
{
	if ( pContext == NULL )
		return;

	if ( duration < 0.f )
		duration = 0.f;

	draw_context_t draw_context;

	draw_context.pDrawContext = pContext;
	draw_context.flDuration = static_cast<float>( *realtime ) + duration;
	draw_context.flDistanceSqr = 0.f;

	m_vDrawContext.push_back( draw_context );
}

void CDrawContext::DrawPoint( const Vector &vPoint, const Color &color, float size, float duration )
{
	if ( duration < 0.f )
		duration = 0.f;

	draw_context_t draw_context;

	IDrawContext *pDrawContext = new CDrawPoint( vPoint, color, size );

	draw_context.pDrawContext = pDrawContext;
	draw_context.flDuration = static_cast<float>( *realtime ) + duration;
	draw_context.flDistanceSqr = 0.f;

	m_vDrawContext.push_back( draw_context );
}

void CDrawContext::DrawLine( const Vector &vStart, const Vector &vEnd, const Color &color, float width, float duration )
{
	if ( duration < 0.f )
		duration = 0.f;

	draw_context_t draw_context;

	IDrawContext *pDrawContext = new CDrawLine( vStart, vEnd, color, width );

	draw_context.pDrawContext = pDrawContext;
	draw_context.flDuration = static_cast<float>( *realtime ) + duration;
	draw_context.flDistanceSqr = 0.f;

	m_vDrawContext.push_back( draw_context );
}

void CDrawContext::DrawBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color, float duration )
{
	if ( duration < 0.f )
		duration = 0.f;

	draw_context_t draw_context;

	IDrawContext *pDrawContext = new CDrawBox( vOrigin, vMins, vMaxs, color );

	draw_context.pDrawContext = pDrawContext;
	draw_context.flDuration = static_cast<float>( *realtime ) + duration;
	draw_context.flDistanceSqr = 0.f;

	m_vDrawContext.push_back( draw_context );
}

void CDrawContext::DrawBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color, float duration )
{
	if ( duration < 0.f )
		duration = 0.f;

	draw_context_t draw_context;

	IDrawContext *pDrawContext = new CDrawBoxAngles( vOrigin, vMins, vMaxs, vAngles, color );

	draw_context.pDrawContext = pDrawContext;
	draw_context.flDuration = static_cast<float>( *realtime ) + duration;
	draw_context.flDistanceSqr = 0.f;

	m_vDrawContext.push_back( draw_context );
}

void CDrawContext::DrawClear( void )
{
	for ( size_t i = 0; i < m_vDrawContext.size(); i++ )
	{
		delete m_vDrawContext[ i ].pDrawContext;
	}

	m_vDrawContext.clear();
}

void CDrawContext::SetRenderOrigin( const Vector &vOrigin )
{
	m_vecRenderOrigin = vOrigin;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDrawContext::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_DrawTransparentTriangles_HookEvent )
	{
		if ( m_vDrawContext.empty() )
			return kHookContinue;

		// TODO: optimize
		for ( size_t i = 0; i < m_vDrawContext.size(); i++ )
		{
			m_vDrawContext[ i ].flDistanceSqr = ( m_vDrawContext[ i ].pDrawContext->GetDrawOrigin() - m_vecRenderOrigin ).LengthSqr();
		}

		std::sort( m_vDrawContext.begin(), m_vDrawContext.end() );

		for ( size_t i = 0; i < m_vDrawContext.size(); i++ )
		{
			draw_context_t &draw_context = m_vDrawContext[ i ];

			if ( draw_context.flDuration < static_cast<float>( *realtime ) ||
				 draw_context.pDrawContext->ShouldStopDraw() )
			{
				delete draw_context.pDrawContext;

				m_vDrawContext.erase( m_vDrawContext.begin() + i );
				i--;

				continue;
			}

			draw_context.pDrawContext->Draw();
		}
	}
	else if ( pEvent->GetType() == kV_CalcRefdef_HookEvent )
	{
		m_vecRenderOrigin = refparams->vieworg;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDrawContext::CDrawContext( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_vecRenderOrigin = vec3_origin;
}

//-----------------------------------------------------------------------------
// Load feature
//-----------------------------------------------------------------------------

bool CDrawContext::Load( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CDrawContext::PostLoad( void )
{
	hookevents->RegisterListener( this, kHUD_DrawTransparentTriangles_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );

	FEATURE_REGISTER_CCMD( sc_debug_draw_point );
	FEATURE_REGISTER_CCMD( sc_debug_draw_line );
	FEATURE_REGISTER_CCMD( sc_debug_draw_box );
	FEATURE_REGISTER_CCMD( sc_debug_draw_box_angles );
	FEATURE_REGISTER_CCMD( sc_debug_draw_clear );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CDrawContext::Unload( void )
{
	hookevents->UnregisterListener( this, kHUD_DrawTransparentTriangles_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kV_CalcRefdef_HookEvent, kHookPostCall );

	FEATURE_UNREGISTER_CCMD( sc_debug_draw_point );
	FEATURE_UNREGISTER_CCMD( sc_debug_draw_line );
	FEATURE_UNREGISTER_CCMD( sc_debug_draw_box );
	FEATURE_UNREGISTER_CCMD( sc_debug_draw_box_angles );
	FEATURE_UNREGISTER_CCMD( sc_debug_draw_clear );
}
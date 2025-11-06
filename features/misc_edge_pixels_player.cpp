// SvenInt (c) Sw1ft
// misc_edge_pixels_player.cpp

#include "stdafx.h"
#include "misc_edge_pixels_player.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CEdgePixelsPlayer, edgepixelsplayer, "Misc", "Edge-Pixels Player" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_epp_play_in_demo( "sc_epp_play_in_demo", "1", FCVAR_EXTDLL );

CON_COMMAND( sc_epp_start, "Start visualization from a file" )
{
	if ( args.ArgC() > 10 )
	{
		bool bDemoPlayback = false;

		const char *filename = args[ 1 ];
		double width = atof( args[ 2 ] );
		double height = atof( args[ 3 ] );
		Vector vecPos( atof( args[ 4 ] ), atof( args[ 5 ] ), atof( args[ 6 ] ) );
		Vector vecAngles( atof( args[ 7 ] ), atof( args[ 8 ] ), atof( args[ 9 ] ) );
		int drawcalls = atoi( args[ 10 ] );

		if ( args.ArgC() > 11 )
			bDemoPlayback = !!atoi( args[ 11 ] );

		THIS_FEATURE()->Start( filename, width, height, vecPos, vecAngles, drawcalls, bDemoPlayback );
	}
}

CON_COMMAND( sc_epp_stop, "Stop visualization" )
{
	THIS_FEATURE()->Stop();
}

//-----------------------------------------------------------------------------
// Start playing
//-----------------------------------------------------------------------------

bool CEdgePixelsPlayer::Start( const char *pszFilename, double width, double height, const Vector &vecPos, const Vector &vecAngles, int iDrawCalls, bool bDemoPlayback /* = false */ )
{
	if ( pszFilename == NULL )
		return false;

	int tmp;

	m_width = height; // yeah
	m_height = width;
	m_vecPos = vecPos;
	m_vecAngles = vecAngles;
	m_iDrawCalls = Q_max( 1, iDrawCalls );

	m_pFile = fopen( pszFilename, "rb" );

	if ( m_pFile == NULL )
		return false;

	fseek( m_pFile, 0, SEEK_END );
	m_ulFileSize = ftell( m_pFile );
	fseek( m_pFile, 0, SEEK_SET );

	m_sFilename = pszFilename;

	fread( &tmp, sizeof( int ), 1, m_pFile ); // width
	fread( &tmp, sizeof( int ), 1, m_pFile ); // height

	fread( &tmp, sizeof( int ), 1, m_pFile );
	m_frametime = 1.0 / static_cast<double>( tmp );
	m_lastPlayed = 0.0;

	m_drawAngles.clear();
	m_drawAnglesQueue.clear();

	m_bPlaying = true;
	m_bDemoPlayback = bDemoPlayback;

#if 0
	Msg( "pszFilename: %s\n", pszFilename );
	Msg( "width: %f\n", width );
	Msg( "height: %f\n", height );
	Msg( "vecPos: %.3f %.3f %.3f\n", VectorExpand( vecPos ) );
	Msg( "vecAngles: %.3f %.3f %.3f\n", VectorExpand( vecAngles ) );
	Msg( "iDrawCalls: %d\n", iDrawCalls );
	Msg( "frametime: %f\n", m_frametime );
#endif

	//g_DemoMessage.WriteEdgePixelsPlayer( pszFilename, width, height, vecPos, vecAngles, iDrawCalls );
	return true;
}

//-----------------------------------------------------------------------------
// Stop playing
//-----------------------------------------------------------------------------

bool CEdgePixelsPlayer::Stop( void )
{
	if ( !m_bPlaying )
		return false;

	m_bPlaying = false;

	fclose( m_pFile );
	m_pFile = NULL;

	m_drawAngles.clear();
	m_drawAnglesQueue.clear();

	//g_DemoMessage.WriteEdgePixelsPlayerStop();
	return true;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CEdgePixelsPlayer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( !m_bPlaying || m_pFile == NULL )
		return kHookContinue;

	if ( feof( m_pFile ) || (unsigned long)ftell( m_pFile ) >= m_ulFileSize )
	{
		Stop();
		return kHookContinue;
	}

	Vector localPoint, worldPoint, vecDir, vecAngles, vecEyes;

	if ( !m_bDemoPlayback && !demoplayback )
	{
		vecEyes = localplayer->GetEyePosition();
	}
	else
	{
		vecEyes = *(Vector *)( refparams->simorg ) + *(Vector *)( refparams->viewheight );
	}

#if 0
	if ( m_iDrawCalls == 1 )
	{
		if ( !m_drawAngles.empty() )
		{
			for ( Vector &ang : m_drawAngles )
			{
				cl_enginefuncs->SetViewAngles( ang );
				npc_moveto->function();
			}

			m_drawAngles.erase( m_drawAngles.begin() );
		}
	}
	else if ( !m_drawAnglesQueue.empty() )
#else
	if ( !m_drawAnglesQueue.empty() )
#endif
	{
		std::vector<Vector> &drawAngles = m_drawAnglesQueue[ 0 ];

		for ( Vector &ang : drawAngles )
		{
			cl_enginefuncs->SetViewAngles( ang );
			npc_moveto->function();
		}

		m_drawAnglesQueue.erase( m_drawAnglesQueue.begin() );
	}

	if ( *realtime - m_lastPlayed >= m_frametime )
	{
		m_lastPlayed = *realtime;

		m_drawAngles.clear();
		m_drawAnglesQueue.clear();

		int pixels;
		fread( &pixels, sizeof( int ), 1, m_pFile );

		if ( pixels > 0 )
		{
			double ndc[ 2 ];
			float localSpaceToWorld[ 3 ][ 4 ];

			AngleMatrix( m_vecAngles, localSpaceToWorld );

			localSpaceToWorld[ 0 ][ 3 ] = m_vecPos.x;
			localSpaceToWorld[ 1 ][ 3 ] = m_vecPos.y;
			localSpaceToWorld[ 2 ][ 3 ] = m_vecPos.z;

			for ( int i = 0; i < pixels; i++ )
			{
				worldPoint.Zero();
				localPoint.Zero();

				fread( &ndc, sizeof( ndc ), 1, m_pFile );

				localPoint.x = static_cast<float>( ( 2.0 * ndc[ 0 ] - 1.0 ) * m_width );
				localPoint.y = static_cast<float>( ( 2.0 * ndc[ 1 ] - 1.0 ) * m_height );

				VectorTransform( localPoint, localSpaceToWorld, worldPoint );

				vecDir = worldPoint - vecEyes;
				VectorAngles( vecDir, vecAngles );
				vecAngles.x *= -1.f;
				vecAngles.z = 0.f;

				NormalizeAngles( vecAngles );

				m_drawAngles.push_back( vecAngles );
			}

			std::random_shuffle( m_drawAngles.begin(), m_drawAngles.end() );

			if ( m_iDrawCalls == 1 )
			{
				for ( int i = 0; i < pixels; i++ )
				{
					cl_enginefuncs->SetViewAngles( m_drawAngles[ i ] );
					npc_moveto->function();
				}
			}
			else
			{
				int queued_draw_calls_cur = 0;
				int queued_draw_calls_pixels_delta = pixels / m_iDrawCalls;

				m_drawAnglesQueue.push_back( std::vector<Vector>() );

				for ( int i = 0; i < pixels; i++ )
				{
					if ( queued_draw_calls_cur * queued_draw_calls_pixels_delta <= i )
					{
						m_drawAnglesQueue.push_back( std::vector<Vector>() );
						queued_draw_calls_cur++;
					}

					m_drawAnglesQueue.back().push_back( m_drawAngles[ i ] );
				}

				std::random_shuffle( m_drawAnglesQueue.begin(), m_drawAnglesQueue.end() );

				std::vector<Vector> &drawAngles = m_drawAnglesQueue[ 0 ];

				for ( Vector &ang : drawAngles )
				{
					cl_enginefuncs->SetViewAngles( ang );
					npc_moveto->function();
				}

				m_drawAnglesQueue.erase( m_drawAnglesQueue.begin() );
			}

			m_drawAngles.clear();
		}
	}

	vecDir = m_vecPos - vecEyes;
	VectorAngles( vecDir, vecAngles );
	vecAngles.x *= -1.f;
	vecAngles.z = 0.f;

	NormalizeAngles( vecAngles );
	cl_enginefuncs->SetViewAngles( vecAngles );
	
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CEdgePixelsPlayer::CEdgePixelsPlayer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bPlaying = false;
	m_bDemoPlayback = false;
	m_pFile = NULL;
	m_ulFileSize = 0;
	m_width = 0.0;
	m_height = 0.0;
	m_frametime = 0.0;
	m_lastPlayed = 0.0;
	m_iDrawCalls = 1;
	npc_moveto = NULL;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CEdgePixelsPlayer::Load( void )
{
	npc_moveto = cvar->FindCmd( "npc_moveto" );
	FEATURE_CHECK_SYMBOL( npc_moveto, "npc_moveto" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CEdgePixelsPlayer::PostLoad( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall, kHookPriorityLow );

	FEATURE_REGISTER_CCMD( sc_epp_start );
	FEATURE_REGISTER_CCMD( sc_epp_stop );
	FEATURE_REGISTER_CVAR( sc_epp_play_in_demo );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CEdgePixelsPlayer::Unload( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );

	FEATURE_UNREGISTER_CCMD( sc_epp_start );
	FEATURE_UNREGISTER_CCMD( sc_epp_stop );
	FEATURE_UNREGISTER_CVAR( sc_epp_play_in_demo );

	Stop();
}
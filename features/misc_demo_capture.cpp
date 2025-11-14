// SvenInt (c) Sw1ft
// misc_demo_capture.cpp

#include "stdafx.h"
#include "misc_demo_capture.h"
#include "modules/menu.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDemoCapture, democapture, "Misc", "Demo Capture" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_cap_fps( "sc_cap_fps", "60", FCVAR_EXTDLL );
ConVar sc_cap_slowdown( "sc_cap_slowdown", "1", FCVAR_EXTDLL );
ConVar sc_cap_sampling_min_fps( "sc_cap_sampling_min_fps", "7200", FCVAR_EXTDLL );
ConVar sc_cap_sampling_round_fps( "sc_cap_sampling_round_fps", "1", FCVAR_EXTDLL );

CON_COMMAND( sc_cap_start, "Start capture" )
{
	if ( args.ArgC() == 1 )
	{
		Msg( "Usage:  sc_cap_start <filename>\n" );
		return;
	}

	THIS_FEATURE()->Start( args[ 1 ],
						   static_cast<double>( sc_cap_fps.GetFloat() ),
						   static_cast<double>( sc_cap_slowdown.GetFloat() ),
						   static_cast<double>( sc_cap_sampling_min_fps.GetFloat() ) );
}

CON_COMMAND( sc_cap_stop, "Stop capture" )
{
	THIS_FEATURE()->Stop();
}

//-----------------------------------------------------------------------------
// Start capturing
//-----------------------------------------------------------------------------

bool CDemoCapture::Start( const char *pszFilename, double fps, double slowdown, double sampling_fps )
{
	if ( IsRecording() )
	{
		PrintWarning( "Already recording\n" );
		return false;
	}

	double fpsMultiplier;
	double sampleFrametime;

	m_captureFps = fps;
	m_samplingFps = sampling_fps;
	m_iFpsMultiplier = int( fpsMultiplier = ceil( sampling_fps / fps ) );
	m_fps = sc_cap_sampling_round_fps.GetBool() ? m_captureFps * fpsMultiplier : m_samplingFps;
	m_frametime = ( 1.0 / m_captureFps ) * ( 1.0 / slowdown ); // m_fps

	if ( m_fps < m_frametime )
		m_fps = m_frametime;

	m_iWidth = gameutils->GetScreenWidth();
	m_iHeight = gameutils->GetScreenHeight();
	m_nPixelsBufferSize = m_iWidth * m_iHeight * 3;
	m_pPixelsBuffer = (char *)malloc( m_nPixelsBufferSize );

	sampleFrametime = ( 1.0 / m_fps );

	if ( m_pPixelsBuffer == NULL )
	{
		PrintWarning( "Couldn't allocate memory for pixels buffer\n" );
		return false;
	}

	m_sFilename = pszFilename;

	if ( !OpenPipe() )
	{
		free( (void *)m_pPixelsBuffer );
		return false;
	}

	m_bRecording = true;
	m_bFirstCapture = true;
	m_iCaptureFrameCount = 0;
	m_lastRecordTime = 0.0;

	cvar->SetValue( "fps_max", (float)m_captureFps );
	cvar->SetValue( "host_framerate", (float)sampleFrametime ); // m_frametime

	PrintMsg( "Started recording to file \"%s.mp4\"\n", pszFilename );
	return true;
}

//-----------------------------------------------------------------------------
// Stop capturing
//-----------------------------------------------------------------------------

bool CDemoCapture::Stop( void )
{
	if ( !IsRecording() )
	{
		PrintWarning( "Not recording\n" );
		return false;
	}

	ClosePipe();

	free( (void *)m_pPixelsBuffer );

	cvar->SetValue( "fps_max", 200 );
	cvar->SetValue( "host_framerate", 0 );

	PrintMsg( "Stopped recording to file \"%s\"\n", m_sFilename.c_str() );

	m_bRecording = false;
	return true;
}

//-----------------------------------------------------------------------------
// Save pixels and send them to FFmpeg
//-----------------------------------------------------------------------------

void CDemoCapture::SaveImage( void )
{
	DWORD dwBytesWritten;

	// TODO: use shaders
	glReadPixels( 0, 0, m_iWidth, m_iHeight, GL_RGB, GL_UNSIGNED_BYTE, m_pPixelsBuffer );
	WriteFile( m_hWritePipe, m_pPixelsBuffer, m_nPixelsBufferSize, &dwBytesWritten, NULL );
}

//-----------------------------------------------------------------------------
// Open FFmpeg pipe
//-----------------------------------------------------------------------------

bool CDemoCapture::OpenPipe( void )
{
	static char ffmpeg_args[ 2048 ];

	SECURITY_ATTRIBUTES sa = { sizeof( sa ) };
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	if ( !CreatePipe( &m_hReadPipe, &m_hWritePipe, &sa, m_nPixelsBufferSize ) )
	{
		PrintWarning( "Failed to create FFmpeg pipe\n" );
		return false;
	}

	SetHandleInformation( m_hWritePipe, HANDLE_FLAG_INHERIT, 0 );

	STARTUPINFOA si = { sizeof( si ) };
	si.dwFlags = STARTF_USESTDHANDLES;
	si.hStdInput = m_hReadPipe;
	si.hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );
	si.hStdError = GetStdHandle( STD_ERROR_HANDLE );

	ZeroMemory( &m_pi, sizeof( PROCESS_INFORMATION ) );

	snprintf( ffmpeg_args, Q_ARRAYSIZE( ffmpeg_args ),
			  "ffmpeg -f rawvideo \
-pix_fmt rgb24 \
-s:v %dx%d \
-r %d \
-i pipe:0 \
-preset ultrafast \
-y -movflags +faststart \
-b:v 128k \
-c:v libx264 \
-crf 15 \
-vf vflip \
-pix_fmt yuv420p \
%s.mp4", m_iWidth, m_iHeight, int( m_captureFps ), m_sFilename.c_str() );

	/*
"ffmpeg -f rawvideo \
-pix_fmt rgb24 \
-video_size %dx%d \
-r %d \
-i pipe:0 \
-preset ultrafast \
-y -movflags +faststart \
-b:v 128k \
-c:v libx264 \
-crf 15 \
-vf vflip \
-color_primaries bt709 \
-color_trc bt709 \
-colorspace bt709 \
-color_range tv \
-chroma_sample_location center \
	*/

	if ( !CreateProcess( NULL, ffmpeg_args, NULL, NULL, TRUE, 0, NULL, NULL, &si, &m_pi ) )
	{
		CloseHandle( m_hReadPipe );
		CloseHandle( m_hWritePipe );

		PrintWarning( "Failed to spawn FFmpeg\n" );
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Close FFmpeg pipe
//-----------------------------------------------------------------------------

bool CDemoCapture::ClosePipe( void )
{
	CloseHandle( m_hReadPipe );
	CloseHandle( m_hWritePipe );

	CloseHandle( m_pi.hProcess );
	CloseHandle( m_pi.hThread );

	return true;
}

//-----------------------------------------------------------------------------
// Is recording
//-----------------------------------------------------------------------------

bool CDemoCapture::IsRecording( void ) const
{
	return m_bRecording;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDemoCapture::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// SCR_UpdateScreen post event
	if ( pEvent->GetType() == kSCR_UpdateScreen_HookEvent )
	{
		if ( !IsRecording() || !demoplayback )
			return kHookContinue;

		if ( m_bFirstCapture )
		{
			m_bFirstCapture = false;
			m_lastRecordTime = *realtime;

			SaveImage();
		}
		else if ( *realtime - m_lastRecordTime >= m_frametime )
		{
			m_lastRecordTime = *realtime;
			SaveImage();
		}
	}
	else if ( pEvent->GetType() == kHost_FilterTime_HookEvent )
	{
		if ( !demoplayback && IsRecording() && !m_bFirstCapture )
		{
			Stop();
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDemoCapture::CDemoCapture( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bRecording = false;
	m_bFirstCapture = true;
	m_iCaptureFrameCount = 0;
	m_iFpsMultiplier = 120;
	m_lastRecordTime = 0.0;
	m_captureFps = 60.0;
	m_samplingFps = 7200.0;
	m_fps = 7200.0;
	m_frametime = 1.0 / 7200.0;

	m_iWidth = 1280;
	m_iHeight = 720;
	m_nPixelsBufferSize = 0;
	m_pPixelsBuffer = NULL;

	m_hReadPipe = INVALID_HANDLE_VALUE;
	m_hWritePipe = INVALID_HANDLE_VALUE;
	ZeroMemory( &m_pi, sizeof( PROCESS_INFORMATION ) );
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CDemoCapture::OnEnable( void )
{
	hookevents->RegisterListener( this, kSCR_UpdateScreen_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CDemoCapture::OnDisable( void )
{
	hookevents->UnregisterListener( this, kSCR_UpdateScreen_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CDemoCapture::PostLoad( void )
{
	FEATURE_REGISTER_CVAR( sc_cap_fps );
	FEATURE_REGISTER_CVAR( sc_cap_slowdown );
	FEATURE_REGISTER_CVAR( sc_cap_sampling_min_fps );
	FEATURE_REGISTER_CVAR( sc_cap_sampling_round_fps );
	FEATURE_REGISTER_CCMD( sc_cap_start );
	FEATURE_REGISTER_CCMD( sc_cap_stop );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CDemoCapture::Unload( void )
{
	if ( IsRecording() )
	{
		Stop();
	}

	FEATURE_UNREGISTER_CVAR( sc_cap_fps );
	FEATURE_UNREGISTER_CVAR( sc_cap_slowdown );
	FEATURE_UNREGISTER_CVAR( sc_cap_sampling_min_fps );
	FEATURE_UNREGISTER_CVAR( sc_cap_sampling_round_fps );
	FEATURE_UNREGISTER_CCMD( sc_cap_start );
	FEATURE_UNREGISTER_CCMD( sc_cap_stop );
}
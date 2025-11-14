// SvenInt (c) Sw1ft
// st_input_manager.cpp

#include "stdafx.h"
#include "st_input_manager.h"
#include "modules/server.h"
#include "modules/scripts.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( int, __cdecl, Cbuf_AddText, const char * );
DECLARE_HOOK( int, __cdecl, ServerCmd, const char * );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CInputManager, inputmanager, "Speedrun Tools", "Input Manager" );

FILE *m_pExperimental = NULL;

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

static bool IM_CompareFunc( const im_cstring_t &a, const im_cstring_t &b )
{
	return !stricmp( a, b );
}

static unsigned int IM_HashFunc( const im_cstring_t &a )
{
	return HashStringCaseless( a );
}

static FORCEINLINE bool IM_StringEndsWith( std::string const &str, std::string const &postfix )
{
	if ( postfix.size() > str.size() )
		return false;

	return std::equal( postfix.rbegin(), postfix.rend(), str.rbegin() );
}

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_im_autorecord( "sc_im_autorecord", "0", FCVAR_EXTDLL, "Automatically record inputs at map start" );
ConVar sc_im_autoplay( "sc_im_autoplay", "0", FCVAR_EXTDLL, "Automatically play inputs at map start" );
ConVar sc_im_exact( "sc_im_exact", "0", FCVAR_EXTDLL, "Set origin & velocity" );
ConVar sc_im_goto_exact( "sc_im_goto_exact", "1", FCVAR_EXTDLL, "Set origin & velocity when forwarding / backwarding / goto" );
ConVar sc_im_experimental( "sc_im_experimental", "0", FCVAR_EXTDLL, "Experimental recording" );

static void sc_im_record( const CCommand &args );
static void command_wrapper__sc_im_record() { CCommand args( CVar()->ArgC(), CVar()->ArgV() ); sc_im_record( args ); } \
static ConCommand sc_im_record__command( "sc_im_record", command_wrapper__sc_im_record, "Record inputs to a file" ); \
static void sc_im_record( const CCommand &args )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->Record( args[ 1 ] );
	}
	else
	{
		Msg( "Usage:  sc_im_record <filename>\n" );
	}
}

CON_COMMAND( sc_im_play, "Playback inputs from a file" )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->Playback( args[ 1 ] );
	}
	else
	{
		Msg( "Usage:  sc_im_play <filename>\n" );
	}
}

CON_COMMAND( sc_im_split, "Split playing back inputs" )
{
	THIS_FEATURE()->Split();
}

CON_COMMAND( sc_im_stop, "Stop recording inputs" )
{
	THIS_FEATURE()->Stop();
}

CON_COMMAND( sc_im_goto, "Jump to X frame" )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->Goto( atoi( args[ 1 ] ) );
	}
	else
	{
		Msg( "Usage:  sc_im_goto <frame>\n" );
	}
}

CON_COMMAND( sc_im_forward, "Forward on X frames" )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->Forward( atoi( args[ 1 ] ) );
	}
	else
	{
		Msg( "Usage:  sc_im_forward <frames>\n" );
	}
}

CON_COMMAND( sc_im_backward, "Backward on X frames" )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->Backward( atoi( args[ 1 ] ) );
	}
	else
	{
		Msg( "Usage:  sc_im_backward <frames>\n" );
	}
}

CON_COMMAND( sc_im_frame, "Prints current frame" )
{
	Msg( "Frame: %d\n", THIS_FEATURE()->GetCurrentFrame() );
}

CON_COMMAND( sc_im_test, "Prints current frame" )
{
	edict_t *pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( 1 );

	Msg( "setpos %f %f %f\n", VectorExpand( pPlayer->v.origin ) );
	Msg( "setvel %f %f %f\n", VectorExpand( pPlayer->v.velocity ) );
	Msg( "setang %f %f %f\n", VectorExpand( *playermove->angles() ) );
}

static void sc_im_record_command( const CCommand &args );
static void command_wrapper__sc_im_record_command() { CCommand args( CVar()->ArgC(), CVar()->ArgV() ); sc_im_record_command( args ); } \
static ConCommand sc_im_record_command__command( "sc_im_record_command", command_wrapper__sc_im_record_command, "Record a given command" ); \
static void sc_im_record_command( const CCommand &args )
{
	if ( args.ArgC() >= 2 )
	{
		THIS_FEATURE()->RecordCommand( args[ 1 ] );
	}
	else
	{
		Msg( "Usage:  sc_im_record_command <command>\n" );
	}
}

//-----------------------------------------------------------------------------
// Hooked functions
//-----------------------------------------------------------------------------

DECLARE_FUNC( int, __cdecl, HOOKED_Cbuf_AddText, const char *pszCommand )
{
	THIS_FEATURE()->OnCbuf_AddText( pszCommand );

	return ORIG_Cbuf_AddText( pszCommand );
}

DECLARE_FUNC( int, __cdecl, HOOKED_ServerCmd, const char *pszCommand )
{
	THIS_FEATURE()->OnServerCmd( pszCommand );

	return ORIG_ServerCmd( pszCommand );
}

//-----------------------------------------------------------------------------
// Inputs context
//-----------------------------------------------------------------------------

CInputContext::CInputContext()
{
	memset( &m_FrameBuffer, 0, sizeof( m_FrameBuffer ) );
	m_iVersion = 0;
	m_iCurrentFrame = 0;
	m_bSavedInfos = false;
}

//-----------------------------------------------------------------------------
// Initialize
//-----------------------------------------------------------------------------

void CInputContext::Init( const char *pszFilename )
{
	m_sFileName = pszFilename;

	if ( !IM_StringEndsWith( m_sFileName, ".bin" ) )
		m_sFileName += ".bin";

	m_sFilePath = IM_FILE_PATH;
	m_sFilePath += m_sFileName;

	m_iVersion = IM_FILE_VERSION;
	m_iCurrentFrame = 0;

	m_bSavedInfos = false;
}

//-----------------------------------------------------------------------------
// Clear frames vector and strings
//-----------------------------------------------------------------------------

void CInputContext::Clear( void )
{
	m_iCurrentFrame = 0;

	for ( size_t i = 0; i < m_frames.size(); i++ )
	{
		if ( m_frames[ i ].commands != NULL )
			free( (void *)m_frames[ i ].commands );
	}

	m_frames.clear();

	m_sFileName.clear();
	m_sFilePath.clear();
}

//-----------------------------------------------------------------------------
// Parse inputs from a file
//-----------------------------------------------------------------------------

bool CInputContext::ParseFile( FILE *hStream, int iVersion, int &iErrorCode )
{
	size_t ulBytesRead, ulCommandBufferLength;
	bool bParseOK = true;
	int iFrameCount = 1;

	iErrorCode = 0;

	while ( bParseOK )
	{
		memset( &m_FrameBuffer.info, 0, IM_FRAME_INFO_SIZE );
		memset( &m_FrameBuffer.inputs, 0, IM_FRAME_INPUTS_SIZE );

		m_FrameBuffer.commands = NULL;

		// Infos since version 3
		if ( iVersion >= 3 )
		{
			ulBytesRead = fread( &m_FrameBuffer.info, 1, IM_FRAME_INFO_SIZE, hStream );

			if ( ulBytesRead != IM_FRAME_INFO_SIZE )
			{
				//bParseOK = false;
				//iErrorCode = 1;
				break;
			}
		}

		// Inputs since version 1
		ulBytesRead = fread( &m_FrameBuffer.inputs, 1, IM_FRAME_INPUTS_SIZE, hStream );

		// Command buffer since version 2
		if ( iVersion >= 2 )
		{
			// Failed to read inputs
			if ( ulBytesRead != IM_FRAME_INPUTS_SIZE )
			{
				// Not OK since command buffer was added in version 2
				if ( iVersion >= 3 )
				{
					bParseOK = false;
					iErrorCode = 2;
				}

				break;
			}

			ulBytesRead = fread( &ulCommandBufferLength, 1, sizeof( size_t ), hStream );

			if ( ulBytesRead != sizeof( size_t ) )
			{
				bParseOK = false;
				iErrorCode = 3;
				break;
			}

			// Have some commands to execute
			if ( ulCommandBufferLength > 0 )
			{
				char *pszCommandBuffer = (char *)malloc( ulCommandBufferLength + 1 );

				if ( pszCommandBuffer != NULL )
				{
					ulBytesRead = fread( pszCommandBuffer, 1, ulCommandBufferLength, hStream );

					if ( ulBytesRead != ulCommandBufferLength )
					{
						bParseOK = false;
						iErrorCode = 4;

						free( (void *)pszCommandBuffer );
						break;
					}

					pszCommandBuffer[ ulCommandBufferLength ] = '\0';

					m_FrameBuffer.commands = pszCommandBuffer;
				}
				else
				{
					THIS_FEATURE()->PrintWarning( "CInputContext::LoadFromFile: failed to allocate memory for command buffer\n" );
				}
			}
		}
		else if ( ulBytesRead != IM_FRAME_INPUTS_SIZE )
		{
			break;
		}

		// Everything seems to be ok
		m_frames.push_back( m_FrameBuffer );
		iFrameCount++;
	}

	if ( !bParseOK )
	{
		THIS_FEATURE()->PrintWarning( "CInputContext::LoadFromFileInternal: corrupted file \"%s\" (frame: %d, error code: %d)\n", m_sFileName.c_str(), iFrameCount, iErrorCode );
	}

	return bParseOK;
}

//-----------------------------------------------------------------------------
// Load inputs from a file
//-----------------------------------------------------------------------------

bool CInputContext::LoadFromFile( void )
{
	FILE *hStream = fopen( m_sFilePath.c_str(), "rb" );

	if ( hStream )
	{
		int header_buffer = IM_FILE_HEADER;
		int header_version = IM_FILE_VERSION;

		fread( &header_buffer, 1, sizeof( short ), hStream );
		fread( &header_version, 1, sizeof( short ), hStream );

		if ( header_buffer == IM_FILE_HEADER )
		{
			if ( header_version >= 1 && header_version <= IM_FILE_VERSION )
			{
				bool bParseOK;
				int iErrorCode;

				bParseOK = ParseFile( hStream, header_version, iErrorCode );

				fclose( hStream );
				return bParseOK;
			}
			else
			{
				THIS_FEATURE()->PrintWarning( "CInputContext::LoadFromFile: unsupported version of file \"%s\"\n", m_sFileName.c_str() );
				fclose( hStream );
			}
		}
		else
		{
			THIS_FEATURE()->PrintWarning( "CInputContext::LoadFromFile: unsupported file \"%s\"\n", m_sFileName.c_str() );
			fclose( hStream );
		}
	}
	else
	{
		THIS_FEATURE()->PrintWarning( "CInputContext::LoadFromFile: unable to open file \"%s\"\n", m_sFileName.c_str() );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Save recorded inputs to a file
//-----------------------------------------------------------------------------

bool CInputContext::SaveToFile( void )
{
	FILE *hStream = fopen( m_sFilePath.c_str(), "wb" );

	if ( hStream != NULL )
	{
		size_t ulCommandBufferLength;

		int header_buffer = IM_FILE_HEADER;
		int header_version = IM_FILE_VERSION;

		fwrite( &header_buffer, 1, sizeof( short ), hStream );
		fwrite( &header_version, 1, sizeof( short ), hStream );

		for ( size_t i = 0; i < m_frames.size(); i++ )
		{
			im_frame_t &frame = m_frames[ i ];

			fwrite( &frame.info, IM_FRAME_INFO_SIZE, 1, hStream );
			fwrite( &frame.inputs, IM_FRAME_INPUTS_SIZE, 1, hStream );

			ulCommandBufferLength = ( frame.commands != NULL ? strlen( frame.commands ) : 0 );

			fwrite( &ulCommandBufferLength, sizeof( size_t ), 1, hStream );

			if ( ulCommandBufferLength > 0 )
				fwrite( frame.commands, ulCommandBufferLength, 1, hStream );
		}

		fclose( hStream );
		return true;
	}
	else
	{
		THIS_FEATURE()->PrintWarning( "CInputContext::SaveToFile: unable to open file \"%s\"\n", m_sFileName.c_str() );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Erase frames after from the current one
//-----------------------------------------------------------------------------

void CInputContext::Split( void )
{
	//Assert( THIS_FEATURE()->IsPlayingback() );

	if ( m_iCurrentFrame != m_frames.size() - 1 )
	{
		VectorCopy( CurrentFrame().info.origin, *reinterpret_cast<Vector *>( m_FrameBuffer.info.origin ) );
		VectorCopy( CurrentFrame().info.velocity, *reinterpret_cast<Vector *>( m_FrameBuffer.info.velocity ) );

		m_frames.erase( m_frames.begin() + m_iCurrentFrame /* + 1 */, m_frames.end() );

		m_bSavedInfos = true;
	}
}

//-----------------------------------------------------------------------------
// Goto / Forward / backward
//-----------------------------------------------------------------------------

void CInputContext::GotoFrame( int iFrame )
{
	edict_t *pPlayer;
	int iCurrentFrame;

	if ( m_frames.size() == 0 )
		return;

	iCurrentFrame = iFrame - 1;

	// Clamp
	if ( iCurrentFrame < 0 )
		iCurrentFrame = 0;
	else if ( iCurrentFrame >= (int)m_frames.size() )
		iCurrentFrame = m_frames.size() - 1;

	// Apply new frame
	m_iCurrentFrame = iCurrentFrame;

	// Set origin & velocity
	if ( Modules::server->Host_IsServerActive() && ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
	{
		if ( sc_im_goto_exact.GetBool() )
		{
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.origin ), pPlayer->v.origin );
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.velocity ), pPlayer->v.velocity );
		}

		cl_enginefuncs->SetViewAngles( CurrentFrame().inputs.realviewangles );
	}
}

void CInputContext::ForwardFrames( int iFrames )
{
	edict_t *pPlayer;
	int iCurrentFrame;

	if ( m_frames.size() == 0 )
		return;

	//Assert( iFrames > 0 );

	iCurrentFrame = m_iCurrentFrame + iFrames;

	// Clamp
	if ( iCurrentFrame < 0 )
		iCurrentFrame = 0;
	else if ( iCurrentFrame >= (int)m_frames.size() )
		iCurrentFrame = m_frames.size() - 1;

	// Apply new frame
	m_iCurrentFrame = iCurrentFrame;

	// Set origin & velocity
	if ( Modules::server->Host_IsServerActive() && ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
	{
		if ( sc_im_goto_exact.GetBool() )
		{
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.origin ), pPlayer->v.origin );
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.velocity ), pPlayer->v.velocity );
		}

		cl_enginefuncs->SetViewAngles( CurrentFrame().inputs.realviewangles );
	}
}

void CInputContext::BackwardFrames( int iFrames )
{
	edict_t *pPlayer;
	int iCurrentFrame;

	if ( m_frames.size() == 0 )
		return;

	//Assert( iFrames > 0 );

	iCurrentFrame = m_iCurrentFrame - iFrames;

	// Clamp
	if ( iCurrentFrame < 0 )
		iCurrentFrame = 0;
	else if ( iCurrentFrame >= (int)m_frames.size() )
		iCurrentFrame = m_frames.size() - 1;

	// Apply new frame
	m_iCurrentFrame = iCurrentFrame;

	// Set origin & velocity
	if ( Modules::server->Host_IsServerActive() && ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
	{
		if ( sc_im_goto_exact.GetBool() )
		{
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.origin ), pPlayer->v.origin );
			VectorCopy( *reinterpret_cast<Vector *>( CurrentFrame().info.velocity ), pPlayer->v.velocity );
		}

		cl_enginefuncs->SetViewAngles( CurrentFrame().inputs.realviewangles );
	}
}

//-----------------------------------------------------------------------------
// IncrementFramesCounter
//-----------------------------------------------------------------------------

void CInputContext::IncrementFramesCounter( void )
{
	m_iCurrentFrame++;
}

//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

im_frame_t &CInputContext::CurrentFrame( void )
{
	return m_frames[ m_iCurrentFrame ];
}

im_frame_t &CInputContext::FrameBuffer( void )
{
	return m_FrameBuffer;
}

std::vector<im_frame_t> &CInputContext::Frames( void )
{
	return m_frames;
}

int CInputContext::FrameCounter( void ) const
{
	return m_iCurrentFrame;
}

int CInputContext::Version( void ) const
{
	return m_iVersion;
}

const char *CInputContext::FileName( void ) const
{
	return m_sFileName.c_str();
}

//-----------------------------------------------------------------------------
// Record input
//-----------------------------------------------------------------------------

void CInputContext::RecordInput( float frametime, usercmd_t *cmd, int active )
{
	Vector va;

	cl_enginefuncs->GetViewAngles( va );

	m_FrameBuffer.inputs.realviewangles[ 0 ] = va[ 0 ];
	m_FrameBuffer.inputs.realviewangles[ 1 ] = va[ 1 ];
	m_FrameBuffer.inputs.realviewangles[ 2 ] = va[ 2 ];

	m_FrameBuffer.inputs.viewangles[ 0 ] = cmd->viewangles[ 0 ];
	m_FrameBuffer.inputs.viewangles[ 1 ] = cmd->viewangles[ 1 ];
	m_FrameBuffer.inputs.viewangles[ 2 ] = cmd->viewangles[ 2 ];

	m_FrameBuffer.inputs.forwardmove = cmd->forwardmove;
	m_FrameBuffer.inputs.sidemove = cmd->sidemove;

	m_FrameBuffer.inputs.buttons = cmd->buttons;
	m_FrameBuffer.inputs.impulse = cmd->impulse;
	m_FrameBuffer.inputs.weaponselect = cmd->weaponselect;

	THIS_FEATURE()->SavedInputsSignal( true );

	// Save input
	m_frames.push_back( m_FrameBuffer );

	if ( sc_im_experimental.GetBool() && m_pExperimental != NULL )
	{
		static char buffar[ 2048 ];

		edict_t *pPlayer;

		if ( Modules::server->Host_IsServerActive() &&
			 ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
		{
			fprintf( m_pExperimental, "%d:%.3f %.3f %.3f:%.3f %.3f %.3f:%.3f %.3f %.3f\n",
					 cmd->buttons, VectorExpand( va ), VectorExpand( pPlayer->v.origin ), VectorExpand( pPlayer->v.velocity ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Playback input
//-----------------------------------------------------------------------------

void CInputContext::PlaybackInput( float frametime, usercmd_t *cmd, int active )
{
	Vector va;

	if ( m_iCurrentFrame == m_frames.size() - 1 )
	{
		Modules::scripts->Callbacks()->OnPlayEnd( m_sFileName.c_str(), (int)m_frames.size() );
		THIS_FEATURE()->Stop();
		return;
	}

	va[ 0 ] = CurrentFrame().inputs.realviewangles[ 0 ];
	va[ 1 ] = CurrentFrame().inputs.realviewangles[ 1 ];
	va[ 2 ] = CurrentFrame().inputs.realviewangles[ 2 ];

	cmd->viewangles[ 0 ] = CurrentFrame().inputs.viewangles[ 0 ];
	cmd->viewangles[ 1 ] = CurrentFrame().inputs.viewangles[ 1 ];
	cmd->viewangles[ 2 ] = CurrentFrame().inputs.viewangles[ 2 ];

	cmd->forwardmove = CurrentFrame().inputs.forwardmove;
	cmd->sidemove = CurrentFrame().inputs.sidemove;

	cmd->buttons = CurrentFrame().inputs.buttons;
	cmd->impulse = CurrentFrame().inputs.impulse;
	cmd->weaponselect = CurrentFrame().inputs.weaponselect;

	cl_enginefuncs->SetViewAngles( va );

	Modules::scripts->Callbacks()->OnPlayInput( m_sFileName.c_str(), m_iCurrentFrame + 1, cmd );
}

//-----------------------------------------------------------------------------
// Record
//-----------------------------------------------------------------------------

bool CInputManager::Record( const char *pszFilename )
{
	if ( m_state != IM_NONE )
	{
		PrintWarning( "Unable to record file \"%s\". Already %s file \"%s\"\n",
					  pszFilename,
					  m_state == IM_RECORDING ? "recording" : "playing back",
					  m_InputContext.FileName() );

		return false;
	}

	m_InputContext.Init( pszFilename );

	gameutils->PrintChatText( "Recording inputs to file \"%s\"...\n", m_InputContext.FileName() );
	PrintMsg( "Recording inputs to file \"%s\"...\n", m_InputContext.FileName() );

	m_state = IM_RECORDING;
	m_bSavedInputs = false;
	m_bForceViewAngles = false;
	m_sQueuedCommands.clear();

	if ( sc_im_experimental.GetBool() )
		m_pExperimental = fopen( SVENINT_FOLDER_NAME "/input_manager/experimental.txt", "w" );

	return true;
}

//-----------------------------------------------------------------------------
// Playback
//-----------------------------------------------------------------------------

bool CInputManager::Playback( const char *pszFilename )
{
	if ( m_state != IM_NONE )
	{
		PrintWarning( "Unable to playback file \"%s\". Already %s file \"%s\"\n",
					  pszFilename,
					  m_state == IM_RECORDING ? "recording" : "playing back",
					  m_InputContext.FileName() );

		return false;
	}

	m_InputContext.Init( pszFilename );

	if ( !m_InputContext.LoadFromFile() )
	{
		PrintWarning( "Playback: failed to playback file \"%s\"\n", m_InputContext.FileName() );
		m_InputContext.Clear();

		return false;
	}

	gameutils->PrintChatText( "Playing back inputs from file \"%s\"...\n", m_InputContext.FileName() );
	PrintMsg( "Playing back inputs from file \"%s\"...\n", m_InputContext.FileName() );

	m_state = IM_PLAYINGBACK;
	m_bForceViewAngles = false;

	// Command buffer
	if ( m_InputContext.Frames().size() > 0 && m_InputContext.Frames()[ 0 ].commands != NULL )
	{
		cl_enginefuncs->pfnClientCmd( m_InputContext.Frames()[ 0 ].commands );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Split playback and start recording
//-----------------------------------------------------------------------------

bool CInputManager::Split( void )
{
	if ( m_state == IM_NONE )
	{
		PrintWarning( "Unable to split inputs when not playing back anything\n" );
		return false;
	}
	else if ( m_state == IM_RECORDING )
	{
		PrintWarning( "Unable to split inputs. Currently recording file \"%s\"\n", m_InputContext.FileName() );
		return false;
	}

	m_InputContext.Split();

	m_state = IM_RECORDING;
	m_bSavedInputs = false;
	m_bForceViewAngles = true;

	gameutils->PrintChatText( "Splitted inputs from file \"%s\" on frame %d. Recording now...\n", m_InputContext.FileName(), m_InputContext.FrameCounter() + 1 );
	PrintMsg( "Splitted inputs from file \"%s\" on frame %d. Recording now...\n", m_InputContext.FileName(), m_InputContext.FrameCounter() + 1 );

	return true;
}

//-----------------------------------------------------------------------------
// Stop recording / playing back
//-----------------------------------------------------------------------------

bool CInputManager::Stop( bool bAutoStop /* = false */ )
{
	if ( m_state != IM_NONE )
	{
		if ( m_state == IM_RECORDING )
		{
			PrintMsg( "Saved recorded inputs to file \"%s\" (frames: %d)\n", m_InputContext.FileName(), m_InputContext.Frames().size() );
			if ( !bAutoStop )
				gameutils->PrintChatText( "Saved recorded inputs to file \"%s\" (frames: %d)\n", m_InputContext.FileName(), m_InputContext.Frames().size() );

			m_InputContext.SaveToFile();
		}
		else
		{
			PrintMsg( "Stopped playing back inputs from file \"%s\" (frames: %d)\n", m_InputContext.FileName(), GetCurrentFrame() );
			if ( !bAutoStop )
				gameutils->PrintChatText( "Stopped playing back inputs from file \"%s\" (frames: %d)\n", m_InputContext.FileName(), GetCurrentFrame() );
		}
	}

	if ( m_pExperimental != NULL )
		fclose( m_pExperimental );

	m_InputContext.Clear();

	m_state = IM_NONE;
	return true;
}

//-----------------------------------------------------------------------------
// Goto / Forward / backward
//-----------------------------------------------------------------------------

void CInputManager::Goto( int iFrame )
{
	if ( m_state != IM_PLAYINGBACK )
	{
		PrintWarning( "It's allowed to jump to frame only when playing back inputs\n" );
		return;
	}

	m_InputContext.GotoFrame( iFrame );

	gameutils->PrintChatText( "Jumped to frame %d\n", GetCurrentFrame() );
	PrintMsg( "Jumped to frame %d\n", GetCurrentFrame() );
}

void CInputManager::Forward( int iFrames )
{
	int iSavedCurrentFrame, iFramesDifference;

	if ( m_state != IM_PLAYINGBACK )
	{
		PrintWarning( "It's allowed to forward frames only when playing back inputs\n" );
		return;
	}

	iSavedCurrentFrame = m_InputContext.FrameCounter();

	m_InputContext.ForwardFrames( iFrames );

	iFramesDifference = abs( m_InputContext.FrameCounter() - iSavedCurrentFrame );

	if ( iFramesDifference > 0 )
	{
		gameutils->PrintChatText( "Forwarded on %d frames. Current frame: %d\n", iFramesDifference, GetCurrentFrame() );
		PrintMsg( "Forwarded on %d frames. Current frame: %d\n", iFramesDifference, GetCurrentFrame() );
	}
}

void CInputManager::Backward( int iFrames )
{
	int iSavedCurrentFrame, iFramesDifference;

	if ( m_state != IM_PLAYINGBACK )
	{
		PrintWarning( "It's allowed to backward frames only when playing back inputs\n" );
		return;
	}

	iSavedCurrentFrame = m_InputContext.FrameCounter();

	m_InputContext.BackwardFrames( iFrames );

	iFramesDifference = abs( iSavedCurrentFrame - m_InputContext.FrameCounter() );

	if ( iFramesDifference > 0 )
	{
		gameutils->PrintChatText( "Backwarded on %d frames. Current frame: %d\n", iFramesDifference, GetCurrentFrame() );
		PrintMsg( "Backwarded on %d frames. Current frame: %d\n" );
	}
}

//-----------------------------------------------------------------------------
// Record a given command / sequence of commands
//-----------------------------------------------------------------------------

void CInputManager::RecordCommand( const char *pszCommand )
{
	if ( m_state != IM_RECORDING )
	{
		PrintWarning( "It's allowed to record commands when recording inputs\n" );
		return;
	}

	m_sQueuedCommands += pszCommand;
	m_sQueuedCommands += '\n';
}

//-----------------------------------------------------------------------------
// IsInAction / IsRecording / IsPlayingback
//-----------------------------------------------------------------------------

bool CInputManager::IsInAction( void ) const
{
	return m_state != IM_NONE;
}

bool CInputManager::IsRecording( void ) const
{
	return m_state == IM_RECORDING;
}

bool CInputManager::IsPlayingback( void ) const
{
	return m_state == IM_PLAYINGBACK;
}

//-----------------------------------------------------------------------------
// Get current frame
//-----------------------------------------------------------------------------

int CInputManager::GetCurrentFrame( void ) const
{
	return m_InputContext.FrameCounter() + 1;
}

//-----------------------------------------------------------------------------
// Main callback
//-----------------------------------------------------------------------------

void CInputManager::CreateMove( float frametime, usercmd_t *cmd, int active )
{
	if ( m_state == IM_RECORDING )
	{
		if ( m_bForceViewAngles && m_InputContext.Frames().size() > 0 )
		{
			Vector va;

			VectorCopy( m_InputContext.CurrentFrame().inputs.realviewangles, va );
			VectorCopy( m_InputContext.CurrentFrame().inputs.viewangles, cmd->viewangles );

			cl_enginefuncs->SetViewAngles( va );

			m_bForceViewAngles = false;
		}

		m_InputContext.RecordInput( frametime, cmd, active );
	}
	else if ( m_state == IM_PLAYINGBACK )
	{
		m_InputContext.PlaybackInput( frametime, cmd, active );
	}
}

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

void CInputManager::GameFrame( bool bPostRunCmd )
{
	if ( cls->state != ca_active )
		return;

	// Save commands in this frame
	if ( bPostRunCmd )
	{
		if ( m_state == IM_RECORDING )
		{
			// Store commands
			if ( m_bSavedInputs && m_InputContext.Frames().size() > 0 && m_sQueuedCommands.length() > 0 )
			{
				m_InputContext.CurrentFrame().commands = strdup( m_sQueuedCommands.c_str() );
				m_sQueuedCommands.clear();
			}
		}
		else if ( m_state == IM_PLAYINGBACK )
		{
			// Command buffer
			if ( m_InputContext.Frames().size() > 0 && m_InputContext.CurrentFrame().commands != NULL )
			{
				cl_enginefuncs->pfnClientCmd( m_InputContext.CurrentFrame().commands );
			}
		}
	}
	else
	{
		if ( m_state != IM_NONE )
		{
			if ( m_state == IM_RECORDING )
			{
				// Save origin & velocity
				edict_t *pPlayer;

				if ( Modules::server->Host_IsServerActive() && ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
				{
					VectorCopy( pPlayer->v.origin, *reinterpret_cast<Vector *>( m_InputContext.FrameBuffer().info.origin ) );
					VectorCopy( pPlayer->v.velocity, *reinterpret_cast<Vector *>( m_InputContext.FrameBuffer().info.velocity ) );
				}
				else
				{
					memset( &m_InputContext.FrameBuffer().info, 0, IM_FRAME_INFO_SIZE );
				}
			}
			else
			{
				if ( sc_im_exact.GetBool() )
				{
					// Set origin & velocity
					edict_t *pPlayer;

					if ( Modules::server->Host_IsServerActive() && ( pPlayer = sv_enginefuncs->pfnPEntityOfEntIndex( playermove->player_index() + 1 ) ) != NULL )
					{
						VectorCopy( *reinterpret_cast<Vector *>( m_InputContext.CurrentFrame().info.origin ), pPlayer->v.origin );
						VectorCopy( *reinterpret_cast<Vector *>( m_InputContext.CurrentFrame().info.velocity ), pPlayer->v.velocity );
					}
				}

				// Command buffer
				//if ( m_InputContext.FrameCounter() < (int)m_InputContext.Frames().size() && m_InputContext.CurrentFrame().commands != NULL )
				//{
				//	cl_enginefuncs->ClientCmd( m_InputContext.CurrentFrame().commands );
				//}
			}

			m_InputContext.IncrementFramesCounter();
		}

		m_bSavedInputs = false;
	}
}

void CInputManager::OnBeginLoading( void )
{
	Stop( true );
}

void CInputManager::OnFirstClientdataReceived( void )
{
	if ( sc_im_autorecord.GetBool() || sc_im_autoplay.GetBool() )
	{
		if ( m_state == IM_NONE )
		{
			char mapname_buffer[ MAX_PATH ];

			char *pszMapName = mapname_buffer;
			char *pszExt = NULL;

			strncpy( mapname_buffer, cl_enginefuncs->pfnGetLevelName(), MAX_PATH );
			mapname_buffer[ MAX_PATH - 1 ] = 0;

			// maps/<mapname>.bsp to <mapname>.bsp
			while ( *pszMapName ) { if ( *pszMapName == '/' ) { pszMapName++; break; } pszMapName++; }

			pszExt = pszMapName;

			// <mapname>.bsp to <mapname>
			while ( *pszExt ) { if ( *pszExt == '.' ) { *pszExt = 0; break; } pszExt++; }

			if ( sc_im_autoplay.GetBool() )
			{
				Playback( pszMapName );
			}
			else
			{
				Record( pszMapName );
			}
		}
	}
}

void CInputManager::OnVideoInit( void )
{
	Stop( true );
}

void CInputManager::SavedInputsSignal( bool bSaved )
{
	m_bSavedInputs = bSaved;
}

//-----------------------------------------------------------------------------
// Record weapon selection and client/server commands
//-----------------------------------------------------------------------------

void CInputManager::OnCbuf_AddText( const char *pszCommand )
{
	if ( m_state == IM_RECORDING )
	{
		if ( *pszCommand != '\n' )
		{
			if ( !strncmp( "weapon_", pszCommand, strlen( "weapon_" ) ) ||
				 !strncmp( "impulse ", pszCommand, strlen( "impulse " ) ) ||
				 m_WhitelistedCommands.Find( pszCommand ) != NULL )
			{
				m_sQueuedCommands += pszCommand;
				m_sQueuedCommands += '\n';
			}
		}
	}
}

void CInputManager::OnServerCmd( const char *pszCommand )
{
	if ( m_state == IM_RECORDING )
	{
		if ( *pszCommand != '\n' )
		{
			if ( !strncmp( "weapon_", pszCommand, strlen( "weapon_" ) ) )
			{
				m_sQueuedCommands += pszCommand;
				m_sQueuedCommands += '\n';
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CInputManager::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		OnVideoInit();
	}
	else if ( pEvent->GetType() == kOnFirstClientdataReceived_HookEvent )
	{
		OnFirstClientdataReceived();
	}
	else if ( pEvent->GetType() == kSCR_BeginLoadingPlaque_HookEvent )
	{
		OnBeginLoading();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CInputManager::CInputManager( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ),
																				m_WhitelistedCommands( 31, IM_CompareFunc, IM_HashFunc )
{
	m_state = IM_NONE;
	m_bSavedInputs = false;
	m_bForceViewAngles = false;

	m_hCbuf_AddText = DETOUR_INVALID_HANDLE;
	m_hServerCmd = DETOUR_INVALID_HANDLE;
	m_pfnCbuf_AddText = NULL;
	m_pfnServerCmd = NULL;
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CInputManager::Load( void )
{
	ud_t inst;

	m_pfnServerCmd = cl_enginefuncs->pfnServerCmd;

	MemoryUtils()->InitDisasm( &inst, cl_enginefuncs->pfnServerCmd, 32, 17 );
	if ( MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Ijmp )
		{
			m_pfnServerCmd = (unsigned char *)MemoryUtils()->CalcAbsoluteAddress( cl_enginefuncs->pfnServerCmd );
		}
	}

	if ( m_pfnServerCmd == NULL )
		return false;

	uint8_t *pfnClientCmd = (uint8_t *)cl_enginefuncs->pfnClientCmd;

	MemoryUtils()->InitDisasm( &inst, cl_enginefuncs->pfnClientCmd, 32, 17 );
	if ( MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Ijmp )
		{
			pfnClientCmd = (uint8_t *)MemoryUtils()->CalcAbsoluteAddress( cl_enginefuncs->pfnClientCmd );
		}
	}

	if ( pfnClientCmd == NULL )
		return false;

	int iDisassembledBytes = 0;
	MemoryUtils()->InitDisasm( &inst, pfnClientCmd, 32, 24 );

	while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Icall )
		{
			m_pfnCbuf_AddText = (uint8_t *)MemoryUtils()->CalcAbsoluteAddress( pfnClientCmd );
			break;
		}

		pfnClientCmd += iDisassembledBytes;
	}

	FEATURE_CHECK_SYMBOL( m_pfnCbuf_AddText, "Cbuf_AddText" );

	m_WhitelistedCommands.Insert( "kill" );
	m_WhitelistedCommands.Insert( "stuck_kill" );
	m_WhitelistedCommands.Insert( "gibme" );
	m_WhitelistedCommands.Insert( "lastinv" );
	m_WhitelistedCommands.Insert( "drop" );
	m_WhitelistedCommands.Insert( "dropammo" );
	m_WhitelistedCommands.Insert( "sc_chasecam" );
	m_WhitelistedCommands.Insert( "thirdperson" );
	m_WhitelistedCommands.Insert( "firstperson" );
	m_WhitelistedCommands.Insert( "npc_moveto" );
	m_WhitelistedCommands.Insert( "npc_findcover" );
	m_WhitelistedCommands.Insert( "medic" );
	m_WhitelistedCommands.Insert( "healme" );
	m_WhitelistedCommands.Insert( "grenade" );
	m_WhitelistedCommands.Insert( "takecover" );
	m_WhitelistedCommands.Insert( "stuck" );
	m_WhitelistedCommands.Insert( "unstuck" );
	m_WhitelistedCommands.Insert( "sc_freeze" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CInputManager::PostLoad( void )
{
	m_hCbuf_AddText = Detours()->DetourFunction( m_pfnCbuf_AddText, HOOKED_Cbuf_AddText, GET_FUNC_PTR( ORIG_Cbuf_AddText ) );
	m_hServerCmd = Detours()->DetourFunction( m_pfnServerCmd, HOOKED_ServerCmd, GET_FUNC_PTR( ORIG_ServerCmd ) );

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kOnFirstClientdataReceived_HookEvent );
	hookevents->RegisterListener( this, kSCR_BeginLoadingPlaque_HookEvent );

	FEATURE_REGISTER_CVAR( sc_im_autorecord );
	FEATURE_REGISTER_CVAR( sc_im_autoplay );
	FEATURE_REGISTER_CVAR( sc_im_exact );
	FEATURE_REGISTER_CVAR( sc_im_goto_exact );
	FEATURE_REGISTER_CVAR( sc_im_experimental );
	FEATURE_REGISTER_CCMD( sc_im_record_ );
	FEATURE_REGISTER_CCMD( sc_im_play );
	FEATURE_REGISTER_CCMD( sc_im_split );
	FEATURE_REGISTER_CCMD( sc_im_stop );
	FEATURE_REGISTER_CCMD( sc_im_goto );
	FEATURE_REGISTER_CCMD( sc_im_forward );
	FEATURE_REGISTER_CCMD( sc_im_backward );
	FEATURE_REGISTER_CCMD( sc_im_frame );
	FEATURE_REGISTER_CCMD( sc_im_test );
	FEATURE_REGISTER_CCMD( sc_im_record_command_ );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CInputManager::Unload( void )
{
	FEATURE_UNREGISTER_CVAR( sc_im_autorecord );
	FEATURE_UNREGISTER_CVAR( sc_im_autoplay );
	FEATURE_UNREGISTER_CVAR( sc_im_exact );
	FEATURE_UNREGISTER_CVAR( sc_im_goto_exact );
	FEATURE_UNREGISTER_CVAR( sc_im_experimental );
	FEATURE_UNREGISTER_CCMD( sc_im_record_ );
	FEATURE_UNREGISTER_CCMD( sc_im_play );
	FEATURE_UNREGISTER_CCMD( sc_im_split );
	FEATURE_UNREGISTER_CCMD( sc_im_stop );
	FEATURE_UNREGISTER_CCMD( sc_im_goto );
	FEATURE_UNREGISTER_CCMD( sc_im_forward );
	FEATURE_UNREGISTER_CCMD( sc_im_backward );
	FEATURE_UNREGISTER_CCMD( sc_im_frame );
	FEATURE_UNREGISTER_CCMD( sc_im_test );
	FEATURE_UNREGISTER_CCMD( sc_im_record_command_ );

	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kOnFirstClientdataReceived_HookEvent );
	hookevents->UnregisterListener( this, kSCR_BeginLoadingPlaque_HookEvent );

	Detours()->RemoveDetour( m_hCbuf_AddText );
	Detours()->RemoveDetour( m_hServerCmd );
}
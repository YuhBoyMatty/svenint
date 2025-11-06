// SvenInt (c) Sw1ft
// misc_demo_message.cpp

#include "stdafx.h"
#include "misc_demo_message.h"
#include "misc_edge_pixels_player.h"
#include "game/messagebuffer.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CDemoMessage, demomessage, "Misc", "Demo Message" );

static CMessageBuffer demoMsg;
static char demobuffer[ 2048 ];

//-----------------------------------------------------------------------------
// WriteSegmentInfo
//-----------------------------------------------------------------------------

void CDemoMessage::WriteSegmentInfo( float flTime, const char *pszFormattedTimer, const char *pszMapname )
{
	if ( demoplayback )
		return;

	demoMsg.Init( "SegmentInfo", demobuffer, Q_ARRAYSIZE( demobuffer ) );

	demoMsg.WriteLong( TYPE_USER_DEMOMSG );
	demoMsg.WriteShort( kDemoMsgSegmentInfo );
	demoMsg.WriteFloat( flTime );
	demoMsg.WriteString( (char *)pszFormattedTimer );
	demoMsg.WriteString( (char *)pszMapname );

	demobuffer[ Q_ARRAYSIZE( demobuffer ) - 1 ] = 0;

	cl_enginefuncs->pDemoAPI->WriteClientDLLMessage( demoMsg.GetBuffer()->cursize, demoMsg.GetBuffer()->data );
}

//-----------------------------------------------------------------------------
// WriteEdgePixelsPlayer
//-----------------------------------------------------------------------------

void CDemoMessage::WriteEdgePixelsPlayer( const char *pszFilename, double width, double height, const Vector &vecPos, const Vector &vecAngles, int iDrawCalls )
{
	if ( demoplayback )
		return;

	union
	{
		double m_dbl;
		struct
		{
			unsigned long low;
			unsigned long high;
		} m_llu;
	} dbl;

	demoMsg.Init( "Edge-Pixels Player", demobuffer, Q_ARRAYSIZE( demobuffer ) );

	demoMsg.WriteLong( TYPE_USER_DEMOMSG );
	demoMsg.WriteShort( kDemoMsgEdgePixelsPlayer );

	demoMsg.WriteByte( 0 );
	demoMsg.WriteString( (char *)pszFilename );

	dbl.m_dbl = width;
	demoMsg.WriteLong( dbl.m_llu.low );
	demoMsg.WriteLong( dbl.m_llu.high );

	dbl.m_dbl = height;
	demoMsg.WriteLong( dbl.m_llu.low );
	demoMsg.WriteLong( dbl.m_llu.high );

	demoMsg.WriteFloat( vecPos.x );
	demoMsg.WriteFloat( vecPos.y );
	demoMsg.WriteFloat( vecPos.z );

	demoMsg.WriteFloat( vecAngles.x );
	demoMsg.WriteFloat( vecAngles.y );
	demoMsg.WriteFloat( vecAngles.z );

	demoMsg.WriteLong( iDrawCalls );

	demobuffer[ Q_ARRAYSIZE( demobuffer ) - 1 ] = 0;

	cl_enginefuncs->pDemoAPI->WriteClientDLLMessage( demoMsg.GetBuffer()->cursize, demoMsg.GetBuffer()->data );
}

//-----------------------------------------------------------------------------
// WriteEdgePixelsPlayerStop
//-----------------------------------------------------------------------------

void CDemoMessage::WriteEdgePixelsPlayerStop( void )
{
	if ( demoplayback )
		return;

	demoMsg.Init( "Edge-Pixels Player", demobuffer, Q_ARRAYSIZE( demobuffer ) );

	demoMsg.WriteLong( TYPE_USER_DEMOMSG );
	demoMsg.WriteShort( kDemoMsgEdgePixelsPlayer );

	demoMsg.WriteByte( 1 );

	demobuffer[ Q_ARRAYSIZE( demobuffer ) - 1 ] = 0;

	cl_enginefuncs->pDemoAPI->WriteClientDLLMessage( demoMsg.GetBuffer()->cursize, demoMsg.GetBuffer()->data );
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CDemoMessage::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	CMessageBuffer msgBuffer;

	auto buffer = pEvent->GetArg<unsigned char *>( "buffer" );
	auto size = pEvent->GetArg<int>( "size" );

	msgBuffer.Init( "UserDemoMsg", buffer, size, true );

	if ( msgBuffer.ReadLong() != TYPE_USER_DEMOMSG )
		return kHookContinue;

	int msgType = msgBuffer.ReadShort();

	switch ( msgType )
	{
	case kDemoMsgSegmentInfo:
	{
		float flSegmentTime = msgBuffer.ReadFloat();

		ConColorMsg( { 255, 165, 0, 255 }, "> Finished segment in " );
		ConColorMsg( { 179, 255, 32, 255 }, msgBuffer.ReadString() );
		ConColorMsg( { 122, 200, 0, 255 }, " (%.6f) ", flSegmentTime );
		ConColorMsg( { 255, 165, 0, 255 }, "(map: %s)\n", msgBuffer.ReadString() );

		break;
	}

	case kDemoMsgEdgePixelsPlayer:
	{
		extern ConVar sc_epp_play_in_demo;

		if ( !sc_epp_play_in_demo.GetBool() )
		{
			//if ( !!msgBuffer.ReadByte() )
			//	g_pEngineFuncs->ClientCmd( "StopSong" );
			//else
			//	g_pEngineFuncs->ClientCmd( "PlayMedia \"Bad Apple!!.mp3\"" );

			if ( !( !!msgBuffer.ReadByte() ) )
				gameutils->PrintChatText( "* Now Playing: C:\\Music\\Bad Apple!!.mp3" );
			break;
		}

		bool bStop = !!msgBuffer.ReadByte();

		if ( bStop )
		{
			Features::edgepixelsplayer->Stop();
			break;
		}

		gameutils->PrintChatText( "* Now Playing: C:\\Music\\Bad Apple!!.mp3" );

		union
		{
			double m_dbl;
			struct
			{
				unsigned long low;
				unsigned long high;
			} m_llu;
		} dbl;

		std::string sFilename;
		double width, height;
		Vector vecPos, vecAngles;
		int drawcalls;

		sFilename = msgBuffer.ReadString();

		dbl.m_llu.low = msgBuffer.ReadLong();
		dbl.m_llu.high = msgBuffer.ReadLong();
		width = dbl.m_dbl;

		dbl.m_llu.low = msgBuffer.ReadLong();
		dbl.m_llu.high = msgBuffer.ReadLong();
		height = dbl.m_dbl;

		vecPos.x = msgBuffer.ReadFloat();
		vecPos.y = msgBuffer.ReadFloat();
		vecPos.z = msgBuffer.ReadFloat();

		vecAngles.x = msgBuffer.ReadFloat();
		vecAngles.y = msgBuffer.ReadFloat();
		vecAngles.z = msgBuffer.ReadFloat();

		drawcalls = msgBuffer.ReadLong();

		Features::edgepixelsplayer->Start( sFilename.c_str(), width, height, vecPos, vecAngles, drawcalls, true );
		break;
	}

	default:
	{
		PrintWarning( "Unrecognized message type\n" );
		break;
	}
	}

	return kHookSupercedeStop;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CDemoMessage::CDemoMessage( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CDemoMessage::PostLoad( void )
{
	hookevents->RegisterListener( this, kDemo_ReadBuffer_HookEvent, kHookCall, kHookPriorityHigh );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CDemoMessage::Unload( void )
{
	hookevents->UnregisterListener( this, kDemo_ReadBuffer_HookEvent );
}
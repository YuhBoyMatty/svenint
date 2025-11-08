// SvenInt (c) Sw1ft
// st_timer.cpp

#include "stdafx.h"
#include "st_timer.h"
#include "r_drawing.h"
#include "misc_demo_message.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CSpeedrunTimer, timer, "Speedrun Tools", "Timer" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

ConVar sc_st_transmit_timer( "sc_st_transmit_timer", "1", FCVAR_EXTDLL, "Transmit to all clients the segment's time" );

CON_COMMAND( sc_st_reset_timer, "Reset speedrun timer" )
{
	THIS_FEATURE()->StartTimer();
}

//-----------------------------------------------------------------------------
// Current segment time
//-----------------------------------------------------------------------------

float CSpeedrunTimer::SegmentCurrentTime( void )
{
	if ( Modules::server->Host_IsServerActive() && m_bSegmentStarted )
		return gpGlobals->time - m_flSegmentStart;

	return 0.f;
}

//-----------------------------------------------------------------------------
// Show timer
//-----------------------------------------------------------------------------

void CSpeedrunTimer::ShowTimer( float flTime, bool bServer )
{
	if ( !bServer )
	{
		m_flTimerTime = flTime;
		m_flLastTimerUpdate = (float)*realtime;
	}

	int minutes = static_cast<int>( flTime ) / 60;
	int seconds = static_cast<int>( flTime ) % 60;
	int ms = static_cast<int>( ( flTime - floorf( flTime ) ) * 1000.f );

	int iSpriteWidth = Features::drawing->GetNumberSpriteWidth();
	int iSpriteHeight = Features::drawing->GetNumberSpriteHeight();

	int iThickness = int( (float)iSpriteWidth / 8.f );

	int x = int( (float)gameutils->GetScreenWidth() * m_pWidthScreenFraction->GetFloat() );
	int y = int( (float)gameutils->GetScreenHeight() * m_pHeightScreenFraction->GetFloat() );

	int r = int( 255.f * m_pColor->GetColor()[ 0 ] );
	int g = int( 255.f * m_pColor->GetColor()[ 1 ] );
	int b = int( 255.f * m_pColor->GetColor()[ 2 ] );
	int a = 232;

	x += Features::drawing->DrawDigit( minutes / 10, x, y, r, g, b, FONT_ALIGN_LEFT );
	x += Features::drawing->DrawDigit( minutes % 10, x, y, r, g, b, FONT_ALIGN_LEFT );

	cl_enginefuncs->pfnFillRGBA( x + ( iSpriteWidth / 2 ) - iThickness,
								 y + ( iSpriteHeight / 6 ),
								 iThickness * 2,
								 iThickness * 2,
								 r, g, b, a );

	cl_enginefuncs->pfnFillRGBA( x + ( iSpriteWidth / 2 ) - iThickness,
								 y + iSpriteHeight - ( iSpriteHeight / 4 ),
								 iThickness * 2,
								 iThickness * 2,
								 r, g, b, a );

	x += iSpriteWidth;

	x += Features::drawing->DrawDigit( seconds / 10, x, y, r, g, b, FONT_ALIGN_LEFT );
	x += Features::drawing->DrawDigit( seconds % 10, x, y, r, g, b, FONT_ALIGN_LEFT );

	cl_enginefuncs->pfnFillRGBA( x + ( iSpriteWidth / 2 ) - iThickness,
								 y + iSpriteHeight - ( iSpriteHeight / 4 ),
								 iThickness * 2,
								 iThickness * 2,
								 r, g, b, a );

	x += iSpriteWidth;

	x += Features::drawing->DrawDigit( ms / 100, x, y, r, g, b, FONT_ALIGN_LEFT );
	x += Features::drawing->DrawDigit( ( ms / 10 ) % 10, x, y, r, g, b, FONT_ALIGN_LEFT );
	Features::drawing->DrawDigit( ms % 10, x, y, r, g, b, FONT_ALIGN_LEFT );
}

//-----------------------------------------------------------------------------
// Start timer
//-----------------------------------------------------------------------------

void CSpeedrunTimer::StartTimer( void )
{
	if ( Modules::server->Host_IsServerActive() && !demoplayback )
	{
		m_bSegmentStarted = true;
		m_flSegmentStart = gpGlobals->time;
		m_flSegmentTime = 0.f;

		ConColorMsg( { 255, 165, 0, 255 }, "> Started segment (map: %s)\n", gpGlobals->pStringBase + gpGlobals->mapname );
	}
	else
	{
		m_bSegmentStarted = false;
		m_flSegmentStart = 0.f;
		m_flSegmentTime = 0.f;
	}
}

//-----------------------------------------------------------------------------
// Stop timer
//-----------------------------------------------------------------------------

void CSpeedrunTimer::StopTimer( void )
{
	if ( !Modules::server->Host_IsServerActive() || demoplayback || !m_bSegmentStarted )
	{
		m_bSegmentStarted = false;
		m_flSegmentStart = 0.f;
		return;
	}

	char timer_buffer[ 128 ];

	float flSegmentTime = m_flSegmentTime = gpGlobals->time - m_flSegmentStart;
	const char *pszMapname = gpGlobals->pStringBase + gpGlobals->mapname;

	int minutes = static_cast<int>( flSegmentTime ) / 60;
	int seconds = static_cast<int>( flSegmentTime ) % 60;
	int ms = static_cast<int>( ( flSegmentTime - floorf( flSegmentTime ) ) * 1000.f );

	snprintf( timer_buffer, Q_ARRAYSIZE( timer_buffer ), "%d%d:%d%d,%d%d%d",
				minutes / 10, minutes % 10,
				seconds / 10, seconds % 10,
				ms / 100, ( ms / 10 ) % 10, ms % 10 );

	gameutils->PrintChatText( "Finished segment in %s (%.6f) (map: %s)\n", timer_buffer, flSegmentTime, pszMapname );

	ConColorMsg( { 255, 165, 0, 255 }, "> Finished segment in " );
	ConColorMsg( { 179, 255, 32, 255 }, timer_buffer );
	ConColorMsg( { 122, 200, 0, 255 }, " (%.6f) ", flSegmentTime );
	ConColorMsg( { 255, 165, 0, 255 }, "(map: %s)\n", pszMapname );

	Features::demomessage->WriteSegmentInfo( flSegmentTime, timer_buffer, pszMapname );

	m_bSegmentStarted = false;
	m_flSegmentStart = 0.f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CSpeedrunTimer::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_flTimerTime = 0.f;
		m_flLastTimerUpdate = -1.f;
	}
	else if ( pEvent->GetType() == kHUD_Redraw_HookEvent )
	{
		if ( Modules::server->Host_IsServerActive() )
		{
			float flSegmentTime;

			if ( m_bSegmentStarted )
				flSegmentTime = gpGlobals->time - m_flSegmentStart;
			else
				flSegmentTime = m_flSegmentTime;

			ShowTimer( flSegmentTime, true );

			if ( sc_st_transmit_timer.GetBool() )
			{
				sv_enginefuncs->pfnMessageBegin( MSG_BROADCAST, SVC_SVENINT, NULL, NULL );
					sv_enginefuncs->pfnWriteByte( SVENINT_COMM_TIMER );
					sv_enginefuncs->pfnWriteCoord( flSegmentTime );
				sv_enginefuncs->pfnMessageEnd();
			}
		}
		else if ( m_flLastTimerUpdate > 0.f && (float)*realtime - m_flLastTimerUpdate <= 1.f )
		{
			ShowTimer( m_flTimerTime, false );
		}
	}
	else if ( pEvent->GetType() == kSCR_BeginLoadingPlaque_HookEvent )
	{
		m_bIsC17 = false;

		m_iNihilanthIndex = 0;
		m_pNihilanthVars = NULL;

		StopTimer();
	}
	else if ( pEvent->GetType() == kOnFirstClientdataReceived_HookEvent )
	{
		StartTimer();

		if ( !Modules::server->Host_IsServerActive() )
			return kHookContinue;
		
		const char *pszMapname = gpGlobals->pStringBase + gpGlobals->mapname;

		m_iNihilanthIndex = 0;
		m_pNihilanthVars = NULL;

		if ( !stricmp( pszMapname, "hl_c17" ) )
		{
			m_bIsC17 = true;

			edict_t *pNihilanth = NULL;

			if ( ( pNihilanth = sv_enginefuncs->pfnFindEntityByString( NULL, "targetname", "nihilanth" ) ) != NULL )
			{
				m_iNihilanthIndex = sv_enginefuncs->pfnIndexOfEdict( pNihilanth );
				m_pNihilanthVars = &pNihilanth->v;
			}
		}
		else
		{
			m_bIsC17 = false;
		}
	}
	else if ( pEvent->GetType() == kHost_FilterTime_HookEvent )
	{
		if ( !pEvent->GetReturn<qboolean>() || cls->state != ca_active )
			return kHookContinue;

		if ( m_bIsC17 && m_iNihilanthIndex != 0 )
		{
			edict_t *pNihilanth = sv_enginefuncs->pfnPEntityOfEntIndex( m_iNihilanthIndex );

			if ( pNihilanth != NULL && &pNihilanth->v == m_pNihilanthVars )
			{
				float flHealth = m_pNihilanthVars->health;
				if ( flHealth <= 0.f )
				{
					StopTimer();

					m_iNihilanthIndex = 0;
					m_pNihilanthVars = NULL;
				}
			}
			else
			{
				m_iNihilanthIndex = 0;
				m_pNihilanthVars = NULL;
			}
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CSpeedrunTimer::CSpeedrunTimer( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pWidthScreenFraction = NULL;
	m_pHeightScreenFraction = NULL;
	m_pColor = NULL;

	m_bSegmentStarted = false;

	m_bIsC17 = false;
	m_iNihilanthIndex = 0;
	m_pNihilanthVars = NULL;

	m_flSegmentTime = 0.f;
	m_flSegmentStart = 0.f;

	m_flTimerTime = 0.f;
	m_flLastTimerUpdate = -1.f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CSpeedrunTimer::OnEnable( void )
{
	m_bIsC17 = false;

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kSCR_BeginLoadingPlaque_HookEvent );
	hookevents->RegisterListener( this, kOnFirstClientdataReceived_HookEvent );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CSpeedrunTimer::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_Redraw_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kSCR_BeginLoadingPlaque_HookEvent );
	hookevents->UnregisterListener( this, kOnFirstClientdataReceived_HookEvent );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CSpeedrunTimer::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	Color clrTimer;
	UnpackRGB( *(unsigned char *)&clrTimer.r,
			   *(unsigned char *)&clrTimer.g,
			   *(unsigned char *)&clrTimer.b,
			   RGB_SVENISH );

	m_pWidthScreenFraction = Modules::menu->AddParamFloat( this, "WidthScreenFraction", NULL, 0.01f, 0.f, 1.f );
	m_pHeightScreenFraction = Modules::menu->AddParamFloat( this, "HeightScreenFraction", NULL, 0.35f, 0.f, 1.f );
	m_pColor = Modules::menu->AddParamColorRGB( this, "Color", NULL, clrTimer );
	
	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CSpeedrunTimer::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_st_reset_timer );
	FEATURE_REGISTER_CVAR( sc_st_transmit_timer );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CSpeedrunTimer::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_st_reset_timer );
	FEATURE_UNREGISTER_CVAR( sc_st_transmit_timer );
}
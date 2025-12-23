// SvenInt (c) Sw1ft
// hook_events.cpp

#include "stdafx.h"
#include "hook_events.h"

//-----------------------------------------------------------------------------
// CHookEvent
//-----------------------------------------------------------------------------

void CHookEvent::ArgAccessFailure( const char *pszArgName )
{
	char buffer[ 256 ];
	snprintf( buffer, Q_ARRAYSIZE( buffer ), "[SvenInt] Accessing a non-existent argument \"%s\" in CHookEvent::GetArg() (event type: %d)", pszArgName, m_nEventType );

#ifdef WIN32
	MessageBox( NULL, buffer, "Fatal Error", MB_ICONERROR );
	TerminateProcess( GetCurrentProcess(), EXIT_FAILURE );
#else
	printf( buffer );
	exit( 1 );
#endif
}

//-----------------------------------------------------------------------------
// Hook events
//-----------------------------------------------------------------------------

CHookEvents::CHookEvents()
{
	m_hookEvents.resize( kHookEventLast );
}

bool CHookEvents::RegisterListener( IHookEventListener *pListener, int iHookEvent, int iHookType /* = kHookCall */, int iHookPriority /* = kHookPriorityNormal */ )
{
	Assert( iHookEvent > kHookEventNone && iHookEvent < kHookEventLast );

	HooksCallChain *hooks;
	hook_events_t &chain = m_hookEvents[ iHookEvent ];

	if ( iHookType == kHookCall )
		hooks = &chain.hooks;
	else
		hooks = &chain.posthooks;

	auto it = std::find( hooks->begin(), hooks->end(), pListener );
	if ( it != hooks->end() )
		return false;

	if ( iHookPriority == kHookPriorityNormal )
	{
		hooks->insert( hooks->begin() + hooks->size() / 2, pListener );
	}
	else if ( iHookPriority == kHookPriorityLow )
	{
		hooks->push_back( pListener );
	}
	else
	{
		hooks->insert( hooks->begin(), pListener );
	}

	DevMsg( "<SvenInt::HookEvents> Registered %shook event listener of type %d (priority: %s)\n",
			iHookType == kHookCall ? "" : "'post' ",
			iHookEvent,
			iHookPriority == kHookPriorityNormal ? "Normal" : iHookPriority == kHookPriorityHigh ? "High" : "Low" );
	return true;
}

bool CHookEvents::UnregisterListener( IHookEventListener *pListener, int iHookEvent, int iHookType /* = kHookCall */ )
{
	Assert( iHookEvent > kHookEventNone && iHookEvent < kHookEventLast );

	HooksCallChain *hooks;
	hook_events_t &chain = m_hookEvents[ iHookEvent ];

	if ( iHookType == kHookCall )
		hooks = &chain.hooks;
	else
		hooks = &chain.posthooks;

	auto it = std::find( hooks->begin(), hooks->end(), pListener );
	if ( it == hooks->end() )
		return false;

	hooks->erase( it );
	return true;
}

const HooksCallChain &CHookEvents::GetCallChain( int iHookEvent, int iHookType )
{
	return iHookType == kHookCall ? m_hookEvents[ iHookEvent ].hooks : m_hookEvents[ iHookEvent ].posthooks;
}

CHookEvents gHookEvents;
CHookEvents *HookEvents()
{
	return &gHookEvents;
}

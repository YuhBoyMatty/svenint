// SvenInt (c) Sw1ft
// hook_events.h

#ifndef SINT_HOOK_EVENTS_H
#define SINT_HOOK_EVENTS_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>

//-----------------------------------------------------------------------------
// Hook priorities
//-----------------------------------------------------------------------------

typedef enum
{
	kHookPriorityNormal = 0,	// place hook in the middle of the call chain ( default priority )
	kHookPriorityLow,			// place hook at the bottom of the call chain ( will be one of the last to be called )
	kHookPriorityHigh			// place hook at the top of the call chain ( will be one of the first to be called )
} EHookPriority;

//-----------------------------------------------------------------------------
// Hook types
//-----------------------------------------------------------------------------

typedef enum
{
	kHookCall = 0,	// call hook then run the intercepted function
	kHookPostCall	// run the intercepted function then call hook
} EHookType;

//-----------------------------------------------------------------------------
// Hook codes
//-----------------------------------------------------------------------------

typedef enum
{
	kHookContinue = 0,	// keep going, any changes of return value or call arguments are saved ( behavior: return ORIG_Func() )
	kHookHandled,		// the hook situation was handled but nothing significant was changed ( behavior: return ORIG_Func() )
	kHookOverride,		// run the intercepted function but use our return value instead ( behavior: ORIG_Func(); return myResult )
	kHookSupercede,		// don't run the intercepted function at all ( behavior: return myResult until all hook events will be called )
	kHookSupercedeStop	// apply kHookSupercede but leave the hooked function immediately ( behavior: return myResult )
} EHookResult;

//-----------------------------------------------------------------------------
// Hook event types
//-----------------------------------------------------------------------------

typedef enum
{
	kHookEventNone = 0,

	kHUD_VidInit_HookEvent,
	kHUD_Redraw_HookEvent,
	kHUD_UpdateClientData_HookEvent,
	kCL_CreateMove_HookEvent,
	kV_CalcRefdef_HookEvent,
	kHUD_AddEntity_HookEvent,
	kHUD_DrawTransparentTriangles_HookEvent,
	kHUD_StudioEvent_HookEvent,
	kHUD_PostRunCmd_HookEvent,
	kDemo_ReadBuffer_HookEvent,
	kHUD_Frame_HookEvent,
	kHUD_Key_Event_HookEvent,
	kHUD_OnClientDisconnect_HookEvent,
	kOnFirstClientdataReceived_HookEvent,

	kIN_Move_HookEvent,
	kKey_Event_HookEvent,

	kwglSwapBuffers_HookEvent,
	kVGuiClientPanelPaint_HookEvent,

	kHost_FilterTime_HookEvent,
	kGameFrameEvent = kHost_FilterTime_HookEvent,

	kSCR_UpdateScreen_HookEvent,
	kSCR_BeginLoadingPlaque_HookEvent,
	kSCR_EndLoadingPlaque_HookEvent,

	kNetchan_Transmit_HookEvent,
	kPacketEvent = kNetchan_Transmit_HookEvent,

	kMSG_WriteUsercmd_HookEvent,

	kServerInfo_HookEvent,

	kStudioSetupBones_HookEvent,
	kStudioRenderModel_HookEvent,

	kHookEventLast
} EHookEventType;

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

class CHookEvent
{
public:
	CHookEvent( int nEventType, EHookResult &hookResult ) : m_pReturn( 0 )
	{
		m_nEventType = nEventType;
		m_pHookResult = &hookResult;
		m_args.reserve( 2 );
	}

	inline int GetType( void ) const { return m_nEventType; }
	inline const EHookResult GetHookResult( void ) const { return *m_pHookResult; }

	inline void SetReturn( void *ptr ) { m_pReturn = ptr; }
	inline void PushArg( const char *pszArgName, void *ptr ) { m_args.push_back( { pszArgName, ptr } ); }

	template <class T>
	inline T &GetReturn( void ) const { return *(T *)m_pReturn; }

	template <class T>
	T &GetArg( const char *pszArgName );

private:
	void ArgAccessFailure( const char *pszArgName );

private:
	typedef struct
	{
		const char *pszArgName;
		void *pVar;
	} hook_event_arg_t;

	int								m_nEventType;
	EHookResult						*m_pHookResult;
	void							*m_pReturn;
	std::vector<hook_event_arg_t>	m_args;
};

#pragma warning( push )
#pragma warning( disable : 4715 )

template <class T>
inline T &CHookEvent::GetArg( const char *pszArgName )
{
	for ( const hook_event_arg_t &arg : m_args )
	{
	#if 0
		if ( !strcmp( arg.pszArgName, pszArgName ) )
	#else
		// Statically defined strings and their pointers are fine to be used for comparison
		if ( arg.pszArgName == pszArgName )
	#endif
			return *(T *)arg.pVar;
	}

#if 0
	static T dummy;
	memset( &dummy, 0, sizeof( T ) );

	return dummy;
#else
	ArgAccessFailure( pszArgName );
#endif
}

#pragma warning( pop )

//-----------------------------------------------------------------------------
// Hook event interface listener
//-----------------------------------------------------------------------------

class IHookEventListener
{
public:
	virtual ~IHookEventListener() {}
	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) = 0;
};

//-----------------------------------------------------------------------------
// CHookEvents
//-----------------------------------------------------------------------------

typedef std::vector<IHookEventListener *> HooksCallChain;

class CHookEvents
{
	friend class CHooksModule;

public:
	CHookEvents();

	bool RegisterListener( IHookEventListener *pListener, int iHookEvent, int iHookType = kHookCall, int iHookPriority = kHookPriorityNormal );
	bool UnregisterListener( IHookEventListener *pListener, int iHookEvent, int iHookType = kHookCall );

	const HooksCallChain &GetCallChain( int iHookEvent, int iHookType );

private:
	typedef struct
	{
		HooksCallChain hooks;	// pre call hooks
		HooksCallChain posthooks; // post call hooks
	} hook_events_t;

	std::vector<hook_events_t> m_hookEvents;
};

CHookEvents *HookEvents();

#endif // SINT_HOOK_EVENTS_H
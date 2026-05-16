// SvenInt (c) Sw1ft
// svenmod_emulation.cpp

#include "stdafx.h"
#include "game/interface.h"

bool gbLoadedAsSvenModPlugin = false;

typedef struct api_version_s
{
	int major_version;
	int minor_version;
} api_version_t;

typedef enum
{
	PLUGIN_CONTINUE = 0,	// keep going
	PLUGIN_OVERRIDE,		// run the game dll function but use our return value instead
	PLUGIN_STOP,			// don't run the game dll function at all
	PLUGIN_CALL_STOP		// call the game dll function and stop further hooks from executing 
} PLUGIN_RESULT;

class IClientPlugin
{
public:
	virtual ~IClientPlugin() {}

	virtual api_version_t GetAPIVersion( void ) = 0;

	virtual bool Load( CreateInterfaceFn pfnSvenModFactory, void *pSvenModAPI, void *pPluginHelpers ) = 0;

	virtual void PostLoad( bool bGlobalLoad ) = 0;

	virtual void Unload( void ) = 0;

	virtual bool Pause( void ) = 0;

	virtual void Unpause( void ) = 0;

	virtual void OnFirstClientdataReceived( void *pcldata, float flTime ) = 0;

	virtual void OnBeginLoading( void ) = 0;

	virtual void OnEndLoading( void ) = 0;

	virtual void OnDisconnect( void ) = 0;

	virtual void GameFrame( void *state, double frametime, bool bPostRunCmd ) = 0;

	// Called to draw 2D paints after rendering the game view
	virtual PLUGIN_RESULT Draw( void ) = 0;

	// Called to redraw client's HUD
	virtual PLUGIN_RESULT DrawHUD( float time, int intermission ) = 0;

	// Name of the plugin
	virtual const char *GetName( void ) = 0;

	// Author of the plugin
	virtual const char *GetAuthor( void ) = 0;

	// Version of the plugin
	virtual const char *GetVersion( void ) = 0;

	// Description of the plugin
	virtual const char *GetDescription( void ) = 0;

	// URL of the plugin
	virtual const char *GetURL( void ) = 0;

	// Build date of the plugin
	virtual const char *GetDate( void ) = 0;

	// Tag of the plugin to log
	virtual const char *GetLogTag( void ) = 0;
};

typedef struct CSvenInt : IClientPlugin
{
	virtual api_version_t GetAPIVersion( void );

	virtual bool Load( CreateInterfaceFn pfnSvenModFactory, void *pSvenModAPI, void *pPluginHelpers );

	virtual void PostLoad( bool bGlobalLoad );

	virtual void Unload( void );

	virtual bool Pause( void );

	virtual void Unpause( void );

	virtual void OnFirstClientdataReceived( void *pcldata, float flTime );

	virtual void OnBeginLoading( void );

	virtual void OnEndLoading( void );

	virtual void OnDisconnect( void );

	virtual void GameFrame( void *state, double frametime, bool bPostRunCmd );

	// Called to draw 2D paints after rendering the game view
	virtual PLUGIN_RESULT Draw( void );

	// Called to redraw client's HUD
	virtual PLUGIN_RESULT DrawHUD( float time, int intermission );

	// Name of the plugin
	virtual const char *GetName( void );

	// Author of the plugin
	virtual const char *GetAuthor( void );

	// Version of the plugin
	virtual const char *GetVersion( void );

	// Description of the plugin
	virtual const char *GetDescription( void );

	// URL of the plugin
	virtual const char *GetURL( void );

	// Build date of the plugin
	virtual const char *GetDate( void );

	// Tag of the plugin to log
	virtual const char *GetLogTag( void );
} CSvenInt;

#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName)                         \
	static IBaseInterface *__Create##className##interfaceName##_interface() { return (IBaseInterface*)&globalVarName; } \
	static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName);

CSvenInt gSvenInt;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CSvenInt, IClientPlugin, "ClientPlugin002", gSvenInt );

api_version_t CSvenInt::GetAPIVersion()
{
	return { ( 4 ), ( 0 ) };
}

bool CSvenInt::Load( CreateInterfaceFn pfnSvenModFactory, void *pSvenModAPI, void *unused )
{
	gbLoadedAsSvenModPlugin = true;
	return true;
}

void CSvenInt::PostLoad( bool bGlobalLoad ) {}

void CSvenInt::Unload( void )
{
}

bool CSvenInt::Pause( void )
{
	return false;
}

void CSvenInt::Unpause( void ) {}

void CSvenInt::OnFirstClientdataReceived( void *pcldata, float flTime )
{

}

void CSvenInt::OnBeginLoading( void )
{

}

void CSvenInt::OnEndLoading( void )
{

}

void CSvenInt::OnDisconnect( void )
{

}

void CSvenInt::GameFrame( void *, double, bool ) {}

PLUGIN_RESULT CSvenInt::Draw( void )
{
	return PLUGIN_RESULT::PLUGIN_CONTINUE;
}

PLUGIN_RESULT CSvenInt::DrawHUD( float time, int intermission )
{
	return PLUGIN_RESULT::PLUGIN_CONTINUE;
}

const char *CSvenInt::GetName()
{
	return "SvenInt";
}

const char *CSvenInt::GetAuthor()
{
	return "Sw1ft / Reality / xWhitey / kolokola777";
}

const char *CSvenInt::GetVersion()
{
	return SVENINT_VERSION_STRING;
}

const char *CSvenInt::GetDescription()
{
	return "A mod for Sven Co-op that improves gameplay";
}

const char *CSvenInt::GetURL()
{
	return "https://github.com/sw1ft747/svenint";
}

const char *CSvenInt::GetDate()
{
	return __TIMESTAMP__;
}

const char *CSvenInt::GetLogTag()
{
	return "SvenInt";
}

InterfaceReg *InterfaceReg::s_pInterfaceRegs = NULL;

InterfaceReg::InterfaceReg( InstantiateInterfaceFn fn, const char *pName ) : m_pName( pName )
{
	m_CreateFn = fn;
	m_pNext = s_pInterfaceRegs;
	s_pInterfaceRegs = this;
}

extern "C"
#ifdef WIN32
__declspec( dllexport )
#else
__attribute__( ( visibility( "default" ) ) )
#endif
void *CreateInterface( const char *_pszName, int *_pReturnCode )
{
	InterfaceReg *pCur;

	for ( pCur = InterfaceReg::s_pInterfaceRegs; pCur; pCur = pCur->m_pNext )
	{
		if ( !strcmp( pCur->m_pName, _pszName ) )
		{
			if ( _pReturnCode )
				*_pReturnCode = IFACE_OK;

			return pCur->m_CreateFn();
		}
	}

	if ( _pReturnCode )
		*_pReturnCode = IFACE_FAILED;

	return NULL;
}
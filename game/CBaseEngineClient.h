// SvenInt (c) Sw1ft
// CBaseEngineClient.h

#ifndef SINT_IENGINECLIENT_H
#define SINT_IENGINECLIENT_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: abstract class CBaseEngineClient that provides
// compatibility with different SC versions
//-----------------------------------------------------------------------------

class CBaseEngineClient
{
public:
	virtual					~CBaseEngineClient() {}

	virtual void			*GetInstance( void ) { return 0; }
	virtual int				GetCompatibilityVersion( void ) { return 0; }

	virtual void			Con_DnPrintf( int pos, const char *pszMessage, ... ) { }
	virtual int				DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) { return 0; }
	virtual int				VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) { return 0; }
	virtual void			*LoadClientModel( const char *pszModelName ) { return 0; }
	virtual char			*NET_AdrToString( struct netadr_s *a, int n ) { return 0; } // arg #1 is referenced so inclusion of hlsdk_mini.h is not needed
	virtual int				NET_StringToAdr( char *s, struct netadr_s *a ) { return 0; }
	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) { return 0; }
	virtual void			ChangeSkymap( const char *pszSkyboxName ) { }
	virtual void			*ChangeSkycolor( float r, float g, float b ) { return 0; }
	virtual int				QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) { return 0; }
	virtual void			PushView( class RenderTarget *a1, bool a2, bool a3 ) { }
	virtual void			PopView( void ) { }
	virtual void			**RenderView( struct ref_params_s *pparams, bool a2, bool a3, int a4 ) { return 0; }
};

//-----------------------------------------------------------------------------
// Get CBaseEngineClient based on SC version
//-----------------------------------------------------------------------------

CBaseEngineClient *GetBaseEngineClient( void *pEngineClient, int iGameVersion );

//-----------------------------------------------------------------------------
// IEngineClient SC 5.22
//-----------------------------------------------------------------------------

class IEngineClient_5_22
{
public:
	virtual					~IEngineClient_5_22() {}

	virtual void			Con_DnPrintf( int pos, const char *pszMessage, ... ) = 0;
	virtual int				DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) = 0;
	virtual int				VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) = 0;
	virtual void			*LoadClientModel( const char *pszModelName ) = 0;
	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) = 0;
	virtual void			ChangeSkymap( const char *pszSkyboxName ) = 0;
	virtual int				QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) = 0;
};

//-----------------------------------------------------------------------------
// IEngineClient SC 5.23
//-----------------------------------------------------------------------------

class IEngineClient_5_23
{
public:
	virtual					~IEngineClient_5_23() {}

	virtual void			Con_DnPrintf( int pos, const char *pszMessage, ... ) = 0;
	virtual int				DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) = 0;
	virtual int				VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) = 0;
	virtual void			*LoadClientModel( const char *pszModelName ) = 0;
	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) = 0;
	virtual void			ChangeSkymap( const char *pszSkyboxName ) = 0;
	virtual void			*ChangeSkycolor( float r, float g, float b ) = 0;
	virtual int				QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) = 0;
	virtual void			PushView( class RenderTarget *a1, bool a2, bool a3 ) = 0;
	virtual void			PopView( void ) = 0;
	virtual void			**RenderView( struct ref_params_s *pparams, bool a2, bool a3, int a4 ) = 0;
};

//-----------------------------------------------------------------------------
// IEngineClient SC 5.26
//-----------------------------------------------------------------------------

class IEngineClient_5_26
{
public:
	virtual					~IEngineClient_5_26() {}

	virtual void			Con_DnPrintf( int pos, const char *pszMessage, ... ) = 0;
	virtual int				DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) = 0;
	virtual int				VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) = 0;
	virtual void			*LoadClientModel( const char *pszModelName ) = 0;
	virtual char			*NET_AdrToString( struct netadr_s a, int n) = 0;
	virtual int /* qboolean */ NET_StringToAdr( char *s, struct netadr_s *a ) = 0;
	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) = 0;
	virtual void			ChangeSkymap( const char *pszSkyboxName ) = 0;
	virtual void			*ChangeSkycolor( float r, float g, float b ) = 0;
	virtual int				QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) = 0;
	virtual void			PushView( class RenderTarget *a1, bool a2, bool a3 ) = 0;
	virtual void			PopView( void ) = 0;
	virtual void			**RenderView( struct ref_params_s *pparams, bool a2, bool a3, int a4 ) = 0;
};

#define ENGINECLIENT_INTERFACE_VERSION "SCEngineClient002" // Version '002' since 5.26

#endif // SINT_IENGINECLIENT_H

// SvenInt (c) Sw1ft
// CBaseEngineClient.cpp

#include "stdafx.h"
#include "CBaseEngineClient.h"

#include <stdarg.h>

//-----------------------------------------------------------------------------
// Dummy engine client for game versions older than 5.22
//-----------------------------------------------------------------------------

static CBaseEngineClient gEngineClientDummy;

//-----------------------------------------------------------------------------
// CEngineClient SC 5.22
//-----------------------------------------------------------------------------

class CEngineClient_5_22 final : public CBaseEngineClient
{
public:
	void Init( void *pInstance ) { m_pInstance = static_cast<IEngineClient_5_22 *>( pInstance ); }

	virtual void *GetInstance( void ) override { return m_pInstance; }
	virtual int GetCompatibilityVersion( void ) override { return 522; }

	virtual void Con_DnPrintf( int pos, const char *pszMessage, ... ) override
	{
		va_list va;
		va_start( va, pszMessage );

		m_pInstance->Con_DnPrintf( pos, pszMessage, va );

		va_end( va );
	}

	virtual int DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) override
	{
		return m_pInstance->DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual int VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) override
	{
		return m_pInstance->VGUI2_DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual void *LoadClientModel( const char *pszModelName ) override
	{
		return m_pInstance->LoadClientModel( pszModelName );
	}

	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) override
	{
		return m_pInstance->GetServerAddress( pAddress );
	}

	virtual void ChangeSkymap( const char *pszSkyboxName ) override
	{
		m_pInstance->ChangeSkymap( pszSkyboxName );
	}

	virtual int QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) override
	{
		return m_pInstance->QueueDemoSound( a1, a2, a3, a4, a5, a6, a7 );
	}

private:
	IEngineClient_5_22 *m_pInstance;
};

static CEngineClient_5_22 gEngineClient_5_22;

//-----------------------------------------------------------------------------
// CEngineClient SC 5.23
//-----------------------------------------------------------------------------

class CEngineClient_5_23 final : public CBaseEngineClient
{
public:
	void Init( void *pInstance ) { m_pInstance = static_cast<IEngineClient_5_23 *>( pInstance ); }

	virtual void *GetInstance( void ) override { return m_pInstance; }
	virtual int GetCompatibilityVersion( void ) override { return 523; }

	virtual void Con_DnPrintf( int pos, const char *pszMessage, ... ) override
	{
		va_list va;
		va_start( va, pszMessage );

		m_pInstance->Con_DnPrintf( pos, pszMessage, va );

		va_end( va );
	}

	virtual int DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) override
	{
		return m_pInstance->DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual int VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) override
	{
		return m_pInstance->VGUI2_DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual void *LoadClientModel( const char *pszModelName ) override
	{
		return m_pInstance->LoadClientModel( pszModelName );
	}

	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) override
	{
		return m_pInstance->GetServerAddress( pAddress );
	}

	virtual void ChangeSkymap( const char *pszSkyboxName ) override
	{
		m_pInstance->ChangeSkymap( pszSkyboxName );
	}

	virtual void *ChangeSkycolor( float r, float g, float b ) override
	{
		return m_pInstance->ChangeSkycolor( r, g, b );
	}

	virtual int QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) override
	{
		return m_pInstance->QueueDemoSound( a1, a2, a3, a4, a5, a6, a7 );
	}

	virtual void PushView( class RenderTarget *a1, bool a2, bool a3 ) override
	{
		m_pInstance->PushView( a1, a2, a3 );
	}

	virtual void PopView( void ) override
	{
		m_pInstance->PopView();
	}

	virtual void **RenderView( struct ref_params_s *pparams, bool a2, bool a3, int a4 ) override
	{
		return m_pInstance->RenderView( pparams, a2, a3, a4 );
	}

private:
	IEngineClient_5_23 *m_pInstance;
};

static CEngineClient_5_23 gEngineClient_5_23;

//-----------------------------------------------------------------------------
// CEngineClient SC 5.26
//-----------------------------------------------------------------------------

class CEngineClient_5_26 final : public CBaseEngineClient
{
public:
	void Init( void *pInstance ) { m_pInstance = static_cast<IEngineClient_5_26 *>( pInstance ); }

	virtual void *GetInstance( void ) override { return m_pInstance; }
	virtual int GetCompatibilityVersion( void ) override { return 526; }

	virtual void Con_DnPrintf( int pos, const char *pszMessage, ... ) override
	{
		va_list va;
		va_start( va, pszMessage );

		m_pInstance->Con_DnPrintf( pos, pszMessage, va );

		va_end( va );
	}

	virtual int DrawCharacterOpq( int x, int y, int number, int r, int g, int b, int unknown ) override
	{
		return m_pInstance->DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual int VGUI2_DrawCharacterOpq( int x, int y, int number, int r, int g, int b, unsigned int unknown ) override
	{
		return m_pInstance->VGUI2_DrawCharacterOpq( x, y, number, r, g, b, unknown );
	}

	virtual void *LoadClientModel( const char *pszModelName ) override
	{
		return m_pInstance->LoadClientModel( pszModelName );
	}

	virtual char *NET_AdrToString( struct netadr_s *a, int n ) override
	{
		return m_pInstance->NET_AdrToString( *a, n );
	}

	virtual int NET_StringToAdr( char *s, struct netadr_s *a ) override
	{
		return m_pInstance->NET_StringToAdr( s, a );
	}

	virtual struct netadr_s *GetServerAddress( struct netadr_s *pAddress ) override
	{
		return m_pInstance->GetServerAddress( pAddress );
	}

	virtual void ChangeSkymap( const char *pszSkyboxName ) override
	{
		m_pInstance->ChangeSkymap( pszSkyboxName );
	}

	virtual void *ChangeSkycolor( float r, float g, float b ) override
	{
		return m_pInstance->ChangeSkycolor( r, g, b );
	}

	virtual int QueueDemoSound( float *a1, int a2, const char *a3, float a4, float a5, int a6, int a7 ) override
	{
		return m_pInstance->QueueDemoSound( a1, a2, a3, a4, a5, a6, a7 );
	}

	virtual void PushView( class RenderTarget *a1, bool a2, bool a3 ) override
	{
		m_pInstance->PushView( a1, a2, a3 );
	}

	virtual void PopView( void ) override
	{
		m_pInstance->PopView();
	}

	virtual void **RenderView( struct ref_params_s *pparams, bool a2, bool a3, int a4 ) override
	{
		return m_pInstance->RenderView( pparams, a2, a3, a4 );
	}

private:
	IEngineClient_5_26 *m_pInstance;
};

static CEngineClient_5_26 gEngineClient_5_26;

//-----------------------------------------------------------------------------
// Get CBaseEngineClient based on SC version
//-----------------------------------------------------------------------------

CBaseEngineClient *GetBaseEngineClient( void *pEngineClient, int iGameVersion )
{
	if ( pEngineClient == NULL || iGameVersion == 0 )
		return &gEngineClientDummy;

	if ( iGameVersion >= SVEN_VERSION_CHECK( 5, 26, 0 ) )
	{
		gEngineClient_5_26.Init( pEngineClient );
		return &gEngineClient_5_26;
	}
	else if ( iGameVersion >= SVEN_VERSION_CHECK( 5, 23, 0 ) )
	{
		gEngineClient_5_23.Init( pEngineClient );
		return &gEngineClient_5_23;
	}
	else if ( iGameVersion == SVEN_VERSION_CHECK( 5, 22, 0 ) )
	{
		gEngineClient_5_22.Init( pEngineClient );
		return &gEngineClient_5_22;
	}
	
	return &gEngineClientDummy;
}
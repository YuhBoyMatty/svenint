// SvenInt (c) Sw1ft
// misc_demo_message.h

#ifndef SINT_FEATURE_DEMO_MESSAGE_H
#define SINT_FEATURE_DEMO_MESSAGE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// User messages
//-----------------------------------------------------------------------------

#define TYPE_USER_DEMOMSG ( -1 )

typedef enum
{
	kDemoMsgSegmentInfo = 0,
	kDemoMsgUndefined,
	kDemoMsgEdgePixelsPlayer
} EUserDemoMessage;

//-----------------------------------------------------------------------------
// Demo message feature
//-----------------------------------------------------------------------------

class CDemoMessage final : public CBaseFeature, IHookEventListener
{
public:
	CDemoMessage( const char *pszCategoryName, const char *pszName );

	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void WriteSegmentInfo( float flTime, const char *pszFormattedTimer, const char *pszMapname );
	void WriteEdgePixelsPlayer( const char *pszFilename, double width, double height, const Vector &vecPos, const Vector &vecAngles, int iDrawCalls );
	void WriteEdgePixelsPlayerStop( void );

private:
	bool ReadClientDLLMessage( int size, unsigned char *buffer );
};

EXTERN_FEATURE( CDemoMessage, demomessage );

#endif // SINT_FEATURE_DEMO_MESSAGE_H
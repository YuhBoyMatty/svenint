// SvenInt (c) Sw1ft
// misc_demo_capture.h

#ifndef SINT_FEATURE_DEMO_CAPTURE_H
#define SINT_FEATURE_DEMO_CAPTURE_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Demo capture feature
//-----------------------------------------------------------------------------

class CDemoCapture final : public CBaseFeature, IHookEventListener
{
public:
	CDemoCapture( const char *pszCategoryName, const char *pszName );

	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	bool Start( const char *pszFilename, double fps, double slowdown, double sampling_fps );
	bool Stop( void );

	void SaveImage( void );

	bool OpenPipe( void );
	bool ClosePipe( void );

	bool IsRecording( void ) const;

private:
	bool m_bRecording;
	bool m_bFirstCapture;
	int m_iCaptureFrameCount;
	int m_iFpsMultiplier;
	double m_lastRecordTime;
	double m_captureFps;
	double m_samplingFps;
	double m_fps;
	double m_frametime;

	std::string m_sFilename;

	int m_iWidth;
	int m_iHeight;
	int m_nPixelsBufferSize;
	char *m_pPixelsBuffer;

	HANDLE m_hReadPipe;
	HANDLE m_hWritePipe;
	PROCESS_INFORMATION m_pi;
};

EXTERN_FEATURE( CDemoCapture, democapture );

#endif // SINT_FEATURE_DEMO_CAPTURE_H
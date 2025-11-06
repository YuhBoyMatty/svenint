// SvenInt (c) Sw1ft
// misc_edge_pixels_player.h

#ifndef SINT_FEATURE_EDGE_PIXELS_PLAYER_H
#define SINT_FEATURE_EDGE_PIXELS_PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Edge pixels player feature
//-----------------------------------------------------------------------------

class CEdgePixelsPlayer final : public CBaseFeature, IHookEventListener
{
public:
	CEdgePixelsPlayer( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	bool Start( const char *pszFilename, double width, double height, const Vector &vecPos, const Vector &vecAngles, int iDrawCalls, bool bDemoPlayback = false );
	bool Stop( void );

private:
	bool m_bPlaying;
	bool m_bDemoPlayback;
	FILE *m_pFile;
	unsigned long m_ulFileSize;
	std::string m_sFilename;
	double m_width;
	double m_height;
	double m_frametime;
	double m_lastPlayed;
	Vector m_vecPos;
	Vector m_vecAngles;
	int m_iDrawCalls;
	std::vector<Vector> m_drawAngles;
	std::vector<std::vector<Vector>> m_drawAnglesQueue;
	cmd_function_t *npc_moveto;
};

EXTERN_FEATURE( CEdgePixelsPlayer, edgepixelsplayer );

#endif // SINT_FEATURE_EDGE_PIXELS_PLAYER_H
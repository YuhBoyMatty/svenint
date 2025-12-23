// SvenInt (c) Sw1ft
// misc_models_downloader.h

#ifndef SINT_FEATURE_MODELS_DOWNLOADER_H
#define SINT_FEATURE_MODELS_DOWNLOADER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"
#include "utils/hashdict.h"

//-----------------------------------------------------------------------------
// ModelsDownloadBuffer
//-----------------------------------------------------------------------------

struct ModelsDownloadBuffer
{
	char *memory;
	size_t size;
};

//-----------------------------------------------------------------------------
// wootguy's models database
//-----------------------------------------------------------------------------

struct wootguy_db_model
{
	uint32_t size;
	uint32_t polys;
	uint32_t repoid;
};

//-----------------------------------------------------------------------------
// Queued model to download
//-----------------------------------------------------------------------------

struct queued_model_download
{
	std::string modelname;
	std::future<bool> f;
};

//-----------------------------------------------------------------------------
// player_model_t
//-----------------------------------------------------------------------------

typedef struct
{
	char		name[ 260 ];
	char		modelname[ 260 ];
	model_t*	model;
} player_model_t;

//-----------------------------------------------------------------------------
// Player models downloader feature
//-----------------------------------------------------------------------------

class CModelsDownloader final : public CBaseFeature, IHookEventListener
{
public:
	CModelsDownloader( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	bool DownloadModels( void );
	void ParseModels( char *json );

	void SaveModel( char *data, size_t size, const char *name, const char *ext );
	bool DownloadModel( std::string sModelname, wootguy_db_model &modelinfo );
	bool IsMissingPlayerModel( const char *pszModelname );

private:
	//CMenuValueInteger *m_pMaxModelsDownload;
	CMenuValueInteger	*m_pMaxSize;
	CMenuValueInteger	*m_pMaxPolys;
	CMenuValueInteger	*m_pDownloadSpeed;
	CMenuValueInteger	*m_pTimeout;
	CMenuValueFloat		*m_pCheckInterval;

	float m_flNextThink;

	bool m_bDownloadedDatabase;
	std::future<bool> m_fModelsDatabase;

	CHashDict<wootguy_db_model> m_ModelsDatabase;
	std::vector<queued_model_download> m_QueuedModels;

	player_model_t (*DM_PlayerState)[ MAX_CLIENTS ];
};

EXTERN_FEATURE( CModelsDownloader, modelsdownloader );

#endif // SINT_FEATURE_MODELS_DOWNLOADER_H

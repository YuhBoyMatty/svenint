// SvenInt (c) Sw1ft
// Some of the logic was taken from hzqst's MetaHookSv plugin SCModelDownloader (c) hzqst
// https://github.com/hzqst/MetaHookSv/tree/main/Plugins/SCModelDownloader
// misc_models_downloader.cpp

#include "stdafx.h"
#include "misc_models_downloader.h"
#include "modules/server.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// curl
//-----------------------------------------------------------------------------

#define SINT_USE_GAME_CURL ( 1 )

#if SINT_USE_GAME_CURL
typedef void CURL;
typedef int CURLcode;

#define CURLE_OK 0
#define CURLOPT_URL 10002
#define CURLOPT_WRITEFUNCTION 20011
#define CURLOPT_WRITEDATA 10001
#define CURLOPT_TIMEOUT 13
#define CURLOPT_MAX_RECV_SPEED_LARGE 146
#define CURLOPT_USERAGENT 10018
#define CURLOPT_HTTP_VERSION 84L
#define CURL_HTTP_VERSION_1_1 2
#define CURLOPT_TRANSFER_ENCODING 209L
#define CURLOPT_FOLLOWLOCATION 52
#define CURLOPT_HEADER 42
#define CURLOPT_NOBODY 43

typedef CURL *( *curl_easy_init_t )( void );
typedef CURLcode( *curl_easy_setopt_t )( CURL *, int, ... );
typedef CURLcode( *curl_easy_perform_t )( CURL * );
typedef const char *( *curl_easy_strerror_t )( CURLcode );
typedef void ( *curl_easy_cleanup_t )( CURL * );

static curl_easy_init_t curl_easy_init = NULL;
static curl_easy_setopt_t curl_easy_setopt = NULL;
static curl_easy_perform_t curl_easy_perform = NULL;
static curl_easy_strerror_t curl_easy_strerror = NULL;
static curl_easy_cleanup_t curl_easy_cleanup = NULL;
#else
#include <curl/curl.h>
#endif

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CModelsDownloader, modelsdownloader, "Misc", "Models Downloader" );

//-----------------------------------------------------------------------------
// ModelsDownloadBufferWriteCallback
//-----------------------------------------------------------------------------

static size_t ModelsDownloadBufferStringWriteCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    size_t realsize = size * nmemb;
    ModelsDownloadBuffer *mem = (ModelsDownloadBuffer *)userp;

    char *ptr = (char *)MemRealloc( mem->memory, mem->size + realsize + 1 );
    if ( ptr == NULL )
        return 0;

    mem->memory = ptr;
    memcpy( &( mem->memory[ mem->size ] ), contents, realsize );
    mem->size += realsize;
    mem->memory[ mem->size ] = 0;

    return realsize;
}

static size_t ModelsDownloadBufferWriteCallback( void *contents, size_t size, size_t nmemb, void *userp )
{
    size_t realsize = size * nmemb;
    ModelsDownloadBuffer *mem = (ModelsDownloadBuffer *)userp;

    char *ptr = (char *)MemRealloc( mem->memory, mem->size + realsize );
    if ( ptr == NULL )
        return 0;

    mem->memory = ptr;
    memcpy( &( mem->memory[ mem->size ] ), contents, realsize );
    mem->size += realsize;

    return realsize;
}

//-----------------------------------------------------------------------------
// Get hash
//-----------------------------------------------------------------------------

static int SCModel_Hash( const char *name, size_t length )
{
    int hash = 0;

    for ( size_t i = 0; i < length; i++ )
    {
        char ch = (char)name[ i ];
        hash = ( ( hash << 5 ) - hash ) + ch;
        hash = hash % 15485863; // prevent hash ever increasing beyond 31 bits
    }

    return hash;
}

//-----------------------------------------------------------------------------
// Download models database
//-----------------------------------------------------------------------------

bool CModelsDownloader::DownloadModels( void )
{
    CURLcode res;
    CURL *curl = curl_easy_init();

    if ( curl == NULL )
        return false;

    ModelsDownloadBuffer chunk = { NULL, 0 };
    chunk.memory = (char *)MemAlloc( 1 );

    curl_easy_setopt( curl, CURLOPT_URL, "https://raw.githubusercontent.com/wootguy/pmodels/master/database/sc/models.json" );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, ModelsDownloadBufferStringWriteCallback );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk );
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, m_pTimeout->GetInt() );
    curl_easy_setopt( curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_pDownloadSpeed->GetInt() * 1024 );

    res = curl_easy_perform( curl );

    if ( res == CURLE_OK )
    {
        ParseModels( chunk.memory );
    }
    else
    {
        PrintWarning2( "Failed to download models database. Reason: %s\n", curl_easy_strerror( res ) );
    }

    curl_easy_cleanup( curl );
    MemFree( chunk.memory );

    return res == CURLE_OK;
}

//-----------------------------------------------------------------------------
// Parse JSON
//-----------------------------------------------------------------------------

void CModelsDownloader::ParseModels( char *json )
{
    char *ptr = json;
    int models = 0;
    int namesizesum = 0;

    while ( ( ptr = strstr( ptr, "\": {" ) ) != NULL )
    {
        char *key_end = ptr;
        char *key_start = key_end;

        while ( *( key_start - 1 ) != '"' && key_start > json )
            key_start--;

        int key_len = (int)( key_end - key_start );

        uint32_t polys = 0, size = 0;
        uint32_t date = 0;

        char *data_start = strstr( ptr, "{" );
        if ( data_start )
        {
        #pragma warning( push )
        #pragma warning( push )
        #pragma warning( disable : 6031 )
        #pragma warning( disable : 6387 )

            sscanf( strstr( data_start, "\"polys\":" ), "\"polys\": %u", &polys );
            sscanf( strstr( data_start, "\"size\":" ), "\"size\": %u", &size );
            sscanf( strstr( data_start, "\"date\":" ), "\"date\": %u", &date );

        #pragma warning( pop )
        #pragma warning( pop )
        }

        models++;
        namesizesum += key_len + 16;

        char origch = key_start[ key_len ];
        key_start[ key_len ] = '\0';

        m_ModelsDatabase.Insert( key_start, { size, polys, (uint32_t)( SCModel_Hash( key_start, key_len ) % 32 ) } );

        key_start[ key_len ] = origch;

        ptr += 4;
    }
}

//-----------------------------------------------------------------------------
// SaveModel
//-----------------------------------------------------------------------------

void CModelsDownloader::SaveModel( char *data, size_t size, const char *name, const char *ext )
{
    char path[ 260 ];
    snprintf( path, Q_ARRAYSIZE( path ), "models/player/%s", name );

    filesystem->CreateDirHierarchy( "models/player", "GAMEDOWNLOAD" );
    filesystem->CreateDirHierarchy( path, "GAMEDOWNLOAD" );

#if 1
    snprintf( path, Q_ARRAYSIZE( path ), "models/player/%s/%s%s", name, name, ext );

    FileHandle_t hFile = filesystem->Open( path, "wb", "GAMEDOWNLOAD" );
    if ( hFile )
    {
        filesystem->Write( data, size, hFile );
        filesystem->Close( hFile );
    }
#else
    snprintf( path, Q_ARRAYSIZE( path ), "svencoop_downloads/models/player/%s/%s%s", name, name, ext );
     
    FILE *pFile = fopen( path, "wb" );
    if ( pFile != NULL )
    {
        fwrite( data, 1, size, pFile );
        fclose( pFile );
    }
#endif
}

//-----------------------------------------------------------------------------
// DownloadModel
//-----------------------------------------------------------------------------

bool CModelsDownloader::DownloadModel( std::string sModelname, wootguy_db_model &modelinfo )
{
    char szUrl[ 260 ];

    CURLcode res;
    CURL *curl = curl_easy_init();

    if ( curl == NULL )
        return false;

    ModelsDownloadBuffer chunk = { NULL, 0 };
    chunk.memory = (char *)MemAlloc( 1 );

    snprintf( szUrl, Q_ARRAYSIZE( szUrl ),
              "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%s.mdl",
              modelinfo.repoid, sModelname.c_str(), sModelname.c_str() );

    curl_easy_setopt( curl, CURLOPT_URL, szUrl );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, ModelsDownloadBufferWriteCallback );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk );
    //curl_easy_setopt( curl, 145, 1L ); // CURLOPT_FOLLOWLOCATION
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, m_pTimeout->GetInt() );
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "ModelsDownloader/1.0" );
    curl_easy_setopt( curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 );
    curl_easy_setopt( curl, CURLOPT_TRANSFER_ENCODING, 0L );
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt( curl, CURLOPT_HEADER, 0L );
    curl_easy_setopt( curl, CURLOPT_NOBODY, 0L );
    curl_easy_setopt( curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_pDownloadSpeed->GetInt() * 1024 );

    res = curl_easy_perform( curl );

    if ( res == CURLE_OK )
    {
        SaveModel( chunk.memory, chunk.size, sModelname.c_str(), ".mdl" );
    }
    else
    {
        PrintWarning2( "Failed to download model \"%s\" (%s). Reason: %s\n", sModelname.c_str(), szUrl, curl_easy_strerror( res ) );
    }

    MemFree( chunk.memory );

    if ( res != CURLE_OK )
    {
        curl_easy_cleanup( curl );
        return false;
    }

    chunk = { NULL, 0 };
    chunk.memory = (char *)MemAlloc( 1 );

    snprintf( szUrl, Q_ARRAYSIZE( szUrl ),
              "https://wootdata.github.io/scmodels_data_%d/models/player/%s/%s.bmp",
              modelinfo.repoid, sModelname.c_str(), sModelname.c_str() );

    curl_easy_setopt( curl, CURLOPT_URL, szUrl );
    curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, ModelsDownloadBufferWriteCallback );
    curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk );
    //curl_easy_setopt( curl, 145, 1L ); // CURLOPT_FOLLOWLOCATION
    curl_easy_setopt( curl, CURLOPT_TIMEOUT, m_pTimeout->GetInt() );
    curl_easy_setopt( curl, CURLOPT_USERAGENT, "ModelsDownloader/1.0" );
    curl_easy_setopt( curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1 );
    curl_easy_setopt( curl, CURLOPT_TRANSFER_ENCODING, 0L );
    curl_easy_setopt( curl, CURLOPT_FOLLOWLOCATION, 1L );
    curl_easy_setopt( curl, CURLOPT_HEADER, 0L );
    curl_easy_setopt( curl, CURLOPT_NOBODY, 0L );
    curl_easy_setopt( curl, CURLOPT_MAX_RECV_SPEED_LARGE, m_pDownloadSpeed->GetInt() * 1024 );

    res = curl_easy_perform( curl );

    if ( res == CURLE_OK )
    {
        SaveModel( chunk.memory, chunk.size, sModelname.c_str(), ".bmp" );
    }
    else
    {
        PrintWarning2( "Failed to download model preview \"%s\" (%s). Reason: %s\n", sModelname.c_str(), szUrl, curl_easy_strerror( res ) );
    }

    curl_easy_cleanup( curl );
    MemFree( chunk.memory );

    return res == CURLE_OK;
}

//-----------------------------------------------------------------------------
// IsMissingPlayerModel
//-----------------------------------------------------------------------------

bool CModelsDownloader::IsMissingPlayerModel( const char *pszModelname )
{
    bool bMissingModel = true;
    FileFindHandle_t hFindHandle;

    const char *pszDirectoryName = filesystem->FindFirst( "models/player/*", &hFindHandle );
    while ( pszDirectoryName != NULL )
    {
        if ( filesystem->FindIsDirectory( hFindHandle ) && *pszDirectoryName != '.' )
        {
            if ( !stricmp( pszDirectoryName, pszModelname ) )
            {
                bMissingModel = false;
                break;
            }
        }

        pszDirectoryName = filesystem->FindNext( hFindHandle );
    }

    filesystem->FindClose( hFindHandle );
    return bMissingModel;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CModelsDownloader::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
    if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
    {
        m_flNextThink = -1.f;
        return kHookContinue;
    }

	// Host_FilterTime post event
    if ( !pEvent->GetReturn<qboolean>() )
        return kHookContinue;

    for ( size_t i = 0; i < m_QueuedModels.size(); i++ )
    {
        queued_model_download &dl = m_QueuedModels[ i ];
        if ( !dl.f.valid() || dl.f.wait_for( std::chrono::seconds( 0 ) ) != std::future_status::ready )
            continue;
        
        if ( dl.f.get() )
        {
            PrintMsg( "Downloaded model \"%s\"\n", dl.modelname.c_str() );

            if ( DM_PlayerState != NULL )
            {
                for ( int i = 0; i < cl_enginefuncs->GetMaxClients(); i++ )
                {
                    player_info_t *pPlayerInfo = enginestudio->PlayerInfo( i );
                    if ( pPlayerInfo == NULL )
                        continue;

                    char *pszModelname = pPlayerInfo->model;
                    if ( pszModelname[ 0 ] == '\0' )
                        continue;

                    if ( stricmp( pszModelname, dl.modelname.c_str() ) )
                        continue;

                    // Reset model
                    (*DM_PlayerState)[ i ].name[ 0 ] = '\0';
                    (*DM_PlayerState)[ i ].model = NULL;
                }
            }

            m_flNextThink = -1.f;
        }

        m_QueuedModels.erase( m_QueuedModels.begin() + i );
        i--;
    }

    if ( m_flNextThink > cl_enginefuncs->GetClientTime() )
        return kHookContinue;

    if ( !m_bDownloadedDatabase )
    {
        if ( !m_fModelsDatabase.valid() )
        {
            m_fModelsDatabase = std::async( [ this ] { return this->DownloadModels(); } );
        }
        else if ( m_fModelsDatabase.wait_for( std::chrono::seconds( 0 ) ) == std::future_status::ready )
        {
            m_bDownloadedDatabase = m_fModelsDatabase.get();

            if ( m_bDownloadedDatabase )
                PrintMsg( "Downloaded models database\n" );
        }
    }

    m_flNextThink = cl_enginefuncs->GetClientTime() + m_pCheckInterval->GetFloat();

    if ( cls->state != ca_active ||
         !m_bDownloadedDatabase ||
         !m_QueuedModels.empty() ||
         Modules::server->Host_IsServerActive() )
    {
        return kHookContinue;
    }

    for ( int i = 0; i < cl_enginefuncs->GetMaxClients(); i++ )
    {
        player_info_t *pPlayerInfo = enginestudio->PlayerInfo( i );
        if ( pPlayerInfo == NULL )
            continue;

        char *pszModelname = pPlayerInfo->model;
        if ( pszModelname[ 0 ] == '\0' || pszModelname[ 1 ] == '\0' )
            continue;

        wootguy_db_model *modeldb = m_ModelsDatabase.Find( pszModelname );
        if ( modeldb == NULL )
            continue;

        if ( modeldb->size / 1024 > (uint32_t)m_pMaxSize->GetInt() )
            continue;

        if ( modeldb->polys > (uint32_t)m_pMaxPolys->GetInt() )
            continue;

        if ( !IsMissingPlayerModel( pszModelname ) )
            continue;

        std::string sModelname = pszModelname;
        std::transform( sModelname.begin(), sModelname.end(), sModelname.begin(), []( unsigned char c ) { return std::tolower( c ); } );

        m_QueuedModels.push_back( { sModelname,
                                  std::async( [ this, sModelname, modeldb ] { return this->DownloadModel( sModelname, *modeldb ); } ) } );

        PrintMsg( "Started downloading model \"%s\" (repoid: %u) (size: %u KB) (polys: %u)\n", pszModelname, modeldb->repoid, modeldb->size / 1024, modeldb->polys );
        break;
    }

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CModelsDownloader::CModelsDownloader( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ),
                                                                                            m_ModelsDatabase( 2047 )
{
    SetInitiallyDisabled();

    //m_pMaxModelsDownload = NULL;
    m_pMaxSize = NULL;
    m_pMaxPolys = NULL;
    m_pDownloadSpeed = NULL;
    m_pTimeout = NULL;
    m_pCheckInterval = NULL;

    m_flNextThink = 0.f;
    m_bDownloadedDatabase = false;

    DM_PlayerState = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CModelsDownloader::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CModelsDownloader::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CModelsDownloader::Load( void )
{
	Modules::menu->BindFeature( this );

    //m_pMaxModelsDownload = Modules::menu->AddParamInteger( this, "MaxModelsDownload", "Max models to download in the background", 3, 0, 10 );
	m_pMaxSize = Modules::menu->AddParamInteger( this, "MaxSize", "Max size of a model to download (KB)", 5120, 0, 15360 );
	m_pMaxPolys = Modules::menu->AddParamInteger( this, "MaxPolys", "Max polygons of a model to download", 70000, 0, 200000 );
    m_pDownloadSpeed = Modules::menu->AddParamInteger( this, "DownloadSpeed", "Max download speed (KB)", 1024, 0, 10240 );
    m_pTimeout = Modules::menu->AddParamInteger( this, "Timeout", "Max timeout (s)", 30, 0, 300 );
    m_pCheckInterval = Modules::menu->AddParamFloat( this, "CheckInterval", "Check interval for missing player models", 30.f, 0.f, 60.f );

#if SINT_USE_GAME_CURL
#ifdef WIN32
    module_t hCurl = MemoryUtils()->GetModule( "libcurl.dll" );
#else
    module_t hCurl = MemoryUtils()->GetModule( "libcurl.so.4" );
#endif
    if ( hCurl == NULL )
    {
        PrintWarning( "Missing curl library to continue\n" );
        return false;
    }

    curl_easy_init = (curl_easy_init_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_init" );
    curl_easy_setopt = (curl_easy_setopt_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_setopt" );
    curl_easy_perform = (curl_easy_perform_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_perform" );
    curl_easy_strerror = (curl_easy_strerror_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_strerror" );
    curl_easy_cleanup = (curl_easy_cleanup_t)MemoryUtils()->GetProcAddress( hCurl, "curl_easy_cleanup" );

    if ( curl_easy_init == NULL || curl_easy_setopt == NULL ||
         curl_easy_perform == NULL || curl_easy_strerror == NULL ||
         curl_easy_cleanup == NULL )
    {
        PrintWarning( "Failed to get required curl functions\n" );
        return false;
    }
#endif

    if ( gamedata->Initialized() && gamedata->PreferRVA() )
    {
        DM_PlayerState = (decltype( DM_PlayerState ))( gamedata->FindRVA( GameData::Modules::Engine, "Engine", "DM_PlayerState" ) );
    }
    else
    {
    #ifdef WIN32
        ud_t inst;
        MemoryUtils()->InitDisasm( &inst, enginestudio->SetupPlayerModel, 32, 32 );

        while ( MemoryUtils()->Disassemble( &inst ) )
        {
            if ( inst.mnemonic == UD_Iadd &&
                 inst.operand[ 0 ].type == UD_OP_REG &&
                 inst.operand[ 1 ].type == UD_OP_IMM )
            {
                DM_PlayerState = (decltype( DM_PlayerState ))( inst.operand[ 1 ].lval.udword );
                break;
            }
        }

        FEATURE_CHECK_SYMBOL( DM_PlayerState, "DM_PlayerState" );
    #endif
    }

    if ( DM_PlayerState == NULL )
        PrintWarning2( "Instant load of downloaded models is unavailable\n" );

    GAMEDATA_DUMP_FILE_OFFSET( "DM_PlayerState", DM_PlayerState, GameData::Modules::Engine );
	return true;
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CModelsDownloader::Unload( void )
{
    /*
    while ( m_fModelsDatabase.valid() )
    {
        auto status = m_fModelsDatabase.wait_for( std::chrono::seconds( 1 ) );
        if ( status == std::future_status::ready )
        {
            m_fModelsDatabase.get();
            break;
        }
    }

    for ( const queued_model_download &dl : m_QueuedModels )
    {
        while ( dl.f.valid() )
        {
            auto status = dl.f.wait_for( std::chrono::seconds( 1 ) );
            if ( status == std::future_status::ready )
                break;
        }
    }
    */
}

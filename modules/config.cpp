// SvenInt (c) Sw1ft
// config.cpp

#include "stdafx.h"
#include "config.h"
#include "menu.h"
#include "svenint.h"
#include "features/base_feature.h"

#ifndef WIN32
#include <algorithm>
#endif

namespace Modules { static CConfigModule configModule; CConfigModule *config = &configModule; }

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define INI_BUFFER_LENGTH		( 256 )
#define INI_COMMENT_PREFIX		( ";#" )
#define INI_SECTION_PREFIX		( '[' )
#define INI_SECTION_POSTFIX		( ']' )
#define INI_PARAMETER_DELIMITER ( "=" )
#define INI_STRIP_CHARS			( " \t\n" )
#define INI_STRIP_CHARS_LEN		( sizeof( INI_STRIP_CHARS ) - 1 )
#define INI_COMMENT_PREFIX_LEN	( sizeof( INI_COMMENT_PREFIX ) - 1 )

//-----------------------------------------------------------------------------
// Enumerations
//-----------------------------------------------------------------------------

typedef enum
{
	kCfgParseOK = 0,
	kCfgParseMissingFile,
	kCfgParseErrorSectionStart,
	kCfgParseErrorSectionEnd,
	kCfgParseErrorSectionEmpty,
	kCfgParseErrorEmptyKey,
	kCfgParseErrorEmptyValue
} ECfgParseState;

typedef enum
{
	kCfgNone = 0,
	kCfgExport,
	kCfgImport
} ECfgState;

//-----------------------------------------------------------------------------
// Radix conversion
//-----------------------------------------------------------------------------

static constexpr const char *tetrades_table[ 16 ] =
{
	"0000", "0001", "0010", "0011",
	"0100", "0101", "0110", "0111",
	"1000", "1001", "1010", "1011",
	"1100", "1101", "1110", "1111",
};

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_load_config, "Load a config file" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszFileName = args[ 1 ];

		const char *pszExtension = NULL;
		const char *buffer = pszFileName;

		while ( *buffer )
		{
			if ( *buffer == '.' )
				pszExtension = buffer;

			buffer++;
		}

		if ( pszExtension && stricmp( pszExtension, ".ini" ) )
		{
			Msg( "sc_load_config: expected name of the file with \".ini\" extension\n" );
			return;
		}

		std::string sPrevConfig;
		std::string sFileName = args[ 1 ];

		if ( !pszExtension )
			sFileName += ".ini";

		Modules::config->Load( sFileName.c_str() );
	}
	else
	{
		Modules::config->Load( "default.ini" );
	}
}

CON_COMMAND( sc_save_config, "Save a config file" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszFileName = args[ 1 ];

		const char *pszExtension = NULL;
		const char *buffer = pszFileName;

		while ( *buffer )
		{
			if ( *buffer == '.' )
				pszExtension = buffer;

			buffer++;
		}

		if ( pszExtension && stricmp( pszExtension, ".ini" ) )
		{
			Msg( "sc_save_config: expected name of the file with \".ini\" extension\n" );
			return;
		}

		std::string sPrevConfig;
		std::string sFileName = args[ 1 ];

		if ( !pszExtension )
			sFileName += ".ini";

		Modules::config->Save( sFileName.c_str() );
	}
	else
	{
		Modules::config->Save( "default.ini" );
	}
}

CON_COMMAND( sc_load_shader_config, "Load a shader config file" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszFileName = args[ 1 ];

		const char *pszExtension = NULL;
		const char *buffer = pszFileName;

		while ( *buffer )
		{
			if ( *buffer == '.' )
				pszExtension = buffer;

			buffer++;
		}

		if ( pszExtension && stricmp( pszExtension, ".ini" ) )
		{
			Msg( "sc_load_shader_config: expected name of the file with \".ini\" extension\n" );
			return;
		}

		std::string sPrevConfig;
		std::string sFileName = args[ 1 ];

		if ( !pszExtension )
			sFileName += ".ini";

		Modules::config->LoadShader( sFileName.c_str() );
	}
	else
	{
		ConMsg( "Usage:  sc_load_shader_config <filename>\n" );
	}
}

CON_COMMAND( sc_save_shader_config, "Save a shader config file" )
{
	if ( args.ArgC() > 1 )
	{
		const char *pszFileName = args[ 1 ];

		const char *pszExtension = NULL;
		const char *buffer = pszFileName;

		while ( *buffer )
		{
			if ( *buffer == '.' )
				pszExtension = buffer;

			buffer++;
		}

		if ( pszExtension && stricmp( pszExtension, ".ini" ) )
		{
			Msg( "sc_save_shader_config: expected name of the file with \".ini\" extension\n" );
			return;
		}

		std::string sPrevConfig;
		std::string sFileName = args[ 1 ];

		if ( !pszExtension )
			sFileName += ".ini";

		Modules::config->SaveShader( sFileName.c_str() );
	}
	else
	{
		ConMsg( "Usage:  sc_save_shader_config <filename>\n" );
	}
}

CON_COMMAND( sc_config_set_param, "Sets a config parameter" )
{
	if ( args.ArgC() > 3 )
	{
		const char *pszSection = args[ 1 ];
		const char *pszProperty = args[ 2 ];
		const char *pszValue = args[ 3 ];

		if ( !Modules::config->SetParam( pszSection, pszProperty, pszValue, false ) )
			Warning( "[SvenInt::Config] Specified parameter \"%s\" in section \"%s\" doesn't exist\n", pszProperty, pszSection );
	}
	else
	{
		ConMsg( "Usage:  sc_config_set_param <section> <param> <value>\n" );
	}
}

CON_COMMAND( sc_config_toggle_param, "Toggles a config parameter" )
{
	if ( args.ArgC() > 2 )
	{
		const char *pszSection = args[ 1 ];
		const char *pszProperty = args[ 2 ];

		if ( !Modules::config->SetParam( pszSection, pszProperty, NULL, true ) )
			Warning( "[SvenInt::Config] Specified parameter \"%s\" in section \"%s\" doesn't exist\n", pszProperty, pszSection );
	}
	else
	{
		ConMsg( "Usage:  sc_config_toggle_param <section> <param>\n" );
	}
}

//-----------------------------------------------------------------------------
// Config property
//-----------------------------------------------------------------------------

CConfigProperty::CConfigProperty( const char *pszPropName, int iPropType, void *pDefaultValue )
{
	m_pszPropName = pszPropName;
	m_iPropType = iPropType;
	m_iRadix = 10;

	memset( &m_value, 0, sizeof( m_value ) );
	memset( &m_defaultValue, 0, sizeof( m_defaultValue ) );

	if ( m_iPropType == kCfgPropertyCString )
	{
		m_value.m_cstring = (char *)MemCalloc( sizeof( char ), CFG_PROPERTY_CSTRING_SIZE );

		if ( m_value.m_cstring == NULL )
		{
			Assert( m_value.m_cstring != NULL );
			return;
		}
	}

	if ( pDefaultValue == NULL )
		return;

	switch ( m_iPropType )
	{
	case kCfgPropertyInteger:
		m_value.m_int = m_defaultValue.m_int = *(int *)pDefaultValue;
		break;
		
	case kCfgPropertyUInteger:
		m_value.m_uint = m_defaultValue.m_uint = *(unsigned int *)pDefaultValue;
		break;

	case kCfgPropertyFloat:
		m_value.m_float = m_defaultValue.m_float = *(float *)pDefaultValue;
		break;

	case kCfgPropertyBoolean:
		m_value.m_boolean = m_defaultValue.m_boolean = *(bool *)pDefaultValue;
		break;

	case kCfgPropertyCString:
		strncpy( m_value.m_cstring, (char *)pDefaultValue, CFG_PROPERTY_CSTRING_SIZE - 1 );
		m_defaultValue.m_cstring = (char *)pDefaultValue; // this is dangerous!!
		break;

	case kCfgPropertyColorRGB:
		m_value.m_color[ 0 ] = m_defaultValue.m_color[ 0 ] = ( (float *)pDefaultValue )[ 0 ];
		m_value.m_color[ 1 ] = m_defaultValue.m_color[ 1 ] = ( (float *)pDefaultValue )[ 1 ];
		m_value.m_color[ 2 ] = m_defaultValue.m_color[ 2 ] = ( (float *)pDefaultValue )[ 2 ];
		break;

	case kCfgPropertyColorRGBA:
		m_value.m_color[ 0 ] = m_defaultValue.m_color[ 0 ] = ( (float *)pDefaultValue )[ 0 ];
		m_value.m_color[ 1 ] = m_defaultValue.m_color[ 1 ] = ( (float *)pDefaultValue )[ 1 ];
		m_value.m_color[ 2 ] = m_defaultValue.m_color[ 2 ] = ( (float *)pDefaultValue )[ 2 ];
		m_value.m_color[ 3 ] = m_defaultValue.m_color[ 3 ] = ( (float *)pDefaultValue )[ 3 ];
		break;

	case kCfgPropertyVector:
		m_value.m_vector[ 0 ] = m_defaultValue.m_vector[ 0 ] = ( (float *)pDefaultValue )[ 0 ];
		m_value.m_vector[ 1 ] = m_defaultValue.m_vector[ 1 ] = ( (float *)pDefaultValue )[ 1 ];
		m_value.m_vector[ 2 ] = m_defaultValue.m_vector[ 2 ] = ( (float *)pDefaultValue )[ 2 ];
		break;
	}
}

CConfigProperty::~CConfigProperty()
{
	if ( m_iPropType == kCfgPropertyCString )
	{
		if ( m_value.m_cstring != NULL )
		{
			MemFree( m_value.m_cstring );
			m_value.m_cstring = NULL;
		}

		m_defaultValue.m_cstring = NULL;
	}
}

void CConfigProperty::Revert( void )
{
	static_assert( sizeof( m_value ) == sizeof( m_defaultValue ), "Different sizes" );

	if ( m_iPropType == kCfgPropertyCString )
	{
		if ( m_value.m_cstring != NULL && m_defaultValue.m_cstring != NULL )
			strncpy( m_value.m_cstring, m_defaultValue.m_cstring, CFG_PROPERTY_CSTRING_SIZE - 1 );
	}
	else
	{
		memcpy( &m_value, &m_defaultValue, sizeof( m_value ) );
	}
}

void CConfigProperty::CopyStringFrom( const char *pszStr )
{
	if ( pszStr == NULL )
		return;

	if ( m_value.m_cstring == NULL )
		return;

	if ( m_iPropType != kCfgPropertyCString )
		return;

	if ( pszStr[ 0 ] == '\0' )
		m_value.m_cstring[ 0 ] = '\0';

	strncpy( m_value.m_cstring, (char *)pszStr, CFG_PROPERTY_CSTRING_SIZE - 1 );
}

//-----------------------------------------------------------------------------
// Module constructor / destructor
//-----------------------------------------------------------------------------

CConfigModule::CConfigModule() : m_Config( 31 ),
								m_ShadersConfig( 15 ),
								m_ConfigTable( 1023, m_Functor, m_Functor ),
								m_SectionsTable( 255, m_Functor, m_Functor )
{
	m_hConfigFile = NULL;
	m_pszCurrentSection = NULL;
	m_iRadix = 10;

	m_bSectionsDuped = false;
	m_bSectionsFilled = false;

	m_State = kCfgNone;
	m_ParseState = kCfgParseOK;
	m_iLastErrorLine = 0;
	m_pszLastErrorMessage = "";
}

CConfigModule::~CConfigModule()
{
	//Shutdown();
}

//-----------------------------------------------------------------------------
// Config listener
//-----------------------------------------------------------------------------

void CConfigModule::RegisterListener( IConfigListener *pListener )
{
	auto it = std::find( m_listeners.begin(), m_listeners.end(), pListener );
	if ( it != m_listeners.end() )
		return;

	m_listeners.push_back( pListener );
}

void CConfigModule::UnregisterListener( IConfigListener *pListener )
{
	auto it = std::find( m_listeners.begin(), m_listeners.end(), pListener );
	if ( it == m_listeners.end() )
		return;

	m_listeners.erase( it );
}

//-----------------------------------------------------------------------------
// Load a config file
//-----------------------------------------------------------------------------

bool CConfigModule::Load( const char *pszFilename )
{
	std::string sPath = SVENINT_MAKE_PATH( SVENINT_CONFIG_FOLDER );
	sPath += pszFilename;

	if ( LoadEx( m_Config, sPath ) )
	{
		Msg( "[SvenInt::Config] Loaded config \"%s\"\n", pszFilename );

		for ( IConfigListener *pListener : m_listeners )
		{
			pListener->OnConfigLoad( pszFilename, false );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Save a config file
//-----------------------------------------------------------------------------

bool CConfigModule::Save( const char *pszFilename )
{
	std::string sPath = SVENINT_MAKE_PATH( SVENINT_CONFIG_FOLDER );
	sPath += pszFilename;

	if ( SaveEx( m_Config, sPath ) )
	{
		Msg( "[SvenInt::Config] Saved config \"%s\"\n", pszFilename );

		for ( IConfigListener *pListener : m_listeners )
		{
			pListener->OnConfigSave( pszFilename, false );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Load a shader config file
//-----------------------------------------------------------------------------

bool CConfigModule::LoadShader( const char *pszFilename )
{
	std::string sPath = SVENINT_MAKE_PATH( SVENINT_SHADERS_CONFIG_FOLDER );
	sPath += pszFilename;

	if ( LoadEx( m_ShadersConfig, sPath ) )
	{
		Msg( "[SvenInt::Config] Loaded shader config \"%s\"\n", pszFilename );

		for ( IConfigListener *pListener : m_listeners )
		{
			pListener->OnConfigLoad( pszFilename, true );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Save a shader config file
//-----------------------------------------------------------------------------

bool CConfigModule::SaveShader( const char *pszFilename )
{
	std::string sPath = SVENINT_MAKE_PATH( SVENINT_SHADERS_CONFIG_FOLDER );
	sPath += pszFilename;

	if ( SaveEx( m_ShadersConfig, sPath ) )
	{
		Msg( "[SvenInt::Config] Saved shader config \"%s\"\n", pszFilename );

		for ( IConfigListener *pListener : m_listeners )
		{
			pListener->OnConfigSave( pszFilename, true );
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Add a property
//-----------------------------------------------------------------------------

CConfigProperty *CConfigModule::AddProperty( const char *pszSectionName, const char *pszPropertyName, int iPropType, void *pDefaultValue )
{
	std::vector<CConfigProperty *> *props = m_Config.Find( pszSectionName );
	if ( props == NULL )
	{
		props = m_Config.Insert( pszSectionName, std::vector<CConfigProperty *>() );
	}

	CConfigProperty *prop = new( MemAlloc( sizeof( CConfigProperty ) ) ) CConfigProperty( pszPropertyName, iPropType, pDefaultValue );
	//CConfigProperty *prop = new CConfigProperty( pszPropertyName, iPropType, pDefaultValue );
	props->push_back( prop );

	return prop;
}

//-----------------------------------------------------------------------------
// Add a shader property
//-----------------------------------------------------------------------------

CConfigProperty *CConfigModule::AddShadersProperty( const char *pszShaderName, const char *pszPropertyName, int iPropType, void *pDefaultValue )
{
	std::vector<CConfigProperty *> *props = m_ShadersConfig.Find( pszShaderName );
	if ( props == NULL )
	{
		props = m_ShadersConfig.Insert( pszShaderName, std::vector<CConfigProperty *>() );
	}

	CConfigProperty *prop = new( MemAlloc( sizeof( CConfigProperty ) ) ) CConfigProperty( pszPropertyName, iPropType, pDefaultValue );
	//CConfigProperty *prop = new CConfigProperty( pszPropertyName, iPropType, pDefaultValue );
	props->push_back( prop );

	return prop;
}

//-----------------------------------------------------------------------------
// Find a property
//-----------------------------------------------------------------------------

CConfigProperty *CConfigModule::FindProperty( const char *pszSectionName, const char *pszPropertyName )
{
	std::vector<CConfigProperty *> *props = m_Config.Find( pszSectionName );
	if ( props == NULL )
		return NULL;

	for ( size_t i = 0; i < props->size(); i++ )
	{
		if ( !stricmp( pszPropertyName, props->at( i )->GetName() ) )
			return props->at( i );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Find a shader property
//-----------------------------------------------------------------------------

CConfigProperty *CConfigModule::FindShadersProperty( const char *pszShaderName, const char *pszPropertyName )
{
	std::vector<CConfigProperty *> *props = m_ShadersConfig.Find( pszShaderName );
	if ( props == NULL )
		return NULL;

	for ( size_t i = 0; i < props->size(); i++ )
	{
		if ( !stricmp( pszPropertyName, props->at( i )->GetName() ) )
			return props->at( i );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Revert properties of given section
//-----------------------------------------------------------------------------

void CConfigModule::RevertSectionProperties( const char *pszSectionName )
{
	std::vector<CConfigProperty *> *props = m_Config.Find( pszSectionName );
	if ( props == NULL )
		return;

	for ( CConfigProperty *prop : *props )
	{
		prop->Revert();
	}
}

//-----------------------------------------------------------------------------
// Revert properties of given shader section
//-----------------------------------------------------------------------------

void CConfigModule::RevertShaderSectionProperties( const char *pszSectionName )
{
	std::vector<CConfigProperty *> *props = m_ShadersConfig.Find( pszSectionName );
	if ( props == NULL )
		return;

	for ( CConfigProperty *prop : *props )
	{
		prop->Revert();
	}
}

//-----------------------------------------------------------------------------
// Load a config
//-----------------------------------------------------------------------------

bool CConfigModule::LoadEx( CHashDict<std::vector<CConfigProperty *>> &cfg, const std::string &sPath )
{
	const char *buffer;
	std::string clrBuffer;

	if ( !BeginImport( sPath.c_str() ) )
		return false;

	for ( int i = 0; i < cfg.Count(); i++ )
	{
		HashIterator_t it = cfg.First( i );

		while ( cfg.IsValidIterator( it ) )
		{
			const char *pszSection = cfg.KeyAt( i, it );
			std::vector<CConfigProperty *> &props = cfg.ValueAt( i, it );

			if ( !BeginSectionImport( pszSection ) )
			{
				it = cfg.Next( i, it );
				continue;
			}

			for ( size_t i = 0; i < props.size(); i++ )
			{
				CConfigProperty *prop = props[ i ];

				switch ( prop->GetType() )
				{
				case kCfgPropertyInteger:
					SetConversionRadix( prop->GetRadix() );
					ImportParam( prop->GetName(), prop->GetIntRef() );
					ResetRadix();
					break;
					
				case kCfgPropertyUInteger:
					SetConversionRadix( prop->GetRadix() );
					ImportParam( prop->GetName(), prop->GetUIntRef() );
					ResetRadix();
					break;

				case kCfgPropertyFloat:
					ImportParam( prop->GetName(), prop->GetFloatRef() );
					break;

				case kCfgPropertyBoolean:
					ImportParam( prop->GetName(), prop->GetBoolRef() );
					break;

				case kCfgPropertyCString:
					if ( ImportParam( prop->GetName(), &buffer ) )
					{
						if ( buffer == NULL )
							break;

						if ( buffer[ 0 ] != '\0' )
							prop->CopyStringFrom( buffer );
						MemFree( (void *)buffer );
					}
					break;

				case kCfgPropertyColorRGB:
				case kCfgPropertyColorRGBA:
					clrBuffer = prop->GetName();

					clrBuffer += "_R";
					ImportParam( clrBuffer.c_str(), prop->GetColor() + 0 );

					clrBuffer.back() = 'G';
					ImportParam( clrBuffer.c_str(), prop->GetColor() + 1 );

					clrBuffer.back() = 'B';
					ImportParam( clrBuffer.c_str(), prop->GetColor() + 2 );

					if ( prop->GetType() == kCfgPropertyColorRGBA )
					{
						clrBuffer.back() = 'A';
						ImportParam( clrBuffer.c_str(), prop->GetColor() + 3 );
					}
					break;

				case kCfgPropertyVector:
					clrBuffer = prop->GetName();

					clrBuffer += "_X";
					ImportParam( clrBuffer.c_str(), prop->GetVector() + 0 );

					clrBuffer.back() = 'Y';
					ImportParam( clrBuffer.c_str(), prop->GetVector() + 1 );

					clrBuffer.back() = 'Z';
					ImportParam( clrBuffer.c_str(), prop->GetVector() + 2 );
					break;
				}
			}

			EndSectionImport();
			it = cfg.Next( i, it );
		}
	}

	EndImport();
	return true;
}

//-----------------------------------------------------------------------------
// Save a config
//-----------------------------------------------------------------------------

bool CConfigModule::SaveEx( CHashDict<std::vector<CConfigProperty *>> &cfg, const std::string &sPath )
{
	std::string clrBuffer;

	if ( !BeginExport( sPath.c_str() ) )
		return false;

	for ( int i = 0; i < cfg.Count(); i++ )
	{
		HashIterator_t it = cfg.First( i );

		while ( cfg.IsValidIterator( it ) )
		{
			const char *pszSection = cfg.KeyAt( i, it );
			std::vector<CConfigProperty *> &props = cfg.ValueAt( i, it );

			if ( !BeginSectionExport( pszSection ) )
			{
				it = cfg.Next( i, it );
				continue;
			}

			for ( size_t i = 0; i < props.size(); i++ )
			{
				CConfigProperty *prop = props[ i ];

				switch ( prop->GetType() )
				{
				case kCfgPropertyInteger:
					SetConversionRadix( prop->GetRadix() );
					ExportParam( prop->GetName(), prop->GetIntRef() );
					ResetRadix();
					break;
					
				case kCfgPropertyUInteger:
					SetConversionRadix( prop->GetRadix() );
					ExportParam( prop->GetName(), prop->GetUIntRef() );
					ResetRadix();
					break;

				case kCfgPropertyFloat:
					ExportParam( prop->GetName(), prop->GetFloatRef() );
					break;

				case kCfgPropertyBoolean:
					ExportParam( prop->GetName(), prop->GetBoolRef() );
					break;

				case kCfgPropertyCString:
					if ( prop->GetCString() != NULL && prop->GetCString()[ 0 ] != '\0' )
						ExportParam( prop->GetName(), const_cast<const char *>( prop->GetCString() ) );
					break;

				case kCfgPropertyColorRGB:
				case kCfgPropertyColorRGBA:
					clrBuffer = prop->GetName();

					clrBuffer += "_R";
					ExportParam( clrBuffer.c_str(), prop->GetColor() + 0);

					clrBuffer.back() = 'G';
					ExportParam( clrBuffer.c_str(), prop->GetColor() + 1 );

					clrBuffer.back() = 'B';
					ExportParam( clrBuffer.c_str(), prop->GetColor() + 2 );

					if ( prop->GetType() == kCfgPropertyColorRGBA )
					{
						clrBuffer.back() = 'A';
						ExportParam( clrBuffer.c_str(), prop->GetColor() + 3 );
					}
					break;

				case kCfgPropertyVector:
					clrBuffer = prop->GetName();

					clrBuffer += "_X";
					ExportParam( clrBuffer.c_str(), prop->GetVector() + 0);

					clrBuffer.back() = 'Y';
					ExportParam( clrBuffer.c_str(), prop->GetVector() + 1 );

					clrBuffer.back() = 'Z';
					ExportParam( clrBuffer.c_str(), prop->GetVector() + 2 );
					break;
				}
			}

			EndSectionExport();
			it = cfg.Next( i, it );
		}
	}

	EndExport();
	return true;
}

//-----------------------------------------------------------------------------
// Set a config parameter
//-----------------------------------------------------------------------------

bool CConfigModule::SetParam( const char *pszSection, const char *pszProperty, const char *pszValue, bool bToggle )
{
	if ( pszSection != NULL )
	{
		pszSection = MemStrdup( pszSection );

		char *c = (char *)pszSection;
		while ( *c )
		{
			if ( *c == '_' ) // 'Key_Spam' -> 'Key Spam'
				*c = ' ';

			c++;
		}
	}

	std::vector<CConfigProperty *> *props = m_Config.Find( pszSection );
	if ( props == NULL || !bToggle && pszValue == NULL )
	{
		if ( pszSection != NULL )
			MemFree( (void *)pszSection );

		return false;
	}

	for ( size_t i = 0; i < props->size(); i++ )
	{
		CConfigProperty *prop = props->at( i );
		if ( stricmp( pszProperty, prop->GetName() ) )
			continue;

		switch ( prop->GetType() )
		{
		case kCfgPropertyInteger:
			*prop = bToggle ? ( prop->GetInt() ? 0 : 1 ) : atoi( pszValue );
			break;

		case kCfgPropertyUInteger:
		{
			unsigned int val = (unsigned int)strtoul( pszValue, NULL, prop->GetRadix() );
			*prop = bToggle ? ( prop->GetInt() ? 0 : 1 ) : val;
			break;
		}

		case kCfgPropertyFloat:
			*prop = bToggle ? ( prop->GetFloat() == 0.f ? 1.f : 0.f ) : (float)atof( pszValue );
			break;

		case kCfgPropertyBoolean:
		{
			const bool bPrevValue = prop->GetBool();

			if ( !bToggle )
			{
				if ( !stricmp( pszValue, "true" ) )
					*prop = true;
				else if ( !stricmp( pszValue, "false" ) )
					*prop = false;
				else
					*prop = bool( !!atoi( pszValue ) );
			}
			else
			{
				*prop = !prop->GetBool();
			}

			if ( bPrevValue == prop->GetBool() )
				break;

			if ( !stricmp( prop->GetName(), "Enable" ) )
			{
				for ( CMenuCategory &category : Modules::menu->m_categories )
				{
					bool bFoundCategory = false;

					for ( CMenuFeature &feature : category.m_features )
					{
						if ( !feature.m_bToggleable ||
							 feature.m_pCfgEnabled == NULL ||
							 stricmp( feature.m_pFeature->GetName(), pszSection ) )
						{
							continue;
						}

						feature.m_pFeature->Toggle();
						bFoundCategory = true;
					}

					if ( bFoundCategory )
						break;
				}
			}

			break;
		}

		case kCfgPropertyCString:
			if ( bToggle )
				break;

			prop->CopyStringFrom( pszValue );
			break;

		case kCfgPropertyColorRGB:
		case kCfgPropertyColorRGBA:
		{
			if ( bToggle )
				break;

			float *pflColor = prop->GetColor();

			unsigned char nRGBA[ 4 ];
			int nParamsRead = sscanf( pszValue, "%hhu %hhu %hhu %hhu", nRGBA, nRGBA + 1, nRGBA + 2, nRGBA + 3 );

			if ( nParamsRead >= 3 )
			{
				pflColor[ 0 ] = (float)nRGBA[ 0 ] / 255.f;
				pflColor[ 1 ] = (float)nRGBA[ 1 ] / 255.f;
				pflColor[ 2 ] = (float)nRGBA[ 2 ] / 255.f;
				if ( prop->GetType() == kCfgPropertyColorRGBA && nParamsRead >= 4 )
					pflColor[ 3 ] = (float)nRGBA[ 3 ] / 255.f;
			}
			else if ( nParamsRead == 2 )
			{
				pflColor[ 0 ] = (float)nRGBA[ 0 ] / 255.f;
				pflColor[ 1 ] = (float)nRGBA[ 1 ] / 255.f;
			}
			else if ( nParamsRead == 1 )
			{
				pflColor[ 0 ] = (float)nRGBA[ 0 ] / 255.f;
			}

			break;
		}

		case kCfgPropertyVector:
		{
			if ( bToggle )
				break;

			float *pVec = prop->GetVector();

			float vec[ 3 ];
			int nParamsRead = sscanf( pszValue, "%f %f %f", vec, vec + 1, vec + 2 );

			if ( nParamsRead >= 3 )
			{
				pVec[ 0 ] = vec[ 0 ];
				pVec[ 1 ] = vec[ 1 ];
				pVec[ 2 ] = vec[ 2 ];
			}
			else if ( nParamsRead == 2 )
			{
				pVec[ 0 ] = vec[ 0 ];
				pVec[ 1 ] = vec[ 1 ];
			}
			else if ( nParamsRead == 1 )
			{
				pVec[ 0 ] = vec[ 0 ];
			}

			break;
		}
		}

		if ( pszSection != NULL )
			MemFree( (void *)pszSection );

		return true;
	}

	if ( pszSection != NULL )
		MemFree( (void *)pszSection );

	return false;
}

//-----------------------------------------------------------------------------
// Clear up configs
//-----------------------------------------------------------------------------

void CConfigModule::Clear( void )
{
	for ( int i = 0; i < m_Config.Count(); i++ )
	{
		HashIterator_t it = m_Config.First( i );

		while ( m_Config.IsValidIterator( it ) )
		{
			std::vector<CConfigProperty *> &props = m_Config.ValueAt( i, it );
			for ( size_t i = 0; i < props.size(); i++ )
			{
				props[ i ]->~CConfigProperty();
				MemFree( props[ i ] );
				//delete props[ i ];
			}
			props.clear();

			it = m_Config.Next( i, it );
		}
	}

	m_Config.Clear();

	for ( int i = 0; i < m_ShadersConfig.Count(); i++ )
	{
		HashIterator_t it = m_ShadersConfig.First( i );

		while ( m_ShadersConfig.IsValidIterator( it ) )
		{
			std::vector<CConfigProperty *> &props = m_ShadersConfig.ValueAt( i, it );
			for ( size_t i = 0; i < props.size(); i++ )
			{
				props[ i ]->~CConfigProperty();
				MemFree( props[ i ] );
				//delete props[ i ];
			}
			props.clear();

			it = m_ShadersConfig.Next( i, it );
		}
	}

	m_ShadersConfig.Clear();
}

//-----------------------------------------------------------------------------
// INI files export / import
//-----------------------------------------------------------------------------

int CConfigModule::GetHashTableSize()
{
	return m_ConfigTable.Count();
}

bool CConfigModule::ReinitializeHashTable( int iBuckets )
{
	if ( m_State == kCfgNone )
	{
		Assert( iBuckets > 2 );

		ClearConfigTable();
		ClearSectionsTable();

		m_ConfigTable.Resize( iBuckets );
		return true;
	}

	return false;
}

void CConfigModule::SetConversionRadix( int radix )
{
	Assert( radix == 2 || radix == 8 || radix == 10 || radix == 16 );

	m_iRadix = radix;
}

void CConfigModule::ResetRadix( void )
{
	m_iRadix = 10;
}

int CConfigModule::GetLastState( void )
{
	return m_ParseState;
}

const char *CConfigModule::GetLastErrorMessage( void )
{
	return m_pszLastErrorMessage;
}

int CConfigModule::GetLastErrorLine( void )
{
	return m_iLastErrorLine;
}

//-----------------------------------------------------------------------------
// CConfigModule Import
//-----------------------------------------------------------------------------

bool CConfigModule::BeginImport( const char *pszFilePath )
{
	if ( m_State == kCfgNone )
	{
		m_bSectionsDuped = true;
		m_bSectionsFilled = true;

		if ( !ParseFile( pszFilePath ) )
		{
			if ( GetLastState() == kCfgParseMissingFile )
			{
				Warning( "[SvenInt] Missing the config file \"./%s\" to import\n", pszFilePath );
			}
			else
			{
				Warning( "[SvenInt] Failed to export the config file \"./%s\". Reason: %s (%d)\n", pszFilePath, GetLastErrorMessage(), GetLastErrorLine() );
			}

			ClearConfigTable();
			ClearSectionsTable();

			m_bSectionsDuped = false;

			return false;
		}

		m_pszCurrentSection = NULL;
		m_State = kCfgImport;

		return true;
	}

	return false;
}

void CConfigModule::EndImport( void )
{
	if ( m_State == kCfgImport )
	{
		ClearConfigTable();
		ClearSectionsTable();

		m_bSectionsDuped = false;

		m_pszCurrentSection = NULL;
		m_State = kCfgNone;
	}
}

bool CConfigModule::BeginSectionImport( const char *pszSectionName )
{
	if ( m_State != kCfgImport || m_pszCurrentSection || !m_SectionsTable.Find( pszSectionName ) )
		return false;

	m_pszCurrentSection = pszSectionName;
	return true;
}

void CConfigModule::EndSectionImport( void )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		m_pszCurrentSection = NULL;
	}
}

bool CConfigModule::ImportParam( const char *pszPropertyName, const char **value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			*value = MemStrdup( pszValue );
			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, short *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = (short)atoi( pszValue );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (short)strtol( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (short)strtol( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (short)strtol( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (short)strtol( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, int *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = atoi( pszValue );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (int)strtol( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (int)strtol( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (int)strtol( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (int)strtol( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, int64 *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = (int64)atoll( pszValue );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (int64)strtoll( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (int64)strtoll( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (int64)strtoll( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (int64)strtoll( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, uint16 *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = (uint16)strtoul( pszValue, NULL, 10 );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (uint16)strtoul( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (uint16)strtoul( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (uint16)strtoul( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (uint16)strtoul( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, uint32 *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = (uint32)strtoul( pszValue, NULL, 10 );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (uint32)strtoul( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (uint32)strtoul( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (uint32)strtoul( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (uint32)strtoul( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, uint64 *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( m_iRadix == 10 )
			{
				*value = (uint64)strtoull( pszValue, NULL, 10 );
			}
			//else if ( m_iRadix == 16 )
			//{
			//	*value = (uint64)strtoull( pszValue, NULL, 16 );
			//}
			//else if ( m_iRadix == 8 )
			//{
			//	*value = (uint64)strtoull( pszValue, NULL, 8 );
			//}
			//else if ( m_iRadix == 2 )
			//{
			//	*value = (uint64)strtoull( pszValue, NULL, 2 );
			//}
			else
			{
				*value = (uint64)strtoull( pszValue, NULL, m_iRadix );
			}

			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, float *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			*value = static_cast<float>( atof( pszValue ) );
			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, double *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			*value = atof( pszValue );
			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, char *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			*value = (char)atoi( pszValue );
			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, unsigned char *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			*value = (unsigned char)strtoul( pszValue, NULL, 10 );
			return true;
		}
	}

	return false;
}

bool CConfigModule::ImportParam( const char *pszPropertyName, bool *value )
{
	if ( m_State == kCfgImport && m_pszCurrentSection )
	{
		const char *pszValue = FindValueFromConfig( m_pszCurrentSection, pszPropertyName );

		if ( pszValue )
		{
			if ( !stricmp( pszValue, "true" ) )
			{
				*value = true;
			}
			else if ( !stricmp( pszValue, "false" ) )
			{
				*value = false;
			}
			else
			{
				*value = static_cast<bool>( atoi( pszValue ) );
			}

			return true;
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// CConfigModule Export
//-----------------------------------------------------------------------------

bool CConfigModule::BeginExport( const char *pszFilePath )
{
	if ( m_State == kCfgNone )
	{
		m_hConfigFile = fopen( pszFilePath, "w" );

		if ( !m_hConfigFile )
		{
			Warning( "[SvenInt] Unable to create/write the config file \"./%s\" for export\n", pszFilePath );
			return false;
		}

		ClearSectionsTable();

		m_bSectionsDuped = false;

		m_pszCurrentSection = NULL;
		m_State = kCfgExport;

		return true;
	}

	return false;
}

void CConfigModule::EndExport( void )
{
	if ( m_State == kCfgExport )
	{
		ClearSectionsTable();
		fclose( m_hConfigFile );

		m_bSectionsDuped = false;

		m_pszCurrentSection = NULL;
		m_hConfigFile = NULL;

		m_State = kCfgNone;
	}
}

bool CConfigModule::BeginSectionExport( const char *pszSectionName )
{
	if ( m_State != kCfgExport || m_pszCurrentSection || m_SectionsTable.Find( pszSectionName ) )
		return false;

	m_bSectionsFilled = true;

	m_SectionsTable.Insert( pszSectionName );
	m_pszCurrentSection = pszSectionName;

	fprintf( m_hConfigFile, "[%s]\n", pszSectionName );

	return true;
}

void CConfigModule::EndSectionExport( void )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "\n" );
		m_pszCurrentSection = NULL;
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, const char *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %s\n", pszPropertyName, value );
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, short *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %hi\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%hiX\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%hio\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			fprintf( m_hConfigFile, "%s = %s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %hi\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, int *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %d\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%X\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%o\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			// XD
			fprintf( m_hConfigFile, "%s = %s%s%s%s%s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 28 ) & 0x0F ],
					 tetrades_table[ ( *value >> 24 ) & 0x0F ],
					 tetrades_table[ ( *value >> 20 ) & 0x0F ],
					 tetrades_table[ ( *value >> 16 ) & 0x0F ],
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %d\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, int64 *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %lli\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%llX\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%llo\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			// lmao XD
			// is that actually needed??
			fprintf( m_hConfigFile, "%s = %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 60 ) & 0x0F ],
					 tetrades_table[ ( *value >> 56 ) & 0x0F ],
					 tetrades_table[ ( *value >> 52 ) & 0x0F ],
					 tetrades_table[ ( *value >> 48 ) & 0x0F ],
					 tetrades_table[ ( *value >> 44 ) & 0x0F ],
					 tetrades_table[ ( *value >> 40 ) & 0x0F ],
					 tetrades_table[ ( *value >> 36 ) & 0x0F ],
					 tetrades_table[ ( *value >> 32 ) & 0x0F ],
					 tetrades_table[ ( *value >> 28 ) & 0x0F ],
					 tetrades_table[ ( *value >> 24 ) & 0x0F ],
					 tetrades_table[ ( *value >> 20 ) & 0x0F ],
					 tetrades_table[ ( *value >> 16 ) & 0x0F ],
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %lli\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, uint16 *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %hu\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%huX\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%huo\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			fprintf( m_hConfigFile, "%s = %s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %hu\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, uint32 *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %u\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%X\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%o\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			fprintf( m_hConfigFile, "%s = %s%s%s%s%s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 28 ) & 0x0F ],
					 tetrades_table[ ( *value >> 24 ) & 0x0F ],
					 tetrades_table[ ( *value >> 20 ) & 0x0F ],
					 tetrades_table[ ( *value >> 16 ) & 0x0F ],
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %u\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, uint64 *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		if ( m_iRadix == 10 )
		{
			fprintf( m_hConfigFile, "%s = %llu\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 16 )
		{
			fprintf( m_hConfigFile, "%s = 0x%llX\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 8 )
		{
			fprintf( m_hConfigFile, "%s = 0%llo\n", pszPropertyName, *value );
		}
		else if ( m_iRadix == 2 )
		{
			fprintf( m_hConfigFile, "%s = %s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n", pszPropertyName,
					 tetrades_table[ ( *value >> 60 ) & 0x0F ],
					 tetrades_table[ ( *value >> 56 ) & 0x0F ],
					 tetrades_table[ ( *value >> 52 ) & 0x0F ],
					 tetrades_table[ ( *value >> 48 ) & 0x0F ],
					 tetrades_table[ ( *value >> 44 ) & 0x0F ],
					 tetrades_table[ ( *value >> 40 ) & 0x0F ],
					 tetrades_table[ ( *value >> 36 ) & 0x0F ],
					 tetrades_table[ ( *value >> 32 ) & 0x0F ],
					 tetrades_table[ ( *value >> 28 ) & 0x0F ],
					 tetrades_table[ ( *value >> 24 ) & 0x0F ],
					 tetrades_table[ ( *value >> 20 ) & 0x0F ],
					 tetrades_table[ ( *value >> 16 ) & 0x0F ],
					 tetrades_table[ ( *value >> 12 ) & 0x0F ],
					 tetrades_table[ ( *value >> 8 ) & 0x0F ],
					 tetrades_table[ ( *value >> 4 ) & 0x0F ],
					 tetrades_table[ *value & 0x0F ] );
		}
		else
		{
			fprintf( m_hConfigFile, "%s = %llu\n", pszPropertyName, *value );
		}
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, float *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %.6f\n", pszPropertyName, *value );
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, double *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %lf\n", pszPropertyName, *value );
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, char *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %c\n", pszPropertyName, *value );
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, unsigned char *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %hhu\n", pszPropertyName, *value );
	}
}

void CConfigModule::ExportParam( const char *pszPropertyName, bool *value )
{
	if ( m_State == kCfgExport && m_pszCurrentSection )
	{
		fprintf( m_hConfigFile, "%s = %s\n", pszPropertyName, ( *value ? "true" : "false" ) );
	}
}

//-----------------------------------------------------------------------------
// INI files parser
//-----------------------------------------------------------------------------

void CConfigModule::ClearConfigTable()
{
	for ( int i = 0; i < m_ConfigTable.Count(); i++ )
	{
		HashIterator_t it = m_ConfigTable.First( i );

		while ( m_ConfigTable.IsValidIterator( it ) )
		{
			ConfigPair_t &pair = m_ConfigTable.At( i, it );

		#pragma warning(push)
		#pragma warning(disable : 6001)

			MemFree( (void *)pair.property );

		#pragma warning(pop)

			it = m_ConfigTable.Next( i, it );
		}
	}

	m_ConfigTable.Clear();
}

void CConfigModule::ClearSectionsTable()
{
	if ( m_bSectionsFilled )
	{
		if ( m_bSectionsDuped )
		{
			for ( int i = 0; i < m_SectionsTable.Count(); i++ )
			{
				HashIterator_t it = m_SectionsTable.First( i );

				while ( m_SectionsTable.IsValidIterator( it ) )
				{
					auto section = m_SectionsTable.At( i, it );

					MemFree( (void *)section );

					it = m_SectionsTable.Next( i, it );
				}
			}
		}

		m_SectionsTable.Clear();
		m_bSectionsFilled = false;
	}
}

const char *CConfigModule::FindValueFromConfig( const char *pszSection, const char *pszProperty )
{
	ConfigPair_t find_pair =
	{
		pszSection,
		pszProperty,
		NULL
	};

	ConfigPair_t *pair = m_ConfigTable.Find( find_pair );

	if ( pair )
	{
		return pair->value;
	}

	return NULL;
}

bool CConfigModule::ParseFile( const char *pszFilePath )
{
	FILE *file = fopen( pszFilePath, "r" );

	if ( file )
	{
		int line = 0;
		bool bParsingSection = true;

		int iBufferSize = INI_BUFFER_LENGTH;
		char *pszFileBuffer = NULL;

		if ( !pszFileBuffer )
			pszFileBuffer = (char *)MemAlloc( iBufferSize );

		long int endpos;
		fseek( file, 0, SEEK_END );
		endpos = ftell( file );
		rewind( file );

		const char *pszSection = NULL;

		// Read line by line
		while ( fgets( pszFileBuffer, iBufferSize, file ) )
		{
			size_t length = strlen( pszFileBuffer );

			// Increase buffer size
			while ( pszFileBuffer[ length - 1 ] != '\n' && ftell( file ) != endpos )
			{
				iBufferSize *= 2;

				void *realloc_mem = MemRealloc( pszFileBuffer, iBufferSize );

				if ( !realloc_mem )
				{
					fclose( file );
					return false;
				}

				pszFileBuffer = (char *)realloc_mem;
				fgets( pszFileBuffer + length, iBufferSize - length, file );

				length = strlen( pszFileBuffer );
			}

			++line;

			// Strip string from spaces and comments
			char *str = ParseUtil_LStrip( pszFileBuffer );
			ParseUtil_RemoveComment( str );
			ParseUtil_RStrip( str );

			// Nothing here, skip
			if ( !*str )
				continue;

			// Parsing section
			if ( bParsingSection )
			{
			L_PARSE_SECTION:
				if ( *str == INI_SECTION_PREFIX )
				{
					char *end = str + strlen( str ) - 1;

					// Exclude prefix of section
					++str;

					if ( *end == INI_SECTION_POSTFIX )
					{
						// Exclude postfix of section
						*end = '\0';

						// Strip
						str = ParseUtil_Strip( str );

						if ( *str )
						{
							// Next try to parse parameters
							bParsingSection = false;

							const char **section_item = m_SectionsTable.Find( str );

							if ( section_item )
							{
								// Already in the table
								pszSection = *section_item;
							}
							else
							{
								// Dupe and save name of section
								pszSection = MemStrdup( str );
								m_SectionsTable.Insert( pszSection );
							}
						}
						else
						{
							m_pszLastErrorMessage = "section name is empty";
							m_ParseState = kCfgParseErrorSectionEmpty;
							m_iLastErrorLine = line;
							return false;
						}
					}
					else
					{
						m_pszLastErrorMessage = "expected end-of-section identifier";
						m_ParseState = kCfgParseErrorSectionEnd;
						m_iLastErrorLine = line;
						return false;
					}
				}
				else
				{
					m_pszLastErrorMessage = "expected identifier of a section";
					m_ParseState = kCfgParseErrorSectionStart;
					m_iLastErrorLine = line;
					return false;
				}
			}
			else
			{
				// Found prefix of section, start parsing a new one
				if ( *str == INI_SECTION_PREFIX )
				{
					bParsingSection = true;
					goto L_PARSE_SECTION;
				}

				// Split parameter
				char *key = strtok( str, INI_PARAMETER_DELIMITER );

				if ( !key || !*key )
				{
					m_pszLastErrorMessage = "key of a parameter is empty";
					m_ParseState = kCfgParseErrorEmptyKey;
					m_iLastErrorLine = line;
					return false;
				}

				char *value = strtok( NULL, INI_PARAMETER_DELIMITER );

				if ( !value || !*value )
				{
					m_pszLastErrorMessage = "value of a parameter is empty";
					m_ParseState = kCfgParseErrorEmptyValue;
					m_iLastErrorLine = line;
					return false;
				}

				// Strip
				ParseUtil_RStrip( key );
				value = ParseUtil_Strip( value );

				// Add entry
				const char *pszKey = (const char *)MemAlloc( strlen( key ) + strlen( value ) + 2 );

				if ( !pszKey )
				{
					Warning( "[SvenInt] CConfigModule::ParseFile: cannot allocate memory\n" );
					continue;
				}

				const char *pszValue = pszKey + strlen( key ) + 1;

				memcpy( (void *)pszKey, key, strlen( key ) + 1 );
				memcpy( (void *)pszValue, value, strlen( value ) + 1 );

				m_ConfigTable.Insert( { pszSection, pszKey, pszValue } );
			}
		}

		MemFree( pszFileBuffer );
		fclose( file );

		m_ParseState = kCfgParseOK;
		m_iLastErrorLine = -1;

		return true;
	}
	else
	{
		m_ParseState = kCfgParseMissingFile;
		return false;
	}

	return false;
}

bool CConfigModule::ParseUtil_ContainsChars( char ch, const char *chars, size_t length )
{
	for ( size_t i = 0; i < length; ++i )
	{
		if ( chars[ i ] == ch )
			return true;
	}

	return false;
}

char *CConfigModule::ParseUtil_LStrip( char *str )
{
	while ( *str && ParseUtil_ContainsChars( *str, INI_STRIP_CHARS, INI_STRIP_CHARS_LEN ) )
		++str;

	return str;
}

void CConfigModule::ParseUtil_RStrip( char *str )
{
	char *end = str + strlen( str ) - 1;

	if ( end < str )
		return;

	while ( end >= str && ParseUtil_ContainsChars( *end, INI_STRIP_CHARS, INI_STRIP_CHARS_LEN ) )
	{
		*end = '\0';
		--end;
	}
}

char *CConfigModule::ParseUtil_Strip( char *str )
{
	char *result = ParseUtil_LStrip( str );
	ParseUtil_RStrip( result );
	return result;
}

void CConfigModule::ParseUtil_RemoveComment( char *str )
{
	while ( *str && !ParseUtil_ContainsChars( *str, INI_COMMENT_PREFIX, INI_COMMENT_PREFIX_LEN ) )
		++str;

	if ( *str )
		*str = '\0';
}

//-----------------------------------------------------------------------------
// Init module
//-----------------------------------------------------------------------------

bool CConfigModule::Init( void )
{
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_load_config ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_load_shader_config ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_save_config ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_save_shader_config ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_config_set_param ) );
	Globals::cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_config_toggle_param ) );

	return true;
}

//-----------------------------------------------------------------------------
// Shutdown module
//-----------------------------------------------------------------------------

void CConfigModule::Shutdown( void )
{
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_load_config ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_load_shader_config ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_save_config ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_save_shader_config ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_config_set_param ) );
	Globals::cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_config_toggle_param ) );

	Clear();
	m_listeners.clear();

	m_Config.Purge();
	m_ShadersConfig.Purge();

	ClearConfigTable();
	ClearSectionsTable();

	m_ConfigTable.Purge();
	m_SectionsTable.Purge();
}

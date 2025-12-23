// SvenInt (c) Sw1ft
// client.h

#ifndef SINT_CONFIG_MODULE_H
#define SINT_CONFIG_MODULE_H

#ifdef _WIN32
#pragma once
#endif

#include "utils/hash.h"
#include "utils/hashdict.h"

//-----------------------------------------------------------------------------
// Property types
//-----------------------------------------------------------------------------

#define CFG_PROPERTY_CSTRING_SIZE ( 128 )

typedef enum
{
	kCfgPropertyInteger = 0,
	kCfgPropertyUInteger,
	kCfgPropertyFloat,
	kCfgPropertyBoolean,
	kCfgPropertyCString,
	kCfgPropertyColorRGB,
	kCfgPropertyColorRGBA,
	kCfgPropertyVector,
} eCfgPropertyType;

//-----------------------------------------------------------------------------
// Field types
//-----------------------------------------------------------------------------

#define CONFIG_DECLARE_DEDUCE_FIELDTYPE( fieldType, type ) template<> struct ConfigFieldTypeDeducer<type> { enum { CONFIG_FIELD_TYPE = fieldType }; };
#define ConfigDeduceFieldType(T) (int)ConfigFieldTypeDeducer<T>::CONFIG_FIELD_TYPE

template <typename T> struct ConfigFieldTypeDeducer {};
template <typename T>
#ifdef _WIN32
FORCEINLINE
#else
__attribute__((always_inline))
#endif
constexpr int ConfigDeduceValueFieldType( T value ) { return ConfigFieldTypeDeducer<T>::CONFIG_FIELD_TYPE; }

CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyInteger, int );
CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyUInteger, unsigned int );
CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyFloat, float );
CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyBoolean, bool );
CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyCString, const char * );
CONFIG_DECLARE_DEDUCE_FIELDTYPE( kCfgPropertyCString, char * );

//-----------------------------------------------------------------------------
// Config property
//-----------------------------------------------------------------------------

class CConfigProperty
{
public:
	CConfigProperty( const char *pszPropName, int iPropType, void *pDefaultValue );
	~CConfigProperty();

	// Reset to default value, string is ignored
	void Revert( void );
	void CopyStringFrom( const char *pszStr );

	void inline operator=( int val ) { m_value.m_int = val; }
	void inline operator=( unsigned int val ) { m_value.m_int = val; }
	void inline operator=( float val ) { m_value.m_float = val; }
	void inline operator=( bool val ) { m_value.m_boolean = val; }

	inline const char *GetName( void ) const { return m_pszPropName; }
	inline int GetType( void ) const { return m_iPropType; }
	inline int GetRadix( void ) const { return m_iRadix; }
	inline void SetRadix( int radix ) { m_iRadix = radix; }

	inline int *GetIntRef( void ) { return &m_value.m_int; }
	inline unsigned int *GetUIntRef( void ) { return &m_value.m_uint; }
	inline float *GetFloatRef( void ) { return &m_value.m_float; }
	inline bool *GetBoolRef( void ) { return &m_value.m_boolean; }
	inline char **GetCStringRef( void ) { return &m_value.m_cstring; }

	inline int GetInt( void ) { return m_value.m_int; }
	inline int GetUInt( void ) { return m_value.m_uint; }
	inline float GetFloat( void ) { return m_value.m_float; }
	inline bool GetBool( void ) { return m_value.m_boolean; }
	inline char *GetCString( void ) { return m_value.m_cstring; }
	inline float *GetColor( void ) { return m_value.m_color; }
	inline float *GetVector( void ) { return m_value.m_vector; }

	inline int GetCStringSize( void ) const { return CFG_PROPERTY_CSTRING_SIZE; }

private:
	const char *m_pszPropName;
	int m_iPropType : 16;
	int m_iRadix : 16;

	union
	{
		int m_int;
		unsigned int m_uint;
		float m_float;
		bool m_boolean;
		char *m_cstring;
		float m_color[ 4 ];
		float m_vector[ 3 ];
	} m_value;

	union
	{
		int m_int;
		unsigned int m_uint;
		float m_float;
		bool m_boolean;
		float m_color[ 4 ];
		float m_vector[ 3 ];
	} m_defaultValue;
};

//-----------------------------------------------------------------------------
// IConfigListener
//-----------------------------------------------------------------------------

class IConfigListener
{
public:
	virtual ~IConfigListener() {}

	virtual void OnConfigLoad( const char *pszFilename, bool bShaderConfig ) = 0;
	virtual void OnConfigSave( const char *pszFilename, bool bShaderConfig ) = 0;
};

//-----------------------------------------------------------------------------
// Config module
//-----------------------------------------------------------------------------

class CConfigModule
{
public:
	CConfigModule();
	~CConfigModule();

	bool				Init( void );
	void				Shutdown( void );

public:
	void				RegisterListener( IConfigListener *pListener );
	void				UnregisterListener( IConfigListener *pListener );

	bool				Load( const char *pszFilename );
	bool				LoadShader( const char *pszFilename );

	bool				Save( const char *pszFilename );
	bool				SaveShader( const char *pszFilename );

	CConfigProperty		*AddProperty( const char *pszSectionName, const char *pszPropertyName, int iPropType, void *pDefaultValue );
	CConfigProperty		*AddShadersProperty( const char *pszShaderName, const char *pszPropertyName, int iPropType, void *pDefaultValue );
	
	CConfigProperty		*FindProperty( const char *pszSectionName, const char *pszPropertyName );
	CConfigProperty		*FindShadersProperty( const char *pszShaderName, const char *pszPropertyName );

	template <class T>
	inline CConfigProperty *AddProperty( const char *pszSectionName, const char *pszPropertyName, T defaultValue )
	{
		return AddProperty( pszSectionName, pszPropertyName, ConfigDeduceFieldType( T ), &defaultValue );
	}
	inline CConfigProperty *AddProperty( const char *pszSectionName, const char *pszPropertyName, const char *defaultValue )
	{
		return AddProperty( pszSectionName, pszPropertyName, kCfgPropertyCString, (void *)defaultValue );
	}
	inline CConfigProperty *AddProperty( const char *pszSectionName, const char *pszPropertyName, char *defaultValue )
	{
		return AddProperty( pszSectionName, pszPropertyName, kCfgPropertyCString, (void *)defaultValue );
	}
	
	template <class T>
	inline CConfigProperty *AddShadersProperty( const char *pszSectionName, const char *pszPropertyName, T defaultValue )
	{
		return AddShadersProperty( pszSectionName, pszPropertyName, ConfigDeduceFieldType( T ), &defaultValue );
	}

	void				RevertSectionProperties( const char *pszSectionName );
	void				RevertShaderSectionProperties( const char *pszSectionName );

	bool				SetParam( const char *pszSection, const char *pszProperty, const char *pszValue, bool bToggle );
	
private:
	bool				LoadEx( CHashDict<std::vector<CConfigProperty *>> &cfg, const std::string &sPath );
	bool				SaveEx( CHashDict<std::vector<CConfigProperty *>> &cfg, const std::string &sPath );
	void				Clear( void );

public:
	//-----------------------------------------------------------------------------
	// Utility
	//-----------------------------------------------------------------------------

	int					GetHashTableSize();
	bool				ReinitializeHashTable( int iBuckets );

	void				SetConversionRadix( int radix );
	void				ResetRadix( void );

	int					GetLastState( void );
	const char			*GetLastErrorMessage( void );
	int					GetLastErrorLine( void );

	//-----------------------------------------------------------------------------
	// Import
	//-----------------------------------------------------------------------------

	bool				BeginImport( const char *pszFilePath );
	void				EndImport( void );

	bool				BeginSectionImport( const char *pszSectionName );
	void				EndSectionImport( void );

	bool				ImportParam( const char *pszPropertyName, const char **value );
	bool				ImportParam( const char *pszPropertyName, short *value );
	bool				ImportParam( const char *pszPropertyName, int *value );
	bool				ImportParam( const char *pszPropertyName, int64 *value );
	bool				ImportParam( const char *pszPropertyName, uint16 *value );
	bool				ImportParam( const char *pszPropertyName, uint32 *value );
	bool				ImportParam( const char *pszPropertyName, uint64 *value );
	bool				ImportParam( const char *pszPropertyName, float *value );
	bool				ImportParam( const char *pszPropertyName, double *value );
	bool				ImportParam( const char *pszPropertyName, char *value );
	bool				ImportParam( const char *pszPropertyName, unsigned char *value );
	bool				ImportParam( const char *pszPropertyName, bool *value );

	//-----------------------------------------------------------------------------
	// Export
	//-----------------------------------------------------------------------------

	bool				BeginExport( const char *pszFilePath );
	void				EndExport( void );

	bool				BeginSectionExport( const char *pszSectionName );
	void				EndSectionExport( void );

	void				ExportParam( const char *pszPropertyName, const char *value );
	void				ExportParam( const char *pszPropertyName, short *value );
	void				ExportParam( const char *pszPropertyName, int *value );
	void				ExportParam( const char *pszPropertyName, int64 *value );
	void				ExportParam( const char *pszPropertyName, uint16 *value );
	void				ExportParam( const char *pszPropertyName, uint32 *value );
	void				ExportParam( const char *pszPropertyName, uint64 *value );
	void				ExportParam( const char *pszPropertyName, float *value );
	void				ExportParam( const char *pszPropertyName, double *value );
	void				ExportParam( const char *pszPropertyName, char *value );
	void				ExportParam( const char *pszPropertyName, unsigned char *value );
	void				ExportParam( const char *pszPropertyName, bool *value );

private:
	// INI files parser
	void				ClearConfigTable();
	void				ClearSectionsTable();

	const char			*FindValueFromConfig( const char *pszSection, const char *pszProperty );
	bool				ParseFile( const char *pszFilePath );

	bool				ParseUtil_ContainsChars( char ch, const char *chars, size_t length );
	char				*ParseUtil_LStrip( char *str );
	void				ParseUtil_RStrip( char *str );
	char				*ParseUtil_Strip( char *str );
	void				ParseUtil_RemoveComment( char *str );

private:
	typedef struct ConfigPair_s
	{
		const char *section;
		const char *property;
		const char *value;
	} ConfigPair_t;

	typedef const char *section_cstring_t;

	class CLookupFunctor
	{
	public:
		CLookupFunctor() {}

		// The compare function for config pairs
		bool operator()( const ConfigPair_t &a, const ConfigPair_t &b ) const
		{
			return !stricmp( a.section, b.section ) && !stricmp( a.property, b.property );
		}

		// The hash function for config pairs
		unsigned int operator()( const ConfigPair_t &item ) const
		{
			return HashStringCaseless( item.property );
		}

		// The compare function for sections
		bool operator()( const section_cstring_t &a, const section_cstring_t &b ) const
		{
			return !stricmp( a, b );
		}

		// The hash function for sections
		unsigned int operator()( const section_cstring_t &item ) const
		{
			return HashStringCaseless( item );
		}
	};

private:
	CHashDict<std::vector<CConfigProperty *>> m_Config;
	CHashDict<std::vector<CConfigProperty *>> m_ShadersConfig;

	std::vector<IConfigListener *> m_listeners;

	CLookupFunctor m_Functor;

	CHash<ConfigPair_t, CLookupFunctor &, CLookupFunctor &> m_ConfigTable;
	CHash<const char *, CLookupFunctor &, CLookupFunctor &> m_SectionsTable;

	bool		m_bSectionsDuped;
	bool		m_bSectionsFilled;

	FILE		*m_hConfigFile;
	const char *m_pszCurrentSection;
	int			m_iRadix;

	int			m_State;
	int			m_ParseState;
	int			m_iLastErrorLine;
	const char *m_pszLastErrorMessage;
};

namespace Modules { extern CConfigModule *config; }

#endif // SINT_CONFIG_MODULE_H

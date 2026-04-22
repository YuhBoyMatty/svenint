// SvenInt (c) Sw1ft
// base_feature.h

#ifndef SINT_BASE_FEATURE_H
#define SINT_BASE_FEATURE_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define EXPOSE_FEATURE_SINGLETON( className, var, category, name ) namespace Features { \
		static className feature_##var( category, name ); \
		className *var = &feature_##var; \
	} static className *this_feature = Features::var;
#define EXTERN_FEATURE( className, var ) namespace Features { extern className *var; }

#define THIS_FEATURE() this_feature
#define THIS_FEATURE_IS_LOADED() this_feature->IsLoaded()
#define THIS_FEATURE_IS_ENABLED() this_feature->IsEnabled()
#define THIS_FEATURE_IS_RUNNING() this_feature->IsLoaded() && this_feature->IsEnabled()

#define FEATURE_REQUIRE_GAMEDATA( symbolPointer, symbolName ) if ( symbolPointer == NULL ) \
	{ PrintWarning( "Feature load failure, gamedata symbol \"%s\" is NULL\n", symbolName ); return false; }

#define FEATURE_SYMBOL_LOOKUP_FAIL( symbol ) PrintWarning( "Failed to locate symbol \"%s\"\n", symbol )
#define FEATURE_PRINT_SYMBOL( symbol, address ) DevMsg( "<%s::%s> Found symbol \"%s\" at address 0x%X\n", GetCategoryName(), GetName(), symbol, address )

#define FEATURE_SYMBOL_PATTERN_LOOKUP_FAIL( symbol ) PrintWarning( "Failed to locate signature of symbol \"%s\"\n", symbol )
#define FEATURE_PRINT_SYMBOL_PATTERN( symbol, address ) DevMsg( "<%s::%s> Found signature of symbol \"%s\" at address 0x%X\n", GetCategoryName(), GetName(), symbol, address )
#define FEATURE_PRINT_SYMBOL_PATTERNS( symbol, address, patternVar, patternIndex ) \
	DevMsg( "<%s::%s> Found signature of symbol \"%s\" for version \"%s\" at address 0x%X\n", GetCategoryName(), GetName(), symbol, GET_PATTERN_NAME_BY_INDEX( patternVar, patternIndex ), address )

#define FEATURE_CHECK_SYMBOL( symbolPointer, symbolName ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_LOOKUP_FAIL( symbolName ); return false; } else { FEATURE_PRINT_SYMBOL( symbolName, symbolPointer ); }

#define FEATURE_CHECK_SYMBOL_PATTERN( symbolPointer, symbolName ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_PATTERN_LOOKUP_FAIL( symbolName ); return false; } else { FEATURE_PRINT_SYMBOL_PATTERN( symbolName, symbolPointer ); }

#define FEATURE_CHECK_SYMBOL_PATTERNS( symbolPointer, symbolName, patternVar, patternIndex ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_PATTERN_LOOKUP_FAIL( symbolName ); return false; } else { FEATURE_PRINT_SYMBOL_PATTERNS( symbolName, symbolPointer, patternVar, patternIndex ); }

#define FEATURE_CHECK_SYMBOL_STATUS( symbolPointer, symbolName ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_LOOKUP_FAIL( symbolName ); bOK = false; } else { FEATURE_PRINT_SYMBOL( symbolName, symbolPointer ); }

#define FEATURE_CHECK_SYMBOL_PATTERN_STATUS( symbolPointer, symbolName ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_PATTERN_LOOKUP_FAIL( symbolName ); bOK = false; } else { FEATURE_PRINT_SYMBOL_PATTERN( symbolName, symbolPointer ); }

#define FEATURE_CHECK_SYMBOL_PATTERNS_STATUS( symbolPointer, symbolName, patternVar, patternIndex ) if ( symbolPointer == NULL ) \
	{ FEATURE_SYMBOL_PATTERN_LOOKUP_FAIL( symbolName ); bOK = false; } else { FEATURE_PRINT_SYMBOL_PATTERNS( symbolName, symbolPointer, patternVar, patternIndex ); }

#define FEATURE_REGISTER_CVAR( name ) Globals::cvar->RegisterConCommand( &name )
#define FEATURE_REGISTER_CCMD( name ) Globals::cvar->RegisterConCommand( &name##_command )
#define FEATURE_UNREGISTER_CVAR( name ) Globals::cvar->UnregisterConCommand( &name )
#define FEATURE_UNREGISTER_CCMD( name ) Globals::cvar->UnregisterConCommand( &name##_command )

#define FEATURE_CON_COMMAND_TOGGLE( name, description ) CON_COMMAND( name, description ) { \
		if ( args.ArgC() > 1 ) { \
			const bool bEnable = !!atoi( args[ 1 ] ); \
			if ( bEnable != THIS_FEATURE()->IsEnabled() ) { \
				THIS_FEATURE()->Toggle(); \
				THIS_FEATURE()->PrintState(); \
			} \
		} \
		else { \
			THIS_FEATURE()->Toggle(); \
			THIS_FEATURE()->PrintState(); \
		} \
	}

#define FEATURE_CON_COMMAND_HOLD( name, description ) \
	CON_COMMAND_CMDNAME( name##_down, "+" #name, "" ) { \
		if ( !THIS_FEATURE()->IsEnabled() ) \
			THIS_FEATURE()->Toggle(); \
	} \
	CON_COMMAND_CMDNAME( name##_up, "-" #name, "" ) { \
		if ( THIS_FEATURE()->IsEnabled() ) \
			THIS_FEATURE()->Toggle(); \
	}

//-----------------------------------------------------------------------------
// Features namespace
//-----------------------------------------------------------------------------

namespace Features
{
}

//-----------------------------------------------------------------------------
// Load / unload all features
//-----------------------------------------------------------------------------

// Returns 'false' if at least one feature can't be loaded, otherwise 'true'
bool AreFeaturesLoaded();
bool LoadFeatures();
void PostLoadFeatures();
void UnloadFeatures();

bool AreShaderFeaturesLoaded();
bool LoadShaderFeatures();
void PostLoadShaderFeatures();
void DisableShaderFeatures();

//-----------------------------------------------------------------------------
// Purpose: abstract class to create a loadable feature
//-----------------------------------------------------------------------------

class CBaseFeature
{
	friend class CMenuModule;

	friend bool LoadFeatures();
	friend void PostLoadFeatures();
	friend void UnloadFeatures();
	
	friend bool LoadShaderFeatures();
	friend void PostLoadShaderFeatures();
	friend void DisableShaderFeatures();

public:
	CBaseFeature();
	CBaseFeature( const char *pszCategoryName, const char *pszName );
	virtual ~CBaseFeature() {}

	virtual bool		Load( void ) { return true; }
	virtual void		PostLoad( void ) {}
	virtual void		Unload( void ) {}

	virtual void		OnEnable( void ) {}
	virtual void		OnDisable( void ) {}

	virtual bool		IsLoaded( void ) final { return m_bLoaded; }
	virtual bool		IsEnabled( void ) final { return m_bEnabled; }

public:
	void				Enable( void );
	void				Disable( void );
	bool				Toggle( void );

	const char			*GetCategoryName( void );
	const char			*GetName( void );

	void				PrintState( bool bCustomState = false, bool bState = true );

	void				PrintMsg( const char *pszMessage, ... );
	void				PrintWarning( const char *pszMessage, ... );
	void				PrintWarning2( const char *pszMessage, ... );

	static void			PrintFeatures( void );

protected:
	void				SetFeatureInfo( const char *pszCategoryName, const char *pszName );
	inline void			SetLoaded( bool state ) { m_bLoaded = state; }
	inline void			SetInitiallyEnabled( void ) { m_bEnabled = true; }
	inline void			SetInitiallyDisabled( void ) { m_bEnabled = false; }

private:
	bool m_bLoaded;
	bool m_bEnabled;

	const char *m_pszCategoryName;
	const char *m_pszName;

	CBaseFeature *m_pNext;
	CBaseFeature *m_pPrev;

	static CBaseFeature *s_pBaseFeaturesList;
};

#endif // SINT_BASE_FEATURE_H
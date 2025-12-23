// SvenInt (c) Sw1ft
// keyvalues_parser.h

#ifndef SINT_KEYVALUES_H
#define SINT_KEYVALUES_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>
#include <string>

namespace KeyValuesParser
{
	enum ParseResult
	{
		PARSE_OK = 0,
		PARSE_FAILED
	};

	struct KeyValues
	{
		inline KeyValues() : bSection( false ), sKey(), sValue(), keyvalues()
		{
		}

		inline KeyValues( const char *pszSectionName ) : bSection( true ), sKey( pszSectionName ), sValue(), keyvalues()
		{
		}

		inline KeyValues( const char *pszKey, const char *pszValue ) : bSection( false ), sKey( pszKey ), sValue( pszValue ), keyvalues()
		{
		}

		inline KeyValues( std::string &sSectionName ) : bSection( true ), sKey( sSectionName ), sValue(), keyvalues()
		{
		}

		inline KeyValues( std::string &sKey, std::string &sValue ) : bSection( false ), sKey( sKey ), sValue( sValue ), keyvalues()
		{
		}

		inline ~KeyValues()
		{
			Clear();

			sKey.clear();
			sValue.clear();

			keyvalues.clear();
		}

		inline bool IsSection() { return bSection; }
		inline bool IsPair() { return !bSection; }

		inline void AddItem( KeyValues *kv )
		{
			if ( bSection )
				keyvalues.push_back( kv );
		}

		inline void AddItem( KeyValues &kv )
		{
			if ( bSection )
				keyvalues.push_back( &kv );
		}

		inline KeyValuesParser::KeyValues *FindKey( const char *pszKey )
		{
			for ( size_t i = 0; i < keyvalues.size(); i++ )
			{
				if ( keyvalues[ i ]->Key() == pszKey )
					return keyvalues[ i ];
			}

			return NULL;
		}

		inline std::string &Key() { return sKey; }
		inline std::string &GetName() { return sKey; }

		inline std::string &Value() { return sValue; }
		inline std::string &GetValue() { return sValue; }

		inline std::vector<KeyValues *> &GetList() { return keyvalues; }

		void Clear();

		bool bSection;

		std::string sKey;
		std::string sValue;

		std::vector<KeyValues *> keyvalues;
	};

	KeyValues *LoadFromFile( const char *pszFilename, int *result_code );

	const char *GetLastErrorMessage();
	int GetLastErrorLine();

	void UsesEscapeSequences( bool state );
}

#endif // SINT_KEYVALUES_H

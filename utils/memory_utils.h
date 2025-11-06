// SvenInt (c) Sw1ft
// memory_utils.h

#ifndef SINT_MEMORYUTILS_H
#define SINT_MEMORYUTILS_H

#ifdef _WIN32
#pragma once
#endif

#include <future>

#include "patterns.h"
#include "utils/hashtable.h"
#include "udis86/udis86.h"

#ifndef WIN32
#include <sys/mman.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE				4096
#define PAGE_ALIGN_UP(x)		((x + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define ALIGN_ADDR(addr)		((long)addr & ~(PAGE_SIZE - 1))
#endif

// not actual flags as in Windows
#define MEM_RESERVE				MAP_PRIVATE
#define MEM_COMMIT				MAP_ANONYMOUS

#define PAGE_NOACCESS			0
#define PAGE_READONLY			PROT_READ
#define PAGE_READWRITE			PROT_READ | PROT_WRITE
#define PAGE_EXECUTE_READ		PROT_READ | PROT_EXEC
#define PAGE_EXECUTE_READWRITE	PROT_READ | PROT_WRITE | PROT_EXEC

// can be returned in function 'IMemoryUtils::VirtualProtect' from param @pfOldProtect
#define PAGE_SHARED				(1 << 3)
#define PAGE_PRIVATE			(1 << 4)

#define MEMUTILS_DUPE_SYMBOLS_NAME 0
#endif

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------

typedef void *module_t;

typedef struct moduleinfo_s
{
	module_t hModule;
	void *pBaseOfDll;
	unsigned int SizeOfImage;
} moduleinfo_t;

//-----------------------------------------------------------------------------
// Purpose: interface to memory's API
//-----------------------------------------------------------------------------

class CMemoryUtils
{
public:
	CMemoryUtils();
	~CMemoryUtils();

	//-----------------------------------------------------------------------------
	// Initialize ud structure
	//-----------------------------------------------------------------------------

	void InitDisasm( ud_t *instruction, void *buffer, uint8_t mode, size_t buffer_length = 128 );

	//-----------------------------------------------------------------------------
	// Disassemble memory
	// For more abilities, use udis86 functions: udis86/include/extern.h
	//-----------------------------------------------------------------------------

	int Disassemble( ud_t *instruction );
	
	//-----------------------------------------------------------------------------
	// Dump in the console operands of a disassembled instruction
	//-----------------------------------------------------------------------------

	void DumpOperands( ud_t *instruction );

	//-----------------------------------------------------------------------------
	// Set memory/page protection
	//-----------------------------------------------------------------------------

	bool VirtualProtect( void *pAddress, size_t size, int fNewProtect, int *pfOldProtect );

	//-----------------------------------------------------------------------------
	// Allocate virtual memory
	//-----------------------------------------------------------------------------

	void *VirtualAlloc( void *pAddress, size_t size, int fAllocationType, int fProtection );

	//-----------------------------------------------------------------------------
	// Free allocated virtual memory
	//-----------------------------------------------------------------------------

	bool VirtualFree( void *pAddress, size_t size, int fFreeType = 0 );

	//-----------------------------------------------------------------------------
	// Patch memory address with given length
	//-----------------------------------------------------------------------------

	void PatchMemory( void *pAddress, unsigned char *pPatchBytes, int length );

	//-----------------------------------------------------------------------------
	// NOP memory address with given length
	//-----------------------------------------------------------------------------

	void MemoryNOP( void *pAddress, int length );

	//-----------------------------------------------------------------------------
	// Calculate absolute function address from CALL/JMP opcode
	// Pointer @pCallOpcode MUST point to CALL/JMP opcode
	//-----------------------------------------------------------------------------

	void *CalcAbsoluteAddress( void *pCallOpcode );

	//-----------------------------------------------------------------------------
	// Calculate relative address for calling/jumping from CALL/JMP opcode
	// Pointer @pFrom MUST point to CALL/JMP opcode
	//-----------------------------------------------------------------------------

	void *CalcRelativeAddress( void *pFrom, void *pTo );

	//-----------------------------------------------------------------------------
	// Get a virtual methods table
	//-----------------------------------------------------------------------------

	void *GetVTable( void *pClassInstance );

	//-----------------------------------------------------------------------------
	// Get a virtual function from virtual methods table
	//-----------------------------------------------------------------------------

	void *GetVirtualFunction( void *pClassInstance, int nFunctionIndex );

	//-----------------------------------------------------------------------------
	// Get module info
	//-----------------------------------------------------------------------------

	bool RetrieveModuleInfo( module_t hModule );
	
	//-----------------------------------------------------------------------------
	// Get module info
	//-----------------------------------------------------------------------------

	bool RetrieveModuleInfo( module_t hModule, moduleinfo_t *pModInfo );

	//-----------------------------------------------------------------------------
	// Lookup for a symbol in Symbol Table
	// In Windows, will be called GetProcAddress function
	//-----------------------------------------------------------------------------

	void *ResolveSymbol( module_t hModule, const char *pszSymbol );

	//-----------------------------------------------------------------------------
	// Find signature from pattern_s structure
	//-----------------------------------------------------------------------------

	void *FindPattern( module_t hModule, pattern_t *pPattern, unsigned int offset = 0 );
	void *FindPatternWithin( module_t hModule, pattern_t *pPattern, void *pSearchStart, void *pSearchEnd );

	//-----------------------------------------------------------------------------
	// Find signature from string with given mask
	// If signature: "\xD9\x1D\x2A\x2A\x2A\x2A\x75\x0A\xA1", then mask: "xx????xxx"
	//-----------------------------------------------------------------------------

	void *FindPattern( module_t hModule, const char *pszPattern, char *pszMask, unsigned int offset = 0 );
	void *FindPatternWithin( module_t hModule, const char *pszPattern, char *pszMask, void *pSearchStart, void *pSearchEnd );

	//-----------------------------------------------------------------------------
	// Find signature from string but ignore a specific byte
	// If signature: "\xD9\x1D\x2A\x2A\x2A\x2A\x75\x0A\xA1", then ignore byte can be: '0x2A'
	//-----------------------------------------------------------------------------

	void *FindPattern( module_t hModule, const char *pszPattern, unsigned int length, unsigned int offset = 0, char ignoreByte = '\x2A' );
	void *FindPatternWithin( module_t hModule, const char *pszPattern, unsigned int length, void *pSearchStart, void *pSearchEnd, char ignoreByte = '\x2A' );

	//-----------------------------------------------------------------------------
	// Find signature from range of bytes with a specific byte to ignore
	// For example: unsigned char sig[] = { 0xD9, 0x1D, 0x2A, 0x2A, 0x2A, 0x2A, 0x75, 0x0A, 0xA1 };
	//-----------------------------------------------------------------------------

	void *FindPattern( module_t hModule, unsigned char *pPattern, unsigned int length, unsigned int offset = 0, unsigned char ignoreByte = 0x2A );
	void *FindPatternWithin( module_t hModule, unsigned char *pPattern, unsigned int length, void *pSearchStart, void *pSearchEnd, unsigned char ignoreByte = 0x2A );

	//-----------------------------------------------------------------------------
	// Lookup for a string
	//-----------------------------------------------------------------------------

	void *FindString( module_t hModule, const char *pszString, unsigned int offset = 0 );
	void *FindStringWithin( module_t hModule, const char *pszString, void *pSearchStart, void *pSearchEnd );

	//-----------------------------------------------------------------------------
	// Lookup for an address
	//-----------------------------------------------------------------------------

	void *FindAddress( module_t hModule, void *pAddress, unsigned int offset = 0 );
	void *FindAddressWithin( module_t hModule, void *pAddress, void *pSearchStart, void *pSearchEnd );
	
	//-----------------------------------------------------------------------------
	// Lookup for a VTable
	//-----------------------------------------------------------------------------

	void *FindVTable( module_t hModule, const char *pszVTable );

	//-----------------------------------------------------------------------------
	// Wrappers
	//-----------------------------------------------------------------------------

	inline void *FindPattern( module_t hModule, pattern_reg_t *patterns, int *patternIndex )
	{
		for ( int i = 0; patterns[ i ].name != NULL; i++ )
		{
			void *pAddress = NULL;
			pattern_t *pattern = patterns[ i ].pattern;

			if ( ( pAddress = FindPattern( hModule, pattern ) ) != NULL )
			{
				if ( patternIndex != NULL )
					*patternIndex = i;

				return pAddress;
			}
		}

		return NULL;
	}

	inline std::future<void *> FindPatternAsync( module_t hModule, pattern_t *pPattern, unsigned int offset = 0 )
	{
		CMemoryUtils *pMemoryUtils = this;
		return std::async( [pMemoryUtils, hModule, pPattern, offset] { return pMemoryUtils->FindPattern( hModule, pPattern, offset ); } );
	}

	inline void FindPatternAsync( module_t hModule, pattern_reg_t *patterns, std::vector<std::future<void *>> &futures )
	{
		// Maybe 4 as minimum
		futures.reserve( 4 );

		CMemoryUtils *pMemoryUtils = this;

		for ( int i = 0; patterns[ i ].name != NULL; i++ )
		{
			pattern_t *pattern = patterns[ i ].pattern;
			futures.push_back( std::async( [pMemoryUtils, hModule, pattern] { return pMemoryUtils->FindPattern( hModule, pattern ); } ) );
		}
	}

	inline void *GetPatternFutureValue( std::vector<std::future<void *>> &futures, int *patternIndex )
	{
		void *pAddress = NULL;

		for ( size_t i = 0; i < futures.size(); i++ )
		{
			void *adr = futures[ i ].get();

			if ( pAddress == NULL && adr != NULL )
			{
				if ( patternIndex != NULL )
					*patternIndex = i;

				pAddress = adr;
			}
		}

		return pAddress;
	}

#ifndef WIN32
private:
	typedef struct symbol_s
	{
		const char *name;
		size_t length;
		void *address;
	} symbol_t;

	class CLookupFunctor
	{
	public:
		CLookupFunctor() {}

		// Compare
		bool operator()( const symbol_t &a, const symbol_t &b ) const
		{
			return a.length == b.length && !strcmp( a.name, b.name );
		}

		// Get hash
		unsigned int operator()( const symbol_t &sym ) const
		{
			return HashKey( (unsigned char *)sym.name, sym.length );
		}
	};

	class CSymbolTable
	{
	public:
		CSymbolTable() : m_SymbolsTable( 3, m_Functor, m_Functor )
		{
		}

		CSymbolTable( int tableSize ) : m_SymbolsTable( tableSize, m_Functor, m_Functor )
		{
		}

		~CSymbolTable()
		{
		#if MEMUTILS_DUPE_SYMBOLS_NAME
			for ( int i = 0; i < m_SymbolsTable.Count(); i++ )
			{
				HashIterator_t it = m_SymbolsTable.First( i );

				while ( m_SymbolsTable.IsValidIterator( it ) )
				{
					symbol_t &sym = m_SymbolsTable.At( i, it );

					free( (void *)sym.name );

					it = m_SymbolsTable.Next( i, it );
				}
			}
		#endif
		}

		void ResizeTable( int tableSize )
		{
			m_SymbolsTable.Resize( tableSize );
		}

		symbol_t *FindSymbol( const char *pszSymbol, int length )
		{
			symbol_t symbol_find =
			{
				pszSymbol,
				(size_t)length,
				NULL
			};

			return m_SymbolsTable.Find( symbol_find );
		}

		symbol_t *FindSymbol( const char *pszSymbol )
		{
			return FindSymbol( pszSymbol, strlen( pszSymbol ) );
		}

		symbol_t *InternSymbol( const char *pszSymbol, int length, void *pAddress )
		{
		#if MEMUTILS_DUPE_SYMBOLS_NAME
			pszSymbol = strdup( pszSymbol );
		#endif

			symbol_t symbol =
			{
				pszSymbol,
				(size_t)length,
				pAddress
			};

			bool bInserted = m_SymbolsTable.Insert( symbol );

			if ( !bInserted )
			{
			#if MEMUTILS_DUPE_SYMBOLS_NAME
				free( (void *)pszSymbol );
			#endif
			}

			return m_SymbolsTable.Find( symbol );
		}

		symbol_t *InternSymbol( const char *pszSymbol, void *pAddress )
		{
			return InternSymbol( pszSymbol, strlen( pszSymbol ), pAddress );
		}

	private:
		CLookupFunctor m_Functor;
		CHash<symbol_t, CLookupFunctor &, CLookupFunctor &> m_SymbolsTable;
	};

	class CModuleSymbolTable
	{
	public:
		CModuleSymbolTable() : table()
		{
			handle = NULL;
			last_pos = 0;
		}

	public:
		CSymbolTable table;
		module_t handle;
		size_t last_pos;
	};

private:
	int ReadMemoryProtection( void *pAddress );
#endif

private:
	CHashTable<module_t, moduleinfo_t> m_ModuleInfoTable;

#ifndef WIN32
	std::vector<CModuleSymbolTable *> m_SymbolTables;
#endif
};

//-----------------------------------------------------------------------------
// Get a function from virtual methods table
//-----------------------------------------------------------------------------

inline void *GetVTableFunction( void *pBaseClass, int nIndex )
{
	const unsigned long *pVTable = *static_cast<unsigned long **>( pBaseClass );
	return (void *)( pVTable[ nIndex ] );
}

template <typename T>
inline T GetVTableFunction( void *pBaseClass, int nIndex )
{
	const unsigned long *pVTable = *static_cast<unsigned long **>( pBaseClass );
	return reinterpret_cast<T>( pVTable[ nIndex ] );
}

//-----------------------------------------------------------------------------
// Cast to a function
//-----------------------------------------------------------------------------

template <typename T>
inline T function_cast( void *pFunction )
{
	return reinterpret_cast<T>( pFunction );
}

//-----------------------------------------------------------------------------
// Cast to a type
//-----------------------------------------------------------------------------

template <typename T>
inline T view_as( void *pPointer )
{
	return reinterpret_cast<T>( pPointer );
}

//-----------------------------------------------------------------------------
// Offset an address
//-----------------------------------------------------------------------------

inline void *GetOffset( void *ptr, unsigned long offset )
{
	return (void *)( (unsigned char *)ptr + offset );
}

//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------

extern CMemoryUtils *g_pMemoryUtils;
inline CMemoryUtils *MemoryUtils() { return g_pMemoryUtils; };

#endif // SINT_MEMORYUTILS_H
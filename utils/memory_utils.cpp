// SvenInt (c) Sw1ft
// memory_utils.cpp

#include "memory_utils.h"

#include <Windows.h>

#ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <link.h>
#endif

//-----------------------------------------------------------------------------
// CMemoryUtils implementation
//-----------------------------------------------------------------------------

CMemoryUtils::CMemoryUtils() : m_ModuleInfoTable( 15 )
{
}

CMemoryUtils::~CMemoryUtils()
{
#ifndef WIN32
	for ( size_t i = 0; i < m_SymbolTables.size(); i++ )
	{
		delete m_SymbolTables[ i ];
	}

	m_SymbolTables.clear();
#endif

	//m_ModuleInfoTable.Purge();
}

//-----------------------------------------------------------------------------
// Disassembler
//-----------------------------------------------------------------------------

void CMemoryUtils::InitDisasm( ud_t *instruction, void *buffer, uint8_t mode, size_t buffer_length /* = 128 */ )
{
	ud_init( instruction );
	ud_set_mode( instruction, mode );
	ud_set_input_buffer( instruction, (const uint8_t *)buffer, buffer_length );
}

int CMemoryUtils::Disassemble( ud_t *instruction )
{
	return ud_disassemble( instruction );
}

void CMemoryUtils::DumpOperands( ud_t *instruction )
{
	extern void Msg( const char *pszMessageFormat, ... );

	Msg( "mnemonic: %d\n", instruction->mnemonic );

	for ( int i = 0; i < 4; i++ )
	{
		Msg( "operand[ %d ].type: %d\n", i, instruction->operand[ i ].type );
		Msg( "operand[ %d ].size: %d\n", i, instruction->operand[ i ].size );
		Msg( "operand[ %d ].base: %d\n", i, instruction->operand[ i ].base );
		Msg( "operand[ %d ].index: %d\n", i, instruction->operand[ i ].index );
		Msg( "operand[ %d ].scale: %d\n", i, instruction->operand[ i ].scale );
		Msg( "operand[ %d ].offset: %d\n", i, instruction->operand[ i ].offset );
		Msg( "operand[ %d ].lval.sbyte: %d\n", i, instruction->operand[ i ].lval.sbyte );
		Msg( "operand[ %d ].lval.ubyte: %d\n", i, instruction->operand[ i ].lval.ubyte );
		Msg( "operand[ %d ].lval.sword: %d\n", i, instruction->operand[ i ].lval.sword );
		Msg( "operand[ %d ].lval.uword: %d\n", i, instruction->operand[ i ].lval.uword );
		Msg( "operand[ %d ].lval.sdword: %d\n", i, instruction->operand[ i ].lval.sdword );
		Msg( "operand[ %d ].lval.udword: %d\n", i, instruction->operand[ i ].lval.udword );
		Msg( "operand[ %d ].lval.sqword: %d\n", i, instruction->operand[ i ].lval.sqword );
		Msg( "operand[ %d ].lval.uqword: %d\n", i, instruction->operand[ i ].lval.uqword );
		Msg( "operand[ %d ].lval.ptr.seg: %d\n", i, instruction->operand[ i ].lval.ptr.seg );
		Msg( "operand[ %d ].lval.ptr.off: %d\n\n", i, instruction->operand[ i ].lval.ptr.off );
	}
}

//-----------------------------------------------------------------------------
// Virtual memory
//-----------------------------------------------------------------------------

bool CMemoryUtils::VirtualProtect( void *pAddress, size_t size, int fNewProtect, int *pfOldProtect )
{
#ifdef WIN32
	if ( pfOldProtect == NULL )
	{
		static int oldprotect;
		pfOldProtect = &oldprotect;
	}

	return !!( ::VirtualProtect( pAddress, size, fNewProtect, (PDWORD)pfOldProtect ) );
#else
	pAddress = (void *)( ALIGN_ADDR( pAddress ) );
	size = PAGE_ALIGN_UP( size );

	if ( pfOldProtect )
	{
		*pfOldProtect = ReadMemoryProtection( pAddress );
	}

	return mprotect( pAddress, size, fNewProtect ) != -1;
#endif

	return false;
}

void *CMemoryUtils::VirtualAlloc( void *pAddress, size_t size, int fAllocationType, int fProtection )
{
#ifdef WIN32
	return ::VirtualAlloc( pAddress, size, fAllocationType, fProtection );
#else
	pAddress = (void *)( ALIGN_ADDR( pAddress ) );
	size = PAGE_ALIGN_UP( size );

	void *addr = mmap( pAddress, size, fProtection, fAllocationType, -1, 0 );

	if ( addr == MAP_FAILED )
		return NULL;

	return addr;
#endif

	return NULL;
}

bool CMemoryUtils::VirtualFree( void *pAddress, size_t size, int fFreeType /* = 0 */ )
{
#ifdef WIN32
#pragma warning(push)
#pragma warning(disable : 28160)

	return !!( ::VirtualFree( pAddress, size, fFreeType ) );

#pragma warning(pop)
#else
	pAddress = (void *)( ALIGN_ADDR( pAddress ) );
	size = PAGE_ALIGN_UP( size );

	return (void *)munmap( pAddress, size ) != MAP_FAILED;
#endif

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void CMemoryUtils::PatchMemory( void *pAddress, unsigned char *pPatchBytes, int length )
{
	int dwProtection;

	this->VirtualProtect( pAddress, length, PAGE_EXECUTE_READWRITE, &dwProtection );

	memcpy( pAddress, pPatchBytes, length );

	this->VirtualProtect( pAddress, length, dwProtection, NULL );
}

void CMemoryUtils::MemoryNOP( void *pAddress, int length )
{
	int dwProtection;

	this->VirtualProtect( pAddress, length, PAGE_EXECUTE_READWRITE, &dwProtection );

	memset( pAddress, 0x90, length );

	this->VirtualProtect( pAddress, length, dwProtection, NULL );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void *CMemoryUtils::CalcAbsoluteAddress( void *pCallAddress )
{
	unsigned long luRelativeAddress = *(uint32_t *)( (uint8_t *)pCallAddress + 1 );
	return (void *)( luRelativeAddress + (uint32_t)pCallAddress + sizeof( void * ) + 1 );
}

void *CMemoryUtils::CalcRelativeAddress( void *pFrom, void *pTo )
{
	return (void *)( (uint32_t)pTo - ( (uint32_t)pFrom + sizeof( void * ) + 1 ) );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void *CMemoryUtils::GetVTable( void *pClassInstance )
{
	void *pVTable = *static_cast<void **>( pClassInstance );
	return pVTable;
}

void *CMemoryUtils::GetVirtualFunction( void *pClassInstance, int nFunctionIndex )
{
	void **pVTable = *static_cast<void ***>( pClassInstance );
	return reinterpret_cast<void *>( pVTable[ nFunctionIndex ] );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool CMemoryUtils::RetrieveModuleInfo( module_t hModule )
{
	if ( !hModule )
		return false;

	moduleinfo_t ModuleInfoEntry = { hModule, NULL, NULL };

#ifdef WIN32
	MEMORY_BASIC_INFORMATION memInfo;

	if ( VirtualQuery( hModule, &memInfo, sizeof( MEMORY_BASIC_INFORMATION ) ) )
	{
		if ( memInfo.State != MEM_COMMIT || memInfo.Protect == PAGE_NOACCESS )
			return false;

		unsigned int dwAllocationBase = (unsigned int)memInfo.AllocationBase;
		ModuleInfoEntry.pBaseOfDll = memInfo.AllocationBase;

		IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)dwAllocationBase;
		IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)( dwAllocationBase + dos->e_lfanew );

		IMAGE_FILE_HEADER *file = &pe->FileHeader;
		IMAGE_OPTIONAL_HEADER *opt = &pe->OptionalHeader;

		if ( dos->e_magic == IMAGE_DOS_SIGNATURE && pe->Signature == IMAGE_NT_SIGNATURE && opt->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC && file->Machine == IMAGE_FILE_MACHINE_I386 )
		{
			/*
			if ( (file->Characteristics & IMAGE_FILE_DLL) == 0 )
				return false;
			*/

			ModuleInfoEntry.SizeOfImage = opt->SizeOfImage;
			ModuleInfoEntry.hModule = hModule;

			m_ModuleInfoTable.Insert( hModule, ModuleInfoEntry );

			return true;
		}
	}
#else
#if 0
	Dl_info info;
	void *pAddress = NULL;

	// Try to find CreateInterface symbol to don't check memory mappings
	// pAddress = dlsym(hModule, "CreateInterface");

	// We need to waste our time. Is there really no other way?
	if ( !pAddress )
	{
		struct link_map *dlmap;

		if ( dlinfo( hModule, RTLD_DI_LINKMAP, &dlmap ) != 0 )
			return false;

		static char szBuffer[ 1024 ];
		const char *pszTargetModulePath = dlmap->l_name;

		FILE *file = fopen( "/proc/self/maps", "r" );

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			const char *pszModulePath = strchr( szBuffer, '/' );

			if ( pszModulePath )
			{
				size_t count = strlen( pszTargetModulePath );

				if ( memcmp( pszTargetModulePath, pszModulePath, count ) == 0 )
				{
					pAddress = (void *)strtoul( szBuffer, NULL, 16 );
					break;
				}
			}
		}

		fclose( file );
	}

	if ( dladdr( pAddress, &info ) && info.dli_fbase && info.dli_fname )
	{
		unsigned int dwAllocationBase = (unsigned int)info.dli_fbase;
		ModuleInfoEntry.pBaseOfDll = info.dli_fbase;
	#else
	struct link_map *dlmap;

	if ( dlinfo( hModule, RTLD_DI_LINKMAP, &dlmap ) == 0 && dlmap->l_addr && dlmap->l_name )
	{
		unsigned int dwAllocationBase = (unsigned int)dlmap->l_addr;
		ModuleInfoEntry.pBaseOfDll = (void *)dlmap->l_addr;
	#endif	
		Elf32_Ehdr *file = (Elf32_Ehdr *)dwAllocationBase;

		if ( !memcmp( ELFMAG, file->e_ident, SELFMAG ) && file->e_ident[ EI_VERSION ] == EV_CURRENT && file->e_ident[ EI_CLASS ] == ELFCLASS32 && file->e_machine == EM_386 && file->e_ident[ EI_DATA ] == ELFDATA2LSB )
		{
			/*
			if ( file->e_type != ET_DYN )
				return false;
			*/

			Elf32_Phdr *phdr = (Elf32_Phdr *)( dwAllocationBase + file->e_phoff );

			for ( unsigned short i = 0; i < file->e_phnum; i++ )
			{
				Elf32_Phdr *hdr = &phdr[ i ];

				if ( hdr->p_type == PT_LOAD && hdr->p_flags == ( PF_X | PF_R ) )
				{
					ModuleInfoEntry.SizeOfImage = PAGE_ALIGN_UP( hdr->p_filesz );

					m_ModuleInfoTable.Insert( ModuleInfoEntry );

					return true;
				}
			}
		}
	}
#endif

	return false;
}

bool CMemoryUtils::RetrieveModuleInfo( module_t hModule, moduleinfo_t *pModInfo )
{
	if ( !hModule )
		return false;

	moduleinfo_t *pHashEntry = NULL;

	if ( pHashEntry = m_ModuleInfoTable.Find( hModule ) )
	{
		*pModInfo = *pHashEntry;
		return true;
	}

#ifdef WIN32
	MEMORY_BASIC_INFORMATION memInfo;

	if ( VirtualQuery( hModule, &memInfo, sizeof( MEMORY_BASIC_INFORMATION ) ) )
	{
		if ( memInfo.State != MEM_COMMIT || memInfo.Protect == PAGE_NOACCESS )
			return false;

		unsigned int dwAllocationBase = (unsigned int)memInfo.AllocationBase;
		pModInfo->pBaseOfDll = memInfo.AllocationBase;

		IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)dwAllocationBase;
		IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)( dwAllocationBase + dos->e_lfanew );

		IMAGE_FILE_HEADER *file = &pe->FileHeader;
		IMAGE_OPTIONAL_HEADER *opt = &pe->OptionalHeader;

		if ( dos->e_magic == IMAGE_DOS_SIGNATURE && pe->Signature == IMAGE_NT_SIGNATURE && opt->Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC && file->Machine == IMAGE_FILE_MACHINE_I386 )
		{
			/*
			if ( (file->Characteristics & IMAGE_FILE_DLL) == 0 )
				return false;
			*/

			pModInfo->SizeOfImage = opt->SizeOfImage;
			pModInfo->hModule = hModule;

			m_ModuleInfoTable.Insert( hModule, *pModInfo );
			return true;
		}
	}
#else
#if 0
	Dl_info info;
	void *pAddress = NULL;

	// Try to find CreateInterface symbol to don't check memory mappings
	// pAddress = dlsym(hModule, "CreateInterface");

	// We need to waste our time. Is there really no other way?
	if ( !pAddress )
	{
		struct link_map *dlmap;

		if ( dlinfo( hModule, RTLD_DI_LINKMAP, &dlmap ) != 0 )
			return false;

		static char szBuffer[ 1024 ];
		const char *pszTargetModulePath = dlmap->l_name;

		FILE *file = fopen( "/proc/self/maps", "r" );

		while ( fgets( szBuffer, sizeof( szBuffer ), file ) )
		{
			const char *pszModulePath = strchr( szBuffer, '/' );

			if ( pszModulePath )
			{
				size_t count = strlen( pszTargetModulePath );

				if ( memcmp( pszTargetModulePath, pszModulePath, count ) == 0 )
				{
					pAddress = (void *)strtoul( szBuffer, NULL, 16 );
					break;
				}
			}
		}

		fclose( file );
	}

	if ( dladdr( pAddress, &info ) && info.dli_fbase && info.dli_fname )
	{
		unsigned int dwAllocationBase = (unsigned int)info.dli_fbase;
		pModInfo->pBaseOfDll = info.dli_fbase;
	#else
	struct link_map *dlmap;

	if ( dlinfo( hModule, RTLD_DI_LINKMAP, &dlmap ) == 0 && dlmap->l_addr && dlmap->l_name )
	{
		unsigned int dwAllocationBase = (unsigned int)dlmap->l_addr;
		pModInfo->pBaseOfDll = (void *)dlmap->l_addr;
	#endif	
		Elf32_Ehdr *file = (Elf32_Ehdr *)dwAllocationBase;

		if ( !memcmp( ELFMAG, file->e_ident, SELFMAG ) && file->e_ident[ EI_VERSION ] == EV_CURRENT && file->e_ident[ EI_CLASS ] == ELFCLASS32 && file->e_machine == EM_386 && file->e_ident[ EI_DATA ] == ELFDATA2LSB )
		{
			/*
			if ( file->e_type != ET_DYN )
				return false;
			*/

			Elf32_Phdr *phdr = (Elf32_Phdr *)( dwAllocationBase + file->e_phoff );

			for ( unsigned short i = 0; i < file->e_phnum; i++ )
			{
				Elf32_Phdr *hdr = &phdr[ i ];

				if ( hdr->p_type == PT_LOAD && hdr->p_flags == ( PF_X | PF_R ) )
				{
					pModInfo->SizeOfImage = PAGE_ALIGN_UP( hdr->p_filesz );
					pModInfo->hModule = hModule;

					m_ModuleInfoTable.Insert( hModule, *pModInfo );

					return true;
				}
			}
		}
	}
#endif

	return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void *CMemoryUtils::ResolveSymbol( module_t hModule, const char *pszSymbol )
{
#ifdef WIN32
	return GetProcAddress( (HMODULE)hModule, pszSymbol );
#else
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		bool new_table = false;

		CModuleSymbolTable *dltable = NULL;
		symbol_t *symbol = NULL;
		CSymbolTable *table = NULL;

		for ( size_t i = 0; i < m_SymbolTables.size(); i++ )
		{
			dltable = m_SymbolTables[ i ];

			if ( dltable->handle == hModule )
			{
				table = &dltable->table;
				break;
			}
		}

		if ( table == NULL )
		{
			new_table = true;

			dltable = new CModuleSymbolTable();
			table = &dltable->table;

			m_SymbolTables.push_back( dltable );
		}

		if ( !new_table && ( symbol = table->FindSymbol( pszSymbol, strlen( pszSymbol ) ) ) != NULL )
		{
			return symbol->address;
		}

		int dlfile;

		struct link_map *dlmap;
		struct stat dlstat;

		if ( dlinfo( hModule, RTLD_DI_LINKMAP, &dlmap ) != 0 || !dlmap->l_addr || !dlmap->l_name )
		{
			return NULL;
		}

		dlfile = open( dlmap->l_name, O_RDONLY );

		if ( dlfile == -1 || fstat( dlfile, &dlstat ) == -1 )
		{
			close( dlfile );
			return NULL;
		}

		Elf32_Ehdr *file_hdr = (Elf32_Ehdr *)mmap( NULL, dlstat.st_size, PROT_READ, MAP_PRIVATE, dlfile, 0 );
		unsigned int map_base = (unsigned int)file_hdr;

		if ( file_hdr == MAP_FAILED )
		{
			close( dlfile );
			return NULL;
		}

		close( dlfile );

		if ( file_hdr->e_shoff == 0 || file_hdr->e_shstrndx == SHN_UNDEF )
		{
			munmap( file_hdr, dlstat.st_size );
			return NULL;
		}

		Elf32_Shdr *symtab_hdr = NULL;
		Elf32_Shdr *strtab_hdr = NULL;

		Elf32_Shdr *sections = (Elf32_Shdr *)( map_base + file_hdr->e_shoff );
		unsigned short sections_count = file_hdr->e_shnum;

		Elf32_Shdr *shstrtab_hdr = &sections[ file_hdr->e_shstrndx ];
		const char *shstrtab = (const char *)( map_base + shstrtab_hdr->sh_offset );

		for ( unsigned short i = 0; i < sections_count; i++ )
		{
			Elf32_Shdr *hdr = &sections[ i ];

		#if 0
			const char *section_name = shstrtab + hdr->sh_name;

			if ( !strcmp( section_name, ".symtab" ) )
			{
				symtab_hdr = hdr;
			}
			else if ( !strcmp( section_name, ".strtab" ) )
			{
				strtab_hdr = hdr;
			}
		#else
			unsigned short section_type = hdr->sh_type;

			if ( section_type == SHT_SYMTAB )
			{
				symtab_hdr = hdr;
			}
			else if ( section_type == SHT_STRTAB )
			{
				const char *section_name = shstrtab + hdr->sh_name;

				if ( !strcmp( section_name, ".strtab" ) )
				{
					strtab_hdr = hdr;
				}
			}
		#endif
		}

		if ( symtab_hdr == NULL || strtab_hdr == NULL )
		{
			munmap( file_hdr, dlstat.st_size );
			return NULL;
		}

		Elf32_Sym *symtab = (Elf32_Sym *)( map_base + symtab_hdr->sh_offset );
		const char *strtab = (const char *)( map_base + strtab_hdr->sh_offset );
		size_t symbol_count = symtab_hdr->sh_size / symtab_hdr->sh_entsize;

		if ( new_table )
		{
			int tableSize = static_cast<int>( (float)symbol_count * ( 7.f / 11.f ) );

			if ( tableSize < 3 )
				tableSize = 3;
			else if ( tableSize > 65535 )
				tableSize = 65535;

			table->ResizeTable( tableSize );
		}

		for ( size_t i = dltable->last_pos; i < symbol_count; i++ )
		{
			Elf32_Sym &sym = symtab[ i ];

			unsigned char sym_type = ELF32_ST_TYPE( sym.st_info );
			const char *sym_name = strtab + sym.st_name;

			if ( sym.st_shndx == SHN_UNDEF || ( sym_type != STT_FUNC && sym_type != STT_OBJECT ) )
				continue;

			symbol_t *current_symbol = table->InternSymbol( sym_name, strlen( sym_name ), moduleInfo.pBaseOfDll + sym.st_value );

			if ( !strcmp( pszSymbol, sym_name ) )
			{
				symbol = current_symbol;
				dltable->last_pos = ++i;
				break;
			}
		}

		munmap( file_hdr, dlstat.st_size );

		return symbol ? symbol->address : NULL;
	}
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Sig scanner
//-----------------------------------------------------------------------------

void *CMemoryUtils::FindPattern( module_t hModule, pattern_t * pPattern, unsigned int offset /* = 0 */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nLength = pPattern->length;
		unsigned char *pSignature = &pPattern->signature;

		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - nLength;

		while ( pSearchStart < pSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nLength; i++ )
			{
				if ( pSignature[ i ] != pPattern->ignorebyte && pSignature[ i ] != pSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPattern( module_t hModule, const char *pszPattern, char *pszMask, unsigned int offset /* = 0 */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nMaskLength = strlen( pszMask );

		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - nMaskLength;

		while ( pSearchStart < pSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nMaskLength; i++ )
			{
				if ( pszMask[ i ] != '?' && pszPattern[ i ] != pSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPattern( module_t hModule, const char *pszPattern, unsigned int length, unsigned int offset /* = 0 */, char ignoreByte /* = '0x2A' */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - length;

		while ( pSearchStart < pSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < length; i++ )
			{
				if ( pszPattern[ i ] != ignoreByte && pszPattern[ i ] != pSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPattern( module_t hModule, unsigned char *pPattern, unsigned int length, unsigned int offset /* = 0 */, unsigned char ignoreByte /* = 0x2A */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - length;

		while ( pSearchStart < pSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < length; i++ )
			{
				if ( pPattern[ i ] != ignoreByte && pPattern[ i ] != pSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindString( module_t hModule, const char *pszString, unsigned int offset /* = 0 */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nLength = strlen( pszString );

		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - nLength;

		while ( pSearchStart < pSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nLength; i++ )
			{
				if ( pszString[ i ] != pSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindAddress( module_t hModule, void *pAddress, unsigned int offset /* = 0 */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pSearchStart = (unsigned char *)moduleInfo.pBaseOfDll + offset;
		unsigned char *pSearchEnd = pSearchStart + moduleInfo.SizeOfImage - sizeof( void * );

		while ( pSearchStart < pSearchEnd )
		{
			if ( *(unsigned long *)pSearchStart == (unsigned long)pAddress )
				return (void *)pSearchStart;

			pSearchStart++;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Lookup for a VTable
//-----------------------------------------------------------------------------

void *CMemoryUtils::FindVTable( module_t hModule, const char *pszVTable )
{
#ifdef WIN32
	moduleinfo_t moduleInfo;

	if ( !RetrieveModuleInfo( hModule, &moduleInfo ) )
		return NULL;

	const IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)moduleInfo.pBaseOfDll;
	if ( dos_header->e_magic != IMAGE_DOS_SIGNATURE )
		return NULL;

	const IMAGE_NT_HEADERS *nt_headers = (IMAGE_NT_HEADERS *)( (uint8_t *)moduleInfo.pBaseOfDll + dos_header->e_lfanew );
	if ( nt_headers->Signature != IMAGE_NT_SIGNATURE )
		return NULL;

	uint8_t *rdata_start = NULL, *rdata_end = NULL, *data_start = NULL, *data_end = NULL;

	IMAGE_SECTION_HEADER *section_header = IMAGE_FIRST_SECTION( nt_headers );
	for ( int i = nt_headers->FileHeader.NumberOfSections; i > 0; i--, section_header++ )
	{
		if ( !stricmp( (const char *)section_header->Name, ".rdata" ) )
		{
			rdata_start = (uint8_t *)moduleInfo.pBaseOfDll + section_header->VirtualAddress;
			rdata_end = rdata_start + section_header->SizeOfRawData;
		}
		else if ( !stricmp( (const char *)section_header->Name, ".data" ) )
		{
			data_start = (uint8_t *)moduleInfo.pBaseOfDll + section_header->VirtualAddress;
			data_end = data_start + section_header->SizeOfRawData;
		}
	}

	if ( rdata_start == NULL || data_start == NULL )
		return NULL;

	std::string sRTTITypeDescriptorName = ".?AV";
	sRTTITypeDescriptorName += pszVTable;
	sRTTITypeDescriptorName += "@@";

	// Lookup for the type descriptor struct
	uint8_t *pRTTITypeDescriptor = (uint8_t *)FindStringWithin( hModule, sRTTITypeDescriptorName.c_str(), data_start, data_end );
	if ( pRTTITypeDescriptor == NULL )
		return NULL;

	// Offset to the beginning of the description struct
	pRTTITypeDescriptor -= 0x8;

	uint8_t *pSearch = NULL;
	uint8_t *pStart = rdata_start, *pEnd = rdata_end;
	std::vector<uint8_t *> xrefs;

	// Find all possible xrefs for the type descriptor
	while ( ( pSearch = (uint8_t *)FindAddressWithin( hModule, pStart, pEnd, pRTTITypeDescriptor ) ) != NULL )
	{
		pStart = pSearch + sizeof( void * );
		xrefs.push_back( pSearch );
	}

	if ( xrefs.empty() )
		return NULL;

	for ( uint8_t *xref : xrefs )
	{
		// Doesn't point to the complete object locator
		if ( *(uint32_t *)( xref - ( sizeof( void * ) * 2 ) ) != 0 )
			continue;

		// Offset to the beginning of the struct
		uint8_t *pRTTICompleteObjectLocator = xref - 0xC;
		pRTTICompleteObjectLocator = (uint8_t *)FindAddressWithin( hModule, rdata_start, rdata_end, pRTTICompleteObjectLocator );
		if ( pRTTICompleteObjectLocator == NULL )
			break; // mission failed uh oh

		uint8_t *vftable = pRTTICompleteObjectLocator + sizeof( void * );
		return vftable; // we did good
	}
#endif

	return NULL;
}

//-----------------------------------------------------------------------------
// Find a pattern within a range
//-----------------------------------------------------------------------------

void *CMemoryUtils::FindPatternWithin( module_t hModule, pattern_t * pPattern, void *pSearchStart, void *pSearchEnd )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nLength = pPattern->length;
		unsigned char *pSignature = &pPattern->signature;

		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - nLength;

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nLength; i++ )
			{
				if ( pSignature[ i ] != pPattern->ignorebyte && pSignature[ i ] != pModuleSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPatternWithin( module_t hModule, const char *pszPattern, char *pszMask, void *pSearchStart, void *pSearchEnd )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nMaskLength = strlen( pszMask );

		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - nMaskLength;

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nMaskLength; i++ )
			{
				if ( pszMask[ i ] != '?' && pszPattern[ i ] != pModuleSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPatternWithin( module_t hModule, const char *pszPattern, unsigned int length, void *pSearchStart, void *pSearchEnd, char ignoreByte /* = '0x2A' */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - length;

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < length; i++ )
			{
				if ( pszPattern[ i ] != ignoreByte && pszPattern[ i ] != pModuleSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindPatternWithin( module_t hModule, unsigned char *pPattern, unsigned int length, void *pSearchStart, void *pSearchEnd, unsigned char ignoreByte /* = 0x2A */ )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - length;

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < length; i++ )
			{
				if ( pPattern[ i ] != ignoreByte && pPattern[ i ] != pModuleSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindStringWithin( module_t hModule, const char *pszString, void *pSearchStart, void *pSearchEnd )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned long nLength = strlen( pszString );

		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - nLength;

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			bool bFound = true;

			for ( register unsigned long i = 0; i < nLength; i++ )
			{
				if ( pszString[ i ] != pModuleSearchStart[ i ] )
				{
					bFound = false;
					break;
				}
			}

			if ( bFound )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

void *CMemoryUtils::FindAddressWithin( module_t hModule, void *pSearchStart, void *pSearchEnd, void *pAddress )
{
	moduleinfo_t moduleInfo;

	if ( RetrieveModuleInfo( hModule, &moduleInfo ) )
	{
		unsigned char *pModuleSearchStart = (unsigned char *)moduleInfo.pBaseOfDll;
		unsigned char *pModuleSearchEnd = pModuleSearchStart + moduleInfo.SizeOfImage - sizeof( void * );

		if ( pModuleSearchStart > (unsigned char *)pSearchStart || pModuleSearchEnd < (unsigned char *)pSearchEnd )
			return NULL;

		pModuleSearchStart = (unsigned char *)pSearchStart;
		pModuleSearchEnd = (unsigned char *)pSearchEnd;

		while ( pModuleSearchStart < pModuleSearchEnd )
		{
			if ( *(unsigned long *)pModuleSearchStart == (unsigned long)pAddress )
				return (void *)pModuleSearchStart;

			pModuleSearchStart++;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Read memory's protection in Linux
//-----------------------------------------------------------------------------

#ifndef WIN32

int CMemoryUtils::ReadMemoryProtection( void *pAddress )
{
	static char szBuffer[ 1024 ];
	int iProtection = 0;

	FILE *file = fopen( "/proc/self/maps", "r" );

	while ( fgets( szBuffer, ARRAYSIZE( szBuffer ), file ) )
	{
		// Tokenize format: [START_ADDR]-[END_ADDR] [PAGE_PROT] ....

		char *pszEndPtr1 = NULL;
		char *pszEndPtr2 = NULL;

		unsigned long luStartAddress = strtoul( szBuffer, &pszEndPtr1, 16 );
		unsigned long luEndAddress = strtoul( pszEndPtr1 + 1, &pszEndPtr2, 16 );

		const char *pszProtection = pszEndPtr2 + 1;

		if ( luStartAddress <= (unsigned long)pAddress && (unsigned long)pAddress < luEndAddress )
		{
			if ( pszProtection[ 0 ] == 'r' )
			{
				iProtection |= PROT_READ;
			}

			if ( pszProtection[ 1 ] == 'w' )
			{
				iProtection |= PROT_WRITE;
			}

			if ( pszProtection[ 2 ] == 'x' )
			{
				iProtection |= PROT_EXEC;
			}

			if ( pszProtection[ 3 ] == 'p' )
			{
				iProtection |= PAGE_PRIVATE;
			}
			else if ( pszProtection[ 3 ] == 's' )
			{
				iProtection |= PAGE_SHARED;
			}

			break;
		}
	}

	fclose( file );

	return iProtection;
}

#endif

//-----------------------------------------------------------------------------
// Export Memory API
//-----------------------------------------------------------------------------

static CMemoryUtils g_MemoryUtils;
CMemoryUtils *g_pMemoryUtils = &g_MemoryUtils;
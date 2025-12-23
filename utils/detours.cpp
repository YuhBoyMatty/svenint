// SvenInt (c) Sw1ft
// detours.cpp

#pragma warning(disable : 26812)

#include <vector>

#include "detours.h"
#include "memalloc.h"
#include "udis86/udis86.h"

#ifdef WIN32
#include <Windows.h>
#include <tlhelp32.h>
#endif

#ifndef WIN32
#define DETOURS_PIC_EXECUTABLE
#endif

#define FOR_EACH_DETOUR( iter ) for ( unsigned int iter = 0; iter < m_detours.size(); iter++ )

//-----------------------------------------------------------------------------
// Detour types
//-----------------------------------------------------------------------------

typedef enum
{
	kDetourFunction = 0,
	kDetourVTableFunction,
	kDetourIATFunction
} eDetourType;

//-----------------------------------------------------------------------------
// CDetourFunction
//-----------------------------------------------------------------------------

class CDetourFunction
{
	friend class CDetourContext;

public:
	CDetourFunction( DetourHandle_t handle, CDetourContext *pContext, void *pDetour, void **ppTrampoline );

	bool			IsPaused( void ) const;
	void			*GetDetour( void ) const;
	CDetourContext *GetContext( void ) const;

	void			SetTrampoline( void *pTrampoline );

public:
	bool			operator==( DetourHandle_t handle ) const;

private:
	DetourHandle_t m_handle;
	CDetourContext *m_pContext;

	void *m_pDetour;
	void **m_ppTrampoline;

	bool m_bPaused;
};

//-----------------------------------------------------------------------------
// CDetourContext
//-----------------------------------------------------------------------------

class CDetourContext
{
public:
	CDetourContext( int type, void *pDetourTarget );
	~CDetourContext();

	bool			Init( int iDisasmMinBytes );

	// Pause all detours
	bool			Pause( void );
	bool			Unpause( void );

	void			InstallDetours( void );
	void			RemoveDetours( void );

public:
	int				GetType( void ) const;

	bool			HasDetours( void ) const;
	void			*GetDetourTarget( void ) const;

public:
	CDetourFunction *FindDetourByFunction( void *pDetourFunction );

	CDetourFunction *AddDetour( DetourHandle_t hDetour, void *pDetourFunction, void **ppTrampoline, int iDetourPriority, bool bPaused );
	bool			RemoveDetour( DetourHandle_t hDetour );

	bool			PauseDetour( DetourHandle_t hDetour );
	bool			UnpauseDetour( DetourHandle_t hDetour );

private:
	int m_type;
	std::vector<CDetourFunction *> m_detours;

	bool m_bDetoursAttached;

	void *m_pFunction;
	void *m_pGateway;

	int m_nStolenBytes;
	uint8_t *m_pOriginalBytes;
	uint8_t *m_pPatchedBytes;
};

//-----------------------------------------------------------------------------
// CDetoursAPI implementation
//-----------------------------------------------------------------------------

CDetours::CDetours() : m_DetoursTable( 255 ), m_ContextsTable( 255 )
{
	m_iDetourHandles = 0;
	m_bAutoPause = false;
	m_bGlobalAttach = false;
	m_bSuspendThreads = true;

#ifdef WIN32
	m_dwCurrentThreadID = 0;
	m_dwCurrentProcessID = 0;
#endif
}

CDetours::~CDetours()
{
}

void CDetours::Init( void )
{
#ifdef WIN32
	m_dwCurrentThreadID = (uint32_t)GetCurrentThreadId();
	m_dwCurrentProcessID = (uint32_t)GetCurrentProcessId();
#endif
}

DetourHandle_t CDetours::DetourFunction( void *pFunction,
										 void *pDetourFunction,
										 void **ppOriginalFunction,
										 int iDetourPriority /* = kDetourPriorityNormal */,
										 bool bPause /* = false */,
										 int iDisasmMinBytes /* = 5 */ )
{
	return CreateDetour( kDetourFunction, pFunction, pDetourFunction, ppOriginalFunction, iDetourPriority, bPause, iDisasmMinBytes );
}

DetourHandle_t CDetours::DetourFunctionByName( module_t hModule,
											   const char *pszFunctionName,
											   void *pDetourFunction,
											   void **ppOriginalFunction,
											   int iDetourPriority /* = kDetourPriorityNormal */,
											   bool bPause /* = false */,
											   int iDisasmMinBytes /* = 5 */ )
{
#ifdef WIN32
	void *pFunction = GetProcAddress( (HMODULE)hModule, pszFunctionName );
#else
	void *pFunction = MemoryUtils()->ResolveSymbol( hModule, pszFunctionName );
#endif

	if ( pFunction == NULL )
		return DETOUR_INVALID_HANDLE;

	return CreateDetour( kDetourFunction, pFunction, pDetourFunction, ppOriginalFunction, iDetourPriority, bPause, iDisasmMinBytes );
}

DetourHandle_t CDetours::DetourVirtualFunction( void *pClassInstance,
												int nFunctionIndex,
												void *pDetourFunction,
												void **ppOriginalFunction,
												int iDetourPriority /* = kDetourPriorityNormal */,
												bool bPause /* = false */ )
{
	if ( pClassInstance == NULL )
		return DETOUR_INVALID_HANDLE;

	void *pVTable = *static_cast<void **>( pClassInstance );

	if ( pVTable == NULL )
		return DETOUR_INVALID_HANDLE;

	void *pFunction = (void *)( (uint32_t *)pVTable + nFunctionIndex );

	return CreateDetour( kDetourVTableFunction, pFunction, pDetourFunction, ppOriginalFunction, iDetourPriority, bPause, 5 );
}

bool CDetours::PauseDetour( DetourHandle_t hDetour )
{
	CDetourFunction **ppDetour = m_DetoursTable.Find( hDetour );

	if ( ppDetour )
	{
		return ( *ppDetour )->GetContext()->PauseDetour( hDetour );
	}

	return false;
}

bool CDetours::UnpauseDetour( DetourHandle_t hDetour )
{
	CDetourFunction **ppDetour = m_DetoursTable.Find( hDetour );

	if ( ppDetour )
	{
		return ( *ppDetour )->GetContext()->UnpauseDetour( hDetour );
	}

	return false;
}

bool CDetours::RemoveDetour( DetourHandle_t hDetour )
{
	CDetourFunction **ppDetour = m_DetoursTable.Find( hDetour );

	if ( ppDetour )
	{
		CDetourContext *pContext = ( *ppDetour )->GetContext();
		bool bRemoved = pContext->RemoveDetour( hDetour );

		if ( bRemoved )
		{
			m_DetoursTable.Remove( hDetour );

			if ( !pContext->HasDetours() )
			{
				m_ContextsTable.Remove( pContext->GetDetourTarget() );

				MemFreeInstance( pContext );
				//delete pContext;
			}

			return true;
		}
	}

	return false;
}

bool CDetours::PauseAllDetours( void )
{
	bool bPaused = false;

	for ( int i = 0; i < m_ContextsTable.Count(); i++ )
	{
		HashTableIterator_t it = m_ContextsTable.First( i );

		while ( m_ContextsTable.IsValidIterator( it ) )
		{
			bPaused = m_ContextsTable.ValueAt( i, it )->Pause() || bPaused;
			it = m_ContextsTable.Next( i, it );
		}
	}

	return bPaused;
}

bool CDetours::UnpauseAllDetours( void )
{
	bool bUnpaused = false;

	for ( int i = 0; i < m_ContextsTable.Count(); i++ )
	{
		HashTableIterator_t it = m_ContextsTable.First( i );

		while ( m_ContextsTable.IsValidIterator( it ) )
		{
			bUnpaused = m_ContextsTable.ValueAt( i, it )->Unpause() || bUnpaused;
			it = m_ContextsTable.Next( i, it );
		}
	}

	return bUnpaused;
}

DetourHandle_t CDetours::CreateDetour( int type, void *pFunction, void *pDetourFunction, void **ppOriginalFunction, int iDetourPriority, bool bPause, int iDisasmMinBytes )
{
	if ( pFunction == NULL )
		return DETOUR_INVALID_HANDLE;

	if ( m_bAutoPause )
		bPause = true;

	if ( iDisasmMinBytes < 5 )
	{
		iDisasmMinBytes = 5;
	}

	CDetourContext *pContext = NULL;
	CDetourContext **pContextEntry = m_ContextsTable.Find( pFunction );

	if ( pContextEntry )
	{
		pContext = *pContextEntry;

		if ( !pContext->FindDetourByFunction( pDetourFunction ) )
		{
			CDetourFunction *pDetour = NULL;
			DetourHandle_t hDetour = AllocateDetourHandle();

			if ( pDetour = pContext->AddDetour( hDetour, pDetourFunction, ppOriginalFunction, iDetourPriority, bPause ) )
			{
				m_DetoursTable.Insert( hDetour, pDetour );
				return hDetour;
			}
		}
	}
	else
	{
		pContext = MemAllocInstance( pContext, type, pFunction );
		//pContext = new CDetourContext( type, pFunction );

		if ( !pContext->Init( iDisasmMinBytes ) )
		{
			MemFreeInstance( pContext );
			//delete pContext;
		}
		else
		{
			CDetourFunction *pDetour = NULL;
			DetourHandle_t hDetour = AllocateDetourHandle();

			m_ContextsTable.Insert( pFunction, pContext );
			pDetour = pContext->AddDetour( hDetour, pDetourFunction, ppOriginalFunction, iDetourPriority, bPause );

			if ( pDetour )
			{
				m_DetoursTable.Insert( hDetour, pDetour );
				return hDetour;
			}
		}
	}

	return DETOUR_INVALID_HANDLE;
}

void CDetours::AutoPauseDetours( bool state )
{
	m_bAutoPause = state;
}

void CDetours::BeginGlobalAttach( void )
{
	m_bGlobalAttach = true;

	SuspendThreads();
}

void CDetours::EndGlobalAttach( void )
{
	ResumeThreads();

	m_bGlobalAttach = false;
}

DetourHandle_t CDetours::AllocateDetourHandle( void )
{
	return m_iDetourHandles++;
}

void CDetours::SuspendThreads( void )
{
	if ( !m_bSuspendThreads )
		return;

#ifdef WIN32
	HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, 0 );

	if ( hSnapshot != INVALID_HANDLE_VALUE )
	{
		THREADENTRY32 te;
		te.dwSize = sizeof( te );

		if ( Thread32First( hSnapshot, &te ) )
		{
			do
			{
				if ( te.dwSize >= FIELD_OFFSET( THREADENTRY32, th32OwnerProcessID ) + sizeof( te.th32OwnerProcessID ) )
				{
					if ( (uint32_t)te.th32OwnerProcessID == m_dwCurrentProcessID && (uint32_t)te.th32ThreadID != m_dwCurrentThreadID )
					{
						HANDLE hThread = ::OpenThread( THREAD_ALL_ACCESS, FALSE, te.th32ThreadID );

						if ( hThread != NULL )
						{
							m_SuspendedThreads.push_back( (uint32_t)te.th32ThreadID );

							SuspendThread( hThread );
							CloseHandle( hThread );
						}
					}
				}
				te.dwSize = sizeof( te );

			} while ( Thread32Next( hSnapshot, &te ) );
		}

		CloseHandle( hSnapshot );
	}
#endif
}

void CDetours::ResumeThreads( void )
{
	if ( !m_bSuspendThreads )
		return;

#ifdef WIN32
	for ( int i = m_SuspendedThreads.size() - 1; i >= 0; --i )
	{
		HANDLE hThread = OpenThread( THREAD_ALL_ACCESS, FALSE, m_SuspendedThreads[ i ] );

		if ( hThread != NULL )
		{
			ResumeThread( hThread );
			CloseHandle( hThread );
		}
	}

	m_SuspendedThreads.clear();
#endif
}

//-----------------------------------------------------------------------------
// Export Detours API
//-----------------------------------------------------------------------------

static CDetours g_Detours;
CDetours *g_pDetours = &g_Detours;

//-----------------------------------------------------------------------------
// CDetourFunction implementation
//-----------------------------------------------------------------------------

CDetourFunction::CDetourFunction( DetourHandle_t handle, CDetourContext *pContext, void *pDetour, void **ppTrampoline )
{
	m_handle = handle;

	m_pContext = pContext;
	m_pDetour = pDetour;
	m_ppTrampoline = ppTrampoline;

	m_bPaused = false;
}

inline CDetourContext *CDetourFunction::GetContext( void ) const
{
	return m_pContext;
}

inline void *CDetourFunction::GetDetour( void ) const
{
	return m_pDetour;
}

inline void CDetourFunction::SetTrampoline( void *pTrampoline )
{
	*m_ppTrampoline = pTrampoline;
}

inline bool CDetourFunction::IsPaused( void ) const
{
	return m_bPaused;
}

bool CDetourFunction::operator==( DetourHandle_t handle ) const
{
	return m_handle == handle;
}

//-----------------------------------------------------------------------------
// CDetourContext implementation
//-----------------------------------------------------------------------------

CDetourContext::CDetourContext( int type, void *pDetourTarget )
{
	m_type = type;

	m_bDetoursAttached = false;

	m_pFunction = pDetourTarget;
	m_pGateway = NULL;

	// kDetourFunction
	m_nStolenBytes = 0;
	m_pOriginalBytes = NULL;
	m_pPatchedBytes = NULL;
}

CDetourContext::~CDetourContext()
{
	RemoveDetours();

	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( m_type == kDetourVTableFunction )
			pDetour->SetTrampoline( m_pGateway );
		else
			pDetour->SetTrampoline( m_pFunction );

		MemFree( pDetour );
		//delete pDetour;
	}

	if ( m_type == kDetourFunction )
	{
		if ( m_pOriginalBytes != NULL )
		{
			MemFree( (void *)m_pOriginalBytes );
			m_pOriginalBytes = m_pPatchedBytes = NULL;
		}

		if ( m_pGateway != NULL )
		{
			MemoryUtils()->VirtualFree( m_pGateway, 0, MEM_RELEASE );
			m_pGateway = NULL;
		}
	}

	m_detours.clear();
}

bool CDetourContext::Init( int iDisasmMinBytes )
{
	if ( m_type == kDetourFunction )
	{
		std::vector<void *> calleeFunctions;
		m_nStolenBytes = 0;

		ud_t instruction;
		uint8_t *buffer = (uint8_t *)m_pFunction;

		ud_init( &instruction );
		ud_set_mode( &instruction, 32 );
		ud_set_input_buffer( &instruction, (const uint8_t *)buffer, iDisasmMinBytes <= 15 ? 15 : iDisasmMinBytes ); // 15 - longest x86 instruction

	#ifdef DETOURS_PIC_EXECUTABLE
		uint8_t *pGetPcThunk = NULL;
		uint8_t *pGetPcThunkRet = 0;
		uint8_t cMovOpcode = 0;
	#endif

		// Disassemble for JMP opcode
		do
		{
			int disassembledBytes = ud_disassemble( &instruction );

			// Save callee functions, then we can calc their relative addresses in gateway
			if ( instruction.mnemonic == UD_Icall || instruction.mnemonic == UD_Ijmp )
			{
			#ifdef DETOURS_PIC_EXECUTABLE
				if ( pGetPcThunk == NULL )
				{
					uint8_t *pGetPcThunkAbs = MemoryUtils()->CalcAbsoluteAddress( buffer );
					if ( *pGetPcThunkAbs == 0x8B &&			// mov REGISTER, [esp]
						*( pGetPcThunkAbs + 2 ) == 0x24 &&	// <- esp
						*( pGetPcThunkAbs + 3 ) == 0xC3 )	// ret
					{
						switch ( *( pGetPcThunkAbs + 1 ) )
						{
						case 0x04: // eax
							cMovOpcode = 0xB8;
							break;

						case 0x1C: // ebx
							cMovOpcode = 0xBB;
							break;

						case 0x0C: // ecx
							cMovOpcode = 0xB9;
							break;

						case 0x14: // edx
							cMovOpcode = 0xBa;
							break;

						case 0x34: // esi
							cMovOpcode = 0xBE;
							break;

						case 0x3C: // edi
							cMovOpcode = 0xBF;
							break;

						case 0x2C: // ebp
							cMovOpcode = 0xBD;
							break;
						}
					}

					if ( cMovOpcode != 0 )
					{
						pGetPcThunk = buffer;
						pGetPcThunkRet = buffer + disassembledBytes;
					}
				}
			#endif

				uint32_t luRelativeAddress = *(uint32_t *)( buffer + 1 );
				void *pAbsoluteAddress = (void *)( luRelativeAddress + (uint32_t)buffer + sizeof( void * ) + 1 );

				calleeFunctions.push_back( pAbsoluteAddress );
			}

			buffer += disassembledBytes;
			m_nStolenBytes += disassembledBytes;

		} while ( m_nStolenBytes < iDisasmMinBytes );

		int nNOPs = m_nStolenBytes - 5;

		// Allocate needed bytes once
		m_pOriginalBytes = (uint8_t *)MemAlloc( m_nStolenBytes * 2 );

		if ( !m_pOriginalBytes )
		{
			calleeFunctions.clear();
			return false;
		}

		m_pPatchedBytes = m_pOriginalBytes + m_nStolenBytes;

		memcpy( m_pOriginalBytes, m_pFunction, m_nStolenBytes );
		*m_pPatchedBytes = 0xE9; // JMP opcode; relative address will be calculated later

		// NOP the rest opcodes, just for safe
		if ( nNOPs )
			memset( m_pPatchedBytes + 5, 0x90, nNOPs );

		// Initializaing the gateway

		// Alloc size: m_nStolenBytes + 5 ( JMP [relative offset] )
		uint8_t callBytes[ 5 ] = { 0xE9 };
		m_pGateway = MemoryUtils()->VirtualAlloc( NULL, m_nStolenBytes + sizeof( callBytes ), MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE );

		if ( !m_pGateway )
		{
			calleeFunctions.clear();
			return false;
		}

		memcpy( m_pGateway, m_pOriginalBytes, m_nStolenBytes );

		// Write relative addresses to CALL opcodes
		if ( calleeFunctions.size() > 0 )
		{
			int iter = 0;
			int disassembledBytes = 0, totalDisassembledBytes = 0;

			buffer = (uint8_t *)m_pGateway;

			ud_init( &instruction );
			ud_set_mode( &instruction, 32 );
			ud_set_input_buffer( &instruction, (const uint8_t *)buffer, m_nStolenBytes );

			while ( disassembledBytes = ud_disassemble( &instruction ) )
			{
				totalDisassembledBytes += disassembledBytes;

				if ( instruction.mnemonic == UD_Icall || instruction.mnemonic == UD_Ijmp )
				{
					*(uint32_t *)( buffer + 1 ) = ( (uint32_t)calleeFunctions[ iter ] - ( (uint32_t)buffer + sizeof( void * ) + 1 ) );
					++iter;
				}

				buffer += disassembledBytes;
			}
		}

	#ifdef DETOURS_PIC_EXECUTABLE
		if ( cMovOpcode != 0 )
		{
			const uint32_t ulOffsetToThunk = (uint32_t)pGetPcThunk - (uint32_t)m_pFunction;
			*( (uint8_t *)m_pGateway + ulOffsetToThunk ) = cMovOpcode;
			*(uint32_t *)( (uint8_t *)m_pGateway + ulOffsetToThunk + 1 ) = (uint32_t)pGetPcThunkRet;
		}
	#endif

		// Calc relative address to jump back to original function
		*(uint32_t *)( callBytes + 1 ) = ( (uint32_t)m_pFunction + m_nStolenBytes ) - ( ( (uint32_t)m_pGateway + m_nStolenBytes ) + sizeof( void * ) + 1 );

		// And copy it
		memcpy( (uint8_t *)m_pGateway + m_nStolenBytes, callBytes, sizeof( callBytes ) );

		calleeFunctions.clear();
		return true;
	}
	else if ( m_type == kDetourVTableFunction )
	{
		m_pGateway = *static_cast<void **>( m_pFunction );
		return true;
	}

	return false;
}

bool CDetourContext::Pause( void )
{
	bool bPaused = false;

	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( !pDetour->IsPaused() )
		{
			bPaused = true;
			pDetour->m_bPaused = true;
		}
	}

	if ( bPaused )
	{
		InstallDetours();
	}

	return bPaused;
}

bool CDetourContext::Unpause( void )
{
	bool bUnpaused = false;

	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( pDetour->IsPaused() )
		{
			bUnpaused = true;
			pDetour->m_bPaused = false;
		}
	}

	if ( bUnpaused )
	{
		InstallDetours();
	}

	return bUnpaused;
}

void CDetourContext::InstallDetours( void )
{
	if ( !g_Detours.IsGlobalAttach() )
		g_Detours.SuspendThreads();

	CDetourFunction *pFirstDetour = NULL;
	CDetourFunction *pLastDetour = NULL;

	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( !pDetour->IsPaused() )
		{
			if ( pFirstDetour == NULL )
				pFirstDetour = pDetour;

			if ( pLastDetour != NULL )
			{
				pLastDetour->SetTrampoline( pDetour->GetDetour() );
			}

			pLastDetour = pDetour;
		}
	}

	if ( pFirstDetour == NULL )
	{
		if ( !g_Detours.IsGlobalAttach() )
			g_Detours.ResumeThreads();

		RemoveDetours();
		return;
	}

	int dwProtection;

	if ( m_type == kDetourFunction )
	{
		// Relative address
		*(uint32_t*)( m_pPatchedBytes + 1 ) = (uint32_t)pFirstDetour->GetDetour() - ( (uint32_t)m_pFunction + sizeof( void * ) + 1 );

		MemoryUtils()->VirtualProtect( m_pFunction, m_nStolenBytes, PAGE_EXECUTE_READWRITE, &dwProtection );

		memcpy( m_pFunction, m_pPatchedBytes, m_nStolenBytes );

		MemoryUtils()->VirtualProtect( m_pFunction, m_nStolenBytes, dwProtection, NULL );
	}
	else if ( m_type == kDetourVTableFunction )
	{
		MemoryUtils()->VirtualProtect( m_pFunction, sizeof( void * ), PAGE_EXECUTE_READWRITE, &dwProtection );

		*(void **)( m_pFunction ) = pFirstDetour->GetDetour();

		MemoryUtils()->VirtualProtect( m_pFunction, sizeof( void * ), dwProtection, NULL );
	}

	pLastDetour->SetTrampoline( m_pGateway );
	m_bDetoursAttached = true;

	if ( !g_Detours.IsGlobalAttach() )
		g_Detours.ResumeThreads();
}

void CDetourContext::RemoveDetours( void )
{
	if ( !m_bDetoursAttached )
		return;

	if ( !g_Detours.IsGlobalAttach() )
		g_Detours.SuspendThreads();

	int dwProtection;

	if ( m_type == kDetourFunction )
	{
		MemoryUtils()->VirtualProtect( m_pFunction, m_nStolenBytes, PAGE_EXECUTE_READWRITE, &dwProtection );

		memcpy( m_pFunction, m_pOriginalBytes, m_nStolenBytes );

		MemoryUtils()->VirtualProtect( m_pFunction, m_nStolenBytes, dwProtection, NULL );
	}
	else if ( m_type == kDetourVTableFunction )
	{
		MemoryUtils()->VirtualProtect( m_pFunction, sizeof( void * ), PAGE_EXECUTE_READWRITE, &dwProtection );

		*(void **)( m_pFunction ) = m_pGateway;

		MemoryUtils()->VirtualProtect( m_pFunction, sizeof( void * ), dwProtection, NULL );
	}

	if ( m_type == kDetourVTableFunction )
	{
		FOR_EACH_DETOUR( i )
		{
			CDetourFunction *pDetour = m_detours[ i ];
			pDetour->SetTrampoline( m_pGateway );
		}
	}
	else
	{
		FOR_EACH_DETOUR( i )
		{
			CDetourFunction *pDetour = m_detours[ i ];
			pDetour->SetTrampoline( m_pFunction );
		}
	}

	m_bDetoursAttached = false;

	if ( !g_Detours.IsGlobalAttach() )
		g_Detours.ResumeThreads();
}

inline bool CDetourContext::HasDetours( void ) const
{
	return m_detours.size() > 0;
}

inline void *CDetourContext::GetDetourTarget( void ) const
{
	return m_pFunction;
}

inline int CDetourContext::GetType( void ) const
{
	return m_type;
}

CDetourFunction *CDetourContext::FindDetourByFunction( void *pDetourFunction )
{
	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( pDetour->GetDetour() == pDetourFunction )
			return pDetour;
	}

	return NULL;
}

CDetourFunction *CDetourContext::AddDetour( DetourHandle_t hDetour, void *pDetourFunction, void **ppTrampoline, int iDetourPriority, bool bPaused )
{
	CDetourFunction *pDetour = MemAllocInstance( (CDetourFunction *)NULL, hDetour, this, pDetourFunction, ppTrampoline);
	//CDetourFunction *pDetour = new CDetourFunction( hDetour, this, pDetourFunction, ppTrampoline );

	if ( pDetour == NULL )
		return NULL;

	//m_detours.push_back( pDetour );

	if ( iDetourPriority == kDetourPriorityNormal )
	{
		m_detours.insert( m_detours.begin() + m_detours.size() / 2, pDetour );
	}
	else if ( iDetourPriority == kDetourPriorityLow )
	{
		m_detours.push_back( pDetour );
	}
	else
	{
		m_detours.insert( m_detours.begin(), pDetour );
	}

	if ( bPaused )
		pDetour->m_bPaused = true;
	else
		InstallDetours();

	return pDetour;
}

bool CDetourContext::RemoveDetour( DetourHandle_t hDetour )
{
	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( *pDetour == hDetour )
		{
			bool bPaused = pDetour->IsPaused();

			MemFree( pDetour );
			//delete pDetour;

			m_detours.erase( m_detours.begin() + i );

			if ( !bPaused )
				InstallDetours();

			return true;
		}
	}

	return false;
}

bool CDetourContext::PauseDetour( DetourHandle_t hDetour )
{
	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( *pDetour == hDetour )
		{
			if ( pDetour->IsPaused() )
				return false;

			pDetour->m_bPaused = true;
			InstallDetours();

			return true;
		}
	}

	return false;
}

bool CDetourContext::UnpauseDetour( DetourHandle_t hDetour )
{
	FOR_EACH_DETOUR( i )
	{
		CDetourFunction *pDetour = m_detours[ i ];

		if ( *pDetour == hDetour )
		{
			if ( !pDetour->IsPaused() )
				return false;

			pDetour->m_bPaused = false;
			InstallDetours();

			return true;
		}
	}

	return false;
}

// SvenInt (c) Sw1ft
// detours.h

#ifndef SINT_DETOURS_H
#define SINT_DETOURS_H

#ifdef _WIN32
#pragma once
#endif

#include <vector>

#include "utils/hashtable.h"
#include "memory_utils.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define FUNC_SIGNATURE( retType, callConv, typeName, ... ) typedef retType ( callConv *typeName )( __VA_ARGS__ )

#define DECLARE_FUNC_PTR( retType, callConv, funcName, ... ) typedef retType ( callConv *(funcName##Fn) )( __VA_ARGS__ ); funcName##Fn funcName = 0
#define DECLARE_FUNC_PTR_STATIC( retType, callConv, funcName, ... ) typedef retType ( callConv *(funcName##Fn) )( __VA_ARGS__ ); static funcName##Fn funcName = 0
#define DECLARE_FUNC( retType, callConv, funcName, ... ) retType callConv funcName( __VA_ARGS__ )

#define DECLARE_CLASS_FUNC( retType, funcName, thisPtr, ... ) static retType __fastcall funcName( thisPtr, void *edx, __VA_ARGS__ )

#define GET_FUNC_PTR( funcName ) (void **)&funcName

#define DECLARE_HOOK( retType, callConv, funcName, ... ) \
	typedef retType ( callConv *funcName##Fn )( __VA_ARGS__ ); \
	static retType callConv HOOKED_##funcName( __VA_ARGS__ ); \
	funcName##Fn ORIG_##funcName = 0

#define DECLARE_CLASS_HOOK( retType, funcName, thisPtr, ... ) \
	typedef retType ( __thiscall *funcName##Fn )( thisPtr, __VA_ARGS__ ); \
	static retType __fastcall HOOKED_##funcName( thisPtr, void *edx, __VA_ARGS__ ); \
	funcName##Fn ORIG_##funcName = 0

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class CDetourFunction;
class CDetourContext;

//-----------------------------------------------------------------------------
// Detour handle
//-----------------------------------------------------------------------------

typedef int DetourHandle_t;
#define DETOUR_INVALID_HANDLE (DetourHandle_t)(-1)

//-----------------------------------------------------------------------------
// CDetours interface
//-----------------------------------------------------------------------------

class CDetours
{
	friend class CDetourContext;

public:
	CDetours();
	~CDetours();

	void Init( void );

	//-----------------------------------------------------------------------------
	// Hook function
	// 
	// @param ppOriginalFunction - pointer to pointer to original function to call
	// @param iDisasmMinBytes - minimum bytes to steal from original function, it
	// can be helpful if detour's trampoline crashes (i.e. jump-short opcode causes it)
	// ToDo: fix it..
	//-----------------------------------------------------------------------------

	DetourHandle_t DetourFunction( void *pFunction, void *pDetourFunction, void **ppOriginalFunction, bool bPause = false, int iDisasmMinBytes = 5 );

	//-----------------------------------------------------------------------------
	// Find function from import address table (using Sys_GetProcAddress) and hook it
	//-----------------------------------------------------------------------------

	DetourHandle_t DetourFunctionByName( module_t hModule, const char *pszFunctionName, void *pDetourFunction, void **ppOriginalFunction, bool bPause = false, int iDisasmMinBytes = 5 );

	//-----------------------------------------------------------------------------
	// Hook function from virtual methods table
	//-----------------------------------------------------------------------------

	DetourHandle_t DetourVirtualFunction( void *pClassInstance, int nFunctionIndex, void *pDetourFunction, void **ppOriginalFunction, bool bPause = false );

	//-----------------------------------------------------------------------------
	// Pause function. Returns true if success
	//-----------------------------------------------------------------------------

	bool PauseDetour( DetourHandle_t hDetour );

	//-----------------------------------------------------------------------------
	// Unpause function. Returns true if success
	//-----------------------------------------------------------------------------

	bool UnpauseDetour( DetourHandle_t hDetour );

	//-----------------------------------------------------------------------------
	// Remove/unhook function
	//-----------------------------------------------------------------------------

	bool RemoveDetour( DetourHandle_t hDetour );

	//-----------------------------------------------------------------------------
	// Pause all registered detours
	//-----------------------------------------------------------------------------

	bool PauseAllDetours( void );

	//-----------------------------------------------------------------------------
	// Unpause all registered detours
	//-----------------------------------------------------------------------------

	bool UnpauseAllDetours( void );
	
	//-----------------------------------------------------------------------------
	// When creating detours, auto pause them
	//-----------------------------------------------------------------------------

	void AutoPauseDetours( bool state );
	
	//-----------------------------------------------------------------------------
	// Forcibly suspend and resume all threads in one go to attach all pending detours
	//-----------------------------------------------------------------------------

	void BeginGlobalAttach( void );
	void EndGlobalAttach( void );
	inline bool IsGlobalAttach( void ) const { return m_bGlobalAttach; }
	
	inline void EnableSuspendThreads( bool state ) { m_bSuspendThreads = state; }
	inline bool IsSuspendThreadsEnabled( void ) const { return m_bSuspendThreads; }

private:
	DetourHandle_t CreateDetour( int type, void *pFunction, void *pDetourFunction, void **ppOriginalFunction, bool bPause, int iDisasmMinBytes );
	DetourHandle_t AllocateDetourHandle( void );

	void SuspendThreads( void );
	void ResumeThreads( void );

private:
	CHashTable<DetourHandle_t, CDetourFunction *> m_DetoursTable;
	CHashTable<void *, CDetourContext *> m_ContextsTable;

#ifdef WIN32
	std::vector<uint32_t> m_SuspendedThreads;
	uint32_t m_dwCurrentThreadID;
	uint32_t m_dwCurrentProcessID;
#endif

	int m_iDetourHandles;
	bool m_bAutoPause;
	bool m_bGlobalAttach;
	bool m_bSuspendThreads;
};

extern CDetours *g_pDetours;
inline CDetours *Detours() { return g_pDetours; }

#endif // SINT_DETOURS_H
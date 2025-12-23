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

#ifdef _WIN32
#define DECLARE_CALLCONV( callConv ) callConv
#else
#define DECLARE_CALLCONV( callConv ) __attribute__((callConv))
#endif

#ifdef _WIN32
#define CALLCONV_CDECL __cdecl
#define CALLCONV_STDCALL __stdcall
#define CALLCONV_FASTCALL __fastcall
#define CALLCONV_THISCALL __thiscall
#else
#define CALLCONV_CDECL __attribute__((cdecl))
#define CALLCONV_STDCALL __attribute__((__stdcall))
#define CALLCONV_FASTCALL __attribute__((__fastcall))
#define CALLCONV_THISCALL __attribute__((__thiscall))
#endif

#define FUNC_SIGNATURE( retType, callConv, typeName, ... ) typedef retType ( callConv *typeName )( __VA_ARGS__ )

#define DECLARE_FUNC_PTR( retType, callConv, funcName, ... ) typedef retType ( callConv *funcName##Fn )( __VA_ARGS__ ); funcName##Fn funcName = 0
#define DECLARE_FUNC_PTR_STATIC( retType, callConv, funcName, ... ) typedef retType ( callConv *funcName##Fn )( __VA_ARGS__ ); static funcName##Fn funcName = 0
#define DECLARE_FUNC( retType, callConv, funcName, ... ) retType callConv funcName( __VA_ARGS__ )

#ifdef _WIN32
#define DECLARE_CLASS_FUNC( retType, funcName, thisPtr, ... ) static retType CALLCONV_FASTCALL funcName( thisPtr, void *edx, __VA_ARGS__ )
#else
#define DECLARE_CLASS_FUNC( retType, funcName, thisPtr, ... ) static retType CALLCONV_FASTCALL funcName( thisPtr, void *edx, ##__VA_ARGS__ )
#endif

#define GET_FUNC_PTR( funcName ) (void **)&funcName

#define DECLARE_HOOK( retType, callConv, funcName, ... ) \
	typedef retType ( callConv *funcName##Fn )( __VA_ARGS__ ); \
	static retType callConv HOOKED_##funcName( __VA_ARGS__ ); \
	funcName##Fn ORIG_##funcName = 0

#ifdef _WIN32
#define DECLARE_CLASS_HOOK( retType, funcName, thisPtr, ... ) \
	typedef retType ( CALLCONV_THISCALL *funcName##Fn )( thisPtr, __VA_ARGS__ ); \
	static retType CALLCONV_FASTCALL HOOKED_##funcName( thisPtr, void *edx, __VA_ARGS__ ); \
	funcName##Fn ORIG_##funcName = 0
#else
#define DECLARE_CLASS_HOOK( retType, funcName, thisPtr, ... ) \
	typedef retType ( CALLCONV_FASTCALL *funcName##Fn )( thisPtr, void *edx, ##__VA_ARGS__ ); \
	static retType CALLCONV_FASTCALL HOOKED_##funcName( thisPtr, void *edx, ##__VA_ARGS__ ); \
	funcName##Fn ORIG_##funcName = 0
#endif

#ifdef WIN32
#define ARG_THISPTR( thisptr ) thisptr // thiscall
#else
#define ARG_THISPTR( thisptr ) thisptr, edx // fastcall
#endif

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
// Detour priorities
//-----------------------------------------------------------------------------

typedef enum
{
	kDetourPriorityNormal = 0,	// place hook in the middle of the call chain ( default priority )
	kDetourPriorityLow,			// place hook at the bottom of the call chain ( will be one of the last to be called )
	kDetourPriorityHigh			// place hook at the top of the call chain ( will be one of the first to be called )
} EDetourPriority;

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

	DetourHandle_t DetourFunction( void *pFunction,
								   void *pDetourFunction,
								   void **ppOriginalFunction,
								   int iDetourPriority = kDetourPriorityNormal,
								   bool bPause = false,
								   int iDisasmMinBytes = 5 );

	//-----------------------------------------------------------------------------
	// Find function from import address table (using Sys_GetProcAddress) and hook it
	//-----------------------------------------------------------------------------

	DetourHandle_t DetourFunctionByName( module_t hModule,
										 const char *pszFunctionName,
										 void *pDetourFunction,
										 void **ppOriginalFunction,
										 int iDetourPriority = kDetourPriorityNormal,
										 bool bPause = false,
										 int iDisasmMinBytes = 5 );

	//-----------------------------------------------------------------------------
	// Hook function from virtual methods table
	//-----------------------------------------------------------------------------

	DetourHandle_t DetourVirtualFunction( void *pClassInstance,
										  int nFunctionIndex,
										  void *pDetourFunction,
										  void **ppOriginalFunction,
										  int iDetourPriority = kDetourPriorityNormal,
										  bool bPause = false );

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
	DetourHandle_t CreateDetour( int type, void *pFunction, void *pDetourFunction, void **ppOriginalFunction, int iDetourPriority, bool bPause, int iDisasmMinBytes );
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

// SvenInt (c) Sw1ft
// hooks.h

#ifndef SINT_HOOKS_MODULE_H
#define SINT_HOOKS_MODULE_H

#ifdef _WIN32
#pragma once
#endif

#include "utils/detours.h"

//-----------------------------------------------------------------------------
// Hooks module
//-----------------------------------------------------------------------------

class CHooksModule
{
public:
	CHooksModule();

	bool			Init( void );
	void			Shutdown( void );

	void			AttachDetours( void );
	void			DeattachDetours( void );

private:
	bool			AttachDetour( const char *pszFunctionName, void *pFunction, void *pDetourFunction, void **ppOriginalFunction );
	bool			AttachDetour( const char *pszFunctionName, void *pClassInstance, int vtidx, void *pDetourFunction, void **ppOriginalFunction );

private:
	std::vector<DetourHandle_t> m_hDetours;
};

namespace Modules { extern CHooksModule *hooks; }

#endif // SINT_HOOKS_MODULE_H

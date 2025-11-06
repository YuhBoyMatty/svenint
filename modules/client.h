// SvenInt (c) Sw1ft
// client.h

#ifndef SINT_CLIENT_MODULE_H
#define SINT_CLIENT_MODULE_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Client module
//-----------------------------------------------------------------------------

class CClientModule
{
public:
	CClientModule();

	bool			Init( void );
	void			Shutdown( void );
};

namespace Modules { extern CClientModule *client; }

#endif // SINT_CLIENT_MODULE_H

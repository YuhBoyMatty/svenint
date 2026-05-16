// SvenInt (c) Sw1ft
// player_keyspam.cpp

#include "stdafx.h"
#include "player_keyspam.h"
#ifndef WIN32
#include <SDL.h>
#endif

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CKeySpam, keyspam, "Player", "Key Spam" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CKeySpam::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( *key_dest == 1 )
		return kHookContinue;

	bool bSpam = !m_pHoldMode->GetBool();

	if ( m_pSpamE->GetBool() )
	{
		static bool key_down = true;

	#ifdef WIN32
		if ( bSpam || GetAsyncKeyState( 0x45 ) ) // E
	#else
		if ( bSpam || Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_E ) ) // E
	#endif
		{
			if ( key_down )
				cl_enginefuncs->pfnClientCmd( "-use" );
			else
				cl_enginefuncs->pfnClientCmd( "+use" );

			key_down = !key_down;
		}
		else
		{
			cl_enginefuncs->pfnClientCmd( "-use" );
			key_down = false;
		}
	}

	if ( m_pSpamW->GetBool() )
	{
		static bool key_down = true;

	#ifdef WIN32
		if ( bSpam || GetAsyncKeyState( 0x57 ) ) // W
	#else
		if ( bSpam || Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_W ) ) // W
	#endif
		{
			if ( key_down )
				cl_enginefuncs->pfnClientCmd( "-forward" );
			else
				cl_enginefuncs->pfnClientCmd( "+forward" );

			key_down = !key_down;
		}
		else
		{
			cl_enginefuncs->pfnClientCmd( "-forward" );
			key_down = false;
		}
	}

	if ( m_pSpamS->GetBool() )
	{
		static bool key_down = true;

	#ifdef WIN32
		if ( bSpam || GetAsyncKeyState( 0x53 ) ) // S
	#else
		if ( bSpam || Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_S ) ) // S
	#endif
		{
			if ( key_down )
				cl_enginefuncs->pfnClientCmd( "-back" );
			else
				cl_enginefuncs->pfnClientCmd( "+back" );

			key_down = !key_down;
		}
		else
		{
			cl_enginefuncs->pfnClientCmd( "-back" );
			key_down = false;
		}
	}

	if ( m_pSpamCTRL->GetBool() )
	{
		static bool key_down = true;

	#ifdef WIN32
		if ( bSpam || GetAsyncKeyState( VK_LCONTROL ) ) // CTRL
	#else
		if ( bSpam || Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_LCTRL ) ) // CTRL
	#endif
		{
			if ( key_down )
				cl_enginefuncs->pfnClientCmd( "-duck" );
			else
				cl_enginefuncs->pfnClientCmd( "+duck" );

			key_down = !key_down;
		}
		else
		{
			cl_enginefuncs->pfnClientCmd( "-duck" );
			key_down = false;
		}
	}

#ifdef WIN32
	if ( m_pSpamQ->GetBool() && ( bSpam || GetAsyncKeyState( 0x51 ) ) ) // Q
#else
	if ( m_pSpamQ->GetBool() && ( bSpam || Modules::menu->SDL_IsKeyPressed( SDL_SCANCODE_Q ) ) ) // Q
#endif
	{
		cl_enginefuncs->pfnClientCmd( "lastinv" );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CKeySpam::CKeySpam( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pHoldMode = NULL;
	m_pSpamCTRL = NULL;
	m_pSpamW = NULL;
	m_pSpamS = NULL;
	m_pSpamE = NULL;
	m_pSpamQ = NULL;

	key_dest = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CKeySpam::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CKeySpam::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );

	cl_enginefuncs->pfnClientCmd( "-duck;-forward;-back;-use" );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CKeySpam::Load( void )
{
	ud_t inst;
	bool bOK = true;

	Modules::menu->BindFeature( this );

	m_pHoldMode = Modules::menu->AddParamBool( this, "Hold", NULL, true );
	m_pSpamCTRL = Modules::menu->AddParamBool( this, "SpamCTRL", NULL, false );
	m_pSpamW = Modules::menu->AddParamBool( this, "SpamW", NULL, false );
	m_pSpamS = Modules::menu->AddParamBool( this, "SpamS", NULL, false );
	m_pSpamE = Modules::menu->AddParamBool( this, "SpamE", NULL, false );
	m_pSpamQ = Modules::menu->AddParamBool( this, "SpamQ", NULL, false );

	cmd_function_t *messagemode = cvar->FindCmd( "messagemode" );
	if ( messagemode == NULL )
		return false;

#ifdef WIN32
	MemoryUtils()->InitDisasm( &inst, messagemode->function, 32, 16 );
	if ( MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Imov &&
			 inst.operand[ 0 ].type == UD_OP_MEM &&
			 inst.operand[ 1 ].type == UD_OP_IMM &&
			 inst.operand[ 1 ].lval.udword == 1 )
		{
			key_dest = reinterpret_cast<int *>( inst.operand[ 0 ].lval.udword );
		}
	}
#else
	uint8_t *p = (uint8_t *)messagemode->function;
	uint32_t ulRelocOffset = 0;
	int iDisassembledBytes = 0;

	MemoryUtils()->InitDisasm( &inst, messagemode->function, 32, 32 );
	while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
	{
		if ( inst.mnemonic == UD_Icall && ulRelocOffset == 0 )
		{
			ulRelocOffset = (uint32_t)( p + iDisassembledBytes );
		}
		else if ( inst.mnemonic == UD_Iadd && inst.operand[ 0 ].type == UD_OP_REG &&
				  inst.operand[ 0 ].base == UD_R_EBX && inst.operand[ 1 ].type == UD_OP_IMM )
		{
			ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
		}
		else if ( ulRelocOffset != 0 && inst.mnemonic == UD_Ilea &&
				  inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_EAX &&
				  inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].base == UD_R_EBX )
		{
			ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
			key_dest = reinterpret_cast<int *>( ulRelocOffset );
			break;
		}

		p += iDisassembledBytes;
	}
#endif

	FEATURE_CHECK_SYMBOL( key_dest, "key_dest" );
	return true;
}

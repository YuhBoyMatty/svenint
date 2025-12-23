// SvenInt (c) Sw1ft
// mov_use_key.cpp

#include "stdafx.h"
#include "mov_use_key.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CUseKey, usekey, "Movement", "Use Key" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CUseKey::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( !bPostCall )
	{
		m_flOldValue = cl_movespeedkey->value;

		if ( in_speed != NULL && in_speed->state & 0x1 )
			return kHookContinue;

		if ( m_pCustom->GetBool() )
		{
			cl_movespeedkey->value = m_pCustomValue->GetFloat();
		}
		else if ( m_pOGStyle->GetBool() )
		{
			cl_movespeedkey->value = 0.14f;
		}
		else if ( m_pNoSlowdown->GetBool() )
		{
			cl_movespeedkey->value = 1.f;
		}
	}
	else
	{
		cl_movespeedkey->value = m_flOldValue;
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CUseKey::CUseKey( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pOGStyle = NULL;
	m_pNoSlowdown = NULL;
	m_pCustom = NULL;
	m_pCustomValue = NULL;

	in_speed = NULL;
	cl_movespeedkey = NULL;
	m_flOldValue = 0.3f;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CUseKey::OnEnable( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CUseKey::OnDisable( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CUseKey::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pOGStyle = Modules::menu->AddParamBool( this, "OGStyle", "+use OG Style ( approximate )", true );
	m_pNoSlowdown = Modules::menu->AddParamBool( this, "NoSlowdown", NULL, false );
	m_pCustom = Modules::menu->AddParamBool( this, "Custom", NULL, false );
	m_pCustomValue = Modules::menu->AddParamFloat( this, "CustomValue", NULL, 1.f, 0.f, 1.f );

	bool bOK = true;

	if ( SVEN_VERSION() >= SVENINT_VERSION_CHECK( 5, 26, 0 ) )
	{
		cmd_function_t *speed = cvar->FindCmd( "+speed" ); // thank you devs for removing g_kbkeys
		if ( speed != NULL && speed->function != NULL )
		{
		#ifdef WIN32 
			if ( *(uint8_t *)speed->function == 0x68 ) // PUSH opcode
				in_speed = *(kbutton_t **)( (uint8_t *)speed->function + 1 );
		#else
			ud_t inst;

			uint8_t *p = (uint8_t *)speed->function;
			uint32_t ulRelocOffset = 0;
			int iDisassembledBytes = 0;

			MemoryUtils()->InitDisasm( &inst, speed->function, 32, 20 );
			while ( iDisassembledBytes = MemoryUtils()->Disassemble( &inst ) )
			{
				if ( inst.mnemonic == UD_Icall && ulRelocOffset == 0 )
				{
					ulRelocOffset = (uint32_t)( p + iDisassembledBytes );
				}
				else if ( inst.mnemonic == UD_Iadd && inst.operand[ 0 ].type == UD_OP_REG &&
						  inst.operand[ 0 ].base == UD_R_EAX && inst.operand[ 1 ].type == UD_OP_IMM )
				{
					ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
				}
				else if ( ulRelocOffset != 0 && inst.mnemonic == UD_Ilea &&
						  inst.operand[ 0 ].type == UD_OP_REG && inst.operand[ 0 ].base == UD_R_EDX &&
						  inst.operand[ 1 ].type == UD_OP_MEM && inst.operand[ 1 ].base == UD_R_EAX )
				{
					ulRelocOffset += (uint32_t)inst.operand[ 1 ].lval.udword;
					in_speed = reinterpret_cast<kbutton_t *>( ulRelocOffset );
					break;
				}

				p += iDisassembledBytes;
			}
		#endif
		}
	}
	else
	{
		in_speed = cl_funcs->KB_Find( "in_speed" );
	}

	FEATURE_CHECK_SYMBOL_STATUS( in_speed, "in_speed" );
	if ( !bOK )
		PrintWarning2( "Expect incorrect work when holding Shift\n" );

	cl_movespeedkey = cvar->FindCvar( "cl_movespeedkey" );
	FEATURE_CHECK_SYMBOL( cl_movespeedkey, "cl_movespeedkey" );

	return true;
}
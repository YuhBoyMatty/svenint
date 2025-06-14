// The code here is so fucking mess but I have no intention to refactor it LOL

#pragma warning(disable : 6011)

#include <vector>
#include <Windows.h>
#include <tlhelp32.h>

#include <base_feature.h>
#include <dbg.h>

#include <ISvenModAPI.h>
#include <IDetoursAPI.h>
#include <IMemoryUtils.h>

#include "../patterns.h"
#include "../game/utils.h"
#include "../utils/patcher.h"

//-----------------------------------------------------------------------------
// Mem. patchers
//-----------------------------------------------------------------------------

MEMORY_PATCHER(GaussTertiaryAttack);
MEMORY_PATCHER(MinigunTertiaryAttack);
MEMORY_PATCHER(HandGrenadeTertiaryAttack);
MEMORY_PATCHER(ShockRifleTertiaryAttack);

MEMORY_PATCHER(GaussTertiaryAttack_Server);
MEMORY_PATCHER(MinigunTertiaryAttack_Server);
MEMORY_PATCHER(HandGrenadeTertiaryAttack_Server);
MEMORY_PATCHER(ShockRifleTertiaryAttack_Server);
MEMORY_PATCHER(GluonGunTertiaryAttack_Server);

static bool s_bTertiaryAttackGlitchInitialized = false;
static bool s_bTertiaryAttackGlitchInitialized_Server = false;

static BYTE s_TertiaryAttackPatchedBytes[] =
{
	0xC3, // RETURN
	0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 // NOP
};

//-----------------------------------------------------------------------------
// what
//-----------------------------------------------------------------------------

// xWhitey: Make me 1
#define HOOK_INTEPR_BOUND ( 0 )

DECLARE_HOOK( void, __cdecl, ORIG_CL_ComputeClientInterpAmount, usercmd_t * );

constexpr unsigned int patchInterpDupeSize = 0x1DF;

static float *pflhost_frametime = NULL;
static float *pflg_flInterpolationAmount = NULL;

static void *pfnCL_ComputeClientInterpAmount = NULL;
static DetourHandle_t hCL_ComputeClientInterpAmount = NULL;

//-----------------------------------------------------------------------------
// Patches module feature
//-----------------------------------------------------------------------------

class CPatchesModule : public CBaseFeature
{
public:
	CPatchesModule();

	virtual bool Load();
	virtual void PostLoad();

	virtual void Unload();

public:
	bool PatchInterp();

	void SuspendThreads();
	void ResumeThreads();

private:
	void *m_pDupedInterpClamp;
	void *m_pInterpClampBegin;

	std::vector<DWORD> m_SuspendedThreads;
	DWORD m_dwCurrentThreadID;
	DWORD m_dwCurrentProcessID;
};

//-----------------------------------------------------------------------------
// Tertiary attack junk
//-----------------------------------------------------------------------------

void EnableTertiaryAttackGlitch()
{
	GaussTertiaryAttack.Patch();
	MinigunTertiaryAttack.Patch();
	HandGrenadeTertiaryAttack.Patch();
	ShockRifleTertiaryAttack.Patch();
}

void DisableTertiaryAttackGlitch()
{
	GaussTertiaryAttack.Unpatch();
	MinigunTertiaryAttack.Unpatch();
	HandGrenadeTertiaryAttack.Unpatch();
	ShockRifleTertiaryAttack.Unpatch();
}

bool IsTertiaryAttackGlitchPatched()
{
	return GaussTertiaryAttack.IsPatched();
}

bool IsTertiaryAttackGlitchInit()
{
	return s_bTertiaryAttackGlitchInitialized;
}

void EnableTertiaryAttackGlitch_Server() // what's the point of loading 'server' library when you create a server????
{
	GaussTertiaryAttack_Server.Patch();
	MinigunTertiaryAttack_Server.Patch();
	HandGrenadeTertiaryAttack_Server.Patch();
	ShockRifleTertiaryAttack_Server.Patch();
	GluonGunTertiaryAttack_Server.Patch();
}

void DisableTertiaryAttackGlitch_Server()
{
	GaussTertiaryAttack_Server.Unpatch();
	MinigunTertiaryAttack_Server.Unpatch();
	HandGrenadeTertiaryAttack_Server.Unpatch();
	ShockRifleTertiaryAttack_Server.Unpatch();
	GluonGunTertiaryAttack_Server.Unpatch();
}

bool IsTertiaryAttackGlitchPatched_Server()
{
	return GaussTertiaryAttack_Server.IsPatched();
}

bool IsTertiaryAttackGlitchInit_Server()
{
	return s_bTertiaryAttackGlitchInitialized_Server;
}

static bool InitTertiaryAttackPatch(CPatcher &patcher, void *pfnTertiaryAttack)
{
	ud_t instruction;

	MemoryUtils()->InitDisasm(&instruction, pfnTertiaryAttack, 32, 30);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_REG && instruction.operand[1].type == UD_OP_MEM)
	{
		MemoryUtils()->Disassemble(&instruction);

		if (instruction.mnemonic == UD_Ijmp && instruction.operand[0].type == UD_OP_MEM)
		{
			patcher.Init(pfnTertiaryAttack, s_TertiaryAttackPatchedBytes, sizeof(s_TertiaryAttackPatchedBytes));
			return true;
		}
		else
		{
			Warning("InitTertiaryAttackPatch: invalid instruction #2\n");
			return false;
		}
	}

	Warning("InitTertiaryAttackPatch: invalid instruction #1\n");
	return false;
}

static void InitTertiaryAttackGlitch()
{
	ud_t instruction;
	DWORD *dwVTable[] = { NULL, NULL, NULL, NULL };
	
	constexpr unsigned long tertiaryAttackOffset = 150;
	
	constexpr unsigned long offset1 = 0x63;
	constexpr unsigned long offset2 = 0x63;
	constexpr unsigned long offset3 = 0x63;
	
#if defined(SC_5_26)
	constexpr unsigned long offset4 = 0x6C;
#else
	constexpr unsigned long offset4 = 0x67;
#endif

	void *weapon_gauss = Sys_GetProcAddress( SvenModAPI()->Modules()->Client, "weapon_gauss" ); // vtable <- (byte *)weapon_gauss + 0x63
	void *weapon_minigun = Sys_GetProcAddress( SvenModAPI()->Modules()->Client, "weapon_minigun" ); // vtable <- (byte *)weapon_minigun + 0x63
	void *weapon_handgrenade = Sys_GetProcAddress( SvenModAPI()->Modules()->Client, "weapon_handgrenade" ); // vtable <- (byte *)weapon_handgrenade + 0x63
	void *weapon_shockrifle = Sys_GetProcAddress( SvenModAPI()->Modules()->Client, "weapon_shockrifle" ); // vtable <- (byte *)weapon_shockrifle + 0x67

	if ( !weapon_gauss || !weapon_minigun || !weapon_handgrenade || !weapon_shockrifle )
	{
		Warning("Tertiary Attack Glitch: missing exported \"weapon_*\" funcs\n");
		return;
	}

	// weapon_gauss
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_gauss + offset1, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[0] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch: can't init weapon_gauss\n");
		
	// weapon_minigun
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_minigun + offset2, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[1] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch: can't init weapon_minigun\n");
		
	// weapon_handgrenade
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_handgrenade + offset3, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[2] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch: can't init weapon_handgrenade\n");
		
	// weapon_shockrifle
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_shockrifle + offset4, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[3] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch: can't init weapon_shockrifle\n");;

	if ( !InitTertiaryAttackPatch(GaussTertiaryAttack, (void *)dwVTable[0][tertiaryAttackOffset]) ) // CBasePlayerWeapon::TertiaryAttack
		return;
	
	if ( !InitTertiaryAttackPatch(MinigunTertiaryAttack, (void *)dwVTable[1][tertiaryAttackOffset]) )
		return;
	
	if ( !InitTertiaryAttackPatch(HandGrenadeTertiaryAttack, (void *)dwVTable[2][tertiaryAttackOffset]) )
		return;
	
	if ( !InitTertiaryAttackPatch(ShockRifleTertiaryAttack, (void *)dwVTable[3][tertiaryAttackOffset]) )
		return;

	s_bTertiaryAttackGlitchInitialized = true;
}

void InitTertiaryAttackGlitch_Server(HMODULE hServerDLL)
{
	ud_t instruction;
	DWORD *dwVTable[] = { NULL, NULL, NULL, NULL, NULL };
	
#if defined(SC_5_26)
	constexpr unsigned long tertiaryAttackOffset = 153;
	
	constexpr unsigned long offset1 = 0x87;
	constexpr unsigned long offset2 = 0x8B;
	constexpr unsigned long offset3 = 0x87;
	constexpr unsigned long offset4 = 0x96;
	constexpr unsigned long offset5 = 0x8F;
#else
	constexpr unsigned long tertiaryAttackOffset = 151;

	constexpr unsigned long offset1 = 0x7B;
	constexpr unsigned long offset2 = 0x7B;
	constexpr unsigned long offset3 = 0x7B;
	constexpr unsigned long offset4 = 0x83;
	constexpr unsigned long offset5 = 0x83;
#endif

	void *weapon_gauss = Sys_GetProcAddress( hServerDLL, "weapon_gauss" ); // vtable <- (byte *)weapon_gauss + 0x7B
	void *weapon_minigun = GetProcAddress( hServerDLL, "weapon_minigun" ); // vtable <- (byte *)weapon_minigun + 0x7B
	void *weapon_handgrenade = GetProcAddress( hServerDLL, "weapon_handgrenade" ); // vtable <- (byte *)weapon_handgrenade + 0x7B
	void *weapon_shockrifle = GetProcAddress( hServerDLL, "weapon_shockrifle" ); // vtable <- (byte *)weapon_shockrifle + 0x83
	void *weapon_egon = GetProcAddress( hServerDLL, "weapon_egon" ); // vtable <- (byte *)weapon_egon + 0x83

	if ( !weapon_gauss || !weapon_minigun || !weapon_handgrenade || !weapon_shockrifle || !weapon_egon )
	{
		Warning("Tertiary Attack Glitch [S]: missing exported \"weapon_*\" funcs\n");
	}

	// weapon_gauss
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_gauss + offset1, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[0] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch [S]: can't init weapon_gauss\n");

	// weapon_minigun
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_minigun + offset2, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[1] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch [S]: can't init weapon_minigun\n");

	// weapon_handgrenade
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_handgrenade + offset3, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[2] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch [S]: can't init weapon_handgrenade\n");

	// weapon_shockrifle
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_shockrifle + offset4, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[3] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch [S]: can't init weapon_shockrifle\n");

	// weapon_egon
	MemoryUtils()->InitDisasm(&instruction, (BYTE *)weapon_egon + offset5, 32, 15);
	MemoryUtils()->Disassemble(&instruction);

	if (instruction.mnemonic == UD_Imov && instruction.operand[0].type == UD_OP_MEM && instruction.operand[1].type == UD_OP_IMM)
		dwVTable[4] = reinterpret_cast<DWORD *>(instruction.operand[1].lval.udword);
	else
		return Warning("Tertiary Attack Glitch [S]: can't init weapon_egon\n");

	if ( !InitTertiaryAttackPatch(GaussTertiaryAttack_Server, (void *)dwVTable[0][tertiaryAttackOffset]) ) // CBasePlayerWeapon::TertiaryAttack
		return;
	
	if ( !InitTertiaryAttackPatch(MinigunTertiaryAttack_Server, (void *)dwVTable[1][tertiaryAttackOffset]) )
		return;
	
	if ( !InitTertiaryAttackPatch(HandGrenadeTertiaryAttack_Server, (void *)dwVTable[2][tertiaryAttackOffset]) )
		return;
	
	if ( !InitTertiaryAttackPatch(ShockRifleTertiaryAttack_Server, (void *)dwVTable[3][tertiaryAttackOffset]) )
		return;
	
	if ( !InitTertiaryAttackPatch(GluonGunTertiaryAttack_Server, (void *)dwVTable[4][tertiaryAttackOffset]) )
		return;

	s_bTertiaryAttackGlitchInitialized_Server = true;
}

//-----------------------------------------------------------------------------
// ex_interp & cl_updaterate patch
//-----------------------------------------------------------------------------

void CPatchesModule::SuspendThreads()
{
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		THREADENTRY32 te;
		te.dwSize = sizeof(te);

		if (Thread32First(hSnapshot, &te))
		{
			do
			{
				if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID))
				{
					if (te.th32OwnerProcessID == m_dwCurrentProcessID && te.th32ThreadID != m_dwCurrentThreadID)
					{
						HANDLE hThread = ::OpenThread(THREAD_ALL_ACCESS, FALSE, te.th32ThreadID);

						if (hThread != NULL)
						{
							m_SuspendedThreads.push_back(te.th32ThreadID);

							SuspendThread(hThread);
							CloseHandle(hThread);
						}
					}
				}
				te.dwSize = sizeof(te);

			} while (Thread32Next(hSnapshot, &te));
		}

		CloseHandle(hSnapshot);
	}
}

void CPatchesModule::ResumeThreads()
{
	for (int i = m_SuspendedThreads.size() - 1; i >= 0; --i)
	{
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, m_SuspendedThreads[i]);

		if (hThread != NULL)
		{
			ResumeThread(hThread);
			CloseHandle(hThread);
		}
	}

	m_SuspendedThreads.clear();
}

bool CPatchesModule::PatchInterp()
{
#if !HOOK_INTEPR_BOUND
	DWORD dwProtection;

	void *pPatchInterpString = MemoryUtils()->FindString(SvenModAPI()->Modules()->Hardware, "cl_updaterate min");

	if (!pPatchInterpString)
	{
		Warning("'Patch Interp' failed initialization\n");
		return false;
	}

	BYTE *pPatchInterp = (BYTE *)MemoryUtils()->FindAddress(SvenModAPI()->Modules()->Hardware, pPatchInterpString);

	if (!pPatchInterp)
	{
		Warning("'Patch Interp' failed initialization #2\n");
		return false;
	}
	
#if defined(SC_5_26)
	constexpr unsigned int patchstart = 0x1F;
#else
	constexpr unsigned int patchstart = 0x1F;
#endif

	m_pInterpClampBegin = (void *)(pPatchInterp - patchstart);
	m_pDupedInterpClamp = (void *)malloc(patchInterpDupeSize);


	if (!m_pDupedInterpClamp)
	{
		Sys_Error("[SvenInt] Failed to allocate memory");
		return false;
	}

	memcpy(m_pDupedInterpClamp, m_pInterpClampBegin, patchInterpDupeSize);
	VirtualProtect(m_pInterpClampBegin, patchInterpDupeSize, PAGE_EXECUTE_READWRITE, &dwProtection);

	// go back to PUSH opcode
	pPatchInterp -= 0x1;

	if (*pPatchInterp != 0x68) // check PUSH opcode
	{
		Warning("'Patch Interp' failed initialization #3\n");
		return false;
	}

	if (*(pPatchInterp - patchstart) != 0x7A) // JP opcode
	{
		Warning("'Patch Interp' failed initialization #4\n");
		return false;
	}

	// Patch cl_updaterate min.
	*(pPatchInterp - patchstart) = 0xEB;
	
	// Go to PUSH string 'cl_updaterate max...'
	pPatchInterp += 0x3F;

	if (*pPatchInterp != 0x68) // check PUSH opcode
	{
		Warning("'Patch Interp' failed initialization #5\n");
		return false;
	}

	if (*(pPatchInterp - patchstart) != 0x7A) // JP opcode
	{
		Warning("'Patch Interp' failed initialization #6\n");
		return false;
	}

	// Patch cl_updaterate max.
	*(pPatchInterp - patchstart) = 0xEB;
	
	// Go to PUSH string 'ex_interp forced up...'
	pPatchInterp += 0x62;

#if defined(SC_5_26)
	if (*pPatchInterp != 0x68) // check PUSH opcode
#else
	if (*pPatchInterp != 0xB8) // check MOV, EAX ... opcode
#endif
	{
		Warning("'Patch Interp' failed initialization #7\n");
		return false;
	}

	if (*(pPatchInterp - 0x8) != 0x7D) // JNL opcode
	{
		Warning("'Patch Interp' failed initialization #8\n");
		return false;
	}

	// Patch ex_interp force up
	*(pPatchInterp - 0x8) = 0xEB;
	
#if defined(SC_5_26)
	constexpr unsigned long forceDownOffset = 0x1C;
	constexpr unsigned long forceDownCheckOpcode = 0x0F;
#else
	constexpr unsigned long forceDownOffset = 0xD;
	constexpr unsigned long forceDownCheckOpcode = 0x7E;
#endif

	if (*(pPatchInterp + forceDownOffset) != forceDownCheckOpcode) // JLE opcode
	{
		Warning("'Patch Interp' failed initialization #9\n");
		return false;
	}

	// Patch ex_interp force down
	*(pPatchInterp + forceDownOffset) = 0xEB; // TODO FIX MEEEEE    0F 8E BF 00 00 00  use JMP opcode

	VirtualProtect(m_pInterpClampBegin, patchInterpDupeSize, dwProtection, &dwProtection);
#endif // #if !HOOK_INTEPR_BOUND

	return true;
}

DECLARE_FUNC(void, __cdecl, HOOKED_CL_ComputeClientInterpAmount, usercmd_t *cmd)
{
	static cvar_t *cl_interp = NULL;
		
	if ( cl_interp == NULL )
	{
		cl_interp = CVar()->FindCvar("cl_interp");
	}
	
	float interp_sec = cl_interp->value;
	
	// CL_DriftInterpolationAmount
	if ( interp_sec != *pflg_flInterpolationAmount )
	{
		float maxmove = *pflhost_frametime * 0.05f;
		float diff = interp_sec - *pflg_flInterpolationAmount;
		
		if ( diff > 0.f )
		{
			if ( diff > maxmove )
				diff = maxmove;
		}
		else
		{
			diff = -diff;
			
			if ( diff > maxmove )
				diff = -maxmove;
		}
		
		*pflg_flInterpolationAmount += diff;
	}
	
#ifndef boundmax
#define boundmax( num, high ) ( (num) < (high) ? (num) : (high) )
#define boundmin( num, low ) ( (num) >= (low) ? (num) : (low) )
#define bound( low, num, high ) ( boundmin( boundmax(num, high), low ) )
#endif // boundmax
	
	interp_sec = bound( 0.f, *pflg_flInterpolationAmount * 1000.f, 100.f );
	cmd->lerp_msec = interp_sec;
}

//-----------------------------------------------------------------------------
// Patches module feature impl
//-----------------------------------------------------------------------------

CPatchesModule::CPatchesModule()
{
	m_pDupedInterpClamp = NULL;
	m_pInterpClampBegin = NULL;

	m_dwCurrentThreadID = 0;
	m_dwCurrentProcessID = 0;
}

bool CPatchesModule::Load()
{
	ud_t inst;

	m_dwCurrentThreadID = GetCurrentThreadId();
	m_dwCurrentProcessID = GetCurrentProcessId();

	pfnCL_ComputeClientInterpAmount = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Hardware, Patterns::Hardware::CL_ComputeClientInterpAmount );

	if ( !pfnCL_ComputeClientInterpAmount )
	{
		Warning("Cannot locate CL_ComputeClientInterpAmount\n");
		return false;
	}
	
	void *phost_frametime = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Hardware, Patterns::Hardware::host_frametime );

	if ( !phost_frametime )
	{
		Warning("Cannot locate host_frametime\n");
		return false;
	}
	
	void *pg_flInterpolationAmount = MemoryUtils()->FindPatternWithin( SvenModAPI()->Modules()->Hardware,
																		Patterns::Hardware::g_flInterpolationAmount,
																		pfnCL_ComputeClientInterpAmount,
																		(unsigned char *)pfnCL_ComputeClientInterpAmount + 0x221 );
	
	void *pNextCmdTime = MemoryUtils()->FindPattern( SvenModAPI()->Modules()->Hardware, Patterns::Hardware::flNextCmdTime );

	if ( !pNextCmdTime )
	{
		Warning("Cannot locate flNextCmdTime\n");
		return false;
	}

	void *pTextureLoadAddress = MemoryUtils()->FindString( SvenModAPI()->Modules()->Hardware, "Texture load: %6.1fms" );

	if ( !pTextureLoadAddress )
	{
		Warning("Cannot locate \"Texture load\" string\n");
		return false;
	}

	void *pTextureLoad = MemoryUtils()->FindAddress( SvenModAPI()->Modules()->Hardware, pTextureLoadAddress );

	if ( !pTextureLoad )
	{
		Warning("Cannot locate \"Texture load\" address\n");
		return false;
	}

	// g_dbGameSpeed
	MemoryUtils()->InitDisasm(&inst, (BYTE *)pTextureLoad - 0xA, 32, 15);
	MemoryUtils()->Disassemble(&inst);

	if (inst.mnemonic == UD_Ifmul && inst.operand[0].type == UD_OP_MEM)
	{
		g_dbGameSpeed = reinterpret_cast<double *>(inst.operand[0].lval.udword);
	}
	else
	{
		Warning("Cannot locate g_dbGameSpeed\n");
		return false;
	}

	// g_flNextCmdTime
	MemoryUtils()->InitDisasm(&inst, (BYTE *)pNextCmdTime, 32, 15);
	MemoryUtils()->Disassemble(&inst);

	if (inst.mnemonic == UD_Ifstp && inst.operand[0].type == UD_OP_MEM)
	{
		g_flNextCmdTime = reinterpret_cast<float *>(inst.operand[0].lval.udword);
	}
	else
	{
		Warning("Cannot locate flNextCmdTime #2\n");
		return false;
	}
	
	// dbRealtime
	MemoryUtils()->InitDisasm(&inst, (BYTE *)g_pEngineFuncs->pNetAPI->SendRequest + 0x88, 32, 15);
	MemoryUtils()->Disassemble(&inst);

	if (inst.mnemonic == UD_Ifld && inst.operand[0].type == UD_OP_MEM)
	{
		dbRealtime = reinterpret_cast<double *>(inst.operand[0].lval.udword);
	}
	else
	{
		Warning("Cannot locate dbRealtime\n");
		return false;
	}
	
	// host_frametime
	pflhost_frametime = *(float **)( (unsigned char *)phost_frametime + 0x1 );

	// g_flInterpolationAmount
	pflg_flInterpolationAmount = *(float **)( (unsigned char *)pg_flInterpolationAmount + 0x1 );

	int dwProtection;
	MemoryUtils()->VirtualProtect(g_dbGameSpeed, sizeof(double), PAGE_EXECUTE_READWRITE, &dwProtection);

	return true;
}

void CPatchesModule::PostLoad()
{
	InitTertiaryAttackGlitch();

	if ( !s_bTertiaryAttackGlitchInitialized )
	{
		Warning("Failed to initialize client-side tertiary attack glitch\n");
	}

	if ( !PatchInterp() )
	{
		Warning("Failed to patch ex_interp and cl_updaterate\n");
	}
	
#if HOOK_INTEPR_BOUND
	hCL_ComputeClientInterpAmount = DetoursAPI()->DetourFunction( pfnCL_ComputeClientInterpAmount,
																HOOKED_CL_ComputeClientInterpAmount,
																GET_FUNC_PTR( ORIG_CL_ComputeClientInterpAmount ) );
#endif // #if HOOK_INTEPR_BOUND
}

void CPatchesModule::Unload()
{
	UTIL_SetGameSpeed(1.0);
	UTIL_SendPacket(true);

	//cvar_t *fps_max = CVar()->FindCvar("fps_max");

	//if (fps_max)
	//	*dbRealtime = 1.f / fps_max->value;

	if ( IsTertiaryAttackGlitchPatched() )
	{
		DisableTertiaryAttackGlitch();
	}

	if ( IsTertiaryAttackGlitchInit_Server() && IsTertiaryAttackGlitchPatched_Server() )
	{
		DisableTertiaryAttackGlitch_Server();
	}

#if !HOOK_INTEPR_BOUND
	if (m_pDupedInterpClamp)
	{
	#if 0
		SuspendThreads();
	#endif

		DWORD dwProtection;
		VirtualProtect(m_pInterpClampBegin, patchInterpDupeSize, PAGE_EXECUTE_READWRITE, &dwProtection);

		memcpy(m_pInterpClampBegin, m_pDupedInterpClamp, patchInterpDupeSize);

		VirtualProtect(m_pInterpClampBegin, patchInterpDupeSize, dwProtection, &dwProtection);

	#if 0
		ResumeThreads();
	#endif

		free(m_pDupedInterpClamp);
	}
#else // !HOOK_INTEPR_BOUND
	DetoursAPI()->RemoveDetour( hCL_ComputeClientInterpAmount );
#endif // #if HOOK_INTEPR_BOUND
}

//-----------------------------------------------------------------------------
// Create singleton
//-----------------------------------------------------------------------------

CPatchesModule g_PatchesModule;
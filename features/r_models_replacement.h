// SvenInt (c) Sw1ft
// r_models_replacement.h

#ifndef SINT_FEATURE_MODELS_REPLACEMENT_H
#define SINT_FEATURE_MODELS_REPLACEMENT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "modules/menu.h"
#include "game/hook_events.h"
#include "utils/detours.h"

//-----------------------------------------------------------------------------
// Structures
//-----------------------------------------------------------------------------

struct player_model_replacement_info_s
{
	uint64_t steamid;
	int random_model;
	bool model_replaced;
};

//-----------------------------------------------------------------------------
// Player models replacement feature
//-----------------------------------------------------------------------------

class CModelsReplacement final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CModelsReplacement( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

public:
	void ReloadRandomModels( void );
	void ReloadTargetPlayers( void );
	void ReloadIgnoredPlayers( void );

	void ResetPlayersInfo( void );
	void ResetLocalPlayerInfo( void );
	void ResetPlayerInfo( int index );
	void CheckPlayerInfo( int index );
	void UpdatePlayerModel( int index );

private:
	CMenuElementButton *m_pResetReplacement;
	CMenuValueBool *m_pReplaceOnSelf;
	CMenuValueBool *m_pReplaceAllPlayers;
	CMenuValueBool *m_pReplacePlayersWithRandom;
	CMenuValueBool *m_pReplaceTargetedPlayers;
	CMenuValueBool *m_pIgnoreSpecifiedPlayers;
	CMenuValueText *m_pReplaceModel;

	uint32_t m_ulModelOffset;
	uint8_t *m_pUserInfo;

	DetourHandle_t m_hNetMsgHook_UpdateUserInfo;
	DetourHandle_t m_hSetupPlayerModel;

	player_model_replacement_info_s m_PlayerModelReplacementInfo[ MAX_CLIENTS ];

	std::vector<std::string> m_RandomModels;

	CHashTable<uint64_t, std::string> m_TargetPlayers;
	CHash<uint64_t> m_IgnorePlayers;
};

EXTERN_FEATURE( CModelsReplacement, modelsreplacement );

#endif // SINT_FEATURE_MODELS_REPLACEMENT_H
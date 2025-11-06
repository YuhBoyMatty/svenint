// SvenInt (c) Sw1ft
// visual_esp.h

#ifndef SINT_FEATURE_ESP_H
#define SINT_FEATURE_ESP_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "misc_entity_list.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// ESP feature
//-----------------------------------------------------------------------------

class CESP final : public CBaseFeature, IHookEventListener, IMenuElementButtonCallback
{
public:
	CESP( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;
	virtual void OnButtonPressed( CMenuElementButton *pButton ) override;

public:
	inline float *GetFriendPlayerColor( void ) { return m_pFriendPlayerColor->GetColor(); }
	inline float *GetEnemyPlayerColor( void ) { return m_pEnemyPlayerColor->GetColor(); }
	inline float *GetFriendColor( void ) { return m_pFriendColor->GetColor(); }
	inline float *GetEnemyColor( void ) { return m_pEnemyColor->GetColor(); }
	inline float *GetItemColor( void ) { return m_pItemColor->GetColor(); }
	inline float *GetNeutralColor( void ) { return m_pNeutralColor->GetColor(); }

private:
	void Draw( void );
	void DrawSoundESP( void );

	void DrawPlayerInfo_Default( int index, int iHealth, bool bIsEntityFriend, float top_mid_x, float top_mid_y, float bottom_mid_x, float bottom_mid_y );
	void DrawEntityInfo_Default( int index, class_info_t classInfo, float bottom_mid_x, float bottom_mid_y, int r, int g, int b );

	void DrawPlayerInfo_SAMP( int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop );
	void DrawEntityInfo_SAMP( int index, class_info_t classInfo, Vector vecTop, int r, int g, int b );

	void DrawPlayerInfo_L4D( int index, int iHealth, bool bDucking, bool bIsEntityFriend, Vector vecTop );
	void DrawEntityInfo_L4D( int index, class_info_t classInfo, Vector vecTop, int r, int g, int b );

	void DrawBox( bool bPlayer, bool bItem, int iHealth, int x, int y, int w, int h, int r, int g, int b );
	void DrawBones( int index, studiohdr_t *pStudioHeader );
	void UpdateBones( int index );

private:
	CMenuElementButton *m_pPlayerStyleDefault;
	CMenuElementButton *m_pPlayerStyleSAMP;
	CMenuElementButton *m_pPlayerStyleL4D;

	CMenuValueBool *m_pDebug;
	CMenuValueBool *m_pOptimize;
	CMenuValueBool *m_pSnapLines;
	CMenuValueBool *m_pOutlineBox;

	CMenuValueBool *m_pShowEntityIndex;
	CMenuValueBool *m_pShowPlayerHealth;
	CMenuValueBool *m_pShowPlayerArmor;
	CMenuValueBool *m_pShowDistance;
	CMenuValueBool *m_pShowEntityName;
	CMenuValueBool *m_pShowPlayerName;
	CMenuValueBool *m_pShowItems;
	CMenuValueBool *m_pShowVisiblePlayers;
	CMenuValueBool *m_pShowSkeleton;
	CMenuValueBool *m_pShowBonesName;
	CMenuValueBool *m_pIgnoreUnknownEnts;

	CMenuValueBool *m_pSoundESP;
	CMenuValueFloat *m_pSoundESPFadeOut;
	CMenuValueColorRGBA *m_pSoundESPColor;

	CMenuValueFloat *m_pDistance;
	CMenuValueInteger *m_pBoxFillAlpha;

	CMenuValueList *m_pDrawBoxType;
	CMenuValueList *m_pDrawPlayerStyle;
	CMenuValueList *m_pDrawEntityStyle;
	CMenuValueList *m_pDrawTargets;
	CMenuValueList *m_pDrawBoxTargets;
	CMenuValueList *m_pDrawDistanceTargets;
	CMenuValueList *m_pDrawSkeletonTargets;

	CMenuValueColorRGB *m_pFriendPlayerColor;
	CMenuValueColorRGB *m_pEnemyPlayerColor;
	CMenuValueColorRGB *m_pFriendColor;
	CMenuValueColorRGB *m_pEnemyColor;
	CMenuValueColorRGB *m_pItemColor;
	CMenuValueColorRGB *m_pNeutralColor;

	void *m_pfnCClient_SoundEngine__PlayFMODSound;
	DetourHandle_t m_hCClient_SoundEngine__PlayFMODSound;
};

EXTERN_FEATURE( CESP, esp );

#endif // SINT_FEATURE_ESP_H
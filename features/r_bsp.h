// SvenInt (c) Sw1ft
// r_bsp.h

#ifndef SINT_FEATURE_BSP_H
#define SINT_FEATURE_BSP_H

#ifdef _WIN32
#pragma once
#endif

#include <unordered_map>

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Entity kv
//-----------------------------------------------------------------------------

typedef std::unordered_map<std::string, std::string> CBspEntityKeyValues;

//-----------------------------------------------------------------------------
// Trigger type
//-----------------------------------------------------------------------------

typedef enum
{
	TRIGGER_ONCE = 0,
	TRIGGER_MULTIPLE,
	TRIGGER_HURT,
	TRIGGER_HURT_HEAL,
	TRIGGER_PUSH,
	TRIGGER_TELEPORT,
	TRIGGER_CHANGELEVEL,
	TRIGGER_ANTIRUSH
} EBspTriggerType;

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

class CBspTriggerEntity
{
public:
	int iType;
	int iModel;

	Vector vecOrigin;
	Vector vecMins;
	Vector vecMaxs;

	Vector vecMidPoint;
	Vector vecDirection;

	union
	{
		int iDamage;
		int iSpeed;
		float flPercentage;
	};
};

class CBspMonsterSpawn
{
public:
	char szClassname[ 48 ];
	Vector vecOrigin;
};

class CBspFuncWall
{
public:
	Vector vecOrigin;
	Vector vecMins;
	Vector vecMaxs;
};

//-----------------------------------------------------------------------------
// Drawing feature
//-----------------------------------------------------------------------------

class CBsp final : public CBaseFeature, IHookEventListener
{
public:
	CBsp( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	inline uint8_t *GetData( void ) const { return m_pBSP; }
	inline const std::vector<CBspTriggerEntity> &GetTriggers( void ) const { return m_triggers; }
	inline const std::vector<CBspMonsterSpawn> &GetSpawns( void ) const { return m_monsterSpawns; }

private:
	float GetPulsatingAlpha( float a, float time, float speed );
	void DrawTrianglesBox( const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs, float r, float g, float b, float a, float width, bool wireframe );

	void Draw( void );
	void DrawTriangles( void );

	void LoadBSP( void );
	bool LoadEntsFromBSP( unsigned char *bsp, lump_t *lump_entities );

	void DeleteBspData( void );

private:
	CMenuValueBool *m_pWireframe;
	CMenuValueBool *m_pShowSpawns;
	CMenuValueBool *m_pShowWalls;
	CMenuValueBool *m_pShowTriggers;
	CMenuValueBool *m_pShowTriggersInfo;
	CMenuValueBool *m_pShowTriggerOnce;
	CMenuValueBool *m_pShowTriggerMultiple;
	CMenuValueBool *m_pShowTriggerHurt;
	CMenuValueBool *m_pShowTriggerHurtHeal;
	CMenuValueBool *m_pShowTriggerPush;
	CMenuValueBool *m_pShowTriggerTeleport;
	CMenuValueBool *m_pShowTriggerChangelevel;
	CMenuValueBool *m_pShowTriggerAntirush;

	CMenuValueBool *m_pTriggerPulsate;
	CMenuValueFloat *m_pTriggerPulsateOffset;
	CMenuValueFloat *m_pTriggerPulsateSpeed;

	CMenuValueColorRGBA *m_pTriggerOnceColor;
	CMenuValueColorRGBA *m_pTriggerMultipleColor;
	CMenuValueColorRGBA *m_pTriggerHurtColor;
	CMenuValueColorRGBA *m_pTriggerHurtHealColor;
	CMenuValueColorRGBA *m_pTriggerPushColor;
	CMenuValueColorRGBA *m_pTriggerTeleportColor;
	CMenuValueColorRGBA *m_pTriggerChangelevelColor;
	CMenuValueColorRGBA *m_pTriggerAntirushColor;

	uint8_t *m_pBSP;

	std::vector<CBspEntityKeyValues> m_ents;
	std::vector<CBspTriggerEntity> m_triggers;
	std::vector<CBspMonsterSpawn> m_monsterSpawns;
	std::vector<CBspFuncWall> m_funcWalls;
};

EXTERN_FEATURE( CBsp, bsp );

#endif // SINT_FEATURE_BSP_H
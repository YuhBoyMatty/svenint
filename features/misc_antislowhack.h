// SvenInt (c) Sw1ft
// misc_antislowhack.h

#ifndef SINT_FEATURE_ANTISLOWHACK_H
#define SINT_FEATURE_ANTISLOWHACK_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"
#include <vector>

//-----------------------------------------------------------------------------
// Satinized command
//-----------------------------------------------------------------------------

struct sanitized_command
{
	std::string sCmdName;
	std::string sRawCmd;
};

//-----------------------------------------------------------------------------
// Anti slowhack feature
//-----------------------------------------------------------------------------

class CAntiSlowhack final : public CBaseFeature
{
public:
	CAntiSlowhack( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

public:
	bool CheckCommandString( char *pszCommand );

private:
	std::vector<char *> *Split( const char *pszCommand, char chDelimiter );

	sanitized_command GetSanitizedCommand( const char *start, const char *end );
	std::vector<sanitized_command> *SanitizeCommand( const char *pszCommand );

	bool IsCommandInPredefinedAllowedList( const char *pszCommand );
	bool IsCommandDangerous( const char *pszCommand );

private:
	CMenuValueList *m_pMode;
	CMenuValueText *m_pWhitelist;
	CMenuValueBool *m_pNotifyPlayers;
	CMenuValueText *m_pNotifyPlayersText;

	DetourHandle_t m_hNetMsgHook_StuffText;
	DetourHandle_t m_hNetMsgHook_Director;
};

EXTERN_FEATURE( CAntiSlowhack, antislowhack );

#endif // SINT_FEATURE_ANTISLOWHACK_H
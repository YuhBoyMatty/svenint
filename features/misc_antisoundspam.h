// SvenInt (c) Sw1ft
// misc_antisoundspam.h

#ifndef SINT_FEATURE_ANTISOUNDSPAM_H
#define SINT_FEATURE_ANTISOUNDSPAM_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "utils/detours.h"
#include "modules/menu.h"
#include "utils/hash.h"

//-----------------------------------------------------------------------------
// AntiSpam structure
//-----------------------------------------------------------------------------

struct anti_sound_spam_t
{
	anti_sound_spam_t() : entindex( 0 ), blocktime( 0.f ), lastplayed( 0.f )
	{
		sound[ 0 ] = '\0';
	}

	int entindex;
	char sound[ 64 ];

	float blocktime;
	float lastplayed;
};

//-----------------------------------------------------------------------------
// Prevents studio sound event from spamming too frequently
//-----------------------------------------------------------------------------

class CAntiSoundSpam final : public CBaseFeature, IHookEventListener
{
public:
	CAntiSoundSpam( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;
	virtual void OnEnable( void ) override;
	virtual void OnDisable( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

private:
	static bool HashMap_Compare( const anti_sound_spam_t &a, const anti_sound_spam_t &b ) { return ( a.entindex == b.entindex && strcmp( a.sound, b.sound ) == 0 ); }
	static unsigned int HashMap_Hash( const anti_sound_spam_t &a ) { return HashStringCaseless( a.sound ); }

private:
	CMenuValueFloat *m_pBlockTime;
	CMenuValueFloat *m_pMinDiffTime;
	CMenuValueFloat *m_pMaxDiffTime;

	CHash<anti_sound_spam_t> m_soundsList;
};

EXTERN_FEATURE( CAntiSoundSpam, antisoundspam );

#endif // SINT_FEATURE_ANTISOUNDSPAM_H
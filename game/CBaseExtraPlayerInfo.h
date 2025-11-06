// SvenInt (c) Sw1ft
// CBaseExtraPlayerInfo.h

#ifndef SINT_CBASEEXTRAPLAYERINFO_H
#define SINT_CBASEEXTRAPLAYERINFO_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: abstract class CBaseExtraPlayerInfo that provides
// compatibility with different SC versions
//-----------------------------------------------------------------------------

class CBaseExtraPlayerInfo
{
public:
	virtual					~CBaseExtraPlayerInfo() {}

	virtual void			*GetStruct( void ) { return 0; }
	virtual int				GetCompatibilityVersion( void ) { return 0; }

	virtual float			GetFrags( int i ) { return 0.f; }
	virtual int				GetDeaths( int i ) { return 0; }
	virtual int				GetPlayerClass( int i ) { return 0; }
	virtual int				GetTeamNumber( int i ) { return 0; }
	virtual char			*GetTeamName( int i ) { return 0; }
	virtual float			GetHealth( int i ) { return 0.f; }
	virtual float			GetArmor( int i ) { return 0.f; }

	virtual void			SetHealth( int i, float health ) { }
};

//-----------------------------------------------------------------------------
// Get CBaseExtraPlayerInfo based on SC version
//-----------------------------------------------------------------------------

CBaseExtraPlayerInfo *GetBaseExtraPlayerInfo( void *pExtraPlayerInfo, int iGameVersion );

#endif // SINT_CBASEEXTRAPLAYERINFO_H

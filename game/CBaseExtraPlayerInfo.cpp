// SvenInt (c) Sw1ft
// CBaseExtraPlayerInfo.cpp

#include "stdafx.h"
#include "CBaseExtraPlayerInfo.h"

//-----------------------------------------------------------------------------
// Dummy extra player info
//-----------------------------------------------------------------------------

static CBaseExtraPlayerInfo gExtraPlayerInfoDummy;

//-----------------------------------------------------------------------------
// CExtraPlayerInfo >= SC 5.26
//-----------------------------------------------------------------------------

class CExtraPlayerInfo_5_26 final : public CBaseExtraPlayerInfo
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<extra_player_info_5_26_t *>( pStruct ); }

	virtual void			*GetStruct( void ) override { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 526; }

	virtual float			GetFrags( int i ) override { return m_pStruct[ i ].frags; }
	virtual int				GetDeaths( int i ) override { return m_pStruct[ i ].deaths; }
	virtual int				GetPlayerClass( int i ) override { return m_pStruct[ i ].playerclass; }
	virtual int				GetTeamNumber( int i ) override { return m_pStruct[ i ].teamnumber; }
	virtual char			*GetTeamName( int i ) override { return m_pStruct[ i ].teamname; }
	virtual float			GetHealth( int i ) override { return m_pStruct[ i ].health; }
	virtual float			GetArmor( int i ) override { return m_pStruct[ i ].armor; }

	virtual void			SetHealth( int i, float health ) override { m_pStruct[ i ].health = health; }

private:
	extra_player_info_5_26_t *m_pStruct;
};

static CExtraPlayerInfo_5_26 gExtraPlayerInfo_5_26;

//-----------------------------------------------------------------------------
// CExtraPlayerInfo >= 5.22
//-----------------------------------------------------------------------------

class CExtraPlayerInfo final : public CBaseExtraPlayerInfo
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<extra_player_info_t *>( pStruct ); }

	virtual void			*GetStruct( void ) override { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 515; }

	virtual float			GetFrags( int i ) override { return m_pStruct[ i ].frags; }
	virtual int				GetDeaths( int i ) override { return m_pStruct[ i ].deaths; }
	virtual int				GetPlayerClass( int i ) override { return m_pStruct[ i ].playerclass; }
	virtual int				GetTeamNumber( int i ) override { return m_pStruct[ i ].teamnumber; }
	virtual char			*GetTeamName( int i ) override { return m_pStruct[ i ].teamname; }
	virtual float			GetHealth( int i ) override { return m_pStruct[ i ].health; }
	virtual float			GetArmor( int i ) override { return m_pStruct[ i ].armor; }

	virtual void			SetHealth( int i, float health ) override { m_pStruct[ i ].health = health; }

private:
	extra_player_info_t *m_pStruct;
};

static CExtraPlayerInfo gExtraPlayerInfo;

//-----------------------------------------------------------------------------
// CExtraPlayerInfo 5.11
//-----------------------------------------------------------------------------

class CExtraPlayerInfo_5_11 final : public CBaseExtraPlayerInfo
{
public:
	void Init( void *pStruct ) { m_pStruct = static_cast<extra_player_info_5_11_t *>( pStruct ); }

	virtual void			*GetStruct( void ) override { return m_pStruct; }
	virtual int				GetCompatibilityVersion( void ) override { return 500; }

	virtual float			GetFrags( int i ) override { return static_cast<float>( m_pStruct[ i ].frags ); }
	virtual int				GetDeaths( int i ) override { return m_pStruct[ i ].deaths; }
	virtual int				GetPlayerClass( int i ) override { return m_pStruct[ i ].playerclass; }
	virtual char			*GetTeamName( int i ) override { return m_pStruct[ i ].teamname; }
	virtual float			GetHealth( int i ) override { return static_cast<float>( m_pStruct[ i ].health ); }

	virtual void			SetHealth( int i, float health ) override { m_pStruct[ i ].health = health; }

private:
	extra_player_info_5_11_t *m_pStruct;
};

static CExtraPlayerInfo_5_11 gExtraPlayerInfo_5_11;

//-----------------------------------------------------------------------------
// Get CBaseExtraPlayerInfo based on SC version
//-----------------------------------------------------------------------------

CBaseExtraPlayerInfo *GetBaseExtraPlayerInfo( void *pExtraPlayerInfo, int iGameVersion )
{
	if ( pExtraPlayerInfo == NULL || iGameVersion == 0 )
		return &gExtraPlayerInfoDummy;

	if ( iGameVersion >= 526 )
	{
		gExtraPlayerInfo_5_26.Init( pExtraPlayerInfo );
		return &gExtraPlayerInfo_5_26;
	}
	else if ( iGameVersion >= 515 /* 522 */ )
	{
		gExtraPlayerInfo.Init( pExtraPlayerInfo );
		return &gExtraPlayerInfo;
	}

	gExtraPlayerInfo_5_11.Init( pExtraPlayerInfo );
	return &gExtraPlayerInfo_5_11;
}
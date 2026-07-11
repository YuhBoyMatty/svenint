// SvenInt (c) Sw1ft
// st_input_manager.h

#ifndef SINT_FEATURE_INPUT_MANAGER_H
#define SINT_FEATURE_INPUT_MANAGER_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"
#include "modules/menu.h"

//-----------------------------------------------------------------------------
// Macro definitions
//-----------------------------------------------------------------------------

#define IM_FILE_HEADER		( 0x4D49 ) // IM
#define IM_FILE_VERSION		( 3 )
#define IM_FILE_PATH		( SVENINT_FOLDER_NAME "/input_manager/" )
#define IM_FRAME_INFO_SIZE	( sizeof( im_frame_info_t ) )
#define IM_FRAME_INPUTS_SIZE ( sizeof( im_frame_inputs_t ) )

//-----------------------------------------------------------------------------
// Type declarations
//-----------------------------------------------------------------------------

typedef const char *im_cstring_t;

//-----------------------------------------------------------------------------
// States
//-----------------------------------------------------------------------------

typedef enum
{
	IM_NONE = 0,
	IM_RECORDING,
	IM_PLAYINGBACK
} im_state_t;

//-----------------------------------------------------------------------------
// Structures of a single input frame
//-----------------------------------------------------------------------------

struct im_frame_info_t
{
	float			origin[ 3 ];
	float			velocity[ 3 ];
};

struct im_frame_inputs_t
{
	float			realviewangles[ 3 ];
	float			viewangles[ 3 ];
	float			forwardmove;
	float			sidemove;
	float			upmove;
	unsigned short	buttons;
	unsigned char	impulse;
	unsigned char	weaponselect;
};

struct im_frame_t
{
	im_frame_info_t			info;
	im_frame_inputs_t		inputs;

	const char *commands;
};

//-----------------------------------------------------------------------------
// Inputs context
//-----------------------------------------------------------------------------

class CInputContext
{
	friend class CInputManager;

public:
	CInputContext();

	void					Init( const char *pszFilename );
	void					Clear( void );

	bool					LoadFromFile( void );
	bool					SaveToFile( void );

	void					Split( void );

	void					GotoFrame( int iFrame );
	void					ForwardFrames( int iFrames );
	void					BackwardFrames( int iFrames );

	void					IncrementFramesCounter( void );

	im_frame_t				&CurrentFrame( void );
	im_frame_t				&FrameBuffer( void );
	std::vector<im_frame_t> &Frames( void );
	int						FrameCounter( void ) const;

	int						Version( void ) const;
	const char				*FileName( void ) const;

public:
	// Callbacks
	void					RecordInput( float frametime, usercmd_t *cmd, int active );
	void					PlaybackInput( float frametime, usercmd_t *cmd, int active );

private:
	bool					ParseFile( FILE *hStream, int iVersion, int &iErrorCode );

private:
	int						m_iVersion;

	std::vector<im_frame_t> m_frames;
	im_frame_t				m_FrameBuffer;
	int						m_iCurrentFrame;

	std::string				m_sFileName;
	std::string				m_sFilePath;

	bool					m_bSavedInfos;
};

//-----------------------------------------------------------------------------
// Input manager feature
//-----------------------------------------------------------------------------

class CInputManager final : public CBaseFeature, IHookEventListener
{
public:
	CInputManager( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	// Main interface
	bool			Record( const char *pszFilename );
	bool			Playback( const char *pszFilename );
	bool			Split( void );
	bool			Stop( bool bAutoStop = false );

	void			Goto( int iFrame );
	void			Forward( int iFrames );
	void			Backward( int iFrames );

	void			RecordCommand( const char *pszCommand );
	void			RecordCommandNow( const char *pszCommand );

	bool			IsInAction( void ) const;
	bool			IsRecording( void ) const;
	bool			IsPlayingback( void ) const;

	int				GetCurrentFrame( void ) const;
	inline FILE		*GetExperimental( void ) { return m_pExperimental; }

public:
	// Callbacks
	void			CreateMove( float frametime, usercmd_t *cmd, int active );
	void			GameFrame( bool bPostRunCmd );
	void			OnBeginLoading( void );
	void			OnFirstClientdataReceived( void );
	void			OnVideoInit( void );

	void			SavedInputsSignal( bool bSaved );

	// Callbacks for hooked functions
	void			OnCbuf_AddText( const char *pszCommand );
	void			OnServerCmd( const char *pszCommand );
	void			OnCmdStart( edict_t *player, usercmd_t *cmd );

public:
	inline CInputContext &GetInputContext( void ) { return m_InputContext; };

private:
	int					m_state;
	bool				m_bSavedInputs;
	bool				m_bForceViewAngles;
	FILE				*m_pExperimental;

	CInputContext		m_InputContext;

	std::string			m_sQueuedCommands;
	CHash<im_cstring_t> m_WhitelistedCommands;
	
	void				*m_pfnCbuf_AddText;
	void				*m_pfnServerCmd;
	DetourHandle_t		m_hCbuf_AddText;
	DetourHandle_t		m_hServerCmd;
	DetourHandle_t		m_hCmdStart;
};

EXTERN_FEATURE( CInputManager, inputmanager );

#endif // SINT_FEATURE_INPUT_MANAGER_H

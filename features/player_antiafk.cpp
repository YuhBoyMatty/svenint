// SvenInt (c) Sw1ft
// player_antiafk.cpp

#include "stdafx.h"
#include "player_antiafk.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAntiAFK, antiafk, "Player", "Anti-AFK" );

static UserMsgHookFn ORIG_UserMsgHook_Health = NULL;

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

FEATURE_CON_COMMAND_TOGGLE( sc_antiafk, "Prevent an Anti-AFK plugin from kicking / slaying" );

//-----------------------------------------------------------------------------
// Usermsg Health
//-----------------------------------------------------------------------------

static int UserMsgHook_Health( const char *pszUserMsg, int iSize, void *pBuffer )
{
	auto result = ORIG_UserMsgHook_Health( pszUserMsg, iSize, pBuffer );

	THIS_FEATURE()->OnRespawn();

	return result;
}

//-----------------------------------------------------------------------------
// Respawn event
//-----------------------------------------------------------------------------

void CAntiAFK::OnRespawn( void )
{
	if ( !m_bWaitingForRespawn )
		return;

	m_vecAFKPoint.x = localplayer->GetOrigin().x;
	m_vecAFKPoint.y = localplayer->GetOrigin().y;

	m_bDead = false;
	m_bComingBackToAFKPoint = false;

	m_flComingBackStartTime = -1.f;

	m_bWaitingForRespawn = false;
}

//-----------------------------------------------------------------------------
// AntiAFK main routine
//-----------------------------------------------------------------------------

void CAntiAFK::AntiAFK( usercmd_t *cmd )
{
	int nMode = m_pType->GetInt();
	bool bDead = localplayer->IsDead();

	if ( m_bDead != bDead )
	{
		if ( bDead )
		{
			OnDie();
		}
	}

	if ( bDead || ( cmd->buttons & ( IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT ) ) )
		return;

	if ( m_pStayWithinRadius->GetBool() && !( nMode == 1 || nMode == 2 ) )
	{
		if ( m_bWaitingForRespawn || m_bWaitingForClientdata )
			return;

		if ( m_vecAFKPoint.x == 0.0f && m_vecAFKPoint.y == 0.0f )
			m_vecAFKPoint = localplayer->GetOrigin().AsVector2D();

		Vector2D vecOrigin = localplayer->GetOrigin().AsVector2D();
		float flDistanceToAFKPointSqr = ( m_vecAFKPoint - vecOrigin ).LengthSqr();

		if ( m_bComingBackToAFKPoint || flDistanceToAFKPointSqr > M_SQR( m_pStayRadius->GetFloat() ) ) // moved out of range
		{
			bool bReset = false;

			if ( m_flComingBackStartTime == -1.0f )
			{
				m_flComingBackStartTime = cl_enginefuncs->GetClientTime();
			}
			else if ( m_pResetStayPos->GetBool() && cl_enginefuncs->GetClientTime() - m_flComingBackStartTime >= 10.f )
			{
				// Coming to the AFK point too long, reset current state
				Reset();
				bReset = true;
			}

			if ( !bReset )
			{
				if ( m_bComingBackToAFKPoint && flDistanceToAFKPointSqr <= M_SQR( 25.0f ) )
				{
					m_bComingBackToAFKPoint = false;
					m_flComingBackStartTime = -1.0f;
					return;
				}

				Vector2D vecForward;
				Vector2D vecRight;

				Vector2D vecDir = m_vecAFKPoint - vecOrigin;

				m_bComingBackToAFKPoint = true;
				vecDir.NormalizeInPlace();

				// Rotate the wish vector by a random direction, must help if we stuck somewhere
				int nRandom = cl_enginefuncs->pfnRandomLong( 0, 1 );

				float flTheta = m_pStayRadiusOffsetAngle->GetFloat() * static_cast<float>( M_PI ) / 180.0f;

				float ct = cosf( flTheta );
				float st = sinf( flTheta );

				if ( nRandom == 1 ) // counter clockwise
				{
					vecDir.x = vecDir.x * ct - vecDir.y * st;
					vecDir.y = vecDir.x * st + vecDir.y * ct;
				}
				else // clockwise
				{
					vecDir.x = vecDir.x * ct + vecDir.y * st;
					vecDir.y = -vecDir.x * st + vecDir.y * ct;
				}

				// Forward angles
				vecForward.x = cosf( cmd->viewangles.y * static_cast<float>( M_PI ) / 180.f );
				vecForward.y = sinf( cmd->viewangles.y * static_cast<float>( M_PI ) / 180.f );

				// Make a right vector of angles. Rotate forward vector as a complex number by 90 deg.
				vecRight.x = vecForward.y;
				vecRight.y = -vecForward.x;

				// Multiply by max movement speed
				vecForward *= localplayer->GetMaxSpeed();
				vecRight *= localplayer->GetMaxSpeed();

				// Project onto direction vector
				float forwardmove = DotProduct( vecForward, vecDir );
				float sidemove = DotProduct( vecRight, vecDir );

				// Apply moves
				cmd->forwardmove = forwardmove;
				cmd->sidemove = sidemove;

				return;
			}
		}
	}
	else
	{
		m_bComingBackToAFKPoint = false;
		m_flComingBackStartTime = -1.0f;
	}

	switch ( nMode )
	{
	case 0:
	{
		static bool forward_step = true;

		cmd->forwardmove = forward_step ? 50.0f : -50.0f;

		forward_step = !forward_step;

		break;
	}

	case 1:
	case 2:
	{
		bool bSuicided = false;
		bool bHasAnyWeapon = false;

		switch ( localplayer->GetCurrentWeaponID() )
		{
		case WEAPON_CROWBAR:
		case WEAPON_MEDKIT:
		case WEAPON_WRENCH:
		case WEAPON_BARNACLE_GRAPPLE:
			if ( nMode == 1 )
				cl_enginefuncs->pfnClientCmd( "gibme\n" );
			else
				cl_enginefuncs->pfnClientCmd( "kill\n" );

			bSuicided = true;
			break;
		}

		if ( !bSuicided )
		{
			bool bFound = false;
			WEAPON *pWeapon = NULL;

			for ( int i = 0; i < inventory->GetMaxWeaponSlots(); i++ )
			{
				for ( int j = 0; j < inventory->GetMaxWeaponPositions(); j++ )
				{
					if ( pWeapon = inventory->GetWeapon( i, j ) )
					{
						if ( !bHasAnyWeapon && inventory->HasAmmo( pWeapon ) )
							bHasAnyWeapon = true;

						switch ( pWeapon->iId )
						{
						case WEAPON_CROWBAR:
						case WEAPON_WRENCH:
						case WEAPON_BARNACLE_GRAPPLE:
							inventory->SelectWeapon( pWeapon );
							bFound = true;

							break;

						case WEAPON_MEDKIT:
							if ( inventory->GetPrimaryAmmoCount( pWeapon ) > 0 )
							{
								inventory->SelectWeapon( pWeapon );
								bFound = true;
							}

							break;
						}

						if ( bFound )
							break;
					}
				}

				if ( bFound )
					break;
			}

			if ( !bFound )
			{
				if ( bHasAnyWeapon )
				{
					if ( !( localplayer->GetCurrentWeaponID() == WEAPON_NONE &&
						 ( localplayer->Time() - m_flClientDataLastUpdate ) >= 0.5f ) )
						cl_enginefuncs->pfnClientCmd( "gibme\n" );
				}
				else if ( nMode == 1 )
				{
					cl_enginefuncs->pfnClientCmd( "gibme\n" );
				}
				else
				{
					cl_enginefuncs->pfnClientCmd( "kill\n" );
				}
			}
		}

		break;
	}

	case 3:
	{
		constexpr int delay_count = 30;

		static int delay = 0;
		static int attack_button_idx = 0;
		static int attack_button = IN_ATTACK;

		static const int attack_buttons[] =
		{
			IN_ATTACK,
			IN_JUMP,
			IN_DUCK,
			IN_USE,
			IN_CANCEL,
			IN_LEFT,
			IN_RIGHT,
			IN_ATTACK2,
			IN_RUN,
			IN_RELOAD
			// IN_ALT1
		};

		++delay;

		if ( delay > delay_count )
		{
			if ( attack_button_idx == ( sizeof( attack_buttons ) / sizeof( *attack_buttons ) ) )
				attack_button_idx = 0;

			attack_button = attack_buttons[ attack_button_idx++ ];

			delay = 0;
		}

		cmd->buttons |= attack_button;

		{
			constexpr int delay_count = 60;

			static int delay = 0;
			static int movement_button = IN_MOVERIGHT;

			WalkAround( cmd, delay, movement_button, delay_count );

			RotateCamera();
		}

		break;
	}

	case 4:
	{
		constexpr int delay_count = 60;

		static int delay = 0;
		static int movement_button = IN_MOVERIGHT;

		WalkAround( cmd, delay, movement_button, delay_count );

		RotateCamera();

		break;
	}

	case 5:
	{
		cmd->buttons |= IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT;
		cmd->sidemove = localplayer->GetMaxSpeed();

		RotateCamera();

		break;
	}
	}

	if ( localplayer->GetWaterLevel() == WL_EYES )
	{
		cmd->upmove = localplayer->GetMaxSpeed();
	}
}

//-----------------------------------------------------------------------------
// WalkAround
//-----------------------------------------------------------------------------

void CAntiAFK::WalkAround( usercmd_t *cmd, int &delay, int &movement_button, const int delay_count )
{
REPEAT:
	if ( movement_button == IN_MOVERIGHT )
	{
		if ( delay > delay_count )
		{
			movement_button = IN_BACK;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->sidemove = localplayer->GetMaxSpeed();
		}
	}
	else if ( movement_button == IN_BACK )
	{
		if ( delay > delay_count )
		{
			movement_button = IN_MOVELEFT;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->forwardmove = -localplayer->GetMaxSpeed();
		}
	}
	else if ( movement_button == IN_MOVELEFT )
	{
		if ( delay > delay_count )
		{
			movement_button = IN_FORWARD;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->sidemove = -localplayer->GetMaxSpeed();
		}
	}
	else if ( movement_button == IN_FORWARD )
	{
		if ( delay > delay_count )
		{
			movement_button = IN_MOVERIGHT;
			delay = 0;

			goto REPEAT;
		}
		else
		{
			cmd->forwardmove = localplayer->GetMaxSpeed();
		}
	}

	cmd->buttons |= movement_button;
	++delay;
}

//-----------------------------------------------------------------------------
// Rotate camera
//-----------------------------------------------------------------------------

void CAntiAFK::RotateCamera( void )
{
	if ( !m_pRotateCamera->GetBool() )
		return;
	
	Vector va;

	float flRotationAngle = m_pRotationAngle->GetFloat();
	float flRotationAngleAbs = fabsf( flRotationAngle );

	static float s_flPitchDirection = 1.0f;

	cl_enginefuncs->GetViewAngles( va );

	if ( s_flPitchDirection > 0.0f )
	{
		if ( va.x + flRotationAngleAbs > 89.0f )
			s_flPitchDirection = -1.0f;
		else
			va.x += flRotationAngleAbs;
	}
	else if ( s_flPitchDirection < 0.0f )
	{
		if ( va.x - flRotationAngleAbs < -89.0f )
			s_flPitchDirection = 1.0f;
		else
			va.x -= flRotationAngleAbs;
	}

	va.y += flRotationAngle;
	va.y = NormalizeAngle( va.y );

	cl_enginefuncs->SetViewAngles( va );
}

//-----------------------------------------------------------------------------
// Died
//-----------------------------------------------------------------------------

void CAntiAFK::OnDie( void )
{
	m_bDead = true;
	m_bComingBackToAFKPoint = false;

	m_vecAFKPoint.x = 0.f;
	m_vecAFKPoint.y = 0.f;

	m_flComingBackStartTime = -1.f;

	m_bWaitingForRespawn = true;
}

//-----------------------------------------------------------------------------
// Reset state
//-----------------------------------------------------------------------------

void CAntiAFK::Reset( void )
{
	m_bDead = false;
	m_bComingBackToAFKPoint = false;

	m_vecAFKPoint.x = 0.f;
	m_vecAFKPoint.y = 0.f;

	m_flComingBackStartTime = -1.f;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAntiAFK::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		Reset();
		m_bWaitingForRespawn = true;
	}
	else if ( pEvent->GetType() == kHUD_UpdateClientData_HookEvent )
	{
		if ( pEvent->GetReturn<int>() ) // changed
			m_flClientDataLastUpdate = pEvent->GetArg<float>( "flTime" );
	}
	else if ( pEvent->GetType() == kSCR_EndLoadingPlaque_HookEvent )
	{
		OnDie();
	}
	else // kCL_CreateMove post event
	{
		AntiAFK( pEvent->GetArg<usercmd_t *>( "cmd" ) );
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAntiAFK::CAntiAFK( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();
	Reset();

	m_pType = NULL;
	m_pRotateCamera = NULL;
	m_pStayWithinRadius = NULL;
	m_pResetStayPos = NULL;
	m_pStayRadius = NULL;
	m_pStayRadiusOffsetAngle = NULL;
	m_pRotationAngle = NULL;
	m_pChangeNickname = NULL;
	m_pNicknamePostfix = NULL;
	m_pSendMessage = NULL;
	m_pMessage = NULL;
	m_pSendMessageNotAFK = NULL;
	m_pMessageNotAFK = NULL;

	m_bWaitingForClientdata = false;
	m_bWaitingForRespawn = false;

	m_flClientDataLastUpdate = -1.f;
	m_hUserMsgHook_Health = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAntiAFK::OnEnable( void )
{
	Reset();

	m_bWaitingForClientdata = false;
	m_bWaitingForRespawn = false;

	player_info_t *pPlayerInfo = enginestudio->PlayerInfo( playermove->player_index() );
	if ( m_pChangeNickname->GetBool() && pPlayerInfo != NULL )
	{
		m_sNickname = pPlayerInfo->name;

		char cmd[ 256 ];
		snprintf( cmd, Q_ARRAYSIZE( cmd ), "name \"%s %s\"", m_sNickname.c_str(), m_pNicknamePostfix->GetCString() );
		cl_enginefuncs->pfnClientCmd( cmd );
	}
	else
	{
		m_sNickname.clear();
	}

	if ( m_pSendMessage->GetBool() )
	{
		char cmd[ 256 ];
		snprintf( cmd, Q_ARRAYSIZE( cmd ), "say \"/me %s\"", m_pMessage->GetCString() );
		cl_enginefuncs->pfnClientCmd( cmd );
	}

	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_UpdateClientData_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kSCR_EndLoadingPlaque_HookEvent );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAntiAFK::OnDisable( void )
{
	if ( !m_sNickname.empty() )
	{
		char cmd[ 256 ];
		snprintf( cmd, Q_ARRAYSIZE( cmd ), "name \"%s\"", m_sNickname.c_str() );
		cl_enginefuncs->pfnClientCmd( cmd );
	}

	if ( m_pSendMessageNotAFK->GetBool() )
	{
		char cmd[ 256 ];
		snprintf( cmd, Q_ARRAYSIZE( cmd ), "say \"/me %s\"", m_pMessageNotAFK->GetCString() );
		cl_enginefuncs->pfnClientCmd( cmd );
	}

	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_UpdateClientData_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kSCR_EndLoadingPlaque_HookEvent );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAntiAFK::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pType = Modules::menu->AddParamList( this, "Type", NULL, 0, " 0 - Step Forward & Back\0 1 - Spam Gibme\0 2 - Spam Kill\0 3 - Walk Around & Spam Inputs\0 4 - Walk Around\0 5 - Go Right\0\0" );
	m_pRotateCamera = Modules::menu->AddParamBool( this, "RotateCamera", NULL, true );
	m_pStayWithinRadius = Modules::menu->AddParamBool( this, "StayWithinRange", NULL, true );
	m_pResetStayPos = Modules::menu->AddParamBool( this, "ResetStayPos", NULL, true );
	m_pStayRadius = Modules::menu->AddParamFloat( this, "StayRadius", NULL, 200.f, 10.f, 500.f );
	m_pStayRadiusOffsetAngle = Modules::menu->AddParamFloat( this, "StayRadiusOffsetAngle", NULL, 30.f, 0.f, 89.f );
	m_pRotationAngle = Modules::menu->AddParamFloat( this, "RotationAngle", NULL, -0.7f, -7.f, 7.f );

	Modules::menu->AddElementSeparator( this, "Notification to Players" );

	m_pChangeNickname = Modules::menu->AddParamBool( this, "ChangeNickname", NULL, false );
	m_pNicknamePostfix = Modules::menu->AddParamText( this, "NicknamePostfix", NULL, "[AFK]" );

	m_pSendMessage = Modules::menu->AddParamBool( this, "SendMessage", NULL, false );
	m_pMessage = Modules::menu->AddParamText( this, "Message", NULL, "is AFK" );

	m_pSendMessageNotAFK = Modules::menu->AddParamBool( this, "SendMessageNotAFK", NULL, false );
	m_pMessageNotAFK = Modules::menu->AddParamText( this, "MessageNotAFK", NULL, "is not AFK anymore" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAntiAFK::PostLoad( void )
{
	FEATURE_REGISTER_CCMD( sc_antiafk );
	m_hUserMsgHook_Health = gamehooks->HookUserMessage( "Health", UserMsgHook_Health, &ORIG_UserMsgHook_Health );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAntiAFK::Unload( void )
{
	FEATURE_UNREGISTER_CCMD( sc_antiafk );
	gamehooks->UnhookUserMessage( m_hUserMsgHook_Health );
}
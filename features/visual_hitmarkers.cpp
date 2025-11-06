// SvenInt (c) Sw1ft
// visual_hitmarkers.cpp

#include "stdafx.h"
#include "svenint.h"
#include "visual_hitmarkers.h"
#include "r_drawing.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CHitMarkers, hitmarkers, "Visual", "Hit Markers" );

UserMsgHookFn ORIG_UserMsgHook_CreateBlood = NULL;

//-----------------------------------------------------------------------------
// CreateBlood event
//-----------------------------------------------------------------------------

int UserMsgHook_CreateBlood( const char *pszUserMsg, int iSize, void *pBuffer )
{
	if ( THIS_FEATURE()->IsEnabled() )
		THIS_FEATURE()->AddHitmarker( pszUserMsg, iSize, pBuffer );

	return ORIG_UserMsgHook_CreateBlood( pszUserMsg, iSize, pBuffer );
}

//-----------------------------------------------------------------------------
// Add a hitmarker
//-----------------------------------------------------------------------------

void CHitMarkers::AddHitmarker( const char *pszUserMsg, int iSize, void *pBuffer )
{
	const float flTime = cl_enginefuncs->GetClientTime();

	if ( !m_bHitmarkerOnce )
		return;

	if ( flTime - m_flLastWeaponAttack > 0.3f )
		return;

	Vector vecOrigin, vecDir;
	m_CreateBloodBuffer.Init( pszUserMsg, pBuffer, iSize, true );

	vecOrigin.x = m_CreateBloodBuffer.ReadCoord();
	vecOrigin.y = m_CreateBloodBuffer.ReadCoord();
	vecOrigin.z = m_CreateBloodBuffer.ReadCoord();

	vecDir = vecOrigin - localplayer->GetEyePosition();
	vecDir.NormalizeInPlace();

	if ( VEC_RAD2DEG( acosf( localplayer->GetForwardVector().Dot( vecDir ) ) ) >= 10.f )
		return;

	if ( m_pSound->GetBool() )
		cl_enginefuncs->pfnPlaySoundByName( SVENINT_FOLDER_NAME "/hitmarker/hitmarker.wav", 1.5f );

	m_hitMarkers.push_back( { vecOrigin, flTime + m_pStayTime->GetFloat() } );
	m_bHitmarkerOnce = false;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CHitMarkers::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHost_FilterTime_HookEvent )
	{
		if ( pEvent->GetReturn<qboolean>() )
			m_bHitmarkerOnce = true;
	}
	else if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_hitMarkers.clear();
		m_flLastWeaponAttack = -1.f;
	}
	else if ( pEvent->GetType() == kCL_CreateMove_HookEvent )
	{
		if ( !localplayer->IsDead() &&
			 UTIL_IsFiring( pEvent->GetArg<usercmd_t *>( "cmd" ) ) )
		{
			m_flLastWeaponAttack = cl_enginefuncs->GetClientTime();
		}
	}
	else // kVGuiClientPanelPaint event
	{
		if ( m_hHitMarkerTexture == -1 )
		{
			m_hHitMarkerTexture = surface->CreateNewTextureID( true );
			surface->DrawSetTextureFile( m_hHitMarkerTexture, "svenint/tex/hitmarker", true, false );
		}

		int size = m_pSize->GetInt();
		float flTime = cl_enginefuncs->GetClientTime();

		for ( size_t i = 0; i < m_hitMarkers.size(); i++ )
		{
			float vecScreen[ 2 ];
			hitmarker_s &hitmarker = m_hitMarkers[ i ];

			if ( hitmarker.time <= flTime ||
				 !UTIL_WorldToScreen( hitmarker.origin, vecScreen ) )
			{
				m_hitMarkers.erase( m_hitMarkers.begin() + i );
				i--;

				continue;
			}

			int x = (int)vecScreen[ 0 ] - ( size / 2 );
			int y = (int)vecScreen[ 1 ] - ( size / 2 );

			Features::drawing->DrawTexture( m_hHitMarkerTexture,
											x, y, x + size, y + size,
											255, 255, 255, 255 );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CHitMarkers::CHitMarkers( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pSound = NULL;
	m_pSize = NULL;
	m_pStayTime = NULL;

	m_bHitmarkerOnce = true;
	m_flLastWeaponAttack = -1.f;
	m_hHitMarkerTexture = -1;
	m_hUserMsgHook_CreateBlood = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CHitMarkers::OnEnable( void )
{
	hookevents->RegisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CHitMarkers::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHost_FilterTime_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CHitMarkers::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pSound = Modules::menu->AddParamBool( this, "Sound", NULL, true );
	m_pSize = Modules::menu->AddParamInteger( this, "Size", NULL, 24, 2, 100 );
	m_pStayTime = Modules::menu->AddParamFloat( this, "StayTime", NULL, 0.3f, 0.01f, 3.f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CHitMarkers::PostLoad( void )
{
	m_hUserMsgHook_CreateBlood = gamehooks->HookUserMessage( "CreateBlood", UserMsgHook_CreateBlood, &ORIG_UserMsgHook_CreateBlood );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CHitMarkers::Unload( void )
{
	if ( surface->IsTextureIDValid( m_hHitMarkerTexture ) )
		surface->DeleteTextureByID( m_hHitMarkerTexture );

	gamehooks->UnhookUserMessage( m_hUserMsgHook_CreateBlood );
}
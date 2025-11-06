// SvenInt (c) Sw1ft
// misc_antisoundspam.cpp

#include "stdafx.h"
#include "misc_antisoundspam.h"
#include "modules/menu.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAntiSoundSpam, antisoundspam, "Misc", "Anti Sound Spam" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAntiSoundSpam::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_VidInit_HookEvent )
	{
		m_soundsList.Clear();
		return kHookContinue;
	}
	
	// HUD_StudioEvent event
	auto studio_event = pEvent->GetArg<mstudioevent_t *>( "studio_event" );
	auto entity = pEvent->GetArg<cl_entity_t *>( "entity" );

	if ( studio_event->event != CL_EVENT_SOUND || entity == NULL || studio_event->options[ 0 ] == '\0' )
		return kHookContinue;
	
	anti_sound_spam_t dummy;
	dummy.entindex = entity->index;
	strncpy_s( dummy.sound, Q_ARRAYSIZE( anti_sound_spam_t::sound ), studio_event->options, Q_ARRAYSIZE( mstudioevent_t::options ) );

	float flTime = cl_enginefuncs->GetClientTime();
	anti_sound_spam_t *sound = m_soundsList.Find( dummy );

	if ( sound == NULL )
	{
		// Not dummy now!
		dummy.blocktime = 0.f;
		m_soundsList.Insert( dummy );
		return kHookContinue;
	}

	float flTimeDifference = flTime - sound->lastplayed;
	if ( sound->blocktime >= flTime )
	{
		return kHookSupercedeStop;
	}
	else if ( sound->blocktime != 0.f )
	{
		sound->blocktime = 0.f;
	}
	else if ( entity->curstate.renderfx == kRenderFxDeadPlayer || flTimeDifference >= m_pMinDiffTime->GetFloat() && flTimeDifference <= m_pMaxDiffTime->GetFloat() )
	{
		sound->blocktime = flTime + m_pBlockTime->GetFloat();
		PrintMsg( "Blocked sound: %s (ent: %d)\n", studio_event->options, entity->index );

		return kHookSupercedeStop;
	}

	sound->lastplayed = flTime;
	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAntiSoundSpam::CAntiSoundSpam( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName ),
																					m_soundsList( 63, HashMap_Compare, HashMap_Hash )
{
	m_pBlockTime = NULL;
	m_pMinDiffTime = NULL;
	m_pMaxDiffTime = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CAntiSoundSpam::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->RegisterListener( this, kHUD_StudioEvent_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CAntiSoundSpam::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_VidInit_HookEvent );
	hookevents->UnregisterListener( this, kHUD_StudioEvent_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CAntiSoundSpam::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pBlockTime = Modules::menu->AddParamFloat( this, "BlockTime", NULL, 2.f, 0.f, 30.f );
	m_pMinDiffTime = Modules::menu->AddParamFloat( this, "MinDiffTime", NULL, 0.05f, 0.f, 1.f );
	m_pMaxDiffTime = Modules::menu->AddParamFloat( this, "MaxDiffTime", NULL, 0.2f, 0.f, 1.5f );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CAntiSoundSpam::PostLoad( void )
{
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CAntiSoundSpam::Unload( void )
{
}
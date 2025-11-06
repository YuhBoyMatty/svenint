// SvenInt (c) Sw1ft
// Realization was taken from Bunnymod XT, credits go to BXT contributors
// r_viewmodel_tweaks.cpp

#include "stdafx.h"
#include "r_viewmodel_tweaks.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CViewModelTweaks, viewmodeltweaks, "Render", "Viewmodel Tweaks" );

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CViewModelTweaks::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kStudioSetupBones_HookEvent )
	{
		if ( studiorenderer->m_pCurrentEntity != cl_enginefuncs->GetViewModel() )
			return kHookContinue;

		if ( m_pDisableGlowShell->GetBool() && studiorenderer->m_pCurrentEntity->curstate.renderfx == kRenderFxGlowShell )
		{
			studiorenderer->m_pCurrentEntity->curstate.renderfx = kRenderFxNone;
		}

		float( *rotationmatrix )[ 3 ][ 4 ] = studiorenderer->m_protationmatrix;
		mstudioseqdesc_t *pSequenceDesc = (mstudioseqdesc_t *)( (byte *)studiorenderer->m_pStudioHeader + studiorenderer->m_pStudioHeader->seqindex ) + studiorenderer->m_pCurrentEntity->curstate.sequence;

		if ( m_pLeftHanded->GetBool() )
		{
			// FIXME: hook R_StudioDrawPoints
			//cl_enginefuncs->pTriAPI->CullFace( TRI_NONE );

			( *rotationmatrix )[ 0 ][ 1 ] *= -1.f;
			( *rotationmatrix )[ 1 ][ 1 ] *= -1.f;
			( *rotationmatrix )[ 2 ][ 1 ] *= -1.f;
		}

		if ( m_pFOV->GetFloat() > 0.f )
		{
			for ( int i = 0; i < 3; i++ )
			{
				( *rotationmatrix )[ i ][ 3 ] += studiorenderer->m_vNormal[ i ] * m_pFOV->GetFloat();
			}
		}

		if ( m_pDisableIdleAnim->GetBool() )
		{
			if ( strstr( pSequenceDesc->label, "idle" ) != NULL || strstr( pSequenceDesc->label, "fidget" ) != NULL )
			{
				studiorenderer->m_pCurrentEntity->curstate.framerate = 0; // don't animate at all
			}
		}

		if ( m_pDisableEquipAnim->GetBool() )
		{
			if ( strstr( pSequenceDesc->label, "holster" ) != NULL || strstr( pSequenceDesc->label, "draw" ) != NULL ||
					strstr( pSequenceDesc->label, "deploy" ) != NULL || strstr( pSequenceDesc->label, "up" ) != NULL ||
					strstr( pSequenceDesc->label, "down" ) != NULL )
			{
				studiorenderer->m_pCurrentEntity->curstate.sequence = 0; // instead set to idle sequence

				pSequenceDesc = (mstudioseqdesc_t *)( (byte *)studiorenderer->m_pStudioHeader + studiorenderer->m_pStudioHeader->seqindex ) + studiorenderer->m_pCurrentEntity->curstate.sequence;

				pSequenceDesc->numframes = 1;
				pSequenceDesc->fps = 1;
			}
		}
	}
	// StudioRenderModel event
	else if ( studiorenderer->m_pCurrentEntity == cl_enginefuncs->GetViewModel() )
	{
		int old_rendermode = studiorenderer->m_pCurrentEntity->curstate.rendermode;
		if ( m_pSemiTransparent->GetBool() )
		{
			cl_enginefuncs->pTriAPI->RenderMode( kRenderTransAdd );
			cl_enginefuncs->pTriAPI->Brightness( 2 );
		}
		else
		{
			cl_enginefuncs->pTriAPI->RenderMode( old_rendermode );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CViewModelTweaks::CViewModelTweaks( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pLeftHanded = NULL;
	m_pSemiTransparent = NULL;
	m_pDisableGlowShell = NULL;
	m_pDisableIdleAnim = NULL;
	m_pDisableEquipAnim = NULL;
	m_pFOV = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CViewModelTweaks::OnEnable( void )
{
	hookevents->RegisterListener( this, kStudioSetupBones_HookEvent );
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CViewModelTweaks::OnDisable( void )
{
	hookevents->UnregisterListener( this, kStudioSetupBones_HookEvent );
	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CViewModelTweaks::Load( void )
{
	Modules::menu->BindFeature( this );

	m_pLeftHanded = Modules::menu->AddParamBool( this, "LeftHanded", NULL, false );
	m_pSemiTransparent = Modules::menu->AddParamBool( this, "SemiTransparent", NULL, false );
	m_pDisableGlowShell = Modules::menu->AddParamBool( this, "DisableGlowShell", NULL, true );
	m_pDisableIdleAnim = Modules::menu->AddParamBool( this, "DisableIdleAnim", NULL, true );
	m_pDisableEquipAnim = Modules::menu->AddParamBool( this, "DisableEquipAnim", NULL, true );
	m_pFOV = Modules::menu->AddParamFloat( this, "FOV", NULL, 0.f, 0.f, 90.f );

	return true;
}
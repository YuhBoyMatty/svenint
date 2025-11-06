// SvenInt (c) Sw1ft
// r_chams.cpp

#include "stdafx.h"
#include "r_chams.h"
#include "player_firstperson_roaming.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

DECLARE_HOOK( void, APIENTRY, glColor4f, GLfloat, GLfloat, GLfloat, GLfloat );

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CChams, chams, "Render", "Chams" );

//-----------------------------------------------------------------------------
// glColor4f hook
//-----------------------------------------------------------------------------

DECLARE_FUNC( void, APIENTRY, HOOKED_glColor4f, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
	if ( THIS_FEATURE_IS_ENABLED() )
		THIS_FEATURE()->glColor4fEvent( red, green, blue );

	ORIG_glColor4f( red, green, blue, alpha );
}

//-----------------------------------------------------------------------------
// glColor4f event
//-----------------------------------------------------------------------------

void CChams::glColor4fEvent( GLfloat &red, GLfloat &green, GLfloat &blue )
{
	if ( !m_bOverrideColor )
		return;

	if ( m_iChamsMode == 2 )
	{
		red *= m_flOverrideColor[ 0 ];
		green *= m_flOverrideColor[ 1 ];
		blue *= m_flOverrideColor[ 2 ];
	}
	else
	{
		red = m_flOverrideColor[ 0 ];
		green = m_flOverrideColor[ 1 ];
		blue = m_flOverrideColor[ 2 ];
	}
}

//-----------------------------------------------------------------------------
// Glow
//-----------------------------------------------------------------------------

bool CChams::Glow( cl_entity_s *pEntity, int iGlow, bool bGlowWall, int iChams, int iGlowWidth, float flGlowColor[ 3 ] )
{
	if ( !m_pGlow->GetBool() )
		return false;

	if ( bGlowWall )
	{
		enginestudio->SetChromeOrigin();
		enginestudio->SetForceFaceFlags( 0 );

		if ( iGlow == 1 )
		{
			glDepthFunc( GL_GREATER );

			if ( m_pGlowOptimize->GetBool() )
				glDisable( GL_DEPTH_TEST );

			pEntity->curstate.renderfx = 0;
			studiorenderer->StudioRenderFinal_Hardware();

			if ( !m_pGlowOptimize->GetBool() )
			{
				glDepthFunc( GL_LESS );
				studiorenderer->StudioRenderFinal_Hardware();
			}
		}

		glDepthFunc( GL_GREATER );
		glDisable( GL_DEPTH_TEST );

		enginestudio->SetForceFaceFlags( STUDIO_NF_CHROME );

		pEntity->curstate.renderfx = kRenderFxGlowShell;
		pEntity->curstate.renderamt = iGlowWidth;
		pEntity->curstate.rendermode = 0;

		pEntity->curstate.rendercolor.r = byte( 255.f * flGlowColor[ 0 ] );
		pEntity->curstate.rendercolor.g = byte( 255.f * flGlowColor[ 1 ] );
		pEntity->curstate.rendercolor.b = byte( 255.f * flGlowColor[ 2 ] );

		if ( iGlow != 0 )
			cl_enginefuncs->pTriAPI->SpriteTexture( studiorenderer->m_pChromeSprite, 0 );

		studiorenderer->StudioRenderFinal_Hardware();

		if ( !m_pGlowOptimize->GetBool() )
			glEnable( GL_DEPTH_TEST );

		if ( !m_pChams->GetBool() )
		{
			if ( iGlow == 0 )
			{
				enginestudio->SetForceFaceFlags( 0 );
				pEntity->curstate.renderfx = 0;

				studiorenderer->StudioRenderFinal_Hardware();
			}

			glEnable( GL_DEPTH_TEST );
			glDepthFunc( GL_LESS );

			if ( iGlow == 0 && !m_pGlowOptimize->GetBool() )
				studiorenderer->StudioRenderFinal_Hardware();
		}
	}
	else
	{
		enginestudio->SetChromeOrigin();
		enginestudio->SetForceFaceFlags( 0 );

		if ( iGlow == 1 )
		{
			pEntity->curstate.renderfx = 0;
			studiorenderer->StudioRenderFinal_Hardware();
		}

		enginestudio->SetForceFaceFlags( STUDIO_NF_CHROME );

		pEntity->curstate.renderfx = kRenderFxGlowShell;
		pEntity->curstate.renderamt = iGlowWidth;
		pEntity->curstate.rendermode = 0;

		pEntity->curstate.rendercolor.r = byte( 255.f * flGlowColor[ 0 ] );
		pEntity->curstate.rendercolor.g = byte( 255.f * flGlowColor[ 1 ] );
		pEntity->curstate.rendercolor.b = byte( 255.f * flGlowColor[ 2 ] );

		if ( iGlow != 0 )
			cl_enginefuncs->pTriAPI->SpriteTexture( studiorenderer->m_pChromeSprite, 0 );

		studiorenderer->StudioRenderFinal_Hardware();

		if ( iGlow == 0 && !m_pChams->GetBool() )
		{
			enginestudio->SetForceFaceFlags( 0 );
			pEntity->curstate.renderfx = 0;

			studiorenderer->StudioRenderFinal_Hardware();
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Chams
//-----------------------------------------------------------------------------

bool CChams::Chams( cl_entity_s *pEntity, int iChams, bool bChamsWall, float flChamsColor[ 3 ], float flChamsWallColor[ 3 ] )
{
	if ( !m_pChams->GetBool() )
		return false;

	// Processed in OpenGL module
	m_bOverrideColor = true;
	m_iChamsMode = iChams;

	pEntity->curstate.rendermode = 0;
	pEntity->curstate.renderfx = 0;
	pEntity->curstate.renderamt = 0;

	enginestudio->SetForceFaceFlags( 0 );

	if ( iChams != 2 )
		glDisable( GL_TEXTURE_2D );

	glBindTexture( GL_TEXTURE_2D, 0 );

	if ( bChamsWall )
	{
		// Processed in OpenGL module
		m_flOverrideColor = flChamsWallColor;

		glDepthFunc( GL_GREATER );
		glDisable( GL_DEPTH_TEST );

		studiorenderer->StudioRenderFinal_Hardware();
	}

	// Processed in OpenGL module
	m_flOverrideColor = flChamsColor;

	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LESS );

	studiorenderer->StudioRenderFinal_Hardware();

	if ( iChams != 2 )
		glEnable( GL_TEXTURE_2D );

	m_bOverrideColor = false;

	return true;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CChams::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	// StudioRenderModel event
	bool bRenderHandled = false;
	cl_entity_s *pEntity = enginestudio->GetCurrentEntity();

	if ( pEntity == NULL )
		return kHookContinue;

	if ( !pEntity->player )
	{
		const char *pszModelName = pEntity->model->name;

		if ( pszModelName && *pszModelName )
		{
			if ( strstr( pszModelName, "cubemath" ) )
			{
				return kHookContinue;
			}

			const char *pszSlashLastOccur = strrchr( pszModelName, '/' );

			if ( pszSlashLastOccur )
				pszModelName = pszSlashLastOccur + 1;

			if ( ( pszModelName[ 0 ] == 'v' || pszModelName[ 0 ] == 'w' ) && pszModelName[ 1 ] == '_' ) // view / world item
			{
				bRenderHandled = Glow( pEntity,
									   m_pGlowItemsMode->GetInt(),
									   m_pGlowItemsBehindWall->GetBool(),
									   m_pChamsItemsMode->GetInt(),
									   m_pGlowItemsWidth->GetInt(),
									   m_pGlowItemsColor->GetColor() );

				bRenderHandled = Chams( pEntity,
										m_pChamsItemsMode->GetInt(),
										m_pChamsItemsBehindWall->GetBool(),
										m_pChamsItemsColor->GetColor(),
										m_pChamsItemsBehindWallColor->GetColor() ) || bRenderHandled;
			}
			else // world entity
			{
				bRenderHandled = Glow( pEntity,
									   m_pGlowEntitiesMode->GetInt(),
									   m_pGlowEntitiesBehindWall->GetBool(),
									   m_pChamsEntitiesMode->GetInt(),
									   m_pGlowEntitiesWidth->GetInt(),
									   m_pGlowEntitiesColor->GetColor() );

				bRenderHandled = Chams( pEntity,
										m_pChamsEntitiesMode->GetInt(),
										m_pChamsEntitiesBehindWall->GetBool(),
										m_pChamsEntitiesColor->GetColor(),
										m_pChamsEntitiesBehindWallColor->GetColor() ) || bRenderHandled;
			}
		}
	}
	else // player entity
	{
		if ( Features::firstpersonroaming->IsEnabled() &&
			 studiorenderer->m_pCurrentEntity == Features::firstpersonroaming->GetTargetPlayer() )
			return kHookSupercede;

		bRenderHandled = Glow( pEntity,
							   m_pGlowPlayersMode->GetInt(),
							   m_pGlowPlayersBehindWall->GetBool(),
							   m_pChamsPlayersMode->GetInt(),
							   m_pGlowPlayersWidth->GetInt(),
							   m_pGlowPlayersColor->GetColor() );

		bRenderHandled = Chams( pEntity,
								m_pChamsPlayersMode->GetInt(),
								m_pChamsPlayersBehindWall->GetBool(),
								m_pChamsPlayersColor->GetColor(),
								m_pChamsPlayersBehindWallColor->GetColor() ) || bRenderHandled;
	}

	return bRenderHandled ? kHookSupercede : kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CChams::CChams( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	SetInitiallyDisabled();

	m_pChams = NULL;

	m_pChamsPlayersBehindWall = NULL;
	m_pChamsPlayersMode = NULL;
	m_pChamsPlayersColor = NULL;
	m_pChamsPlayersBehindWallColor = NULL;

	m_pChamsEntitiesBehindWall = NULL;
	m_pChamsEntitiesMode = NULL;
	m_pChamsEntitiesColor = NULL;
	m_pChamsEntitiesBehindWallColor = NULL;

	m_pChamsItemsBehindWall = NULL;
	m_pChamsItemsMode = NULL;
	m_pChamsItemsColor = NULL;
	m_pChamsItemsBehindWallColor = NULL;

	m_pGlow = NULL;
	m_pGlowOptimize = NULL;

	m_pGlowPlayersBehindWall = NULL;
	m_pGlowPlayersMode = NULL;
	m_pGlowPlayersWidth = NULL;
	m_pGlowPlayersColor = NULL;

	m_pGlowEntitiesBehindWall = NULL;
	m_pGlowEntitiesMode = NULL;
	m_pGlowEntitiesWidth = NULL;
	m_pGlowEntitiesColor = NULL;

	m_pGlowItemsBehindWall = NULL;
	m_pGlowItemsMode = NULL;
	m_pGlowItemsWidth = NULL;
	m_pGlowItemsColor = NULL;

	m_bOverrideColor = false;
	m_flOverrideColor = NULL;
	m_iChamsMode = 0;

	m_pfnglColor4f = NULL;
	m_hglColor4f = DETOUR_INVALID_HANDLE;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CChams::OnEnable( void )
{
	hookevents->RegisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CChams::OnDisable( void )
{
	hookevents->UnregisterListener( this, kStudioRenderModel_HookEvent );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CChams::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pChams = Modules::menu->AddParamBool( this, "Chams", NULL, true );

	m_pChamsPlayersBehindWall = Modules::menu->AddParamBool( this, "ChamsPlayersBehindWall", NULL, true );
	m_pChamsPlayersMode = Modules::menu->AddParamList( this, "ChamsPlayersMode", NULL, 0, " 0 - Flat\0 1 - Texture\0 2 - Material\0\0" );
	m_pChamsPlayersColor = Modules::menu->AddParamColorRGB( this, "ChamsPlayersColor", NULL, Color( 0, 255, 0, 255 ) );
	m_pChamsPlayersBehindWallColor = Modules::menu->AddParamColorRGB( this, "ChamsPlayersBehindWallColor", NULL, Color( 255, 0, 0, 255 ) );
	
	m_pChamsEntitiesBehindWall = Modules::menu->AddParamBool( this, "ChamsEntitiesBehindWall", NULL, true );
	m_pChamsEntitiesMode = Modules::menu->AddParamList( this, "ChamsEntitiesMode", NULL, 0, " 0 - Flat\0 1 - Texture\0 2 - Material\0\0" );
	m_pChamsEntitiesColor = Modules::menu->AddParamColorRGB( this, "ChamsEntitiesColor", NULL, Color( 0, 255, 0, 255 ) );
	m_pChamsEntitiesBehindWallColor = Modules::menu->AddParamColorRGB( this, "ChamsEntitiesBehindWallColor", NULL, Color( 255, 0, 0, 255 ) );

	m_pChamsItemsBehindWall = Modules::menu->AddParamBool( this, "ChamsItemsBehindWall", NULL, true );
	m_pChamsItemsMode = Modules::menu->AddParamList( this, "ChamsItemsMode", NULL, 0, " 0 - Flat\0 1 - Texture\0 2 - Material\0\0" );
	m_pChamsItemsColor = Modules::menu->AddParamColorRGB( this, "ChamsItemsColor", NULL, Color( 0, 0, 255, 255 ) );
	m_pChamsItemsBehindWallColor = Modules::menu->AddParamColorRGB( this, "ChamsItemsBehindWallColor", NULL, Color( 255, 0, 0, 255 ) );

	Modules::menu->AddElementSeparator( this );

	m_pGlow = Modules::menu->AddParamBool( this, "Glow", NULL, false );
	m_pGlowOptimize = Modules::menu->AddParamBool( this, "GlowOptimize", NULL, false );

	m_pGlowPlayersBehindWall = Modules::menu->AddParamBool( this, "GlowPlayersBehindWall", NULL, true );
	m_pGlowPlayersMode = Modules::menu->AddParamList( this, "GlowPlayersMode", NULL, 0, " 0 - Outline\0 1 - Shell\0 2 - Ghost\0\0" );
	m_pGlowPlayersWidth = Modules::menu->AddParamInteger( this, "GlowPlayersWidth", NULL, 10, 0, 50 );
	m_pGlowPlayersColor = Modules::menu->AddParamColorRGB( this, "GlowPlayersColor", NULL, Color( 0, 255, 0, 255 ) );

	m_pGlowEntitiesBehindWall = Modules::menu->AddParamBool( this, "GlowEntitiesBehindWall", NULL, true );
	m_pGlowEntitiesMode = Modules::menu->AddParamList( this, "GlowEntitiesMode", NULL, 0, " 0 - Outline\0 1 - Shell\0 2 - Ghost\0\0" );
	m_pGlowEntitiesWidth = Modules::menu->AddParamInteger( this, "GlowEntitiesWidth", NULL, 10, 0, 50 );
	m_pGlowEntitiesColor = Modules::menu->AddParamColorRGB( this, "GlowEntitiesColor", NULL, Color( 255, 0, 0, 255 ) );

	m_pGlowItemsBehindWall = Modules::menu->AddParamBool( this, "GlowItemsBehindWall", NULL, true );
	m_pGlowItemsMode = Modules::menu->AddParamList( this, "GlowItemsMode", NULL, 0, " 0 - Outline\0 1 - Shell\0 2 - Ghost\0\0" );
	m_pGlowItemsWidth = Modules::menu->AddParamInteger( this, "GlowItemsWidth", NULL, 10, 0, 50 );
	m_pGlowItemsColor = Modules::menu->AddParamColorRGB( this, "GlowItemsColor", NULL, Color( 0, 0, 255, 255 ) );

	m_flOverrideColor = m_pChamsPlayersColor->GetColor();

	m_pfnglColor4f = GetProcAddress( (HMODULE)GameData::Modules::OpenGL, "glColor4f" );
	FEATURE_CHECK_SYMBOL( m_pfnglColor4f, "glColor4f" );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CChams::PostLoad( void )
{
	m_hglColor4f = Detours()->DetourFunction( m_pfnglColor4f, HOOKED_glColor4f, GET_FUNC_PTR( ORIG_glColor4f ) );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CChams::Unload( void )
{
	Detours()->RemoveDetour( m_hglColor4f );
}
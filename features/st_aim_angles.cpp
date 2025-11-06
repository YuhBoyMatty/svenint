// SvenInt (c) Sw1ft
// st_aim_angles.cpp

#include "stdafx.h"
#include "st_aim_angles.h"
#include "st_input_manager.h"
#include "strafe/strafe_utils.h"

using namespace Globals;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CAimAngles, aimangles, "Speedrun Tools", "Aim Angles" );

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_st_follow_point, "Set local player view angles to a point and follow it" )
{
	if ( args.ArgC() != 5 )
	{
		Msg( ( "Usage: sc_st_follow_point <x> <y> <z> <lerp>\n" ) );
		THIS_FEATURE()->StopFollowingPoint();
		return;
	}

	THIS_FEATURE()->FollowPoint( (float)atof( args[ 1 ] ), (float)atof( args[ 2 ] ), (float)atof( args[ 3 ] ), (float)atof( args[ 4 ] ) );
}

CON_COMMAND( sc_st_follow_point_stop, "Stop following point" )
{
	THIS_FEATURE()->StopFollowingPoint();
}

CON_COMMAND( sc_st_setangles, "Set local player view angles" )
{
	if ( args.ArgC() != 4 )
	{
		Msg( "Usage: sc_st_setangles <pitch> <yaw> <frames>\n" );
		return;
	}

	THIS_FEATURE()->SetAngles( (float)atof( args.Arg( 1 ) ), (float)atof( args.Arg( 2 ) ), atoi( args.Arg( 3 ) ) );
}

CON_COMMAND( sc_st_setangles_stop, "Stop setting angles" )
{
	THIS_FEATURE()->StopSetAngles();
}

CON_COMMAND( sc_st_setangles2, "Set local player view angles with given interpolation" )
{
	if ( args.ArgC() != 4 )
	{
		Msg( "Usage: sc_st_setangles <pitch> <yaw> <lerp>\n" );
		THIS_FEATURE()->StopSetAngles2();
		return;
	}

	THIS_FEATURE()->SetAngles2( (float)atof( args.Arg( 1 ) ), (float)atof( args.Arg( 2 ) ), (float)atof( args.Arg( 3 ) ) );
}

CON_COMMAND( sc_st_setangles2_stop, "Stop setting angles" )
{
	THIS_FEATURE()->StopSetAngles2();
}

//-----------------------------------------------------------------------------
// FollowPoint
//-----------------------------------------------------------------------------

void CAimAngles::FollowPoint( float x, float y, float z, float lerp )
{
	m_vecFollowPoint.x = x;
	m_vecFollowPoint.y = y;
	m_vecFollowPoint.z = z;
	m_flFollowPointLerp = lerp;

	m_bFollowPoint = true;
}

//-----------------------------------------------------------------------------
// SetAngles
//-----------------------------------------------------------------------------

void CAimAngles::SetAngles( float pitch, float yaw, int frames )
{
	if ( frames <= 0 )
		return;
	
	Vector va;
	cl_enginefuncs->GetViewAngles( va );

	float flPitch = pitch;
	float flYaw = yaw;

	float flNormalizedPitch = (float)Strafe::NormalizeDeg( static_cast<double>( flPitch ) - va[ PITCH ] );
	float flNormalizedYaw = (float)Strafe::NormalizeDeg( static_cast<double>( flYaw ) - va[ YAW ] );

	m_vecSetAngles[ PITCH ] = flPitch;
	m_vecSetAngles[ YAW ] = flYaw;

	m_vecSetAnglesSpeed[ PITCH ] = std::abs( flNormalizedPitch ) / frames;
	m_vecSetAnglesSpeed[ YAW ] = std::abs( flNormalizedYaw ) / frames;

	m_bSetAngles = true;
}

//-----------------------------------------------------------------------------
// SetAngles2
//-----------------------------------------------------------------------------

void CAimAngles::SetAngles2( float pitch, float yaw, float lerp )
{
	m_vecSetAngles2.x = pitch;
	m_vecSetAngles2.y = yaw;
	m_vecSetAngles2.z = 0.f;
	m_flSetAngles2Lerp = lerp;

	NormalizeAngles( m_vecSetAngles2 );

	m_bSetAngles2 = true;
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CAimAngles::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( !( m_bSetAngles || m_bSetAngles2 || m_bFollowPoint ) ||
		 !( !Features::inputmanager->IsInAction() || Features::inputmanager->IsRecording() ) )
		return kHookContinue;
	
	Vector va;
	auto cmd = pEvent->GetArg<usercmd_t *>( "cmd" );

	auto ChangeAngleBySpeed = []( float &flAngle, float flTargetAngle, float flChangeSpeed ) -> bool
	{
	#ifdef min
	#undef min
	#endif

		double adjustedTarget = (float)Strafe::NormalizeDeg( (double)flTargetAngle );
		double normalizedDiff = (float)Strafe::NormalizeDeg( adjustedTarget - (double)flAngle );
		double additionAbs = std::min( static_cast<double>( flChangeSpeed ), std::abs( normalizedDiff ) );

		flAngle = static_cast<float>( (double)flAngle + std::copysign( additionAbs, normalizedDiff ) );

		if ( std::abs( normalizedDiff ) > flChangeSpeed )
		{
			return true;
		}

		flAngle = flTargetAngle;
		return false;
	};

	if ( m_bFollowPoint )
	{
		Vector vecAngles, vecDir;

		cl_enginefuncs->GetViewAngles( va );

		vecDir = m_vecFollowPoint - localplayer->GetEyePosition();

		vecAngles.x = -atan2f( vecDir.z, vecDir.Length2D() ) * (float)( 180.0 / M_PI );
		vecAngles.y = atan2f( vecDir.y, vecDir.x ) * (float)( 180.0 / M_PI );
		vecAngles.z = 0.f;

		NormalizeAngles( vecAngles );

		float flNormalizedPitch = (float)Strafe::NormalizeDeg( vecAngles[ PITCH ] - va[ PITCH ] );
		float flNormalizedYaw = (float)Strafe::NormalizeDeg( vecAngles[ YAW ] - va[ YAW ] );

		float flSetPitchSpeed = std::abs( flNormalizedPitch ) * m_flFollowPointLerp;
		float flSetYawSpeed = std::abs( flNormalizedYaw ) * m_flFollowPointLerp;

		bool bPitchChanged = ChangeAngleBySpeed( va[ PITCH ], vecAngles[ PITCH ], flSetPitchSpeed );
		bool bYawChanged = ChangeAngleBySpeed( va[ YAW ], vecAngles[ YAW ], flSetYawSpeed );

		NormalizeAngles( va );

		if ( bPitchChanged || bYawChanged )
		{
			cl_enginefuncs->SetViewAngles( va );
			VectorCopy( va, cmd->viewangles );
		}
	}
	else if ( m_bSetAngles )
	{
		cl_enginefuncs->GetViewAngles( va );

		va.y = NormalizeAngle( va.y );

		bool bPitchChanged = ChangeAngleBySpeed( va[ PITCH ], m_vecSetAngles[ PITCH ], m_vecSetAnglesSpeed[ PITCH ] );
		bool bYawChanged = ChangeAngleBySpeed( va[ YAW ], m_vecSetAngles[ YAW ], m_vecSetAnglesSpeed[ YAW ] );

		//if ( bPitchChanged )
		//	cmd->viewangles[PITCH] = va[PITCH];

		//if ( bYawChanged )
		//	cmd->viewangles[YAW] = va[YAW];

		NormalizeAngles( va );

		if ( !bPitchChanged && !bYawChanged )
		{
			m_bSetAngles = false;
		}
		else
		{
			cl_enginefuncs->SetViewAngles( va );
			VectorCopy( va, cmd->viewangles );
		}
	}
	else if ( m_bSetAngles2 )
	{
		cl_enginefuncs->GetViewAngles( va );

		float flNormalizedPitch = (float)Strafe::NormalizeDeg( m_vecSetAngles2[ PITCH ] - va[ PITCH ] );
		float flNormalizedYaw = (float)Strafe::NormalizeDeg( m_vecSetAngles2[ YAW ] - va[ YAW ] );

		float flSetPitchSpeed = std::abs( flNormalizedPitch ) * m_flSetAngles2Lerp;
		float flSetYawSpeed = std::abs( flNormalizedYaw ) * m_flSetAngles2Lerp;

		bool bPitchChanged = ChangeAngleBySpeed( va[ PITCH ], m_vecSetAngles2[ PITCH ], flSetPitchSpeed );
		bool bYawChanged = ChangeAngleBySpeed( va[ YAW ], m_vecSetAngles2[ YAW ], flSetYawSpeed );

		NormalizeAngles( va );

		if ( !bPitchChanged && !bYawChanged )
		{
			m_bSetAngles = false;
		}
		else
		{
			cl_enginefuncs->SetViewAngles( va );
			VectorCopy( va, cmd->viewangles );
		}
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CAimAngles::CAimAngles( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_bSetAngles = false;
	m_bSetAngles2 = false;
	m_flSetAngles2Lerp = 1.f;
	m_bFollowPoint = false;
	m_flFollowPointLerp = 1.f;
}

//-----------------------------------------------------------------------------
// Post load feature
//-----------------------------------------------------------------------------

void CAimAngles::PostLoad( void )
{
	hookevents->RegisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall, kHookPriorityHigh );

	FEATURE_REGISTER_CCMD( sc_st_follow_point );
	FEATURE_REGISTER_CCMD( sc_st_follow_point_stop );
	FEATURE_REGISTER_CCMD( sc_st_setangles );
	FEATURE_REGISTER_CCMD( sc_st_setangles_stop );
	FEATURE_REGISTER_CCMD( sc_st_setangles2 );
	FEATURE_REGISTER_CCMD( sc_st_setangles2_stop );
}

//-----------------------------------------------------------------------------
// Unload feature
//-----------------------------------------------------------------------------

void CAimAngles::Unload( void )
{
	hookevents->UnregisterListener( this, kCL_CreateMove_HookEvent, kHookPostCall );

	FEATURE_UNREGISTER_CCMD( sc_st_follow_point );
	FEATURE_UNREGISTER_CCMD( sc_st_follow_point_stop );
	FEATURE_UNREGISTER_CCMD( sc_st_setangles );
	FEATURE_UNREGISTER_CCMD( sc_st_setangles_stop );
	FEATURE_UNREGISTER_CCMD( sc_st_setangles2 );
	FEATURE_UNREGISTER_CCMD( sc_st_setangles2_stop );
}
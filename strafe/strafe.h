/** This file is modified version of hlstrafe used in SourcePauseTool
 * Original authors are YaLTeR, Matherunner, Jukspa
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#pragma once

#include "game/hlsdk_mini.h"

namespace Strafe
{
#ifdef YAW
#undef YAW
#endif

	enum class StrafeType : unsigned char
	{
		MAXACCEL = 0,
		MAXANGLE,
		MAXDECCEL,
		CONSTSPEED
	};

	enum class StrafeDir : unsigned char
	{
		LEFT = 0,
		RIGHT,
		BEST,
		YAW,
		POINT
	};

	enum class Button : unsigned char
	{
		FORWARD = 0,
		FORWARD_LEFT,
		LEFT,
		BACK_LEFT,
		BACK,
		BACK_RIGHT,
		RIGHT,
		FORWARD_RIGHT
	};

	enum class PositionType : unsigned char
	{
		GROUND = 0,
		AIR,
		WATER
	};

	enum class HullType : unsigned char
	{
		NORMAL = 0,
		DUCKED = 1,
		POINT = 2
	};

	struct StrafeButtons
	{
		StrafeButtons() : AirLeft( Button::FORWARD ), AirRight( Button::FORWARD ), GroundLeft( Button::FORWARD ), GroundRight( Button::FORWARD )
		{
		}

		Button AirLeft;
		Button AirRight;
		Button GroundLeft;
		Button GroundRight;
	};

	struct PlayerData
	{
		PlayerData() :
			WaterLevel( 0 ),
			WaterType( 0 ),
			WaterJumpTime( 0 ),
			DuckTime( 0 ),
			Jump( false ),
			Duck( false ),
			Use( false ),
			Walk( false ),
			HasLongJumpModule( false ),
			ReduceWishspeed( false ),
			Ducking( false ),
			InDuck( false ),
			WaterJump( false ),
			Origin{ 0.f, 0.f, 0.f },
			Velocity{ 0.f, 0.f, 0.f },
			BaseVelocity{ 0.f, 0.f, 0.f },
			PosType( PositionType::GROUND ),
			ViewHeight( 0.f ),
			EntFriction( 1.f ),
			EntGravity( 1.f ),
			ClientMaxspeed( 270.f ),
			SpeedBeforeFriction( 0.0 )
		{
		}

		PlayerData( const PlayerData &other );
		PlayerData &operator=( const PlayerData &other );

		short WaterLevel : 8;
		short WaterType : 8;

		short WaterJumpTime;
		short DuckTime;

		bool Jump; // Force IN_JUMP
		bool Duck; // Force IN_DUCK
		bool Use; // Force IN_USE
		bool Walk; // Force IN_SPEED
		bool HasLongJumpModule;
		bool ReduceWishspeed;
		bool Ducking;
		bool InDuck;
		bool WaterJump;

		union { float Origin[ 3 ]; Vector vecOrigin; };
		union { float Velocity[ 3 ]; Vector vecVelocity; };
		union { float BaseVelocity[ 3 ]; Vector vecBaseVelocity; };

		PositionType PosType;

		float ViewHeight;
		float EntFriction;
		float EntGravity;
		float ClientMaxspeed;

		double SpeedBeforeFriction;
	};

	struct MovementVars
	{
		float Frametime = 1.f / 200.f;
		float Gravity;
		float Maxvelocity;
		float Maxspeed;
		float Stopspeed;
		float Stepsize;
		float Bounce;
		float Friction;
		float Edgefriction;
		float Accelerate;
		float Airaccelerate;
	};

	struct ProcessedFrame
	{
		ProcessedFrame() :
			Pitch( 0.f ),
			Yaw( 0.f ),
			Forward( false ),
			Left( false ),
			Right( false ),
			Back( false ),
			Processed( false ),
			Forwardspeed( 0.f ),
			Sidespeed( 0.f )
		{
		}

		float Pitch;
		float Yaw;

		bool Forward;
		bool Left;
		bool Right;
		bool Back;

		bool Processed;

		float Forwardspeed;
		float Sidespeed;
	};

	struct Frame
	{
		Frame() :
			Strafe( false ),
			StrafeVectorial( false ),
			UseGivenButtons( false ),
			AutoJump( false ),
			AutoDucktap( false ),
			Unused( false ),
			VectorialIncrement( 2.5f ),
			VectorialIncrementInvert( false ),
			VectorialOffset( 0.f ),
			VectorialSnap( 170.f ),
			Type( StrafeType::MAXACCEL ),
			Dir( StrafeDir::YAW ),
			Yaw( 0.0 ),
			X( 0.0 ),
			Y( 0.0 )
		{
		}

	public:
		PlayerData player;
		MovementVars vars;

		float VectorialIncrement;
		bool VectorialIncrementInvert;
		float VectorialOffset;
		float VectorialSnap;

		StrafeButtons buttons;
		StrafeType Type;
		StrafeDir Dir;

		bool Strafe;
		bool StrafeVectorial;
		bool UseGivenButtons;
		bool AutoJump;
		bool AutoDucktap;
		bool Unused;

		double Yaw;
		double X;
		double Y;

	public:
		inline StrafeType GetType() const { return Type; }
		inline StrafeDir GetDir() const { return Dir; }
		inline void SetType( StrafeType value ) { Type = value; }
		inline void SetDir( StrafeDir value ) { Dir = value; }

		inline double GetYaw() const { return Yaw; }
		inline double GetX() const { return X; }
		inline double GetY() const { return Y; }

		inline void SetYaw( double value ) { Yaw = value; }
		inline void SetX( double value ) { X = value; }
		inline void SetY( double value ) { Y = value; }
	};

	class CHLGameMovementPrediction
	{
	public:
		CHLGameMovementPrediction( Frame &_frame, ProcessedFrame &_out, Frame *_prevframe = NULL ) :
			frame( _frame ),
			player( _frame.player ),
			vars( _frame.vars ),
			out( _out ),
			prevframe( _prevframe  )
		{
			OnLadder = false;
		}

		inline Frame &GetStrafeFrame( void ) { return frame; }
		inline MovementVars &GetMovementVars( void ) { return vars; }
		inline PlayerData &GetPlayerData( void ) { return player; }

	public:
		// Can be useful to access some of functions, let them be public
		Vector GetPlayerMins( bool bDucked );
		Vector GetPlayerMaxs( bool bDucked );

		pmtrace_t TracePlayer( Vector &start, Vector &end, bool bDucked );
		pmtrace_t TraceLine( Vector &start, Vector &end );

		void CheckVelocity( void );
		int ClipVelocity( Vector in, Vector normal, Vector out, float overbounce );

		void AddGravity( void );
		void AddCorrectGravity( void );
		void FixupGravityVelocity( void );

		void Friction( void );

		void Accelerate( Vector wishdir, float wishspeed, float accel );
		void AirAccelerate( Vector wishdir, float wishspeed, float accel );

		int FlyMove( void );
		void WalkMove( void );
		void AirMove( void );
		void WaterMove( void );

		bool InWater( void );
		bool CheckWater( void );

		void CatagorizePosition( void );

		float SplineFraction( float value, float scale );

		void FixPlayerCrouchStuck( int direction );
		void UnDuck( void );
		void Duck( void );

		void LadderMove( physent_t *pLadder );
		physent_t *Ladder( void );

		void Jump( void );
		void WaterJump( void );
		void CheckWaterJump( void );

		void CheckParameters( void );
		void ReduceTimers( void );

		void PlayerMove( void );

	private:
		Frame &frame;
		PlayerData &player;
		MovementVars &vars;

		ProcessedFrame &out;

		Frame *prevframe;

		bool OnLadder;
		Vector MoveDir;
		Vector Forward, Right, Up;
	};

	/*
		Computes the new velocity given unit acceleration vector and wishspeed
		and stores the result in player.Velocity.
	*/
	void VectorFME( PlayerData &player, MovementVars &vars, double wishspeed, const double a[ 2 ] );

	/*
		Predicts jump, changes player.PosType
	*/
	bool PredictJump( Frame &frame );

	/*
		Predicts jump, changes player.Duck and vars.Frametime.
	*/
	bool PredictDucktap( Frame &frame, int iLowFrictionFPS );

	/*
		Applies the ground friction the same way as PM_Friction would, changing player.Velocity.
	*/
	void Friction( Frame &frame );

	/*
		Vectorial strafing function
	*/
	void StrafeVectorial( Frame &frame, ProcessedFrame &out, bool yawChanged );

	/*
		Main strafe function
	*/
	void Strafe( Frame &frame, ProcessedFrame &out );

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal speed gain.
	*/
	double MaxAccelTheta( Frame &frame, double wishspeed );

	/*
		Returns the angle between velocity and wishdir in [0, Pi] that will
		keep the speed constant as far as possible. Under certain conditions
		the angle from MaxAccelTheta will be returned.
	*/
	double ConstSpeedTheta( Frame &frame, double wishspeed );

	/*
		Returns the angle in radians - [-Pi; Pi) - between velocity and wishdir that will
		result in maximal speed gain into the given yaw - [-Pi; Pi). If velocity is zero, vel_yaw will
		be used in place of velocity angle.
	*/
	double MaxAccelIntoYawTheta( Frame &frame, double wishspeed, double vel_yaw, double yaw );

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal velocity angle change.
	*/
	double MaxAngleTheta( Frame &frame, double wishspeed );

	/*
		Returns the angle in radians - [0; Pi] - between velocity and wishdir that will
		result in maximal decceleration.
	*/
	double MaxDeccelTheta( Frame &frame, double wishspeed );

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes sideways with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double SideStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right );
	double SideStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right );
	double SideStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right, bool &strafed );
	double SideStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right );

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes to the best dir with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double BestStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw );
	double BestStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw );
	double BestStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool &strafed );
	double BestStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw );

	/*
		Finds the best yaw to use for the corresponding strafe type taking the anglemod compensation into account, then
		strafes to the given yaw with that yaw and returns it in radians, given fixed buttons.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double YawStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw );
	double YawStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw );
	double YawStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw, bool &strafed );
	double YawStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw );

	/*
		Finds the best yaw to use for the given strafe type taking the anglemod compensation into account, then
		strafes to the given point if needed with that yaw and returns it in radians, given fixed buttons. If not strafing
		is better, set strafed to false, otherwise - to true.
		The resulting velocity is stored in player.Velocity.
		Uses vel_yaw instead of the Velocity angle if Velocity is zero.
	*/
	double PointStrafe( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, StrafeType type, double point[ 2 ], bool &strafed );
}

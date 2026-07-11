/** This file is modifided version of HLTAS' hlstrafe used in SourcePauseTool
 * Original authors are YaLTeR, Matherunner, Jukspa
 * HLTAS/hlstrafe: https://github.com/HLTAS/hlstrafe
 * SPT: https://github.com/YaLTeR/SourcePauseTool
*/

#include "strafe.h"
#include "strafe_utils.h"
#include "gamedata.h"

#include <algorithm>

bool gbStrafedRight = false;

namespace Strafe
{
	//-----------------------------------------------------------------------------
	// HL Movement Prediction
	//-----------------------------------------------------------------------------

	#define TIME_TO_DUCK		0.4f
	#define MAX_CLIMB_SPEED		150.f
	#define STUCK_MOVEUP		1
	#define STUCK_MOVEDOWN		-1
	#define	STOP_EPSILON		0.1f
	#define PLAYER_LONGJUMP_SPEED 350.f
	#define PLAYER_DUCKING_MULTIPLIER 0.333f
	#define	CONTENTS_CURRENT_0		-9
	#define	CONTENTS_CURRENT_90		-10
	#define	CONTENTS_CURRENT_180	-11
	#define	CONTENTS_CURRENT_270	-12
	#define	CONTENTS_CURRENT_UP		-13
	#define	CONTENTS_CURRENT_DOWN	-14
	#define CONTENTS_TRANSLUCENT	-15

	Vector CHLGameMovementPrediction::GetPlayerMins( bool bDucked )
	{
		return bDucked ? VEC_DUCK_HULL_MIN : VEC_HULL_MIN;
	}

	Vector CHLGameMovementPrediction::GetPlayerMaxs( bool bDucked )
	{
		return bDucked ? VEC_DUCK_HULL_MAX : VEC_HULL_MAX;
	}

	pmtrace_t CHLGameMovementPrediction::TracePlayer( Vector &start, Vector &end, bool bDucked )
	{
		const int oldusehull = Globals::playermove->usehull();
		Globals::playermove->setusehull( bDucked ? PM_HULL_DUCKED_PLAYER : PM_HULL_PLAYER );

		pmtrace_t trace = Globals::playermove->funcs()->PM_PlayerTrace( start, end, PM_NORMAL, -1 );

		Globals::playermove->setusehull( oldusehull );
		return trace;
	}
	
	pmtrace_t CHLGameMovementPrediction::TraceLine( Vector &start, Vector &end )
	{
		const int oldusehull = Globals::playermove->usehull();
		Globals::playermove->setusehull( PM_HULL_POINT );

		pmtrace_t trace = Globals::playermove->funcs()->PM_PlayerTrace( start, end, PM_NORMAL, -1 );

		Globals::playermove->setusehull( oldusehull );
		return trace;
	}

	void CHLGameMovementPrediction::CheckVelocity( void )
	{
		int		i;

		//
		// bound velocity
		//
		for ( i = 0; i < 3; i++ )
		{
			// See if it's bogus.
			if ( !Vec_IsFloatFinite( player.vecVelocity[ i ] ) )
			{
				player.vecVelocity[ i ] = 0;
			}
			if ( !Vec_IsFloatFinite( player.vecOrigin[ i ] ) )
			{
				player.vecOrigin[ i ] = 0;
			}

			// Bound it.
			if ( player.vecVelocity[ i ] > vars.Maxvelocity )
			{
				player.vecVelocity[ i ] = vars.Maxvelocity;
			}
			else if ( player.vecVelocity[ i ] < -vars.Maxvelocity )
			{
				player.vecVelocity[ i ] = -vars.Maxvelocity;
			}
		}
	}

	int CHLGameMovementPrediction::ClipVelocity( Vector in, Vector normal, Vector out, float overbounce )
	{
		float	backoff;
		float	change;
		float angle;
		int		i, blocked;

		angle = normal[ 2 ];

		blocked = 0x00;            // Assume unblocked.
		if ( angle > 0.f )      // If the plane that is blocking us has a positive z component, then assume it's a floor.
			blocked |= 0x01;		// 
		if ( !angle )         // If the plane has no Z, it is vertical (wall/step)
			blocked |= 0x02;		// 

		// Determine how far along plane to slide based on incoming direction.
		// Scale by overbounce factor.
		backoff = DotProduct( in, normal ) * overbounce;

		for ( i = 0; i < 3; i++ )
		{
			change = normal[ i ] * backoff;
			out[ i ] = in[ i ] - change;
			// If out velocity is too small, zero it out.
			if ( out[ i ] > -STOP_EPSILON && out[ i ] < STOP_EPSILON )
				out[ i ] = 0.f;
		}

		// Return blocking flags.
		return blocked;
	}

	void CHLGameMovementPrediction::AddGravity( void )
	{
		float	ent_gravity;

		if ( player.EntGravity )
			ent_gravity = player.EntGravity;
		else
			ent_gravity = 1.f;

		// Add gravity incorrectly
		player.vecVelocity[ 2 ] -= ( ent_gravity * vars.Gravity * vars.Frametime );
		player.vecVelocity[ 2 ] += player.vecBaseVelocity[ 2 ] * vars.Frametime;
		player.vecBaseVelocity[ 2 ] = 0.f;
		CheckVelocity();
	}

	void CHLGameMovementPrediction::AddCorrectGravity( void )
	{
		float	ent_gravity;

		if ( player.WaterJumpTime )
			return;

		if ( player.EntGravity )
			ent_gravity = player.EntGravity;
		else
			ent_gravity = 1.f;

		// Add gravity so they'll be in the correct position during movement
		// yes, this 0.5 looks wrong, but it's not.  
		player.vecVelocity[ 2 ] -= ( ent_gravity * vars.Gravity * 0.5f * vars.Frametime );
		player.vecVelocity[ 2 ] += player.vecBaseVelocity[ 2 ] * vars.Frametime;
		player.vecBaseVelocity[ 2 ] = 0.f;

		CheckVelocity();
	}

	void CHLGameMovementPrediction::FixupGravityVelocity( void )
	{
		float	ent_gravity;

		if ( player.WaterJumpTime )
			return;

		if ( player.EntGravity )
			ent_gravity = player.EntGravity;
		else
			ent_gravity = 1.f;

		// Get the correct velocity for the end of the dt 
		player.vecVelocity[ 2 ] -= ( ent_gravity * vars.Gravity * vars.Frametime * 0.5f );

		CheckVelocity();
	}

	void CHLGameMovementPrediction::Friction( void )
	{
		float *vel;
		float	speed, newspeed, control;
		float	friction;
		float	drop;
		Vector newvel;

		// If we are in water jump cycle, don't apply friction
		if ( player.WaterJumpTime )
			return;

		// Get velocity
		vel = player.vecVelocity;

		// Calculate speed
		speed = sqrt( vel[ 0 ] * vel[ 0 ] + vel[ 1 ] * vel[ 1 ] + vel[ 2 ] * vel[ 2 ] );

		// If too slow, return
		if ( speed < 0.1f )
		{
			return;
		}

		drop = 0.f;

		// apply ground friction
		if ( player.PosType == PositionType::GROUND )  // On an entity that is the ground
		{
			Vector start, stop;
			pmtrace_t trace;

			start[ 0 ] = stop[ 0 ] = player.vecOrigin[ 0 ] + vel[ 0 ] / speed * 16.f;
			start[ 1 ] = stop[ 1 ] = player.vecOrigin[ 1 ] + vel[ 1 ] / speed * 16.f;
			start[ 2 ] = player.vecOrigin[ 2 ] + GetPlayerMins( player.Ducking )[ 2 ];
			stop[ 2 ] = start[ 2 ] - 34.f;

			trace = TracePlayer( start, stop, player.Ducking );

			if ( trace.fraction == 1.f )
				friction = vars.Friction * vars.Edgefriction;
			else
				friction = vars.Friction;

			// Grab friction value.
			//friction = pmove->movevars->friction;      

			friction *= player.EntFriction;  // player friction?

			// Bleed off some speed, but if we have less than the bleed
			//  threshhold, bleed the theshold amount.
			control = ( speed < vars.Stopspeed ) ? vars.Stopspeed : speed;
			// Add the amount to t'he drop amount.
			drop += control * friction * vars.Frametime;
		}

		// apply water friction
		//	if (pmove->waterlevel)
		//		drop += speed * pmove->movevars->waterfriction * waterlevel * vars.Frametime;

		// scale the velocity
		newspeed = speed - drop;
		if ( newspeed < 0.f )
			newspeed = 0.f;

		// Determine proportion of old speed we are using.
		newspeed /= speed;

		// Adjust velocity according to proportion.
		newvel[ 0 ] = vel[ 0 ] * newspeed;
		newvel[ 1 ] = vel[ 1 ] * newspeed;
		newvel[ 2 ] = vel[ 2 ] * newspeed;

		VectorCopy( newvel, player.vecVelocity );
	}

	void CHLGameMovementPrediction::Accelerate( Vector wishdir, float wishspeed, float accel )
	{
		int			i;
		float		addspeed, accelspeed, currentspeed;

		// If waterjumping, don't accelerate
		if ( player.WaterJumpTime )
			return;

		// See if we are changing direction a bit
		currentspeed = DotProduct( player.vecVelocity, wishdir );

		// Reduce wishspeed by the amount of veer.
		addspeed = wishspeed - currentspeed;

		// If not going to add any speed, done.
		if ( addspeed <= 0.f )
			return;

		// Determine amount of accleration.
		accelspeed = accel * vars.Frametime * wishspeed * player.EntFriction;

		// Cap at addspeed
		if ( accelspeed > addspeed )
			accelspeed = addspeed;

		// Adjust velocity.
		for ( i = 0; i < 3; i++ )
		{
			player.vecVelocity[ i ] += accelspeed * wishdir[ i ];
		}
	}

	void CHLGameMovementPrediction::AirAccelerate( Vector wishdir, float wishspeed, float accel )
	{
		int			i;
		float		addspeed, accelspeed, currentspeed, wishspd = wishspeed;

		if ( player.WaterJumpTime )
			return;

		// Cap speed
		//wishspd = VectorNormalize (pmove->wishveloc);

		if ( wishspd > 30.f )
			wishspd = 30.f;
		// Determine veer amount
		currentspeed = DotProduct( player.vecVelocity, wishdir );
		// See how much to add
		addspeed = wishspd - currentspeed;
		// If not adding any, done.
		if ( addspeed <= 0 )
			return;
		// Determine acceleration speed after acceleration

		accelspeed = accel * wishspeed * vars.Frametime * player.EntFriction;
		// Cap it
		if ( accelspeed > addspeed )
			accelspeed = addspeed;

		// Adjust pmove vel.
		for ( i = 0; i < 3; i++ )
		{
			player.vecVelocity[ i ] += accelspeed * wishdir[ i ];
		}
	}

	int CHLGameMovementPrediction::FlyMove( void )
	{
		int			bumpcount, numbumps;
		Vector		dir;
		float		d;
		int			numplanes;
		Vector		planes[ MAX_CLIP_PLANES ];
		Vector		primal_velocity, original_velocity;
		Vector      new_velocity;
		int			i, j;
		pmtrace_t	trace;
		Vector		end;
		float		time_left, allFraction;
		int			blocked;

		numbumps = 4;           // Bump up to four times

		blocked = 0;           // Assume not blocked
		numplanes = 0;           //  and not sliding along any planes
		VectorCopy( player.vecVelocity, original_velocity );  // Store original velocity
		VectorCopy( player.vecVelocity, primal_velocity );

		allFraction = 0;
		time_left = vars.Frametime;   // Total time for this movement operation.

		for ( bumpcount = 0; bumpcount < numbumps; bumpcount++ )
		{
			if ( !player.vecVelocity[ 0 ] && !player.vecVelocity[ 1 ] && !player.vecVelocity[ 2 ] )
				break;

			// Assume we can move all the way from the current origin to the
			//  end point.
			for ( i = 0; i < 3; i++ )
				end[ i ] = player.vecOrigin[ i ] + time_left * player.vecVelocity[ i ];

			// See if we can make it from origin to end point.
			trace = TracePlayer( player.vecOrigin, end, player.Ducking );

			allFraction += trace.fraction;
			// If we started in a solid object, or we were in solid space
			//  the whole way, zero out our velocity and return that we
			//  are blocked by floor and wall.
			if ( trace.allsolid )
			{	// entity is trapped in another solid
				VectorCopy( vec3_origin, player.vecVelocity );
				//Con_DPrintf("Trapped 4\n");
				return 4;
			}

			// If we moved some portion of the total distance, then
			//  copy the end position into the player.vecOrigin and 
			//  zero the plane counter.
			if ( trace.fraction > 0.f )
			{	// actually covered some distance
				VectorCopy( trace.endpos, player.vecOrigin );
				VectorCopy( player.vecVelocity, original_velocity );
				numplanes = 0;
			}

			// If we covered the entire distance, we are done
			//  and can return.
			if ( trace.fraction == 1.f )
				break;		// moved the entire distance

			//if (!trace.ent)
			//	Sys_Error ("PlayerTrace: !trace.ent");

			// Save entity that blocked us (since fraction was < 1.0)
			//  for contact
			// Add it if it's not already in the list!!!
			//AddToTouched( trace, player.vecVelocity );

			// If the plane we hit has a high z component in the normal, then
			//  it's probably a floor
			if ( trace.plane.normal[ 2 ] > 0.7f )
			{
				blocked |= 1;		// floor
			}
			// If the plane has a zero z component in the normal, then it's a 
			//  step or wall
			if ( trace.plane.normal[ 2 ] == 0.f )
			{
				blocked |= 2;		// step / wall
				//Con_DPrintf("Blocked by %i\n", trace.ent);
			}

			// Reduce amount of vars.Frametime left by total time left * fraction
			//  that we covered.
			time_left -= time_left * trace.fraction;

			// Did we run out of planes to clip against?
			if ( numplanes >= MAX_CLIP_PLANES )
			{	// this shouldn't really happen
				//  Stop our movement if so.
				VectorCopy( vec3_origin, player.vecVelocity );
				//Con_DPrintf("Too many planes 4\n");

				break;
			}

			// Set up next clipping plane
			VectorCopy( trace.plane.normal, planes[ numplanes ] );
			numplanes++;
			//

			// modify original_velocity so it parallels all of the clip planes
			//
					// relfect player velocity 
					// Only give this a try for first impact plane because you can get yourself stuck in an acute corner by jumping in place
					//  and pressing forward and nobody was really using this bounce/reflection feature anyway...
			if ( numplanes == 1 && !OnLadder && ( ( player.PosType == PositionType::GROUND ) || ( player.EntFriction != 1.f ) ) )
			{
				for ( i = 0; i < numplanes; i++ )
				{
					if ( planes[ i ][ 2 ] > 0.7f )
					{// floor or slope
						ClipVelocity( original_velocity, planes[ i ], new_velocity, 1 );
						VectorCopy( new_velocity, original_velocity );
					}
					else
						ClipVelocity( original_velocity, planes[ i ], new_velocity, 1.f + vars.Bounce * ( 1.f - player.EntFriction ) );
				}

				VectorCopy( new_velocity, player.vecVelocity );
				VectorCopy( new_velocity, original_velocity );
			}
			else
			{
				for ( i = 0; i < numplanes; i++ )
				{
					ClipVelocity(
						original_velocity,
						planes[ i ],
						player.vecVelocity,
						1 );
					for ( j = 0; j < numplanes; j++ )
						if ( j != i )
						{
							// Are we now moving against this plane?
							if ( DotProduct( player.vecVelocity, planes[ j ] ) < 0.f )
								break;	// not ok
						}
					if ( j == numplanes )  // Didn't have to clip, so we're ok
						break;
				}

				// Did we go all the way through plane set
				if ( i != numplanes )
				{	// go along this plane
					// player.vecVelocity is set in clipping call, no need to set again.
					;
				}
				else
				{	// go along the crease
					if ( numplanes != 2 )
					{
						//Con_Printf ("clip velocity, numplanes == %i\n",numplanes);
						VectorCopy( vec3_origin, player.vecVelocity );
						//Con_DPrintf("Trapped 4\n");

						break;
					}
					CrossProduct( planes[ 0 ], planes[ 1 ], dir );
					d = DotProduct( dir, player.vecVelocity );
					VectorScale( dir, d, player.vecVelocity );
				}

				//
				// if original velocity is against the original velocity, stop dead
				// to avoid tiny occilations in sloping corners
				//
				if ( DotProduct( player.vecVelocity, primal_velocity ) <= 0.f )
				{
					//Con_DPrintf("Back\n");
					VectorCopy( vec3_origin, player.vecVelocity );
					break;
				}
			}
		}

		if ( allFraction == 0.f )
		{
			VectorCopy( vec3_origin, player.vecVelocity );
			//Con_DPrintf( "Don't stick\n" );
		}

		return blocked;
	}

	void CHLGameMovementPrediction::WalkMove( void )
	{
		int			clip;
		bool		oldonground;
		int i;

		Vector		wishvel;
		float       spd;
		float		fmove, smove;
		Vector		wishdir;
		float		wishspeed;

		Vector forward, right, up;
		Vector dest, start;
		Vector original, originalvel;
		Vector down, downvel;
		float downdist, updist;

		pmtrace_t trace;

		VectorCopy( Forward, forward );
		VectorCopy( Right, right );
		VectorCopy( Up, up );

		// Copy movement amounts
		if ( frame.StrafeVectorial )
		{
			fmove = out.Forwardspeed;
			smove = out.Sidespeed;
		}
		else
		{
			fmove = out.Forward * out.Forwardspeed - out.Back * out.Forwardspeed;
			smove = out.Right * out.Sidespeed - out.Left * out.Sidespeed;
		}

		// Zero out z components of movement vectors
		forward[ 2 ] = 0.f;
		right[ 2 ] = 0.f;

		VectorNormalize( forward );  // Normalize remainder of vectors.
		VectorNormalize( right );    // 

		for ( i = 0; i < 2; i++ )       // Determine x and y parts of velocity
			wishvel[ i ] = forward[ i ] * fmove + right[ i ] * smove;

		wishvel[ 2 ] = 0.f;             // Zero out z part of velocity

		VectorCopy( wishvel, wishdir );   // Determine maginitude of speed of move
		wishspeed = VectorNormalize( wishdir );

		//
		// Clamp to server defined max speed
		//
		if ( wishspeed > vars.Maxspeed )
		{
			VectorScale( wishvel, vars.Maxspeed / wishspeed, wishvel );
			wishspeed = vars.Maxspeed;
		}

		// Set pmove velocity
		player.vecVelocity[ 2 ] = 0.f;
		Accelerate( wishdir, wishspeed, vars.Accelerate );
		player.vecVelocity[ 2 ] = 0.f;

		// Add in any base velocity to the current velocity.
		VectorAdd( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );

		spd = player.vecVelocity.Length();

		if ( spd < 1.f )
		{
			VectorClear( player.vecVelocity );
			return;
		}

		// If we are not moving, do nothing
		//if (!player.vecVelocity[0] && !player.vecVelocity[1] && !player.vecVelocity[2])
		//	return;

		oldonground = ( player.PosType == PositionType::GROUND );

		// first try just moving to the destination	
		dest[ 0 ] = player.vecOrigin[ 0 ] + player.vecVelocity[ 0 ] * vars.Frametime;
		dest[ 1 ] = player.vecOrigin[ 1 ] + player.vecVelocity[ 1 ] * vars.Frametime;
		dest[ 2 ] = player.vecOrigin[ 2 ];

		// first try moving directly to the next spot
		VectorCopy( dest, start );
		trace = TracePlayer( player.vecOrigin, dest, player.Ducking );
		// If we made it all the way, then copy trace end
		//  as new player position.
		if ( trace.fraction == 1.f )
		{
			VectorCopy( trace.endpos, player.vecOrigin );
			return;
		}

		if ( !oldonground &&   // Don't walk up stairs if not on ground.
			 player.WaterLevel == 0 )
			return;

		if ( player.WaterJumpTime )         // If we are jumping out of water, don't do anything more.
			return;

		// Try sliding forward both on ground and up 16 pixels
		//  take the move that goes farthest
		VectorCopy( player.vecOrigin, original );       // Save out original pos &
		VectorCopy( player.vecVelocity, originalvel );  //  velocity.

		// Slide move
		clip = FlyMove();

		// Copy the results out
		VectorCopy( player.vecOrigin, down );
		VectorCopy( player.vecVelocity, downvel );

		// Reset original values.
		VectorCopy( original, player.vecOrigin );

		VectorCopy( originalvel, player.vecVelocity );

		// Start out up one stair height
		VectorCopy( player.vecOrigin, dest );
		dest[ 2 ] += vars.Stepsize;

		trace = TracePlayer( player.vecOrigin, dest, player.Ducking );
		// If we started okay and made it part of the way at least,
		//  copy the results to the movement start position and then
		//  run another move try.
		if ( !trace.startsolid && !trace.allsolid )
		{
			VectorCopy( trace.endpos, player.vecOrigin );
		}

		// slide move the rest of the way.
		clip = FlyMove();

		// Now try going back down from the end point
		//  press down the stepheight
		VectorCopy( player.vecOrigin, dest );
		dest[ 2 ] -= vars.Stepsize;

		trace = TracePlayer( player.vecOrigin, dest, player.Ducking );

		// If we are not on the ground any more then
		//  use the original movement attempt
		if ( trace.plane.normal[ 2 ] < 0.7f )
			goto usedown;
		// If the trace ended up in empty space, copy the end
		//  over to the origin.
		if ( !trace.startsolid && !trace.allsolid )
		{
			VectorCopy( trace.endpos, player.vecOrigin );
		}
		// Copy this origion to up.
		VectorCopy( player.vecOrigin, up );

		// decide which one went farther
		downdist = ( down[ 0 ] - original[ 0 ] ) * ( down[ 0 ] - original[ 0 ] )
			+ ( down[ 1 ] - original[ 1 ] ) * ( down[ 1 ] - original[ 1 ] );
		updist = ( up[ 0 ] - original[ 0 ] ) * ( up[ 0 ] - original[ 0 ] )
			+ ( up[ 1 ] - original[ 1 ] ) * ( up[ 1 ] - original[ 1 ] );

		if ( downdist > updist )
		{
		usedown:
			VectorCopy( down, player.vecOrigin );
			VectorCopy( downvel, player.vecVelocity );
		}
		else // copy z value from slide move
			player.vecVelocity[ 2 ] = downvel[ 2 ];

	}

	void CHLGameMovementPrediction::WaterMove( void )
	{
		int		i;
		Vector	wishvel;
		float	wishspeed;
		Vector	wishdir;
		Vector	start, dest;
		Vector	forward, right, up;
		Vector  temp;
		pmtrace_t	trace;

		float fmove, smove;
		float speed, newspeed, addspeed, accelspeed;

		VectorCopy( Forward, forward );
		VectorCopy( Right, right );
		VectorCopy( Up, up );

		// Copy movement amounts
		if ( frame.StrafeVectorial )
		{
			fmove = out.Forwardspeed;
			smove = out.Sidespeed;
		}
		else
		{
			fmove = out.Forward * out.Forwardspeed - out.Back * out.Forwardspeed;
			smove = out.Right * out.Sidespeed - out.Left * out.Sidespeed;
		}

		//
		// user intentions
		//
		for ( i = 0; i < 3; i++ )
			wishvel[ i ] = forward[ i ] * fmove + right[ i ] * smove;

		// FIXME
		float upmove = 0.f; // pmove->cmd.upmove

		// Sinking after no other movement occurs
		if ( fmove == 0.f && smove == 0.f && upmove == 0.f )
			wishvel[ 2 ] -= 60.f;		// drift towards bottom
		else  // Go straight up by upmove amount.
			wishvel[ 2 ] += upmove;

		// Copy it over and determine speed
		VectorCopy( wishvel, wishdir );
		wishspeed = VectorNormalize( wishdir );

		// Cap speed.
		if ( wishspeed > vars.Maxspeed )
		{
			VectorScale( wishvel, vars.Maxspeed / wishspeed, wishvel );
			wishspeed = vars.Maxspeed;
		}
		// Slow us down a bit.
		wishspeed *= 0.8f;

		VectorAdd( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );
		// Water friction
		VectorCopy( player.vecVelocity, temp );
		speed = VectorNormalize( temp );
		if ( speed )
		{
			newspeed = speed - vars.Frametime * speed * vars.Friction * player.EntFriction;

			if ( newspeed < 0.f )
				newspeed = 0.f;
			VectorScale( player.vecVelocity, newspeed / speed, player.vecVelocity );
		}
		else
			newspeed = 0.f;

		//
		// water acceleration
		//
		if ( wishspeed < 0.1f )
		{
			return;
		}

		addspeed = wishspeed - newspeed;
		if ( addspeed > 0.f )
		{

			VectorNormalize( wishvel );
			accelspeed = vars.Accelerate * wishspeed * vars.Frametime * player.EntFriction;
			if ( accelspeed > addspeed )
				accelspeed = addspeed;

			for ( i = 0; i < 3; i++ )
				player.vecVelocity[ i ] += accelspeed * wishvel[ i ];
		}

		// Now move
		// assume it is a stair or a slope, so press down from stepheight above
		VectorMA( player.vecOrigin, vars.Frametime, player.vecVelocity, dest );
		VectorCopy( dest, start );
		start[ 2 ] += vars.Stepsize + 1.f;
		trace = TracePlayer( start, dest, player.Ducking );
		if ( !trace.startsolid && !trace.allsolid )	// FIXME: check steep slope?
		{	// walked up the step, so just keep result and exit
			VectorCopy( trace.endpos, player.vecOrigin );
			return;
		}

		// Try moving straight along out normal path.
		FlyMove();
	}

	void CHLGameMovementPrediction::AirMove( void )
	{
		int			i;
		Vector		wishvel;
		float		fmove, smove;
		Vector		wishdir;
		Vector		forward, right, up;
		float		wishspeed;

		// Copy movement amounts
		VectorCopy( Forward, forward );
		VectorCopy( Right, right );
		VectorCopy( Up, up );

		// Copy movement amounts
		if ( frame.StrafeVectorial )
		{
			fmove = out.Forwardspeed;
			smove = out.Sidespeed;
		}
		else
		{
			fmove = out.Forward * out.Forwardspeed - out.Back * out.Forwardspeed;
			smove = out.Right * out.Sidespeed - out.Left * out.Sidespeed;
		}

		// Zero out z components of movement vectors
		forward[ 2 ] = 0.f;
		right[ 2 ] = 0.f;
		// Renormalize
		VectorNormalize( forward );
		VectorNormalize( right );

		// Determine x and y parts of velocity
		for ( i = 0; i < 2; i++ )
		{
			wishvel[ i ] = forward[ i ] * fmove + right[ i ] * smove;
		}
		// Zero out z part of velocity
		wishvel[ 2 ] = 0.f;

		// Determine maginitude of speed of move
		VectorCopy( wishvel, wishdir );
		wishspeed = VectorNormalize( wishdir );

		// Clamp to server defined max speed
		if ( wishspeed > vars.Maxspeed )
		{
			VectorScale( wishvel, vars.Maxspeed / wishspeed, wishvel );
			wishspeed = vars.Maxspeed;
		}

		AirAccelerate( wishdir, wishspeed, vars.Airaccelerate );

		// Add in any base velocity to the current velocity.
		VectorAdd( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );

		FlyMove();
	}

	bool CHLGameMovementPrediction::InWater( void )
	{
		return ( player.WaterLevel > 1 );
	}

	bool CHLGameMovementPrediction::CheckWater( void )
	{
		Vector	point;
		int		cont;
		int		truecont;
		float     height;
		float		heightover2;

		const Vector vecMins = GetPlayerMins( player.Ducking );
		const Vector vecMaxs = GetPlayerMaxs( player.Ducking );

		// Pick a spot just above the players feet.
		point[ 0 ] = player.vecOrigin[ 0 ] + ( vecMins[ 0 ] + vecMaxs[ 0 ] ) * 0.5f;
		point[ 1 ] = player.vecOrigin[ 1 ] + ( vecMins[ 1 ] + vecMaxs[ 1 ] ) * 0.5f;
		point[ 2 ] = player.vecOrigin[ 2 ] + vecMins[ 2 ] + 1.f;

		// Assume that we are not in water at all.
		player.WaterLevel = 0;
		player.WaterType = CONTENTS_EMPTY;

		// Grab point contents.
		cont = Globals::playermove->funcs()->PM_PointContents( point, &truecont );
		// Are we under water? (not solid and not empty?)
		if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
		{
			// Set water type
			player.WaterType = cont;

			// We are at least at level one
			player.WaterLevel = 1;

			height = ( vecMins[ 2 ] + vecMaxs[ 2 ] );
			heightover2 = height * 0.5f;

			// Now check a point that is at the player hull midpoint.
			point[ 2 ] = player.vecOrigin[ 2 ] + heightover2;
			cont = Globals::playermove->funcs()->PM_PointContents( point, NULL );
			// If that point is also under water...
			if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
			{
				// Set a higher water level.
				player.WaterLevel = 2;

				// Now check the eye position.  (view_ofs is relative to the origin)
				point[ 2 ] = player.vecOrigin[ 2 ] + player.ViewHeight;

				cont = Globals::playermove->funcs()->PM_PointContents( point, NULL );
				if ( cont <= CONTENTS_WATER && cont > CONTENTS_TRANSLUCENT )
					player.WaterLevel = 3;  // In over our eyes
			}

			// Adjust velocity based on water current, if any.
			if ( ( truecont <= CONTENTS_CURRENT_0 ) &&
				 ( truecont >= CONTENTS_CURRENT_DOWN ) )
			{
				// The deeper we are, the stronger the current.
				static Vector current_table[] =
				{
					{1, 0, 0}, {0, 1, 0}, {-1, 0, 0},
					{0, -1, 0}, {0, 0, 1}, {0, 0, -1}
				};

				VectorMA( player.vecBaseVelocity, 50.f * player.WaterLevel, current_table[ CONTENTS_CURRENT_0 - truecont ], player.vecBaseVelocity );
			}
		}

		return player.WaterLevel > 1;
	}

	void CHLGameMovementPrediction::CatagorizePosition( void )
	{
		Vector		point;
		pmtrace_t		tr;

		// if the player hull point one unit down is solid, the player
		// is on ground

		// see if standing on something solid	

			// Doing this before we move may introduce a potential latency in water detection, but
			// doing it after can get us stuck on the bottom in water if the amount we move up
			// is less than the 1 pixel 'threshold' we're about to snap to.	Also, we'll call
			// this several times per frame, so we really need to avoid sticking to the bottom of
			// water on each call, and the converse case will correct itself if called twice.
		CheckWater();

		point[ 0 ] = player.vecOrigin[ 0 ];
		point[ 1 ] = player.vecOrigin[ 1 ];
		point[ 2 ] = player.vecOrigin[ 2 ] - 2.f;

		if ( player.vecVelocity[ 2 ] > 180.f )   // Shooting up really fast.  Definitely not on ground.
		{
			player.PosType = PositionType::AIR;
		}
		else
		{
			// Try and move down.
			tr = TracePlayer( player.vecOrigin, point, player.Ducking );
			// If we hit a steep plane, we are not on ground
			if ( tr.plane.normal[ 2 ] < 0.7f )
				player.PosType = PositionType::AIR;	// too steep
			else
				player.PosType = PositionType::GROUND;  // Otherwise, point to index of ent under us.

			// If we are on something...
			if ( player.PosType == PositionType::GROUND )
			{
				// Then we are not in water jump sequence
				player.WaterJumpTime = 0;
				// If we could make the move, drop us down that 1 pixel
				if ( player.WaterLevel < 2 && !tr.startsolid && !tr.allsolid )
					VectorCopy( tr.endpos, player.vecOrigin );
			}

			// Standing on an entity other than the world
			if ( tr.ent > 0 )   // So signal that we are touching something.
			{
				//AddToTouched( tr, player.vecVelocity );
			}
		}
	}

	float CHLGameMovementPrediction::SplineFraction( float value, float scale )
	{
		float valueSquared;

		value = scale * value;
		valueSquared = value * value;

		// Nice little ease-in, ease-out spline-like curve
		return 3.f * valueSquared - 2.f * valueSquared * value;
	}

	void CHLGameMovementPrediction::FixPlayerCrouchStuck( int direction )
	{
		int     hitent;
		int i;
		Vector test;

		hitent = Globals::playermove->funcs()->PM_TestPlayerPosition( player.vecOrigin, NULL );
		if ( hitent == -1 )
			return;

		VectorCopy( player.vecOrigin, test );
		for ( i = 0; i < 36; i++ )
		{
			player.vecOrigin[ 2 ] += (float)direction;
			hitent = Globals::playermove->funcs()->PM_TestPlayerPosition( player.vecOrigin, NULL );
			if ( hitent == -1 )
				return;
		}

		VectorCopy( test, player.vecOrigin ); // Failed
	}

	void CHLGameMovementPrediction::UnDuck( void )
	{
		int i;
		pmtrace_t trace;
		Vector newOrigin;

		VectorCopy( player.vecOrigin, newOrigin );

		if ( player.PosType == PositionType::GROUND )
		{
			for ( i = 0; i < 3; i++ )
			{
				newOrigin[ i ] += ( GetPlayerMins( true )[ i ] - GetPlayerMins( false )[ i ] );
			}
		}

		trace = TracePlayer( newOrigin, newOrigin, player.Ducking );

		if ( !trace.startsolid )
		{
			// Oh, no, changing hulls stuck us into something, try unsticking downward first.
			trace = TracePlayer( newOrigin, newOrigin, false );
			if ( trace.startsolid )
			{
				// See if we are stuck?  If so, stay ducked with the duck hull until we have a clear spot
				//Con_Printf( "unstick got stuck\n" );
				//pmove->usehull = 1;
				return;
			}

			player.Ducking = false;
			player.InDuck = false;
			player.ViewHeight = VEC_VIEW.z;
			player.DuckTime = 0;

			VectorCopy( newOrigin, player.vecOrigin );

			// Recatagorize position since ducking can change origin
			CatagorizePosition();
		}
	}

	void CHLGameMovementPrediction::Duck( void )
	{
		int i;
		float time;
		float duckFraction;

		int oldbuttons, buttons = ( player.Duck ? IN_DUCK : 0 );

		if ( prevframe != NULL )
			oldbuttons = ( prevframe->player.Duck ? IN_DUCK : 0 );
		else
			oldbuttons = buttons;

		int buttonsChanged = ( oldbuttons ^ buttons );	// These buttons have changed this frame
		int nButtonPressed = buttonsChanged & buttons;		// The changed ones still down are "pressed"

		int duckchange = buttonsChanged & IN_DUCK ? 1 : 0;
		int duckpressed = nButtonPressed & IN_DUCK ? 1 : 0;

		if ( player.Ducking && frame.StrafeVectorial )
		{
			out.Forwardspeed *= PLAYER_DUCKING_MULTIPLIER;
			out.Sidespeed *= PLAYER_DUCKING_MULTIPLIER;
			//pmove->cmd.upmove *= PLAYER_DUCKING_MULTIPLIER;
		}

		if ( ( buttons & IN_DUCK ) || player.InDuck || player.Ducking )
		{
			if ( buttons & IN_DUCK )
			{
				if ( ( nButtonPressed & IN_DUCK ) && !player.Ducking )
				{
					// Use 1 second so super long jump will work
					player.DuckTime = 1000;
					player.InDuck = true;
				}

				time = Q_max( 0.f, ( 1.f - (float)player.DuckTime / 1000.f ) );

				if ( player.InDuck )
				{
					// Finish ducking immediately if duck time is over or not on ground
					if ( ( (float)player.DuckTime / 1000.f <= ( 1.f - TIME_TO_DUCK ) ) ||
						 ( player.PosType == PositionType::AIR ) )
					{
						player.ViewHeight = VEC_DUCK_VIEW.z;
						player.Ducking = true;
						player.InDuck = false;

						// HACKHACK - Fudge for collision bug - no time to fix this properly
						if ( player.PosType == PositionType::GROUND )
						{
							for ( i = 0; i < 3; i++ )
							{
								player.vecOrigin[ i ] -= ( GetPlayerMins( true )[ i ] - GetPlayerMins( false )[ i ] );
							}
							// See if we are stuck?
							FixPlayerCrouchStuck( STUCK_MOVEUP );

							// Recatagorize position since ducking can change origin
							CatagorizePosition();
						}
					}
					else
					{
						float fMore = ( VEC_DUCK_HULL_MIN.x - VEC_HULL_MIN.x );

						// Calc parametric time
						duckFraction = SplineFraction( time, ( 1.f / TIME_TO_DUCK ) );
						player.ViewHeight = ( ( VEC_DUCK_VIEW.z - fMore ) * duckFraction ) + ( VEC_VIEW.z * ( 1.f - duckFraction ) );
					}
				}
			}
			else
			{
				// Try to unduck
				UnDuck();
			}
		}
	}

	void CHLGameMovementPrediction::LadderMove( physent_t *pLadder )
	{
		Vector		ladderCenter;
		pmtrace_t	trace;
		qboolean	onFloor;
		Vector		floor;
		Vector		modelmins, modelmaxs;

		Globals::playermove->funcs()->PM_GetModelBounds( pLadder->model, modelmins, modelmaxs );

		VectorAdd( modelmins, modelmaxs, ladderCenter );
		VectorScale( ladderCenter, 0.5f, ladderCenter );

		OnLadder = true;

		// On ladder, convert movement to be relative to the ladder

		VectorCopy( player.vecOrigin, floor );
		floor[ 2 ] += GetPlayerMins( player.Ducking )[ 2 ] - 1.f;

		if ( Globals::playermove->funcs()->PM_PointContents( floor, NULL ) == CONTENTS_SOLID )
			onFloor = true;
		else
			onFloor = false;

		player.EntGravity = 0.f;
		Globals::playermove->funcs()->PM_TraceModel( pLadder, player.vecOrigin, ladderCenter, &trace );
		if ( trace.fraction != 1.f )
		{
			float forward = 0.f, right = 0.f;
			Vector vpn, v_right;
			float flSpeed = MAX_CLIMB_SPEED;

			// they shouldn't be able to move faster than their maxspeed
			if ( flSpeed > vars.Maxspeed )
			{
				flSpeed = vars.Maxspeed;
			}

			VectorCopy( Forward, vpn );
			VectorCopy( Right, v_right );

			if ( player.Ducking )
			{
				flSpeed *= PLAYER_DUCKING_MULTIPLIER;
			}

			if ( frame.StrafeVectorial )
			{
				forward = out.Forwardspeed;
				right = out.Sidespeed;
			}
			else
			{
				forward = out.Forward * out.Forwardspeed - out.Back * out.Forwardspeed;
				right = out.Right * out.Sidespeed - out.Left * out.Sidespeed;
			}

			if ( frame.AutoJump || player.Jump )
			{
				OnLadder = false;
				VectorScale( trace.plane.normal, 270.f, player.vecVelocity );
			}
			else
			{
				if ( forward != 0.f || right != 0.f )
				{
					Vector velocity, perp, cross, lateral, tmp;
					float normal;

					//ALERT(at_console, "pev %.2f %.2f %.2f - ",
					//	pev->velocity.x, pev->velocity.y, pev->velocity.z);
					// Calculate player's intended velocity
					//Vector velocity = (forward * gpGlobals->v_forward) + (right * gpGlobals->v_right);
					VectorScale( vpn, forward, velocity );
					VectorMA( velocity, right, v_right, velocity );


					// Perpendicular in the ladder plane
					// Vector perp = CrossProduct( Vector(0,0,1), trace.vecPlaneNormal );
					// perp = perp.Normalize();
					VectorClear( tmp );
					tmp[ 2 ] = 1.f;
					CrossProduct( tmp, trace.plane.normal, perp );
					VectorNormalize( perp );


					// decompose velocity into ladder plane
					normal = DotProduct( velocity, trace.plane.normal );
					// This is the velocity into the face of the ladder
					VectorScale( trace.plane.normal, normal, cross );


					// This is the player's additional velocity
					VectorSubtract( velocity, cross, lateral );

					// This turns the velocity into the face of the ladder into velocity that
					// is roughly vertically perpendicular to the face of the ladder.
					// NOTE: It IS possible to face up and move down or face down and move up
					// because the velocity is a sum of the directional velocity and the converted
					// velocity through the face of the ladder -- by design.
					CrossProduct( trace.plane.normal, perp, tmp );
					VectorMA( lateral, -normal, tmp, player.vecVelocity );
					if ( onFloor && normal > 0.f )	// On ground moving away from the ladder
					{
						VectorMA( player.vecVelocity, MAX_CLIMB_SPEED, trace.plane.normal, player.vecVelocity );
					}
					//pev->velocity = lateral - (CrossProduct( trace.vecPlaneNormal, perp ) * normal);
				}
				else
				{
					VectorClear( player.vecVelocity );
				}
			}
		}
	}

	physent_t *CHLGameMovementPrediction::Ladder( void )
	{
		int			i;
		physent_t *pe;
		hull_t *hull;
		int			num;
		Vector		test;

		for ( i = 0; i < Globals::playermove->nummoveent(); i++ )
		{
			pe = Globals::playermove->moveents() + i;

			if ( pe->model && (modtype_t)Globals::playermove->funcs()->PM_GetModelType( pe->model ) == mod_brush && pe->skin == CONTENTS_LADDER )
			{

				hull = (hull_t *)Globals::playermove->funcs()->PM_HullForBsp( pe, test );
				num = hull->firstclipnode;

				// Offset the test point appropriately for this hull.
				VectorSubtract( player.vecOrigin, test, test );

				// Test the player's hull for intersection with this model
				if ( Globals::playermove->funcs()->PM_HullPointContents( hull, num, test ) == CONTENTS_EMPTY )
					continue;

				return pe;
			}
		}

		return NULL;
	}

	void CHLGameMovementPrediction::Jump( void )
	{
		int i;

		// See if we are waterjumping.  If so, decrement count and return.
		if ( player.WaterJumpTime )
		{
			player.WaterJumpTime -= (int)( vars.Frametime * 1000.f );
			if ( player.WaterJumpTime < 0 )
			{
				player.WaterJumpTime = 0;
			}
			return;
		}

		// If we are in the water most of the way...
		if ( player.WaterLevel >= 2 )
		{	// swimming, not jumping
			player.PosType = PositionType::AIR; // actually, PositionType::WATER

			if ( player.WaterType == CONTENTS_WATER )    // We move up a certain amount
				player.vecVelocity[ 2 ] = 100.f;
			else if ( player.WaterType == CONTENTS_SLIME )
				player.vecVelocity[ 2 ] = 80.f;
			else  // LAVA
				player.vecVelocity[ 2 ] = 50.f;

			return;
		}

		// No more effect
		if ( player.PosType == PositionType::AIR )
		{
			// Flag that we jumped.
			// HACK HACK HACK
			// Remove this when the game .dll no longer does physics code!!!!
			//pmove->oldbuttons |= IN_JUMP;	// don't jump again until released
			return;		// in air, so no effect
		}

		// In the air now.
		player.PosType = PositionType::AIR;

		// Acclerate upward
		// If we are ducking...
		if ( ( player.InDuck ) || ( player.Ducking ) )
		{
			// Adjust for super long jump module
			// UNDONE -- note this should be based on forward angles, not current velocity.
			if ( player.HasLongJumpModule &&
				 ( player.Duck ) &&
				 ( player.DuckTime > 0 ) &&
				 player.vecVelocity.Length() > 50.f )
			{
				Vector forward;

				VectorCopy( Forward, forward );

				for ( i = 0; i < 2; i++ )
				{
					player.vecVelocity[ i ] = forward[ i ] * PLAYER_LONGJUMP_SPEED * 1.6f;
				}

				player.vecVelocity[ 2 ] = sqrt( 2.f * 800.f * 56.f );
			}
			else
			{
				player.vecVelocity[ 2 ] = sqrt( 2.f * 800.f * 45.f );
			}
		}
		else
		{
			player.vecVelocity[ 2 ] = sqrt( 2.f * 800.f * 45.f );
		}

		// Decay it for simulation
		FixupGravityVelocity();
	}

	void CHLGameMovementPrediction::WaterJump( void )
	{
		if ( player.WaterJumpTime > 10000 )
		{
			player.WaterJumpTime = 10000;
		}

		if ( !player.WaterJumpTime )
			return;

		player.WaterJumpTime -= (int)( vars.Frametime * 1000.f );
		if ( player.WaterJumpTime < 0 || !player.WaterLevel )
		{
			player.WaterJumpTime = 0;
			player.WaterJump = false;
		}

		player.vecVelocity[ 0 ] = MoveDir[ 0 ];
		player.vecVelocity[ 1 ] = MoveDir[ 1 ];
	}

	void CHLGameMovementPrediction::CheckWaterJump( void )
	{
		constexpr float WJ_HEIGHT = 8.f;

		Vector	vecStart, vecEnd;
		Vector	flatforward;
		Vector	flatvelocity;
		float curspeed;
		pmtrace_t tr;

		// Already water jumping.
		if ( player.WaterJumpTime )
			return;

		// Don't hop out if we just jumped in
		if ( player.vecVelocity[ 2 ] < -180.f )
			return; // only hop out if we are moving up

		// See if we are backing up
		flatvelocity[ 0 ] = player.vecVelocity[ 0 ];
		flatvelocity[ 1 ] = player.vecVelocity[ 1 ];
		flatvelocity[ 2 ] = 0.f;

		// Must be moving
		curspeed = VectorNormalize( flatvelocity );

		// see if near an edge
		flatforward[ 0 ] = Forward[ 0 ];
		flatforward[ 1 ] = Forward[ 1 ];
		flatforward[ 2 ] = 0.f;
		VectorNormalize( flatforward );

		// Are we backing into water from steps or something?  If so, don't pop forward
		if ( curspeed != 0.f && ( DotProduct( flatvelocity, flatforward ) < 0.f ) )
			return;

		VectorCopy( player.vecOrigin, vecStart );
		vecStart[ 2 ] += WJ_HEIGHT;

		VectorMA( vecStart, 24.f, flatforward, vecEnd );

		// Trace, this trace should use the point sized collision hull
		tr = TraceLine( vecStart, vecEnd );
		if ( tr.fraction < 1.f && fabs( tr.plane.normal[ 2 ] ) < 0.1f )  // Facing a near vertical wall?
		{
			vecStart[ 2 ] += GetPlayerMaxs( player.Ducking )[ 2 ] - WJ_HEIGHT;
			VectorMA( vecStart, 24.f, flatforward, vecEnd );
			VectorMA( vec3_origin, -50.f, tr.plane.normal, MoveDir );

			tr = Globals::playermove->funcs()->PM_PlayerTrace( vecStart, vecEnd, PM_NORMAL, -1 );
			if ( tr.fraction == 1.f )
			{
				player.WaterJumpTime = 2000;
				player.vecVelocity[ 2 ] = 225.f;
				player.WaterJump = true;
			}
		}
	}

	void CHLGameMovementPrediction::CheckParameters( void )
	{
		float spd;
		float maxspeed;
		float fmove, smove, umove;
		Vector v_angle;

		if ( frame.StrafeVectorial )
		{
			fmove = out.Forwardspeed;
			smove = out.Sidespeed;
			umove = 0.f;
		}
		else
		{
			fmove = out.Forward * out.Forwardspeed - out.Back * out.Forwardspeed;
			smove = out.Right * out.Sidespeed - out.Left * out.Sidespeed;
			umove = 0.f;
		}

		spd = ( fmove * fmove ) + ( smove * smove ) + ( umove * umove );
		spd = sqrt( spd );

		maxspeed = player.ClientMaxspeed; //atof( pmove->Info_ValueForKey( pmove->physinfo, "maxspd" ) );
		if ( maxspeed != 0.f )
		{
			vars.Maxspeed = Q_min( maxspeed, vars.Maxspeed );
		}

		if ( ( player.PosType == PositionType::GROUND ) && ( player.Walk || player.Use ) )
		{
			if ( SVEN_VERSION() > SVEN_VERSION_CHECK( 5, 22, 0 ) || player.Walk )
			{
				vars.Maxspeed *= 0.3f;
			}
		}

		if ( ( spd != 0.f ) &&
			 ( spd > vars.Maxspeed ) )
		{
			float fRatio = vars.Maxspeed / spd;
			out.Forwardspeed *= fRatio;
			out.Sidespeed *= fRatio;
			//pmove->cmd.upmove *= fRatio;
		}
	}

	void CHLGameMovementPrediction::ReduceTimers( void )
	{
		if ( player.DuckTime > 0 )
		{
			player.DuckTime -= (int)( vars.Frametime * 1000.f );
			if ( player.DuckTime < 0 )
			{
				player.DuckTime = 0;
			}
		}
	}

	void CHLGameMovementPrediction::PlayerMove( void )
	{
		physent_t *pLadder = NULL;

		// Adjust speeds etc.
		CheckParameters();

		ReduceTimers();

		// Convert view angles to vectors
		QAngle vecViewAngles( out.Pitch, out.Yaw, 0.f );
		AngleVectors( vecViewAngles, &Forward, &Right, &Up );

		// Now that we are "unstuck", see where we are ( waterlevel and type, pmove->onground ).
		CatagorizePosition();

		pLadder = Ladder();

		Duck();

		if ( pLadder )
		{
			LadderMove( pLadder );
		}
		else
		{
			OnLadder = false;
		}

		// Slow down, I'm pulling it! (a box maybe) but only when I'm standing on ground
		//
		// JoshA: Moved this to CheckParamters rather than working on the velocity,
		// as otherwise it affects every integration step incorrectly.
		if ( ( player.PosType == PositionType::GROUND ) && player.Use )
		{
			if ( SVEN_VERSION() <= SVEN_VERSION_CHECK( 5, 22, 0 ) )
			{
				player.vecVelocity *= 0.3f;
			}
		}

		// Handle movement
		if ( OnLadder )
		{
			CheckWater();

			// Was jump button pressed?
			// If so, set velocity to 270 away from ladder.  This is currently wrong.
			// Also, set MOVE_TYPE to walk, too.
			if ( frame.AutoJump || frame.player.Jump )
			{
				if ( !pLadder )
				{
					Jump();
				}
			}

			// Perform the move accounting for any base velocity.
			VectorAdd( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );
			FlyMove();
			VectorSubtract( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );
			return;
		}
		
		if ( !InWater() )
		{
			AddCorrectGravity();
		}

		// If we are leaping out of the water, just update the counters.
		if ( player.WaterJumpTime )
		{
			WaterJump();
			FlyMove();

			// Make sure waterlevel is set correctly
			CheckWater();
			return;
		}

		// If we are swimming in the water, see if we are nudging against a place we can jump up out
		//  of, and, if so, start out jump.  Otherwise, if we are not moving up, then reset jump timer to 0
		if ( player.WaterLevel >= 2 )
		{
			if ( player.WaterLevel == 2 )
			{
				CheckWaterJump();
			}

			// If we are falling again, then we must not trying to jump out of water any more.
			if ( player.vecVelocity[ 2 ] < 0 && player.WaterJumpTime )
			{
				player.WaterJumpTime = 0;
			}

			// Was jump button pressed?
			if ( frame.AutoJump || player.Jump )
			{
				Jump();
			}

			// Perform regular water movement
			WaterMove();

			VectorSubtract( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );

			// Get a final position
			CatagorizePosition();
		}
		else // Not underwater
		{
			// Was jump button pressed?
			if ( frame.AutoJump || player.Jump )
			{
				if ( !pLadder )
				{
					Jump();
				}
			}

			// Fricion is handled before we add in any base velocity. That way, if we are on a conveyor, 
			//  we don't slow when standing still, relative to the conveyor.
			if ( player.PosType == PositionType::GROUND )
			{
				player.vecVelocity[ 2 ] = 0.f;
				Friction();
			}

			// Make sure velocity is valid.
			CheckVelocity();

			// Are we on ground now
			if ( player.PosType == PositionType::GROUND )
			{
				WalkMove();
			}
			else
			{
				AirMove();  // Take into account movement when in air.
			}

			// Set final flags.
			CatagorizePosition();

			// Now pull the base velocity back out.
			// Base velocity is set if you are on a moving object, like
			//  a conveyor (or maybe another monster?)
			VectorSubtract( player.vecVelocity, player.vecBaseVelocity, player.vecVelocity );

			// Make sure velocity is valid.
			CheckVelocity();

			// Add any remaining gravitational component.
			if ( !InWater() )
			{
				FixupGravityVelocity();
			}

			// If we are on ground, no downward velocity.
			if ( player.PosType == PositionType::GROUND )
			{
				player.vecVelocity[ 2 ] = 0.f;
			}
		}
	}

	//-----------------------------------------------------------------------------
	// HLStrafe
	//-----------------------------------------------------------------------------

	PlayerData::PlayerData( const PlayerData &other )
	{
		memcpy( this, &other, sizeof( PlayerData ) );
	}

	PlayerData &PlayerData::operator=( const PlayerData &other )
	{
		if ( this != &other )
			memcpy( this, &other, sizeof( PlayerData ) );
		return *this;
	}

	void VectorFME( PlayerData &player, MovementVars &vars, double wishspeed, const double a[ 2 ] )
	{
		double wishspeed_capped = ( player.PosType == PositionType::GROUND ) ? wishspeed : 30;
		double tmp = wishspeed_capped - DotProduct<float, double, 2>( player.Velocity, a );

		if ( tmp <= 0.0 )
			return;

		double accel = ( player.PosType == PositionType::GROUND ) ? vars.Accelerate : vars.Airaccelerate;
		double accelspeed = accel * wishspeed * player.EntFriction * vars.Frametime;

		if ( accelspeed <= tmp )
			tmp = accelspeed;

		player.Velocity[ 0 ] += static_cast<float>( a[ 0 ] * tmp );
		player.Velocity[ 1 ] += static_cast<float>( a[ 1 ] * tmp );
	}

	static inline void MapSpeeds( ProcessedFrame &out, const float flMaxspeed )
	{
		if ( out.Forward )
			out.Forwardspeed += flMaxspeed;

		if ( out.Back )
			out.Forwardspeed -= flMaxspeed;

		if ( out.Right )
			out.Sidespeed += flMaxspeed;

		if ( out.Left )
			out.Sidespeed -= flMaxspeed;
	}

	static inline double ButtonsPhi( Button button )
	{
		switch ( button )
		{
		case Button::FORWARD: return 0;
		case Button::FORWARD_LEFT: return M_PI / 4;
		case Button::LEFT: return M_PI / 2;
		case Button::BACK_LEFT: return 3 * M_PI / 4;
		case Button::BACK: return -M_PI;
		case Button::BACK_RIGHT: return -3 * M_PI / 4;
		case Button::RIGHT: return -M_PI / 2;
		case Button::FORWARD_RIGHT: return -M_PI / 4;
		default: return 0;
		}
	}

	static inline Button GetBestButtons( double theta, bool right )
	{
		if ( theta < M_PI / 8 )
			return Button::FORWARD;
		else if ( theta < 3 * M_PI / 8 )
			return right ? Button::FORWARD_RIGHT : Button::FORWARD_LEFT;
		else if ( theta < 5 * M_PI / 8 )
			return right ? Button::RIGHT : Button::LEFT;
		else if ( theta < 7 * M_PI / 8 )
			return right ? Button::BACK_RIGHT : Button::BACK_LEFT;
		else
			return Button::BACK;
	}

	double MaxAccelTheta( Frame &frame, double wishspeed )
	{
		double accel = ( frame.player.PosType == PositionType::GROUND ) ? frame.vars.Accelerate : frame.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * frame.player.EntFriction * frame.vars.Frametime;

		if ( accelspeed <= 0.0 )
			return M_PI;

		if ( IsZero<float, 2>( frame.player.Velocity ) )
			return 0.0;

		double wishspeed_capped = ( frame.player.PosType == PositionType::GROUND ) ? wishspeed : 30;
		double tmp = wishspeed_capped - accelspeed;

		if ( tmp <= 0.0 )
			return M_PI / 2;

		double speed = Length<float, 2>( frame.player.Velocity );

		if ( tmp < speed )
			return std::acos( tmp / speed );

		return 0.0;
	}

	double ConstSpeedTheta( Frame &frame, double wishspeed )
	{
		double gamma1 = frame.player.EntFriction * frame.vars.Frametime * wishspeed;
		double speedsqr = DotProduct<float, float, 2>( frame.player.Velocity, frame.player.Velocity );
		double numer, denom;

		if ( frame.player.PosType == PositionType::GROUND )
		{
			gamma1 *= frame.vars.Accelerate;

			double sqrdiff = frame.player.SpeedBeforeFriction * frame.player.SpeedBeforeFriction - speedsqr;
			double tmp = sqrdiff / gamma1;

			if ( tmp + gamma1 <= 2 * wishspeed )
			{
				numer = tmp - gamma1;
				denom = 2 * std::sqrt( speedsqr );
			}
			else if ( gamma1 > wishspeed && wishspeed * wishspeed >= sqrdiff )
			{
				numer = -std::sqrt( wishspeed * wishspeed - sqrdiff );
				denom = std::sqrt( speedsqr );
			}
			else
			{
				return MaxAccelTheta( frame, wishspeed );
			}
		}
		else
		{
			gamma1 *= frame.vars.Airaccelerate;

			if ( gamma1 <= 60 )
			{
				numer = -gamma1;
				denom = 2 * std::sqrt( speedsqr );
			}
			else
			{
				numer = -30;
				denom = std::sqrt( speedsqr );
			}
		}

		if ( denom < std::fabs( numer ) )
			return MaxAccelTheta( frame, wishspeed );

		return std::acos( numer / denom );
	}

	double MaxAccelIntoYawTheta( Frame &frame, double wishspeed, double vel_yaw, double yaw )
	{
		if ( !IsZero<float, 2>( frame.player.Velocity ) )
			vel_yaw = Atan2( frame.player.Velocity[ 1 ], frame.player.Velocity[ 0 ] );

		double theta = MaxAccelTheta( frame, wishspeed );

		if ( theta == 0.0 || theta == M_PI )
			return NormalizeRad( yaw - vel_yaw + theta );

		return std::copysign( theta, NormalizeRad( yaw - vel_yaw ) );
	}

	double MaxAngleTheta( Frame &frame, double wishspeed )
	{
		double speed = Length<float, 2>( frame.player.Velocity );
		double accel = ( frame.player.PosType == PositionType::GROUND ) ? frame.vars.Accelerate : frame.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * frame.player.EntFriction * frame.vars.Frametime;

		if ( accelspeed <= 0.0 )
		{
			double wishspeed_capped = ( frame.player.PosType == PositionType::GROUND ) ? wishspeed : 30;
			accelspeed *= -1;

			if ( accelspeed >= speed )
			{
				if ( wishspeed_capped >= speed )
					return 0.0;
				else
					return std::acos( wishspeed_capped / speed ); // The actual angle needs to be _less_ than this.
			}
			else
			{
				if ( wishspeed_capped >= speed )
					return std::acos( accelspeed / speed );
				else
					return std::acos( std::min( accelspeed, wishspeed_capped ) / speed ); // The actual angle needs to be _less_ than this if wishspeed_capped <= accelspeed.
			}
		}
		else
		{
			if ( accelspeed >= speed )
				return M_PI;
			else
				return std::acos( -1 * accelspeed / speed );
		}
	}

	double MaxDeccelTheta( Frame &frame, double wishspeed )
	{
		double speed = Length<float, 2>( frame.player.Velocity );
		double accel = ( frame.player.PosType == PositionType::GROUND ) ? frame.vars.Accelerate : frame.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * frame.player.EntFriction * frame.vars.Frametime;

		if ( accelspeed < 0.0 )
		{
			double wishspeed_capped = ( frame.player.PosType == PositionType::GROUND ) ? wishspeed : 30;
			return std::acos( wishspeed_capped / speed ); // The actual angle needs to be _less_ than this.
		}
		else
		{
			return M_PI;
		}
	}

	static double SideStrafeGeneral( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double theta, bool right, float new_vel[ 2 ] )
	{
		if ( !IsZero<float, 2>( frame.player.Velocity ) )
			vel_yaw = Atan2( frame.player.Velocity[ 1 ], frame.player.Velocity[ 0 ] );

		if ( frame.UseGivenButtons )
		{
			if ( frame.player.PosType == PositionType::GROUND )
			{
				if ( right )
					usedButton = frame.buttons.GroundRight;
				else
					usedButton = frame.buttons.GroundLeft;
			}
			else
			{
				if ( right )
					usedButton = frame.buttons.AirRight;
				else
					usedButton = frame.buttons.AirLeft;
			}
		}
		else
		{
			// If the velocity is zero, theta is based on the viewangle yaw, which means the button
			// will be based on the viewangle yaw, which is wrong. Force the button to forward when
			// the velocity is zero, this makes sense as it will be set to forward anyway as soon
			// as the velocity becomes non-zero (because theta is small initially).

			if ( IsZero<float, 2>( frame.player.Velocity ) )
				usedButton = Button::FORWARD;
			else
				usedButton = GetBestButtons( theta, right );
		}

		double phi = ButtonsPhi( usedButton );
		theta = right ? -theta : theta;

		double yaw = NormalizeRad( vel_yaw - phi + theta );

		//double avec[ 2 ] = { std::cos( yaw + phi ), std::sin( yaw + phi ) };

		//PlayerData pl = frame.player;
		//VectorFME( pl, frame.vars, wishspeed, avec );
		//VecCopy<float, 2>( pl.Velocity, new_vel );

		gbStrafedRight = right;
		return yaw;
	}

	double SideStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right )
	{
		double theta = MaxAccelTheta( frame, wishspeed );

		float new_vel[ 2 ];

		double yaw = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, right, new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return yaw;
	}

	double SideStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right )
	{
		double theta = ConstSpeedTheta( frame, wishspeed );

		float new_vel[ 2 ];

		double yaw = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, right, new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return yaw;
	}

	double SideStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right )
	{
		double theta = MaxAngleTheta( frame, wishspeed );

		float new_vel[ 2 ];

		double yaw = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, right, new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return yaw;
	}

	double SideStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool right, bool &strafed )
	{
		// Check a bunch of stuff.
		double speed = Length<float, 2>( frame.player.Velocity );
		double accel = ( frame.player.PosType == PositionType::GROUND ) ? frame.vars.Accelerate : frame.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * frame.player.EntFriction * frame.vars.Frametime;
		double wishspeed_capped = ( frame.player.PosType == PositionType::GROUND ) ? wishspeed : 30;

		if ( speed == 0.0 || accelspeed == 0.0 || ( accelspeed < 0.0 && accelspeed <= -wishspeed_capped * 2 ) )
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double theta = MaxDeccelTheta( frame, wishspeed );

		float new_vel[ 2 ];

		double yaw = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, right, new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return yaw;
	}

	double BestStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw )
	{
		float temp_vel[ 2 ], orig_vel[ 2 ];
		double yaws[ 2 ];

		Button buttons[ 2 ];

		VecCopy<float, 2>( frame.player.Velocity, orig_vel );

		yaws[ 0 ] = SideStrafeMaxAccel( frame, wishspeed, buttons[ 0 ], vel_yaw, false );

		VecCopy<float, 2>( frame.player.Velocity, temp_vel );
		VecCopy<float, 2>( orig_vel, frame.player.Velocity );

		yaws[ 1 ] = SideStrafeMaxAccel( frame, wishspeed, buttons[ 1 ], vel_yaw, true );

		double speedsqrs[ 2 ] =
		{
			DotProduct<float, float, 2>( temp_vel, temp_vel ),
			DotProduct<float, float, 2>( frame.player.Velocity, frame.player.Velocity )
		};

		if ( speedsqrs[ 0 ] > speedsqrs[ 1 ] )
		{
			//VecCopy<float, 2>( temp_vel, frame.player.Velocity );
			usedButton = buttons[ 0 ];
			return yaws[ 0 ];
		}
		else
		{
			usedButton = buttons[ 1 ];
			return yaws[ 1 ];
		}
	}

	double BestStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw )
	{
		float temp_vel[ 2 ], orig_vel[ 2 ];
		double yaws[ 2 ];

		Button buttons[ 2 ];

		VecCopy<float, 2>( frame.player.Velocity, orig_vel );

		yaws[ 0 ] = SideStrafeMaxAngle( frame, wishspeed, buttons[ 0 ], vel_yaw, false );

		VecCopy<float, 2>( frame.player.Velocity, temp_vel );
		VecCopy<float, 2>( orig_vel, frame.player.Velocity );

		yaws[ 1 ] = SideStrafeMaxAngle( frame, wishspeed, buttons[ 1 ], vel_yaw, true );

		double old_speed = Length<float, 2>( orig_vel );
		double speeds[ 2 ] = { Length<float, 2>( temp_vel ), Length<float, 2>( frame.player.Velocity ) };

		double cosangles[ 2 ] =
		{
			DotProduct<float, float, 2>( temp_vel, orig_vel ) / ( old_speed * speeds[ 0 ] ),
			DotProduct<float, float, 2>( frame.player.Velocity, orig_vel ) / ( old_speed * speeds[ 1 ] )
		};

		if ( cosangles[ 0 ] < cosangles[ 1 ] )
		{
			//VecCopy<float, 2>( temp_vel, frame.player.Velocity );
			usedButton = buttons[ 0 ];
			return yaws[ 0 ];
		}
		else
		{
			usedButton = buttons[ 1 ];
			return yaws[ 1 ];
		}
	}

	double BestStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, bool &strafed )
	{
		float temp_vel[ 2 ], orig_vel[ 2 ];
		double yaws[ 2 ];

		Button buttons[ 2 ];

		VecCopy<float, 2>( frame.player.Velocity, orig_vel );

		yaws[ 0 ] = SideStrafeMaxDeccel( frame, wishspeed, buttons[ 0 ], vel_yaw, false, strafed );

		VecCopy<float, 2>( frame.player.Velocity, temp_vel );
		VecCopy<float, 2>( orig_vel, frame.player.Velocity );

		yaws[ 1 ] = SideStrafeMaxDeccel( frame, wishspeed, buttons[ 1 ], vel_yaw, true, strafed );

		// The condition for strafed does not depend on the strafing direction so
		// either both functions returned true, or both returned false.
		if ( !strafed )
			return 0.0;

		double speedsqrs[ 2 ] =
		{
			DotProduct<float, float, 2>( temp_vel, temp_vel ),
			DotProduct<float, float, 2>( frame.player.Velocity, frame.player.Velocity )
		};

		if ( speedsqrs[ 0 ] < speedsqrs[ 1 ] )
		{
			//VecCopy<float, 2>( temp_vel, frame.player.Velocity );
			usedButton = buttons[ 0 ];
			return yaws[ 0 ];
		}
		else
		{
			usedButton = buttons[ 1 ];
			return yaws[ 1 ];
		}
	}

	double BestStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw )
	{
		float temp_vel[ 2 ], orig_vel[ 2 ];
		double yaws[ 2 ];

		Button buttons[ 2 ];

		VecCopy<float, 2>( frame.player.Velocity, orig_vel );

		yaws[ 0 ] = SideStrafeConstSpeed( frame, wishspeed, buttons[ 0 ], vel_yaw, false );

		VecCopy<float, 2>( frame.player.Velocity, temp_vel );
		VecCopy<float, 2>( orig_vel, frame.player.Velocity );

		yaws[ 1 ] = SideStrafeConstSpeed( frame, wishspeed, buttons[ 1 ], vel_yaw, true );

		double speedsqrs[ 2 ] =
		{
			DotProduct<float, float, 2>( temp_vel, temp_vel ),
			DotProduct<float, float, 2>( frame.player.Velocity, frame.player.Velocity )
		};

		double oldspeedsqr = DotProduct<float, float, 2>( orig_vel, orig_vel );

		if ( std::fabs( oldspeedsqr - speedsqrs[ 0 ] ) <= std::fabs( oldspeedsqr - speedsqrs[ 1 ] ) )
		{
			//VecCopy<float, 2>( temp_vel, frame.player.Velocity );
			usedButton = buttons[ 0 ];
			return yaws[ 0 ];
		}
		else
		{
			usedButton = buttons[ 1 ];
			return yaws[ 1 ];
		}
	}

	double YawStrafeMaxAccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw )
	{
		double theta = MaxAccelIntoYawTheta( frame, wishspeed, vel_yaw, yaw );

		float new_vel[ 2 ];

		double result = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, std::fabs( theta ), ( theta < 0 ), new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return result;
	}

	double YawStrafeMaxAngle( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw )
	{
		double theta = MaxAngleTheta( frame, wishspeed );

		float new_vel[ 2 ];

		if ( !IsZero<float, 2>( frame.player.Velocity ) )
			vel_yaw = Atan2( frame.player.Velocity[ 1 ], frame.player.Velocity[ 0 ] );

		double result = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, ( NormalizeRad( yaw - vel_yaw ) < 0 ), new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return result;
	}

	double YawStrafeMaxDeccel( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw, bool &strafed )
	{
		// Check a bunch of stuff.
		double speed = Length<float, 2>( frame.player.Velocity );
		double accel = ( frame.player.PosType == PositionType::GROUND ) ? frame.vars.Accelerate : frame.vars.Airaccelerate;
		double accelspeed = accel * wishspeed * frame.player.EntFriction * frame.vars.Frametime;
		double wishspeed_capped = ( frame.player.PosType == PositionType::GROUND ) ? wishspeed : 30;

		if ( speed == 0.0 || accelspeed == 0.0 || ( accelspeed < 0.0 && accelspeed <= -wishspeed_capped * 2 ) )
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double theta = MaxDeccelTheta( frame, wishspeed );

		float new_vel[ 2 ];

		vel_yaw = Atan2( frame.player.Velocity[ 1 ], frame.player.Velocity[ 0 ] );
		double result = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, ( NormalizeRad( yaw - vel_yaw ) < 0 ), new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return result;
	}

	double YawStrafeConstSpeed( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, double yaw )
	{
		double theta = ConstSpeedTheta( frame, wishspeed );

		float new_vel[ 2 ];

		double result = SideStrafeGeneral( frame, wishspeed, usedButton, vel_yaw, theta, ( NormalizeRad( yaw - vel_yaw ) < 0 ), new_vel );

		//VecCopy<float, 2>( new_vel, frame.player.Velocity );
		return result;
	}

	double PointStrafe( Frame &frame, double wishspeed, Button &usedButton, double vel_yaw, StrafeType type, double point[ 2 ], bool &strafed )
	{
		// Covers the case where both vectors are zero.
		if ( Distance<float, double, 2>( frame.player.Origin, point ) <= 2.0 )
		{
			strafed = false;
			return 0.0;
		}

		strafed = true;

		double difference[ 2 ];
		VecSubtract<double, float, 2>( point, frame.player.Origin, difference );
		double yaw = Atan2( difference[ 1 ], difference[ 0 ] );

		frame.SetYaw( yaw * M_RAD2DEG );

		switch ( type )
		{
		default:
		case StrafeType::MAXACCEL: return YawStrafeMaxAccel( frame, wishspeed, usedButton, vel_yaw, yaw );
		case StrafeType::MAXANGLE: return YawStrafeMaxAngle( frame, wishspeed, usedButton, vel_yaw, yaw );
		case StrafeType::MAXDECCEL: return YawStrafeMaxDeccel( frame, wishspeed, usedButton, vel_yaw, yaw, strafed );
		case StrafeType::CONSTSPEED: return YawStrafeConstSpeed( frame, wishspeed, usedButton, vel_yaw, yaw );
			// TODO add the rest of the calls when the functions are done.
		}
	}

	void StrafeVectorial( Frame &frame, ProcessedFrame &out, bool yawChanged )
	{
		ProcessedFrame dummy;

		bool useGivenButtons = frame.UseGivenButtons;
		StrafeButtons buttons = frame.buttons;

		frame.UseGivenButtons = true;
		frame.buttons = StrafeButtons();

		Strafe( frame, dummy );

		if ( dummy.Forward )
		{
			if ( !yawChanged && frame.VectorialIncrement > 0 )
			{
				double adjustedTarget = NormalizeDeg( frame.GetYaw() + frame.VectorialOffset );
				double normalizedDiff = NormalizeDeg( adjustedTarget - out.Yaw );
				double additionAbs = std::min( static_cast<double>( frame.VectorialIncrement ), std::abs( normalizedDiff ) );

			#pragma warning(push)
			#pragma warning(disable: 4244)

				if ( std::abs( normalizedDiff ) > frame.VectorialSnap )
					out.Yaw = adjustedTarget;
				else
					out.Yaw = frame.VectorialIncrementInvert ? ( out.Yaw + additionAbs ) : ( out.Yaw + std::copysign( additionAbs, normalizedDiff ) );

			#pragma warning(pop)
			}

			double thetaDeg = dummy.Yaw;
			double diff = Strafe::NormalizeDeg( out.Yaw - thetaDeg ) * M_DEG2RAD;

			out.Forwardspeed = static_cast<float>( std::cos( diff ) * frame.player.ClientMaxspeed );
			out.Sidespeed = static_cast<float>( std::sin( diff ) * frame.player.ClientMaxspeed );
			out.Processed = true;
		}

		frame.UseGivenButtons = useGivenButtons;
		frame.buttons = buttons;
	}

	void Strafe( Frame &frame, ProcessedFrame &out )
	{
		double wishspeed = frame.player.ClientMaxspeed;

		if ( frame.player.ReduceWishspeed )
			wishspeed *= PLAYER_DUCKING_MULTIPLIER;

		bool strafed = true;
		Button usedButton;

		double vel_yaw = out.Yaw * M_DEG2RAD;

		switch ( frame.GetDir() )
		{
		case StrafeDir::LEFT:
			if ( frame.GetType() == StrafeType::MAXACCEL )
				out.Yaw = static_cast<float>( SideStrafeMaxAccel( frame, wishspeed, usedButton, vel_yaw, false ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXANGLE )
				out.Yaw = static_cast<float>( SideStrafeMaxAngle( frame, wishspeed, usedButton, vel_yaw, false ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::CONSTSPEED )
				out.Yaw = static_cast<float>( SideStrafeConstSpeed( frame, wishspeed, usedButton, vel_yaw, false ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXDECCEL )
			{
				auto yaw = static_cast<float>( SideStrafeMaxDeccel( frame, wishspeed, usedButton, vel_yaw, false, strafed ) * M_RAD2DEG );

				if ( strafed )
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::RIGHT:
			if ( frame.GetType() == StrafeType::MAXACCEL )
				out.Yaw = static_cast<float>( SideStrafeMaxAccel( frame, wishspeed, usedButton, vel_yaw, true ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXANGLE )
				out.Yaw = static_cast<float>( SideStrafeMaxAngle( frame, wishspeed, usedButton, vel_yaw, true ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::CONSTSPEED )
				out.Yaw = static_cast<float>( SideStrafeConstSpeed( frame, wishspeed, usedButton, vel_yaw, true ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXDECCEL )
			{
				auto yaw = static_cast<float>( SideStrafeMaxDeccel( frame, wishspeed, usedButton, vel_yaw, true, strafed ) * M_RAD2DEG );

				if ( strafed )
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::BEST:
			if ( frame.GetType() == StrafeType::MAXACCEL )
				out.Yaw = static_cast<float>( BestStrafeMaxAccel( frame, wishspeed, usedButton, vel_yaw ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXANGLE )
				out.Yaw = static_cast<float>( BestStrafeMaxAngle( frame, wishspeed, usedButton, vel_yaw ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::CONSTSPEED )
				out.Yaw = static_cast<float>( BestStrafeConstSpeed( frame, wishspeed, usedButton, vel_yaw ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXDECCEL )
			{
				auto yaw = static_cast<float>( BestStrafeMaxDeccel( frame, wishspeed, usedButton, vel_yaw, strafed ) * M_RAD2DEG );

				if ( strafed )
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::YAW:
			if ( frame.GetType() == StrafeType::MAXACCEL )
				out.Yaw = static_cast<float>( YawStrafeMaxAccel( frame, wishspeed, usedButton, vel_yaw, frame.GetYaw() * M_DEG2RAD ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXANGLE )
				out.Yaw = static_cast<float>( YawStrafeMaxAngle( frame, wishspeed, usedButton, vel_yaw, frame.GetYaw() * M_DEG2RAD ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::CONSTSPEED )
				out.Yaw = static_cast<float>( YawStrafeConstSpeed( frame, wishspeed, usedButton, vel_yaw, frame.GetYaw() * M_DEG2RAD ) * M_RAD2DEG );
			else if ( frame.GetType() == StrafeType::MAXDECCEL )
			{
				auto yaw = static_cast<float>( YawStrafeMaxDeccel( frame, wishspeed, usedButton, vel_yaw, frame.GetYaw() * M_DEG2RAD, strafed ) * M_RAD2DEG );

				if ( strafed )
					out.Yaw = yaw;
			}
			break;

		case StrafeDir::POINT:
		{
			double point[] = { frame.GetX(), frame.GetY() };
			auto yaw = static_cast<float>( PointStrafe( frame, wishspeed, usedButton, vel_yaw, frame.GetType(), point, strafed ) * M_RAD2DEG );

			if ( strafed )
				out.Yaw = yaw;
		}
		break;

		default:
			strafed = false;
			break;
		}

		if ( strafed )
		{
			out.Forward = ( usedButton == Button::FORWARD || usedButton == Button::FORWARD_LEFT || usedButton == Button::FORWARD_RIGHT );
			out.Back = ( usedButton == Button::BACK || usedButton == Button::BACK_LEFT || usedButton == Button::BACK_RIGHT );
			out.Right = ( usedButton == Button::RIGHT || usedButton == Button::FORWARD_RIGHT || usedButton == Button::BACK_RIGHT );
			out.Left = ( usedButton == Button::LEFT || usedButton == Button::FORWARD_LEFT || usedButton == Button::BACK_LEFT );
			out.Processed = true;

			MapSpeeds( out, frame.player.ClientMaxspeed );
		}
	}

	bool PredictJump( Frame &frame )
	{
		if ( frame.AutoJump && frame.player.PosType == PositionType::GROUND )
		{
			frame.player.PosType = PositionType::AIR;
			return true;
		}

		return false;
	}

	bool PredictDucktap( Frame &frame, int iLowFrictionFPS )
	{
		if ( !frame.AutoDucktap )
			return false;

		if ( frame.player.PosType == Strafe::PositionType::GROUND && !frame.player.InDuck )
		{
			if ( iLowFrictionFPS != 0 )
				frame.vars.Frametime = 1.f / (float)iLowFrictionFPS;

			//if ( prevframe.strafe_frame.player.PosType == Strafe::PositionType::AIR )
				frame.player.Duck = true;

			return true;
		}

		return false;
	}

	void Friction( Frame &frame )
	{
		frame.player.SpeedBeforeFriction = Length<float, 2>( frame.player.Velocity );

		if ( frame.player.PosType != PositionType::GROUND )
			return;

		// Doing all this in floats, mismatch is too real otherwise.
		auto speed = static_cast<float>( std::sqrt( static_cast<double>( frame.player.Velocity[ 0 ] * frame.player.Velocity[ 0 ] +
										 frame.player.Velocity[ 1 ] * frame.player.Velocity[ 1 ] +
										 frame.player.Velocity[ 2 ] * frame.player.Velocity[ 2 ] ) ) );
		if ( speed < 0.1 )
			return;

		auto friction = float{ frame.vars.Friction * frame.player.EntFriction };
		auto control = ( speed < frame.vars.Stopspeed ) ? frame.vars.Stopspeed : speed;
		auto drop = control * friction * frame.vars.Frametime;
		auto newspeed = std::max( speed - drop, 0.f );

		VecScale<float, 3>( frame.player.Velocity, newspeed / speed, frame.player.Velocity );
	}
}

/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#ifndef __GAME_VEHICLEBASE_H__
#define __GAME_VEHICLEBASE_H__

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
//#include "VehicleBase.h"

/*
===============================================================================

  idAFEntity_Vehicle

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Base, idAFEntity_Vehicle )
END_CLASS

/*
================
idAFEntity_Vehicle::idAFEntity_Vehicle
================
*/
idAFEntity_Vehicle::idAFEntity_Vehicle( void )
{
	player = NULL;
	eyesJoint = INVALID_JOINT;
	steeringWheelJoint = INVALID_JOINT;
	wheelRadius = 0.0f;
	steerAngle = 0.0f;
	steerSpeed = 0.0f;
	dustSmoke = NULL;
}

/*
================
idAFEntity_Vehicle::Spawn
================
*/
void idAFEntity_Vehicle::Spawn( void )
{
	const char *eyesJointName = spawnArgs.GetString( "eyesJoint", "eyes" );
	const char *steeringWheelJointName = spawnArgs.GetString( "steeringWheelJoint", "steeringWheel" );

	LoadAF();

	SetCombatModel();

	SetPhysics( af.GetPhysics() );

	fl.takedamage = true;

	if ( !eyesJointName[ 0 ] )
	{
		gameLocal.Error( "idAFEntity_Vehicle '%s' no eyes joint specified", name.c_str() );
	}
	eyesJoint = animator.GetJointHandle( eyesJointName );
	if ( !steeringWheelJointName[ 0 ] )
	{
		gameLocal.Error( "idAFEntity_Vehicle '%s' no steering wheel joint specified", name.c_str() );
	}
	steeringWheelJoint = animator.GetJointHandle( steeringWheelJointName );

	spawnArgs.GetFloat( "wheelRadius", "20", wheelRadius );
	spawnArgs.GetFloat( "steerSpeed", "5", steerSpeed );

	player = NULL;
	steerAngle = 0.0f;

	const char *smokeName = spawnArgs.GetString( "smoke_vehicle_dust", "muzzlesmoke" );
	if ( *smokeName != '\0' )
	{
		dustSmoke = static_cast<const idDeclParticle *>(declManager->FindType( DECL_PARTICLE, smokeName ));
	}
}

/*
================
idAFEntity_Vehicle::Use
================
*/
void idAFEntity_Vehicle::Use( idPlayer *other )
{
	idVec3 origin;
	idMat3 axis;

	if ( player )
	{
		if ( player == other )
		{
			other->Unbind();
			player = NULL;

			af.GetPhysics()->SetComeToRest( true );
		}
	}
	else
	{
		player = other;
		animator.GetJointTransform( eyesJoint, gameLocal.time, origin, axis );
		origin = renderEntity.origin + origin * renderEntity.axis;
		player->GetPhysics()->SetOrigin( origin );
		player->BindToBody( this, 0, true );

		af.GetPhysics()->SetComeToRest( false );
		af.GetPhysics()->Activate();
	}
}

/*
================
idAFEntity_Vehicle::GetSteerAngle
================
*/
float idAFEntity_Vehicle::GetSteerAngle( void )
{
	float idealSteerAngle, angleDelta;

	idealSteerAngle = player->usercmd.rightmove * (30.0f / 128.0f);
	angleDelta = idealSteerAngle - steerAngle;

	if ( angleDelta > steerSpeed )
	{
		steerAngle += steerSpeed;
	}
	else if ( angleDelta < -steerSpeed )
	{
		steerAngle -= steerSpeed;
	}
	else
	{
		steerAngle = idealSteerAngle;
	}

	return steerAngle;
}


/*
===============================================================================

  idAFEntity_VehicleSimple

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleSimple )
END_CLASS

/*
================
idAFEntity_VehicleSimple::idAFEntity_VehicleSimple
================
*/
idAFEntity_VehicleSimple::idAFEntity_VehicleSimple( void )
{
	int i;
	for ( i = 0; i < 4; i++ )
	{
		suspension[ i ] = NULL;
	}
}

/*
================
idAFEntity_VehicleSimple::~idAFEntity_VehicleSimple
================
*/
idAFEntity_VehicleSimple::~idAFEntity_VehicleSimple( void )
{
	delete wheelModel;
	wheelModel = NULL;
}

/*
================
idAFEntity_VehicleSimple::Spawn
================
*/
void idAFEntity_VehicleSimple::Spawn( void )
{
	static const char *wheelJointKeys[] = {
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static idVec3 wheelPoly[ 4 ] = { idVec3( 2, 2, 0 ), idVec3( 2, -2, 0 ), idVec3( -2, -2, 0 ), idVec3( -2, 2, 0 ) };

	int i;
	idVec3 origin;
	idMat3 axis;
	idTraceModel trm;

	trm.SetupPolygon( wheelPoly, 4 );
	trm.Translate( idVec3( 0, 0, -wheelRadius ) );
	wheelModel = new idClipModel( trm );

	for ( i = 0; i < 4; i++ )
	{
		const char *wheelJointName = spawnArgs.GetString( wheelJointKeys[ i ], "" );
		if ( !wheelJointName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSimple '%s' no '%s' specified", name.c_str(), wheelJointKeys[ i ] );
		}
		wheelJoints[ i ] = animator.GetJointHandle( wheelJointName );
		if ( wheelJoints[ i ] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleSimple '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}

		GetAnimator()->GetJointTransform( wheelJoints[ i ], 0, origin, axis );
		origin = renderEntity.origin + origin * renderEntity.axis;

		suspension[ i ] = new idAFConstraint_Suspension();
		suspension[ i ]->Setup( va( "suspension%d", i ), af.GetPhysics()->GetBody( 0 ), origin, af.GetPhysics()->GetAxis( 0 ), wheelModel );
		suspension[ i ]->SetSuspension( g_vehicleSuspensionUp.GetFloat(),
										g_vehicleSuspensionDown.GetFloat(),
										g_vehicleSuspensionKCompress.GetFloat(),
										g_vehicleSuspensionDamping.GetFloat(),
										g_vehicleTireFriction.GetFloat() );

		af.GetPhysics()->AddConstraint( suspension[ i ] );
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleSimple::Think
================
*/
void idAFEntity_VehicleSimple::Think( void )
{
	int i;
	float force = 0.0f, velocity = 0.0f, steerAngle = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation wheelRotation, steerRotation;

	if ( thinkFlags & TH_THINK )
	{

		if ( player )
		{
// capture the input from a player
			velocity = g_vehicleVelocity.GetFloat();
			if ( player->usercmd.forwardmove < 0 )
			{
				velocity = -velocity;
			}
			force = idMath::Fabs( player->usercmd.forwardmove * g_vehicleForce.GetFloat() ) * (1.0f / 128.0f);
			steerAngle = GetSteerAngle();
		}

		// update the wheel motor force and steering
		for ( i = 0; i < 2; i++ )
		{

// front wheel drive
			if ( velocity != 0.0f )
			{
				suspension[ i ]->EnableMotor( true );
			}
			else
			{
				suspension[ i ]->EnableMotor( false );
			}
			suspension[ i ]->SetMotorVelocity( velocity );
			suspension[ i ]->SetMotorForce( force );

			// update the wheel steering
			suspension[ i ]->SetSteerAngle( steerAngle );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if ( steerAngle < 0.0f )
		{
			suspension[ 0 ]->SetMotorVelocity( velocity * 0.5f );
		}
		else if ( steerAngle > 0.0f )
		{
			suspension[ 1 ]->SetMotorVelocity( velocity * 0.5f );
		}

		// update suspension with latest cvar settings
		for ( i = 0; i < 4; i++ )
		{
			suspension[ i ]->SetSuspension( g_vehicleSuspensionUp.GetFloat(),
											g_vehicleSuspensionDown.GetFloat(),
											g_vehicleSuspensionKCompress.GetFloat(),
											g_vehicleSuspensionDamping.GetFloat(),
											g_vehicleTireFriction.GetFloat() );
		}

		// run the physics
		RunPhysics();

		// move and rotate the wheels visually
		for ( i = 0; i < 4; i++ )
		{
			idAFBody *body = af.GetPhysics()->GetBody( 0 );

			origin = suspension[ i ]->GetWheelOrigin();
			velocity = body->GetPointVelocity( origin ) * body->GetWorldAxis()[ 0 ];
			wheelAngles[ i ] += velocity * MS2SEC( gameLocal.msec ) / wheelRadius;

			// additional rotation about the wheel axis
			wheelRotation.SetAngle( RAD2DEG( wheelAngles[ i ] ) );
			wheelRotation.SetVec( 0, -1, 0 );

			if ( i < 2 )
			{
  // rotate the wheel for steering
				steerRotation.SetAngle( steerAngle );
				steerRotation.SetVec( 0, 0, 1 );
				// set wheel rotation
				animator.SetJointAxis( wheelJoints[ i ], JOINTMOD_WORLD, wheelRotation.ToMat3() * steerRotation.ToMat3() );
			}
			else
			{
					 // set wheel rotation
				animator.SetJointAxis( wheelJoints[ i ], JOINTMOD_WORLD, wheelRotation.ToMat3() );
			}

			// set wheel position for suspension
			origin = (origin - renderEntity.origin) * renderEntity.axis.Transpose();
			GetAnimator()->SetJointPos( wheelJoints[ i ], JOINTMOD_WORLD_OVERRIDE, origin );
		}
/*
		// spawn dust particle effects
		if ( force != 0.0f && !( gameLocal.framenum & 7 ) ) {
			int numContacts;
			idAFConstraint_Contact *contacts[2];
			for ( i = 0; i < 4; i++ ) {
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[i]->GetClipModel()->GetId(), contacts, 2 );
				for ( int j = 0; j < numContacts; j++ ) {
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[j]->GetContact().point, contacts[j]->GetContact().normal.ToMat3() );
				}
			}
		}
*/
	}

	UpdateAnimation();
	if ( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}
}


/*
===============================================================================

  idAFEntity_VehicleFourWheels

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleFourWheels )
END_CLASS


/*
================
idAFEntity_VehicleFourWheels::idAFEntity_VehicleFourWheels
================
*/
idAFEntity_VehicleFourWheels::idAFEntity_VehicleFourWheels( void )
{
	int i;

	for ( i = 0; i < 4; i++ )
	{
		wheels[ i ] = NULL;
		wheelJoints[ i ] = INVALID_JOINT;
		wheelAngles[ i ] = 0.0f;
	}
	steering[ 0 ] = NULL;
	steering[ 1 ] = NULL;
}

/*
================
idAFEntity_VehicleFourWheels::Spawn
================
*/
void idAFEntity_VehicleFourWheels::Spawn( void )
{
	int i;
	static const char *wheelBodyKeys[] = {
		"wheelBodyFrontLeft",
		"wheelBodyFrontRight",
		"wheelBodyRearLeft",
		"wheelBodyRearRight"
	};
	static const char *wheelJointKeys[] = {
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static const char *steeringHingeKeys[] = {
		"steeringHingeFrontLeft",
		"steeringHingeFrontRight",
	};

	const char *wheelBodyName, *wheelJointName, *steeringHingeName;

	for ( i = 0; i < 4; i++ )
	{
		wheelBodyName = spawnArgs.GetString( wheelBodyKeys[ i ], "" );
		if ( !wheelBodyName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), wheelBodyKeys[ i ] );
		}
		wheels[ i ] = af.GetPhysics()->GetBody( wheelBodyName );
		if ( !wheels[ i ] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' can't find wheel body '%s'", name.c_str(), wheelBodyName );
		}
		wheelJointName = spawnArgs.GetString( wheelJointKeys[ i ], "" );
		if ( !wheelJointName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), wheelJointKeys[ i ] );
		}
		wheelJoints[ i ] = animator.GetJointHandle( wheelJointName );
		if ( wheelJoints[ i ] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}
	}

	for ( i = 0; i < 2; i++ )
	{
		steeringHingeName = spawnArgs.GetString( steeringHingeKeys[ i ], "" );
		if ( !steeringHingeName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s' no '%s' specified", name.c_str(), steeringHingeKeys[ i ] );
		}
		steering[ i ] = static_cast<idAFConstraint_Hinge *>(af.GetPhysics()->GetConstraint( steeringHingeName ));
		if ( !steering[ i ] )
		{
			gameLocal.Error( "idAFEntity_VehicleFourWheels '%s': can't find steering hinge '%s'", name.c_str(), steeringHingeName );
		}
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleFourWheels::Think
================
*/
void idAFEntity_VehicleFourWheels::Think( void )
{
	int i;
	float force = 0.0f, velocity = 0.0f, steerAngle = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation rotation;

	if ( thinkFlags & TH_THINK )
	{

		if ( player )
		{
// capture the input from a player
			velocity = g_vehicleVelocity.GetFloat();
			if ( player->usercmd.forwardmove < 0 )
			{
				velocity = -velocity;
			}
			force = idMath::Fabs( player->usercmd.forwardmove * g_vehicleForce.GetFloat() ) * (1.0f / 128.0f);
			steerAngle = GetSteerAngle();
		}

		// update the wheel motor force
		for ( i = 0; i < 2; i++ )
		{
			wheels[ 2 + i ]->SetContactMotorVelocity( velocity );
			wheels[ 2 + i ]->SetContactMotorForce( force );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if ( steerAngle < 0.0f )
		{
			wheels[ 2 ]->SetContactMotorVelocity( velocity * 0.5f );
		}
		else if ( steerAngle > 0.0f )
		{
			wheels[ 3 ]->SetContactMotorVelocity( velocity * 0.5f );
		}

		// update the wheel steering
		steering[ 0 ]->SetSteerAngle( steerAngle );
		steering[ 1 ]->SetSteerAngle( steerAngle );
		for ( i = 0; i < 2; i++ )
		{
			steering[ i ]->SetSteerSpeed( 3.0f );
		}

		// update the steering wheel
		animator.GetJointTransform( steeringWheelJoint, gameLocal.time, origin, axis );
		rotation.SetVec( axis[ 2 ] );
		rotation.SetAngle( -steerAngle );
		animator.SetJointAxis( steeringWheelJoint, JOINTMOD_WORLD, rotation.ToMat3() );

		// run the physics
		RunPhysics();

		// rotate the wheels visually
		for ( i = 0; i < 4; i++ )
		{
			if ( force == 0.0f )
			{
				velocity = wheels[ i ]->GetLinearVelocity() * wheels[ i ]->GetWorldAxis()[ 0 ];
			}
			wheelAngles[ i ] += velocity * MS2SEC( gameLocal.msec ) / wheelRadius;
			// give the wheel joint an additional rotation about the wheel axis
			rotation.SetAngle( RAD2DEG( wheelAngles[ i ] ) );
			axis = af.GetPhysics()->GetAxis( 0 );
			rotation.SetVec( (wheels[ i ]->GetWorldAxis() * axis.Transpose())[ 2 ] );
			animator.SetJointAxis( wheelJoints[ i ], JOINTMOD_WORLD, rotation.ToMat3() );
		}

		// spawn dust particle effects
		if ( force != 0.0f && !(gameLocal.framenum & 7) )
		{
			int numContacts;
			idAFConstraint_Contact *contacts[ 2 ];
			for ( i = 0; i < 4; i++ )
			{
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[ i ]->GetClipModel()->GetId(), contacts, 2 );
				for ( int j = 0; j < numContacts; j++ )
				{
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[ j ]->GetContact().point, contacts[ j ]->GetContact().normal.ToMat3() );
				}
			}
		}
	}

	UpdateAnimation();
	if ( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}
}


/*
===============================================================================

  idAFEntity_VehicleSixWheels

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Vehicle, idAFEntity_VehicleSixWheels )
END_CLASS

	/*
================
idAFEntity_VehicleSixWheels::idAFEntity_VehicleSixWheels
================
*/
idAFEntity_VehicleSixWheels::idAFEntity_VehicleSixWheels( void )
{
	int i;

	for ( i = 0; i < 6; i++ )
	{
		wheels[ i ] = NULL;
		wheelJoints[ i ] = INVALID_JOINT;
		wheelAngles[ i ] = 0.0f;
	}
	steering[ 0 ] = NULL;
	steering[ 1 ] = NULL;
	steering[ 2 ] = NULL;
	steering[ 3 ] = NULL;
}

/*
================
idAFEntity_VehicleSixWheels::Spawn
================
*/
void idAFEntity_VehicleSixWheels::Spawn( void )
{
	int i;
	static const char *wheelBodyKeys[] = {
		"wheelBodyFrontLeft",
		"wheelBodyFrontRight",
		"wheelBodyMiddleLeft",
		"wheelBodyMiddleRight",
		"wheelBodyRearLeft",
		"wheelBodyRearRight"
	};
	static const char *wheelJointKeys[] = {
		"wheelJointFrontLeft",
		"wheelJointFrontRight",
		"wheelJointMiddleLeft",
		"wheelJointMiddleRight",
		"wheelJointRearLeft",
		"wheelJointRearRight"
	};
	static const char *steeringHingeKeys[] = {
		"steeringHingeFrontLeft",
		"steeringHingeFrontRight",
		"steeringHingeRearLeft",
		"steeringHingeRearRight"
	};

	const char *wheelBodyName, *wheelJointName, *steeringHingeName;

	for ( i = 0; i < 6; i++ )
	{
		wheelBodyName = spawnArgs.GetString( wheelBodyKeys[ i ], "" );
		if ( !wheelBodyName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), wheelBodyKeys[ i ] );
		}
		wheels[ i ] = af.GetPhysics()->GetBody( wheelBodyName );
		if ( !wheels[ i ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' can't find wheel body '%s'", name.c_str(), wheelBodyName );
		}
		wheelJointName = spawnArgs.GetString( wheelJointKeys[ i ], "" );
		if ( !wheelJointName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), wheelJointKeys[ i ] );
		}
		wheelJoints[ i ] = animator.GetJointHandle( wheelJointName );
		if ( wheelJoints[ i ] == INVALID_JOINT )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' can't find wheel joint '%s'", name.c_str(), wheelJointName );
		}
	}

	for ( i = 0; i < 4; i++ )
	{
		steeringHingeName = spawnArgs.GetString( steeringHingeKeys[ i ], "" );
		if ( !steeringHingeName[ 0 ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s' no '%s' specified", name.c_str(), steeringHingeKeys[ i ] );
		}
		steering[ i ] = static_cast<idAFConstraint_Hinge *>(af.GetPhysics()->GetConstraint( steeringHingeName ));
		if ( !steering[ i ] )
		{
			gameLocal.Error( "idAFEntity_VehicleSixWheels '%s': can't find steering hinge '%s'", name.c_str(), steeringHingeName );
		}
	}

	memset( wheelAngles, 0, sizeof( wheelAngles ) );
	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_VehicleSixWheels::Think
================
*/
void idAFEntity_VehicleSixWheels::Think( void )
{
	int i;
	float force = 0.0f, velocity = 0.0f, steerAngle = 0.0f;
	idVec3 origin;
	idMat3 axis;
	idRotation rotation;

	if ( thinkFlags & TH_THINK )
	{

		if ( player )
		{
// capture the input from a player
			velocity = g_vehicleVelocity.GetFloat();
			if ( player->usercmd.forwardmove < 0 )
			{
				velocity = -velocity;
			}
			force = idMath::Fabs( player->usercmd.forwardmove * g_vehicleForce.GetFloat() ) * (1.0f / 128.0f);
			steerAngle = GetSteerAngle();
		}

		// update the wheel motor force
		for ( i = 0; i < 6; i++ )
		{
			wheels[ i ]->SetContactMotorVelocity( velocity );
			wheels[ i ]->SetContactMotorForce( force );
		}

		// adjust wheel velocity for better steering because there are no differentials between the wheels
		if ( steerAngle < 0.0f )
		{
			for ( i = 0; i < 3; i++ )
			{
				wheels[ (i << 1) ]->SetContactMotorVelocity( velocity * 0.5f );
			}
		}
		else if ( steerAngle > 0.0f )
		{
			for ( i = 0; i < 3; i++ )
			{
				wheels[ 1 + (i << 1) ]->SetContactMotorVelocity( velocity * 0.5f );
			}
		}

		// update the wheel steering
		steering[ 0 ]->SetSteerAngle( steerAngle );
		steering[ 1 ]->SetSteerAngle( steerAngle );
		steering[ 2 ]->SetSteerAngle( -steerAngle );
		steering[ 3 ]->SetSteerAngle( -steerAngle );
		for ( i = 0; i < 4; i++ )
		{
			steering[ i ]->SetSteerSpeed( 3.0f );
		}

		// update the steering wheel
		animator.GetJointTransform( steeringWheelJoint, gameLocal.time, origin, axis );
		rotation.SetVec( axis[ 2 ] );
		rotation.SetAngle( -steerAngle );
		animator.SetJointAxis( steeringWheelJoint, JOINTMOD_WORLD, rotation.ToMat3() );

		// run the physics
		RunPhysics();

		// rotate the wheels visually
		for ( i = 0; i < 6; i++ )
		{
			if ( force == 0.0f )
			{
				velocity = wheels[ i ]->GetLinearVelocity() * wheels[ i ]->GetWorldAxis()[ 0 ];
			}
			wheelAngles[ i ] += velocity * MS2SEC( gameLocal.msec ) / wheelRadius;
			// give the wheel joint an additional rotation about the wheel axis
			rotation.SetAngle( RAD2DEG( wheelAngles[ i ] ) );
			axis = af.GetPhysics()->GetAxis( 0 );
			rotation.SetVec( (wheels[ i ]->GetWorldAxis() * axis.Transpose())[ 2 ] );
			animator.SetJointAxis( wheelJoints[ i ], JOINTMOD_WORLD, rotation.ToMat3() );
		}

		// spawn dust particle effects
		if ( force != 0.0f && !(gameLocal.framenum & 7) )
		{
			int numContacts;
			idAFConstraint_Contact *contacts[ 2 ];
			for ( i = 0; i < 6; i++ )
			{
				numContacts = af.GetPhysics()->GetBodyContactConstraints( wheels[ i ]->GetClipModel()->GetId(), contacts, 2 );
				for ( int j = 0; j < numContacts; j++ )
				{
					gameLocal.smokeParticles->EmitSmoke( dustSmoke, gameLocal.time, gameLocal.random.RandomFloat(), contacts[ j ]->GetContact().point, contacts[ j ]->GetContact().normal.ToMat3() );
				}
			}
		}
	}

	UpdateAnimation();
	if ( thinkFlags & TH_UPDATEVISUALS )
	{
		Present();
		LinkCombat();
	}
}


/*
===============================================================================

  idAFEntity_SteamPipe

===============================================================================
*/

CLASS_DECLARATION( idAFEntity_Base, idAFEntity_SteamPipe )
END_CLASS


/*
================
idAFEntity_SteamPipe::idAFEntity_SteamPipe
================
*/
idAFEntity_SteamPipe::idAFEntity_SteamPipe( void )
{
	steamBody = 0;
	steamForce = 0.0f;
	steamUpForce = 0.0f;
	steamModelDefHandle = -1;
	memset( &steamRenderEntity, 0, sizeof( steamRenderEntity ) );
}

/*
================
idAFEntity_SteamPipe::~idAFEntity_SteamPipe
================
*/
idAFEntity_SteamPipe::~idAFEntity_SteamPipe( void )
{
	if ( steamModelDefHandle >= 0 )
	{
		gameRenderWorld->FreeEntityDef( steamModelDefHandle );
	}
}

/*
================
idAFEntity_SteamPipe::Save
================
*/
void idAFEntity_SteamPipe::Save( idSaveGame *savefile ) const
{}

/*
================
idAFEntity_SteamPipe::Restore
================
*/
void idAFEntity_SteamPipe::Restore( idRestoreGame *savefile )
{
	Spawn();
}

/*
================
idAFEntity_SteamPipe::Spawn
================
*/
void idAFEntity_SteamPipe::Spawn( void )
{
	idVec3 steamDir;
	const char *steamBodyName;

	LoadAF();

	SetCombatModel();

	SetPhysics( af.GetPhysics() );

	fl.takedamage = true;

	steamBodyName = spawnArgs.GetString( "steamBody", "" );
	steamForce = spawnArgs.GetFloat( "steamForce", "2000" );
	steamUpForce = spawnArgs.GetFloat( "steamUpForce", "10" );
	steamDir = af.GetPhysics()->GetAxis( steamBody )[ 2 ];
	steamBody = af.GetPhysics()->GetBodyId( steamBodyName );
	force.SetPosition( af.GetPhysics(), steamBody, af.GetPhysics()->GetOrigin( steamBody ) );
	force.SetForce( steamDir * -steamForce );

	InitSteamRenderEntity();

	BecomeActive( TH_THINK );
}

/*
================
idAFEntity_SteamPipe::InitSteamRenderEntity
================
*/
void idAFEntity_SteamPipe::InitSteamRenderEntity( void )
{
	const char	*temp;
	const idDeclModelDef *modelDef;

	memset( &steamRenderEntity, 0, sizeof( steamRenderEntity ) );
	steamRenderEntity.shaderParms[ SHADERPARM_RED ] = 1.0f;
	steamRenderEntity.shaderParms[ SHADERPARM_GREEN ] = 1.0f;
	steamRenderEntity.shaderParms[ SHADERPARM_BLUE ] = 1.0f;
	modelDef = NULL;
	temp = spawnArgs.GetString( "model_steam" );
	if ( *temp != '\0' )
	{
		if ( !strstr( temp, "." ) )
		{
			modelDef = static_cast<const idDeclModelDef *>(declManager->FindType( DECL_MODELDEF, temp, false ));
			if ( modelDef )
			{
				steamRenderEntity.hModel = modelDef->ModelHandle();
			}
		}

		if ( !steamRenderEntity.hModel )
		{
			steamRenderEntity.hModel = renderModelManager->FindModel( temp );
		}

		if ( steamRenderEntity.hModel )
		{
			steamRenderEntity.bounds = steamRenderEntity.hModel->Bounds( &steamRenderEntity );
		}
		else
		{
			steamRenderEntity.bounds.Zero();
		}
		steamRenderEntity.origin = af.GetPhysics()->GetOrigin( steamBody );
		steamRenderEntity.axis = af.GetPhysics()->GetAxis( steamBody );
		steamModelDefHandle = gameRenderWorld->AddEntityDef( &steamRenderEntity );
	}
}

/*
================
idAFEntity_SteamPipe::Think
================
*/
void idAFEntity_SteamPipe::Think( void )
{
	idVec3 steamDir;

	if ( thinkFlags & TH_THINK )
	{
		steamDir.x = gameLocal.random.CRandomFloat() * steamForce;
		steamDir.y = gameLocal.random.CRandomFloat() * steamForce;
		steamDir.z = steamUpForce;
		force.SetForce( steamDir );
		force.Evaluate( gameLocal.time );
		//gameRenderWorld->DebugArrow( colorWhite, af.GetPhysics()->GetOrigin( steamBody ), af.GetPhysics()->GetOrigin( steamBody ) - 10.0f * steamDir, 4 );
	}

	if ( steamModelDefHandle >= 0 )
	{
		steamRenderEntity.origin = af.GetPhysics()->GetOrigin( steamBody );
		steamRenderEntity.axis = af.GetPhysics()->GetAxis( steamBody );
		gameRenderWorld->UpdateEntityDef( steamModelDefHandle, &steamRenderEntity );
	}

	idAFEntity_Base::Think();
}


/*
===============================================================================

  idAFEntity_ClawFourFingers

===============================================================================
*/

const idEventDef EV_SetFingerAngle( "setFingerAngle", "f" );
const idEventDef EV_StopFingers( "stopFingers" );

CLASS_DECLARATION( idAFEntity_Base, idAFEntity_ClawFourFingers )
EVENT( EV_SetFingerAngle, idAFEntity_ClawFourFingers::Event_SetFingerAngle )
EVENT( EV_StopFingers, idAFEntity_ClawFourFingers::Event_StopFingers )
END_CLASS

static const char *clawConstraintNames[] = {
	"claw1", "claw2", "claw3", "claw4"
};

/*
================
idAFEntity_ClawFourFingers::idAFEntity_ClawFourFingers
================
*/
idAFEntity_ClawFourFingers::idAFEntity_ClawFourFingers( void )
{
	fingers[ 0 ] = NULL;
	fingers[ 1 ] = NULL;
	fingers[ 2 ] = NULL;
	fingers[ 3 ] = NULL;
}

/*
================
idAFEntity_ClawFourFingers::Save
================
*/
void idAFEntity_ClawFourFingers::Save( idSaveGame *savefile ) const
{
	int i;

	for ( i = 0; i < 4; i++ )
	{
		fingers[ i ]->Save( savefile );
	}
}

/*
================
idAFEntity_ClawFourFingers::Restore
================
*/
void idAFEntity_ClawFourFingers::Restore( idRestoreGame *savefile )
{
	int i;

	for ( i = 0; i < 4; i++ )
	{
		fingers[ i ] = static_cast<idAFConstraint_Hinge *>(af.GetPhysics()->GetConstraint( clawConstraintNames[ i ] ));
		fingers[ i ]->Restore( savefile );
	}

	SetCombatModel();
	LinkCombat();
}

/*
================
idAFEntity_ClawFourFingers::Spawn
================
*/
void idAFEntity_ClawFourFingers::Spawn( void )
{
	int i;

	LoadAF();

	SetCombatModel();

	af.GetPhysics()->LockWorldConstraints( true );
	af.GetPhysics()->SetForcePushable( true );
	SetPhysics( af.GetPhysics() );

	fl.takedamage = true;

	for ( i = 0; i < 4; i++ )
	{
		fingers[ i ] = static_cast<idAFConstraint_Hinge *>(af.GetPhysics()->GetConstraint( clawConstraintNames[ i ] ));
		if ( !fingers[ i ] )
		{
			gameLocal.Error( "idClaw_FourFingers '%s': can't find claw constraint '%s'", name.c_str(), clawConstraintNames[ i ] );
		}
	}
}

/*
================
idAFEntity_ClawFourFingers::Event_SetFingerAngle
================
*/
void idAFEntity_ClawFourFingers::Event_SetFingerAngle( float angle )
{
	int i;

	for ( i = 0; i < 4; i++ )
	{
		fingers[ i ]->SetSteerAngle( angle );
		fingers[ i ]->SetSteerSpeed( 0.5f );
	}
	af.GetPhysics()->Activate();
}

/*
================
idAFEntity_ClawFourFingers::Event_StopFingers
================
*/
void idAFEntity_ClawFourFingers::Event_StopFingers( void )
{
	int i;

	for ( i = 0; i < 4; i++ )
	{
		fingers[ i ]->SetSteerAngle( fingers[ i ]->GetAngle() );
	}
}

#endif /* !__GAME_VEHICLEBASE_H__ */
/*
=========================================================

	Odljev Source Code
	2019, Admer456

	This code is licenced under GPLv3.

=========================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
//#include "VehicleBase.h"
#include "VehicleCar.h"

/*
========================================================

	admVehicleEntity_Car

========================================================
*/

CLASS_DECLARATION( idAFEntity_VehicleFourWheels, admVehicleEntity_Car )
END_CLASS

admVehicleEntity_Car::admVehicleEntity_Car( void )
{
	player = NULL;
	eyesJoint = INVALID_JOINT;
	steeringWheelJoint = INVALID_JOINT;
	wheelRadius = 0.0f;
	steerAngle = 0.0f;
	steerSpeed = 0.0f;
	dustSmoke = NULL;
}

// TODO: IMPLEMENT -456
void admVehicleEntity_Car::Spawn()
{

}

void admVehicleEntity_Car::Use( idPlayer *player )
{

}

void admVehicleEntity_Car::Think()
{

}
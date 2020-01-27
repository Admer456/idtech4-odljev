#pragma once

/*
===============================================================================

idAFEntity_Vehicle

===============================================================================
*/

class idAFEntity_Vehicle : public idAFEntity_Base
{
public:
	CLASS_PROTOTYPE( idAFEntity_Vehicle );

	idAFEntity_Vehicle( void );

	void					Spawn( void );
	void					Use( idPlayer *player );

protected:
	idPlayer *				player;
	jointHandle_t			eyesJoint;
	jointHandle_t			steeringWheelJoint;
	float					wheelRadius;
	float					steerAngle;
	float					steerSpeed;
	const idDeclParticle *	dustSmoke;

	float					GetSteerAngle( void );
};

/*
	.def keyvalues required for this vehicle:
	eyesJoint (default "eyes")
	steeringWheelJoint (default "steeringwheel")

	wheelRadius (default 20)
	steerSpeed (default 5)

	OPTIONAL* smoke_vehicle_dust (default "muzzlesmoke")
*/

/*
===============================================================================

idAFEntity_VehicleSimple

===============================================================================
*/

class idAFEntity_VehicleSimple : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSimple );

	idAFEntity_VehicleSimple( void );
	~idAFEntity_VehicleSimple( void );

	void					Spawn( void );
	virtual void			Think( void );

protected:
	idClipModel *			wheelModel;
	idAFConstraint_Suspension *	suspension[ 4 ];
	jointHandle_t			wheelJoints[ 4 ];
	float					wheelAngles[ 4 ];
};

/*
	.def keyvalues required for this vehicle:
	eyesJoint (default "eyes")
	steeringWheelJoint (default "steeringwheel")

	wheelJointFrontLeft
	wheelJointFrontRight
	wheelJointBackLeft
	wheelJointBackRight

	??? suspension0
	??? suspension1
	??? suspension2
	??? suspension3

	wheelRadius (default 20)
	steerSpeed (default 5)

	OPTIONAL* smoke_vehicle_dust (default "muzzlesmoke")
*/

/*
===============================================================================

idAFEntity_VehicleFourWheels

===============================================================================
*/

class idAFEntity_VehicleFourWheels : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleFourWheels );

	idAFEntity_VehicleFourWheels( void );

	virtual void			Spawn( void );
	virtual void			Think( void );

protected:
	idAFBody *				wheels[ 4 ];
	idAFConstraint_Hinge *	steering[ 2 ];
	jointHandle_t			wheelJoints[ 4 ];
	float					wheelAngles[ 4 ];
};

/*
	.def keyvalues required for this vehicle:
	eyesJoint (default "eyes")
	steeringWheelJoint (default "steeringwheel")

	wheelJointFrontLeft
	wheelJointFrontRight
	wheelJointBackLeft
	wheelJointBackRight

	wheelBodyFrontLeft
	wheelBodyFrontRight
	wheelBodyRearLeft
	wheelBodyRearRight

	steeringHingeFrontLeft
	steeringHingeFrontRight

	??? suspension0
	??? suspension1
	??? suspension2
	??? suspension3

	wheelRadius (default 20)
	steerSpeed (default 5)

	OPTIONAL* smoke_vehicle_dust (default "muzzlesmoke")
*/

/*
===============================================================================

idAFEntity_VehicleSixWheels

===============================================================================
*/

class idAFEntity_VehicleSixWheels : public idAFEntity_Vehicle
{
public:
	CLASS_PROTOTYPE( idAFEntity_VehicleSixWheels );

	idAFEntity_VehicleSixWheels( void );

	void					Spawn( void );
	virtual void			Think( void );

private:
	idAFBody *				wheels[ 6 ];
	idAFConstraint_Hinge *	steering[ 4 ];
	jointHandle_t			wheelJoints[ 6 ];
	float					wheelAngles[ 6 ];
};


/*
===============================================================================

idAFEntity_SteamPipe

===============================================================================
*/

class idAFEntity_SteamPipe : public idAFEntity_Base
{
public:
	CLASS_PROTOTYPE( idAFEntity_SteamPipe );

	idAFEntity_SteamPipe( void );
	~idAFEntity_SteamPipe( void );

	void					Spawn( void );
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

	virtual void			Think( void );

private:
	int						steamBody;
	float					steamForce;
	float					steamUpForce;
	idForce_Constant		force;
	renderEntity_t			steamRenderEntity;
	qhandle_t				steamModelDefHandle;

	void					InitSteamRenderEntity( void );
};


/*
===============================================================================

idAFEntity_ClawFourFingers

===============================================================================
*/

class idAFEntity_ClawFourFingers : public idAFEntity_Base
{
public:
	CLASS_PROTOTYPE( idAFEntity_ClawFourFingers );

	idAFEntity_ClawFourFingers( void );

	void					Spawn( void );
	void					Save( idSaveGame *savefile ) const;
	void					Restore( idRestoreGame *savefile );

private:
	idAFConstraint_Hinge *	fingers[ 4 ];

	void					Event_SetFingerAngle( float angle );
	void					Event_StopFingers( void );
};

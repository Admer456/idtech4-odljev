#pragma once

/*
========================================================

	admVehicleEntity_Car

========================================================
*/

#ifndef __GAME_VEHICLECAR_H__
#define __GAME_VEHICLECAR_H__

class admVehicleEntity_Car : public idAFEntity_VehicleFourWheels
{
public:
	CLASS_PROTOTYPE( admVehicleEntity_Car );

	admVehicleEntity_Car();

	virtual void					Spawn( void ) override;
	virtual void					Use( idPlayer *player );
	virtual void					Think( void ) override;

protected:
	float							engineHorsepower;
	bool							isHeldHandbrake;
	bool							isHeldClutch;
};

#endif /* !_GAME_VEHICLECAR_H */
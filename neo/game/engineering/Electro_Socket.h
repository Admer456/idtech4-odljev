/*
===============================================================================
	Electrical socket
	On multimeter - give measurement voltage ( handled on the weapon side )
	On screwdriver - nothing
	On cutter - short circuit
===============================================================================
*/

#pragma once

class admElectroSocket : public admElectroBase
{
public:
	CLASS_PROTOTYPE( admElectroSocket );

						admElectroSocket();
						~admElectroSocket();

	void				SpawnCustom( void );
	void				Think( void );

	virtual void		OnMultimeter(	idWeapon *weap );
	virtual void		OnScrewdriver(	idWeapon *weap );
	virtual void		OnCutter(		idWeapon *weap );

private:
	float				voltageEffectiveOriginal; // spawnArgs voltage
	float				voltageEffective; // spawnArgs voltage
	int					currentType; // 0 - DC, 1 - AC

	float				voltageCurrent; // current, actual voltage
	float				voltageMaximum; // AC voltage amplitude
	float				voltageNoise;	// voltage noise, if any

	float				cycle; // current angle; omega*time
	float				omega; // angular velocity
	float				frequency; // frequency of oscillating, if AC

	bool				pluggedIn; // is a plug plugged into this
};

#pragma once

/*
===============================================================================
	Base electrical entity
	Provides different functions for usage with:

	- multimeters
	- screwdrivers
	- cutters

	On multimeter - measures 220V
	On screwdriver - short, force the player to jump a bit
	On cutter - toggle between on and off
===============================================================================
*/

enum VoltageType
{
	VT_DC,
	VT_AC
};

enum ElectroState
{
	EL_OFF,
	EL_ON,
	EL_SHORTED
};

class admElectroBase : public idEntity
{
public:
	CLASS_PROTOTYPE( admElectroBase );

						admElectroBase();
						~admElectroBase();

	void				Spawn( void );
	virtual	void		SpawnCustom( void );

	virtual void		Think( void );

	virtual void		OnMultimeter(	idWeapon *weap );
	virtual void		OnScrewdriver(	idWeapon *weap );
	virtual void		OnCutter(		idWeapon *weap );

	virtual void		OnShorted( void );
	virtual void		TurnOff( void );
	virtual void		TurnOn( void );


protected:
	int electroState; // 0 = off; 1 = on, nominal; 2 = shorted;

	idList<idEntityPtr<idEntity>> on_targets;
	idList<idEntityPtr<idEntity>> off_targets;
	idList<idEntityPtr<idEntity>> short_targets;
};


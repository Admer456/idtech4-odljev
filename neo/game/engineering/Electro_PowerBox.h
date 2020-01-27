#pragma once

/*
===============================================================================
	Power box with circuit breakers
	
	Refers to a set of sockets, all set by the mapper.
	If a short circuit occurs or wires are cut, then
	sockets lose power.

	On multimeter - nothing
	On screwdriver - if wires are cut, short circuit
	On cutter - cut wires
===============================================================================
*/

enum PowerBoxState
{
	PowerBox_Normal,
	PowerBox_Stripped,
	PowerBox_Cut
};

class admElectroPowerBox : public admElectroBase
{
public:
	CLASS_PROTOTYPE( admElectroPowerBox );

	void		SpawnCustom( void );
	void		SpawnThink( void );
	void		Think( void );

	void		OnMultimeter(	idWeapon *weap );
	void		OnScrewdriver(	idWeapon *weap );
	void		OnCutter(		idWeapon *weap );

	void		OnShorted( void );
	void		TurnOff( void );
	void		TurnOn( void );

private:
	int			powerBoxState;

	idList<idEntityPtr<idEntity>> sockets;

	idStr		modelUninsulated;
	idStr		modelCut;
};

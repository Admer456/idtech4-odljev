#pragma once

/*
===============================================================================
	Electrical wire entity

	Connects a plug to a device, or anything.
	Can be cut. Conducts current.

	Insulated:
	On multimeter -> nothing
	On screwdriver -> nothing
	On cutter -> stop conducting

	Not insulated:
	On multimeter -> nothing
	On screwdriver -> potentially a short circuit
	On cutter -> stop conducting
	- Will electrocute things, possibly
	
===============================================================================
*/


class admElectroWire : public admElectroBase
{
public:
	CLASS_PROTOTYPE( admElectroWire );

	void		SpawnCustom( void );
	void		SpawnThink( void );
	void		Think( void );

	void		OnMultimeter( idWeapon *weap );
	void		OnScrewdriver( idWeapon *weap );
	void		OnCutter( idWeapon *weap );		

private:
	idAF		af;
	jointHandle_t wireStart;
	jointHandle_t wireEnd;
};
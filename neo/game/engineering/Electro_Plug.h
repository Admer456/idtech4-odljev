#pragma once

/*
===============================================================================
	Electric plug entity
	Plugs into sockets

	On multimeter - nothing
	On screwdriver - nothing
	On cutter - nothing
===============================================================================
*/

class admElectroPlug : public admElectroBase
{
public:
	CLASS_PROTOTYPE( admElectroPlug );

	void		SpawnCustom( void );
	void		Think( void );

	void		CheckForSockets( void );

	void		OnMultimeter(	idWeapon *weap ) {}
	void		OnScrewdriver(	idWeapon *weap ) {}
	void		OnCutter(		idWeapon *weap ) {}

	void		OnUse(			idPlayer *player );
	void		OnUnuse(		idPlayer *player );

private:
	idEntityPtr<idEntity> voltageSource; // the entity it's plugged into
	idEntityPtr<idPlayer> user; // the player that is dragging this plug
};
#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Electro_Base.h"
#include "Electro_Socket.h"

CLASS_DECLARATION( admElectroBase, admElectroSocket )
END_CLASS

admElectroSocket::admElectroSocket()
{
	pluggedIn = false;
	voltageEffectiveOriginal = 0;
	voltageEffective = 0;
	voltageCurrent = 0;
	voltageMaximum = 0;
	voltageNoise = 0;

	cycle = 0;
	omega = 0;
	frequency = 0;

	electroState = EL_ON;
}
admElectroSocket::~admElectroSocket()
{
	pluggedIn = false;
	voltageEffectiveOriginal = 0;
	voltageEffective = 0;
	voltageCurrent = 0;
	voltageMaximum = 0;
	voltageNoise = 0;

	cycle = 0;
	omega = 0;
	frequency = 0;

	electroState = EL_ON;
}

void admElectroSocket::SpawnCustom()
{
	voltageEffectiveOriginal	= spawnArgs.GetFloat( "voltage", "220" );
	voltageNoise				= spawnArgs.GetFloat( "voltageNoise", "2.5" );
	currentType					= spawnArgs.GetFloat( "type", "1" );
	voltageEffective			= voltageEffectiveOriginal;

	if ( currentType == VT_DC )
		voltageMaximum = voltageEffective;

	else if ( currentType == VT_AC )
		voltageMaximum = voltageEffective * idMath::SQRT_TWO;

	cycle		= 0;
	frequency	= spawnArgs.GetFloat( "frequency", "50" );
	omega		= idMath::TWO_PI * frequency;
}

void admElectroSocket::Think()
{
	idEntity::Think();

	if ( electroState == EL_OFF || electroState == EL_SHORTED )
	{
		voltageEffective = 0;
		voltageCurrent = 0;
	}

	else //if ( electroState == EL_ON )
	{
		if ( voltageEffective != voltageEffectiveOriginal )
		{
			voltageEffective = voltageEffectiveOriginal;
		}

		if ( currentType == VT_DC )
		{
			voltageCurrent = voltageCurrent * 0.5 + voltageEffective * 0.5;
		}

		else if ( currentType == VT_AC )
		{
			cycle += omega * MS2SEC( gameLocal.msec );

			if ( cycle > idMath::TWO_PI )
				cycle -= idMath::TWO_PI;

			voltageCurrent = voltageMaximum * sin( cycle );
		}

		if ( voltageNoise )
		{
			idRandom2 randomNoise;
			randomNoise.SetSeed( 2459789 );

			voltageCurrent += voltageNoise * randomNoise.RandomFloat();
		}
	}
}

void admElectroSocket::OnMultimeter( idWeapon *weap )
{
	weap->Measure( voltageEffective );
	common->Printf( va( "Measured voltage: %f\n", weap->MeasurementSize() ) );
}

void admElectroSocket::OnScrewdriver( idWeapon *weap )
{
	// nothing
}

void admElectroSocket::OnCutter( idWeapon *weap )
{
	float damage = 0.028 * idMath::Pow( voltageEffective, 1.5f );

	// TODO: implement insulation
	//if( !weap->GetOwner()->IsElectroInsulated() )
	weap->GetOwner()->Damage( NULL, NULL, idVec3(0,0,0), "damage_electro", damage, -1 );
	
	// ^ Shalakin zakon: Nula + faza = dzenaza

	OnShorted();

	gameLocal.DPrintf( "admElectroSocket voltage %f damage dealt %f\n", voltageEffective, damage );
}

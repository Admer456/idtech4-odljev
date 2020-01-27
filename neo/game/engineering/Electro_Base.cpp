#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Electro_Base.h"

CLASS_DECLARATION( idEntity, admElectroBase )
END_CLASS

admElectroBase::admElectroBase()
{
	on_targets.Clear();
	off_targets.Clear();
	short_targets.Clear();
	electroState = EL_ON;
}

admElectroBase::~admElectroBase()
{
	on_targets.Clear();
	off_targets.Clear();
	short_targets.Clear();
	electroState = EL_OFF;
}

void admElectroBase::Spawn()
{
	FindCustomTargets( "target_on", on_targets );
	FindCustomTargets( "target_off", off_targets );
	FindCustomTargets( "target_short", short_targets );

	GetPhysics()->SetContents( CONTENTS_SOLID );
	BecomeActive( TH_THINK );
	BecomeActive( TH_UPDATEVISUALS );

	SpawnCustom();
}

void admElectroBase::SpawnCustom()
{
	// override this thing uwu
}

void admElectroBase::Think()
{
	idEntity::Think(); // display model and run physics
}

// override these methods in your classes
void admElectroBase::OnMultimeter( idWeapon *weap )
{
	weap->Measure( 220.0f );
}

void admElectroBase::OnScrewdriver( idWeapon *weap )
{
	OnShorted();
	weap->GetOwner()->AddForce( this, 0, 
								idVec3( 0, 0, 0 ), 
								idVec3( 0, 0, 64 ) );
}

void admElectroBase::OnCutter( idWeapon *weap )
{
	if ( electroState == EL_ON )
		TurnOff();
	else if ( electroState == EL_OFF )
		TurnOn();
}

void admElectroBase::OnShorted()
{
	electroState = EL_SHORTED;
	ActivateCustomTargets( this, short_targets );

	common->Printf( "owo\n" );

}
void admElectroBase::TurnOff()
{
	electroState = EL_OFF;
	ActivateCustomTargets( this, off_targets );

	common->Printf( "owo off\n" );
}
void admElectroBase::TurnOn()
{
	electroState = EL_ON;
	ActivateCustomTargets( this, on_targets );

	common->Printf( "owo on\n" );
}
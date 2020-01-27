#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Electro_Base.h"
#include "Electro_Plug.h"
#include "Electro_Wire.h"

CLASS_DECLARATION( admElectroBase, admElectroWire )
END_CLASS

void admElectroWire::SpawnCustom()
{
	idStr fileName;
	if ( !spawnArgs.GetString( "articulatedFigure", "*unknown", fileName ) )
	{
		gameLocal.Error( "AF file not found for %s", GetName() );
	}

	af.SetAnimator( GetAnimator() );
	if ( !af.Load( this, fileName ) )
	{
		gameLocal.Error( "AF file %s not found for %s", fileName.c_str(), GetName() );
	}

	af.Start();

	af.GetPhysics()->Rotate( GetPhysics()->GetAxis().ToRotation() );
	af.GetPhysics()->Translate( GetPhysics()->GetOrigin() );

	af.LoadState( spawnArgs );

	af.UpdateAnimation();
	UpdateVisuals();
}

void admElectroWire::SpawnThink()
{

}

void admElectroWire::Think()
{

}

void admElectroWire::OnMultimeter( idWeapon *weap )
{

}

void admElectroWire::OnScrewdriver( idWeapon *weap )
{

}

void admElectroWire::OnCutter( idWeapon *weap )
{

}
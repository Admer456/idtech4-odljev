#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Electro_Base.h"
#include "Electro_Socket.h"
#include "Electro_PowerBox.h"

CLASS_DECLARATION( admElectroBase, admElectroPowerBox )
END_CLASS

bool IsModelOkay( idStr const &model, bool crashIfNotFound = true )
{
	if ( renderModelManager->CheckModel( model.c_str() ) )
	{
		return true; // Model is okay
	}
	else
	{
		if ( !crashIfNotFound )
			gameLocal.Warning( "Model '%s' not found", model );
		else
			gameLocal.Error( "Model '%s' not found", model);
	}

	return false;
}

void admElectroPowerBox::SpawnCustom()
{
	modelUninsulated = spawnArgs.GetString( "model_stripped" );
	IsModelOkay( modelUninsulated, false );

	modelCut = spawnArgs.GetString( "model_cut" );
	IsModelOkay( modelCut, false );
}

void admElectroPowerBox::SpawnThink()
{
	FindCustomTargets( "target_socket", sockets );
}

void admElectroPowerBox::Think()
{
	// do nothing
}

void admElectroPowerBox::OnMultimeter( idWeapon *weap )
{
	if ( electroState == EL_ON )
	{
		if ( powerBoxState == PowerBox_Cut )
		{
			weap->Measure( 999.9f );
			return;
		}
	}

	weap->Measure( 0.0f );
}

void admElectroPowerBox::OnScrewdriver( idWeapon *weap )
{
	if ( electroState == EL_ON )
	{
		if ( powerBoxState == PowerBox_Stripped )
		{
			OnShorted();
		}
	}
}

void admElectroPowerBox::OnCutter( idWeapon *weap )
{
	if ( electroState == EL_ON )
	{
		if ( powerBoxState == PowerBox_Normal )
		{
			powerBoxState = PowerBox_Stripped;
			SetModel( modelUninsulated );
		}
		else if ( powerBoxState == PowerBox_Stripped )
		{
			powerBoxState = PowerBox_Cut;
			SetModel( modelCut );

			TurnOff();
		}
	}
}

void admElectroPowerBox::OnShorted()
{
	admElectroBase::OnShorted();

	for ( int i = 0; i < sockets.Num(); i++ )
	{
		idEntity *ent = sockets[ i ].GetEntity();
		static_cast<admElectroSocket*>(ent)->OnShorted();
	}
}

void admElectroPowerBox::TurnOff()
{
	admElectroBase::TurnOff();

	for ( int i = 0; i < sockets.Num(); i++ )
	{
		idEntity *ent = sockets[ i ].GetEntity();
		static_cast<admElectroSocket*>(ent)->TurnOff();
	}
}

void admElectroPowerBox::TurnOn()
{
	admElectroBase::TurnOn();

	for ( int i = 0; i < sockets.Num(); i++ )
	{
		idEntity *ent = sockets[ i ].GetEntity();
		static_cast<admElectroSocket*>(ent)->TurnOn();
	}
}





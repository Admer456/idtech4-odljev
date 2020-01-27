#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Computer_Keyboard.h"
#include "Computer_Monitor.h"

CLASS_DECLARATION( idEntity, admComputerKeyboard )
END_CLASS

admComputerKeyboard::admComputerKeyboard()
{
	keys = 0;
	numbers = 0;
	user = NULL;
}

admComputerKeyboard::~admComputerKeyboard()
{
	keys = 0;
	numbers = 0;
	user = NULL;
}

void admComputerKeyboard::Spawn()
{
//	idEntity::Spawn();

	FindTargets();

	GetPhysics()->SetContents( CONTENTS_SOLID );
	BecomeActive( TH_THINK );
	BecomeActive( TH_UPDATEVISUALS );
}

void admComputerKeyboard::Think()
{
	idEntity::Think();

	if ( user )
	{
		UpdatePressedKeys();
		
		idEntity *ent;

		for ( int i = 0; i < targets.Num(); i++ )
		{
			ent = targets[ i ].GetEntity();

			if ( ent )
			{
				admComputerMonitor *mon = static_cast<admComputerMonitor*>(ent);

				if ( mon->timeToDismount )
				{
					mon->timeToDismount = false;

					if ( user )
					{
						OnUnuse( user );
					}
				}
			}
		}
	}

	if ( keys || numbers )
		UpdateMonitor();

	keys = 0;
	numbers = 0;
}

/*
	How this thing works:
	Let's examine this on a 4-bit key scheme.
	keys = 0100, oldkeys = 0000				(case 1. key pressed this frame only)
	keys & !oldkeys = 0100 & 1111 = 0100

	keys = 0100, oldkeys = 0100				(case 2. key held both frames)
	keys & !oldkeys = 0100 % 1011 = 0000

	keys = 0000, oldkeys = 0100				(case 3. key held last frame only)
	keys & !oldkeys = 0000 & 1011 = 0000
*/
void admComputerKeyboard::UpdatePressedKeys()
{
	keys	= user->usercmd.keyboard.keys	 & ~(user->oldusercmd.keyboard.keys);
	numbers = user->usercmd.keyboard.numkeys & ~(user->oldusercmd.keyboard.numkeys);
}

void admComputerKeyboard::UpdateMonitor()
{
	idEntity *ent;

	for ( int i = 0; i < targets.Num(); i++ )
	{
		ent = targets[ i ].GetEntity();

		if ( ent )
		{
			admComputerMonitor *mon = static_cast<admComputerMonitor*>(ent);

			mon->SetKeys( keys, numbers );

			if ( mon->timeToDismount )
			{
				mon->timeToDismount = false;

				if ( user )
				{
					OnUnuse( user );
				}
			}
		} 
	}
}

void admComputerKeyboard::OnUse( idPlayer *player )
{
	player->isUsing = true;
	player->isUsingAgain = false;
	user = player;
	user->Bind( this, false );
}

void admComputerKeyboard::OnUnuse( idPlayer *player )
{
	player->isUsing = false;
	player->isUsingAgain = true;
	user->Unbind();
	player->usedEntity = NULL;
	user = NULL;

	UpdateMonitor();
}
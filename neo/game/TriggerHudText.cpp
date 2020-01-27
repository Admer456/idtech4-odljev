/*
===============================================================================
	HUD text display

	November 2019, Odljev, Admer456
===============================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "TriggerHudText.h"

CLASS_DECLARATION( idEntity, admTriggerHudText )
	EVENT( EV_Activate, admTriggerHudText::Event_Activate )
END_CLASS

admTriggerHudText::admTriggerHudText()
{
	hudStrings[ 0 ].Clear(); // unrolling the for loop
	hudStrings[ 1 ].Clear(); // so the compiler nigga
	hudStrings[ 2 ].Clear(); // doesn't have to :^)

	hudStringColour.Zero();
	hudStringAlpha = 0;

	hudActive = false;

	currentPos = 0;
	currentLine = 0;
	printTimer = 0;
}

admTriggerHudText::~admTriggerHudText()
{
	hudStrings[ 0 ].Clear(); // unrolling the for loop
	hudStrings[ 1 ].Clear(); // so the compiler nigga
	hudStrings[ 2 ].Clear(); // doesn't have to :^)

	hudStringColour.Zero();
	hudStringAlpha = 0;
}

void admTriggerHudText::Spawn()
{
	hudStrings[ 0 ] = spawnArgs.GetString( "hudMsg1", " " );
	hudStrings[ 1 ] = spawnArgs.GetString( "hudMsg2", " " );
	hudStrings[ 2 ] = spawnArgs.GetString( "hudMsg3", " " );

	hudStringColour = spawnArgs.GetVector( "msgColor", "1.0 1.0 1.0" );
	hudStringAlpha = spawnArgs.GetFloat( "msgAlpha", "1.0" );

	hudTextMode = spawnArgs.GetInt( "msgMode", "1" );
	if ( hudTextMode > 1 || hudTextMode < 0 )
	{
		gameLocal.Warning( "&s admTriggerHudText::Spawn() - msgmode out of range! Setting to 0\n", GetName() );
		hudTextMode = 0;
	}
	if ( hudTextMode == 0 )
	{
		currentAlpha = hudStringAlpha;
	}

	blipSound = declManager->FindSound( "sound/blip1fastrev" );

	hudActive = false;
	BecomeActive( TH_THINK );
}

void admTriggerHudText::Think()
{
	static idEntity *ent;
	static idPlayer *pl;
	
	float timeAlive = MS2SEC(gameLocal.GetTime() - hudActivationTime);

	if ( hudActive )
	{
		if ( printTimer++ > 3 )
			printTimer = 0;

		if ( hudTextMode == 1 ) // fade mode
		{
			for ( int i = 0; i < 3; i++ )
				currentStrings[ i ] = hudStrings[ i ];

			if ( timeAlive < 0.5 )
				currentAlpha = hudStringAlpha * (timeAlive * 2.0);
			else if ( timeAlive > 5.0 && timeAlive < 5.5 )
				currentAlpha = hudStringAlpha * (1 - ((timeAlive - 5.0) * 2.0));
		}
		else if ( hudTextMode == 0 && printTimer == 0 ) // scan-out mode
		{
			int len1 = hudStrings[ 0 ].Length();
			int len2 = hudStrings[ 1 ].Length();
			int len3 = hudStrings[ 2 ].Length();
			int totalLength = len1 + len2 + len3 - 3;

			if ( currentPos > totalLength )
			{
				currentAlpha -= 0.01;

				if ( currentAlpha < 0.001 )
				{
					hudActive = false;
					BecomeInactive( TH_THINK );
				}
			}

			if ( currentPos < len1 )
				currentLine = 0;
			else if ( currentPos < len1 + len2 - 1 )
				currentLine = 1;
			else
				currentLine = 2;

			if ( currentPos <= totalLength && currentLine != -1 )
			{
				if ( currentLine == 0 )
					currentStrings[ currentLine ] += hudStrings[ currentLine ][ currentPos ];
				else if ( currentLine == 1 )
					currentStrings[ currentLine ] += hudStrings[ currentLine ][ currentPos - len1 ];
				else if ( currentLine == 2 )
					currentStrings[ currentLine ] += hudStrings[ currentLine ][ currentPos - len1 - len2 + 1 ];
			}

			if ( currentPos <= totalLength )
			{
				currentPos++;

				gameSoundChannel_t soundChannel;
				if ( currentPos % 3 == 0 )
					soundChannel = SND_CHANNEL_DEMONIC;
				else if ( currentPos % 3 == 1 )
					soundChannel = SND_CHANNEL_PDA;
				else if ( currentPos % 3 == 2 )
					soundChannel = SND_CHANNEL_RADIO;

				StartSoundShader( blipSound, soundChannel, SSF_OMNIDIRECTIONAL | SSF_GLOBAL, true, NULL );
			}
		}

		for ( int i = 0; i < MAX_GENTITIES; i++ )
		{
			ent = gameLocal.entities[ i ];
			if ( ent && ent->IsType( idPlayer::Type ) )
			{
				pl = static_cast<idPlayer*>(ent);

				pl->hud->SetStateString( "hudMsg1", currentStrings[ 0 ] );
				pl->hud->SetStateString( "hudMsg2", currentStrings[ 1 ] );
				pl->hud->SetStateString( "hudMsg3", currentStrings[ 2 ] );

				pl->hud->SetStateString( "hudMsgCol",
										 va( "%f %f %f %f",
											 hudStringColour[ 0 ],
											 hudStringColour[ 1 ],
											 hudStringColour[ 2 ],
											 currentAlpha ) );
				pl->hud->SetStateFloat( "hudMsgAlpha", currentAlpha );

				if ( !hudActive )
					pl->hud->HandleNamedEvent( "fadeoutCustomTextOverlay" );
				else 
					pl->hud->HandleNamedEvent( "updateCustomTextOverlay" );
			}
		}
	}
}

void admTriggerHudText::Event_Activate( idEntity *activator )
{
//	for ( int i = 0; i < 3; i++ )
//		hudStrings[ i ].Clear();

	hudActivationTime = gameLocal.GetTime();
	hudActive = true;
}
#pragma once

/*
===============================================================================
	Game text display entity

	This entity will display text on the player's screen when triggered.
	It supports scan-out and fade-in-out.
===============================================================================
*/

class admTriggerHudText : public idEntity
{
public:
	CLASS_PROTOTYPE( admTriggerHudText );

							admTriggerHudText();
							~admTriggerHudText();

	void					Spawn( void );
	void					Think( void );

private:
	idStr					hudStrings[ 3 ];		// spawnArgs HUD strings
	idStr					currentStrings[ 3 ];	// actual strings that get sent to the HUD - if scan-out is used
	idVec3					hudStringColour;
	float					hudStringAlpha;			// spawnArgs string alpha
	float					currentAlpha;			// actual alpha that gets sent to the HUD

	int						hudTextMode;			// 0 = scan-out; 1 = fade
	float					hudActivationTime;
	bool					hudActive;

	int						currentPos;
	int						currentLine;
	int						printTimer;

	const idSoundShader		*blipSound;

	void					Event_Activate( idEntity *activator );
};
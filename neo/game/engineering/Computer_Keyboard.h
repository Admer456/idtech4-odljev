#pragma once

/*
===============================================================================
	Computer keyboard entity
	Accepts user input and sends signals to its monitor entity

	When the player uses it, the player gets bound
	to the keyboard, and has to type keys.
===============================================================================
*/

class admComputerKeyboard : public idEntity
{
public:
	CLASS_PROTOTYPE( admComputerKeyboard );

	admComputerKeyboard();
	~admComputerKeyboard();

	void		Spawn( void );
	void		Think( void );

	void		UpdatePressedKeys( void );
	void		UpdateMonitor( void );

	void		OnUse( idPlayer *player );
	void		OnUnuse( idPlayer *player );

	// 0-26 -> A-Z, 27 & 28 -> (), 29 -> ., 30 -> Enter, 31 -> Backspace
	int			keys;		// which key is pressed this frame 
	int			numbers;	// which number is pressed this frame

private:
	idPlayer	*user;
};
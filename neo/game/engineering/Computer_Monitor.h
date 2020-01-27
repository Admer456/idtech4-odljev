#pragma once

/*
===============================================================================
	Computer monitor entity

	Update its GUI state strings, and manages
	some basic computer functionality

	1. Check user input
	2. Update screen if keys are held
		2.1. Check commands
		2.2. Interpret commands if any
		2.3. Draw screen
	3. Repeat

	To-do:
	- add YugoBASIC interpreter
===============================================================================
*/

// Single instruction
// Has a line number (for goto statements),
// and a line of code to be interpreted
struct BasicInstruction
{
	void Clear()
	{
		lineCode.Clear();
		lineNumber = -1;
	}

	idStr		lineCode;
	int			lineNumber;
};

// BASIC variable template
// variables get dynamically created in the BASIC program
// and get manipulated
template <class Type>
struct BasicVar
{
	Type		varValue;
	idStr		varName;
};

// BASIC data types
// No support for structs and arrays for now
enum BasicDataType
{
	BasicInt,
	BasicFloat,
	BasicString,

	BasicNoType
};

// Low-level opcodes
// Used when actually interpreting an instruction
// An instruction can have multiple opcodes
enum BasicOpCode
{
	BOP_NONE,		// wait
	BOP_DECL,		// declare a var
	BOP_ADD,		// add, src += dst
	BOP_SUBTRACT,	// subtract, src -= dst
	BOP_DIVIDE,		// divide, src /= dst
	BOP_MULTIPLY,	// multiply, src *= dst
	BOP_PRINT,		// print a message, separated by colons
	BOP_GOTO,		// go to a code line
	BOP_FOR,		// start a for loop, limited by an END LOOP
	BOP_ENDIF,		// ends an if statement
	BOP_ENDLOOP,	// ends a for loop
	BOP_END,		// ends a program

	BOP_MAX
};

// Memory of a computer
// Can store as many variables as needed, though I could eventually
// limit it to 640k for all variables, where the program would stop
// running if it reached the maximum memory
struct BasicMemory
{
	void	Clear();

	template <class Type>
	void	CreateVar( idStr const &name, Type const &value );

	template <class Type>
	Type	FindVar( idStr const &name, int *position = 0 );

	idList< BasicVar<int> >			ints;
	idList< BasicVar<float> >		floats;
	idList< BasicVar<idStr> >		strings;

	idStr	screenString;
};

// Single operation
// Refers to none, one or two variables,
// which it performs an operation upon; and an op code
struct BasicOperation
{
public:
	template <class Type>
	void			Init( Type val, BasicOpCode opcode, BasicMemory *memory, idStr *strSource = 0, idStr *strDest = 0 );

	void			Operate();

private:
	BasicDataType	opType = BasicNoType;
	BasicOpCode		opCode = BOP_NONE;
	BasicMemory		*memory = 0;

	idStr			strSource;
	idStr			strDest;

	idStr			valString;
	int				valInt;
	float			valFloat;

	int				addrSource = 0;
	int				addrDest = 0;

};

namespace BasicStringOps
{
	idStr			PrintArguments( idStr &line );
	idStr			FindNextPrintArgument( idStr &line, int &position );
	int				CalcPrintArguments( idStr &line );
	BasicDataType	DetermineType( idStr &line );

	bool			VariableExists( idStr &varname, BasicMemory &memory );

	template <class Type>
	Type			GetValueFromString( idStr &str );

	template <class Type>
	Type			GetValueFromVariable( idStr &varname, BasicMemory *memory );

}

// YugoBASIC Interpreter
// Interprets one instruction per think frame (thus infinite loops are supported)
// Contains a memory module to store variables
struct BasicInterpreter
{
	void				Clear();

	void				AddLine( idStr &str, int line );
	void				Run();
	bool				Parse( BasicInstruction &instruction );
	void				Execute();
	void				End();

	void				NextInstruction();
	void				Goto( int address );

	idList< BasicInstruction >	instructions;
	idList< BasicOperation >	operations;
	BasicInstruction	currentInstruction;
	BasicMemory			memory;
	idStr				printedString;

	int					lastInstruction;
};

enum ComputerMode
{
	Terminal,			// Default mode, from which we can change to other modes, or write BASIC etc.
						// Supported commands: cls, exit (and alternatives), BASIC input, run,
						// [program name], shell,  

	BASIC,				// BASIC mode, which interprets a loaded BASIC program
						// Can actually interpret some Terminal mode commands as well

	Graphical,			// GUI mode, abandons keyboard input and takes mouse input
						// Exiting this returns to either Terminal or BASIC

	GraphicalTerminal,	// Like GUI mode, but you use the keyboard instead
						// Exiting this returns to either Terminal or BASIC

	SystemShell,		// Like Terminal, but is for special system stuff, like admin mode
						// Exiting this returns to Terminal

	LoginToTerminal,	// Login into a terminal, prompts for a username and password
						// Passing this starts Terminal

	Prog_Burek,
	Prog_Shell,
	Prog_Shutdown,
	Prog_
};

constexpr int numScreenStrings = 24;
constexpr int SkipKeyInput = 31;

class admComputerMonitor : public idEntity
{
public:
	CLASS_PROTOTYPE( admComputerMonitor );

						admComputerMonitor();
						~admComputerMonitor();

	void				Spawn( void );
	void				Think( void );
	void				SpawnThink( void );

	void				SetKeys( int keys, int nums );
	void				UpdateScreen();
	void				HandleKeys();
	void				DeleteSingleChar();

	bool				IsCommand( char *command );
	bool				CommandStartsWith( char *substring ); // actually, command starts with
	bool				IsCommandEmpty();

	void				InterpretCommand();

	void				PerformNewline();
	void				PrintL( char *message );
	void				Print( char *message );
	void				PrintSyntaxError();
	void				ClearScreen();

	char				*AsciiToChar( int num );

	bool				timeToDismount;

private:
	idStr				screenStrings[ numScreenStrings ];
	idStr				screenStringsCopy[ numScreenStrings ];
	
	BasicInterpreter	BASIC;
	ComputerMode		mode;
	idList<idEntityPtr<idEntity>> serialPorts;
	idStr				passwordString;

	idList<idStr>		directoryFolders;
	idList<idList<idStr>> directoryProgs;
	
	idStr				progGui;	// GUI program name
	idStr				directoryCurrent;

	int					graphicalCursor; // currently selected option in GraphicalTerminal mode

	bool				isRunningProgram;
	int					keys;
	int					nums;
	int					wasShift;	// user pressed Shift last time
	int					cursor;		// position in input text field
	int					lastline;	// bottom row
};
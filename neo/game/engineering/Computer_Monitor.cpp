/*
===============================================================================
	Computer monitor entity

	November 2019, Odljev, Admer456
===============================================================================
*/

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "Game_local.h"
#include "Computer_Monitor.h"
#include "../framework/KeyInput.h"

template <class Type>
BasicDataType CheckType( Type val )
{
	if ( std::is_same<Type, int>::value )
	{
		return BasicInt;
	}

	else if ( std::is_same<Type, float>::value )
	{
		return BasicFloat;
	}

	else if ( std::is_same<Type, idStr>::value )
	{
		return BasicString;
	}

	return BasicNoType;
}

void BasicMemory::Clear()
{
	ints.Clear();
	floats.Clear();
	strings.Clear();
}

template <class Type>
void BasicMemory::CreateVar( idStr const &name, Type const &value )
{
	BasicVar<Type> var;
	var.varName = name;
	var.varValue = value;

	if ( CheckType( value ) == BasicInt )
		ints.Append( var );

	else if ( CheckType( value ) == BasicFloat )
		floats.Append( var );

	else if ( CheckType( value ) == BasicString )
		strings.Append( var );
}

template <class Type>
Type BasicMemory::FindVar( idStr const &name, int *position )
{
	Type ret;

	if ( std::is_same<Type, int>::result )
	{
		for ( int i = 0; i < ints.Num(); i++ )
		{
			if ( ints[ i ].varName == name )
			{
				*position = i;
				return ints[ i ].varValue;
			}
		}
	}

	else if ( std::is_same<Type, float>::result )
	{
		for ( int i = 0; i < floats.Num(); i++ )
		{
			if ( floats[ i ].varName == name )
			{
				*position = i;
				return floats[ i ].varValue;
			}
		}
	}

	else if ( std::is_same<Type, idStr>::result )
	{
		for ( int i = 0; i < strings.Num(); i++ )
		{
			if ( strings[ i ].varName == name )
			{
				*position = i;
				return strings[ i ].varValue;
			}
		}
	}

	else
	{
		printf( "FUCK YOU DUMB PROGRAMMER\nFindVar is only limited to 3 types: int, float, idStr" );
		assert( 0 );
	}

}

template <class Type>
void BasicOperation::Init( Type val, BasicOpCode opcode, BasicMemory *memory, idStr *strSource, idStr *strDest )
{
	opType = CheckType( val );
	opCode = opcode;

	assert( memory );

	if ( memory )
		this->memory = memory;
	if ( strSource )
		this->strSource = *strSource;
	if ( strDest )
		this->strDest = *strDest;
}

void BasicOperation::Operate()
{
	switch ( opType )
	{
	case BasicString:

		switch ( opCode )
		{
		case BOP_PRINT:
			memory->screenString = BasicStringOps::PrintArguments( valString );
			break;

		case BOP_NONE:
			break;

		default: 
			memory->screenString = "Error: This operation is not available for string";
			break;
		}

		break;
	}
}

idStr BasicStringOps::PrintArguments( idStr &line )
{
	return idStr( "not implemented" );
}

idStr BasicStringOps::FindNextPrintArgument( idStr &line, int &position )
{
	idStr arg;

	for ( int i = position; i < line.Length(); i++ )
	{
		if ( line[ i ] == ',' )
		{
			position++;
			break;
		}

		arg += line[ i ];
	}

	return arg;
}

int BasicStringOps::CalcPrintArguments( idStr &line )
{
	int args = 1;

	for ( int i = 1; i < line.Length() - 3; i++ )
	{
		if ( line[ i ] == 'R' &&
			 line[ i + 1 ] == 'E' &&
			 line[ i + 2 ] == 'M' )
			break;

		if ( line[ i ] == ',' )
			args++;
	}

	return args;
}

BasicDataType BasicStringOps::DetermineType( idStr &line )
{
	for ( int i = 0; i < line.Length(); i++ )
	{
		if ( line[ i ] == '\"' )
			return BasicString;
	}

	for ( int i = 0; i < line.Length(); i++ )
	{
		if ( line[ i ] == '.' )
			return BasicFloat;
	}

	return BasicInt;
}

bool BasicStringOps::VariableExists( idStr &varname, BasicMemory &memory )
{
	if ( varname[ 0 ] == 'i' )
	{
		for ( int i = 0; i < memory.ints.Num(); i++ )
		{
			if ( memory.ints[ i ].varName == varname )
				return true;
		}
	}

	if ( varname[ 0 ] == 'f' )
	{
		for ( int i = 0; i < memory.floats.Num(); i++ )
		{
			if ( memory.floats[ i ].varName == varname )
				return true;
		}
	}

	if ( varname[ 0 ] == 's' )
	{
		for ( int i = 0; i < memory.strings.Num(); i++ )
		{
			if ( memory.strings[ i ].varName == varname )
				return true;
		}
	}

	return false;
}

template <class Type>
Type BasicStringOps::GetValueFromString( idStr &str )
{
	BasicDataType type = DetermineType( str );

	if ( type == BasicInt )
	{
		return atoi( str.c_str() );
	}

	else if ( type == BasicFloat )
	{
		return atof( str.c_str() );
	}

	else if ( type == BasicString )
	{
		idStr ret;
		for ( int i = 1; i < str.Length() - 1; i++ )
			ret += str[ i ];

		return ret;
	}

	return 0;
}

template <class Type>
Type BasicStringOps::GetValueFromVariable( idStr &varname, BasicMemory *memory )
{
	return memory->FindVar( varname );
}

void BasicInterpreter::Clear()
{
	memory.Clear();

	currentInstruction.lineCode.Clear();
	currentInstruction.lineNumber = -1;

	printedString.Clear();

	int lastInstruction = 0;
}

void BasicInterpreter::AddLine( idStr &str, int line )
{
	BasicInstruction newInstruction;
	newInstruction.lineCode = str;
	newInstruction.lineNumber = line;

	idStr copy;
	int copyLine = newInstruction.lineNumber;
	int lineNumberOffset = 1;

	while ( copyLine )
	{
		copyLine /= 10;
		lineNumberOffset++;
	}

	for ( int i = lineNumberOffset; i < newInstruction.lineCode.Length(); i++ )
	{
		copy += newInstruction.lineCode[ i ];
	}

	newInstruction.lineCode = copy;

	instructions.Append( newInstruction );
	lastInstruction++;
}

void BasicInterpreter::Run()
{
	if ( !Parse( instructions[ lastInstruction ] ) )
	{
		Clear();
		return;
	}

	Execute();
	NextInstruction(); // wait until next think
}

bool BasicInterpreter::Parse( BasicInstruction &instruction )
{
	idStr &code = instruction.lineCode;
	BasicOperation op;

	if ( code.StartsWith( "print" ) )
	{
		idStr printString;
		idStr copy;
		for ( int i = 5; i < code.Length(); i++ )
			copy += code[ i ];

		op.Init( idStr("Test"), BOP_PRINT, &memory );
		operations.Append( op );
	}

	else if ( code.StartsWith( "" ) )
	{
		op.Init( 0, BOP_NONE, &memory );
		operations.Append( op );
	}

	else if ( code.StartsWith( "end" ) )
	{
		op.Init( 0, BOP_END, &memory );
		operations.Append( op );
	}

	else
	{
		memory.screenString = idStr("Error: one or more keywords are unrecognisable: ") + code;
		return false;
	}

	return true;
}

void BasicInterpreter::Execute()
{
	for ( int i = 0; i < operations.Num(); i++ )
	{
		operations[ i ].Operate();
	}
}

void BasicInterpreter::NextInstruction()
{
	lastInstruction++;
	operations.Clear();
}

void BasicInterpreter::Goto( int address )
{

}

CLASS_DECLARATION( idEntity, admComputerMonitor )
END_CLASS

admComputerMonitor::admComputerMonitor()
{
	for ( int i = 0; i < 16; i++ )
	{
		screenStrings[ i ].Clear();
		screenStringsCopy[ i ].Clear();
	}

	passwordString.Clear();

	directoryFolders.Clear();
	directoryProgs.Clear();
	directoryCurrent.Clear();

	BASIC.Clear();
	isRunningProgram = false;

	keys = 0;
	nums = 0;
	cursor = -1;
	lastline = 0;
	timeToDismount = false;
	wasShift = false;
}

admComputerMonitor::~admComputerMonitor()
{
	for ( int i = 0; i < 16; i++ )
	{
		screenStrings[ i ].Clear();
		screenStringsCopy[ i ].Clear();
	}

	passwordString.Clear();

	directoryFolders.Clear();
	directoryProgs.Clear();
	directoryCurrent.Clear();

	keys = 0;
	nums = 0;
	cursor = -1;
	lastline = 0;
	timeToDismount = false;
}

/*
	"folder1"					"sys"
	"folder1_prog1"				"shell.cmd"
	"folder1_prog2"				"shutdown.cmd"
	"folder1_prog3"				"burek.exe"

	"folder2"					"progs"

	"folder3"					"temp"
*/

void admComputerMonitor::Spawn()
{
//	idEntity::Spawn();
	GetPhysics()->SetContents( CONTENTS_SOLID );
	BecomeActive( TH_THINK );
	BecomeActive( TH_UPDATEVISUALS );

	idStr folderString;
	idStr programString;

	for ( int folder = 0; folder < 9; folder++ )
	{
		folderString = spawnArgs.GetString( va("folder%d", folder) );

		if ( !folderString.IsEmpty() )
		{
			directoryFolders.Append( folderString );
			
			idStrList programs;

			for ( int program = 0; program < 9; program++ )
			{
				programString = spawnArgs.GetString( va( "folder%d_prog%d", folder, program ) );

				if ( !programString.IsEmpty() )
				{
					programs.Append( programString );
				}
			}

			directoryProgs.Append( programs );
		}
	}

	directoryCurrent = "root";
	
	idStr pwd = spawnArgs.GetString( "password" );

	if ( !pwd.IsEmpty() )
	{
		mode = ComputerMode::LoginToTerminal;
		passwordString = pwd;
		nums |= BIT(SkipKeyInput);
		PrintL( "Logging in, please type password" );
	}
	else
	{
		mode = ComputerMode::Terminal;
	}
}

void admComputerMonitor::Think()
{
	if ( !hasSpawnThought )
		DispatchSpawnThink();

	idEntity::Think();

	if ( mode != ComputerMode::Graphical )
	{
		if ( keys || nums )
		{
			UpdateScreen();
		}
	}
	else
	{

	}
	
	if ( keys )
		keys = 0;
	if ( nums )
		nums = 0;
}

void admComputerMonitor::SpawnThink()
{
	FindCustomTargets( "target_serial", serialPorts );

	common->Printf( "serialPorts: %d (%s)\n", serialPorts.Num(), serialPorts[0].GetEntity()->GetName() );
}

void admComputerMonitor::SetKeys( int keys, int nums )
{
	this->keys = keys;
	this->nums = nums;
}

void admComputerMonitor::UpdateScreen()
{
	HandleKeys();

	for ( int i = 0; i < numScreenStrings - 1; i++ )
	{
		renderEntity.gui[0]->SetStateString( va( "screenString%d", i ), screenStrings[i] );
	}
}

void admComputerMonitor::HandleKeys()
{
	for ( int i = 0; i < 26; i++ )
	{
		if ( keys & BIT( i ) )
		{
			if ( wasShift )
				screenStrings[ lastline ] += AsciiToChar( i + K_a - 32 ); // 97 is ASCII 'a'
			else
				screenStrings[ lastline ] += AsciiToChar( i + K_a );

			cursor++;
		}
	}

	if ( nums & BIT( NUM_SPACE ) )
	{
		screenStrings[ lastline ] += " ";
		cursor++;
	}

	if ( keys & BIT( KEY_ENTER ) )
	{
		InterpretCommand();
		cursor = -1;
	}

	if ( keys & BIT( KEY_DELETE ) )
	{
		if ( cursor > -1 )
		{
			DeleteSingleChar();
			cursor--;
		}
	}

	if ( !wasShift )
	{
		for ( int i = 0; i < 10; i++ )
		{
			if ( nums & BIT( i ) )
			{
				screenStrings[ lastline ] += AsciiToChar( i + K_0 );
				cursor++;
			}
		}
	}

	else //if ( wasShift )
	{
		if ( nums & BIT( 0 ) )
		{
			screenStrings[ lastline ] += "=";
			cursor++;
		}

		if ( nums & BIT( 2 ) )
		{
			screenStrings[ lastline ] += "\"";
			cursor++;
		}

		if ( nums & BIT( 3 ) )
		{
			screenStrings[ lastline ] += ".";
			cursor++;
		}

		if ( nums & BIT( 4 ) )
		{
			screenStrings[ lastline ] += ",";
			cursor++;
		}

		if ( nums & BIT( 5 ) )
		{
			screenStrings[ lastline ] += "-";
			cursor++;
		}

		if ( nums & BIT( 6 ) )
		{
			screenStrings[ lastline ] += "+";
			cursor++;
		}

		if ( nums & BIT( 7 ) )
		{
			screenStrings[ lastline ] += "/";
			cursor++;
		}

		if ( nums & BIT( 8 ) )
		{
			screenStrings[ lastline ] += "(";
			cursor++;
		}

		if ( nums & BIT( 9 ) )
		{
			screenStrings[ lastline ] += ")";
			cursor++;
		}
	}

	if ( nums & BIT( NUM_LCTRL ) )
	{
		// nothing
	}

	if ( nums & BIT( NUM_SHIFT ) )
		wasShift = true;
	else
		wasShift = false;

	gameLocal.Printf( "Cursor pos: %d Screen string: %s\n", cursor, screenStrings[ lastline ].c_str() );
}

void admComputerMonitor::PerformNewline()
{
	if ( lastline < numScreenStrings-2 )
	{
		lastline++;
	}

	else
	{
		for ( int i = 0; i <= numScreenStrings-1; i++ )
			screenStringsCopy[ i ] = screenStrings[ i ];

		for ( int i = 0; i <= numScreenStrings-2; i++ )
			screenStrings[ i ] = screenStringsCopy[ i + 1 ];
	}
}

void admComputerMonitor::DeleteSingleChar()
{
	idStr copy;

	for ( int i = 0; i < cursor; i++ )
		copy += screenStrings[ lastline ][ i ];

	screenStrings[ lastline ].Clear();
	screenStrings[ lastline ] = copy;
}

bool admComputerMonitor::IsCommand( char *command )
{
	return idStr::Ieq( screenStrings[ lastline ], command );
}

bool admComputerMonitor::CommandStartsWith( char *substring )
{
	return screenStrings[ lastline ].StartsWith( substring );
}

bool admComputerMonitor::IsCommandEmpty()
{
	bool empty = true;

	for ( uint i = 0; i < screenStrings[ lastline ].Length(); i++ )
		if ( screenStrings[ lastline ][ i ] != ' ' )
			empty = false;

	return empty;
}

void admComputerMonitor::InterpretCommand()
{
	// universal exit, regardless of the current program
	if ( IsCommand( "quit" )	||
		 IsCommand( "exit" )	||
		 IsCommand( "stand up" )||
		 IsCommand( "dismount" )||
		 IsCommand( "get off" ) ||
		 IsCommand( "izadi" )	||
		 IsCommand( "izadji" )	||
		 IsCommand( "bjezi" )	||
		 IsCommand( "ustani" )	||
		 IsCommand( "ustaj" ) )
	{
		PerformNewline();

		timeToDismount = true;
	}

	switch ( mode )
	{
	case Terminal:
	{
		if ( IsCommandEmpty() )
		{
			PerformNewline();
		}

		// Usable stuff
		else if ( IsCommand( "cls" ) )
		{
			ClearScreen();
		}

		else if ( IsCommand( "dir" ) )
		{
			PerformNewline();
			PrintL( va( "Current dir: %s", directoryCurrent.c_str() ) );
			
			if ( directoryCurrent == "root" )
			{
				for ( int i = 0; i < directoryFolders.Num(); i++ )
					PrintL( (char*)directoryFolders[i].c_str() );
			}
			else
			{
				int folderNum = 0;
				for ( ; folderNum < directoryFolders.Num(); folderNum++ )
					if ( directoryFolders[ folderNum ] == directoryCurrent )
						break;

				for ( int i = 0; i < directoryProgs[ folderNum ].Num(); i++ )
					PrintL( (char*)directoryProgs[ folderNum ][ i ].c_str() );
			}
		}

		// Easter eggs
		else if ( IsCommand( "glup si" ) )
		{
			PerformNewline();
			PrintL( "I ti si." );
		}

		else if ( IsCommand( "volim te" ) )
		{
			PerformNewline();
			PrintL( "Izvini, zauzeta sam. ;)" );
		}

		else if ( IsCommand( "i love you" ) )
		{
			PerformNewline();
			PrintL( "Sorry, but I'm taken. ;)" );
		}

		else if ( IsCommand( "nigga" ) )
		{
			PerformNewline();
			PrintL( "THAT IS RACIST!" );
		}

		else if ( CommandStartsWith( "1" ) ||
				  CommandStartsWith( "2" ) ||
				  CommandStartsWith( "3" ) ||
				  CommandStartsWith( "4" ) ||
				  CommandStartsWith( "5" ) ||
				  CommandStartsWith( "6" ) ||
				  CommandStartsWith( "7" ) ||
				  CommandStartsWith( "8" ) ||
				  CommandStartsWith( "9" ) )
		{
			// BASIC is not implemented yet, it'll take years
			//int line; idStr copy;
			//copy = screenStrings[ lastline ];
			//
			//copy.StripTrailingWhitespace();
			//line = atoi( copy.c_str() );
			//
			//BASIC.AddLine( screenStrings[ lastline ], line );
			PerformNewline();
			PrintL( "Error: YugoBASIC is not installed" );
			PrintL( "on this machine" );
		}

		else if ( IsCommand( "run" ) || IsCommand( "basic" )  )
		{
			//BASIC.Run();
			//isRunningProgram = true;
			PerformNewline();
			PrintL( "Error: YugoBASIC is not installed" );
			PrintL( "on this machine" );
		}

		else if ( IsCommand( "cd.." ) )
		{
			PerformNewline();
			directoryCurrent = "root";
		}

		else if ( CommandStartsWith( "cd" ) )
		{
			idStr folderName;
			for ( int i = 3; i < screenStrings[ lastline ].Length(); i++ )
			{
				folderName += screenStrings[ lastline ][ i ];
			}

			for ( int i = 0; i < directoryFolders.Num(); i++ )
			{
				if ( directoryFolders[ i ] == folderName )
				{
					directoryCurrent = folderName;
					PerformNewline();
					return;
				}
			}

			PerformNewline();
			PrintL( "Error: folder not found" );
		}

		else
		{
			if ( directoryCurrent == "root" )
			{
				PerformNewline();
				PrintL( "Error: no programs in root dir" );
			}
			else
			{
				int folderNum;
				for ( folderNum = 0; folderNum < directoryFolders.Num(); folderNum++ )
				{	
					if ( directoryFolders[ folderNum ] == directoryCurrent )
					{		
						break;
					}		  
				}

				for ( int i = 0; i < directoryProgs[ folderNum ].Num(); i++ )
				{
					if ( screenStrings[ lastline ] == directoryProgs[ folderNum ][ i ] )
					{
						if ( IsCommand( "burek.exe" ) )
							mode = Prog_Burek;
						else if ( IsCommand( "shell.cmd" ) )
							mode = Prog_Shell;
						else if ( IsCommand( "shutdown.cmd" ) )
							mode = Prog_Shutdown;

						PerformNewline();
						InterpretCommand();
						return;
					}
				}
			}

		//	PrintSyntaxError();
		}
	}break;

	case LoginToTerminal:
	{
		if ( IsCommand( (char*)passwordString.c_str() ) )
		{
			mode = Terminal;
			PerformNewline();
			PrintL( "Login successful!" );
		}
		else
		{	
			PerformNewline();
			PrintL( "Invalid password" );
		}

	}break;

	case SystemShell:
	{
		if ( CommandStartsWith( "serial" ) )
		{
			idStr copy;
			for ( int i = 6; i < screenStrings[ lastline ].Length(); i++ )
				copy += screenStrings[ lastline ][ i ];

			gameLocal.Printf( "copy: %s\n", copy.c_str() );

			if ( copy[ 0 ] != ' ' )
			{
				PerformNewline();
				PrintL( "Error: no space between command and arguments" );
				return;
			}

			idStr num;
			int untilChar = 0;

			for ( int i = 0; i < copy.Length(); i++ )
			{
				if ( copy[ i ] == ' ' )
					untilChar++;
				else
					break;
			}

			for ( int i = untilChar; i < copy.Length(); i++ )
			{
				num += copy[ i ];
			}

			gameLocal.Printf( "num: %s\n", num.c_str() );

			if ( num.IsNumeric() )
			{
				int serialPort = atoi( copy.c_str() );

				if ( serialPort >= serialPorts.Num() )
				{
					PerformNewline();
					PrintL( "Error: out of range; check available ports" );
					return;
				}

				idEntity *ent = serialPorts[ serialPort ].GetEntity();
				
				if ( ent->RespondsTo( EV_Activate ) || ent->HasSignal( SIG_TRIGGER ) )
				{
					ent->Signal( SIG_TRIGGER );
					ent->ProcessEvent( &EV_Activate, this );
				}

				ent->ActivateTargets( this );
				PerformNewline();
			}
			else
			{
				PerformNewline();
				PrintL( "Error: non-numeric character detected." );
				PrintL( "Environment variables are not supported." );
			}
		}

		else if ( IsCommand( "cls" ) )
		{
			ClearScreen();
		}

		else if ( CommandStartsWith( "list" ) )
		{
			idStr copy;
			for ( int i = 5; i < screenStrings[ lastline ].Length(); i++ )
				copy += screenStrings[ lastline ][ i ];

			PerformNewline();

			if ( copy.Ieq( "serial" ) || copy.Ieq( "serials" )  )
			{
				PrintL( "Scanning available serial ports..." );

				if ( serialPorts.Num() < 1 )
				{
					PrintL( "None" );
					return;
				}

				for ( int i = 0; i < serialPorts.Num(); i++ )
				{
					PrintL( va("%s at %d", serialPorts[i].GetEntity()->GetName(), i ) );
				}
			}

			else if ( copy.Ieq( "commands" ) )
			{
				PrintL( "==============================" );
				PrintL( "Scanning available commands..." );
				PrintL( "==============================" );
				PrintL( "list -arg (serial; commands)" );
				PrintL( "exit shell" );
				PrintL( "serial -arg (0 to maxrange)" );
				PrintL( "cls" );
			}

			else
			{
				PrintL( "==============================" );
				PrintL( "       SYSTEM SHELL HELP      " );
				PrintL( "==============================" );
				PrintL( "Enter 'list commands'" );
				PrintL( "for available cmds" );
			}
		}

		else if ( IsCommand( "exit shell" ) )
		{
			mode = Terminal;
			PerformNewline();
		}

	}break;

	case GraphicalTerminal:
	{
		if ( IsCommand( "exit program" ) )
		{
			renderEntity.gui[ 0 ]->HandleNamedEvent( "onComputerExit" );
			mode = Terminal;
		}

		else if ( IsCommand( "exit" ) )
		{
			timeToDismount = true;
		}

		else if ( IsCommandEmpty() )
		{

		}

		else
		{
			screenStrings[ lastline ].StripTrailingWhitespace();
			renderEntity.gui[ 0 ]->HandleNamedEvent( screenStrings[ lastline ] );
		}

		ClearScreen();

	}break;

	case Prog_Burek:
	{
		int r = gameLocal.random.RandomInt( 10 );
		switch ( r )
		{
		case 0: PrintL( "VOLIM BUREK sine"							); break;
		case 1: PrintL( "Burek je super"							); break;
		case 2: PrintL( "Burek! <3"									); break;
		case 3: PrintL( "Volim ja burek"							); break;
		case 4: PrintL( "Ma burek je najbolji, nema sumnje"			); break;
		case 5: PrintL( "Burek is, like, the best thing ever"		); break;
		case 6: PrintL( "Burek is awesome"							); break;
		case 7: PrintL( "I love burek"								); break;
		case 8: PrintL( "Did anyone say... burek?"					); break;
		default: PrintL( "Najbolji je kad je frisak iz peci, a?"	); break;
		}

		nums |= BIT( SkipKeyInput );
		mode = ComputerMode::Terminal;
	
	}break;

	case Prog_Shell:
	{
		ClearScreen();
		PrintL( "System shell / configuration mode" );
		PrintL( "---------------------------------" );

		nums |= BIT( SkipKeyInput );
		mode = ComputerMode::SystemShell;
	}break;

	case Prog_Shutdown:
	{
		ClearScreen();

		timeToDismount = true;
		nums |= BIT( SkipKeyInput );
		mode = ComputerMode::LoginToTerminal;
	}break;

	}
}

void admComputerMonitor::PrintL( char *message )
{
	Print( message );
	PerformNewline();
}

void admComputerMonitor::Print( char *message )
{
	screenStrings[ lastline ] += message;
}

void admComputerMonitor::PrintSyntaxError()
{
	PerformNewline();
	PrintL( va( "Unrecognised command! %s", screenStrings[lastline].c_str() ) );
}

void admComputerMonitor::ClearScreen()
{
	for ( int i = 0; i < numScreenStrings-1; i++ )
		screenStrings[ i ].Clear();

	lastline = 0;

	BASIC.Clear();
}

#define CaseChar( a ) case K_##a: return #a;

char* admComputerMonitor::AsciiToChar( int num )
{
	switch ( num )
	{
	CaseChar( a )	CaseChar( A )
	CaseChar( b )	CaseChar( B )
	CaseChar( c )	CaseChar( C )
	CaseChar( d )	CaseChar( D )
	CaseChar( e )	CaseChar( E )
	CaseChar( f )	CaseChar( F )
	CaseChar( g )	CaseChar( G )
	CaseChar( h )	CaseChar( H )
	CaseChar( i )	CaseChar( I )
	CaseChar( j )	CaseChar( J )
	CaseChar( k )	CaseChar( K )
	CaseChar( l )	CaseChar( L )
	CaseChar( m )	CaseChar( M )
	CaseChar( n )	CaseChar( N )
	CaseChar( o )	CaseChar( O )
	CaseChar( p )	CaseChar( P )
	CaseChar( q )	CaseChar( Q )
	CaseChar( r )	CaseChar( R )
	CaseChar( s )	CaseChar( S )
	CaseChar( t )	CaseChar( T )
	CaseChar( u )	CaseChar( U )
	CaseChar( v )	CaseChar( V )
	CaseChar( w )	CaseChar( W )
	CaseChar( x )	CaseChar( X )
	CaseChar( y )	CaseChar( Y )
	CaseChar( z )	CaseChar( Z )
	CaseChar( 0 )	CaseChar( 1 )
	CaseChar( 2 )	CaseChar( 3 )
	CaseChar( 4 )	CaseChar( 5 )
	CaseChar( 6 )	CaseChar( 7 )
	CaseChar( 8 )	CaseChar( 9 )
	}

	return "nan";
}

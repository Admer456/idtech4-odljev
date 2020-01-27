/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

This file is part of the Doom 3 GPL Source Code (?Doom 3 Source Code?).

Doom 3 Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Doom 3 Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Doom 3 Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Doom 3 Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Doom 3 Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

#include "../idlib/precompiled.h"
#include "../idlib/StrRef.h"
#pragma hdrstop

#define	LINE_WIDTH				78
#define CONSOLE_FIRSTREPEAT		200
#define CONSOLE_REPEAT			100

#define	COMMAND_HISTORY			64

// the console will query the cvar and command systems for
// command completion information

namespace {
	struct fhColoredChar {
		char c;
		char color;
	};

	struct consoleEntry_t {
		char buffer[512];
		int time;
	};

	enum class fhTimestampMode {
		Off = 0,
		Milliseconds = 1,
		Iso8601 = 2,
		COUNT
	};

	template<typename OnNewLine, typename OnChar>
	void processEntryBuffer(const consoleEntry_t& entry, fhTimestampMode timestampMode, OnNewLine&& onNewLine, OnChar&& onChar) {
		const int len = ::strlen(entry.buffer);

		char currentColor = idStr::ColorIndex(C_COLOR_CYAN);

		if (timestampMode == fhTimestampMode::Iso8601) {
			const int seconds = (entry.time / 1000) % 60;
			const int minutes = ((entry.time / (1000 * 60)) % 60);
			const int hours = ((entry.time / (1000 * 60 * 60)) % 24);
			const int milliseconds = entry.time % 1000;

			char timestamp[32] = { 0 };
			::sprintf(timestamp, "%02d:%02d:%02d.%03d ", hours, minutes, seconds, milliseconds);
			const int timeStampLen = ::strlen(timestamp);

			for (int x = 0; x < timeStampLen; ++x) {
				onChar(fhColoredChar{ timestamp[x], static_cast<char>(idStr::ColorIndex(C_COLOR_GRAY)) });
			}
		} else if (timestampMode == fhTimestampMode::Milliseconds) {
			char timestamp[32] = { 0 };
			::sprintf(timestamp, "%7d ", entry.time);
			const int timeStampLen = ::strlen(timestamp);

			for (int x = 0; x < timeStampLen; ++x) {
				onChar(fhColoredChar{ timestamp[x], static_cast<char>(idStr::ColorIndex(C_COLOR_GRAY)) });
			}
		}

		for (int x = 0; x < len; ) {
			const char* text = &entry.buffer[x];

			if (idStr::IsColor(text)) {
				if (*(text + 1) == C_COLOR_DEFAULT) {
					currentColor = idStr::ColorIndex(C_COLOR_CYAN);
				}
				else {
					currentColor = idStr::ColorIndex(*(text + 1));
				}

				x += 2;
				continue;
			}

			x += 1;

			if (*text == '\n') {
				onNewLine();
				continue;
			}

			if (*text == '\r') {
				continue;
			}

			if (*text == '\t') {
				continue;
			}

			onChar(fhColoredChar{ *text, currentColor });
		}
	}

	class fhConsoleLine : public idStaticList<fhColoredChar, 256> {
	public:
		void AppendText(char colorIndex, const char* text) {
			const int len = strlen(text);
			for (int i = 0; i < len && Num() < Max(); ++i) {
				this->Append(fhColoredChar{text[i], colorIndex});
			}
		}
	};

	template<typename T, int Capacity>
	class fhRingBuffer {
	public:
		fhRingBuffer()
			: num(0)
			, start(0)
		{}

		T& operator[](int index) {
			assert(index < num);
			return buffer[GetRingIndex(index)];
		}

		const T& operator[](int index) const {
			assert(index < num);
			return buffer[GetRingIndex(index)];
		}

		T& Append(const T& value) {
			const auto ringIndex = GetRingIndex(num);
			buffer[ringIndex] = value;

			if (num < Capacity) {
				num += 1;
			} else {
				start = (start + 1) % Capacity;
			}

			return buffer[ringIndex];
		}

		int GetNum() const {
			return num;
		}

		void Clear() {
			num = 0;
			start = 0;
		}

	private:
		int GetRingIndex(int index) const {
			return (start + index) % Capacity;
		}

		T buffer[Capacity];
		int num = 0;
		int start = 0;
	};
}


class idConsoleLocal : public idConsole {
public:
	idConsoleLocal();

	virtual	void		Init( void ) override;
	virtual void		Shutdown( void ) override;
	virtual	void		LoadGraphics( void ) override;
	virtual	bool		ProcessEvent( const sysEvent_t *event, bool forceAccept ) override;
	virtual	bool		Active( void ) override;
	virtual	void		Close( void ) override;
	virtual	void		Print( const char *text ) override;
	virtual	void		Draw( bool forceFullScreen ) override;
	virtual void        SaveHistory(const char* file) override;
	virtual void        LoadHistory(const char* file) override;

	void				Dump( const char *toFile );
	void				Clear();

	//============================

	const idMaterial *	charSetShader;

private:
	void				KeyDownEvent( int key );

	void				PageUp();
	void				PageDown();
	void				Top();
	void				Bottom();

	void				DrawInput( float y, float fontScale );
	void				DrawNotify();

	template<int BufferSize>
	void				FormatConsoleEntry(idStaticList<fhConsoleLine, BufferSize>& buffer, const consoleEntry_t& entry);

	void                DrawConsoleLine(float y, const fhConsoleLine& line, char* currentColor = nullptr, bool right = false);
	void                PrintConsoleEntry(const consoleEntry_t& entry, float& y);
	void				DrawSolidConsole( float frac );

	void				Scroll();
	void				SetDisplayFraction( float frac );
	void				UpdateDisplayFraction( void );
	fhTimestampMode     GetTimeStampMode() const;
	int                 GetTimeStampPrintLen() const;

	//============================

	bool				keyCatching;

	fhRingBuffer<consoleEntry_t, 1024> entries;
	int					displayOffset;

	fhRingBuffer<idEditField, COMMAND_HISTORY> history;
	int					currentHistoryLine;

	int					lastKeyEvent;	// time of last key event for scroll delay
	int					nextKeyEvent;	// keyboard repeat rate

	float				displayFrac;	// approaches finalFrac at scr_conspeed
	float				finalFrac;		// 0.0 to 1.0 lines of console to display
	int					fracTime;		// time of last displayFrac update

	struct fhConsoleMetrics {
		float fontScale;
		float letterWidth;
		float letterHeight;
		float lineHeight;
		int maxLetterPerLine;
	};

	fhConsoleMetrics	metrics;

	idEditField			consoleField;

	static idCVar		con_speed;
	static idCVar		con_notifyTime;
	static idCVar		con_notifyLines;
	static idCVar		con_noPrint;
	static idCVar		con_fontScale;
	static idCVar		con_size;
	static idCVar		con_timestamps;

	const idMaterial *	whiteShader;
	const idMaterial *	consoleShader;
	int                 startTime;
};

static idConsoleLocal localConsole;
idConsole	*console = &localConsole;

idCVar idConsoleLocal::con_speed( "con_speed", "3", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "speed at which the console moves up and down" );
idCVar idConsoleLocal::con_notifyTime( "con_notifyTime", "3", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "time messages are displayed onscreen when console is pulled up" );
idCVar idConsoleLocal::con_notifyLines("con_notifyLines", "10", CVAR_SYSTEM | CVAR_NOCHEAT | CVAR_ARCHIVE, "max number of console lines displayed onscreen when console is pulled up");
#ifdef DEBUG
idCVar idConsoleLocal::con_noPrint( "con_noPrint", "0", CVAR_BOOL|CVAR_SYSTEM|CVAR_NOCHEAT|CVAR_ARCHIVE, "print on the console but not onscreen when console is pulled up" );
#else
idCVar idConsoleLocal::con_noPrint( "con_noPrint", "1", CVAR_BOOL|CVAR_SYSTEM|CVAR_NOCHEAT|CVAR_ARCHIVE, "print on the console but not onscreen when console is pulled up" );
#endif
idCVar idConsoleLocal::con_fontScale( "con_fontScale", "0.5", CVAR_SYSTEM|CVAR_FLOAT|CVAR_NOCHEAT|CVAR_ARCHIVE, "scale of console font" );
idCVar idConsoleLocal::con_size("con_size", "0.3", CVAR_SYSTEM|CVAR_FLOAT|CVAR_NOCHEAT|CVAR_ARCHIVE, "screen size of console");
idCVar idConsoleLocal::con_timestamps("con_timestamps", "2", CVAR_SYSTEM | CVAR_INTEGER | CVAR_NOCHEAT | CVAR_ARCHIVE, "timestamps in console: 0=Off, 1=Milliseconds, 2=ISO8601");


/*
=============================================================================

	Misc stats

=============================================================================
*/

/*
==================
SCR_DrawTextLeftAlign
==================
*/
static void SCR_DrawTextLeftAlign( float &y, const char *text, ... ) {
	char string[MAX_STRING_CHARS];
	va_list argptr;
	va_start( argptr, text );
	idStr::vsnPrintf( string, sizeof( string ), text, argptr );
	va_end( argptr );
	renderSystem->DrawSmallStringExt( 0, y + 2, string, colorWhite, true, localConsole.charSetShader );
	y += SMALLCHAR_HEIGHT + 4;
}

/*
==================
SCR_DrawTextRightAlign
==================
*/
static void SCR_DrawTextRightAlign( float &y, const char *text, ... ) {
	char string[MAX_STRING_CHARS];
	va_list argptr;
	va_start( argptr, text );
	int i = idStr::vsnPrintf( string, sizeof( string ), text, argptr );
	va_end( argptr );
	renderSystem->DrawSmallStringExt( 635 - i * SMALLCHAR_WIDTH, y + 2, string, colorWhite, true, localConsole.charSetShader );
	y += SMALLCHAR_HEIGHT + 4;
}

/*
==================
SCR_DrawFPS
==================
*/
#define	FPS_FRAMES	4
static float SCR_DrawFPS( int mode ) {
	char		*s;
	int			w;
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int		i, total;
	int		fps;
	static	int	previous;
	int		t, frameTime;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = Sys_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;

	float y = 0;
	int x = 0;

	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 10000 * FPS_FRAMES / total;
		fps = (fps + 5)/10;

		if(mode == 1) {
			s = va( "%3dfps", fps );
			x = 600;
		}
		else if(mode == 2) {
			s = va( "%4.2fms", static_cast<float>(total) / FPS_FRAMES );
			x = 600;
		}
		else {
			s = va( "%3dfps (%4.2fms)", fps, static_cast<float>(total) / FPS_FRAMES );
			x = 565;
		}

		w = strlen( s ) * BIGCHAR_WIDTH;

		renderSystem->DrawScaledStringExt( x, idMath::FtoiFast( y ) + 2, s, colorWhite, true, localConsole.charSetShader, 0.55f );
	}

	return y + BIGCHAR_HEIGHT + 4;
}

/*
==================
SCR_DrawBackEndStats
==================
*/
float SCR_DrawBackEndStats( float y ) {

	int ypos = idMath::FtoiFast( y );
	const float lineHeight = 11;

	char buffer[128];
	const float xpos = 440;
	const float fontScale = 0.55f;

	auto PrintStats = [&](const char* name, const backEndGroupStats_t& stats, const idVec4& color) {
		const float milliseconds = stats.time * 0.001f;
		sprintf( buffer, "%-15s  %4d  %4d  %6d  %6.2f", name, stats.passes, stats.drawcalls, stats.tris, milliseconds );
		renderSystem->DrawScaledStringExt( xpos, ypos, buffer, color, true, localConsole.charSetShader, fontScale );
		ypos += lineHeight;
	};


#define TIME_FRAMES 10
	static backEndStats_t previous[TIME_FRAMES];
	static unsigned frame = 0;

	const auto stats = renderSystem->GetBackEndStats();
	previous[frame] = stats;
	frame = (frame + 1) % TIME_FRAMES;

	backEndStats_t avg;
	for(int j=0; j<TIME_FRAMES; ++j) {
		avg += previous[j];
	}

	avg /= TIME_FRAMES;

	backEndGroupStats_t sm_total;
	for(int i=0; i<3; ++i) {
		sm_total += avg.groups[backEndGroup::ShadowMap0 + i];
	}

	backEndGroupStats_t total;
	for (int i = 0; i < backEndGroup::NUM; ++i) {
		total += avg.groups[i];
	}

	sprintf( buffer, "                    p    dc    tris    time");
	renderSystem->DrawScaledStringExt( xpos, ypos, buffer, colorWhite, true, localConsole.charSetShader, fontScale );
	ypos += lineHeight;

	PrintStats("depth prepass", avg.groups[backEndGroup::DepthPrepass], colorWhite);
	//PrintStats("stencil shadows", avg.groups[backEndGroup::StencilShadows], colorWhite);
	PrintStats("shadow maps", sm_total, colorWhite);
	PrintStats("    0", avg.groups[backEndGroup::ShadowMap0], colorMdGrey);
	PrintStats("    1", avg.groups[backEndGroup::ShadowMap1], colorMdGrey);
	PrintStats("    2", avg.groups[backEndGroup::ShadowMap2], colorMdGrey);
	PrintStats("interaction", avg.groups[backEndGroup::Interaction], colorWhite);
	PrintStats("non-interaction", avg.groups[backEndGroup::NonInteraction], colorWhite);
	//PrintStats("fog lights", avg.groups[backEndGroup::FogLight], colorWhite);
	//PrintStats("blend lights", avg.groups[backEndGroup::BlendLight], colorWhite);
	PrintStats("          total", total, colorWhite);
/*
	sprintf( buffer, "                      total time: %6.2fms", avg.totaltime * 0.001f );
	renderSystem->DrawScaledStringExt( xpos, ypos, buffer, colorWhite, true, localConsole.charSetShader, fontScale );
	ypos += lineHeight;
*/
	return ypos;
}

/*
==================
SCR_DrawMemoryUsage
==================
*/
static float SCR_DrawMemoryUsage( float y ) {
	memoryStats_t allocs, frees;

	Mem_GetStats( allocs );
	SCR_DrawTextRightAlign( y, "total allocated memory: %4d, %4dkB", allocs.num, allocs.totalSize>>10 );

	Mem_GetFrameStats( allocs, frees );
	SCR_DrawTextRightAlign( y, "frame alloc: %4d, %4dkB  frame free: %4d, %4dkB", allocs.num, allocs.totalSize>>10, frees.num, frees.totalSize>>10 );

	Mem_ClearFrameStats();

	return y;
}

/*
==================
SCR_DrawAsyncStats
==================
*/
static float SCR_DrawAsyncStats( float y ) {
	int i, outgoingRate, incomingRate;
	float outgoingCompression, incomingCompression;

	if ( idAsyncNetwork::server.IsActive() ) {

		SCR_DrawTextRightAlign( y, "server delay = %d msec", idAsyncNetwork::server.GetDelay() );
		SCR_DrawTextRightAlign( y, "total outgoing rate = %d KB/s", idAsyncNetwork::server.GetOutgoingRate() >> 10 );
		SCR_DrawTextRightAlign( y, "total incoming rate = %d KB/s", idAsyncNetwork::server.GetIncomingRate() >> 10 );

		for ( i = 0; i < MAX_ASYNC_CLIENTS; i++ ) {

			outgoingRate = idAsyncNetwork::server.GetClientOutgoingRate( i );
			incomingRate = idAsyncNetwork::server.GetClientIncomingRate( i );
			outgoingCompression = idAsyncNetwork::server.GetClientOutgoingCompression( i );
			incomingCompression = idAsyncNetwork::server.GetClientIncomingCompression( i );

			if ( outgoingRate != -1 && incomingRate != -1 ) {
				SCR_DrawTextRightAlign( y, "client %d: out rate = %d B/s (% -2.1f%%), in rate = %d B/s (% -2.1f%%)",
											i, outgoingRate, outgoingCompression, incomingRate, incomingCompression );
			}
		}

		idStr msg;
		idAsyncNetwork::server.GetAsyncStatsAvgMsg( msg );
		SCR_DrawTextRightAlign( y, msg.c_str() );

	} else if ( idAsyncNetwork::client.IsActive() ) {

		outgoingRate = idAsyncNetwork::client.GetOutgoingRate();
		incomingRate = idAsyncNetwork::client.GetIncomingRate();
		outgoingCompression = idAsyncNetwork::client.GetOutgoingCompression();
		incomingCompression = idAsyncNetwork::client.GetIncomingCompression();

		if ( outgoingRate != -1 && incomingRate != -1 ) {
			SCR_DrawTextRightAlign( y, "out rate = %d B/s (% -2.1f%%), in rate = %d B/s (% -2.1f%%)",
										outgoingRate, outgoingCompression, incomingRate, incomingCompression );
		}

		SCR_DrawTextRightAlign( y, "packet loss = %d%%, client prediction = %d",
									(int)idAsyncNetwork::client.GetIncomingPacketLoss(), idAsyncNetwork::client.GetPrediction() );

		SCR_DrawTextRightAlign( y, "predicted frames: %d", idAsyncNetwork::client.GetPredictedFrames() );

	}

	return y;
}

/*
==================
SCR_DrawSoundDecoders
==================
*/
static float SCR_DrawSoundDecoders( float y ) {
	int index, numActiveDecoders;
	soundDecoderInfo_t decoderInfo;

	index = -1;
	numActiveDecoders = 0;
	while( ( index = soundSystem->GetSoundDecoderInfo( index, decoderInfo ) ) != -1 ) {
		int localTime = decoderInfo.current44kHzTime - decoderInfo.start44kHzTime;
		int sampleTime = decoderInfo.num44kHzSamples / decoderInfo.numChannels;
		int percent;
		if ( localTime > sampleTime ) {
			if ( decoderInfo.looping ) {
				percent = ( localTime % sampleTime ) * 100 / sampleTime;
			} else {
				percent = 100;
			}
		} else {
			percent = localTime * 100 / sampleTime;
		}
		SCR_DrawTextLeftAlign( y, "%3d: %3d%% (%1.2f) %s: %s (%dkB)", numActiveDecoders, percent, decoderInfo.lastVolume, decoderInfo.format.c_str(), decoderInfo.name.c_str(), decoderInfo.numBytes >> 10 );
		numActiveDecoders++;
	}
	return y;
}

//=========================================================================

/*
==============
Con_Clear_f
==============
*/
static void Con_Clear_f( const idCmdArgs &args ) {
	localConsole.Clear();
}

/*
==============
Con_Dump_f
==============
*/
static void Con_Dump_f( const idCmdArgs &args ) {
	if ( args.Argc() != 2 ) {
		common->Printf( "usage: conDump <filename>\n" );
		return;
	}

	idStr fileName = args.Argv(1);
	fileName.DefaultFileExtension(".txt");

	common->Printf( "Dumped console text to %s.\n", fileName.c_str() );

	localConsole.Dump( fileName.c_str() );
}

idConsoleLocal::idConsoleLocal()
{
	displayOffset = 0;
}

/*
==============
idConsoleLocal::Init
==============
*/
void idConsoleLocal::Init( void ) {
	keyCatching = false;

	lastKeyEvent = -1;
	nextKeyEvent = CONSOLE_FIRSTREPEAT;

	consoleField.Clear();

	consoleField.SetWidthInChars( LINE_WIDTH );
	currentHistoryLine = -1;

	cmdSystem->AddCommand( "clear", Con_Clear_f, CMD_FL_SYSTEM, "clears the console" );
	cmdSystem->AddCommand( "conDump", Con_Dump_f, CMD_FL_SYSTEM, "dumps the console text to a file" );

	startTime = Sys_Milliseconds();
}

/*
==============
idConsoleLocal::Shutdown
==============
*/
void idConsoleLocal::Shutdown( void ) {
	cmdSystem->RemoveCommand( "clear" );
	cmdSystem->RemoveCommand( "conDump" );
}

/*
==============
LoadGraphics

Can't be combined with init, because init happens before
the renderSystem is initialized
==============
*/
void idConsoleLocal::LoadGraphics() {
	charSetShader = declManager->FindMaterial( "textures/bigchars" );
	whiteShader = declManager->FindMaterial( "_white" );
	consoleShader = declManager->FindMaterial( "console" );
}

/*
================
idConsoleLocal::Active
================
*/
bool idConsoleLocal::Active( void ) {
	return keyCatching;
}

/*
================
idConsoleLocal::Close
================
*/
void idConsoleLocal::Close() {
	keyCatching = false;
	SetDisplayFraction( 0 );
	displayFrac = 0;	// don't scroll to that point, go immediately
}

/*
================
idConsoleLocal::Clear
================
*/
void idConsoleLocal::Clear() {
	Bottom();		// go to end
	entries.Clear();
}

fhTimestampMode idConsoleLocal::GetTimeStampMode() const {
	int i = con_timestamps.GetInteger();
	if (i >= 0 && i < static_cast<int>(fhTimestampMode::COUNT)) {
		return static_cast<fhTimestampMode>(i);
	}

	return fhTimestampMode::Off;
}

int idConsoleLocal::GetTimeStampPrintLen() const {
	switch (GetTimeStampMode()) {
	case fhTimestampMode::Off:
		return 0;
	case fhTimestampMode::Milliseconds:
		return 8;
	case fhTimestampMode::Iso8601:
		return 13;
	}

	assert(false);
	return 0;
}

/*
================
idConsoleLocal::Dump

Save the console contents out to a file
================
*/
void idConsoleLocal::Dump( const char *fileName ) {
	fhFileHandle f = fileSystem->OpenFileWrite( fileName );
	if ( !f ) {
		common->Warning( "couldn't open %s", fileName );
		return;
	}

	const auto timestampLen = GetTimeStampPrintLen();
	const auto timestampMode = GetTimeStampMode();

	for (int i = 0; i < entries.GetNum(); ++i) {
		const auto& entry = entries[i];

		bool newLine = false;
		const auto onNewLine = [&newLine]() {
			newLine = true;
		};

		const auto onChar = [&f, &newLine, timestampLen](fhColoredChar c) {
			if (newLine) {
				f->Write("\n", 1);
				for (int x = 0; x < timestampLen; ++x) {
					f->Write(" ", 1);
				}
				newLine = false;
			}
			f->Write(&c.c, 1);
		};
		processEntryBuffer(entry, timestampMode, onNewLine, onChar);
		f->Write("\n", 1);
	}
}

/*
================
idConsoleLocal::PageUp
================
*/
void idConsoleLocal::PageUp( void ) {
	displayOffset = Min(displayOffset + 1, entries.GetNum());
}

/*
================
idConsoleLocal::PageDown
================
*/
void idConsoleLocal::PageDown( void ) {
	displayOffset = Max(displayOffset - 1, 0);
}

/*
================
idConsoleLocal::Top
================
*/
void idConsoleLocal::Top( void ) {
	displayOffset = Max(entries.GetNum() - 1, 0);
}

/*
================
idConsoleLocal::Bottom
================
*/
void idConsoleLocal::Bottom( void ) {
	displayOffset = 0;
}


/*
=============================================================================

CONSOLE LINE EDITING

==============================================================================
*/

/*
====================
KeyDownEvent

Handles history and console scrollback
====================
*/
void idConsoleLocal::KeyDownEvent( int key ) {

	// Execute F key bindings
	if ( key >= K_F1 && key <= K_F12 ) {
		idKeyInput::ExecKeyBinding( key );
		return;
	}

	// ctrl-L clears screen
	if ( key == 'l' && idKeyInput::IsDown( K_CTRL ) ) {
		Clear();
		return;
	}

	// enter finishes the line
	if ( key == K_ENTER || key == K_KP_ENTER ) {

		common->Printf ( "]%s\n", consoleField.GetBuffer() );

		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, consoleField.GetBuffer() );	// valid command
		cmdSystem->BufferCommandText( CMD_EXEC_APPEND, "\n" );

		// copy line to history buffer
		history.Append(consoleField);
		currentHistoryLine = -1;

		consoleField.Clear();
		consoleField.SetWidthInChars( LINE_WIDTH );

		session->UpdateScreen();// force an update, because the command
								// may take some time
		return;
	}

	// command completion

	if ( key == K_TAB ) {
		consoleField.AutoComplete();
		return;
	}

	// command history (ctrl-p ctrl-n for unix style)

	if ( ( key == K_UPARROW ) ||
		 ( ( tolower(key) == 'p' ) && idKeyInput::IsDown( K_CTRL ) ) ) {

		if (currentHistoryLine < 0 && history.GetNum() > 0) {
			currentHistoryLine = history.GetNum() - 1;
			consoleField = history[currentHistoryLine];
		} else if (currentHistoryLine > 0) {
			currentHistoryLine -= 1;
			consoleField = history[currentHistoryLine];
		}

		return;
	}

	if ( ( key == K_DOWNARROW ) ||
		 ( ( tolower( key ) == 'n' ) && idKeyInput::IsDown( K_CTRL ) ) ) {

		if (currentHistoryLine < history.GetNum() - 1) {
			currentHistoryLine += 1;;
			consoleField = history[currentHistoryLine];
		} else {
			consoleField.Clear();
		}

		return;
	}

	// console scrolling
	if ( key == K_PGUP ) {
		PageUp();
		lastKeyEvent = eventLoop->Milliseconds();
		nextKeyEvent = CONSOLE_FIRSTREPEAT;
		return;
	}

	if ( key == K_PGDN ) {
		PageDown();
		lastKeyEvent = eventLoop->Milliseconds();
		nextKeyEvent = CONSOLE_FIRSTREPEAT;
		return;
	}

	if ( key == K_MWHEELUP ) {
		PageUp();
		return;
	}

	if ( key == K_MWHEELDOWN ) {
		PageDown();
		return;
	}

	// ctrl-home = top of console
	if ( key == K_HOME && idKeyInput::IsDown( K_CTRL ) ) {
		Top();
		return;
	}

	// ctrl-end = bottom of console
	if ( key == K_END && idKeyInput::IsDown( K_CTRL ) ) {
		Bottom();
		return;
	}

	// pass to the normal editline routine
	consoleField.KeyDownEvent( key );
}

/*
==============
Scroll
deals with scrolling text because we don't have key repeat
==============
*/
void idConsoleLocal::Scroll( ) {
	if (lastKeyEvent == -1 || (lastKeyEvent+200) > eventLoop->Milliseconds()) {
		return;
	}
	// console scrolling
	if ( idKeyInput::IsDown( K_PGUP ) ) {
		PageUp();
		nextKeyEvent = CONSOLE_REPEAT;
		return;
	}

	if ( idKeyInput::IsDown( K_PGDN ) ) {
		PageDown();
		nextKeyEvent = CONSOLE_REPEAT;
		return;
	}
}

/*
==============
SetDisplayFraction

Causes the console to start opening the desired amount.
==============
*/
void idConsoleLocal::SetDisplayFraction( float frac ) {
	finalFrac = frac;
	fracTime = com_frameTime;
}

/*
==============
UpdateDisplayFraction

Scrolls the console up or down based on conspeed
==============
*/
void idConsoleLocal::UpdateDisplayFraction( void ) {

	metrics.fontScale = idMath::ClampFloat(0.2f, 2.0f, con_fontScale.GetFloat());
	metrics.letterHeight = SMALLCHAR_HEIGHT * metrics.fontScale;
	metrics.letterWidth = SMALLCHAR_WIDTH * metrics.fontScale;
	metrics.lineHeight = metrics.letterHeight * 1.35f;
	metrics.maxLetterPerLine = (SCREEN_WIDTH / metrics.letterWidth) - 2;

	if ( con_speed.GetFloat() <= 0.1f ) {
		fracTime = com_frameTime;
		displayFrac = finalFrac;
		return;
	}

	// scroll towards the destination height
	if ( finalFrac < displayFrac ) {
		displayFrac -= con_speed.GetFloat() * ( com_frameTime - fracTime ) * 0.001f;
		if ( finalFrac > displayFrac ) {
			displayFrac = finalFrac;
		}
		fracTime = com_frameTime;
	} else if ( finalFrac > displayFrac ) {
		displayFrac += con_speed.GetFloat() * ( com_frameTime - fracTime ) * 0.001f;
		if ( finalFrac < displayFrac ) {
			displayFrac = finalFrac;
		}
		fracTime = com_frameTime;
	}
}

/*
==============
ProcessEvent
==============
*/
bool	idConsoleLocal::ProcessEvent( const sysEvent_t *event, bool forceAccept ) {
	bool consoleKey;
	consoleKey = event->evType == SE_KEY && ( event->evValue == Sys_GetConsoleKey( false ) || event->evValue == Sys_GetConsoleKey( true ) );

#if ID_CONSOLE_LOCK
	// If the console's not already down, and we have it turned off, check for ctrl+alt
	if ( !keyCatching && !com_allowConsole.GetBool() ) {
		if ( !idKeyInput::IsDown( K_CTRL ) || !idKeyInput::IsDown( K_ALT ) ) {
			consoleKey = false;
		}
	}
#endif

	// we always catch the console key event
	if ( !forceAccept && consoleKey ) {
		// ignore up events
		if ( event->evValue2 == 0 ) {
			return true;
		}

		consoleField.ClearAutoComplete();

		// a down event will toggle the destination lines
		if ( keyCatching ) {
			Close();
			Sys_GrabMouseCursor( true );
			cvarSystem->SetCVarBool( "ui_chat", false );
		} else {
			consoleField.Clear();
			keyCatching = true;
			if ( idKeyInput::IsDown( K_SHIFT ) ) {
				// if the shift key is down, don't open the console as much
				SetDisplayFraction( 0.2f );
			} else {
				const float consoleSize = idMath::ClampFloat(0.1f, 1.0f, con_size.GetFloat());
				SetDisplayFraction( consoleSize );
			}
			cvarSystem->SetCVarBool( "ui_chat", true );
		}
		return true;
	}

	// if we aren't key catching, dump all the other events
	if ( !forceAccept && !keyCatching ) {
		return false;
	}

	// handle key and character events
	if ( event->evType == SE_CHAR ) {
		// never send the console key as a character
		if ( event->evValue != Sys_GetConsoleKey( false ) && event->evValue != Sys_GetConsoleKey( true ) ) {
			consoleField.CharEvent( event->evValue );
		}
		return true;
	}

	if ( event->evType == SE_KEY ) {
		// ignore up key events
		if ( event->evValue2 == 0 ) {
			return true;
		}

		KeyDownEvent( event->evValue );
		return true;
	}

	// we don't handle things like mouse, joystick, and network packets
	return false;
}

/*
==============================================================================

PRINTING

==============================================================================
*/

/*
================
Print

Handles cursor positioning, line wrapping, etc
================
*/
void idConsoleLocal::Print( const char *txt ) {
#ifdef ID_ALLOW_TOOLS
	RadiantPrint(txt);

	if (com_editors & EDITOR_MATERIAL) {
		MaterialEditorPrintConsole(txt);
	}
#endif
	auto& entry = entries.Append(consoleEntry_t());
	::strncpy(entry.buffer, txt, sizeof(entry.buffer) - 1 );
	entry.time = Sys_Milliseconds() - startTime;

	if (displayOffset > 0) {
		displayOffset = Min(displayOffset + 1, entries.GetNum());
	}
}

/*
==============================================================================

DRAWING

==============================================================================
*/


/*
================
DrawInput

Draw the editline after a ] prompt
================
*/

void idConsoleLocal::DrawInput(float y, float fontScale) {
	int autoCompleteLength;

	if ( consoleField.GetAutoCompleteLength() != 0 ) {
		autoCompleteLength = strlen( consoleField.GetBuffer() ) - consoleField.GetAutoCompleteLength();

		if ( autoCompleteLength > 0 ) {
			renderSystem->SetColor4( .8f, .2f, .2f, .45f );

			renderSystem->DrawStretchPic( (2 * SMALLCHAR_WIDTH + consoleField.GetAutoCompleteLength() * SMALLCHAR_WIDTH) * fontScale,
							y + 2 * fontScale, autoCompleteLength * SMALLCHAR_WIDTH * fontScale, (SMALLCHAR_HEIGHT - 2) * fontScale, 0, 0, 0, 0, whiteShader );

		}
	}

	renderSystem->SetColor( idStr::ColorForIndex( C_COLOR_CYAN ) );

	renderSystem->DrawScaledChar( SMALLCHAR_WIDTH * fontScale, y, ']', localConsole.charSetShader, fontScale );

	consoleField.Draw(2 * SMALLCHAR_WIDTH * fontScale, y, SCREEN_WIDTH - 3 * SMALLCHAR_WIDTH * fontScale, true, charSetShader, fontScale );
}


/*
================
DrawNotify

Draws the last few lines of output transparently over the game top
================
*/
void idConsoleLocal::DrawNotify() {
	if (con_noPrint.GetBool()) {
		return;
	}

	const int maxLines = con_notifyLines.GetInteger();
	if (maxLines <= 0) {
		return;
	}

	static idList<fhConsoleLine> lineBuffer;
	lineBuffer.SetNum(maxLines, true);
	lineBuffer.Clear();

	const auto now = Sys_Milliseconds() - startTime;
	const auto minTime = now - 1000 * con_notifyTime.GetFloat();

	for (int i=entries.GetNum(); i>0 && lineBuffer.Num() < maxLines; --i) {
		const auto& entry = entries[i-1];
		if (entry.time < minTime) {
			break;
		}

		idStaticList<fhConsoleLine, 8> entryBuffer;

		FormatConsoleEntry(entryBuffer, entry);

		for (int j = entryBuffer.Num(); j > 0 && lineBuffer.Num() < maxLines; --j) {
			lineBuffer.Append(entryBuffer[j - 1]);
		}
	}

	if (lineBuffer.Num() == 0) {
		return;
	}

	float y = metrics.lineHeight * (lineBuffer.Num() - 1);

	char currentColor = idStr::ColorIndex(C_COLOR_CYAN);
	renderSystem->SetColor(idStr::ColorForIndex(C_COLOR_CYAN));

	for (int j = 0; j < lineBuffer.Num(); ++j) {
		const fhConsoleLine& line = lineBuffer[j];
		DrawConsoleLine(y, line, &currentColor);
		y -= metrics.lineHeight;
	}

	renderSystem->SetColor( colorCyan );
}

void idConsoleLocal::DrawConsoleLine(float y, const fhConsoleLine& line, char* currentColor, bool right) {

	char localCurrentColor = currentColor ? *currentColor : -1;

	auto drawLetter = [&](fhColoredChar letter, float x) {
		if (localCurrentColor == -1 || localCurrentColor != letter.color) {
			localCurrentColor = letter.color;
			renderSystem->SetColor(idStr::ColorForIndex(localCurrentColor));
		}

		if (letter.c == ' ') {
			return;
		}

		renderSystem->DrawScaledChar(x, idMath::FtoiFast(y), letter.c, localConsole.charSetShader, metrics.fontScale);
	};

	if (!right) {
		for (int x = 0; x < line.Num(); ++x) {
			drawLetter(line[x], (x + 1)*metrics.letterWidth);
		}
	}
	else {
		for (int x = line.Num(); x > 0; --x) {
			drawLetter(line[x - 1], SCREEN_WIDTH - ((line.Num() - x) + 1)*metrics.letterWidth);
		}
	}

	if (currentColor) {
		*currentColor = localCurrentColor;
	}
};

template<int BufferSize>
void idConsoleLocal::FormatConsoleEntry(idStaticList<fhConsoleLine, BufferSize>& lineBuffer, const consoleEntry_t& entry) {
	fhConsoleLine currentLine;

	const char cyanColorIndex = static_cast<char>(idStr::ColorIndex(C_COLOR_CYAN));
	const auto timestampLen = GetTimeStampPrintLen();
	const auto timestampMode = GetTimeStampMode();


	const auto onNewLine = [&]() {
		if (lineBuffer.Num() == lineBuffer.Max()) {
			return;
		}
		lineBuffer.Append(currentLine);
		currentLine.Clear();

		for (int x = 0; x < timestampLen; ++x) {
			currentLine.Append(fhColoredChar{ ' ', cyanColorIndex });
		}
	};

	const auto onChar = [&](fhColoredChar c) {
		currentLine.Append(c);

		if ( currentLine.Num() == metrics.maxLetterPerLine || currentLine.Num() == currentLine.Max() ) {
			onNewLine();
		}
	};
	processEntryBuffer(entry, timestampMode, onNewLine, onChar);
}

void idConsoleLocal::PrintConsoleEntry(const consoleEntry_t& entry, float& y) {

	idStaticList<fhConsoleLine, 8> lineBuffer;

	FormatConsoleEntry(lineBuffer, entry);

	char currentColor = idStr::ColorIndex(C_COLOR_CYAN);
	renderSystem->SetColor(idStr::ColorForIndex(C_COLOR_CYAN));

	for (int j = lineBuffer.Num(); j > 0; --j) {
		const fhConsoleLine& line = lineBuffer[j - 1];

		DrawConsoleLine(y, line, &currentColor);

		y -= metrics.lineHeight;

		if (y <= 0) {
			return;
		}
	}
}

/*
================
DrawSolidConsole

Draws the console with the solid background
================
*/

void idConsoleLocal::DrawSolidConsole( float frac ) {

	const float consoleHeight = SCREEN_HEIGHT * idMath::ClampFloat(0.1f, 1.0f, frac);

	const float bottomLineHeight = 1.4;
	const float inputLine = consoleHeight - bottomLineHeight - metrics.lineHeight;

	const char cyanColorIndex = static_cast<char>(idStr::ColorIndex(C_COLOR_CYAN));

	// draw the background and version number
	{
		renderSystem->DrawStretchPic( 0, 0, SCREEN_WIDTH, consoleHeight - bottomLineHeight, 0, 1.0f - displayFrac, 1, 1, consoleShader );
		renderSystem->SetColor( colorCyan );
		renderSystem->DrawStretchPic( 0, consoleHeight - bottomLineHeight, SCREEN_WIDTH, bottomLineHeight, 0, 0, 0, 0, whiteShader );

		idStr version = va( "%s - %i", ENGINE_VERSION, BUILD_NUMBER );
		fhConsoleLine line;
		line.AppendText(cyanColorIndex, version.c_str());
		DrawConsoleLine(inputLine, line, nullptr, true);
	}

	float y = inputLine - metrics.lineHeight;

	// draw from the bottom up
	if ( displayOffset > 0 ) {
		fhConsoleLine line;
		for (int x = 0; x < metrics.maxLetterPerLine; ++x) {
			line.Append(fhColoredChar{ (x % 4) ? ' ' : '^', cyanColorIndex });
		}
		DrawConsoleLine(y, line);
		y -= metrics.lineHeight;
	}


	if (entries.GetNum() > 0) {

		int index = (entries.GetNum() - 1) - displayOffset;

		for (; index >= 0; index--) {
			PrintConsoleEntry(entries[index], y);
			if (y<0) {
				break;
			}
		}
	}

	// draw the input prompt, user text, and cursor if desired
	DrawInput( inputLine, metrics.fontScale );

	renderSystem->SetColor( colorCyan );
}

void idConsoleLocal::LoadHistory(const char* fileName) {
	char *	f = nullptr;
	const int len = fileSystem->ReadFile(fileName, reinterpret_cast<void **>(&f), NULL);
	if (!f) {
		common->Printf("couldn't load %s\n", fileName);
		return;
	}

	idStaticList<char, MAX_EDIT_LINE + 1> buffer;

	auto commit = [&]() {
		if (buffer.Num() > 0) {
			buffer.Append('\0');
			idEditField editfield;
			editfield.SetWidthInChars(LINE_WIDTH);
			editfield.Clear();
			editfield.SetBuffer(buffer.Ptr());
			history.Append(editfield);
		}
		buffer.Clear();
	};

	for (int i = 0; i < len; ++i) {
		const char c = f[i];
		if (c == '\n') {
			commit();
			continue;
		}
		if (::isprint(c) && buffer.Num() < MAX_EDIT_LINE) {
			buffer.Append(c);
		}
	}

	commit();

	currentHistoryLine = -1;
	fileSystem->FreeFile(f);
}

void idConsoleLocal::SaveHistory(const char* fileName) {

	fhFileHandle f = fileSystem->OpenFileWrite(fileName);
	if (!f) {
		common->Warning("couldn't open %s", fileName);
		return;
	}

	for (int i=0; i<history.GetNum(); ++i) {
		auto& line = history[i];

		const char* text = line.GetBuffer();
		const int len = text ? strlen(text) : 0;
		if (len > 0) {
			f->Write(text, len);
			f->Write("\n", 1);
		}
	}
}

/*
==============
Draw

ForceFullScreen is used by the editor
==============
*/
void idConsoleLocal::Draw( bool forceFullScreen ) {
	float y = 0.0f;

	if ( !charSetShader ) {
		return;
	}

	if ( forceFullScreen ) {
		// if we are forced full screen because of a disconnect,
		// we want the console closed when we go back to a session state
		Close();
		// we are however catching keyboard input
		keyCatching = true;
	}

	Scroll();

	UpdateDisplayFraction();

	if ( forceFullScreen ) {
		DrawSolidConsole( 1.0f );
	} else if ( displayFrac ) {
		DrawSolidConsole( displayFrac );
	} else {
		// only draw the notify lines if the developer cvar is set,
		// or we are a debug build
		if ( !con_noPrint.GetBool() ) {
			DrawNotify();
		}
	}

	if ( int mode = com_showFPS.GetInteger() ) {
		y = SCR_DrawFPS( mode );
	}

	if ( com_showBackendStats.GetBool() ) {
		y = SCR_DrawBackEndStats( y );
	}

	if ( com_showMemoryUsage.GetBool() ) {
		y = SCR_DrawMemoryUsage( y );
	}

	if ( com_showAsyncStats.GetBool() ) {
		y = SCR_DrawAsyncStats( y );
	}

	if ( com_showSoundDecoders.GetBool() ) {
		y = SCR_DrawSoundDecoders( y );
	}
}

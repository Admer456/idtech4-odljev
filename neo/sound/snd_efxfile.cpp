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
#pragma hdrstop

#include "snd_local.h"

static const ALuint EFFECT_NOT_INITIALIZED = ~0;
static const ALuint EFFECT_IS_NULL = 0;

static float MillibelToGain( long millibels ) {
	return idMath::Pow( 10.0f, millibels / 2000.0f );
};

static long GainToMillibel( float gain ) {
	return static_cast<long>(::log10f( gain ) * 2000.0f);
};

void ConvertEAXToEFX( const EAXREVERBPROPERTIES *eax, EFXEAXREVERBPROPERTIES *efx ) {
#if  0
	efx->flDensity = idMath::Pow( eax->flEnvironmentSize, 3.0f ) / 16.0f;
#else
	efx->flDensity = (eax->flEnvironmentSize - 1) / 15.0f;
#endif
	efx->flDiffusion = eax->flEnvironmentDiffusion;
	efx->flGain = MillibelToGain( eax->lRoom );
	efx->flGainHF = MillibelToGain( eax->lRoomHF );
	efx->flGainLF = MillibelToGain( eax->lRoomLF );
	efx->flDecayTime = eax->flDecayTime;
	efx->flDecayHFRatio = eax->flDecayHFRatio;
	efx->flDecayLFRatio = eax->flDecayLFRatio;
	efx->flReflectionsGain = MillibelToGain( eax->lReflections );
	efx->flReflectionsDelay = eax->flReflectionsDelay;
	efx->flReflectionsPan[0] = eax->vReflectionsPan.x;
	efx->flReflectionsPan[1] = eax->vReflectionsPan.y;
	efx->flReflectionsPan[2] = eax->vReflectionsPan.z;
	efx->flLateReverbGain = MillibelToGain( eax->lReverb );
	efx->flLateReverbDelay = eax->flReverbDelay;
	efx->flLateReverbPan[0] = eax->vReverbPan.x;
	efx->flLateReverbPan[1] = eax->vReverbPan.y;
	efx->flLateReverbPan[2] = eax->vReverbPan.z;
	efx->flEchoTime = eax->flEchoTime;
	efx->flEchoDepth = eax->flEchoDepth;
	efx->flModulationTime = eax->flModulationTime;
	efx->flModulationDepth = eax->flModulationDepth;
	efx->flAirAbsorptionGainHF = MillibelToGain( eax->flAirAbsorptionHF );
	efx->flHFReference = eax->flHFReference;
	efx->flLFReference = eax->flLFReference;
	efx->flRoomRolloffFactor = eax->flRoomRolloffFactor;
	efx->iDecayHFLimit = (eax->ulFlags & 0x20) ? 1 : 0;
}

void ConvertEFXToEAX( const EFXEAXREVERBPROPERTIES *efx, EAXREVERBPROPERTIES *eax ) {
	eax->flEnvironmentSize = idMath::Pow( efx->flDensity * 16.0f, 1.0f / 3.0f );
	eax->flEnvironmentDiffusion = efx->flDiffusion;
	eax->lRoom = GainToMillibel( efx->flGain );
	eax->lRoomHF = GainToMillibel( efx->flGainHF );
	eax->lRoomLF = GainToMillibel( efx->flGainLF );
	eax->flDecayTime = efx->flDecayTime;
	eax->flDecayHFRatio = efx->flDecayHFRatio;
	eax->flDecayLFRatio = efx->flDecayLFRatio;
	eax->lReflections = GainToMillibel( efx->flReflectionsGain );
	eax->flReflectionsDelay = efx->flReflectionsDelay;
	eax->vReflectionsPan.x = efx->flReflectionsPan[0];
	eax->vReflectionsPan.y = efx->flReflectionsPan[1];
	eax->vReflectionsPan.z = efx->flReflectionsPan[2];
	eax->lReverb = GainToMillibel( efx->flLateReverbGain );
	eax->flReverbDelay = efx->flLateReverbDelay;
	eax->vReverbPan.x = efx->flLateReverbPan[0];
	eax->vReverbPan.y = efx->flLateReverbPan[1];
	eax->vReverbPan.z = efx->flLateReverbPan[2];
	eax->flEchoTime = efx->flEchoTime;
	eax->flEchoDepth = efx->flEchoDepth;
	eax->flModulationTime = efx->flModulationTime;
	eax->flModulationDepth = efx->flModulationDepth;
	eax->flAirAbsorptionHF = GainToMillibel( efx->flAirAbsorptionGainHF );
	eax->flHFReference = efx->flHFReference;
	eax->flLFReference = efx->flLFReference;
	eax->flRoomRolloffFactor = efx->flRoomRolloffFactor;
	eax->ulFlags = efx->iDecayHFLimit ? 0x20 : 0x0;
}

/*
===============
idSoundEffect::idSoundEffect
===============
*/
idLocalSoundEffect::idLocalSoundEffect( const char* name )
	: name(name)
	, alEffect(EFFECT_NOT_INITIALIZED)
	, boundToSlot(0)
	, dirty(false)
{
	memset( &eax, 0, sizeof( eax ) );
};

/*
===============
idSoundEffect::~idSoundEffect
===============
*/
idLocalSoundEffect::~idLocalSoundEffect() {
	Purge();
}

void idLocalSoundEffect::SetProperties( const EAXREVERBPROPERTIES& properties ) {
	if ( memcmp(&eax, &properties, sizeof(eax)) == 0 ) {
		return;
	}
	this->eax = properties;
	this->dirty = true;
}

/*
===============
idSoundEffect::Purge
===============
*/
void idLocalSoundEffect::Purge() {
	if (soundSystemLocal.EAXAvailable && !isNull() && isInitialized()) {
		UnbindEffect();
		alDeleteEffects(1, &alEffect);
	}

	alEffect = EFFECT_NOT_INITIALIZED;
	dirty = false;
}

/*
===============
idSoundEffect::Init
===============
*/
void idLocalSoundEffect::Init() {
	Purge();

	alGenEffects(1, &alEffect);
	if (isNull()) {
		return;
	}

	Configure();
}

template<typename T>
void idLocalSoundEffect::SetProperty( ALenum name, const char* text, T value, T minValue, T maxValue ) {
	value = Min( Max( value, minValue ), maxValue );
	alGetError();
	SetProperty( name, value );
	if ( alGetError() != AL_NO_ERROR ) {
		int i = 0;
	}
}

void idLocalSoundEffect::Configure() {
	alEffecti( alEffect, AL_EFFECT_TYPE, AL_EFFECT_EAXREVERB );

	EFXEAXREVERBPROPERTIES efx;
	ConvertEAXToEFX( &eax, &efx );

#define SETPROPERTY(Name, Var) SetProperty( AL_EAXREVERB_ ## Name, #Name, Var, AL_EAXREVERB_MIN_ ## Name, AL_EAXREVERB_MAX_ ## Name )

	SETPROPERTY( DENSITY, efx.flDensity );
	SETPROPERTY( DIFFUSION, efx.flDiffusion );
	SETPROPERTY( GAIN, efx.flGain );
	SETPROPERTY( GAINHF, efx.flGainHF );
	SETPROPERTY( GAINLF, efx.flGainLF );
	SETPROPERTY( DECAY_TIME, efx.flDecayTime );
	SETPROPERTY( DECAY_HFRATIO, efx.flDecayHFRatio );
	SETPROPERTY( DECAY_LFRATIO, efx.flDecayLFRatio );
	SETPROPERTY( REFLECTIONS_GAIN, efx.flReflectionsGain );
	SETPROPERTY( REFLECTIONS_DELAY, efx.flReflectionsDelay );
	SetProperty( AL_EAXREVERB_REFLECTIONS_PAN, &efx.flReflectionsPan[0] );
	SETPROPERTY( LATE_REVERB_GAIN, efx.flLateReverbGain );
	SETPROPERTY( LATE_REVERB_DELAY, efx.flLateReverbDelay );
	SetProperty( AL_EAXREVERB_LATE_REVERB_PAN, &efx.flLateReverbPan[0] );
	SETPROPERTY( ECHO_TIME, efx.flEchoTime );
	SETPROPERTY( ECHO_DEPTH, efx.flEchoDepth );
	SETPROPERTY( MODULATION_TIME, efx.flModulationTime );
	SETPROPERTY( MODULATION_DEPTH, efx.flModulationDepth );
	SETPROPERTY( AIR_ABSORPTION_GAINHF, efx.flAirAbsorptionGainHF );
	SETPROPERTY( HFREFERENCE, efx.flHFReference );
	SETPROPERTY( LFREFERENCE, efx.flLFReference );
	SETPROPERTY( ROOM_ROLLOFF_FACTOR, efx.flRoomRolloffFactor );
	SETPROPERTY( DECAY_HFLIMIT, (efx.iDecayHFLimit) ? AL_TRUE : AL_FALSE );

#undef SETPROPERTY
}

/*
===============
idSoundEffect::BindEffect
===============
*/
void idLocalSoundEffect::BindEffect( ALuint alEffectSlot, float gain ) {
	if ( !isInitialized() ) {
		Init();
	}

	if ( dirty ) {
		Configure();
	}

	alAuxiliaryEffectSloti( alEffectSlot, AL_EFFECTSLOT_EFFECT, alEffect );
	alAuxiliaryEffectSlotf( alEffectSlot, AL_EFFECTSLOT_GAIN, gain );
	boundToSlot = alEffectSlot;
}

void idLocalSoundEffect::UnbindEffect() {
	if ( boundToSlot ) {
		if ( !isNull() && isInitialized() ) {
			alAuxiliaryEffectSloti( boundToSlot, AL_EFFECTSLOT_EFFECT, 0 );
		}
		boundToSlot = 0;
	}
}

bool idLocalSoundEffect::isNull() const {
	return alEffect == EFFECT_IS_NULL;
}

bool idLocalSoundEffect::isInitialized() const {
	return alEffect != EFFECT_NOT_INITIALIZED;
}

/*
===============
idSoundEffect::SetProperty
===============
*/
void idLocalSoundEffect::SetProperty( ALenum name, float value ) {
	alEffectf(alEffect, name, value);
};

/*
===============
idSoundEffect::SetProperty
===============
*/
void idLocalSoundEffect::SetProperty( ALenum name, const float* value ) {
	alEffectfv(alEffect, name, value);
};

/*
===============
idSoundEffect::SetProperty
===============
*/
void idLocalSoundEffect::SetProperty( ALenum name, int value ) {
	alEffecti(alEffect, name, value);
};

/*
===============
idEFXFile::idEFXFile
===============
*/
idEFXFile::idEFXFile( void ) { }

/*
===============
idEFXFile::Clear
===============
*/
void idEFXFile::Clear( void ) {
	filename = "";
	osPath = false;
	effects.DeleteContents( true );
}

/*
===============
idEFXFile::~idEFXFile
===============
*/
idEFXFile::~idEFXFile( void ) {
	Clear();
}

/*
===============
idEFXFile::FindEffect
===============
*/
idLocalSoundEffect* idEFXFile::FindEffect(const idStr& name) {
	for (int i = 0; i < effects.Num(); i++) {
		if (effects[i] && effects[i]->GetName() == name) {
			return effects[i];
		}
	}
	return nullptr;
}

/*
===============
idEFXFile::ReadEffect
===============
*/
idLocalSoundEffect* idEFXFile::ReadEffect( idLexer &src ) {
	idToken token;

	if ( !src.ReadToken( &token ) )
		return nullptr;

	// reverb effect
	if (token != "reverb") {
		// other effect (not supported at the moment)
		src.Error("idEFXFile::ReadEffect: Unknown effect definition: %s", token.c_str());
		return nullptr;
	}

	EAXREVERBPROPERTIES reverb;

	src.ReadTokenOnLine( &token );
	const idToken name = token;

	if ( !src.ReadToken( &token ) ) {
		return nullptr;
	}

	if ( token != "{" ) {
		src.Error( "idEFXFile::ReadEffect: { not found, found %s", token.c_str() );
		return nullptr;
	}

	while (true) {
		if ( !src.ReadToken( &token ) ) {
			src.Error( "idEFXFile::ReadEffect: EOF without closing brace" );
			return nullptr;
		}

		if ( token == "}" ) {
			auto effect = new idLocalSoundEffect( name.c_str() );
			effect->SetProperties( reverb );
			return effect;
		}

		if ( token == "environment" ) {
			src.ReadTokenOnLine( &token );
			reverb.ulEnvironment = token.GetUnsignedLongValue();
		} else if ( token == "environment size" ) {
			reverb.flEnvironmentSize = src.ParseFloat();
		} else if ( token == "environment diffusion" ) {
			reverb.flEnvironmentDiffusion = src.ParseFloat();
		} else if ( token == "room" ) {
			reverb.lRoom = src.ParseInt();
		} else if ( token == "room hf" ) {
			reverb.lRoomHF = src.ParseInt();
		} else if ( token == "room lf" ) {
			reverb.lRoomLF = src.ParseInt();
		} else if ( token == "decay time" ) {
			reverb.flDecayTime = src.ParseFloat();
		} else if ( token == "decay hf ratio" ) {
			reverb.flDecayHFRatio = src.ParseFloat();
		} else if ( token == "decay lf ratio" ) {
			reverb.flDecayLFRatio = src.ParseFloat();
		} else if ( token == "reflections" ) {
			reverb.lReflections = src.ParseInt();
		} else if ( token == "reflections delay" ) {
			reverb.flReflectionsDelay = src.ParseFloat();
		} else if ( token == "reflections pan" ) {
			reverb.vReflectionsPan.x = src.ParseFloat();
			reverb.vReflectionsPan.y = src.ParseFloat();
			reverb.vReflectionsPan.z = src.ParseFloat();
		} else if ( token == "reverb" ) {
			reverb.lReverb = src.ParseInt();
		} else if ( token == "reverb delay" ) {
			reverb.flReverbDelay = src.ParseFloat();
		} else if ( token == "reverb pan" ) {
			reverb.vReverbPan.x = src.ParseFloat();
			reverb.vReverbPan.y = src.ParseFloat();
			reverb.vReverbPan.z = src.ParseFloat();
		} else if ( token == "echo time" ) {
			reverb.flEchoTime = src.ParseFloat();
		} else if ( token == "echo depth" ) {
			reverb.flEchoDepth = src.ParseFloat();
		} else if ( token == "modulation time" ) {
			reverb.flModulationTime = src.ParseFloat();
		} else if ( token == "modulation depth" ) {
			reverb.flModulationDepth = src.ParseFloat();
		} else if ( token == "air absorption hf" ) {
			reverb.flAirAbsorptionHF = src.ParseFloat();
		} else if ( token == "hf reference" ) {
			reverb.flHFReference = src.ParseFloat();
		} else if ( token == "lf reference" ) {
			reverb.flLFReference = src.ParseFloat();
		} else if ( token == "room rolloff factor" ) {
			reverb.flRoomRolloffFactor = src.ParseFloat();
		} else if ( token == "flags" ) {
			src.ReadTokenOnLine( &token );
			reverb.ulFlags = token.GetUnsignedLongValue();
		} else {
			src.ReadTokenOnLine( &token );
			src.Error( "idEFXFile::ReadEffect: Invalid parameter in reverb definition" );
			break;
		}
	}

	return nullptr;
}


/*
===============
idEFXFile::LoadFile
===============
*/
bool idEFXFile::LoadFile( const char *filename, bool OSPath ) {
	Clear();

	idLexer src( LEXFL_NOSTRINGCONCAT );
	idToken token;

	src.LoadFile( filename, OSPath );
	if ( !src.IsLoaded() ) {
		return false;
	}

	if ( !src.ExpectTokenString( "Version" ) ) {
		return false;
	}

	if ( src.ParseInt() != 1 ) {
		src.Error( "idEFXFile::LoadFile: Unknown file version" );
		return false;
	}

	while ( !src.EndOfFile() ) {
		if ( auto effect = ReadEffect( src ) ) {
			effects.Append( effect );
		}
	};

	this->filename = filename;
	this->osPath = OSPath;
	return true;
}


/*
===============
idEFXFile::UnloadFile
===============
*/
void idEFXFile::UnloadFile( void ) {
	Clear();
}

void idEFXFile::ReloadFile() {
	auto filename = this->filename;
	auto osPath = this->osPath;

	Clear();
	LoadFile( filename, osPath );
}
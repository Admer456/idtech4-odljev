/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 2016 Johannes Ohlemacher (http://github.com/eXistence/fhDOOM)

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

#pragma once

#include "../precompiled.h"
#include <qdialog.h>

class QCheckBox;
class QSlider;
class QComboBox;
class QLabel;
class QCheckBox;

class fhSliderNumEdit;

class fhEfxEditor : public QDialog {
public:
	fhEfxEditor( QWidget* parent );
	~fhEfxEditor();

	void Init();
	void UpdateGui();

private:
	void UpdateSliders( const idSoundEffect* effect );

	EAXREVERBPROPERTIES eax;
	idSoundEffect* currentEffect;
	QCheckBox* m_autoSelectCurrentEffect;
	QComboBox* m_locationComboBox;
	QLabel* m_currentLocation;
	QLabel* m_currentEffect;
	fhSliderNumEdit* m_envSize;
	fhSliderNumEdit* m_envDiffusion;
	fhSliderNumEdit* m_room;
	fhSliderNumEdit* m_roomHF;
	fhSliderNumEdit* m_roomLF;
	fhSliderNumEdit* m_decayTime;
	fhSliderNumEdit* m_decayRatioHF;
	fhSliderNumEdit* m_decayRatioLF;
	fhSliderNumEdit* m_reflections;
	fhSliderNumEdit* m_reflectionsDelay;
	fhSliderNumEdit* m_reverb;
	fhSliderNumEdit* m_reverbDelay;
	fhSliderNumEdit* m_reverbPan;
	fhSliderNumEdit* m_echoTime;
	fhSliderNumEdit* m_echoDepth;
	fhSliderNumEdit* m_modulationTime;
	fhSliderNumEdit* m_modulationDepth;
	fhSliderNumEdit* m_airAbsorptionHF;
	fhSliderNumEdit* m_referenceHF;
	fhSliderNumEdit* m_referenceLF;
	fhSliderNumEdit* m_roomRolloffFactor;

	void UpdateGame();
};

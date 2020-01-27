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

#include "EfxEditor.h"
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <QCloseEvent>
#include <QRadioButton>
#include <QSlider>

#include "../sound/sound.h"

#include "../widgets/NumEdit.h"
#include "../widgets/SliderNumEdit.h"
#include "../openal-soft/include/AL/efx-presets.h"

static const QString noneEffectName = "<none>";


fhEfxEditor::fhEfxEditor( QWidget* parent )
: QDialog(parent)
, currentEffect(nullptr) {

	QVBoxLayout* mainLayout = new QVBoxLayout;
	QHBoxLayout* leftRightLayout = new QHBoxLayout;
	leftRightLayout->setSpacing(0);
	leftRightLayout->setMargin(0);
	leftRightLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* left = new QWidget;
	QVBoxLayout* leftLayout = new QVBoxLayout;
	left->setLayout(leftLayout);
	leftRightLayout->addWidget(left);
	left->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );


	{
		m_autoSelectCurrentEffect = new QCheckBox();
		QHBoxLayout* box = new QHBoxLayout();
		QLabel* label = new QLabel( "auto select current effect" );
		label->setFixedWidth( 100 );

		box->addWidget( label );
		box->addWidget( m_autoSelectCurrentEffect );

		leftLayout->addLayout( box );
	}

	{
		m_currentEffect = new QLabel( "foo" );
		QHBoxLayout* box = new QHBoxLayout();
		QLabel* label = new QLabel( "Current Effect:" );
		label->setFixedWidth( 100 );

		box->addWidget( label );
		box->addWidget( m_currentEffect );

		leftLayout->addLayout( box );
	}

	{
		m_currentLocation = new QLabel( "bar" );
		QHBoxLayout* box = new QHBoxLayout();
		QLabel* label = new QLabel( "Current Location:" );
		label->setFixedWidth( 100 );

		box->addWidget( label );
		box->addWidget( m_currentLocation );

		leftLayout->addLayout( box );
	}

	{
		m_locationComboBox = new QComboBox( this );
		QHBoxLayout* box = new QHBoxLayout();
		QLabel* label = new QLabel( "Effect" );
		label->setFixedWidth( 100 );

		box->addWidget( label );
		box->addWidget( m_locationComboBox );

		leftLayout->addLayout( box );
	}

	auto addEffectPropertyFoo = [&]( const char* name, fhSliderNumEdit*& slider, float from, float to, float precision, float defaultValue ) {
		slider = new fhSliderNumEdit( this );
		slider->setMinimum( from );
		slider->setMaximum( to );
		slider->setPrecision( precision );
		slider->setFixedEditWidth( 65 );

		QPushButton* defaultValueButton = new QPushButton( this );
		defaultValueButton->setText( "default" );
		defaultValueButton->setFixedWidth( 60 );

		connect( defaultValueButton, &QPushButton::clicked, [=](bool){
			slider->setValue( defaultValue );
		} );

		QHBoxLayout* box = new QHBoxLayout();
		QLabel* label = new QLabel( name );
		label->setFixedWidth( 100 );

		box->addWidget( label );
		box->addWidget( slider );
		box->addWidget( defaultValueButton );

		leftLayout->addLayout( box );
	};

	auto addEffectProperty = [&]( const char* name, float* property, fhSliderNumEdit*& slider, float from, float to, float precision, float defaultValue ) {
		addEffectPropertyFoo( name, slider, from, to, precision, defaultValue );

		connect( slider, &fhSliderNumEdit::valueChanged, [=]( float v ){
			*property = v;
			this->UpdateGame();
		} );
	};

	auto addEffectProperty2 = [&]( const char* name, long* property, fhSliderNumEdit*& slider, float from, float to, float precision, float defaultValue ) {
		addEffectPropertyFoo( name, slider, from, to, precision, defaultValue );

		connect( slider, &fhSliderNumEdit::valueChanged, [=]( float v ){
			*property = (long)v;
			this->UpdateGame();
		} );
	};

	EFXEAXREVERBPROPERTIES efxDefaults = EFX_REVERB_PRESET_GENERIC;
	EAXREVERBPROPERTIES defaults;
	void ConvertEFXToEAX( const EFXEAXREVERBPROPERTIES *efx, EAXREVERBPROPERTIES *eax );
	ConvertEFXToEAX( &efxDefaults, &defaults );

	addEffectProperty( "env size", &eax.flEnvironmentSize, m_envSize, 1, 16, 2, defaults.flEnvironmentSize );
	addEffectProperty( "env diffusion", &eax.flEnvironmentDiffusion, m_envDiffusion, 0, 1, 2, defaults.flEnvironmentDiffusion );

	addEffectProperty2( "room size", &eax.lRoom, m_room, -10000, 0, 0, defaults.lRoom );
	addEffectProperty2( "room HF", &eax.lRoomHF, m_roomHF, -10000, 0, 0, defaults.lRoomHF );
	addEffectProperty2( "room LF", &eax.lRoomLF, m_roomLF, -10000, 0, 0, defaults.lRoomLF );

	addEffectProperty( "decay time", &eax.flDecayTime, m_decayTime, 0.1, 20, 2, defaults.flDecayTime );
	addEffectProperty( "decay HF ratio", &eax.flDecayHFRatio, m_decayRatioHF, 0.1, 20, 2, defaults.flDecayHFRatio );
	addEffectProperty( "decay LF ratio", &eax.flDecayLFRatio, m_decayRatioLF, 0.1, 20, 2, defaults.flDecayLFRatio );

	addEffectProperty2( "reflections", &eax.lReflections, m_reflections, -10000, 1000, 0, defaults.lReflections );
	addEffectProperty( "reflections delay", &eax.flReverbDelay, m_reflectionsDelay, 0, 0.3, 4, defaults.flReflectionsDelay );

	addEffectProperty2( "reverb", &eax.lReverb, m_reverb, -10000, 2000, 0, defaults.lReverb );
	addEffectProperty( "reverb delay", &eax.flReverbDelay, m_reverbDelay, 0, 0.1, 4, defaults.flReverbDelay );

	addEffectProperty( "echo time", &eax.flEchoTime, m_echoTime, 0.075, 0.25, 4, defaults.flEchoTime );
	addEffectProperty( "echo depth", &eax.flEchoDepth, m_echoDepth, 0, 1, 4, defaults.flEchoDepth );

	addEffectProperty( "modulation time", &eax.flModulationTime, m_modulationTime, 0.004, 4, 4, defaults.flModulationTime );
	addEffectProperty( "modulation depth", &eax.flModulationDepth, m_modulationDepth, 0, 1, 4, defaults.flModulationDepth );

	addEffectProperty( "air absorption HF", &eax.flAirAbsorptionHF, m_airAbsorptionHF, -100, 0, 2, defaults.flAirAbsorptionHF );
	addEffectProperty( "HF reference", &eax.flHFReference, m_referenceHF, 1000, 20000, 0, defaults.flHFReference );
	addEffectProperty( "LF reference", &eax.flLFReference, m_referenceLF, 1000, 20000, 0, defaults.flLFReference );
	addEffectProperty( "room rolloff factor", &eax.flRoomRolloffFactor, m_roomRolloffFactor, 0, 10, 2, defaults.flRoomRolloffFactor );


	mainLayout->addLayout( leftRightLayout );

	this->setLayout(mainLayout);


	QObject::connect( m_locationComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]( int index ){
		QVariant pointer = m_locationComboBox->currentData();
		this->UpdateSliders( (const idSoundEffect*)pointer.value<void*>() );
		this->UpdateGui();
	} );

	QObject::connect( m_autoSelectCurrentEffect, &QCheckBox::stateChanged, [=]( int ){
		this->UpdateGui();
	} );
}

fhEfxEditor::~fhEfxEditor() {
}

void fhEfxEditor::UpdateGame() {
	auto sw = soundSystem->GetPlayingSoundWorld();
	if ( !sw ) {
		return;
	}

	auto effect = sw->GetCurrentSoundEffect();
	if ( !effect ) {
		return;
	}

	effect->SetProperties( this->eax );
	sw->RecreateModifiedEffects();
}

void fhEfxEditor::Init() {
	m_locationComboBox->clear();
	this->setWindowTitle( "EfxEditor" );

	auto sw = soundSystem->GetPlayingSoundWorld();
	if ( !sw ) {
		return;
	}

	m_locationComboBox->addItem( noneEffectName );

	const auto numEffects = sw->GetSoundEffectNum();
	for ( int i = 0; i < numEffects; ++i ) {
		auto effect = sw->GetSoundEffect( i );
		if ( !effect ) {
			continue;
		}

		QVariant pointer;
		pointer.setValue( (void*)effect );

		m_locationComboBox->addItem( effect->GetName().c_str(), pointer );
	}

	UpdateGui();
}

void fhEfxEditor::UpdateGui() {
	auto sw = soundSystem->GetPlayingSoundWorld();
	if ( !sw ) {
		return;
	}

	auto effect = sw->GetCurrentSoundEffect();

	if ( effect ) {
		m_currentEffect->setText( effect->GetName().c_str() );
	}
	else {
		m_currentEffect->setText( noneEffectName );
	}

	if ( m_autoSelectCurrentEffect->checkState() == Qt::CheckState::Checked ) {

		m_locationComboBox->setEnabled( false );

		QVariant pointer;
		pointer.setValue( (void*)effect );
		for ( int i = 0; i < m_locationComboBox->count(); ++i ) {
			if ( pointer == m_locationComboBox->itemData( i ) ) {
				m_locationComboBox->setCurrentIndex( i );
				break;
			}
		}
	}
	else {
		m_locationComboBox->setEnabled( true );
	}

	currentEffect = effect;
}

void fhEfxEditor::UpdateSliders( const idSoundEffect* effect ) {
	if ( effect ) {
		eax = effect->GetProperties();
		m_envSize->setValue( eax.flEnvironmentSize );
		m_envDiffusion->setValue( eax.flEnvironmentDiffusion );
		m_room->setValue( eax.lRoom );
		m_roomHF->setValue( eax.lRoomHF );
		m_roomLF->setValue( eax.lRoomLF );
		m_decayTime->setValue( eax.flDecayTime );
		m_decayRatioHF->setValue( eax.flDecayHFRatio );
		m_decayRatioLF->setValue( eax.flDecayLFRatio );
		m_reflections->setValue( eax.lReflections );
		m_reflectionsDelay->setValue( eax.flReflectionsDelay );
		m_reverb->setValue( eax.lReverb );
		m_reverbDelay->setValue( eax.flReverbDelay );
		m_echoTime->setValue( eax.flEchoTime );
		m_echoDepth->setValue( eax.flEchoDepth );
		m_modulationTime->setValue( eax.flModulationTime );
		m_modulationDepth->setValue( eax.flModulationDepth );
		m_airAbsorptionHF->setValue( eax.flAirAbsorptionHF );
		m_referenceHF->setValue( eax.flHFReference );
		m_referenceLF->setValue( eax.flLFReference );
		m_roomRolloffFactor->setValue( eax.flRoomRolloffFactor );
	}
}
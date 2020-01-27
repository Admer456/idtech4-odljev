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

#include "LightEditor.h"
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qcolordialog.h>
#include <qmessagebox.h>
#include <QCloseEvent>
#include <QRadioButton>
#include <QSlider>

#include "../tools/radiant/QE3.H"
#include "../tools/radiant/GLWidget.h"
#include "../widgets/NumEdit.h"
#include "../widgets/ColorEdit.h"
#include "../widgets/SliderNumEdit.h"

fhLightEditor::fhLightEditor(QWidget* parent)
: QDialog(parent) {

	QVBoxLayout* mainLayout = new QVBoxLayout;
	QHBoxLayout* leftRightLayout = new QHBoxLayout;
	leftRightLayout->setSpacing(0);
	leftRightLayout->setMargin(0);
	leftRightLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* left = new QWidget;
	QVBoxLayout* leftLayout = new QVBoxLayout;
	left->setLayout(leftLayout);
	leftRightLayout->addWidget(left);
	left->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

	m_lighttype = new QComboBox;
	m_lighttype->addItem("Point Light");
	m_lighttype->addItem("Parallel Light");
	m_lighttype->addItem("Projected Light");

	leftLayout->addWidget(m_lighttype);
	leftLayout->addWidget(CreatePointLightParameters());
	leftLayout->addWidget(CreateParallelLightParameters());
	leftLayout->addWidget(CreateProjectedLightParameters());

	QWidget* right = new QWidget;
	QVBoxLayout* rightLayout = new QVBoxLayout;
	right->setLayout( rightLayout );
	leftRightLayout->addWidget( right );

	m_coloredit = new fhColorEdit(this);
	auto colorLayout = new QHBoxLayout;
	colorLayout->addWidget( new QLabel( "Color" ) );
	colorLayout->addWidget( m_coloredit );
	rightLayout->addLayout( colorLayout );

	auto shadowGroup = new QGroupBox("Shadows", this);
	auto shadowLayout = new QGridLayout(this);
	m_shadowMode = new QComboBox(this);
	m_shadowMode->addItem("No Shadows", QVariant((int)shadowMode_t::NoShadows));
	m_shadowMode->addItem("Default", QVariant((int)shadowMode_t::Default));
	m_shadowMode->addItem("Stencil Shadows", QVariant((int)shadowMode_t::StencilShadow));
	m_shadowMode->addItem("Shadow Mapping", QVariant((int)shadowMode_t::ShadowMap));

	m_shadowBrightnessEdit = new fhSliderNumEdit(this);
	m_shadowBrightnessEdit->setMaximum(1.0f);
	m_shadowBrightnessEdit->setMinimum(0.0f);
	m_shadowBrightnessEdit->setAdjustStepSize(0.01);
	m_shadowBrightnessEdit->setPrecision(2);

	m_shadowSoftnessEdit = new fhSliderNumEdit( this );
	m_shadowSoftnessEdit->setMaximum( 10.0f );
	m_shadowSoftnessEdit->setMinimum( 0.0f );
	m_shadowSoftnessEdit->setAdjustStepSize( 0.01 );
	m_shadowSoftnessEdit->setPrecision( 2 );

	m_shadowOffsetBias = new fhSliderNumEdit( this );
	m_shadowOffsetBias->setMaximum( 50.0f );
	m_shadowOffsetBias->setMinimum( 0.0f );
	m_shadowOffsetBias->setAdjustStepSize( 0.1f );
	m_shadowOffsetBias->setPrecision( 1 );

	m_shadowOffsetFactor = new fhSliderNumEdit( this );
	m_shadowOffsetFactor->setMaximum( 50.0f );
	m_shadowOffsetFactor->setMinimum( 0.0f );
	m_shadowOffsetFactor->setAdjustStepSize( 0.1f );
	m_shadowOffsetFactor->setPrecision( 1 );

	shadowGroup->setLayout(shadowLayout);
	shadowLayout->addWidget( new QLabel( "Mode", this ), 0, 0 );
	shadowLayout->addWidget( m_shadowMode, 0, 1 );
	shadowLayout->addWidget(new QLabel("Softness", this), 1, 0);
	shadowLayout->addWidget(m_shadowSoftnessEdit, 1, 1);
	shadowLayout->addWidget(new QLabel("Brightness", this), 2, 0);
	shadowLayout->addWidget( m_shadowBrightnessEdit, 2, 1 );
	shadowLayout->addWidget( new QLabel( "Offset Bias", this ), 3, 0 );
	shadowLayout->addWidget( m_shadowOffsetBias, 3, 1 );
	shadowLayout->addWidget( new QLabel( "Offset Factor", this ), 4, 0 );
	shadowLayout->addWidget( m_shadowOffsetFactor, 4, 1 );

	rightLayout->addWidget(shadowGroup);

	mainLayout->addLayout(leftRightLayout);

	QHBoxLayout* buttonLayout = new QHBoxLayout;
	m_cancelButton = new QPushButton("Cancel", this);
	m_applyButton = new QPushButton("Save", this);
	m_okButton = new QPushButton("OK", this);

	buttonLayout->addWidget(m_cancelButton);
	buttonLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
	buttonLayout->addWidget(m_applyButton);
	buttonLayout->addWidget(m_okButton);
	mainLayout->addLayout(buttonLayout);


	auto materialGroup = new QGroupBox("Material", this);
	auto materialVLayout = new QVBoxLayout(this);
	materialGroup->setLayout(materialVLayout);
	m_material = new QComboBox(this);
	materialVLayout->addWidget(m_material);

	m_materialFile = new QLabel(this);
	materialVLayout->addWidget(m_materialFile);
	LoadMaterials();

	m_drawableMaterial = new idGLDrawableMaterial();
	fhRenderWidget* renderWidget = new fhRenderWidget(this);
	renderWidget->setDrawable(m_drawableMaterial);
	materialVLayout->addWidget(renderWidget);

	rightLayout->addWidget(materialGroup);

	this->setLayout(mainLayout);

	QObject::connect(m_lighttype, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](){
		m_currentData.type = static_cast<fhLightType>(m_lighttype->currentIndex());
		m_modified = true;
		this->UpdateGame();
		UpdateLightParameters();
	});

	QObject::connect(m_coloredit, &fhColorEdit::valueChanged, [=](idVec3 color){
		this->m_currentData.color = color;
		this->m_modified = true;
		this->UpdateGame();
	});

	QObject::connect(m_pointlightParameters.radius, &fhVec3Edit::valueChanged, [=](idVec3 v){
		this->m_currentData.radius = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_pointlightParameters.center, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.center = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_parallellightParameters.radius, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.radius = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_parallellightParameters.direction, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.center = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_projectedlightParameters.target, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.target = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_projectedlightParameters.right, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.right = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_projectedlightParameters.up, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.up = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_projectedlightParameters.start, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.start = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_projectedlightParameters.end, &fhVec3Edit::valueChanged, [=]( idVec3 v ){
		this->m_currentData.end = v;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_shadowMode,  static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](){
		this->m_currentData.shadowMode = static_cast<shadowMode_t>(this->m_shadowMode->currentData().toInt());

		const bool shadowMapping = this->m_currentData.shadowMode == shadowMode_t::ShadowMap;
		this->m_shadowSoftnessEdit->setEnabled(shadowMapping);
		this->m_shadowBrightnessEdit->setEnabled(shadowMapping);
		this->m_shadowOffsetBias->setEnabled(shadowMapping);
		this->m_shadowOffsetFactor->setEnabled(shadowMapping);

		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_material, &QComboBox::currentTextChanged, [=](QString text){
		QByteArray ascii = text.toLocal8Bit();
		this->m_currentData.material = ascii.data();
		this->m_modified = true;
		this->m_drawableMaterial->setMedia(ascii.data());
		renderWidget->updateDrawable();

		if(const idMaterial* material = declManager->FindMaterial(ascii.data())) {
			this->m_materialFile->setText(QString("%1:%2").arg(material->GetFileName()).arg(material->GetLineNum()));
		} else {
			this->m_materialFile->setText("");
		}

		this->UpdateGame();
	} );

	QObject::connect( m_applyButton, &QPushButton::clicked, [=](){
		this->UpdateGame();
		this->m_originalData = this->m_currentData;
		this->m_modified = false;
	});


	QObject::connect( m_okButton, &QPushButton::clicked, [=](){
		this->UpdateGame();
		this->m_originalData = this->m_currentData;
		this->m_modified = false;
		this->close();
	} );

	QObject::connect( m_cancelButton, &QPushButton::clicked, [=](){
		this->m_currentData = this->m_originalData;
		this->m_modified = false;
		this->UpdateGame();
		this->close();
	} );

	m_timer.setInterval(33);
	m_timer.start();

	QObject::connect(&m_timer, &QTimer::timeout, [=](){
		if(this->isVisible())
			renderWidget->updateDrawable();
	});

	QObject::connect(m_shadowBrightnessEdit, &fhSliderNumEdit::valueChanged, [=](float f){
		this->m_currentData.shadowBrightness = f;
		this->m_modified = true;
		this->UpdateGame();
	});

	QObject::connect( m_shadowSoftnessEdit, &fhSliderNumEdit::valueChanged, [=]( float f ){
		this->m_currentData.shadowSoftness = f;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_shadowOffsetBias, &fhSliderNumEdit::valueChanged, [=]( float f ){
		this->m_currentData.shadowPolygonOffsetBias = f;
		this->m_modified = true;
		this->UpdateGame();
	} );

	QObject::connect( m_shadowOffsetFactor, &fhSliderNumEdit::valueChanged, [=]( float f ){
		this->m_currentData.shadowPolygonOffsetFactor = f;
		this->m_modified = true;
		this->UpdateGame();
	} );

	UpdateLightParameters();
}

fhLightEditor::~fhLightEditor() {
}

void fhLightEditor::closeEvent(QCloseEvent* event) {
	if(m_modified) {
		QMessageBox::StandardButton resBtn = QMessageBox::question( this, "Light Editor",
			tr( "You have unsaved changes." ),
			QMessageBox::Cancel | QMessageBox::Save | QMessageBox::Discard );

		if (resBtn == QMessageBox::Save) {
			this->UpdateGame();
			this->m_originalData = this->m_currentData;
			this->m_modified = false;
			event->accept();
			return;
		}

		if (resBtn == QMessageBox::Discard) {
			this->m_currentData = this->m_originalData;
			this->m_modified = false;
			this->UpdateGame();
			event->accept();
			return;
		}

		event->ignore();
		return;
	}

	event->accept();
}

QWidget* fhLightEditor::CreatePointLightParameters() {

	QGroupBox* pointlightGroup = new QGroupBox( tr( "Point Light Parameters" ) );;
	QGridLayout* pointlightgrid = new QGridLayout;

	m_pointlightParameters.radius = new fhVec3Edit(true, this);
	m_pointlightParameters.radius->setMinimumValue(idVec3(0,0,0));
	m_pointlightParameters.radius->setPrecision(1);
	m_pointlightParameters.center = new fhVec3Edit(this);
	m_pointlightParameters.center->setPrecision(1);

	pointlightgrid->addWidget( new QLabel( "Radius" ),        0, 0, Qt::AlignBottom );
	pointlightgrid->addWidget( m_pointlightParameters.radius, 0, 1 );

	pointlightgrid->addWidget( new QLabel( "Center" ),        1, 0, Qt::AlignBottom );
	pointlightgrid->addWidget( m_pointlightParameters.center, 1, 1 );

	pointlightGroup->setLayout( pointlightgrid );

	return pointlightGroup;
}

QWidget* fhLightEditor::CreateParallelLightParameters() {

	QGroupBox* group = new QGroupBox( tr( "Parallel Light Parameters" ) );;
	QGridLayout* grid = new QGridLayout;

	m_parallellightParameters.radius = new fhVec3Edit(true, this);
	m_parallellightParameters.radius->setMinimumValue(idVec3(0,0,0));
	m_parallellightParameters.radius->setPrecision(1);
	m_parallellightParameters.direction = new fhVec3Edit(this);
	m_parallellightParameters.direction->setPrecision(1);

	grid->addWidget( new QLabel( "Radius" ), 0, 0, Qt::AlignBottom );
	grid->addWidget( m_parallellightParameters.radius, 0, 1 );

	grid->addWidget( new QLabel( "Center" ), 1, 0, Qt::AlignBottom );
	grid->addWidget( m_parallellightParameters.direction, 1, 1 );

	group->setLayout( grid );

	return group;
}

QWidget* fhLightEditor::CreateProjectedLightParameters() {

	QGroupBox* group = new QGroupBox( tr( "Projected Light Parameters" ) );;
	QGridLayout* grid = new QGridLayout;

	m_projectedlightParameters.target = new fhVec3Edit(true, this);
	m_projectedlightParameters.right = new fhVec3Edit(this);
	m_projectedlightParameters.up = new fhVec3Edit(this);
	m_projectedlightParameters.start = new fhVec3Edit(true, this);
	m_projectedlightParameters.end = new fhVec3Edit(this);

	m_projectedlightParameters.target->setPrecision(1);
	m_projectedlightParameters.right->setPrecision(1);
	m_projectedlightParameters.up->setPrecision(1);
	m_projectedlightParameters.start->setPrecision(1);
	m_projectedlightParameters.end->setPrecision(1);

	grid->addWidget( new QLabel( "Target" ), 0, 0, Qt::AlignBottom );
	grid->addWidget( m_projectedlightParameters.target, 0, 1 );

	grid->addWidget( new QLabel( "Right" ), 1, 0, Qt::AlignBottom );
	grid->addWidget( m_projectedlightParameters.right, 1, 1 );

	grid->addWidget( new QLabel( "Up" ), 2, 0, Qt::AlignBottom );
	grid->addWidget( m_projectedlightParameters.up, 2, 1 );

	m_projectedlightParameters.explicitStartEnd = new QCheckBox;
	m_projectedlightParameters.explicitStartEnd->setText("Explicit Start/End");
	grid->addWidget(m_projectedlightParameters.explicitStartEnd, 3, 0, 1, 2);

	grid->addWidget( new QLabel( "Start" ), 4, 0, Qt::AlignBottom );
	grid->addWidget( m_projectedlightParameters.start, 4, 1 );

	grid->addWidget( new QLabel( "End" ), 5, 0, Qt::AlignBottom );
	grid->addWidget( m_projectedlightParameters.end, 5, 1 );

	group->setLayout( grid );

	QObject::connect(m_projectedlightParameters.explicitStartEnd, &QCheckBox::stateChanged, [=](){
		this->m_currentData.explicitStartEnd = this->m_projectedlightParameters.explicitStartEnd->isChecked();
		if(this->m_currentData.explicitStartEnd) {
			this->m_currentData.end = this->m_currentData.target;
			this->m_currentData.start = this->m_currentData.target.Normalized() * 8;
		}
		this->m_modified = true;
		this->UpdateLightParameters();
	} );

	return group;
}


void fhLightEditor::UpdateLightParameters() {
	const bool pointlight = (m_lighttype->currentIndex() == 0);
	const bool parallellight = (m_lighttype->currentIndex() == 1);
	const bool projectedlight = (m_lighttype->currentIndex() == 2);
	const bool explicitStartEnd = projectedlight && m_currentData.explicitStartEnd;

	m_pointlightParameters.center->setEnabled(pointlight);
	m_pointlightParameters.radius->setEnabled(pointlight);

	m_parallellightParameters.direction->setEnabled(parallellight);
	m_parallellightParameters.radius->setEnabled(parallellight);

	m_projectedlightParameters.target->setEnabled( projectedlight );
	m_projectedlightParameters.up->setEnabled( projectedlight );
	m_projectedlightParameters.right->setEnabled( projectedlight );
	m_projectedlightParameters.explicitStartEnd->setEnabled( projectedlight );
	m_projectedlightParameters.start->setEnabled( explicitStartEnd );
	m_projectedlightParameters.end->setEnabled( explicitStartEnd );
}


void fhLightEditor::Data::initFromSpawnArgs( const idDict* spawnArgs ) {
	if (spawnArgs->GetVector( "light_right", "-128 0 0", right )) {
		type = fhLightType::Projected;
	}
	else if (spawnArgs->GetInt( "parallel", "0" ) != 0) {
		type = fhLightType::Parallel;
	}
	else {
		type = fhLightType::Point;
	}

	explicitStartEnd = false;

	if(spawnArgs->GetVector("light_start", "0 0 0", start)) {
		explicitStartEnd = true;
	}

	if (spawnArgs->GetVector( "light_end", "0 0 0", end )) {
		explicitStartEnd = true;
	}

	target = spawnArgs->GetVector( "light_target", "0 0 -256" );
	up = spawnArgs->GetVector( "light_up", "0 -128 0" );
	radius = spawnArgs->GetVector( "light_radius", "100 100 100" );
	center = spawnArgs->GetVector( "light_center", "0 0 0" );
	color = spawnArgs->GetVector( "_color", "1 1 1" );

	shadowMode = shadowMode_t::Default;
	int shadowModeInt = 0;
	if(spawnArgs->GetInt("shadowMode", "0", shadowModeInt)) {
		shadowMode = static_cast<shadowMode_t>(shadowModeInt);
	} else if(spawnArgs->GetBool("noshadows", "0")) {
		shadowMode = shadowMode_t::NoShadows;
	}
	shadowBrightness = spawnArgs->GetFloat("shadowBrightness", "0.15");
	shadowSoftness = spawnArgs->GetFloat("shadowSoftness", "1");
	shadowPolygonOffsetBias = spawnArgs->GetFloat("shadowPolygonOffsetBias", "10");
	shadowPolygonOffsetFactor = spawnArgs->GetFloat("shadowPolygonOffsetFactor", "4");

	name = spawnArgs->GetString("name");
	classname = spawnArgs->GetString("classname");
	material = spawnArgs->GetString("texture");
}

void fhLightEditor::Data::toSpawnArgs(idDict* spawnArgs) {

	spawnArgs->Delete("light_right");
	spawnArgs->Delete("light_up");
	spawnArgs->Delete("light_target");
	spawnArgs->Delete("_color");
	spawnArgs->Delete("light_center");
	spawnArgs->Delete("light_radius");
	spawnArgs->Delete("light_start");
	spawnArgs->Delete("light_end");
	spawnArgs->Delete("parallel");
	spawnArgs->Delete("noshadows");
	spawnArgs->Delete("nospecular");
	spawnArgs->Delete("nodiffuse");
	spawnArgs->Delete("shadowMode");
	spawnArgs->Delete("shadowBrightness");
	spawnArgs->Delete("shadowSoftness");

	switch(type) {
	case fhLightType::Projected:
		spawnArgs->SetVector("light_right", right);
		spawnArgs->SetVector("light_up", up);
		spawnArgs->SetVector("light_target", target);
		if(explicitStartEnd) {
			spawnArgs->SetVector("light_start", start);
			spawnArgs->SetVector("light_end", end);
		}
		break;
	case fhLightType::Parallel:
		spawnArgs->SetInt("parallel", 1);
		spawnArgs->SetVector( "light_radius", radius );
		spawnArgs->SetVector( "light_center", center );
		break;
	case fhLightType::Point:
		spawnArgs->SetInt("parallel", 0);
		spawnArgs->SetVector( "light_radius", radius );
		spawnArgs->SetVector( "light_center", center );
		break;
	};

	spawnArgs->SetVector("_color", color);

	spawnArgs->SetInt("shadowMode", (int)shadowMode);
	if(shadowMode == shadowMode_t::ShadowMap) {
		spawnArgs->SetFloat("shadowSoftness", shadowSoftness);
		spawnArgs->SetFloat("shadowBrightness", shadowBrightness);
		spawnArgs->SetFloat("shadowPolygonOffsetBias", shadowPolygonOffsetBias);
		spawnArgs->SetFloat("shadowPolygonOffsetFactor", shadowPolygonOffsetFactor);
	}

	if(!material.IsEmpty()) {
		spawnArgs->Set("texture", material.c_str());
	}

	//TODO(johl): are expected to be there(?), but are not used by the game
	spawnArgs->SetBool("nospecular", false);
	spawnArgs->SetBool("nodiffuse", false);
	spawnArgs->SetFloat("falloff", 0.0f);
}

void fhLightEditor::initFromSpawnArgs(const idDict* spawnArgs) {
	if(!spawnArgs) {
		m_currentData.name.Empty();
		m_currentData.classname.Empty();
		this->setWindowTitle(QString("Light Editor: <no light selected>"));
		return;
	}

	m_currentData.initFromSpawnArgs(spawnArgs);
	m_originalData = m_currentData;
	m_modified = false;

	if(m_currentData.classname != "light") {
		this->setWindowTitle(QString("Light Editor: <no light selected>"));
		return;
	}

	m_lighttype->setCurrentIndex((int)m_currentData.type);
	m_pointlightParameters.radius->set(m_currentData.radius);
	m_pointlightParameters.center->set(m_currentData.center);
	m_parallellightParameters.radius->set( m_currentData.radius );
	m_parallellightParameters.direction->set( m_currentData.center );
	m_projectedlightParameters.target->set( m_currentData.target );
	m_projectedlightParameters.right->set( m_currentData.right );
	m_projectedlightParameters.up->set( m_currentData.up );
	m_projectedlightParameters.start->set( m_currentData.start );
	m_projectedlightParameters.end->set( m_currentData.end );
	m_projectedlightParameters.explicitStartEnd->setChecked( m_currentData.explicitStartEnd );
	m_material->setCurrentText(m_currentData.material.c_str());
	m_coloredit->set(m_currentData.color);

	const int shadowModeIndex = m_shadowMode->findData((int)m_currentData.shadowMode);
	if(shadowModeIndex >= 0) {
		m_shadowMode->setCurrentIndex(shadowModeIndex);
	}

	m_shadowBrightnessEdit->setValue(m_currentData.shadowBrightness);
	m_shadowSoftnessEdit->setValue(m_currentData.shadowSoftness);
	m_shadowOffsetBias->setValue(m_currentData.shadowPolygonOffsetBias);
	m_shadowOffsetFactor->setValue(m_currentData.shadowPolygonOffsetFactor);

	UpdateLightParameters();
	this->setWindowTitle(QString("Light Editor: %1").arg(m_currentData.name.c_str()));
}

void fhLightEditor::UpdateGame() {
	if(m_currentData.name.IsEmpty() || m_currentData.classname != "light")
		return;

	if(!com_editorActive)
	{
		//use ingame
		idEntity* list[128];
		int count = gameEdit->GetSelectedEntities( list, 128 );

		idDict newSpawnArgs;
		m_currentData.toSpawnArgs( &newSpawnArgs );

		for (int i = 0; i < count; ++i) {
			gameEdit->EntityChangeSpawnArgs( list[i], &newSpawnArgs );
			gameEdit->EntityUpdateChangeableSpawnArgs( list[i], NULL );
			gameEdit->EntityUpdateVisuals( list[i] );
		}
	}
	else
	{
		// used from Radiant
		for (brush_t *b = selected_brushes.next; b && b != &selected_brushes; b = b->next) {
			if ((b->owner->eclass->nShowFlags & ECLASS_LIGHT) && !b->entityModel) {
				m_currentData.toSpawnArgs(&b->owner->epairs);
				Brush_Build( b );
			}
		}
		Sys_UpdateWindows( W_ALL );
	}
}


void fhLightEditor::LoadMaterials() {
	m_material->clear();
	int count = declManager->GetNumDecls( DECL_MATERIAL );
	m_material->addItem("");
	for (int i = 0; i < count; i++) {
		const idMaterial *mat = declManager->MaterialByIndex( i, false );
		idStr str = mat->GetName();
		str.ToLower();
		if (str.Icmpn( "lights/", strlen( "lights/" ) ) == 0 || str.Icmpn( "fogs/", strlen( "fogs/" ) ) == 0) {
			m_material->addItem(str.c_str());
		}
	}
}

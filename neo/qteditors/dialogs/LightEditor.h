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
#include "../widgets/Vec3Edit.h"
#include "../widgets/RenderWidget.h"
#include <qdialog.h>
#include <qtimer.h>

class QCheckBox;
class QSlider;
class QComboBox;
class idGLDrawableMaterial;

class fhColorEdit;
class fhSliderNumEdit;

class fhLightEditor : public QDialog {
	Q_OBJECT

public:
	fhLightEditor(QWidget* parent);
	~fhLightEditor();

	void initFromSpawnArgs(const idDict* spawnArgs);

	virtual void closeEvent(QCloseEvent* event) override;

private:
	enum class fhLightType
	{
		Point = 0,
		Parallel = 1,
		Projected = 2
	};

	struct Data {
		idStr name;
		idStr classname;
		idStr material;
		fhLightType type;
		idVec3 radius;
		idVec3 center;
		idVec3 target;
		idVec3 up;
		idVec3 right;
		idVec3 start;
		idVec3 end;
		idVec3 color;
		bool explicitStartEnd;
		shadowMode_t shadowMode;
		float shadowSoftness;
		float shadowBrightness;
		float shadowPolygonOffsetFactor;
		float shadowPolygonOffsetBias;

		void initFromSpawnArgs( const idDict* spawnArgs );
		void toSpawnArgs(idDict* spawnArgs);
	};


	Data m_originalData;
	Data m_currentData;
	bool m_modified;
	QComboBox* m_lighttype;
	QComboBox* m_material;


	fhColorEdit* m_coloredit;
	QPushButton* m_cancelButton;
	QPushButton* m_applyButton;
	QPushButton* m_okButton;
	QComboBox* m_shadowMode;

	fhSliderNumEdit* m_shadowBrightnessEdit;
	fhSliderNumEdit* m_shadowSoftnessEdit;
	fhSliderNumEdit* m_shadowOffsetBias;
	fhSliderNumEdit* m_shadowOffsetFactor;

	idGLDrawableMaterial* m_drawableMaterial;
	QLabel* m_materialFile;

	struct PointLightParameters {
		fhVec3Edit* radius;
		fhVec3Edit* center;
	} m_pointlightParameters;

	struct ParallelLightParameters {
		fhVec3Edit* radius;
		fhVec3Edit* direction;
	} m_parallellightParameters;

	struct ProjectedLightParameters {
		fhVec3Edit* target;
		fhVec3Edit* right;
		fhVec3Edit* up;
		fhVec3Edit* start;
		fhVec3Edit* end;
		QCheckBox* explicitStartEnd;
	} m_projectedlightParameters;

	void UpdateLightParameters();
	void setLightColor(idVec3 color);
	void setLightColor(QColor color);
	void UpdateGame();

	QWidget* CreatePointLightParameters();
	QWidget* CreateParallelLightParameters();
	QWidget* CreateProjectedLightParameters();

	void LoadMaterials();

	QTimer m_timer;
};

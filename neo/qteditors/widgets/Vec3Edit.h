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

class fhNumEdit;

class fhVec3Edit : public QWidget {
	Q_OBJECT

public:
	explicit fhVec3Edit(bool labels, QWidget* parent);
	explicit fhVec3Edit(QWidget* parent);
	~fhVec3Edit();

	idVec3 get() const;
	void set(idVec3 v);

	float getX() const;
	float getY() const;
	float getZ() const;

	void setX(float value);
	void setY(float value);
	void setZ(float value);

	virtual QSize sizeHint() const override;

	void setMinimumValue(idVec3 v);
	void setMaximumValue(idVec3 v);
	void setStepSize(idVec3 v);
	void setStepSize(float f) {
		setStepSize(idVec3(f,f,f));
	}

	void setPrecision(int x, int y, int z);
	void setPrecision(int p) {
		setPrecision(p,p,p);
	}

signals:
	void valueChanged(idVec3 v);

private:
	fhNumEdit* m_xedit;
	fhNumEdit* m_yedit;
	fhNumEdit* m_zedit;
};
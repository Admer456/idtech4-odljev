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

#include "Vec3Edit.h"
#include "NumEdit.h"

fhVec3Edit::fhVec3Edit(bool labels, QWidget* parent)
: QWidget(parent) {
	m_xedit = new fhNumEdit(this);
	m_yedit = new fhNumEdit(this);
	m_zedit = new fhNumEdit(this);

	auto layout = new QGridLayout;
	layout->setSpacing(2);
	layout->setMargin(0);

	if(labels) {
		layout->addWidget(new QLabel("X"), 0, 0, Qt::AlignHCenter);
		layout->addWidget(new QLabel("Y"), 0, 1, Qt::AlignHCenter);
		layout->addWidget(new QLabel("Z"), 0, 2, Qt::AlignHCenter);
		layout->addWidget( m_xedit, 1, 0 );
		layout->addWidget( m_yedit, 1, 1 );
		layout->addWidget( m_zedit, 1, 2 );
	} else {
		layout->addWidget( m_xedit, 0, 0 );
		layout->addWidget( m_yedit, 0, 1 );
		layout->addWidget( m_zedit, 0, 2 );
	}

	QObject::connect(m_xedit, &fhNumEdit::valueChanged, [&]() {this->valueChanged(this->get()); });
	QObject::connect(m_yedit, &fhNumEdit::valueChanged, [&]() {this->valueChanged(this->get()); });
	QObject::connect(m_zedit, &fhNumEdit::valueChanged, [&]() {this->valueChanged(this->get()); });

	this->setLayout(layout);
	this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

fhVec3Edit::fhVec3Edit(QWidget* parent )
: fhVec3Edit( false, parent ) {
}

fhVec3Edit::~fhVec3Edit() {
}

idVec3 fhVec3Edit::get() const {
	return idVec3(getX(), getY(), getZ());
}

void fhVec3Edit::set( idVec3 v ) {
	setX(v.x);
	setY(v.y);
	setZ(v.z);
}

float fhVec3Edit::getX() const {
	return m_xedit->getFloat();
}

float fhVec3Edit::getY() const {
	return m_yedit->getFloat();
}

float fhVec3Edit::getZ() const {
	return m_zedit->getFloat();
}

void fhVec3Edit::setX( float value ) {
	m_xedit->setFloat(value);
}

void fhVec3Edit::setY( float value ) {
	m_yedit->setFloat(value);
}

void fhVec3Edit::setZ( float value ) {
	m_zedit->setFloat(value);
}

QSize fhVec3Edit::sizeHint() const {
	QSize size = QSize(m_xedit->sizeHint().width() * 3, m_xedit->sizeHint().height());
	return size;
}

void fhVec3Edit::setMaximumValue(idVec3 v) {
	m_xedit->setMaximumValue(v.x);
	m_yedit->setMaximumValue(v.y);
	m_zedit->setMaximumValue(v.z);
}

void fhVec3Edit::setMinimumValue( idVec3 v ) {
	m_xedit->setMinimumValue( v.x );
	m_yedit->setMinimumValue( v.y );
	m_zedit->setMinimumValue( v.z );
}

void fhVec3Edit::setStepSize(idVec3 v) {
	m_xedit->setAdjustStepSize(v.x);
	m_yedit->setAdjustStepSize(v.y);
	m_zedit->setAdjustStepSize(v.z);
}

void fhVec3Edit::setPrecision(int x, int y, int z) {
	m_xedit->setPrecision( x );
	m_yedit->setPrecision( y );
	m_zedit->setPrecision( z );
}
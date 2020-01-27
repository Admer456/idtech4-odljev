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

#include "NumEdit.h"
#include <qvalidator.h>

static const int maxDecimals = 3;
static const int defaultRepeatInterval = 20; //repeat every n milliseconds (lower means faster)

fhNumEdit::fhNumEdit(float from, float to, QWidget* parent) : QWidget(parent), m_step(1), m_precision(maxDecimals) {
	m_edit = new QLineEdit(this);
	m_down = new QPushButton("-", this);
	m_up = new QPushButton("+", this);
	m_validator = new QDoubleValidator(from, to, m_precision, this);
	m_validator->setNotation(QDoubleValidator::StandardNotation);

	const QSize updownSize(12,12);
	m_down->setMaximumSize(updownSize);
	m_up->setMaximumSize(updownSize);
	m_up->setAutoRepeat(true);
	m_down->setAutoRepeat(true);
	m_up->setAutoRepeatInterval(defaultRepeatInterval);
	m_down->setAutoRepeatInterval(defaultRepeatInterval);

	m_edit->setValidator(m_validator);

	auto vlayout = new QVBoxLayout;
	vlayout->setSpacing(0);
	vlayout->setMargin(0);
	vlayout->addWidget(m_up);
	vlayout->addWidget(m_down);

	auto hlayout = new QHBoxLayout;
	hlayout->setSpacing( 0 );
	hlayout->setMargin( 0 );
	hlayout->addWidget(m_edit);
	hlayout->addLayout(vlayout);

	this->setLayout(hlayout);
	this->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

	QObject::connect(m_up, &QPushButton::clicked, [=](){ float f = this->getFloat(); this->setFloat(f+m_step); });
	QObject::connect(m_down, &QPushButton::clicked, [=](){ float f = this->getFloat(); this->setFloat(f-m_step); });
	QObject::connect(m_edit, &QLineEdit::textChanged, [=](){this->valueChanged(this->getFloat());} );
}

fhNumEdit::fhNumEdit(QWidget* parent)
: fhNumEdit(-100000, 100000, parent) {
}

fhNumEdit::~fhNumEdit() {
}

void fhNumEdit::setAutoRepeatInterval( int intervalMs ) {
	m_up->setAutoRepeatInterval( intervalMs );
	m_down->setAutoRepeatInterval( intervalMs );
}

void fhNumEdit::setAdjustStepSize( float step ) {
	m_step = step;
}

float fhNumEdit::getFloat() const {
	return m_edit->text().toFloat();
}

int fhNumEdit::getInt() const {
	return m_edit->text().toInt();
}

void fhNumEdit::setInt( int v ) {
	setFloat(v);
}

void fhNumEdit::setFloat( float v ) {
	if ( v > m_validator->top() )
		v = m_validator->top();

	if (v < m_validator->bottom())
		v = m_validator->bottom();

	m_edit->setText( QString::number( v, 'f', m_precision ) );
}

QSize fhNumEdit::sizeHint() const {
	return QSize(60, m_edit->sizeHint().height());
}

void fhNumEdit::setMaximumValue(float f) {
	m_validator->setRange(m_validator->bottom(), f, m_precision);
	m_edit->setValidator(m_validator);
}

void fhNumEdit::setMinimumValue( float f ) {
	m_validator->setRange( f, m_validator->top(), m_precision );
	m_edit->setValidator(m_validator);
}

void fhNumEdit::setPrecision(int p) {
	m_precision = p;
	m_validator->setRange( m_validator->bottom(), m_validator->top(), m_precision );
	m_edit->setValidator( m_validator );
}

float fhNumEdit::maximumValue() const {
	return static_cast<float>(m_validator->top());
}

float fhNumEdit::minimumValue() const {
	return static_cast<float>(m_validator->bottom());
}
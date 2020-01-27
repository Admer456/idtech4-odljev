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

/*
 * For programming purposes, OpenGL matrices are 16-value arrays with base
 * vectors laid out contiguously in memory. The translation components occupy
 * the 13th, 14th, and 15th elements of the 16-element matrix, where indices are
 * numbered from 1 to 16 as described in section 2.11.2 of the OpenGL 2.1
 * Specification.
 *
 **/
class fhRenderMatrix final {
public:
	fhRenderMatrix() {
		memset(m, 0, sizeof(m));
		m[0] = 1.0f;
		m[5] = 1.0f;
		m[10] = 1.0f;
		m[15] = 1.0f;
	}

	explicit fhRenderMatrix(const float* arr) {
		memcpy(m, arr, sizeof(m));
	}

	~fhRenderMatrix() = default;

	fhRenderMatrix(const fhRenderMatrix& rhs) {
		memcpy(m, rhs.m, sizeof(m));
	}

	const fhRenderMatrix& operator=(const fhRenderMatrix& rhs) {
		memcpy(m, rhs.m, sizeof(m));
		return *this;
	}

	fhRenderMatrix operator*(const fhRenderMatrix& rhs) const {
		fhRenderMatrix ret;

		const float* l = ToFloatPtr();
		const float* r = rhs.ToFloatPtr();
		ret[0] = l[0]*r[0] + l[4]*r[1] + l[8]*r[2] + l[12]*r[3];
		ret[1] = l[1]*r[0] + l[5]*r[1] + l[9]*r[2] + l[13]*r[3];
		ret[2] = l[2]*r[0] + l[6]*r[1] + l[10]*r[2] + l[14]*r[3];
		ret[3] = l[3]*r[0] + l[7]*r[1] + l[11]*r[2] + l[15]*r[3];

		ret[4] = l[0] * r[4] + l[4] * r[5] + l[8] * r[6] + l[12] * r[7];
		ret[5] = l[1] * r[4] + l[5] * r[5] + l[9] * r[6] + l[13] * r[7];
		ret[6] = l[2] * r[4] + l[6] * r[5] + l[10] * r[6] + l[14] * r[7];
		ret[7] = l[3] * r[4] + l[7] * r[5] + l[11] * r[6] + l[15] * r[7];

		ret[8] = l[0] * r[8] + l[4] * r[9] + l[8] * r[10] + l[12] * r[11];
		ret[9] = l[1] * r[8] + l[5] * r[9] + l[9] * r[10] + l[13] * r[11];
		ret[10] = l[2] * r[8] + l[6] * r[9] + l[10] * r[10] + l[14] * r[11];
		ret[11] = l[3] * r[8] + l[7] * r[9] + l[11] * r[10] + l[15] * r[11];

		ret[12] = l[0] * r[12] + l[4] * r[13] + l[8] * r[14] + l[12] * r[15];
		ret[13] = l[1] * r[12] + l[5] * r[13] + l[9] * r[14] + l[13] * r[15];
		ret[14] = l[2] * r[12] + l[6] * r[13] + l[10] * r[14] + l[14] * r[15];
		ret[15] = l[3] * r[12] + l[7] * r[13] + l[11] * r[14] + l[15] * r[15];

		return ret;
	}

	idVec3 operator*(const idVec3& v) const {
		idVec3 ret;
		ret.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12];
		ret.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13];
		ret.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14];
		return ret;
	}

	idVec4 operator*(const idVec4 v) const {
		idVec4 ret;
		ret.x = m[0] * v.x + m[4] * v.y + m[8] * v.z + m[12] * v.w;
		ret.y = m[1] * v.x + m[5] * v.y + m[9] * v.z + m[13] * v.w;
		ret.z = m[2] * v.x + m[6] * v.y + m[10] * v.z + m[14] * v.w;
		ret.w = m[3] * v.x + m[7] * v.y + m[11] * v.z + m[15] * v.w;
		return ret;
	}

	const float& operator[](int index) const {
		assert(index >= 0);
		assert(index < 16);
		return m[index];
	}

	float& operator[]( int index ) {
		assert( index >= 0 );
		assert( index < 16 );
		return m[index];
	}

	const float* ToFloatPtr() const	{
		return &m[0];
	}

	float* ToFloatPtr()	{
		return &m[0];
	}

	static fhRenderMatrix CreateProjectionMatrix(float fov, float aspect, float nearClip, float farClip);
	static fhRenderMatrix CreateInfiniteProjectionMatrix( float fov, float aspect, float nearClip );
	static fhRenderMatrix CreateLookAtMatrix(const idVec3& viewOrigin, const idVec3& at, const idVec3& up);
	static fhRenderMatrix CreateLookAtMatrix( const idVec3& dir, const idVec3& up );
	static fhRenderMatrix CreateViewMatrix( const idVec3& origin );
	static fhRenderMatrix FlipMatrix();
	static fhRenderMatrix CreateOrthographicMatrix(float left, float right, float bottom, float top, float nearClip, float farClip) {
		const float a = 2.0f / (right - left);
		const float b = 2.0f/(top - bottom);
		const float c = -2.0f/(farClip - nearClip);
		const float d = -((right + left)/(right - left));
		const float e = -((top + bottom)/(top - bottom));
		const float f = -((farClip + nearClip)/(farClip - nearClip));

		const float m[] = {
			a, 0, 0, 0,
			0, b, 0, 0,
			0, 0, c, 0,
			d, e, f, 1
		};

		return fhRenderMatrix(m);
	}

	static idVec3 OpenGL2Doom( const idVec3& v ) {
#if 0
		static float flipMatrix[16] = {
			// convert from our coordinate system (looking down X)
			// to OpenGL's coordinate system (looking down -Z)
			0, -1, 0, 0,
			0, 0, 1, 0,
			-1, 0, 0, 0,
			0, 0, 0, 1
		};

		static const fhRenderMatrix m( flipMatrix );

		return m * v;
#endif
		return idVec3( -v.z, -v.x, v.y );
	}

	static const fhRenderMatrix identity;
private:
	float m[16];
};
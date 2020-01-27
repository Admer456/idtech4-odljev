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
#include <type_traits>


#pragma once
#include "../idlib/containers/ArrayRef.h"
#include <type_traits>

template<typename T>
class fhRenderList {
public:
	fhRenderList()
		: array(nullptr)
		, capacity(0)
		, size(0) {
	}

	fhRenderList(const fhRenderList&) = delete;
	fhRenderList(fhRenderList&& rl)
		: array(nullptr)
		, capacity(0)
		, size(0) {
		(*this) = std::move(rl);
	}

	const fhRenderList& operator=(const fhRenderList&) = delete;
	const fhRenderList& operator=(fhRenderList&& rl) {
		std::swap(array, rl.array);
		std::swap(capacity, rl.capacity);
		std::swap(size, rl.size);
		return *this;
	}

	void Append(const T& t) {
		if (size == capacity) {
			capacity += 32;
			T* t = R_FrameAllocT<T>(capacity);

			if (size > 0) {
				memcpy(t, array, sizeof(T) * size);
			}

			array = t;
		}

		assert(size < capacity);
		array[size] = t;
		++size;
	}

	void Clear() {
		size = 0;
	}

	int Num() const {
		return size;
	}

	bool IsEmpty() const {
		return Num() == 0;
	}

	const T& operator[](int i) const {
		assert(i < size);
		return array[i];
	}

	using iterator = T * ;
	using const_iterator = const T*;
	iterator begin() { return array; }
	const_iterator begin() const { return array; }
	const_iterator cbegin() const { return array; }
	iterator end() { return array + size; }
	const_iterator end() const { return array + size; }
	const_iterator cend() const { return array + size; }

private:
	T * array;
	int capacity;
	int size;
};

struct drawDepth_t {
	const drawSurf_t*  surf;
	idImage*           texture;
	idVec4             textureMatrix[2];
	idVec4             color;
	float              polygonOffset;
	bool               isSubView;
	float              alphaTestThreshold;
};

struct drawShadow_t {
	const srfTriangles_t* tris;
	const idRenderEntityLocal* entity;
	idImage*           texture;
	idVec4             textureMatrix[2];
	bool               hasTextureMatrix;
	float              alphaTestThreshold;
	unsigned           visibleFlags;
};

struct drawStage_t {
	const drawSurf_t* surf;
	const fhRenderProgram*  program;
	idImage*          textures[4];
	idCinematic*      cinematic;
	float             textureMatrix[16];
	bool              hasBumpMatrix;
	idVec4            bumpMatrix[2];
	depthBlendMode_t  depthBlendMode;
	float             depthBlendRange;
	stageVertexColor_t vertexColor;
	float             polygonOffset;
	idVec4            localViewOrigin;
	idVec4            diffuseColor;
	cullType_t        cullType;
	int               drawStateBits;
	idVec4            shaderparms[4];
	int               numShaderparms;
	fhVertexLayout    vertexLayout;
};

static_assert(std::is_trivial<drawDepth_t>::value, "must be trivial");
static_assert(std::is_trivial<drawShadow_t>::value, "must be trivial");
static_assert(std::is_trivial<drawStage_t>::value, "must be trivial");
static_assert(std::is_trivial<drawInteraction_t>::value, "must be trivial");

class DepthRenderList : public fhRenderList<drawDepth_t> {
public:
	void AddDrawSurfaces( drawSurf_t **surf, int numDrawSurfs );
	void Submit();
};

using StageRenderList = fhRenderList<drawStage_t>;

class InteractionList : public fhRenderList<drawInteraction_t> {
public:
	void AddDrawSurfacesOnLight(const viewLight_t& vLight, const drawSurf_t *surf);
	void Submit(const viewLight_t& vLight);
};

class ShadowRenderList : public fhRenderList<drawShadow_t> {
public:
	ShadowRenderList();
	void AddInteractions( viewLight_t* vlight, const shadowMapFrustum_t* shadowFrustrums, int numShadowFrustrums );
	void Submit( const float* shadowViewMatrix, const float* shadowProjectionMatrix, int side, int lod ) const;
private:
	void AddSurfaceInteraction( const idRenderEntityLocal *entityDef, const srfTriangles_t *tri, const idMaterial* material, unsigned visibleSides );
	idRenderEntityLocal dummy;
};

int  RB_GLSL_CreateStageRenderList( drawSurf_t **drawSurfs, int numDrawSurfs, StageRenderList& renderlist, int maxSort );
void RB_GLSL_SubmitStageRenderList( const StageRenderList& renderlist );
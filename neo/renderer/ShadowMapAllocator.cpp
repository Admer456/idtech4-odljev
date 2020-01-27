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

#include "ShadowMapAllocator.h"

fhShadowMapAllocator shadowMapAllocator;

static const size_t shadowMapSizes[] {
	4096,
	2048,
	1024,
	512,
	256
};

fhShadowMapAllocator::fhShadowMapAllocator() {
	int size = 1;
	for (int i = 0; i < (int)ShadowMapSize::NUM; ++i) {
		freelist[i].AssureSize( size );
		size *= 4;
	}

	FreeAll();
}

fhShadowMapAllocator::~fhShadowMapAllocator()	{
}


bool fhShadowMapAllocator::Allocate( int lod, int num, shadowCoord_t* coords ) {
	switch (lod) {
	case 0:
		return Allocate( ShadowMapSize::SM1024, num, coords );
	case 1:
		return Allocate( ShadowMapSize::SM512, num, coords );
	case 2:
	default:
		return Allocate( ShadowMapSize::SM256, num, coords );
	}
}

void fhShadowMapAllocator::FreeAll() {
	for (int i = 0; i < (int)ShadowMapSize::NUM; ++i) {
		freelist[i].SetNum( 0 );
	}

	freelist[0].Append( shadowCoord_t{ idVec2( 1, 1 ), idVec2( 0, 0 ) } );
}


bool fhShadowMapAllocator::Allocate( ShadowMapSize size, int num, shadowCoord_t* coords ) {
	for (int i = 0; i < num; ++i) {
		if (!Allocate( size, coords[i] )) {
			return false;
		}
	}

	return true;
}


bool fhShadowMapAllocator::Allocate( ShadowMapSize size, shadowCoord_t& coords ) {
	if (!Make( (int)size )) {
		return false;
	}

	idList<shadowCoord_t>& level = freelist[(int)size];
	coords = level[level.Num() - 1];
	level.RemoveIndex( level.Num() - 1 );
	return true;
}

bool fhShadowMapAllocator::Make( int sizeIndex ) {
	if (freelist[sizeIndex].Num() > 0) {
		return true;
	}

	if (sizeIndex == 0 || !Make( sizeIndex - 1 )) {
		return false;
	}

	idList<shadowCoord_t>& thisLevel = freelist[sizeIndex];
	idList<shadowCoord_t>& prevLevel = freelist[sizeIndex - 1];

	//take entry from upper level
	shadowCoord_t oldCoords = prevLevel[prevLevel.Num() - 1];
	prevLevel.RemoveIndex( prevLevel.Num() - 1 );

	//create 4 new entries in this level
	const idVec2 scale( (float)shadowMapSizes[sizeIndex] / (float)shadowMapSizes[0], (float)shadowMapSizes[sizeIndex] / (float)shadowMapSizes[0] );

	shadowCoord_t newCoords[4];

	newCoords[0].scale = scale;
	newCoords[0].offset = oldCoords.offset + idVec2( 0, 0 );

	newCoords[1].scale = scale;
	newCoords[1].offset = oldCoords.offset + idVec2( scale.x, 0 );

	newCoords[2].scale = scale;
	newCoords[2].offset = oldCoords.offset + idVec2( 0, scale.y );

	newCoords[3].scale = scale;
	newCoords[3].offset = oldCoords.offset + idVec2( scale.x, scale.y );

	for (int i = 0; i < 4; ++i) {
		thisLevel.Append( newCoords[i] );
	}

	return true;
}

void fhShadowMapAllocator::Split( const shadowCoord_t& src, shadowCoord_t* dst, idVec2 scale, float size ) {

	dst[0].scale = scale;
	dst[0].offset = src.offset + idVec2( 0, 0 );

	dst[1].scale = scale;
	dst[1].offset = src.offset + idVec2( size, 0 );

	dst[2].scale = scale;
	dst[2].offset = src.offset + idVec2( 0, size );

	dst[3].scale = scale;
	dst[3].offset = src.offset + idVec2( size, size );
}
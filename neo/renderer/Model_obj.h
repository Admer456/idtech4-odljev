/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
Copyright (C) 2016 Johannes Ohlemacher

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
#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#include "Model_local.h"

struct objVertex_t {
	int32 xyz;
	int32 normal;
	int32 st;

	bool operator==(const objVertex_t& rhs) const {
		return xyz == rhs.xyz && normal == rhs.normal && st == rhs.st;
	}

	bool operator!=(const objVertex_t& rhs) const {
		return !(*this == rhs);
	}
};

struct objFace_t {
	objVertex_t vertex[3];
};

struct objSurface_t {
	idStr material;
	idList<objFace_t> faces;
};

struct objModel_t {
	idList<idVec3> xyz;
	idList<idVec3> normal;
	idList<idVec2> st;
	idList<objSurface_t> surface;
	ID_TIME_T timeStamp;
};

objModel_t *OBJ_Load( const char *fileName );
void		OBJ_Free( objModel_t *obj );

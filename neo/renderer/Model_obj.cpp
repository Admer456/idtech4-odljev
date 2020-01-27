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
#include "Model_obj.h"
#include "idlib/StrRef.h"

static idVec3 ParseVec3(fhStrRef s) {
	idVec3 ret;

	s = s.TrimmedLeft();
	ret.x = atof( s.Ptr() );

	while(!s.IsEmpty() && s[0] != ' ') {
		++s;
	}
	s = s.TrimmedLeft();

	ret.y = atof( s.Ptr() );

	while (!s.IsEmpty() && s[0] != ' ') {
		++s;
	}

	s = s.TrimmedLeft();
	ret.z = atof( s.Ptr() );

	return ret;
}

static idVec2 ParseVec2( fhStrRef s ) {
	idVec2 ret;

	s = s.TrimmedLeft();
	ret.x = atof( s.Ptr() );

	while (!s.IsEmpty() && s[0] != ' ') {
		++s;
	}
	s = s.TrimmedLeft();

	ret.y = atof( s.Ptr() );

	return ret;
}

static objVertex_t ParseVertex( fhStrRef s ) {
	objVertex_t ret;

	s = s.TrimmedLeft();
	ret.xyz = atoi( s.Ptr() );

	while (!s.IsEmpty() && s[0] != '/') {
		++s;
	}
	++s;

	ret.st = atoi( s.Ptr() );

	while (!s.IsEmpty() && s[0] != '/') {
		++s;
	}
	++s;

	ret.normal = atoi( s.Ptr() );

	return ret;
}

static objFace_t ParseFace( fhStrRef s ) {
	objFace_t face;

	s = s.TrimmedLeft();

	face.vertex[0] = ParseVertex(s);

	while (!s.IsEmpty() && s[0] != ' ') {
		++s;
	}
	++s;

	face.vertex[1] = ParseVertex(s);

	while (!s.IsEmpty() && s[0] != ' ') {
		++s;
	}
	++s;

	face.vertex[2] = ParseVertex( s );

	return face;
}

static void MakeIndicesAbsolute(objVertex_t& vertex, const objModel_t* model)
{
	if(vertex.xyz < 0) {
		vertex.xyz += model->xyz.Num();
		vertex.xyz += 1;
	}

	if (vertex.normal < 0) {
		vertex.normal += model->normal.Num();
		vertex.normal += 1;
	}

	if (vertex.st < 0) {
		vertex.st += model->st.Num();
		vertex.st += 1;
	}
}

static fhStrRef OBJ_ParseLine(fhStrRef line, objModel_t* model) {
	assert(model);

	line = line.Trimmed();

	if(line.StartsWith("v ")) {
		idVec3 xyz = ParseVec3(line.Substr(2));
		model->xyz.Append(xyz);
	}
	else if(line.StartsWith("vt ")) {
		idVec2 st = ParseVec2( line.Substr( 3 ) );
		st.y = 1.0f - st.y;
		model->st.Append( st );
	}
	else if (line.StartsWith( "vn " )) {
		idVec3 normal = ParseVec3(line.Substr(3));
		model->normal.Append( normal );
	}
	else if (line.StartsWith( "usemtl " )) {
		line = line.Substr(7).Trimmed();
		objSurface_t surface;

		fhStrRef to = line;
		while(!to.IsEmpty() && !idStr::CharIsNewLine(to[0]) && idStr::CharIsPrintable(to[0])) {
			++to;
		}

		idStr name = fhStrRef( line.Ptr(), int( (std::ptrdiff_t) to.Ptr() - (std::ptrdiff_t)line.Ptr() ) ).ToString();

		for(int i=0; i<model->surface.Num(); ++i) {
			if(model->surface[i].material == name) {
				surface = model->surface[i];
				model->surface.RemoveIndex(i);
				break;
			}
		}

		surface.material = name;
		model->surface.Append(surface);
//		model->surface.Last().faces.Resize(4096, 4096);
//		model->surface.Last().faces.SetNum(0, false);
	}
	else if (line.StartsWith( "f " )) {
		objFace_t face = ParseFace(line.Substr(2));

		MakeIndicesAbsolute(face.vertex[0], model);
		MakeIndicesAbsolute(face.vertex[1], model);
		MakeIndicesAbsolute(face.vertex[2], model);

		if(model->surface.Num() == 0) {
			objSurface_t defSurface;
			defSurface.material = "_default";
			model->surface.Append(defSurface);
		}

		model->surface.Last().faces.Append(face);
	}

	while(line && !line.StartsWith("\n")) {
		++line;
	}

	return ++line;
}

static objModel_t* OBJ_Parse(fhStrRef src) {
	objModel_t* model = new objModel_t;
	model->st.SetGranularity(1024);
	model->xyz.SetGranularity(1024);
	model->normal.SetGranularity(1024);

	model->st.Resize(1000 * 300);
	model->xyz.Resize(1000 * 300);
	model->normal.Resize(1000 * 300);
	model->st.SetNum(0, false);
	model->xyz.SetNum(0, false);
	model->normal.SetNum(0, false);

	while(!src.IsEmpty()) {
		src = OBJ_ParseLine(src, model);
	}

	return model;
}

objModel_t *OBJ_Load( const char *fileName ) {
	char *buf = nullptr;
	ID_TIME_T timeStamp = 0;
	objModel_t *obj = nullptr;

	fileSystem->ReadFile( fileName, (void **)&buf, &timeStamp );
	if (!buf) {
		return nullptr;
	}

	obj = OBJ_Parse( fhStrRef(buf) );
	obj->timeStamp = timeStamp;

	fileSystem->FreeFile( buf );

	return obj;
}

void OBJ_Free( objModel_t *obj ) {
	delete obj;
}

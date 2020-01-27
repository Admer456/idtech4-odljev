/*
===========================================================================

Doom 3 GPL Source Code
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

#include "ImageProgram.h"
#include "ImageData.h"
#include "tr_local.h"

/*
=================
R_HeightmapToNormalMap

it is not possible to convert a heightmap into a normal map
properly without knowing the texture coordinate stretching.
We can assume constant and equal ST vectors for walls, but not for characters.
=================
*/
static void R_HeightmapToNormalMap( byte *data, int width, int height, float scale ) {
	int		i, j;
	byte	*depth;

	scale = scale / 256;

	// copy and convert to grey scale
	j = width * height;
	depth = (byte *)R_StaticAlloc( j );
	for (i = 0; i < j; i++) {
		depth[i] = (data[i * 4] + data[i * 4 + 1] + data[i * 4 + 2]) / 3;
	}

	idVec3	dir, dir2;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			int		d1, d2, d3, d4;
			int		a1, a2, a3, a4;

			// FIXME: look at five points?

			// look at three points to estimate the gradient
			a1 = d1 = depth[(i * width + j)];
			a2 = d2 = depth[(i * width + ((j + 1) & (width - 1)))];
			a3 = d3 = depth[(((i + 1) & (height - 1)) * width + j)];
			a4 = d4 = depth[(((i + 1) & (height - 1)) * width + ((j + 1) & (width - 1)))];

			d2 -= d1;
			d3 -= d1;

			dir[0] = -d2 * scale;
			dir[1] = -d3 * scale;
			dir[2] = 1;
			dir.NormalizeFast();

			a1 -= a3;
			a4 -= a3;

			dir2[0] = -a4 * scale;
			dir2[1] = a1 * scale;
			dir2[2] = 1;
			dir2.NormalizeFast();

			dir += dir2;
			dir.NormalizeFast();

			a1 = (i * width + j) * 4;
			data[a1 + 0] = (byte)(dir[0] * 127 + 128);
			data[a1 + 1] = (byte)(dir[1] * 127 + 128);
			data[a1 + 2] = (byte)(dir[2] * 127 + 128);
			data[a1 + 3] = 255;
		}
	}


	R_StaticFree( depth );
}


/*
=================
R_ImageScale
=================
*/
static void R_ImageScale( byte *data, int width, int height, float scale[4] ) {
	int		i, j;
	int		c;

	c = width * height * 4;

	for (i = 0; i < c; i++) {
		j = (byte)(data[i] * scale[i & 3]);
		if (j < 0) {
			j = 0;
		}
		else if (j > 255) {
			j = 255;
		}
		data[i] = j;
	}
}

/*
=================
R_InvertAlpha
=================
*/
static void R_InvertAlpha( byte *data, int width, int height ) {
	int		i;
	int		c;

	c = width * height * 4;

	for (i = 0; i < c; i += 4) {
		data[i + 3] = 255 - data[i + 3];
	}
}

/*
=================
R_InvertColor
=================
*/
static void R_InvertColor( byte *data, int width, int height ) {
	int		i;
	int		c;

	c = width * height * 4;

	for (i = 0; i < c; i += 4) {
		data[i + 0] = 255 - data[i + 0];
		data[i + 1] = 255 - data[i + 1];
		data[i + 2] = 255 - data[i + 2];
	}
}


/*
===================
R_AddNormalMaps

===================
*/
static void R_AddNormalMaps( byte *data1, int width1, int height1, byte *data2, int width2, int height2 ) {
	int		i, j;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if (width2 != width1 || height2 != height1) {
		newMap = R_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	}
	else {
		newMap = NULL;
	}

	// add the normal change from the second and renormalize
	for (i = 0; i < height1; i++) {
		for (j = 0; j < width1; j++) {
			byte	*d1, *d2;
			idVec3	n;
			float   len;

			d1 = data1 + (i * width1 + j) * 4;
			d2 = data2 + (i * width1 + j) * 4;

			n[0] = (d1[0] - 128) / 127.0;
			n[1] = (d1[1] - 128) / 127.0;
			n[2] = (d1[2] - 128) / 127.0;

			// There are some normal maps that blend to 0,0,0 at the edges
			// this screws up compression, so we try to correct that here by instead fading it to 0,0,1
			len = n.LengthFast();
			if (len < 1.0f) {
				n[2] = idMath::Sqrt( 1.0 - (n[0] * n[0]) - (n[1] * n[1]) );
			}

			n[0] += (d2[0] - 128) / 127.0;
			n[1] += (d2[1] - 128) / 127.0;
			n.Normalize();

			d1[0] = (byte)(n[0] * 127 + 128);
			d1[1] = (byte)(n[1] * 127 + 128);
			d1[2] = (byte)(n[2] * 127 + 128);
			d1[3] = 255;
		}
	}

	if (newMap) {
		R_StaticFree( newMap );
	}
}

/*
================
R_SmoothNormalMap
================
*/
static void R_SmoothNormalMap( byte *data, int width, int height ) {
	byte	*orig;
	int		i, j, k, l;
	idVec3	normal;
	byte	*out;
	static float	factors[3][3] = {
		{ 1, 1, 1 },
		{ 1, 1, 1 },
		{ 1, 1, 1 }
	};

	orig = (byte *)R_StaticAlloc( width * height * 4 );
	memcpy( orig, data, width * height * 4 );

	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			normal = vec3_origin;
			for (k = -1; k < 2; k++) {
				for (l = -1; l < 2; l++) {
					byte	*in;

					in = orig + (((j + l)&(height - 1))*width + ((i + k)&(width - 1))) * 4;

					// ignore 000 and -1 -1 -1
					if (in[0] == 0 && in[1] == 0 && in[2] == 0) {
						continue;
					}
					if (in[0] == 128 && in[1] == 128 && in[2] == 128) {
						continue;
					}

					normal[0] += factors[k + 1][l + 1] * (in[0] - 128);
					normal[1] += factors[k + 1][l + 1] * (in[1] - 128);
					normal[2] += factors[k + 1][l + 1] * (in[2] - 128);
				}
			}
			normal.Normalize();
			out = data + (j * width + i) * 4;
			out[0] = (byte)(128 + 127 * normal[0]);
			out[1] = (byte)(128 + 127 * normal[1]);
			out[2] = (byte)(128 + 127 * normal[2]);
		}
	}

	R_StaticFree( orig );
}


/*
===================
R_ImageAdd

===================
*/
static void R_ImageAdd( byte *data1, int width1, int height1, byte *data2, int width2, int height2 ) {
	int		i, j;
	int		c;
	byte	*newMap;

	// resample pic2 to the same size as pic1
	if (width2 != width1 || height2 != height1) {
		newMap = R_Dropsample( data2, width2, height2, width1, height1 );
		data2 = newMap;
	}
	else {
		newMap = NULL;
	}


	c = width1 * height1 * 4;

	for (i = 0; i < c; i++) {
		j = data1[i] + data2[i];
		if (j > 255) {
			j = 255;
		}
		data1[i] = j;
	}

	if (newMap) {
		R_StaticFree( newMap );
	}
}

static const char* axisSides[] = {
	"_ny.tga",
	"_py.tga",
	"_pz.tga",
	"_nz.tga",
	"_nx.tga",
	"_px.tga",
};

static const char* cameraSides[] = {
	"_right.tga",
	"_left.tga",
	"_up.tga",
	"_down.tga",
	"_back.tga",
	"_forward.tga"
};

/*
===================
Load 6 individual cube map files and assemble single cubemap image.
'filename' is the common prefix of all files for that cube map
e.g. 'foo' for 'foo_forward','foo_back', etc.
===================
*/
static bool R_LoadCubeMap( const char* filename, cubeFiles_t cubeFiles, fhImageData* data, ID_TIME_T* timestamp ) {

	assert( cubeFiles == CF_CAMERA || cubeFiles == CF_NATIVE );

	const char	**sides = (cubeFiles == CF_CAMERA) ? cameraSides : axisSides;

	fhImageData images[6];

	for (int i = 0; i < 6; i++) {
		char	fullName[MAX_IMAGE_NAME];
		idStr::snPrintf( fullName, sizeof( fullName ), "%s%s", filename, sides[i] );

		ID_TIME_T time = 0;

		if (!fhImageData::LoadFile( fullName, (!data ? nullptr : &images[i]), false, &time )) {
			common->Warning( "failed to load cube map file: %s", fullName );
			return false;
		}

		if (data) {
			const int size = images[i].GetWidth();

			if (images[i].GetHeight() != size){
				common->Warning( "cube image not quadratic '%s'", fullName );
				return false;
			}

			//convert coordinate system/orientation of cubemaps to match DDS files.
			//This way we can use one skybox shader for all cubemaps (Doom3's own cubemaps and DDS files).
			if (cubeFiles == CF_CAMERA) {
				switch (i) {
				case 0:	// right
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				case 1:	// left
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				case 2:	// up
					R_VerticalFlip( images[i].GetData(), size, size );
					break;
				case 3:	// down
					R_VerticalFlip( images[i].GetData(), size, size );
					break;
				case 4:	// forward
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				case 5: // back
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				}
			}
			else {
				switch (i) {
				case 0:	// right
					break;
				case 1:	// left
					R_VerticalFlip( images[i].GetData(), size, size );
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				case 2:	// up
					R_RotatePic( images[i].GetData(), size );
					R_VerticalFlip( images[i].GetData(), size, size );
					break;
				case 3:	// down
					R_HorizontalFlip( images[i].GetData(), size, size );
					R_RotatePic( images[i].GetData(), size );
					break;
				case 4:	// forward
					R_RotatePic( images[i].GetData(), size );
					R_VerticalFlip( images[i].GetData(), size, size );
					break;
				case 5: // back
					R_RotatePic( images[i].GetData(), size );
					R_HorizontalFlip( images[i].GetData(), size, size );
					break;
				}
			}
		}

		if (timestamp && *timestamp < time) {
			*timestamp = time;
		}
	}

	if (data) {
		return data->LoadCubeMap( images, filename );
	}

	return true;
}

/*
===================
AppendToken
===================
*/
void fhImageProgram::AppendToken( idToken &token ) {
	// add a leading space if not at the beginning
	if (parseBuffer[0]) {
		idStr::Append( parseBuffer, MAX_IMAGE_NAME, " " );
	}
	idStr::Append( parseBuffer, MAX_IMAGE_NAME, token.c_str() );
}

/*
===================
MatchAndAppendToken
===================
*/
void fhImageProgram::MatchAndAppendToken( idLexer &src, const char *match ) {
	if (!src.ExpectTokenString( match )) {
		return;
	}
	// a matched token won't need a leading space
	idStr::Append( parseBuffer, MAX_IMAGE_NAME, match );
}

/*
===================
R_ParseImageProgram_r

If pic is NULL, the timestamps will be filled in, but no image will be generated
If both pic and timestamps are NULL, it will just advance past it, which can be
used to parse an image program from a text stream.
===================
*/
const char* fhImageProgram::ParsePastImageProgram( idLexer &src ) {
	this->parseBuffer[0] = '\0';

	ParseImageProgram_r( src, false, nullptr, nullptr );
	return parseBuffer;
}

bool fhImageProgram::ParseImageProgram_r( idLexer &src, bool toRgba, fhImageData* imageData, ID_TIME_T* timestamp ) {
	idToken		token;

	src.ReadToken( &token );
	AppendToken( token );

	if (!token.Icmp( "heightmap" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		MatchAndAppendToken( src, "," );

		src.ReadToken( &token );
		AppendToken( token );
		float scale = token.GetFloatValue();

		// process it
		if (imageData) {
			R_HeightmapToNormalMap( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight(), scale );
			//depth = TD_BUMP;
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "addnormals" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		MatchAndAppendToken( src, "," );

		fhImageData pic2;
		if (!ParseImageProgram_r( src, true, &pic2, timestamp )) {
			return false;
		}

		// process it
		if (imageData) {
			R_AddNormalMaps( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight(), pic2.GetData(), pic2.GetWidth(), pic2.GetHeight() );
			//depth = TD_BUMP;
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "smoothnormals" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		if (imageData) {
			R_SmoothNormalMap( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight() );
			//depth = TD_BUMP;
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "add" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		MatchAndAppendToken( src, "," );

		fhImageData pic2;
		if (!ParseImageProgram_r( src, true, &pic2, timestamp )) {
			return false;
		}

		// process it
		if (imageData) {
			R_ImageAdd( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight(), pic2.GetData(), pic2.GetWidth(), pic2.GetHeight() );
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "scale" )) {
		float	scale[4];
		int		i;

		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		for (i = 0; i < 4; i++) {
			MatchAndAppendToken( src, "," );
			src.ReadToken( &token );
			AppendToken( token );
			scale[i] = token.GetFloatValue();
		}

		// process it
		if (imageData) {
			R_ImageScale( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight(), scale );
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "invertAlpha" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		// process it
		if (imageData) {
			R_InvertAlpha( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight() );
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "invertColor" )) {
		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		// process it
		if (imageData) {
			R_InvertColor( imageData->GetData(), imageData->GetWidth(), imageData->GetHeight() );
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "makeIntensity" )) {
		int		i;

		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		// copy red to green, blue, and alpha
		if (imageData) {
			byte* pic = imageData->GetData();
			int c = imageData->GetWidth() * imageData->GetHeight() * 4;
			for (i = 0; i < c; i += 4) {
				pic[i + 1] =
					pic[i + 2] =
					pic[i + 3] = pic[i];
			}
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "makeAlpha" )) {
		int		i;

		MatchAndAppendToken( src, "(" );

		if (!ParseImageProgram_r( src, true, imageData, timestamp )) {
			return false;
		}

		// average RGB into alpha, then set RGB to white
		if (imageData) {
			byte* pic = imageData->GetData();
			int c = imageData->GetWidth() * imageData->GetHeight() * 4;
			for (i = 0; i < c; i += 4) {
				pic[i + 3] = (pic[i + 0] + pic[i + 1] + pic[i + 2]) / 3;
				pic[i + 0] =
					pic[i + 1] =
					pic[i + 2] = 255;
			}
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "cameraCubeMap" )) {
		MatchAndAppendToken( src, "(" );

		fhImageProgram p;
		const char* filename = p.ParsePastImageProgram( src );
		idStr::Append( parseBuffer, MAX_IMAGE_NAME, filename );

		if (!R_LoadCubeMap( filename, CF_CAMERA, imageData, timestamp )){
			return false;
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	if (!token.Icmp( "cubeMap" )) {
		MatchAndAppendToken( src, "(" );

		fhImageProgram p;
		const char* filename = p.ParsePastImageProgram( src );
		idStr::Append( parseBuffer, MAX_IMAGE_NAME, filename );

		if (!R_LoadCubeMap( filename, CF_NATIVE, imageData, timestamp )){
			return false;
		}

		MatchAndAppendToken( src, ")" );
		return true;
	}

	return fhImageData::LoadFile( token.c_str(), imageData, toRgba, timestamp );
}

bool fhImageProgram::LoadImageProgram( const char* program, fhImageData* imageData, ID_TIME_T* timestamp ) {

	idLexer src;
	src.LoadMemory( program, strlen( program ), program );
	src.SetFlags( LEXFL_NOFATALERRORS | LEXFL_NOSTRINGCONCAT | LEXFL_NOSTRINGESCAPECHARS | LEXFL_ALLOWPATHNAMES );

	if (imageData) {
		imageData->Clear();
	}

	this->parseBuffer[0] = '\0';

	return ParseImageProgram_r( src, false, imageData, timestamp );
}

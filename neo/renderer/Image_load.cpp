/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.
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

#include "../idlib/precompiled.h"
#pragma hdrstop

#include "tr_local.h"
#include "Sampler.h"
#include "ImageData.h"
#include "Framebuffer.h"

/*
PROBLEM: compressed textures may break the zero clamp rule!
*/

static bool FormatIsDXT( int internalFormat ) {
	if ( internalFormat < GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	|| internalFormat > GL_COMPRESSED_RGBA_S3TC_DXT5_EXT ) {
		return false;
	}
	return true;
}

/*
================
BitsForPixelFormat

Used for determining memory utilization
================
*/
static int BitsForPixelFormat( pixelFormat_t format ) {
	switch (format) {
	case pixelFormat_t::None:
		return 0;
	case pixelFormat_t::RGBA:
		return 32;
	case pixelFormat_t::BGRA:
		return 32;
	case pixelFormat_t::RGB:
		return 24;
	case pixelFormat_t::BGR:
		return 24;
	case pixelFormat_t::DXT1_RGB:
		return 4;
	case pixelFormat_t::DXT1_RGBA:
		return 4;
	case pixelFormat_t::DXT3_RGBA:
		return 8;
	case pixelFormat_t::DXT5_RGBA:
		return 8;
	case pixelFormat_t::DXT5_RxGB:
		return 8;
	case pixelFormat_t::RGTC:
		return 8;
	case pixelFormat_t::DEPTH_24_STENCIL_8:
		return 32;
	case pixelFormat_t::DEPTH_24:
		return 24;
	default:
		common->Error( "BitsForPixelFormat: BAD FORMAT:%i", static_cast<int>(format) );
		return 0;
	}
}

//=======================================================================

/*
==================
SetImageFilterAndRepeat
==================
*/
void idImage::SetImageFilterAndRepeat() {
	sampler = fhSampler::GetSampler( filter, repeat, true, true, depthComparison );
}

/*
================
idImage::Downsize
helper function that takes the current width/height and might make them smaller
================
*/
void idImage::GetDownsize( int &scaled_width, int &scaled_height ) const {
	int size = 0;

	// perform optional picmip operation to save texture memory
	if ( depth == TD_SPECULAR && globalImages->image_downSizeSpecular.GetInteger() ) {
		size = globalImages->image_downSizeSpecularLimit.GetInteger();
		if ( size == 0 ) {
			size = 64;
		}
	} else if ( depth == TD_BUMP && globalImages->image_downSizeBump.GetInteger() ) {
		size = globalImages->image_downSizeBumpLimit.GetInteger();
		if ( size == 0 ) {
			size = 64;
		}
	} else if ( ( allowDownSize || globalImages->image_forceDownSize.GetBool() ) && globalImages->image_downSize.GetInteger() ) {
		size = globalImages->image_downSizeLimit.GetInteger();
		if ( size == 0 ) {
			size = 256;
		}
	}

	if ( size > 0 ) {
		while ( scaled_width > size || scaled_height > size ) {
			if ( scaled_width > 1 ) {
				scaled_width >>= 1;
			}
			if ( scaled_height > 1 ) {
				scaled_height >>= 1;
			}
		}
	}

	// clamp to minimum size
	if ( scaled_width < 1 ) {
		scaled_width = 1;
	}
	if ( scaled_height < 1 ) {
		scaled_height = 1;
	}

	// clamp size to the hardware specific upper limit
	// scale both axis down equally so we don't have to
	// deal with a half mip resampling
	// This causes a 512*256 texture to sample down to
	// 256*128 on a voodoo3, even though it could be 256*256
	while ( scaled_width > glConfig.maxTextureSize
		|| scaled_height > glConfig.maxTextureSize ) {
		scaled_width >>= 1;
		scaled_height >>= 1;
	}
}

/*
================
GenerateImage

The alpha channel bytes should be 255 if you don't
want the channel.

We need a material characteristic to ask for specific texture modes.

Designed limitations of flexibility:

No support for texture borders.

No support for texture border color.

No support for texture environment colors or GL_BLEND or GL_DECAL
texture environments, because the automatic optimization to single
or dual component textures makes those modes potentially undefined.

No non-power-of-two images.

No palettized textures.

There is no way to specify separate wrap/clamp values for S and T

There is no way to specify explicit mip map levels

================
*/
void idImage::GenerateImage( const byte *pic, int width, int height,
					   textureFilter_t filterParm, bool allowDownSizeParm,
					   textureRepeat_t repeatParm, textureDepth_t depthParm ) {

	PurgeImage();

	filter = filterParm;
	allowDownSize = allowDownSizeParm;
	repeat = repeatParm;
	depth = depthParm;

	// this will copy the data into an fhImageData object.
	// This copy is a bit unfortunate, but it should only happen for small build-in images
	// that are generated once at game startup via generate function.
	//TODO(johl): provide a way to generate images from RGBA data directly to eliminate
	//            this copy? Or let all generator functions create fhImageData?
	fhImageData data;
	data.LoadRgbaFromMemory( pic, width, height );

	GenerateImage( data );
}

static GLenum SelectInteralFormat( pixelFormat_t pf ) {
	switch (pf) {
	case pixelFormat_t::DXT1_RGB:
		return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	case pixelFormat_t::DXT1_RGBA:
		return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
	case pixelFormat_t::DXT3_RGBA:
		return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	case pixelFormat_t::DXT5_RGBA:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case pixelFormat_t::DXT5_RxGB:
		return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
	case pixelFormat_t::RGBA:
		return GL_RGBA8;
	case pixelFormat_t::RGBA_32F:
		return GL_RGBA32F;
	case pixelFormat_t::RGB:
		return GL_RGB8;
	case pixelFormat_t::RGTC:
		return GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
	case pixelFormat_t::BGRA:
		return GL_RGBA8;
	case pixelFormat_t::BGR:
		return GL_RGB8;
	case pixelFormat_t::DEPTH_24:
		return GL_DEPTH_COMPONENT24;
	case pixelFormat_t::DEPTH_24_STENCIL_8:
		return GL_DEPTH24_STENCIL8;
	default:
		assert( false && "format not implemented or unknown?" );
		common->FatalError( "unknown PixelFormat" );
		return GL_INVALID_ENUM;
	}
}

static GLenum SelectExternalFormat( pixelFormat_t pf ) {
	switch (pf) {
	case pixelFormat_t::RGBA:
		return GL_RGBA;
	case pixelFormat_t::RGB:
		return GL_RGB;
	case pixelFormat_t::BGRA:
		return GL_BGRA;
	case pixelFormat_t::BGR:
		return GL_BGR;
	default:
		assert(false && "no external format specified");
		common->FatalError( "no external format specified" );
		return GL_INVALID_ENUM;
	}
}

static bool IsCompressed( pixelFormat_t pf ) {
	return pf == pixelFormat_t::DXT1_RGB || pf == pixelFormat_t::DXT1_RGBA || pf == pixelFormat_t::DXT3_RGBA || pf == pixelFormat_t::DXT5_RGBA || pf == pixelFormat_t::DXT5_RxGB || pf == pixelFormat_t::RGTC;
}

/*
====================
GenerateImage
====================
*/
void idImage::GenerateImage( const fhImageData& imageData ) {
	// if we don't have a rendering context, just return after we
	// have filled in the parms.  We must have the values set, or
	// an image match from a shader before OpenGL starts would miss
	// the generated texture
	//FIXME(johl): do we really need this?
	if (!glConfig.isInitialized) {
		return;
	}

	//FIXME(johl): do we really need this?
	if (imageData.GetPixelFormat() == pixelFormat_t::RGBA && imageData.GetNumFaces() == 1 && imageData.GetNumLevels() == 1) {

		// zero the border if desired, allowing clamped projection textures
		// even after picmip resampling or careless artists.
		if (repeat == TR_CLAMP_TO_ZERO) {
			byte	rgba[4];

			rgba[0] = rgba[1] = rgba[2] = 0;
			rgba[3] = 255;
			R_SetBorderTexels( (byte *)imageData.GetData(), uploadWidth, uploadHeight, rgba );
		}
		if (repeat == TR_CLAMP_TO_ZERO_ALPHA) {
			byte	rgba[4];

			rgba[0] = rgba[1] = rgba[2] = 255;
			rgba[3] = 0;
			R_SetBorderTexels( (byte *)imageData.GetData(), uploadWidth, uploadHeight, rgba );
		}
	}

	AllocateStorage( imageData.GetPixelFormat(), imageData.GetWidth(), imageData.GetHeight(), imageData.GetNumFaces(), imageData.GetMaxNumLevels() );

	for (uint32 face = 0; face < imageData.GetNumFaces(); ++face) {
		for (uint32 level = 0; level < imageData.GetNumLevels(); ++level) {
			UploadImage( imageData.GetPixelFormat(), imageData.GetWidth(level), imageData.GetHeight(level), face, level, imageData.GetSize( level ), imageData.GetData( face, level ) );
		}
	}

	if (glConfig.extDirectStateAccessAvailable) {
		glGenerateTextureMipmapEXT( texnum, type == TT_2D ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP );
	}
	else {
		assert(glConfig.arbDirectStateAccessAvailable);
		glGenerateTextureMipmap( texnum );
	}


	SetImageFilterAndRepeat();
}

/*
================
IsStorage
================
*/
bool idImage::IsStorage( pixelFormat_t format, uint32 width, uint32 height, uint32 faces, uint32 mipmaps, uint32 samples ) const {
	return texnum != TEXTURE_NOT_LOADED &&
		pixelFormat == format &&
		uploadHeight == height &&
		uploadWidth == width &&
		this->mipmaps == mipmaps &&
		this->samples == samples &&
		(faces == 1 && type == TT_2D || faces == 6 && type == TT_CUBIC);
}

/*
================
AllocateStorage
================
*/
void idImage::AllocateStorage( pixelFormat_t format, uint32 width, uint32 height, uint32 faces, uint32 mipmaps ) {
	if ( IsStorage(format, width, height, faces, mipmaps, 1) ) {
		return;
	}

	PurgeImage();

	this->pixelFormat = format;
	uploadWidth = width;
	uploadHeight = height;
	this->mipmaps = mipmaps;
	this->samples = 1;

	int target;
	if (faces == 1) {
		type = TT_2D;
		target = GL_TEXTURE_2D;
	}
	else if (faces == 6) {
		type = TT_CUBIC;
		target = GL_TEXTURE_CUBE_MAP;
	}
	else {
		assert( false && "face count must be 1 or 6" );
	}

	auto internalformat2 = SelectInteralFormat( format );
	this->internalFormat = internalformat2;

	hasAlpha = false;

	if (!glConfig.isInitialized) {
		return;
	}

	if (glConfig.extDirectStateAccessAvailable) {
		glGenTextures(1, &texnum);
		glTextureStorage2DEXT(texnum, target, mipmaps, internalformat2, uploadWidth, uploadHeight);

		if (format == pixelFormat_t::DXT5_RxGB) {
			static const GLint agbr[] = { GL_ALPHA, GL_GREEN, GL_BLUE, GL_RED };
			glTextureParameterIivEXT(texnum, GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, agbr);
		}
	}
	else {
		assert(glConfig.arbDirectStateAccessAvailable);

		glCreateTextures(target, 1, &texnum);
		glTextureStorage2D(texnum, mipmaps, internalformat2, uploadWidth, uploadHeight);

		if (format == pixelFormat_t::DXT5_RxGB) {
			static const GLint agbr[] = { GL_ALPHA, GL_GREEN, GL_BLUE, GL_RED };
			glTextureParameterIiv(texnum, GL_TEXTURE_SWIZZLE_RGBA, agbr);
		}
	}
}

/*
================
AllocateStorage
================
*/
void idImage::AllocateMultiSampleStorage( pixelFormat_t format, uint32 width, uint32 height, uint32 samples ) {
	if (samples <= 1) {
		AllocateStorage( format, width, height, 1, 1 );
		return;
	}

	if (IsStorage( format, width, height, 1, 1, samples )) {
		return;
	}

	PurgeImage();

	this->pixelFormat = format;
	uploadWidth = width;
	uploadHeight = height;
	this->mipmaps = mipmaps;
	this->samples = samples;
	type = TT_2D;

	auto internalformat2 = SelectInteralFormat( format );
	this->internalFormat = internalformat2;

	hasAlpha = false;

	if (glConfig.extDirectStateAccessAvailable) {
		glGenTextures(1, &texnum);
		glTextureStorage2DMultisampleEXT(texnum, GL_TEXTURE_2D_MULTISAMPLE, samples, internalformat2, width, height, GL_FALSE);
	}
	else {
		assert(glConfig.arbDirectStateAccessAvailable);

		glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &texnum);
		glTextureStorage2DMultisample(texnum, samples, internalformat2, width, height, GL_FALSE);
	}
}

/*
================
UploadImage
================
*/
void idImage::UploadImage( pixelFormat_t format, uint32 width, uint32 height, uint32 face, uint32 mipmapLevel, uint32 size, const void* data ) {
	if ( pixelFormat != format ) {
		common->Error( "image format mismatch" );
		return;
	}

	const bool compressed = IsCompressed( format );
	const GLenum internalFormat = SelectInteralFormat( format );

	if (glConfig.extDirectStateAccessAvailable) {
		const GLenum facetarget = (type == TT_CUBIC) ? GL_TEXTURE_CUBE_MAP_POSITIVE_X + face : GL_TEXTURE_2D;

		if (compressed) {
			glCompressedTextureSubImage2DEXT(texnum, facetarget, mipmapLevel, 0, 0, width, height, internalFormat, size, data);
		}
		else {
			const GLenum externalFormat = SelectExternalFormat(format);
			glTextureSubImage2DEXT(texnum, facetarget, mipmapLevel, 0, 0, width, height, externalFormat, GL_UNSIGNED_BYTE, data);
		}
	}
	else {
		assert(glConfig.arbDirectStateAccessAvailable);

		if (type == TT_CUBIC) {
			if (compressed) {
				glCompressedTextureSubImage3D(texnum, mipmapLevel, 0, 0, face, width, height, 1, internalFormat, size, data);
			}
			else {
				const GLenum externalFormat = SelectExternalFormat(format);
				glTextureSubImage3D(texnum, mipmapLevel, 0, 0, face, width, height, 1, externalFormat, GL_UNSIGNED_BYTE, data);
			}
		}
		else {
			if (compressed) {
				glCompressedTextureSubImage2D(texnum, mipmapLevel, 0, 0, width, height, internalFormat, size, data);
			}
			else {
				const GLenum externalFormat = SelectExternalFormat(format);
				glTextureSubImage2D(texnum, mipmapLevel, 0, 0, width, height, externalFormat, GL_UNSIGNED_BYTE, data);
			}
		}
	}

}

/*
================
ImageProgramStringToFileCompressedFileName
================
*/
void idImage::ImageProgramStringToCompressedFileName( const char *imageProg, char *fileName ) const {
	const char	*s;
	char	*f;

	strcpy( fileName, "dds/" );
	f = fileName + strlen( fileName );

	int depth = 0;

	// convert all illegal characters to underscores
	// this could conceivably produce a duplicated mapping, but we aren't going to worry about it
	for ( s = imageProg ; *s ; s++ ) {
		if ( *s == '/' || *s == '\\' || *s == '(') {
			if ( depth < 4 ) {
				*f = '/';
				depth ++;
			} else {
				*f = ' ';
			}
			f++;
		} else if ( *s == '<' || *s == '>' || *s == ':' || *s == '|' || *s == '"' || *s == '.' ) {
			*f = '_';
			f++;
		} else if ( *s == ' ' && *(f-1) == '/' ) {	// ignore a space right after a slash
		} else if ( *s == ')' || *s == ',' ) {		// always ignore these
		} else {
			*f = *s;
			f++;
		}
	}
	*f++ = 0;
	strcat( fileName, ".dds" );
}

/*
==================
NumLevelsForImageSize
==================
*/
int	idImage::NumLevelsForImageSize( int width, int height ) const {
	int	numLevels = 1;

	while ( width > 1 || height > 1 ) {
		numLevels++;
		width >>= 1;
		height >>= 1;
	}

	return numLevels;
}

/*
================
WritePrecompressedImage

When we are happy with our source data, we can write out precompressed
versions of everything to speed future load times.
================
*/
void idImage::WritePrecompressedImage() {
	// Always write the precompressed image if we're making a build
	if ( !com_makingBuild.GetBool() ) {
		if ( !globalImages->image_writePrecompressedTextures.GetBool() || !globalImages->image_usePrecompressedTextures.GetBool() ) {
			return;
		}
	}

	if ( !glConfig.isInitialized ) {
		return;
	}

	common->Warning( "idImage::WritePrecompressedImage() is currently not implemented!" );

	//FIXME(johl): remove this? or re-implement with modern compression tech?
#if 0
	char filename[MAX_IMAGE_NAME];
	ImageProgramStringToCompressedFileName( imgName, filename );

	int numLevels = NumLevelsForImageSize( uploadWidth, uploadHeight );
	if ( numLevels > MAX_TEXTURE_LEVELS ) {
		common->Warning( "R_WritePrecompressedImage: level > MAX_TEXTURE_LEVELS for image %s", filename );
		return;
	}

	// glGetTexImage only supports a small subset of all the available internal formats
	// We have to use BGRA because DDS is a windows based format
	int altInternalFormat = 0;
	int bitSize = 0;
	switch ( internalFormat ) {
		case GL_COLOR_INDEX8_EXT:
		case GL_COLOR_INDEX:
			// this will not work with dds viewers but we need it in this format to save disk
			// load speed ( i.e. size )
			altInternalFormat = GL_COLOR_INDEX;
			bitSize = 24;
		break;
		case 1:
		case GL_INTENSITY8:
		case GL_LUMINANCE8:
		case 3:
		case GL_RGB8:
			altInternalFormat = GL_BGR_EXT;
			bitSize = 24;
		break;
		case GL_LUMINANCE8_ALPHA8:
		case 4:
		case GL_RGBA8:
			altInternalFormat = GL_BGRA_EXT;
			bitSize = 32;
		break;
		case GL_ALPHA8:
			altInternalFormat = GL_ALPHA;
			bitSize = 8;
		break;
		default:
			if ( FormatIsDXT( internalFormat ) ) {
				altInternalFormat = internalFormat;
			} else {
				common->Warning("Unknown or unsupported format for %s", filename);
				return;
			}
	}

	if ( globalImages->image_useOffLineCompression.GetBool() && FormatIsDXT( altInternalFormat ) ) {
		idStr outFile = fileSystem->RelativePathToOSPath( filename, "fs_basepath" );
		idStr inFile = outFile;
		inFile.StripFileExtension();
		inFile.SetFileExtension( "tga" );
		idStr format;
		if ( depth == TD_BUMP ) {
			format = "RXGB +red 0.0 +green 0.5 +blue 0.5";
		} else {
			switch ( altInternalFormat ) {
				case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
					format = "DXT1";
					break;
				case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
					format = "DXT1 -alpha_threshold";
					break;
				case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
					format = "DXT3";
					break;
				case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
					format = "DXT5";
					break;
			}
		}
		globalImages->AddDDSCommand( va( "z:/d3xp/compressonator/thecompressonator -convert \"%s\" \"%s\" %s -mipmaps\n", inFile.c_str(), outFile.c_str(), format.c_str() ) );
		return;
	}

	ddsFileHeader_t header;
	memset( &header, 0, sizeof(header) );
	header.dwSize = sizeof(header);
	header.dwFlags = DDSF_CAPS | DDSF_PIXELFORMAT | DDSF_WIDTH | DDSF_HEIGHT;
	header.dwHeight = uploadHeight;
	header.dwWidth = uploadWidth;

	// hack in our monochrome flag for the NV20 optimization
	if ( isMonochrome ) {
		header.dwFlags |= DDSF_ID_MONOCHROME;
	}

	if ( FormatIsDXT( altInternalFormat ) ) {
		// size (in bytes) of the compressed base image
		header.dwFlags |= DDSF_LINEARSIZE;
		header.dwPitchOrLinearSize = ( ( uploadWidth + 3 ) / 4 ) * ( ( uploadHeight + 3 ) / 4 )*
			(altInternalFormat <= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
	}
	else {
		// 4 Byte aligned line width (from nv_dds)
		header.dwFlags |= DDSF_PITCH;
		header.dwPitchOrLinearSize = ( ( uploadWidth * bitSize + 31 ) & -32 ) >> 3;
	}

	header.dwCaps1 = DDSF_TEXTURE;

	if ( numLevels > 1 ) {
		header.dwMipMapCount = numLevels;
		header.dwFlags |= DDSF_MIPMAPCOUNT;
		header.dwCaps1 |= DDSF_MIPMAP | DDSF_COMPLEX;
	}

	header.ddspf.dwSize = sizeof(header.ddspf);
	if ( FormatIsDXT( altInternalFormat ) ) {
		header.ddspf.dwFlags = DDSF_FOURCC;
		switch ( altInternalFormat ) {
		case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
			header.ddspf.dwFourCC = DDS_MAKEFOURCC('D','X','T','1');
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
			header.ddspf.dwFlags |= DDSF_ALPHAPIXELS;
			header.ddspf.dwFourCC = DDS_MAKEFOURCC('D','X','T','1');
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
			header.ddspf.dwFourCC = DDS_MAKEFOURCC('D','X','T','3');
			break;
		case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
			header.ddspf.dwFourCC = DDS_MAKEFOURCC('D','X','T','5');
			break;
		}
	} else {
		header.ddspf.dwFlags = ( internalFormat == GL_COLOR_INDEX8_EXT ) ? DDSF_RGB | DDSF_ID_INDEXCOLOR : DDSF_RGB;
		header.ddspf.dwRGBBitCount = bitSize;
		switch ( altInternalFormat ) {
		case GL_BGRA_EXT:
		case GL_LUMINANCE_ALPHA:
			header.ddspf.dwFlags |= DDSF_ALPHAPIXELS;
			header.ddspf.dwABitMask = 0xFF000000;
			// Fall through
		case GL_BGR_EXT:
		case GL_LUMINANCE:
		case GL_COLOR_INDEX:
			header.ddspf.dwRBitMask = 0x00FF0000;
			header.ddspf.dwGBitMask = 0x0000FF00;
			header.ddspf.dwBBitMask = 0x000000FF;
			break;
		case GL_ALPHA:
			header.ddspf.dwFlags = DDSF_ALPHAPIXELS;
			header.ddspf.dwABitMask = 0xFF000000;
			break;
		default:
			common->Warning( "Unknown or unsupported format for %s", filename );
			return;
		}
	}

	idFile *f = fileSystem->OpenFileWrite( filename );
	if ( f == NULL ) {
		common->Warning( "Could not open %s trying to write precompressed image", filename );
		return;
	}
	common->Printf( "Writing precompressed image: %s\n", filename );

	f->Write( "DDS ", 4 );
	f->Write( &header, sizeof(header) );

	// bind to the image so we can read back the contents
	Bind(0);

	glPixelStorei( GL_PACK_ALIGNMENT, 1 );	// otherwise small rows get padded to 32 bits

	int uw = uploadWidth;
	int uh = uploadHeight;

	// Will be allocated first time through the loop
	byte *data = NULL;

	for ( int level = 0 ; level < numLevels ; level++ ) {

		int size = 0;
		if ( FormatIsDXT( altInternalFormat ) ) {
			size = ( ( uw + 3 ) / 4 ) * ( ( uh + 3 ) / 4 ) *
				(altInternalFormat <= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
		} else {
			size = uw * uh * (bitSize / 8);
		}

		if (data == NULL) {
			data = (byte *)R_StaticAlloc( size );
		}

		if ( FormatIsDXT( altInternalFormat ) ) {
			glGetCompressedTexImageARB( GL_TEXTURE_2D, level, data );
		} else {
			glGetTexImage( GL_TEXTURE_2D, level, altInternalFormat, GL_UNSIGNED_BYTE, data );
		}

		f->Write( data, size );

		uw /= 2;
		uh /= 2;
		if (uw < 1) {
			uw = 1;
		}
		if (uh < 1) {
			uh = 1;
		}
	}

	if (data != NULL) {
		R_StaticFree( data );
	}

	fileSystem->CloseFile( f );

#endif
}

/*
================
ShouldImageBePartialCached

Returns true if there is a precompressed image, and it is large enough
to be worth caching
================
*/
bool idImage::ShouldImageBePartialCached() {
	if ( !glConfig.textureCompressionAvailable ) {
		return false;
	}

	if ( !globalImages->image_useCache.GetBool() ) {
		return false;
	}

	// the allowDownSize flag does double-duty as don't-partial-load
	if ( !allowDownSize ) {
		return false;
	}

	if ( globalImages->image_cacheMinK.GetInteger() <= 0 ) {
		return false;
	}

	// if we are doing a copyFiles, make sure the original images are referenced
	if ( fileSystem->PerformingCopyFiles() ) {
		return false;
	}

	char	filename[MAX_IMAGE_NAME];
	ImageProgramStringToCompressedFileName( imgName, filename );

	// get the file timestamp
	fileSystem->ReadFile( filename, NULL, &timestamp );

	if ( timestamp == FILE_NOT_FOUND_TIMESTAMP ) {
		return false;
	}

	// open it and get the file size
	idFile *f;

	f = fileSystem->OpenFileRead( filename );
	if ( !f ) {
		return false;
	}

	int	len = f->Length();
	fileSystem->CloseFile( f );

	if ( len <= globalImages->image_cacheMinK.GetInteger() * 1024 ) {
		return false;
	}

	// we do want to do a partial load
	return true;
}

/*
================
CheckPrecompressedImage

If fullLoad is false, only the small mip levels of the image will be loaded
================
*/
bool idImage::CheckPrecompressedImage( bool fullLoad ) {
	if ( !glConfig.isInitialized || !glConfig.textureCompressionAvailable ) {
		return false;
	}

#if 1 // ( _D3XP had disabled ) - Allow grabbing of DDS's from original Doom pak files
	// if we are doing a copyFiles, make sure the original images are referenced
	if ( fileSystem->PerformingCopyFiles() ) {
		return false;
	}
#endif

	if ( depth == TD_BUMP && globalImages->image_useNormalCompression.GetInteger() != 2 ) {
		return false;
	}

	// god i love last minute hacks :-)
	if ( com_machineSpec.GetInteger() >= 1 && com_videoRam.GetInteger() >= 128 && imgName.Icmpn( "lights/", 7 ) == 0 ) {
		return false;
	}

	char filename[MAX_IMAGE_NAME];
	ImageProgramStringToCompressedFileName( imgName, filename );

	// get the file timestamp
	ID_TIME_T precompTimestamp;
	fileSystem->ReadFile( filename, NULL, &precompTimestamp );


	if ( precompTimestamp == FILE_NOT_FOUND_TIMESTAMP ) {
		return false;
	}

	if ( !generatorFunction && timestamp != FILE_NOT_FOUND_TIMESTAMP ) {
		if ( precompTimestamp < timestamp ) {
			// The image has changed after being precompressed
			return false;
		}
	}

	timestamp = precompTimestamp;

	fhImageData data;
	if (!fhImageData::LoadFile(filename, &data, false, nullptr)) {
		return false;
	}

	GenerateImage( data );
	return true;
}

/*
===================
UploadPrecompressedImage

This can be called by the front end during nromal loading,
or by the backend after a background read of the file
has completed
===================
*/
void idImage::UploadPrecompressedImage( byte *data, int len ) {
	ddsFileHeader_t	*header = (ddsFileHeader_t *)(data + 4);

	// ( not byte swapping dwReserved1 dwReserved2 )
	header->dwSize = LittleLong( header->dwSize );
	header->dwFlags = LittleLong( header->dwFlags );
	header->dwHeight = LittleLong( header->dwHeight );
	header->dwWidth = LittleLong( header->dwWidth );
	header->dwPitchOrLinearSize = LittleLong( header->dwPitchOrLinearSize );
	header->dwDepth = LittleLong( header->dwDepth );
	header->dwMipMapCount = LittleLong( header->dwMipMapCount );
	header->dwCaps1 = LittleLong( header->dwCaps1 );
	header->dwCaps2 = LittleLong( header->dwCaps2 );

	header->ddspf.dwSize = LittleLong( header->ddspf.dwSize );
	header->ddspf.dwFlags = LittleLong( header->ddspf.dwFlags );
	header->ddspf.dwFourCC = LittleLong( header->ddspf.dwFourCC );
	header->ddspf.dwRGBBitCount = LittleLong( header->ddspf.dwRGBBitCount );
	header->ddspf.dwRBitMask = LittleLong( header->ddspf.dwRBitMask );
	header->ddspf.dwGBitMask = LittleLong( header->ddspf.dwGBitMask );
	header->ddspf.dwBBitMask = LittleLong( header->ddspf.dwBBitMask );
	header->ddspf.dwABitMask = LittleLong( header->ddspf.dwABitMask );

	// generate the texture number
	glGenTextures( 1, &texnum );

	int externalFormat = 0;

	precompressedFile = true;

	const char* fourCC = (const char*)&header->ddspf.dwFourCC;

	uploadWidth = header->dwWidth;
	uploadHeight = header->dwHeight;
    if ( header->ddspf.dwFlags & DDSF_FOURCC ) 
	{
        switch ( header->ddspf.dwFourCC ) 
		{
        case DDS_MAKEFOURCC( 'D', 'X', 'T', '1' ):
			if ( header->ddspf.dwFlags & DDSF_ALPHAPIXELS ) 
			{
				internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
				pixelFormat = pixelFormat_t::DXT1_RGBA;
			} 
			else 
			{
				internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
				pixelFormat = pixelFormat_t::DXT1_RGB;
			}
            break;
        case DDS_MAKEFOURCC( 'D', 'X', 'T', '3' ):
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
			pixelFormat = pixelFormat_t::DXT3_RGBA;
            break;
        case DDS_MAKEFOURCC( 'D', 'X', 'T', '5' ):
            internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			pixelFormat = pixelFormat_t::DXT5_RGBA;
            break;
		case DDS_MAKEFOURCC( 'R', 'X', 'G', 'B' ):
			internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
			pixelFormat = pixelFormat_t::DXT5_RxGB;
			break;
        default:
            common->Warning( "Invalid compressed internal format\n" );
            return;
        }
    } 
	else if ( ( header->ddspf.dwFlags & DDSF_RGBA ) && header->ddspf.dwRGBBitCount == 32 ) 
	{
		externalFormat = GL_BGRA_EXT;
		internalFormat = GL_RGBA8;
    } 
	else if ( ( header->ddspf.dwFlags & DDSF_RGB ) && header->ddspf.dwRGBBitCount == 32 ) 
	{
        externalFormat = GL_BGRA_EXT;
		internalFormat = GL_RGBA8;
    } 
	else if ( ( header->ddspf.dwFlags & DDSF_RGB ) && header->ddspf.dwRGBBitCount == 24 ) 
	{
		if ( header->ddspf.dwFlags & DDSF_ID_INDEXCOLOR ) 
		{
			assert( false && "not supported" );
			externalFormat = GL_COLOR_INDEX;
			internalFormat = GL_COLOR_INDEX8_EXT;
		} 
		else 
		{
			externalFormat = GL_BGR_EXT;
			internalFormat = GL_RGB8;
		}
	} 
	else if ( header->ddspf.dwRGBBitCount == 8 ) 
	{
		assert( false && "not supported" );
		externalFormat = GL_ALPHA;
		internalFormat = GL_ALPHA8;
	}
	else 
	{
		common->Warning( "Invalid uncompressed internal format\n" );
		return;
	}

	type = TT_2D;			// FIXME: we may want to support pre-compressed cube maps in the future

	if(!glConfig.extDirectStateAccessAvailable) 
	{
		Bind(0);
	}

	int numMipmaps = 1;
	if ( header->dwFlags & DDSF_MIPMAPCOUNT ) 
	{
		numMipmaps = header->dwMipMapCount;
	}

	int uw = uploadWidth;
	int uh = uploadHeight;

	// We may skip some mip maps if we are downsizing
	int skipMip = 0;
	GetDownsize( uploadWidth, uploadHeight );

	byte *imagedata = data + sizeof(ddsFileHeader_t) + 4;

	for ( int i = 0 ; i < numMipmaps; i++ ) 
	{
		int size = 0;
		if ( FormatIsDXT( internalFormat ) ) 
		{
			size = ( ( uw + 3 ) / 4 ) * ( ( uh + 3 ) / 4 ) * (internalFormat <= GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ? 8 : 16);
		} 
		else 
		{
			size = uw * uh * (header->ddspf.dwRGBBitCount / 8);
		}

		if ( uw > uploadWidth || uh > uploadHeight ) 
		{
			skipMip++;
		} 
		else 
		{
			if ( FormatIsDXT( internalFormat ) ) 
			{
				if(glConfig.extDirectStateAccessAvailable) 
				{
					glCompressedTextureImage2DEXT( texnum, GL_TEXTURE_2D, i - skipMip, internalFormat, uw, uh, 0, size, imagedata );
				}
				else 
				{
					glCompressedTexImage2DARB( GL_TEXTURE_2D, i - skipMip, internalFormat, uw, uh, 0, size, imagedata );
				}
			} 
			else 
			{
				if(glConfig.extDirectStateAccessAvailable) 
				{
					glTextureImage2DEXT( texnum, GL_TEXTURE_2D, i - skipMip, internalFormat, uw, uh, 0, externalFormat, GL_UNSIGNED_BYTE, imagedata );
				}
				else 
				{
					glTexImage2D( GL_TEXTURE_2D, i - skipMip, internalFormat, uw, uh, 0, externalFormat, GL_UNSIGNED_BYTE, imagedata );
				}
			}
		}

		imagedata += size;
		uw /= 2;
		uh /= 2;
		if (uw < 1) {
			uw = 1;
		}
		if (uh < 1) {
			uh = 1;
		}
	}

	SetImageFilterAndRepeat();
}

/*
===============
ActuallyLoadImage

Absolutely every image goes through this path
On exit, the idImage will have a valid OpenGL texture number that can be bound
===============
*/
void	idImage::ActuallyLoadImage( bool checkForPrecompressed, bool fromBackEnd ) {
	// this is the ONLY place generatorFunction will ever be called
	if ( generatorFunction ) {
		generatorFunction( this );
		return;
	}

	// if we are a partial image, we are only going to load from a compressed file
	if ( isPartialImage ) {
		if ( CheckPrecompressedImage( false ) ) {
			return;
		}
		// this is an error -- the partial image failed to load
		MakeDefault();
		return;
	}

	//
	// load the image from disk
	//

	// see if we have a pre-generated image file that is
	// already image processed and compressed
	if ( checkForPrecompressed && globalImages->image_usePrecompressedTextures.GetBool() ) {
		if ( CheckPrecompressedImage( true ) ) {
			// we got the precompressed image
			return;
		}
		// fall through to load the normal image
	}

	fhImageData imgData;
	bool ok = imgData.LoadProgram(imgName);
	if (!ok || !imgData.IsValid()) {
		common->Warning("Couldn't load image: %s", imgName.c_str());
		MakeDefault();
		return;
	}

	this->timestamp = imgData.GetTimeStamp();

	GenerateImage(imgData);

	this->precompressedFile = false;

	// write out the precompressed version of this file if needed
	WritePrecompressedImage();
}

//=========================================================================================================

/*
===============
PurgeImage
===============
*/
void idImage::PurgeImage() {
	if ( texnum != TEXTURE_NOT_LOADED ) {
		glDeleteTextures( 1, &texnum );	// this should be the ONLY place it is ever called!
		texnum = TEXTURE_NOT_LOADED;
	}

	// clear all the current binding caches, so the next bind will do a real one
	for ( int i = 0 ; i < MAX_MULTITEXTURE_UNITS ; i++ ) {
		backEnd.glState.tmu[i].currentTexture = TEXTURE_NOT_LOADED;
		backEnd.glState.tmu[i].currentTextureType = TT_2D;
	}
}

/*
==============
Bind

Automatically enables 2D mapping, cube mapping, or 3D texturing if needed
==============
*/
void idImage::Bind(int textureUnit) {

	// if this is an image that we are caching, move it to the front of the LRU chain
	if ( partialImage ) {
		if ( cacheUsageNext ) {
			// unlink from old position
			cacheUsageNext->cacheUsagePrev = cacheUsagePrev;
			cacheUsagePrev->cacheUsageNext = cacheUsageNext;
		}
		// link in at the head of the list
		cacheUsageNext = globalImages->cacheLRU.cacheUsageNext;
		cacheUsagePrev = &globalImages->cacheLRU;

		cacheUsageNext->cacheUsagePrev = this;
		cacheUsagePrev->cacheUsageNext = this;
	}

	// load the image if necessary (FIXME: not SMP safe!)
	if ( texnum == TEXTURE_NOT_LOADED ) {
		if ( partialImage ) {
			// if we have a partial image, go ahead and use that
			this->partialImage->Bind(textureUnit);

			// start a background load of the full thing if it isn't already in the queue
			if ( !backgroundLoadInProgress ) {
				StartBackgroundImageLoad();
			}
			return;
		}

		// load the image on demand here, which isn't our normal game operating mode
		ActuallyLoadImage( true, true );	// check for precompressed, load is from back end
	}

	// bump our statistic counters
	frameUsed = backEnd.frameCount;
	bindCount++;

	if ( textureUnit == -1 ) {
		textureUnit = backEnd.glState.currenttmu;
	}

	tmu_t *tmu = &backEnd.glState.tmu[textureUnit];

	if ( tmu->currentTexture != texnum ) {
		if(glConfig.extDirectStateAccessAvailable) {
			if (type == TT_2D) {
				glBindMultiTextureEXT(GL_TEXTURE0 + textureUnit, GL_TEXTURE_2D, texnum);
			}
			else if (type == TT_CUBIC) {
				glBindMultiTextureEXT(GL_TEXTURE0 + textureUnit, GL_TEXTURE_CUBE_MAP, texnum);
			}
		}
		else {
			assert(glConfig.arbDirectStateAccessAvailable);
			glBindTextureUnit(textureUnit, texnum);
		}

		tmu->currentTexture = texnum;
		tmu->currentTextureType = type;
	}

	if ( sampler ) {
		sampler->Bind(textureUnit);
	}

	if ( com_purgeAll.GetBool() ) {
		GLclampf priority = 1.0f;
		glPrioritizeTextures( 1, &texnum, &priority );
	}
}

/*
=============
RB_UploadScratchImage

if rows = cols * 6, assume it is a cube map animation
=============
*/
void idImage::UploadScratch( int textureUnit, const byte *data, int cols, int rows ) {
	int			i;

	// if rows = cols * 6, assume it is a cube map animation
	if ( rows == cols * 6 ) {
		if ( type != TT_CUBIC ) {
			type = TT_CUBIC;
			uploadWidth = -1;	// for a non-sub upload
		}

		rows /= 6;

		if(glConfig.extDirectStateAccessAvailable) {
			// if the scratchImage isn't in the format we want, specify it as a new texture
			if (cols != uploadWidth || rows != uploadHeight) {
				uploadWidth = cols;
				uploadHeight = rows;

				// upload the base level
				for (i = 0; i < 6; i++) {
					glTextureImage2DEXT( texnum, GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + i, 0, GL_RGB8, cols, rows, 0,
						GL_RGBA, GL_UNSIGNED_BYTE, data + cols*rows * 4 * i );
				}
			}
			else {
				// otherwise, just subimage upload it so that drivers can tell we are going to be changing
				// it and don't try and do a texture compression
				for (i = 0; i < 6; i++) {
					glTextureSubImage2DEXT( texnum, GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + i, 0, 0, 0, cols, rows,
						GL_RGBA, GL_UNSIGNED_BYTE, data + cols*rows * 4 * i );
				}
			}

			//TODO(johl): this bind is not strictly required, but the caller relies on this image being bound after
			Bind(textureUnit);
		}
		else {
			assert(glConfig.arbDirectStateAccessAvailable);
			common->Warning("UploadScratch not implemented for cube maps\n");
#if 0
			// if the scratchImage isn't in the format we want, specify it as a new texture
			if (cols != uploadWidth || rows != uploadHeight) {
				uploadWidth = cols;
				uploadHeight = rows;

				// upload the base level
				for (i = 0; i < 6; i++) {
					glTexImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + i, 0, GL_RGB8, cols, rows, 0,
						GL_RGBA, GL_UNSIGNED_BYTE, data + cols*rows * 4 * i );
				}
			}
			else {
				// otherwise, just subimage upload it so that drivers can tell we are going to be changing
				// it and don't try and do a texture compression
				for (i = 0; i < 6; i++) {
					glTexSubImage2D( GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + i, 0, 0, 0, cols, rows,
						GL_RGBA, GL_UNSIGNED_BYTE, data + cols*rows * 4 * i );
				}
			}
#endif
			//TODO(johl): this bind is not strictly required, but the caller relies on this image being bound after
			Bind(textureUnit);
		}

		filter = TF_LINEAR;
		repeat = TR_CLAMP;
	} else {
		// otherwise, it is a 2D image
		if ( type != TT_2D ) {
			type = TT_2D;
			uploadWidth = -1;	// for a non-sub upload
		}

		if(glConfig.extDirectStateAccessAvailable) {
			if (cols != uploadWidth || rows != uploadHeight) {
				uploadWidth = cols;
				uploadHeight = rows;
				glTextureImage2DEXT( texnum, GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			}
			else {
				// otherwise, just subimage upload it so that drivers can tell we are going to be changing
				// it and don't try and do a texture compression
				glTextureSubImage2DEXT( texnum, GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
			}

			//TODO(johl): this bind is not strictly required, but the caller relies on this image being bound after
			Bind( textureUnit );
		}
		else {
			assert(glConfig.arbDirectStateAccessAvailable);
			common->Warning("UploadScratch not implemented\n");
#if 0
			// if the scratchImage isn't in the format we want, specify it as a new texture
			if (cols != uploadWidth || rows != uploadHeight) {
				uploadWidth = cols;
				uploadHeight = rows;
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, cols, rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
			}
			else {
				// otherwise, just subimage upload it so that drivers can tell we are going to be changing
				// it and don't try and do a texture compression
				glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, cols, rows, GL_RGBA, GL_UNSIGNED_BYTE, data );
			}
#endif
			//TODO(johl): this bind is not strictly required, but the caller relies on this image being bound after
			Bind(textureUnit);
		}


		filter = TF_LINEAR;
		repeat = TR_REPEAT;
	}

	SetImageFilterAndRepeat();
}


void idImage::SetClassification( int tag ) {
	classification = tag;
}

/*
==================
StorageSize
==================
*/
int idImage::StorageSize() const {
	int		baseSize;

	if ( texnum == TEXTURE_NOT_LOADED ) {
		return 0;
	}

	switch ( type ) {
	default:
	case TT_2D:
		baseSize = uploadWidth*uploadHeight;
		break;
	case TT_CUBIC:
		baseSize = 6 * uploadWidth*uploadHeight;
		break;
	}

	baseSize *= BitsForPixelFormat( pixelFormat );

	baseSize /= 8;

	// account for mip mapping
	if (mipmaps > 1) {
		baseSize = baseSize * 4 / 3;
	}

	return baseSize;
}

/*
==================
Print
==================
*/
void idImage::Print() const {
	if ( precompressedFile ) {
		common->Printf( "P" );
	} else if ( generatorFunction ) {
		common->Printf( "F" );
	} else {
		common->Printf( " " );
	}

	switch ( type ) {
	case TT_2D:
		common->Printf( " " );
		break;
	case TT_CUBIC:
		common->Printf( "C" );
		break;
	default:
		common->Printf( "<BAD TYPE:%i>", type );
		break;
	}

	common->Printf( "%4i %4i ",	uploadWidth, uploadHeight );

	switch( filter ) {
	case TF_DEFAULT:
		common->Printf( "dflt " );
		break;
	case TF_LINEAR:
		common->Printf( "linr " );
		break;
	case TF_NEAREST:
		common->Printf( "nrst " );
		break;
	default:
		common->Printf( "<BAD FILTER:%i>", filter );
		break;
	}

	switch (pixelFormat) {
	case pixelFormat_t::None:
		common->Printf( "            " );
		break;
	case pixelFormat_t::RGBA:
		common->Printf( "RGBA        " );
		break;
	case pixelFormat_t::BGRA:
		common->Printf( "BGRA        " );
		break;
	case pixelFormat_t::RGB:
		common->Printf( "RGB         " );
		break;
	case pixelFormat_t::BGR:
		common->Printf( "BGR         " );
		break;
	case pixelFormat_t::DXT1_RGB:
		common->Printf( "DXT1 (RGB)  " );
		break;
	case pixelFormat_t::DXT1_RGBA:
		common->Printf( "DXT1 (RGBA) " );
		break;
	case pixelFormat_t::DXT3_RGBA:
		common->Printf( "DXT3 (RGBA) " );
		break;
	case pixelFormat_t::DXT5_RGBA:
		common->Printf( "DXT5 (RGBA) " );
		break;
	case pixelFormat_t::DXT5_RxGB:
		common->Printf( "DXT5 (RxGB) " );
		break;
	case pixelFormat_t::RGTC:
		common->Printf( "RGTC        " );
		break;
	case pixelFormat_t::DEPTH_24_STENCIL_8:
		common->Printf( "Depth24_8   " );
		break;
	case pixelFormat_t::DEPTH_24:
		common->Printf( "Depth24     " );
		break;
	default:
		common->Printf( "<BAD FORMAT:%i>", static_cast<int>(pixelFormat) );
		break;
	}

	switch ( repeat ) {
	case TR_REPEAT:
		common->Printf( "rept " );
		break;
	case TR_CLAMP_TO_ZERO:
		common->Printf( "zero " );
		break;
	case TR_CLAMP_TO_ZERO_ALPHA:
		common->Printf( "azro " );
		break;
	case TR_CLAMP:
		common->Printf( "clmp " );
		break;
	default:
		common->Printf( "<BAD REPEAT:%i>", repeat );
		break;
	}

	common->Printf( "%4ik ", StorageSize() / 1024 );

	common->Printf( " %s\n", imgName.c_str() );
}
// shadertype=glsl

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

#include "global.inc"

layout(binding = 1) uniform sampler2D texture1;
layout(binding = 2) uniform sampler2D texture2;
layout(binding = 3) uniform sampler2D bloodNormal;
layout(binding = 4) uniform sampler2D bloodBlur;

in vs_output
{
  vec2 texcoord;
} frag;

out vec4 result; // 130

#ifndef FXAA_REDUCE_MIN
    #define FXAA_REDUCE_MIN   (1.0/ 128.0)
#endif
#ifndef FXAA_REDUCE_MUL
    #define FXAA_REDUCE_MUL   0
#endif
#ifndef FXAA_SPAN_MAX
    #define FXAA_SPAN_MAX     8.0
#endif


//optimized version for mobile, where dependent
//texture reads can be a bottleneck
vec4 fxaa( sampler2D tex, vec2 fragCoord, vec2 resolution,
		   vec2 v_rgbNW, vec2 v_rgbNE,
		   vec2 v_rgbSW, vec2 v_rgbSE,
		   vec2 v_rgbM )
{
	vec4 color;
	vec2 inverseVP = vec2( 1.0 / resolution.x, 1.0 / resolution.y );
	vec3 rgbNW = texture2D( tex, v_rgbNW ).xyz;
	vec3 rgbNE = texture2D( tex, v_rgbNE ).xyz;
	vec3 rgbSW = texture2D( tex, v_rgbSW ).xyz;
	vec3 rgbSE = texture2D( tex, v_rgbSE ).xyz;
	vec4 texColor = texture2D( tex, v_rgbM );

	vec3 rgbM = texColor.xyz;
	vec3 luma = vec3( 0.299, 0.587, 0.114 );
	float lumaNW = dot( rgbNW, luma );
	float lumaNE = dot( rgbNE, luma );
	float lumaSW = dot( rgbSW, luma );
	float lumaSE = dot( rgbSE, luma );
	float lumaM = dot( rgbM, luma );
	float lumaMin = min( lumaM, min( min( lumaNW, lumaNE ), min( lumaSW, lumaSE ) ) );
	float lumaMax = max( lumaM, max( max( lumaNW, lumaNE ), max( lumaSW, lumaSE ) ) );

	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y = ((lumaNW + lumaSW) - (lumaNE + lumaSE));

	float dirReduce = max( (lumaNW + lumaNE + lumaSW + lumaSE) *
		(0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN );

	float rcpDirMin = 1.0 / (min( abs( dir.x ), abs( dir.y ) ) + dirReduce);
	dir = min( vec2( FXAA_SPAN_MAX, FXAA_SPAN_MAX ),
			   max( vec2( -FXAA_SPAN_MAX, -FXAA_SPAN_MAX ),
					dir * rcpDirMin ) ) * inverseVP;

	vec3 rgbA = 0.5 * (
		texture2D( tex, fragCoord * inverseVP + dir * (1.0 / 3.0 - 0.5) ).xyz +
		texture2D( tex, fragCoord * inverseVP + dir * (2.0 / 3.0 - 0.5) ).xyz);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (
		texture2D( tex, fragCoord * inverseVP + dir * -0.5 ).xyz +
		texture2D( tex, fragCoord * inverseVP + dir * 0.5 ).xyz);

	float lumaB = dot( rgbB, luma );
	if ( (lumaB < lumaMin) || (lumaB > lumaMax) )
		color = vec4( rgbA, texColor.a );
	else
		color = vec4( rgbB, texColor.a );
	return color;
}

vec2 fixScreenTexCoord( vec2 st )
{
	float x = rpCurrentRenderSize.z / rpCurrentRenderSize.x;
	float y = rpCurrentRenderSize.w / rpCurrentRenderSize.y;
	return st * vec2( x, y );
}

float linearDepth()
{
	float ndcDepth = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
	return ndcDepth / gl_FragCoord.w;
}

vec4 mad_sat( vec4 a, vec2 b, vec4 c )
{
	vec4 bb = vec4( b.x, b.y, 1.0, 1.0 );

	return clamp( a * bb + c, 0, 1 );
}

vec4 blurThatShitAdvanced( sampler2D tex, vec2 coord, float distance )
{
	vec2 stepSize = vec2( distance / 50.0, distance / 50.0 );
	float d = 2.3;

	vec4 col = vec4( 0, 0, 0, 0 );
	float weight[ 5 ] = float[]( 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 );
	col += texture2D( texture1, frag.texcoord ) * weight[ 0 ];

	for ( int i = 1; i < 5; ++i )
	{
		col += texture2D( tex, coord + stepSize * i * d ) * weight[ i ];
		col += texture2D( tex, coord + stepSize * -i * d ) * weight[ i ];
	}

	return texture2D( tex, coord ) * 0.05 + vec4( col.rgb, 1 );
}

vec4 blurThatShit( sampler2D tex, vec2 coord, float distance )
{
	vec2 blurUpSt	= coord.xy + (vec2( 0.0, 0.02 ) * distance);
	vec2 blurUpLtSt = coord.xy + (vec2( -0.02, 0.02 ) * distance);
	vec2 blurUpRtSt = coord.xy + (vec2( 0.02, 0.02 ) * distance);

	vec2 blurDnSt   = coord.xy + (vec2( 0.0, -0.02 ) * distance);
	vec2 blurDnLtSt = coord.xy + (vec2( -0.02, -0.02 ) * distance);
	vec2 blurDnRtSt = coord.xy + (vec2( 0.02, -0.02 ) * distance);

	vec2 blurLtSt	= coord.xy + (vec2( -0.02, 0.0 ) * distance);
	vec2 blurRtSt	= coord.xy + (vec2( 0.02, 0.0 ) * distance);

	vec4 blurUp		= texture2D( tex, blurUpSt );
	vec4 blurUpLt	= texture2D( tex, blurUpLtSt );
	vec4 blurUpRt	= texture2D( tex, blurUpRtSt );
	vec4 blurDn		= texture2D( tex, blurDnSt );
	vec4 blurDnLt	= texture2D( tex, blurDnLtSt );
	vec4 blurDnRt	= texture2D( tex, blurDnRtSt );
	vec4 blurLt		= texture2D( tex, blurLtSt );
	vec4 blurRt		= texture2D( tex, blurRtSt );

	vec4 blurredColor = blurUp + blurDn + blurLt + blurRt + blurUpLt + blurUpRt + blurDnLt + blurDnRt;
	blurredColor /= 8.0;

	return blurredColor;
}

float simpleRand( vec2 co, float d )
{
	return fract( sin( dot( co.xy, vec2( 12.9898, 78.233 ) ) ) * 43758.5453 * d );
}

// Enable screen curvature.
#define CURVATURE

// Controls the intensity of the barrel distortion used to emulate the
// curvature of a CRT. 0.0 is perfectly flat, 1.0 is annoyingly
// distorted, higher values are increasingly ridiculous.
#define distortion 0.4

// Simulate a CRT gamma of 2.4.
#define inputGamma  2.4

// Compensate for the standard sRGB gamma of 2.2.
#define outputGamma 2.2

// Macros.
#define TEX2D(c) pow(texture2D(texture1, (c)), vec4(inputGamma))
#define PI 3.141592653589
#define one (vec2(1.0 / 480.0))
#define mod_factor (frag.texcoord.x * aspect_ratio.x)
//#define mod_factor (frag.texcoord.x * aspect_ratio.x)

/*
// The size of one texel, in texture-coordinates.
   vertexOut.one = 1.0 / sourceSize[0].xy;

   // Resulting X pixel-coordinate of the pixel we're drawing.
   vertexOut.mod_factor = texCoord.x * targetSize.x * aspect_ratio.x;

*/

// Apply radial distortion to the given coordinate.
vec2 radialDistortion( vec2 coord, float distortionRatio )
{
	vec2 cc = coord - 0.5;
	float dist = dot( cc, cc ) * distortion * distortionRatio;
	return (coord + cc * (1.0 + dist) * dist);
}

// Calculate the influence of a scanline on the current pixel.
//
// 'distance' is the distance in texture coordinates from the current
// pixel to the scanline in question.
// 'color' is the colour of the scanline at the horizontal location of
// the current pixel.
vec4 scanlineWeights( float distance, vec4 color )
{
	// The "width" of the scanline beam is set as 2*(1 + x^4) for
	// each RGB channel.
	vec4 wid = 2.0 + 2.0 * pow( color, vec4( 4.0 ) );

	// The "weights" lines basically specify the formula that gives
	// you the profile of the beam, i.e. the intensity as
	// a function of distance from the vertical center of the
	// scanline. In this case, it is gaussian if width=2, and
	// becomes nongaussian for larger widths. Ideally this should
	// be normalized so that the integral across the beam is
	// independent of its width. That is, for a narrower beam
	// "weights" should have a higher peak at the center of the
	// scanline than for a wider beam.
	vec4 weights = vec4( distance / 0.3 );
	return 1.4 * exp( -pow( weights * inversesqrt( 0.5 * wid ), wid ) ) / (0.6 + 0.2 * wid);
}

vec4 CRTFilter( vec2 texCoord, float distortionRatio )
{
	float aspect_ratio = 4.0 / 3.0;
//	float aspect_ratio = 16.0 / 9.0;

	// Here's a helpful diagram to keep in mind while trying to
	// understand the code:
	//
	//  |      |      |      |      |
	// -------------------------------
	//  |      |      |      |      |
	//  |  01  |  11  |  21  |  31  | <-- current scanline
	//  |      | @    |      |      |
	// -------------------------------
	//  |      |      |      |      |
	//  |  02  |  12  |  22  |  32  | <-- next scanline
	//  |      |      |      |      |
	// -------------------------------
	//  |      |      |      |      |
	//
	// Each character-cell represents a pixel on the output
	// surface, "@" represents the current pixel (always somewhere
	// in the bottom half of the current scan-line, or the top-half
	// of the next scanline). The grid of lines represents the
	// edges of the texels of the underlying texture.

	// Texture coordinates of the texel containing the active pixel.
#ifdef CURVATURE
	vec2 xy = radialDistortion( texCoord, distortionRatio );
#else
	vec2 xy = texCoord;
#endif

	// Of all the pixels that are mapped onto the texel we are
	// currently rendering, which pixel are we currently rendering?
	vec2 ratio_scale = xy * frag.texcoord.xy - vec2( 0.5 );
	vec2 uv_ratio = fract( ratio_scale );

	// Snap to the center of the underlying texel.
//	xy.y = (floor( ratio_scale.y ) + 0.5) / frag.texcoord.y;

	// Calculate the effective colour of the current and next
	// scanlines at the horizontal location of the current pixel.
	vec4 col = TEX2D( xy );
	vec4 col2 = TEX2D( xy + vec2( 0.0, one.y ) );

	// Calculate the influence of the current and next scanlines on
	// the current pixel.
	vec4 weights = scanlineWeights( uv_ratio.y, col );
	vec4 weights2 = scanlineWeights( 1.0 - uv_ratio.y, col2 );
	vec3 mul_res = (col * weights + col2 * weights2).rgb;

	// dot-mask emulation:
	// Output pixels are alternately tinted green and magenta.
	vec3 dotMaskWeights = mix(
		vec3( 1.0, 0.7, 1.0 ),
		vec3( 0.7, 1.0, 0.7 ),
		floor( mod( mod_factor, 2.0 ) )
	);

	mul_res *= dotMaskWeights;

	return vec4( pow( mul_res, vec3( 1.0 / outputGamma ) ), 1.0 );
}

void main( void )
{
	vec4 color;
	if ( shaderParm0.x > 0 )
	{
		vec2 resolution = shaderParm0.xy;
		vec2 fragCoord = frag.texcoord * resolution;
		vec2 inverseVP = 1.0 / resolution.xy;
		vec2 nw = (fragCoord + vec2( -1.0, -1.0 )) * inverseVP;
		vec2 ne = (fragCoord + vec2( 1.0, -1.0 )) * inverseVP;
		vec2 sw = (fragCoord + vec2( -1.0, 1.0 )) * inverseVP;
		vec2 se = (fragCoord + vec2( 1.0, 1.0 )) * inverseVP;
		vec2 m = vec2( fragCoord * inverseVP );

		color = fxaa( texture1, fragCoord, resolution, nw, ne, sw, se, m );
	}
	else
	{
		color = texture2D( texture1, frag.texcoord );
	}

	float health = shaderParm1.x;
	float heartbeat = shaderParm1.y;

	// blue tint
	color.b *= 1.25;
	color.g *= 1.1;

	// load the distortion map
	vec4 mask = texture2D( texture1, frag.texcoord );
	mask *= color;

	mask.xy -= 0.001f;

	// load the filtered normal map and convert to -1 to 1 range
	vec4 localNormal = texture2D( bloodNormal, frag.texcoord.xy );

	//localNormal.x = localNormal.a;
	localNormal = localNormal * 2 - 1;
	localNormal = (localNormal * mask * 0.7) + (localNormal * 0.3);
	localNormal += localNormal * (1.0 - heartbeat);
	localNormal *= (1.0 - health);

	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec4 screenTexCoord = vec4( gl_FragCoord.x / rpCurrentRenderSize.z, gl_FragCoord.y / rpCurrentRenderSize.w, 0, 0 ); //vposToScreenPosTexCoord( fragment.position.xy );
	screenTexCoord = vec4( frag.texcoord, 1.0, 1.0 );

	screenTexCoord = mad_sat( localNormal, frag.texcoord, screenTexCoord );

	vec4 nvcolor    = vec4( 0.62, 1.0, 0.59, 1.0 );
	vec4 redcolor   = vec4( 1.0, 0.0, 0.0, 1.0 );
	vec4 watercolor = vec4( 0.32, 0.64, 0.8, 1.0 ) * shaderParm1.w;

	float blur = texture2D( bloodBlur, frag.texcoord ).x;
	blur = pow( blur, 0.7 ) * (1.0 - heartbeat) * 1.0;
	blur *= (1.0 - health);
	blur += sqrt(sqrt(shaderParm1.w)) * 0.666;

	vec4 blurredRender		= blurThatShitAdvanced( texture1, frag.texcoord, blur );
	vec4 blurredDistorted	= blurThatShitAdvanced( texture1, screenTexCoord.xy, blur / 2.0 );
	vec4 blurredFinal		= (blurredRender * (heartbeat)) + (blurredDistorted * (1.0 - heartbeat));

	result = blurredFinal * (1.0 - health*heartbeat) + color * health*heartbeat;

	result.rgb += (watercolor.rgb / 12.0);

	if ( shaderParm1.z > 0.2 )
	{
		result = result * 0.3 + CRTFilter( frag.texcoord, pow( shaderParm1.z, 32.0 ) * 0.666 ) * 0.7;
	//	result = CRTFilter( frag.texcoord, pow(shaderParm1.z, 15.0) * 0.666 );
		result = ((vec4( result.r ) + vec4( result.g ) + vec4( result.b )) / 3.0);
		result /= 3.0;
		result *= 8.0;
		result.x = pow( result.x, 0.7 );
		result.y = pow( result.y, 0.7 );
		result.z = pow( result.z, 0.7 );
		result *= nvcolor;

		if ( shaderParm1.z <= 1.0 )
		{
			result = vec4( 1.0 ) * (1.0 - sin( (6.3 * shaderParm1.z) + 1.26 )) + result * (2.0 - sin( (6.3 * shaderParm1.z) + 1.26 ));
		}
	
		result += simpleRand( frag.texcoord, blur + 0.004 ) * nvcolor * 0.3;
	}

	if ( health < 0.4 )
	{
		result.gb *= 2.0 * health;
	}

	result.rgb = pow( result.rgb, vec3( 1.0 / 1.0 ) );

	float bloomFactor = shaderParm0.z * (1.0 + shaderParm1.z);
	if ( bloomFactor > 0 )
	{
		vec4 bloom = texture2D( texture2, frag.texcoord );

		if ( shaderParm1.z > 0.2 )
		{
			bloom = ((vec4( bloom.r ) + vec4( bloom.g ) + vec4( bloom.b )) / 3.0);
			bloom *= nvcolor;

			bloomFactor *= 1.0 + pow( shaderParm1.z, 128.0 ) * 3.0;
		}

		result += bloom * bloomFactor;
	}
 //   result = vec4(blur);
 //   result = localNormal;
 //   result = color;

}
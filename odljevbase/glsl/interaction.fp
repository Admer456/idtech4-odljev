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
#include "shading.inc"

layout( binding = 1 ) uniform sampler2D normalMap;
layout( binding = 2 ) uniform sampler2D lightFalloff;
layout( binding = 3 ) uniform sampler2D lightTexture;
layout( binding = 4 ) uniform sampler2D diffuseMap;
layout( binding = 5 ) uniform sampler2D specularMap;

layout( binding = 8 ) uniform sampler2D normalMapB;
layout( binding = 9 ) uniform sampler2D diffuseMapB;
layout( binding = 10 ) uniform sampler2D specularMapB;

layout( binding = 11 ) uniform sampler2D normalMapC;
layout( binding = 12 ) uniform sampler2D diffuseMapC;
layout( binding = 13 ) uniform sampler2D specularMapC;



in vs_output
{
	vec4 color;
	vec2 texNormal;
	vec2 texDiffuse;
	vec2 texSpecular;
	vec4 texLight;
	vec3 L;
	vec3 V;
	vec3 H;
	vec4 shadow[ 6 ];
	vec3 toLightOrigin;
	float depth;
	vec4 worldspacePosition;
	
	vec3 cubecoord;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;

	vec3 vc;
} frag;

#include "shadows.inc"

out vec4 result;

float DistributionGGX( vec3 N, vec3 H, float roughness )
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max( dot( N, H ), 0.0 );
    float NdotH2 = NdotH * NdotH;
    float PI	 = 3.14159265359;

    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
  
    return (num / denom);
}

float GeometrySchlickGGX( float NdotV, float roughness )
{
	float r = (roughness + 1.0);
	float k = (r*r) / 16.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;

	return (num / denom);
}

float GeometrySmith( vec3 N, vec3 V, vec3 L, float roughness )
{
	float NdotV = max( dot( N, V ), 0.0 );
	float NdotL = max( dot( N, L ), 0.0 );
	float ggx2 = GeometrySchlickGGX( NdotV, roughness );
	float ggx1 = GeometrySchlickGGX( NdotL, roughness );

	return (ggx1 * ggx2);
}

vec3 fresnelSchlick( float cosTheta, vec3 F0, float roughness )
{
	return F0 + (1.0 - F0) * pow( 1.0 - cosTheta, 5.0 * pow( 1.0 - roughness, 2.0 ) );
}

vec4 diffuse( vec2 texcoord, vec3 N, vec3 L, sampler2D parm_diffuseMap )
{
	vec4 ret = texture( parm_diffuseMap, texcoord ) * rpDiffuseColor * halflambert( N, L );
	ret.rgb = pow( ret.rgb, vec3( 1.0 / 2.2 ) );
	return ret;
}

vec4 specular( vec2 texcoord, vec3 N, vec3 L, vec3 V, sampler2D parm_specularMap )
{
	vec4 specTex = texture( parm_specularMap, texcoord );

	specTex.g = specTex.r;
	specTex.b = specTex.r;

	float smoothness = 1.0 - specTex.r;

	vec4 spec = specTex;

	if ( rpShading == 1 )
	{
		spec *= phong( N, L, V, rpSpecularExp * specTex.r );
	}
	else
	{
		vec3 H = normalize( frag.H );
		spec *= blinn( N, H, rpSpecularExp * (smoothness * 0.001) );
	}

	return spec * rpSpecularColor;
}

vec3 normal( vec2 texcoord, sampler2D parm_normalMap )
{
#if 0
	vec3 N = normalize( 2.0 * texture( normalMap, texcoord ).rgb - 1.0 );
#else
	const int NORMAL_RGB = 0;
	const int NORMAL_RxGB = 1;
	const int NORMAL_AG = 2;
	const int NORMAL_RG = 3;

	vec3 N;

	if ( rpNormalMapEncoding == NORMAL_RxGB )
	{
		N = normalize( 2.0 * texture( parm_normalMap, texcoord ).rgb - 1.0 );
	}
	else if ( rpNormalMapEncoding == NORMAL_AG )
	{
		N = 2.0 * texture( parm_normalMap, texcoord ).agb - 1.0;
		N.z = sqrt( 1.0 - N.x * N.x - N.y * N.y );
	}
	else if ( rpNormalMapEncoding == NORMAL_RG )
	{
		N = 2.0 * texture( parm_normalMap, texcoord ).rgb - 1.0;
		N.z = sqrt( 1.0 - N.x * N.x - N.y * N.y );
	}
	else /*if(rpNormalMapEncoding == NORMAL_RGB)*/
	{
		N = normalize( 2.0 * texture( parm_normalMap, texcoord ).rgb - 1.0 );
	}

#endif
	return N;
}

vec4 shadow()
{
	vec4 shadowness = vec4( 1, 1, 1, 1 );

	if ( rpShadowMappingMode == 1 )
	{
	#if 0
		float softness = (0.003 * rpShadowParams.x);
	#else
		float lightDistance = length( frag.toLightOrigin );
		lightDistance = lightDistance * 0.5 + pow( lightDistance, 1.5 )*0.5;
		float softness = (0.0001 * rpShadowParams.x) * ((lightDistance / rpShadowParams.w)*32.0);
	#endif      
		shadowness = pointlightShadow( frag.shadow, frag.toLightOrigin, vec2( softness, softness ) );
	}
	else if ( rpShadowMappingMode == 2 )
	{
		float softness = (0.007 * rpShadowParams.x);
		shadowness = projectedShadow( frag.shadow[ 0 ], vec2( softness, softness ) );
	}
	else if ( rpShadowMappingMode == 3 )
	{
		shadowness = parallelShadow( frag.shadow, -frag.depth, vec2( rpShadowParams.x * 0.0095, rpShadowParams.x * 0.0095 ) );
	}

	return shadowness;
}

vec3 reflect( vec3 I, vec3 N )
{
	return I - 2.0 * dot( N, I ) * N;
}

//vec4 realtime_cubemap()
//{
//	vec3 localNormal = 2.0 * texture2D( normalMap, frag.texDiffuse.st ).agb - 1.0;
//	vec3 normal = normalize( frag.normal );
//	vec3 tangent = normalize( frag.tangent );
//	vec3 binormal = normalize( frag.binormal );
//	mat3 TBN = mat3( tangent, binormal, normal );
//
//	vec3 r = reflect( frag.cubecoord, TBN * localNormal );
//	return texture( rtcm, toOpenGlCorrdinates( r ) );
//}

vec4 CalculateTexture_BlendNone( sampler2D parm_diffuseMap, sampler2D parm_specularMap, sampler2D parm_bumpMap )
{
	const float PI = 3.14159265359;

	vec3	V = normalize( frag.V );
	vec3	L = normalize( frag.L );
	vec2	offset = parallaxOffset( parm_specularMap, frag.texSpecular.st, V );
	vec3	N = normal( frag.texNormal + offset, parm_bumpMap );

	float	NdotL = max( dot( N, L ), 0.0 );

	// diffuse colour
	vec4	Cd = diffuse( frag.texDiffuse + offset, N, L, parm_diffuseMap ) * texture2D( parm_diffuseMap, frag.texDiffuse );
	// specular colour
	vec4	Cs = specular( frag.texSpecular + offset, N, L, V, parm_specularMap );

	// Half-angle
	vec3	H = normalize( frag.H );
	vec3	NdotH = vec3( clamp( dot( N, H ), 0.0, 1.0 ) );

	float	NdotV = max( dot( N, V ), 0.0 );

	// final, resulting pixel colour
	vec4	ret = vec4( 0.0 );

	// determine the RMAO map
	vec4	specTex = texture2D( parm_specularMap, frag.texSpecular );
	vec4	roughness = vec4( specTex.r ); 
	vec4	metallic = vec4( specTex.g ); 
	vec4	aocclusion = vec4( specTex.b );

	// light stuff
	float	lightDistance = length( frag.toLightOrigin );
	vec4	Cl = texture2DProj( lightTexture, frag.texLight.xyw ) * texture2D( lightFalloff, vec2( frag.texLight.z, 0.5 ) ); 
	
	float	attenuation = 1.0 / (lightDistance * lightDistance);
			attenuation = pow( attenuation, 2.2 );

	vec4	radiance = Cl * attenuation;
	vec4	Lo = vec4( 0.0 );

	// base metallic
	vec3	F0 = vec3( 0.04 );
			F0 = mix( F0, Cd.xyz, metallic.xyz );

	// Cook-Torrance BRDF lighting model
	float	NDF = DistributionGGX( N, H, roughness.r );
	float	G = GeometrySmith( N, V, L, roughness.r );
	vec3	F = fresnelSchlick( max( dot( H, V ), 0.0 ), F0, roughness.r ); 

	vec3	kS = F; // specular fracion
	vec3	kD = vec3( 1.0 ) - kS; // diffuse fraction
			kD *= (1.0 - metallic.r);

	// specular reflection for PBR
	vec3	numerator = NDF * G * F;
	float	denominator = 4.0 * max( dot( N, V ), 0.0 ) * max( dot( N, L ), 0.0 );
			denominator = sqrt( denominator );

	vec3	pbr = numerator / max( denominator, 0.00001 );
			Lo += (((vec4( kD, 1.0 ) * Cd) / PI) + (Cs*Cl)) * radiance * NdotL;

	vec3	ambient = vec3( -0.1 ) * Cd.xyz * aocclusion.xyz;

			// final result
			ret = (frag.color * Cl * NdotL * Cd);
			ret += (vec4( pbr, 1.0 ) * (Cs * (1.0 - (roughness * roughness)) * Cl));
						
			ret *= texture2DProj( lightTexture, frag.texLight.xyw );
			ret *= texture2D( lightFalloff, vec2( frag.texLight.z, 0.5 ) );
			ret *= shadow();

			// apply AO
//			ret *= aocclusion;

	return ret;
}

vec4 CalculateTexture_BlendTwo( void )
{
	vec4 colA = CalculateTexture_BlendNone( diffuseMap, specularMap, normalMap );
	vec4 colB = CalculateTexture_BlendNone( diffuseMapB, specularMapB, normalMapB );
	vec4 colFinal;

	float VC = frag.vc.r;

//	colA *= frag.vc.r;
//	colB.rgb *= OneMinusVC;
//	colA += colB;

	colFinal = colA * VC + colB * (1.0 - VC);
	
	return colFinal;
}

vec4 CalculateTexture_BlendThree( void )
{
	vec4 colA = CalculateTexture_BlendNone( diffuseMap, specularMap, normalMap );
	vec4 colB = CalculateTexture_BlendNone( diffuseMapB, specularMapB, normalMapB );
	vec4 colC = CalculateTexture_BlendNone( diffuseMapC, specularMapC, normalMapC );
	vec4 colFinal;

//	vec4 colTest = texture( diffuseMap, frag.texDiffuse );

	// vertex colour preview	return vec4(frag.vc.rgb, 1.0);

	colA *= frag.vc.r;
	colB *= frag.vc.g;
	colC *= frag.vc.b;

	colFinal = colA + colB + colC;

	return colFinal;
}

void main( void )
{
	if ( shaderParm1.x == 0 )
		result = CalculateTexture_BlendNone( diffuseMap, specularMap, normalMap );

	else if ( shaderParm1.x == 1 )
		result = CalculateTexture_BlendTwo();

	else if ( shaderParm1.x == 2 )
		result = CalculateTexture_BlendThree();
}
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

layout(binding = 6) uniform sampler2DShadow shadowMap;
layout(binding = 7) uniform sampler2D jitterMap;

float isOccluded(sampler2DShadow map, vec2 coord, vec2 offset, float ref)
{
  float f = texture(map, vec3(coord + offset, ref));
  return 1 - f;
}

mat2 getJitter()
{
    vec2 jitter = texture(jitterMap, frag.worldspacePosition.xy + frag.worldspacePosition.zz ).xy;
    float cosinus = (jitter.x * 2) - 1;
    float sinus = (jitter.y * 2) - 1;

    return mat2(cosinus, sinus, -sinus, cosinus);  
}

#define DECLARE_POISSON_SAMPLE(KernelSize) float poissonSample(vec2 kernel[KernelSize], vec3 pos, vec2 texsize) { \
    mat2 rot = getJitter(); \
\
    float bias = 0.000; \
    float ret = 0; \
    for(int i=0;i<KernelSize; ++i) { \
       vec2 offset = (rot * kernel[i]) * texsize; \
       ret += isOccluded(shadowMap, pos.st, offset, pos.z - bias); \
    } \
    return ret/KernelSize; \
}

DECLARE_POISSON_SAMPLE(6)
DECLARE_POISSON_SAMPLE(9)
DECLARE_POISSON_SAMPLE(12)
DECLARE_POISSON_SAMPLE(15)

float getShadow(vec4 pos, vec2 coordScale, vec2 coordOffset, vec2 softness)
{   
    pos = pos / pos.w;

    pos.x = pos.x/2.0 + 0.5;
    pos.y = pos.y/2.0 + 0.5;
    pos.z = pos.z/2.0 + 0.5;   

    pos.x *= coordScale.x; 
    pos.y *= coordScale.y; 

    pos.x += coordOffset.x;
    pos.y += coordOffset.y;

//#define FILTER_POISSON6
#define FILTER_POISSON9
//#define FILTER_POISSON12    
//#define FILTER_POISSON15
 
  vec2 texsize = coordScale * softness;

#if defined(FILTER_POISSON6)
    const vec2 kernel[6] = vec2[](
      vec2(-0.4397209, -0.3645),
      vec2(-0.6508484, 0.4861422),
      vec2(0.3237945, -0.1611787),
      vec2(0.008111006, 0.7074075),
      vec2(0.09966598, -0.8482586),
      vec2(0.8881027, 0.4371758)
    );

    return poissonSample(kernel, pos.xyz, texsize);

#elif defined(FILTER_POISSON9)
    const vec2 kernel[9] = vec2[](
      vec2(0.01946597, -0.9665399),
      vec2(-0.2643075, -0.2529854),
      vec2(-0.6608061, -0.5971643),
      vec2(0.5533955, -0.560185),
      vec2(-0.6723549, 0.1494021),
      vec2(0.8941639, 0.0498636),
      vec2(0.3074718, 0.2643286),
      vec2(-0.2686456, 0.9057658),
      vec2(0.5373921, 0.8100548)
    );

    return poissonSample(kernel, pos.xyz, texsize);
   
#elif defined(FILTER_POISSON12)
    const vec2 kernel[12] = vec2[](
      vec2(0.3665185, -0.903574),
      vec2(0.6162645, -0.4545356),
      vec2(-0.2486034, -0.6077372),
      vec2(0.05140426, -0.2567643),
      vec2(-0.2610345, 0.07457025),
      vec2(0.2511628, 0.2595751),
      vec2(-0.6152166, -0.318685),
      vec2(0.8690009, -0.03180576),
      vec2(-0.9257081, 0.1798257),
      vec2(-0.1923947, 0.5614256),
      vec2(0.2714329, 0.7173462),
      vec2(-0.7179349, 0.6915267)
    );

    return poissonSample(kernel, pos.xyz, texsize);
#elif defined(FILTER_POISSON15)
    const vec2 kernel[15] = vec2[](
      vec2(0.3886908, 0.3622644),
      vec2(0.01367045, 0.7141614),
      vec2(-0.1952952, 0.261395),
      vec2(0.167147, -0.08671506),
      vec2(0.5340464, -0.3375779),
      vec2(0.7503276, 0.1110756),
      vec2(-0.02910505, -0.5694501),
      vec2(0.3916242, -0.7194775),
      vec2(-0.3699134, -0.1326576),
      vec2(0.49934, 0.835238),
      vec2(-0.8104502, 0.05760445),
      vec2(-0.571758, 0.6056085),
      vec2(-0.8419966, -0.3448843),
      vec2(-0.5124402, -0.6106064),
      vec2(-0.2231808, -0.9203894)
    );

    return poissonSample(kernel, pos.xyz, texsize);
#else    
    return isOccluded(shadowMap, pos.st, vec2(0,0), pos.z);
#endif  
}

vec4 projectedShadow(vec4 pos, vec2 softness)
{
  float f = getShadow(pos, rpShadowCoords[0].xy, rpShadowCoords[0].zw, softness);

  f = mix(1, rpShadowParams.y, f);

  return vec4(f,f,f,f);
}

vec4 getParallelShadow(vec4 pos[6], int index, vec2 softness)
{
  vec4 debugColors[6] = vec4[](
    vec4(1,1,1,1),
    vec4(1,0,0,1),
    vec4(1,1,0,1),
    vec4(0,1,0,1),
    vec4(0,1,1,1),
    vec4(0,0,1,1)
  );

  float f = getShadow(pos[index], rpShadowCoords[index].xy, rpShadowCoords[index].zw, softness);

  float ret = mix(1, rpShadowParams.y, f);

#if 0
  return debugColors[index] * ret;
#else
  return vec4(ret, ret, ret, 1);
#endif    
}

vec4 parallelShadow(vec4 pos[6], float distance, vec2 softness)
{ 
  const float referenceSize = 256;  

  int i=0;

  vec4 nextCascade = vec4(1,1,1,1);
  float nextfraction = 0; 

  for(; i<5; ++i)
  {
    float t = 10 * (i+1);//rpCascadeDistances[i+1] * 0.1;

    if(distance < (rpCascadeDistances[i] - t))
    {
      break;
    }

    if(distance < (rpCascadeDistances[i] + t))
    {
      vec2 shadowmapSize = rpShadowMapSize[i+1].zw;
      nextCascade = getParallelShadow(pos, i+1, softness * vec2(referenceSize / shadowmapSize.x, referenceSize / shadowmapSize.y));
      nextfraction = (distance - (rpCascadeDistances[i] - t)) / (2*t);
      break;
    }    
  }

  vec2 shadowmapSize = rpShadowMapSize[i].zw;

  softness = softness * vec2(referenceSize / shadowmapSize.x, referenceSize / shadowmapSize.y);

  return mix(getParallelShadow(pos, i, softness), nextCascade, nextfraction);
  //return getParallelShadow(pos, 5, softness * softscale[5]);  
}

vec4 pointlightShadow(vec4 pos[6], vec3 toLightOrigin, vec2 softness)
{  
  vec3 d = toLightOrigin;

  int side = 0;
  float l = d.x;


  if( d.y > l ) {
    side = 2;
    l = d.y;
  }
  if( d.z > l ) {
    side = 4;
    l = d.z;
  }
  if( -d.x > l ) {
    side = 1;
    l = -d.x;
  }
  if( -d.y > l ) {
    side = 3;
    l = -d.y;
  }
  if( -d.z > l ) {
    side = 5;
    l = -d.z;
  }   

  float f = getShadow(pos[side], rpShadowCoords[side].xy, rpShadowCoords[side].zw, softness);  

  f = mix(1, rpShadowParams.y, f);

  return vec4(f,f,f,f);   
}
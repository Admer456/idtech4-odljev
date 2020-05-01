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

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform sampler2D texture1;

in vs_output
{
  vec4 color;
  vec2 texcoord;
} frag;

out vec4 result;

vec2 fixScreenTexCoord(vec2 st)
{
  float x = rpCurrentRenderSize.z / rpCurrentRenderSize.x;
  float y = rpCurrentRenderSize.w / rpCurrentRenderSize.y;
  return st * vec2(x, y);  
}

float linearDepth()
{
  float ndcDepth = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
  return ndcDepth / gl_FragCoord.w;
}

float getDistanceScale()
{
  float maxDistance = 800;

  float depth = linearDepth();
  depth = clamp(depth, 0, maxDistance);

  return (maxDistance - depth)/maxDistance;
}

vec4 test4()
{
  float distanceScaleMagnitude = getDistanceScale();  

  vec2 magnitude = shaderParm1.xy * 0.008;
  vec2 scroll = shaderParm0.xy * 0.3;

  vec2 uv = gl_FragCoord.xy * (1.0 / rpCurrentRenderSize.zw);
  
  vec2 uvDist = uv + scroll;

  vec4 distortion = (texture2D(texture1, uvDist * 2.5) - 0.5) * 2.0;
  
  uv += distortion.xy * magnitude * distanceScaleMagnitude;  

  return texture2D(texture0, fixScreenTexCoord(uv));
}

void main(void)
{
  result = test4();
}
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

class idDrawVert;

struct fhSimpleVert {
  idVec3 xyz;
  idVec2 st;
  byte color[4];

  void SetColor(const idVec3& v);
  void SetColor(const idVec4& v);

  static const int xyzOffset = 0;
  static const int texcoordOffset = 12;
  static const int colorOffset = 20;
};

static_assert(sizeof(fhSimpleVert) == 24, "unexpected size of simple vertex, due to padding?");


class fhImmediateMode
{
public:
  explicit fhImmediateMode(bool geometryOnly = false);
  ~fhImmediateMode();

  void SetTexture(idImage* texture);

  void Begin(GLenum mode);
  void TexCoord2f(float s, float t);
  void TexCoord2fv(const float* v);
  void Color3fv(const float* c);
  void Color3f(float r, float g, float b);
  void Color4f(float r, float g, float b, float a);
  void Color4fv(const float* c);
  void Color4ubv(const byte* bytes);
  void Vertex3fv(const float* c);
  void Vertex3f(float x, float y, float z);
  void Vertex2f(float x, float y);
  void End();

  void Sphere(float radius, int rings, int sectors, bool inverse = false);

  GLenum getCurrentMode() const { return currentMode; }

  static void AddTrianglesFromPolygon(fhImmediateMode& im, const idVec3* xyz, int num);

  static void Init();
  static void ResetStats();
  static int DrawCallCount();
  static int DrawCallVertexSize();
private:
  bool geometryOnly;
  float currentTexCoord[2];
  GLenum currentMode;
  byte currentColor[4];
  idImage* currentTexture;

  int drawVertsUsed;

  static int drawCallCount;
  static int drawCallVertexSize;
};

class fhLineBuffer
{
public:
  fhLineBuffer();
  ~fhLineBuffer();

  void Add(idVec3 from, idVec3 to, idVec4 color);
  void Add(idVec3 from, idVec3 to, idVec3 color);
  void Clear();
  void Commit();

private:
  int verticesAllocated;
  int verticesUsed;
  fhSimpleVert* vertices;
};

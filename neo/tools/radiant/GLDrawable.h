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
#pragma once

#ifdef None
#undef None
#endif

enum class MouseButton {
	None,
	Left,
	Right,
	Middle
};

class idGLDrawable {
public:
	idGLDrawable();
	~idGLDrawable() {};
	virtual void draw( int x, int y, int w, int h );
	virtual void setMedia( const char *name ){}
	virtual void buttonDown( MouseButton button, float x, float y );
	virtual void buttonUp( MouseButton button, float x, float y );
	virtual void mouseMove( float x, float y );
	virtual int getRealTime() { return realTime; };
	virtual bool ScreenCoords() {
		return true;
	}
	void SetRealTime( int i ) {
		realTime = i;
	}
	virtual void Update() {};
	float getScale() {
		return scale;
	}
	void setScale( float f ) {
		scale = f;
	}
protected:
	float scale;
	float xOffset;
	float yOffset;
	float zOffset;
	float pressX;
	float pressY;
	bool  handleMove;
	MouseButton button;
	int realTime;
};

class idGLDrawableWorld : public idGLDrawable {
public:
	idGLDrawableWorld();
	~idGLDrawableWorld();
	void AddTris( srfTriangles_t *tris, const idMaterial *mat );
	virtual void draw( int x, int y, int w, int h ) override;
	virtual void mouseMove( float x, float y ) override;
	virtual void Update() override { worldDirty = true; };
	virtual void buttonDown( MouseButton button, float x, float y ) override;
	void InitWorld();
protected:

	void InitLight( const idVec3& position );

	idRenderWorld *world;
	idRenderModel *worldModel;
	qhandle_t	worldModelDef;
	qhandle_t	lightDef;
	qhandle_t   modelDef;
	float       light;

	//model
	bool worldDirty;
	idStr skinStr;
	idQuat rotation;
	idVec3 lastPress;
	float radius;
	idVec4 rect;
};

class idGLDrawableMaterial : public idGLDrawableWorld {
public:
	idGLDrawableMaterial( const idMaterial *mat ) {
		material = mat;
		scale = 2.0;
		light = 1.0;
		worldDirty = true;
	}

	idGLDrawableMaterial() {
		material = NULL;
		light = 1.0;
		worldDirty = true;
		realTime = 50;
	}

	virtual void setMedia( const char *name ) override;
	virtual void draw( int x, int y, int w, int h ) override;

	//protected:
	const idMaterial *material;
};

class idGLDrawableModel : public idGLDrawableWorld {
public:
	idGLDrawableModel( const char *name );
	idGLDrawableModel();

	virtual void setMedia( const char *name ) override;

	virtual void draw( int x, int y, int w, int h ) override;
	virtual bool ScreenCoords() override {
		return false;
	}
	void SetSkin( const char *skin );
};

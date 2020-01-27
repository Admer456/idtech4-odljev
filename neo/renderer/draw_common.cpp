/*
===========================================================================

Doom 3 GPL Source Code
Copyright (C) 1999-2011 id Software LLC, a ZeniMax Media company.

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
#include "ImmediateMode.h"
#include "RenderList.h"
#include "Framebuffer.h"

/*
=====================
RB_BakeTextureMatrixIntoTexgen
=====================
*/
void RB_BakeTextureMatrixIntoTexgen( idPlane lightProject[3], const float *textureMatrix ) {
	float	genMatrix[16];
	float	final[16];

	genMatrix[0] = lightProject[0][0];
	genMatrix[4] = lightProject[0][1];
	genMatrix[8] = lightProject[0][2];
	genMatrix[12] = lightProject[0][3];

	genMatrix[1] = lightProject[1][0];
	genMatrix[5] = lightProject[1][1];
	genMatrix[9] = lightProject[1][2];
	genMatrix[13] = lightProject[1][3];

	genMatrix[2] = 0;
	genMatrix[6] = 0;
	genMatrix[10] = 0;
	genMatrix[14] = 0;

	genMatrix[3] = lightProject[2][0];
	genMatrix[7] = lightProject[2][1];
	genMatrix[11] = lightProject[2][2];
	genMatrix[15] = lightProject[2][3];

	myGlMultMatrix( genMatrix, textureMatrix, final );

	lightProject[0][0] = final[0];
	lightProject[0][1] = final[4];
	lightProject[0][2] = final[8];
	lightProject[0][3] = final[12];

	lightProject[1][0] = final[1];
	lightProject[1][1] = final[5];
	lightProject[1][2] = final[9];
	lightProject[1][3] = final[13];
}


/*
=============================================================================================

SHADER PASSES

=============================================================================================
*/


//========================================================================


/*
==================
RB_STD_FogAllLights
==================
*/
void RB_STD_FogAllLights(void) {
	if (r_skipFogLights.GetBool() || r_showOverDraw.GetInteger() != 0
		|| backEnd.viewDef->isXraySubview /* dont fog in xray mode*/
		) {
		return;
	}

	RB_LogComment("---------- RB_STD_FogAllLights ----------\n");

	glDisable(GL_STENCIL_TEST);

	for (const viewLight_t* vLight = backEnd.viewDef->viewLights; vLight; vLight = vLight->next) {
		if (!vLight->lightShader->IsFogLight() && !vLight->lightShader->IsBlendLight()) {
			continue;
		}

		if (vLight->lightShader->IsFogLight()) {
			RB_GLSL_FogPass(*vLight);
		}
		else if (vLight->lightShader->IsBlendLight()) {
			RB_GLSL_BlendLight(*vLight);
		}
		glDisable(GL_STENCIL_TEST);
	}

	glEnable(GL_STENCIL_TEST);
}

//=========================================================================================

/*
==================
RB_STD_LightScale

Perform extra blending passes to multiply the entire buffer by
a floating point value
==================
*/
void RB_STD_LightScale( void ) {
	float	v, f;

	if ( backEnd.overBright == 1.0f ) {
		return;
	}

	if ( r_skipLightScale.GetBool() ) {
		return;
	}

	RB_LogComment( "---------- RB_STD_LightScale ----------\n" );

	// the scissor may be smaller than the viewport for subviews
	if ( r_useScissor.GetBool() ) {
		glScissor( backEnd.viewDef->viewport.x1 + backEnd.viewDef->scissor.x1,
			backEnd.viewDef->viewport.y1 + backEnd.viewDef->scissor.y1,
			backEnd.viewDef->scissor.x2 - backEnd.viewDef->scissor.x1 + 1,
			backEnd.viewDef->scissor.y2 - backEnd.viewDef->scissor.y1 + 1 );
		backEnd.currentScissor = backEnd.viewDef->scissor;
	}

	// full screen blends
	GL_ModelViewMatrix.LoadIdentity();

	GL_ProjectionMatrix.Push();
	GL_ProjectionMatrix.LoadIdentity();
	GL_ProjectionMatrix.Ortho(0, 1, 0, 1, -1, 1);

	GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_SRC_COLOR );
	GL_Cull( CT_TWO_SIDED );	// so mirror views also get it
	globalImages->BindNull(0);
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_STENCIL_TEST );

	v = 1;
	while ( idMath::Fabs( v - backEnd.overBright ) > 0.01 ) {	// a little extra slop
		f = backEnd.overBright / v;
		f /= 2;
		if ( f > 1 ) {
			f = 1;
		}

		v = v * f * 2;

		fhImmediateMode im;
		im.Color3f( f, f, f );
		im.Begin( GL_QUADS );
		im.Vertex2f( 0,0 );
		im.Vertex2f( 0,1 );
		im.Vertex2f( 1,1 );
		im.Vertex2f( 1,0 );
		im.End();
	}

	GL_ProjectionMatrix.Pop();
	glEnable( GL_DEPTH_TEST );
	GL_Cull( CT_FRONT_SIDED );
}

//=========================================================================================

/*
=============
RB_STD_DrawView

=============
*/
void	RB_STD_DrawView( void ) {

	fhTimeElapsed timeElapsed(&backEnd.stats.totaltime);

	drawSurf_t	 **drawSurfs;
	int			numDrawSurfs;

	RB_LogComment( "---------- RB_STD_DrawView ----------\n" );

	backEnd.depthFunc = GLS_DEPTHFUNC_EQUAL;

	drawSurfs = (drawSurf_t **)&backEnd.viewDef->drawSurfs[0];
	numDrawSurfs = backEnd.viewDef->numDrawSurfs;

	// clear the z buffer, set the projection matrix, etc
	RB_BeginDrawingView();

	// decide how much overbrighting we are going to do
	RB_DetermineLightScale();

	// fill the depth buffer and clear color buffer to black except on
	// subviews
	RB_GLSL_FillDepthBuffer(drawSurfs, numDrawSurfs);

	if (backEnd.viewDef->viewEntitys) {
		auto source = fhFramebuffer::GetCurrentDrawBuffer();
		auto dest = fhFramebuffer::currentDepthFramebuffer;
		dest->Resize( source->GetWidth(), source->GetHeight() );
		fhFramebuffer::BlitDepth( source, dest );
	}

	// main light renderer

	RB_GLSL_DrawInteractions();

	// disable stencil shadow test
	glStencilFunc( GL_ALWAYS, 128, 255 );

	// uplight the entire screen to crutch up not having better blending range
	RB_STD_LightScale();

	{
		fhTimeElapsed timeElapsed(&backEnd.stats.groups[backEndGroup::NonInteraction].time);
		backEnd.stats.groups[backEndGroup::NonInteraction].passes += 1;

		StageRenderList stageRenderlist;

		// now draw any non-light dependent shading passes
		int	processed = RB_GLSL_CreateStageRenderList( drawSurfs, numDrawSurfs, stageRenderlist, SS_POST_PROCESS );
		RB_GLSL_SubmitStageRenderList(stageRenderlist);

		// fob and blend lights
		RB_STD_FogAllLights();

		// now draw any post-processing effects using _currentRender
		if (processed < numDrawSurfs) {
			stageRenderlist.Clear();
			RB_GLSL_CreateStageRenderList( drawSurfs + processed, numDrawSurfs - processed, stageRenderlist, 1000 );
			RB_GLSL_SubmitStageRenderList(stageRenderlist);
		}
	}

	RB_RenderDebugTools( drawSurfs, numDrawSurfs );
}

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
#include "tr_local.h"
#include "RenderList.h"
#include "RenderProgram.h"
#include "Framebuffer.h"

static void RB_GLSL_SubmitFillDepthRenderList( const DepthRenderList& renderlist ) {
	assert( depthProgram );

	// if we are just doing 2D rendering, no need to fill the depth buffer
	if (!backEnd.viewDef->viewEntitys) {
		return;
	}

	RB_LogComment( "---------- RB_GLSL_SubmitFillDepthRenderList ----------\n" );

	// enable the second texture for mirror plane clipping if needed
	if (backEnd.viewDef->numClipPlanes) {
		globalImages->alphaNotchImage->Bind( 1 );
	}

	// decal surfaces may enable polygon offset
	glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() );

	GL_State( GLS_DEPTHFUNC_LESS );

	// Enable stencil test if we are going to be using it for shadows.
	// If we didn't do this, it would be legal behavior to get z fighting
	// from the ambient pass and the light passes.
	glEnable( GL_STENCIL_TEST );
	glStencilFunc( GL_ALWAYS, 1, 255 );

	GL_UseProgram( depthProgram );

	const viewEntity_t* currentSpace = nullptr;
	bool depthHackActive = false;
	bool alphatestEnabled = false;
	idScreenRect currentScissor;
	if (r_useScissor.GetBool()) {
		auto fb = fhFramebuffer::GetCurrentDrawBuffer();
		glScissor( 0, 0, fb->GetWidth(), fb->GetHeight() );
		currentScissor.x1 = 0;
		currentScissor.y1 = 0;
		currentScissor.x2 = fb->GetWidth();
		currentScissor.y2 = fb->GetHeight();
	}

	fhRenderProgram::SetProjectionMatrix( backEnd.viewDef->projectionMatrix );
	fhRenderProgram::SetAlphaTestEnabled( false );

	const int num = renderlist.Num();
	for (int i = 0; i < num; ++i) {
		const auto& drawdepth = renderlist[i];

		const auto offset = vertexCache.Bind( drawdepth.surf->geo->ambientCache );
		GL_SetupVertexAttributes( fhVertexLayout::Draw, offset );

		if (currentSpace != drawdepth.surf->space) {
			fhRenderProgram::SetModelMatrix( drawdepth.surf->space->modelMatrix );
			fhRenderProgram::SetModelViewMatrix( drawdepth.surf->space->modelViewMatrix );

			if (drawdepth.surf->space->modelDepthHack) {
				RB_EnterModelDepthHack( drawdepth.surf->space->modelDepthHack );
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = true;
			}
			else if (drawdepth.surf->space->weaponDepthHack) {
				RB_EnterWeaponDepthHack();
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = true;
			}
			else if (depthHackActive) {
				RB_LeaveDepthHack();
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = false;
			}

			// change the scissor if needed
			if (r_useScissor.GetBool() && !currentScissor.Equals( drawdepth.surf->scissorRect )) {
				currentScissor = drawdepth.surf->scissorRect;
				glScissor( backEnd.viewDef->viewport.x1 + currentScissor.x1,
					backEnd.viewDef->viewport.y1 + currentScissor.y1,
					currentScissor.x2 + 1 - currentScissor.x1,
					currentScissor.y2 + 1 - currentScissor.y1 );
			}

			currentSpace = drawdepth.surf->space;
		}

		if (drawdepth.polygonOffset) {
			glEnable( GL_POLYGON_OFFSET_FILL );
			glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * drawdepth.polygonOffset );
		}

		if (drawdepth.isSubView) {
			GL_State( GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO | GLS_DEPTHFUNC_LESS );
		}

		if (drawdepth.texture) {
			fhRenderProgram::SetAlphaTestEnabled( true );
			fhRenderProgram::SetAlphaTestThreshold( drawdepth.alphaTestThreshold );
			fhRenderProgram::SetDiffuseMatrix( drawdepth.textureMatrix[0], drawdepth.textureMatrix[1] );
			drawdepth.texture->Bind( 0 );
			alphatestEnabled = true;
		}
		else {
			if (alphatestEnabled) {
				globalImages->whiteImage->Bind( 0 );
				fhRenderProgram::SetAlphaTestEnabled( false );
				alphatestEnabled = false;;
			}
		}

		fhRenderProgram::SetDiffuseColor(drawdepth.color);

		backEnd.stats.groups[backEndGroup::DepthPrepass].drawcalls += 1;
		backEnd.stats.groups[backEndGroup::DepthPrepass].tris += drawdepth.surf->geo->numIndexes / 3;
		RB_DrawElementsWithCounters( drawdepth.surf->geo );

		if (drawdepth.polygonOffset) {
			glDisable( GL_POLYGON_OFFSET_FILL );
		}

		if (drawdepth.isSubView) {
			GL_State( GLS_DEPTHFUNC_LESS );
		}
	}

	if (r_useScissor.GetBool()) {
		auto fb = fhFramebuffer::GetCurrentDrawBuffer();
		glScissor( 0, 0, fb->GetWidth(), fb->GetHeight() );
		backEnd.currentScissor.x1 = 0;
		backEnd.currentScissor.y1 = 0;
		backEnd.currentScissor.x2 = fb->GetWidth();
		backEnd.currentScissor.y2 = fb->GetHeight();
	}
}

static void RB_GLSL_CreateFillDepthRenderList( drawSurf_t **drawSurfs, int numDrawSurfs, DepthRenderList& renderlist ) {
	for (int i = 0; i < numDrawSurfs; i++) {
		drawDepth_t drawdepth;

		const drawSurf_t* surf = drawSurfs[i];
		const srfTriangles_t* tri = surf->geo;
		const idMaterial* shader = surf->material;

		if (!shader->IsDrawn()) {
			continue;
		}

		// some deforms may disable themselves by setting numIndexes = 0
		if (!tri->numIndexes) {
			continue;
		}

		// translucent surfaces don't put anything in the depth buffer and don't
		// test against it, which makes them fail the mirror clip plane operation
		if (shader->Coverage() == MC_TRANSLUCENT) {
			continue;
		}

		if (!tri->ambientCache) {
			common->Printf( "RB_T_FillDepthBuffer: !tri->ambientCache\n" );
			continue;
		}

		// get the expressions for conditionals / color / texcoords
		const float	* regs = surf->shaderRegisters;

		// if all stages of a material have been conditioned off, don't do anything
		int stage = 0;
		for (; stage < shader->GetNumStages(); stage++) {
			const shaderStage_t* pStage = shader->GetStage( stage );
			// check the stage enable condition
			if (regs[pStage->conditionRegister] != 0) {
				break;
			}
		}
		if (stage == shader->GetNumStages()) {
			continue;
		}

		// set polygon offset if necessary
		if (shader->TestMaterialFlag( MF_POLYGONOFFSET )) {
			drawdepth.polygonOffset = shader->GetPolygonOffset();
		}
		else {
			drawdepth.polygonOffset = 0;
		}

		// subviews will just down-modulate the color buffer by overbright
		if (shader->GetSort() == SS_SUBVIEW) {
			drawdepth.isSubView = true;
			drawdepth.color[0] = drawdepth.color[1] = drawdepth.color[2] = (1.0 / backEnd.overBright);
			drawdepth.color[3] = 1;
		}
		else {
			drawdepth.isSubView = false;
			drawdepth.color[0] = drawdepth.color[1] = drawdepth.color[2] = 0;
			drawdepth.color[3] = 1;
		}


		drawdepth.surf = surf;
		drawdepth.texture = nullptr;

		bool drawSolid = false;

		if (shader->Coverage() == MC_OPAQUE) {
			drawSolid = true;
		}

		// we may have multiple alpha tested stages
		if (shader->Coverage() == MC_PERFORATED) {
			// if the only alpha tested stages are condition register omitted,
			// draw a normal opaque surface
			bool	didDraw = false;

			// perforated surfaces may have multiple alpha tested stages
			for (stage = 0; stage < shader->GetNumStages(); stage++) {
				const shaderStage_t* pStage = shader->GetStage( stage );

				if (!pStage->hasAlphaTest) {
					continue;
				}

				// check the stage enable condition
				if (regs[pStage->conditionRegister] == 0) {
					continue;
				}

				// if we at least tried to draw an alpha tested stage,
				// we won't draw the opaque surface
				didDraw = true;

				// set the alpha modulate
				drawdepth.color[3] = regs[pStage->color.registers[3]];

				// skip the entire stage if alpha would be black
				if (drawdepth.color[3] <= 0) {
					continue;
				}

				// bind the texture
				drawdepth.texture = pStage->texture.image;
				drawdepth.alphaTestThreshold = regs[pStage->alphaTestRegister];

				// set texture matrix and texGens

				if (pStage->privatePolygonOffset && !surf->material->TestMaterialFlag( MF_POLYGONOFFSET )) {
					drawdepth.polygonOffset = pStage->privatePolygonOffset;
				}

				if (pStage->texture.hasMatrix) {
					RB_GetShaderTextureMatrix( surf->shaderRegisters, &pStage->texture, drawdepth.textureMatrix );
				}
				else {
					drawdepth.textureMatrix[0] = idVec4::identityS;
					drawdepth.textureMatrix[1] = idVec4::identityT;
				}

				// Append drawdepth inside the loop, because some effects (eg burn-away of dead demons)
				// require multiple stages being rendered to the depth buffer, because those stages are
				// being blended/masked individually.
				renderlist.Append( drawdepth );
			}

			if (!didDraw) {
				drawSolid = true;
			}
		}

		if(drawSolid) {
			renderlist.Append( drawdepth );
		}
	}
}



void DepthRenderList::AddDrawSurfaces( drawSurf_t **surf, int numDrawSurfs ) {
	RB_GLSL_CreateFillDepthRenderList( surf, numDrawSurfs, *this);
}

void DepthRenderList::Submit() {
	RB_GLSL_SubmitFillDepthRenderList(*this);
}

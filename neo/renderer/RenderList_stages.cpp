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

/*
==================
R_WobbleskyTexGen
==================
*/
static void R_CreateWobbleskyTexMatrix( const drawSurf_t *surf, float time, float matrix[16] ) {

	const int *parms = surf->material->GetTexGenRegisters();

	float	wobbleDegrees = surf->shaderRegisters[parms[0]];
	float	wobbleSpeed = surf->shaderRegisters[parms[1]];
	float	rotateSpeed = surf->shaderRegisters[parms[2]];

	wobbleDegrees = wobbleDegrees * idMath::PI / 180;
	wobbleSpeed = wobbleSpeed * 2 * idMath::PI / 60;
	rotateSpeed = rotateSpeed * 2 * idMath::PI / 60;

	// very ad-hoc "wobble" transform
	float	a = time * wobbleSpeed;
	float	s = sin( a ) * sin( wobbleDegrees );
	float	c = cos( a ) * sin( wobbleDegrees );
	float	z = cos( wobbleDegrees );

	idVec3	axis[3];

	axis[2][0] = c;
	axis[2][1] = s;
	axis[2][2] = z;

	axis[1][0] = -sin( a * 2 ) * sin( wobbleDegrees );
	axis[1][2] = -s * sin( wobbleDegrees );
	axis[1][1] = sqrt( 1.0f - (axis[1][0] * axis[1][0] + axis[1][2] * axis[1][2]) );

	// make the second vector exactly perpendicular to the first
	axis[1] -= (axis[2] * axis[1]) * axis[2];
	axis[1].Normalize();

	// construct the third with a cross
	axis[0].Cross( axis[1], axis[2] );

	// add the rotate
	s = sin( rotateSpeed * time );
	c = cos( rotateSpeed * time );

	matrix[0] = axis[0][0] * c + axis[1][0] * s;
	matrix[4] = axis[0][1] * c + axis[1][1] * s;
	matrix[8] = axis[0][2] * c + axis[1][2] * s;

	matrix[1] = axis[1][0] * c - axis[0][0] * s;
	matrix[5] = axis[1][1] * c - axis[0][1] * s;
	matrix[9] = axis[1][2] * c - axis[0][2] * s;

	matrix[2] = axis[2][0];
	matrix[6] = axis[2][1];
	matrix[10] = axis[2][2];

	matrix[3] = matrix[7] = matrix[11] = 0.0f;
	matrix[12] = matrix[13] = matrix[14] = 0.0f;
}


static bool RB_GLSL_CreateShaderStage( const drawSurf_t* surf, const shaderStage_t* pStage, drawStage_t& drawStage ) {
	// set the color
	float color[4];
	color[0] = surf->shaderRegisters[pStage->color.registers[0]];
	color[1] = surf->shaderRegisters[pStage->color.registers[1]];
	color[2] = surf->shaderRegisters[pStage->color.registers[2]];
	color[3] = surf->shaderRegisters[pStage->color.registers[3]];

	// skip the entire stage if an add would be black
	if ((pStage->drawStateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)) == (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE)
		&& color[0] <= 0 && color[1] <= 0 && color[2] <= 0) {
		return false;
	}

	// skip the entire stage if a blend would be completely transparent
	if ((pStage->drawStateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)) == (GLS_SRCBLEND_SRC_ALPHA | GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA)
		&& color[3] <= 0) {
		return false;
	}

	if (pStage->texture.texgen == TG_DIFFUSE_CUBE) {
		return false;
	}
	else if (pStage->texture.texgen == TG_SKYBOX_CUBE || pStage->texture.texgen == TG_WOBBLESKY_CUBE) {
		drawStage.program = skyboxProgram;

		if (pStage->texture.texgen == TG_WOBBLESKY_CUBE) {
			R_CreateWobbleskyTexMatrix( surf, backEnd.viewDef->floatTime, drawStage.textureMatrix );
		}

		idVec4 localViewOrigin;
		R_GlobalPointToLocal( surf->space->modelMatrix, backEnd.viewDef->renderView.vieworg, localViewOrigin.ToVec3() );
		localViewOrigin[3] = 1.0f;

		drawStage.localViewOrigin = localViewOrigin;
		drawStage.vertexLayout = fhVertexLayout::DrawPosColorOnly;
	}
	else if (pStage->texture.texgen == TG_SCREEN) {
		return false;
	}
	else if (pStage->texture.texgen == TG_GLASSWARP) {
		return false;
	}
	else if (pStage->texture.texgen == TG_REFLECT_CUBE) {
		drawStage.program = bumpyEnvProgram;

		idVec4 localViewOrigin;
		R_GlobalPointToLocal( surf->space->modelMatrix, backEnd.viewDef->renderView.vieworg, localViewOrigin.ToVec3() );
		localViewOrigin[3] = 1.0f;

		drawStage.localViewOrigin = localViewOrigin;

		//TODO(johl): do we really need this? bumpyenv.vp uses the texture matrix, but it's always the identiy matrix anyway?
		memcpy( drawStage.textureMatrix, &mat4_identity, sizeof(mat4_identity) );

		// see if there is also a bump map specified
		if (const shaderStage_t *bumpStage = surf->material->GetBumpStage()) {
			//RB_GetShaderTextureMatrix( surf->shaderRegisters, &bumpStage->texture, drawStage.bumpMatrix );
			//drawStage.hasBumpMatrix = true;
			drawStage.textures[2] = bumpStage->texture.image;
		}
		else {
			//drawStage.hasBumpMatrix = true;
			//drawStage.bumpMatrix[0] = idVec4::identityS;
			//drawStage.bumpMatrix[1] = idVec4::identityT;
			drawStage.textures[2] = globalImages->flatNormalMap;
		}

		drawStage.vertexLayout = fhVertexLayout::Draw;
	}
	else {

		//prefer depth blend settings from material. If material does not define a
		// depth blend mode, look at the geometry for depth blend settings (usually
		// set by particle systems for soft particles)

		drawStage.depthBlendMode = pStage->depthBlendMode;
		drawStage.depthBlendRange = pStage->depthBlendRange;

		if (drawStage.depthBlendMode == DBM_UNDEFINED) {
			drawStage.depthBlendMode = surf->geo->depthBlendMode;
			drawStage.depthBlendRange = surf->geo->depthBlendRange;
		}

		if (drawStage.depthBlendMode == DBM_AUTO) {
			if (pStage->drawStateBits & (GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE))
				drawStage.depthBlendMode = DBM_COLORALPHA_ZERO;
			else if (pStage->drawStateBits & (GLS_SRCBLEND_DST_COLOR | GLS_DSTBLEND_ZERO))
				drawStage.depthBlendMode = DBM_COLORALPHA_ONE;
			else
				drawStage.depthBlendMode = DBM_OFF;
		}

		if (drawStage.depthBlendMode != DBM_OFF && drawStage.depthBlendRange > 0.0f) {
			drawStage.program = depthblendProgram;
			drawStage.textures[2] = globalImages->currentDepthImage;
		}
		else {
			drawStage.program = defaultProgram;
		}

		drawStage.vertexLayout = fhVertexLayout::DrawPosColorTexOnly;
	}

	drawStage.cinematic = pStage->texture.cinematic;

	// set the state
	drawStage.drawStateBits = pStage->drawStateBits;
	drawStage.vertexColor = pStage->vertexColor;

	// set privatePolygonOffset if necessary
	if (pStage->privatePolygonOffset) {
		drawStage.polygonOffset = pStage->privatePolygonOffset;
	}

	if (pStage->texture.image) {
		if (pStage->texture.hasMatrix) {
			RB_GetShaderTextureMatrix( surf->shaderRegisters, &pStage->texture, drawStage.bumpMatrix );
			drawStage.hasBumpMatrix = true;
		}

		drawStage.textures[1] = pStage->texture.image;
	}

	drawStage.diffuseColor = idVec4( color );
	return true;
}


static void RB_GLSL_CreateShaderPasses( const drawSurf_t* surf, StageRenderList& renderlist ) {
	const srfTriangles_t* tri = surf->geo;
	const idMaterial* shader = surf->material;

	if (!shader->HasAmbient()) {
		return;
	}

	if (shader->IsPortalSky()) {
		return;
	}

	// some deforms may disable themselves by setting numIndexes = 0
	if (!tri->numIndexes) {
		return;
	}

	if (!tri->ambientCache) {
		common->Printf( "RB_T_RenderShaderPasses: !tri->ambientCache\n" );
		return;
	}

	// get the expressions for conditionals / color / texcoords
	const float	*regs = surf->shaderRegisters;

	// get polygon offset if necessary
	const float polygonOffset = shader->TestMaterialFlag( MF_POLYGONOFFSET ) ? shader->GetPolygonOffset() : 0;


	for (int stage = 0; stage < shader->GetNumStages(); stage++) {
		const shaderStage_t* pStage = shader->GetStage( stage );

		// check the enable condition
		if (regs[pStage->conditionRegister] == 0) {
			continue;
		}

		// skip the stages involved in lighting
		if (pStage->lighting != SL_AMBIENT) {
			continue;
		}

		// skip if the stage is ( GL_ZERO, GL_ONE ), which is used for some alpha masks
		if ((pStage->drawStateBits & (GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)) == (GLS_SRCBLEND_ZERO | GLS_DSTBLEND_ONE)) {
			continue;
		}

		drawStage_t drawstage;
		memset( &drawstage, 0, sizeof(drawstage) );
		drawstage.surf = surf;
		drawstage.cullType = shader->GetCullType();
		drawstage.polygonOffset = polygonOffset;
		drawstage.depthBlendMode = DBM_OFF;
		drawstage.bumpMatrix[0] = idVec4::identityS;
		drawstage.bumpMatrix[1] = idVec4::identityT;
		drawstage.hasBumpMatrix = false;
		memcpy( drawstage.textureMatrix, &mat4_identity, sizeof(mat4_identity) );

		if (glslShaderStage_t *glslStage = pStage->glslStage) { // see if we are a glsl-style stage

			if (r_skipGlsl.GetBool())
				continue;

			if (!glslStage->program)
				continue;

			drawstage.program = glslStage->program;
			drawstage.drawStateBits = pStage->drawStateBits;
			drawstage.numShaderparms = Min( glslStage->numShaderParms, 4 );
			for (int i = 0; i < drawstage.numShaderparms; i++) {
				idVec4& parm = drawstage.shaderparms[i];
				parm[0] = regs[glslStage->shaderParms[i][0]];
				parm[1] = regs[glslStage->shaderParms[i][1]];
				parm[2] = regs[glslStage->shaderParms[i][2]];
				parm[3] = regs[glslStage->shaderParms[i][3]];
			}

			// set textures
			for (int i = 0; i < glslStage->numShaderMaps && i < 4; i++) {
				drawstage.textures[i] = glslStage->shaderMap[i];
			}

			drawstage.vertexLayout = fhVertexLayout::Draw;

			renderlist.Append( drawstage );
		}
		else {
			if (RB_GLSL_CreateShaderStage( surf, pStage, drawstage )) {
				renderlist.Append( drawstage );
			}
		}
	}
}

void RB_GLSL_SubmitStageRenderList( const StageRenderList& renderlist ) {

	const fhRenderProgram* currentProgram = nullptr;
	const viewEntity_t* currentSpace = nullptr;
	stageVertexColor_t currentVertexColor = (stageVertexColor_t)-1;
	bool currentBumpMatrix = true;

	const int num = renderlist.Num();
	bool currentRenderCopied = false;

	for (int i = 0; i < num; ++i) {
		const drawStage_t& drawstage = renderlist[i];

		// if we are about to draw the first surface that needs
		// the rendering in a texture, copy it over
		if (drawstage.surf->material->GetSort() >= SS_POST_PROCESS && !currentRenderCopied) {
			if (r_skipPostProcess.GetBool()) {
				continue;
			}

			// only dump if in a 3d view
			if (backEnd.viewDef->viewEntitys) {
				auto source = fhFramebuffer::GetCurrentDrawBuffer();
				auto dest = fhFramebuffer::currentRenderFramebuffer;

				dest->Resize( source->GetWidth(), source->GetHeight() );
				fhFramebuffer::BlitColor( source, dest );
			}

			currentRenderCopied = true;
		}

		if (currentProgram != drawstage.program) {
			GL_UseProgram( drawstage.program );
			fhRenderProgram::SetProjectionMatrix( backEnd.viewDef->projectionMatrix );
			fhRenderProgram::SetClipRange( backEnd.viewDef->viewFrustum.GetNearDistance(), backEnd.viewDef->viewFrustum.GetFarDistance() );

			// current render
			const int w = backEnd.viewDef->viewport.x2 - backEnd.viewDef->viewport.x1 + 1;
			const int h = backEnd.viewDef->viewport.y2 - backEnd.viewDef->viewport.y1 + 1;
			fhRenderProgram::SetCurrentRenderSize(
				idVec2( globalImages->currentRenderImage->uploadWidth, globalImages->currentRenderImage->uploadHeight ),
				idVec2( w, h ) );

			currentProgram = drawstage.program;
			currentSpace = nullptr;
			currentVertexColor = (stageVertexColor_t)-1;
			currentBumpMatrix = true;
		}

		if (currentSpace != drawstage.surf->space) {
			fhRenderProgram::SetModelViewMatrix( drawstage.surf->space->modelViewMatrix );
			fhRenderProgram::SetModelMatrix( drawstage.surf->space->modelMatrix );

			if (r_useScissor.GetBool() && !backEnd.currentScissor.Equals( drawstage.surf->scissorRect )) {
				const auto& scissor = drawstage.surf->scissorRect;
				glScissor( backEnd.viewDef->viewport.x1 + scissor.x1,
					backEnd.viewDef->viewport.y1 + scissor.y1,
					scissor.x2 + 1 - scissor.x1,
					scissor.y2 + 1 - scissor.y1 );
			}

			currentSpace = drawstage.surf->space;
		}

		const auto offset = vertexCache.Bind( drawstage.surf->geo->ambientCache );
		GL_SetupVertexAttributes( drawstage.vertexLayout, offset );

		GL_Cull( drawstage.cullType );

		if (drawstage.polygonOffset) {
			glEnable( GL_POLYGON_OFFSET_FILL );
			glPolygonOffset( r_offsetFactor.GetFloat(), r_offsetUnits.GetFloat() * drawstage.polygonOffset );
		}

		if (drawstage.surf->space->weaponDepthHack) {
			RB_EnterWeaponDepthHack();
			fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
		}

		if (drawstage.surf->space->modelDepthHack != 0.0f) {
			RB_EnterModelDepthHack( drawstage.surf->space->modelDepthHack );
			fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
		}

		if (drawstage.hasBumpMatrix) {
			fhRenderProgram::SetBumpMatrix( drawstage.bumpMatrix[0], drawstage.bumpMatrix[1] );
			currentBumpMatrix = true;
		}
		else if (currentBumpMatrix) {
			fhRenderProgram::SetBumpMatrix( idVec4::identityS, idVec4::identityT );
			currentBumpMatrix = false;
		}

		fhRenderProgram::SetLocalViewOrigin( drawstage.localViewOrigin );
		fhRenderProgram::SetDiffuseColor( drawstage.diffuseColor );
		fhRenderProgram::SetTextureMatrix( drawstage.textureMatrix );
		fhRenderProgram::SetDepthBlendRange( drawstage.depthBlendRange );
		fhRenderProgram::SetDepthBlendMode( static_cast<int>(drawstage.depthBlendMode) );
		fhRenderProgram::SetShaderParm( 0, drawstage.shaderparms[0] );
		fhRenderProgram::SetShaderParm( 1, drawstage.shaderparms[1] );
		fhRenderProgram::SetShaderParm( 2, drawstage.shaderparms[2] );
		fhRenderProgram::SetShaderParm( 3, drawstage.shaderparms[3] );

		if (currentVertexColor != drawstage.vertexColor) {
			switch (drawstage.vertexColor) {
			case SVC_IGNORE:
				fhRenderProgram::SetColorModulate( idVec4::zero );
				fhRenderProgram::SetColorAdd( idVec4::one );
				break;
			case SVC_MODULATE:
				fhRenderProgram::SetColorModulate( idVec4::one );
				fhRenderProgram::SetColorAdd( idVec4::zero );
				break;
			case SVC_INVERSE_MODULATE:
				fhRenderProgram::SetColorModulate( idVec4::negOne );
				fhRenderProgram::SetColorAdd( idVec4::one );
				break;
			}
			currentVertexColor = drawstage.vertexColor;
		}

		for (int i = 0; i < 4; ++i) {
			if (drawstage.textures[i])
				drawstage.textures[i]->Bind( i );
		}

		if (drawstage.cinematic) {
			if (r_skipDynamicTextures.GetBool()) {
				globalImages->defaultImage->Bind( 1 );
			}
			else {
				// offset time by shaderParm[7] (FIXME: make the time offset a parameter of the shader?)
				// We make no attempt to optimize for multiple identical cinematics being in view, or
				// for cinematics going at a lower framerate than the renderer.
				cinData_t cin = drawstage.cinematic->ImageForTime( (int)(1000 * (backEnd.viewDef->floatTime + backEnd.viewDef->renderView.shaderParms[11])) );

				if (cin.image) {
					auto image = globalImages->cinematicImage;
					image->AllocateStorage( pixelFormat_t::RGBA, cin.imageWidth, cin.imageHeight, 1, 1 );
					image->UploadImage( pixelFormat_t::RGBA, cin.imageWidth, cin.imageHeight, 0, 0, cin.imageWidth * cin.imageHeight * 4, cin.image );
					image->Bind( 1 );
				}
				else {
					globalImages->blackImage->Bind( 1 );
				}
			}
		}

		GL_State( drawstage.drawStateBits );

		backEnd.stats.groups[backEndGroup::NonInteraction].drawcalls += 1;
		backEnd.stats.groups[backEndGroup::NonInteraction].tris += drawstage.surf->geo->numIndexes / 3;
		RB_DrawElementsWithCounters( drawstage.surf->geo );

		if (drawstage.polygonOffset) {
			glDisable( GL_POLYGON_OFFSET_FILL );
		}

		if (drawstage.surf->space->weaponDepthHack || drawstage.surf->space->modelDepthHack != 0.0f) {
			RB_LeaveDepthHack();
			fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
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

	GL_UseProgram( nullptr );
}

int RB_GLSL_CreateStageRenderList( drawSurf_t **drawSurfs, int numDrawSurfs, StageRenderList& renderlist, int maxSort ) {
	// only obey skipAmbient if we are rendering a view
	if (backEnd.viewDef->viewEntitys && r_skipAmbient.GetBool()) {
		return numDrawSurfs;
	}

	RB_LogComment( "---------- RB_GLSL_CreateStageRenderList ----------\n" );

	int i = 0;
	for (; i < numDrawSurfs; i++) {
		if (drawSurfs[i]->material->SuppressInSubview()) {
			continue;
		}

		if (backEnd.viewDef->isXraySubview && drawSurfs[i]->space->entityDef) {
			if (drawSurfs[i]->space->entityDef->parms.xrayIndex != 2) {
				continue;
			}
		}

		// we need to draw the post process shaders after we have drawn the fog lights
		if (drawSurfs[i]->material->GetSort() >= maxSort) {
			break;
		}

		RB_GLSL_CreateShaderPasses( drawSurfs[i], renderlist );
	}

	return i;
}


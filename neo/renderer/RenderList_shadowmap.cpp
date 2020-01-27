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

idCVar r_smObjectCulling( "r_smObjectCulling", "1", CVAR_RENDERER | CVAR_BOOL | CVAR_ARCHIVE, "cull objects/surfaces that are outside the shadow/light frustum when rendering shadow maps" );
idCVar r_smUseStaticOcclusion( "r_smUseStaticOcclusion", "1", CVAR_RENDERER | CVAR_BOOL | CVAR_ARCHIVE, "" );
idCVar r_smSkipStaticOcclusion( "r_smSkipStaticOcclusion", "0", CVAR_RENDERER | CVAR_BOOL, "" );
idCVar r_smSkipNonStaticOcclusion( "r_smSkipNonStaticOcclusion", "0", CVAR_RENDERER | CVAR_BOOL, "" );
idCVar r_smSkipMovingLights( "r_smSkipMovingLights", "0", CVAR_RENDERER | CVAR_BOOL, "" );

ShadowRenderList::ShadowRenderList() {
	dummy.parms.shaderParms[0] = 1;
	dummy.parms.shaderParms[1] = 1;
	dummy.parms.shaderParms[2] = 1;
	dummy.parms.shaderParms[3] = 1;
	dummy.modelMatrix[0] = 1;
	dummy.modelMatrix[5] = 1;
	dummy.modelMatrix[10] = 1;
	dummy.modelMatrix[15] = 1;
}

void ShadowRenderList::AddInteractions( viewLight_t* vlight, const shadowMapFrustum_t* shadowFrustrums, int numShadowFrustrums ) {
	assert( numShadowFrustrums <= 6 );
	assert( numShadowFrustrums >= 0 );

	if (vlight->lightDef->lightHasMoved && r_smSkipMovingLights.GetBool()) {
		return;
	}

	bool staticOcclusionGeometryRendered = false;

	if (vlight->lightDef->parms.occlusionModel && !vlight->lightDef->lightHasMoved && r_smUseStaticOcclusion.GetBool()) {

		if (!r_smSkipStaticOcclusion.GetBool()) {
			int numSurfaces = vlight->lightDef->parms.occlusionModel->NumSurfaces();
			for (int i = 0; i < numSurfaces; ++i) {
				auto surface = vlight->lightDef->parms.occlusionModel->Surface( i );
				AddSurfaceInteraction( &dummy, surface->geometry, surface->shader, ~0 );
			}
		}

		staticOcclusionGeometryRendered = true;
	}

	if (r_smSkipNonStaticOcclusion.GetBool()) {
		return;
	}

	const bool objectCullingEnabled = r_smObjectCulling.GetBool() && (numShadowFrustrums > 0);

	for (idInteraction* inter = vlight->lightDef->firstInteraction; inter; inter = inter->lightNext) {
		const idRenderEntityLocal *entityDef = inter->entityDef;

		if (!entityDef) {
			continue;
		}

		if (entityDef->parms.noShadow) {
			continue;
		}

		if (inter->numSurfaces < 1) {
			continue;
		}

		unsigned visibleSides = ~0;

		if (objectCullingEnabled) {
			visibleSides = 0;

			// cull the entire entity bounding box
			// has referenceBounds been tightened to the actual model bounds?
			idVec3	corners[8];
			for (int i = 0; i < 8; i++) {
				idVec3 tmp;
				tmp[0] = entityDef->referenceBounds[i & 1][0];
				tmp[1] = entityDef->referenceBounds[(i >> 1) & 1][1];
				tmp[2] = entityDef->referenceBounds[(i >> 2) & 1][2];
				R_LocalPointToGlobal( entityDef->modelMatrix, tmp, corners[i] );
			}

			for (int i = 0; i < numShadowFrustrums; ++i) {
				if (!shadowFrustrums[i].Cull( corners  )) {
					visibleSides |= (1 << i);
				}
			}
		}

		if (!visibleSides)
			continue;

		const int num = inter->numSurfaces;
		for (int i = 0; i < num; i++) {
			const auto& surface = inter->surfaces[i];
			const auto* material = surface.shader;

			if (staticOcclusionGeometryRendered && surface.isStaticWorldModel) {
				continue;
			}

			const auto* tris = surface.ambientTris;
			if (!tris || tris->numVerts < 3 || !material) {
				continue;
			}

			AddSurfaceInteraction( entityDef, tris, material, visibleSides );
		}
	}
}

void ShadowRenderList::Submit( const float* shadowViewMatrix, const float* shadowProjectionMatrix, int side, int lod ) const {
	fhRenderProgram::SetProjectionMatrix( shadowProjectionMatrix );
	fhRenderProgram::SetViewMatrix( shadowViewMatrix );
	fhRenderProgram::SetAlphaTestEnabled( false );
	fhRenderProgram::SetDiffuseMatrix( idVec4::identityS, idVec4::identityT );
	fhRenderProgram::SetAlphaTestThreshold( 0.5f );

	const idRenderEntityLocal *currentEntity = nullptr;
	bool currentAlphaTest = false;
	bool currentHasTextureMatrix = false;

	const int sideBit = (1 << side);
	const int num = Num();

	glDepthRange(0, 1);

	for (int i = 0; i < num; ++i) {
		const auto& drawShadow = (*this)[i];

		if (!(drawShadow.visibleFlags & sideBit)) {
			continue;
		}

		if (!drawShadow.tris->ambientCache) {
			//TODO(johl): Some surfaces need lighting later on (e.g. AF/Ragdolls), if we don't create
			//            lighting info for them here (needsLighting=true), those surfaces will show up
			//            completely black in the game (due to missing normals/tangents).
			//            How do we know, if lighting is needed later on?
			//            For now we just assume this to be true for every surface. It seems that it does not effect performance badly.
			R_CreateAmbientCache( const_cast<srfTriangles_t *>(drawShadow.tris), true /*<= just assume lighting is needed*/ );
		}

		const auto offset = vertexCache.Bind( drawShadow.tris->ambientCache );
		GL_SetupVertexAttributes( fhVertexLayout::DrawPosTexOnly, offset );

		if (currentEntity != drawShadow.entity) {
			fhRenderProgram::SetModelMatrix( drawShadow.entity->modelMatrix );
			currentEntity = drawShadow.entity;
		}

		if (drawShadow.texture) {
			if (!currentAlphaTest) {
				fhRenderProgram::SetAlphaTestEnabled( true );
				currentAlphaTest = true;
			}

			drawShadow.texture->Bind( 0 );

			if (drawShadow.hasTextureMatrix) {
				fhRenderProgram::SetDiffuseMatrix( drawShadow.textureMatrix[0], drawShadow.textureMatrix[1] );
				currentHasTextureMatrix = true;
			}
			else if (currentHasTextureMatrix) {
				fhRenderProgram::SetDiffuseMatrix( idVec4::identityS, idVec4::identityT );
				currentHasTextureMatrix = false;
			}
		}
		else if (currentAlphaTest) {
			fhRenderProgram::SetAlphaTestEnabled( false );
			currentAlphaTest = false;
		}

		RB_DrawElementsWithCounters( drawShadow.tris );

		backEnd.stats.groups[backEndGroup::ShadowMap0 + lod].drawcalls += 1;
		backEnd.stats.groups[backEndGroup::ShadowMap0 + lod].tris += drawShadow.tris->numIndexes / 3;
	}
}

void ShadowRenderList::AddSurfaceInteraction( const idRenderEntityLocal *entityDef, const srfTriangles_t *tri, const idMaterial* material, unsigned visibleSides ) {

	if (!material->SurfaceCastsSoftShadow()) {
		return;
	}

	drawShadow_t drawShadow;
	drawShadow.tris = tri;
	drawShadow.entity = entityDef;
	drawShadow.texture = nullptr;
	drawShadow.visibleFlags = visibleSides;

	// we may have multiple alpha tested stages
	if (material->Coverage() == MC_PERFORATED) {
		// if the only alpha tested stages are condition register omitted,
		// draw a normal opaque surface

		float *regs = (float *)R_ClearedFrameAlloc( material->GetNumRegisters() * sizeof(float) );
		material->EvaluateRegisters( regs, entityDef->parms.shaderParms, backEnd.viewDef, nullptr );

		// perforated surfaces may have multiple alpha tested stages
		for (int stage = 0; stage < material->GetNumStages(); stage++) {
			const shaderStage_t* pStage = material->GetStage( stage );

			if (!pStage->hasAlphaTest) {
				continue;
			}

			if (regs[pStage->conditionRegister] == 0) {
				continue;
			}

			drawShadow.texture = pStage->texture.image;
			drawShadow.alphaTestThreshold = 0.5f;

			if (pStage->texture.hasMatrix) {
				drawShadow.hasTextureMatrix = true;
				RB_GetShaderTextureMatrix( regs, &pStage->texture, drawShadow.textureMatrix );
			}
			else {
				drawShadow.hasTextureMatrix = false;
			}

			break;
		}
	}

	Append( drawShadow );
}

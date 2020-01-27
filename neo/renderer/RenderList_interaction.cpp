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
=================
RB_GetNormalEncoding
=================
*/
static int RB_GetNormalEncoding( const idImage* image ) {
	switch (image->pixelFormat) {
	case pixelFormat_t::RGTC:
		return fhRenderProgram::normal_map_encoding_rg;
	case pixelFormat_t::DXT5_RxGB:
		return fhRenderProgram::normal_map_encoding_dxrg;
	case pixelFormat_t::DXT5_RGBA:
		return fhRenderProgram::normal_map_encoding_ag;
	default:
		return fhRenderProgram::normal_map_encoding_rgb;
	}
}

/*
=================
RB_SubmittInteraction
=================
*/
static void RB_SubmittInteraction( drawInteraction_t *din, InteractionList& interactionList, bool isAmbientLight ) {
	
	if ( din->blendMode == materialBlendMode::BLEND_NONE )
	{
		if ( !din->bumpImage )
		{
			return;
		}

		if ( !din->diffuseImage || r_skipDiffuse.GetBool() )
		{
			din->diffuseImage = globalImages->blackImage;
		}
		if ( !din->specularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->specularImage = globalImages->whiteImage;
		}
		if ( !din->bumpImage || r_skipBump.GetBool() )
		{
			din->bumpImage = globalImages->flatNormalMap;
		}

		if ( din->surf->material->GetRealTimeMode() )
		{
			if ( din->surf->material->GetRealTimeMode() == 1 )
				din->hasSSR = true;
			else
				din->hasRTCM = true;
		}

		din->rtcmImage = globalImages->rtcmImage;
		din->rtcmFrontImage = globalImages->rtcmFrontImage;
		din->rtcmBackImage = globalImages->rtcmBackImage;
		din->rtcmRightImage = globalImages->rtcmRightImage;
		din->rtcmLeftImage = globalImages->rtcmLeftImage;
		din->rtcmUpImage = globalImages->rtcmUpImage;
		din->rtcmDownImage = globalImages->rtcmDownImage;

		// if we wouldn't draw anything, don't call the Draw function
		if (
			((din->diffuseColor[ 0 ] > 0 ||
			  din->diffuseColor[ 1 ] > 0 ||
			  din->diffuseColor[ 2 ] > 0) && din->diffuseImage != globalImages->blackImage)
			|| ((din->specularColor[ 0 ] > 0 ||
				 din->specularColor[ 1 ] > 0 ||
				 din->specularColor[ 2 ] > 0) && din->specularImage != globalImages->blackImage) )
		{

			interactionList.Append( *din );
		}
	}

	else if ( din->blendMode == materialBlendMode::BLEND_2WAY )
	{
		if ( !din->blendABumpImage && !din->blendBBumpImage )
		{
			return;
		}

		if ( !din->blendADiffuseImage || r_skipDiffuse.GetBool() )
		{
			din->blendADiffuseImage = globalImages->blackImage;
		}
		if ( !din->blendBDiffuseImage || r_skipDiffuse.GetBool() )
		{
			din->blendBDiffuseImage = globalImages->blackImage;
		}

		if ( !din->blendASpecularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->blendASpecularImage = globalImages->whiteImage;
		}
		if ( !din->blendBSpecularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->blendBSpecularImage = globalImages->whiteImage;
		}

		if ( !din->blendABumpImage || r_skipBump.GetBool() )
		{
			din->blendABumpImage = globalImages->flatNormalMap;
		}
		if ( !din->blendBBumpImage || r_skipBump.GetBool() )
		{
			din->blendBBumpImage = globalImages->flatNormalMap;
		}

		// if we wouldn't draw anything, don't call the Draw function
		if (
			((din->diffuseColor[ 0 ] > 0 ||
			   din->diffuseColor[ 1 ] > 0 ||
			   din->diffuseColor[ 2 ] > 0) && din->diffuseImage != globalImages->blackImage)
			|| ((din->specularColor[ 0 ] > 0 ||
				  din->specularColor[ 1 ] > 0 ||
				  din->specularColor[ 2 ] > 0) && din->specularImage != globalImages->blackImage) )
		{
		
			interactionList.Append( *din );
		}
	}

	else if ( din->blendMode == materialBlendMode::BLEND_3WAY )
	{
		if ( !din->blendABumpImage && !din->blendBBumpImage && !din->blendCBumpImage )
		{
			return;
		}

		if ( !din->blendADiffuseImage || r_skipDiffuse.GetBool() )
		{
			din->blendADiffuseImage = globalImages->blackImage;
		}
		if ( !din->blendBDiffuseImage || r_skipDiffuse.GetBool() )
		{
			din->blendBDiffuseImage = globalImages->blackImage;
		}
		if ( !din->blendCDiffuseImage || r_skipDiffuse.GetBool() )
		{
			din->blendCDiffuseImage = globalImages->blackImage;
		}
	
		if ( !din->blendASpecularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->blendASpecularImage = globalImages->whiteImage;
		}
		if ( !din->blendBSpecularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->blendBSpecularImage = globalImages->whiteImage;
		}
		if ( !din->blendCSpecularImage || r_skipSpecular.GetBool() || isAmbientLight )
		{
			din->blendCSpecularImage = globalImages->whiteImage;
		}
	
		if ( !din->blendABumpImage || r_skipBump.GetBool() )
		{
			din->blendABumpImage = globalImages->flatNormalMap;
		}
		if ( !din->blendBBumpImage || r_skipBump.GetBool() )
		{
			din->blendBBumpImage = globalImages->flatNormalMap;
		}
		if ( !din->blendCBumpImage || r_skipBump.GetBool() )
		{
			din->blendCBumpImage = globalImages->flatNormalMap;
		}
	
		// if we wouldn't draw anything, don't call the Draw function
		if (
			((din->diffuseColor[ 0 ] > 0 ||
			  din->diffuseColor[ 1 ] > 0 ||
			  din->diffuseColor[ 2 ] > 0) && din->diffuseImage != globalImages->blackImage)
			|| ((din->specularColor[ 0 ] > 0 ||
				 din->specularColor[ 1 ] > 0 ||
				 din->specularColor[ 2 ] > 0) && din->specularImage != globalImages->blackImage) )
		{

			interactionList.Append( *din );
		}
	}
}


/*
=============
RB_GLSL_CreateDrawInteractions

=============
*/
static void RB_GLSL_CreateDrawInteractions(const viewLight_t& vLight, const drawSurf_t *surf, InteractionList& interactionList ) {

	if (r_skipInteractions.GetBool()) {
		return;
	}

	for (; surf; surf = surf->nextOnLight) {
		const idMaterial	*surfaceShader = surf->material;
		const float			*surfaceRegs = surf->shaderRegisters;
		const idMaterial	*lightShader = vLight.lightShader;
		const float			*lightRegs = vLight.shaderRegisters;
		drawInteraction_t	inter;
		inter.hasBumpMatrix = inter.hasDiffuseMatrix = inter.hasSpecularMatrix = false;

		if (!surf->geo || !surf->geo->ambientCache) {
			continue;
		}

		inter.surf = surf;
		inter.lightFalloffImage = vLight.falloffImage;

		R_GlobalPointToLocal( surf->space->modelMatrix, vLight.globalLightOrigin, inter.localLightOrigin.ToVec3() );
		R_GlobalPointToLocal( surf->space->modelMatrix, backEnd.viewDef->renderView.vieworg, inter.localViewOrigin.ToVec3() );
		inter.localLightOrigin[3] = 0;
		inter.localViewOrigin[3] = 1;

		// the base projections may be modified by texture matrix on light stages
		idPlane lightProject[4];
		for (int i = 0; i < 4; i++) {
			R_GlobalPlaneToLocal( surf->space->modelMatrix, vLight.lightProject[i], lightProject[i] );
		}

		for (int lightStageNum = 0; lightStageNum < lightShader->GetNumStages(); lightStageNum++) {
			const shaderStage_t	*lightStage = lightShader->GetStage( lightStageNum );

			// ignore stages that fail the condition
			if (!lightRegs[lightStage->conditionRegister]) {
				continue;
			}

			inter.lightImage = lightStage->texture.image;

			memcpy( inter.lightProjection, lightProject, sizeof(inter.lightProjection) );
			// now multiply the texgen by the light texture matrix
			if (lightStage->texture.hasMatrix) {
				float tmp[16];
				RB_GetShaderTextureMatrix( lightRegs, &lightStage->texture, tmp );
				RB_BakeTextureMatrixIntoTexgen( reinterpret_cast<class idPlane *>(inter.lightProjection), tmp );
			}

			inter.bumpImage = NULL;
			inter.specularImage = NULL;
			inter.diffuseImage = NULL;
			inter.diffuseColor[0] = inter.diffuseColor[1] = inter.diffuseColor[2] = inter.diffuseColor[3] = 0;
			inter.specularColor[0] = inter.specularColor[1] = inter.specularColor[2] = inter.specularColor[3] = 0;

			inter.blendABumpImage = NULL;
			inter.blendBBumpImage = NULL;
			inter.blendCBumpImage = NULL;

			inter.blendADiffuseImage = NULL;
			inter.blendBDiffuseImage = NULL;
			inter.blendCDiffuseImage = NULL;

			inter.blendASpecularImage = NULL;
			inter.blendBSpecularImage = NULL;
			inter.blendCSpecularImage = NULL;

			inter.rtcmImage = NULL;
			inter.rtcmFrontImage = NULL;
			inter.rtcmBackImage = NULL;
			inter.rtcmRightImage = NULL;
			inter.rtcmLeftImage = NULL;
			inter.rtcmUpImage = NULL;
			inter.rtcmDownImage = NULL;

			inter.blendMode = static_cast<materialBlendMode>(surfaceShader->GetBlendMode());

			float lightColor[4];

			// backEnd.lightScale is calculated so that lightColor[] will never exceed
			// tr.backEndRendererMaxLight
			lightColor[0] = backEnd.lightScale * lightRegs[lightStage->color.registers[0]];
			lightColor[1] = backEnd.lightScale * lightRegs[lightStage->color.registers[1]];
			lightColor[2] = backEnd.lightScale * lightRegs[lightStage->color.registers[2]];
			lightColor[3] = lightRegs[lightStage->color.registers[3]];

			// go through the individual stages
			for (int surfaceStageNum = 0; surfaceStageNum < surfaceShader->GetNumStages(); surfaceStageNum++) 
			{
				const shaderStage_t	*surfaceStage = surfaceShader->GetStage( surfaceStageNum );

				switch (surfaceStage->lighting) 
				{
				case SL_AMBIENT: 
				{
					// ignore ambient stages while drawing interactions
					break;
				}
				case SL_BUMP: 
				{
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister]) 
					{
						break;
					}
					// draw any previous interaction
					RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					inter.diffuseImage = NULL;
					inter.specularImage = NULL;
					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.bumpImage, inter.bumpMatrix, NULL );
					inter.hasBumpMatrix = surfaceStage->texture.hasMatrix;
					break;
				}
				case SL_DIFFUSE: 
				{
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister] || vLight.lightDef->parms.noDiffuse) 
					{
						break;
					}
					if (inter.diffuseImage) 
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.diffuseImage,
						inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
					inter.diffuseColor[0] *= lightColor[0];
					inter.diffuseColor[1] *= lightColor[1];
					inter.diffuseColor[2] *= lightColor[2];
					inter.diffuseColor[3] *= lightColor[3];
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasDiffuseMatrix = surfaceStage->texture.hasMatrix;
					break;
				}
				case SL_SPECULAR: 
				{
					// ignore stage that fails the condition
					if (!surfaceRegs[surfaceStage->conditionRegister] || vLight.lightDef->parms.noSpecular) 
					{
						break;
					}
					if (inter.specularImage) 
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.specularImage,
						inter.specularMatrix, inter.specularColor.ToFloatPtr() );
					inter.specularColor[0] *= lightColor[0];
					inter.specularColor[1] *= lightColor[1];
					inter.specularColor[2] *= lightColor[2];
					inter.specularColor[3] *= lightColor[3];
					inter.specularColor *= r_specularScale.GetFloat();
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasSpecularMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_BUMP_A:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] )
					{
						break;
					}
					// draw any previous interaction
					RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					inter.blendADiffuseImage = NULL;
					inter.blendASpecularImage = NULL;
					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendABumpImage, inter.bumpMatrix, NULL );
					inter.hasBumpMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_BUMP_B:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] )
					{
						break;
					}
					// draw any previous interaction
					RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					inter.blendBDiffuseImage = NULL;
					inter.blendBSpecularImage = NULL;
					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendBBumpImage, inter.bumpMatrix, NULL );
					inter.hasBumpMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_BUMP_C:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] )
					{
						break;
					}
					// draw any previous interaction
					RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					inter.blendCDiffuseImage = NULL;
					inter.blendCSpecularImage = NULL;
					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendCBumpImage, inter.bumpMatrix, NULL );
					inter.hasBumpMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_DIFFUSE_A:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noDiffuse )
					{
						break;
					}
					if ( inter.blendADiffuseImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendADiffuseImage,
										  inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
					inter.diffuseColor[ 0 ] *= lightColor[ 0 ];
					inter.diffuseColor[ 1 ] *= lightColor[ 1 ];
					inter.diffuseColor[ 2 ] *= lightColor[ 2 ];
					inter.diffuseColor[ 3 ] *= lightColor[ 3 ];
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasDiffuseMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_DIFFUSE_B:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noDiffuse )
					{
						break;
					}
					if ( inter.blendBDiffuseImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendBDiffuseImage,
										  inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
					inter.diffuseColor[ 0 ] *= lightColor[ 0 ];
					inter.diffuseColor[ 1 ] *= lightColor[ 1 ];
					inter.diffuseColor[ 2 ] *= lightColor[ 2 ];
					inter.diffuseColor[ 3 ] *= lightColor[ 3 ];
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasDiffuseMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_DIFFUSE_C:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noDiffuse )
					{
						break;
					}
					if ( inter.blendCDiffuseImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendCDiffuseImage,
										  inter.diffuseMatrix, inter.diffuseColor.ToFloatPtr() );
					inter.diffuseColor[ 0 ] *= lightColor[ 0 ];
					inter.diffuseColor[ 1 ] *= lightColor[ 1 ];
					inter.diffuseColor[ 2 ] *= lightColor[ 2 ];
					inter.diffuseColor[ 3 ] *= lightColor[ 3 ];
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasDiffuseMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_SPECULAR_A:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noSpecular )
					{
						break;
					}
					if ( inter.blendASpecularImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendASpecularImage,
										  inter.specularMatrix, inter.specularColor.ToFloatPtr() );
					inter.specularColor[ 0 ] *= lightColor[ 0 ];
					inter.specularColor[ 1 ] *= lightColor[ 1 ];
					inter.specularColor[ 2 ] *= lightColor[ 2 ];
					inter.specularColor[ 3 ] *= lightColor[ 3 ];
					inter.specularColor *= r_specularScale.GetFloat();
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasSpecularMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_SPECULAR_B:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noSpecular )
					{
						break;
					}
					if ( inter.blendBSpecularImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendBSpecularImage,
										  inter.specularMatrix, inter.specularColor.ToFloatPtr() );
					inter.specularColor[ 0 ] *= lightColor[ 0 ];
					inter.specularColor[ 1 ] *= lightColor[ 1 ];
					inter.specularColor[ 2 ] *= lightColor[ 2 ];
					inter.specularColor[ 3 ] *= lightColor[ 3 ];
					inter.specularColor *= r_specularScale.GetFloat();
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasSpecularMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				case SL_SPECULAR_C:
				{
					// ignore stage that fails the condition
					if ( !surfaceRegs[ surfaceStage->conditionRegister ] || vLight.lightDef->parms.noSpecular )
					{
						break;
					}
					if ( inter.blendCSpecularImage )
					{
						RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
					}

					R_SetDrawInteraction( surfaceStage, surfaceRegs, &inter.blendCSpecularImage,
										  inter.specularMatrix, inter.specularColor.ToFloatPtr() );
					inter.specularColor[ 0 ] *= lightColor[ 0 ];
					inter.specularColor[ 1 ] *= lightColor[ 1 ];
					inter.specularColor[ 2 ] *= lightColor[ 2 ];
					inter.specularColor[ 3 ] *= lightColor[ 3 ];
					inter.specularColor *= r_specularScale.GetFloat();
					inter.vertexColor = surfaceStage->vertexColor;
					inter.hasSpecularMatrix = surfaceStage->texture.hasMatrix;
					break;
				}

				}
			}

			// draw the final interaction
			RB_SubmittInteraction( &inter, interactionList, lightShader->IsAmbientLight() );
		}
	}
}

static void RB_GLSL_SubmitDrawInteractions(const viewLight_t& vLight, const InteractionList& interactionList) 
{
	if (interactionList.IsEmpty())
		return;

	// perform setup here that will be constant for all interactions
	GL_State( GLS_SRCBLEND_ONE | GLS_DSTBLEND_ONE | GLS_DEPTHMASK | backEnd.depthFunc );

	GL_UseProgram( interactionProgram );

	fhRenderProgram::SetShading( r_shading.GetInteger() );
	fhRenderProgram::SetSpecularExp( r_specularExp.GetFloat() );
	fhRenderProgram::SetAmbientLight( vLight.lightDef->lightShader->IsAmbientLight() ? 1 : 0 );

	if (vLight.lightDef->ShadowMode() == shadowMode_t::ShadowMap) {
		const idVec4 globalLightOrigin = idVec4( vLight.globalLightOrigin, 1 );
		fhRenderProgram::SetGlobalLightOrigin( globalLightOrigin );

		const float shadowBrightness = vLight.lightDef->ShadowBrightness();
		const float shadowSoftness = vLight.lightDef->ShadowSoftness();
		fhRenderProgram::SetShadowParams( idVec4( shadowSoftness, shadowBrightness, vLight.nearClip[0], vLight.farClip[0] ) );

		if(vLight.lightDef->parms.parallel) {
			//parallel light
			fhRenderProgram::SetShadowMappingMode( 3 );
			fhRenderProgram::SetPointLightProjectionMatrices( vLight.viewProjectionMatrices[0].ToFloatPtr() );
			fhRenderProgram::SetShadowCoords( vLight.shadowCoords, 6 );
			fhRenderProgram::SetCascadeDistances(
				r_smCascadeDistance0.GetFloat(),
				r_smCascadeDistance1.GetFloat(),
				r_smCascadeDistance2.GetFloat(),
				r_smCascadeDistance3.GetFloat(),
				r_smCascadeDistance4.GetFloat());

			idVec4 shadowmapSizes[6] = {
				idVec4(vLight.nearClip[0], vLight.farClip[0], vLight.width[0], vLight.height[0]),
				idVec4(vLight.nearClip[1], vLight.farClip[1], vLight.width[1], vLight.height[1]),
				idVec4(vLight.nearClip[2], vLight.farClip[2], vLight.width[2], vLight.height[2]),
				idVec4(vLight.nearClip[3], vLight.farClip[3], vLight.width[3], vLight.height[3]),
				idVec4(vLight.nearClip[4], vLight.farClip[4], vLight.width[4], vLight.height[4]),
				idVec4(vLight.nearClip[5], vLight.farClip[5], vLight.width[5], vLight.height[5])
			};

			fhRenderProgram::SetShadowMapSize(shadowmapSizes, 6);
		}
		else if (vLight.lightDef->parms.pointLight) {
			//point light
			fhRenderProgram::SetShadowMappingMode( 1 );
			fhRenderProgram::SetPointLightProjectionMatrices( vLight.viewProjectionMatrices[0].ToFloatPtr() );
			fhRenderProgram::SetShadowCoords(vLight.shadowCoords, 6);

			{
				const idMat3 axis = vLight.lightDef->parms.axis;

				float viewerMatrix[16];

				viewerMatrix[0] = axis[0][0];
				viewerMatrix[4] = axis[0][1];
				viewerMatrix[8] = axis[0][2];
				viewerMatrix[12] = 0;

				viewerMatrix[1] = axis[1][0];
				viewerMatrix[5] = axis[1][1];
				viewerMatrix[9] = axis[1][2];
				viewerMatrix[13] = 0;

				viewerMatrix[2] = axis[2][0];
				viewerMatrix[6] = axis[2][1];
				viewerMatrix[10] = axis[2][2];
				viewerMatrix[14] = 0;

				viewerMatrix[3] = 0;
				viewerMatrix[7] = 0;
				viewerMatrix[11] = 0;
				viewerMatrix[15] = 1;

				fhRenderProgram::SetInverseLightRotation( viewerMatrix );
			}
		}
		else {
			//projected light
			fhRenderProgram::SetShadowMappingMode( 2 );
			fhRenderProgram::SetSpotLightProjectionMatrix( vLight.viewProjectionMatrices[0].ToFloatPtr() );
			fhRenderProgram::SetShadowCoords(vLight.shadowCoords, 1);
		}
	}
	else {
		//no shadows
		fhRenderProgram::SetShadowMappingMode( 0 );
	}

	//make sure depth hacks are disabled
	//FIXME(johl): why is (sometimes) a depth hack enabled at this point?
	RB_LeaveDepthHack();

	fhRenderProgram::SetProjectionMatrix( backEnd.viewDef->projectionMatrix );
	fhRenderProgram::SetPomMaxHeight( -1 );

	const viewEntity_t* currentSpace = nullptr;
	stageVertexColor_t currentVertexColor = (stageVertexColor_t)-1;
	bool currentPomEnabled = false;
	idScreenRect currentScissor;
	bool depthHackActive = false;
	bool currentHasBumpMatrix = false;
	bool currentHasDiffuseMatrix = false;
	bool currentHasSpecularMatrix = false;
	idVec4 currentDiffuseColor = idVec4( 1, 1, 1, 1 );
	idVec4 currentSpecularColor = idVec4( 1, 1, 1, 1 );

	fhRenderProgram::SetDiffuseColor( currentDiffuseColor );
	fhRenderProgram::SetSpecularColor( currentSpecularColor );
	fhRenderProgram::SetBumpMatrix( idVec4::identityS, idVec4::identityT );
	fhRenderProgram::SetSpecularMatrix( idVec4::identityS, idVec4::identityT );
	fhRenderProgram::SetDiffuseMatrix( idVec4::identityS, idVec4::identityT );

	glDepthRange(0, 1);

	if (r_useScissor.GetBool()) {
		auto fb = fhFramebuffer::GetCurrentDrawBuffer();
		glScissor( 0, 0, fb->GetWidth(), fb->GetHeight() );
		currentScissor.x1 = 0;
		currentScissor.y1 = 0;
		currentScissor.x2 = fb->GetWidth();
		currentScissor.y2 = fb->GetHeight();
	}

	const int num = interactionList.Num();
	for (int i = 0; i < num; ++i) 
	{
		const auto& din = interactionList[i];

		const auto offset = vertexCache.Bind( din.surf->geo->ambientCache );
		GL_SetupVertexAttributes( fhVertexLayout::Draw, offset );

		if (currentSpace != din.surf->space) 
		{
			fhRenderProgram::SetModelMatrix( din.surf->space->modelMatrix );
			fhRenderProgram::SetModelViewMatrix( din.surf->space->modelViewMatrix );

			if (din.surf->space->modelDepthHack) 
			{
				RB_EnterModelDepthHack( din.surf->space->modelDepthHack );
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = true;
			}
			else if (din.surf->space->weaponDepthHack) 
			{
				RB_EnterWeaponDepthHack();
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = true;
			}
			else if (depthHackActive) 
			{
				RB_LeaveDepthHack();
				fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
				depthHackActive = false;
			}

			// change the scissor if needed
			if (r_useScissor.GetBool() && !currentScissor.Equals( din.surf->scissorRect )) 
			{
				currentScissor = din.surf->scissorRect;
				glScissor( backEnd.viewDef->viewport.x1 + currentScissor.x1,
					backEnd.viewDef->viewport.y1 + currentScissor.y1,
					currentScissor.x2 + 1 - currentScissor.x1,
					currentScissor.y2 + 1 - currentScissor.y1 );
			}

			currentSpace = din.surf->space;
		}

		fhRenderProgram::SetLocalLightOrigin( din.localLightOrigin );
		fhRenderProgram::SetLocalViewOrigin( din.localViewOrigin );
		fhRenderProgram::SetLightProjectionMatrix( din.lightProjection[0], din.lightProjection[1], din.lightProjection[2] );
		fhRenderProgram::SetLightFallOff( din.lightProjection[3] );

		if (din.hasBumpMatrix) 
		{
			fhRenderProgram::SetBumpMatrix( din.bumpMatrix[0], din.bumpMatrix[1] );
			currentHasBumpMatrix = true;
		}
		else if (currentHasBumpMatrix) 
		{
			fhRenderProgram::SetBumpMatrix( idVec4::identityS, idVec4::identityT );
			currentHasBumpMatrix = false;
		}

		if (din.hasDiffuseMatrix) 
		{
			fhRenderProgram::SetDiffuseMatrix( din.diffuseMatrix[0], din.diffuseMatrix[1] );
			currentHasDiffuseMatrix = true;
		}
		else if (currentHasDiffuseMatrix) 
		{
			fhRenderProgram::SetDiffuseMatrix( idVec4::identityS, idVec4::identityT );
			currentHasDiffuseMatrix = false;
		}

		if (din.hasSpecularMatrix) 
		{
			fhRenderProgram::SetSpecularMatrix( din.specularMatrix[0], din.specularMatrix[1] );
			currentHasSpecularMatrix = true;
		}
		else if (currentHasSpecularMatrix) 
		{
			fhRenderProgram::SetSpecularMatrix( idVec4::identityS, idVec4::identityT );
			currentHasSpecularMatrix = false;
		}

		if (currentVertexColor != din.vertexColor) 
		{
			switch (din.vertexColor) 
			{
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
			currentVertexColor = din.vertexColor;
		}

		if (din.diffuseColor != currentDiffuseColor) 
		{
			fhRenderProgram::SetDiffuseColor( din.diffuseColor );
			currentDiffuseColor = din.diffuseColor;
		}

		if (din.specularColor != currentSpecularColor) 
		{
			fhRenderProgram::SetSpecularColor( din.specularColor );
			currentSpecularColor = din.specularColor;
		}

		static bool pomEnabled = false;

		if ( din.blendMode == materialBlendMode::BLEND_NONE )
		{
			pomEnabled = r_pomEnabled.GetBool() && din.specularImage->hasAlpha;
		}
		else if ( din.blendMode == materialBlendMode::BLEND_2WAY )
		{
			pomEnabled = r_pomEnabled.GetBool()
				&& din.blendASpecularImage->hasAlpha
				&& din.blendBSpecularImage->hasAlpha;
		}
		else if ( din.blendMode == materialBlendMode::BLEND_3WAY )
		{
			pomEnabled = r_pomEnabled.GetBool() 
				&& din.blendASpecularImage->hasAlpha
				&& din.blendBSpecularImage->hasAlpha 
				&& din.blendCSpecularImage->hasAlpha;
		}

		if (pomEnabled != currentPomEnabled) 
		{
			if (pomEnabled) 
			{
				fhRenderProgram::SetPomMaxHeight( r_pomMaxHeight.GetFloat() );
			}
			else 
			{
				fhRenderProgram::SetPomMaxHeight( -1 );
			}
		}

		// in the shader layout, 2 and 3 are reserved for light textures
		din.lightFalloffImage->Bind( 2 );
		din.lightImage->Bind( 3 );

		idVec4 shaderParms( din.blendMode, 0, 0, 0 );
		fhRenderProgram::SetShaderParm( 1, shaderParms );

		// Bind bump, diffuse and spec if blend mode is none
		if ( din.blendMode == materialBlendMode::BLEND_NONE )
		{
			fhRenderProgram::SetNormalMapEncoding( RB_GetNormalEncoding( din.bumpImage ) );

			din.bumpImage->Bind( 1 );
			
			din.diffuseImage->Bind( 4 );
			din.specularImage->Bind( 5 );

			din.rtcmFrontImage->Bind( 8 );
			din.rtcmImage->cubeFiles = CF_CAMERA;
			din.rtcmImage->type = TT_CUBIC;
			din.rtcmImage->Bind( 9 );
		}

		else if ( din.blendMode == materialBlendMode::BLEND_2WAY )
		{
			fhRenderProgram::SetNormalMapEncoding( RB_GetNormalEncoding( din.blendABumpImage ) );

			din.blendABumpImage->Bind( 1 );
			din.blendADiffuseImage->Bind( 4 );
			din.blendASpecularImage->Bind( 5 );

			din.blendBBumpImage->Bind( 8 );
			din.blendBDiffuseImage->Bind( 9 );
			din.blendBSpecularImage->Bind( 10 );
		}

		// Bind a whole set of B+D+S maps if we're blending in three-way mode
		// TO-DO: Check the performance
		else if ( din.blendMode == materialBlendMode::BLEND_3WAY )
		{
			fhRenderProgram::SetNormalMapEncoding( RB_GetNormalEncoding( din.blendABumpImage ) );

			din.blendABumpImage->Bind( 1 );
			din.blendADiffuseImage->Bind( 4 );
			din.blendASpecularImage->Bind( 5 );

			din.blendBBumpImage->Bind( 8 );
			din.blendBDiffuseImage->Bind( 9 );
			din.blendBSpecularImage->Bind( 10 );

			din.blendCBumpImage->Bind( 11 );
			din.blendCDiffuseImage->Bind( 12 );
			din.blendCSpecularImage->Bind( 13 );
		}

		// draw it
		backEnd.stats.groups[backEndGroup::Interaction].drawcalls += 1;
		backEnd.stats.groups[backEndGroup::Interaction].tris += din.surf->geo->numIndexes / 3;
		RB_DrawElementsWithCounters( din.surf->geo );
	}

	if (depthHackActive) {
		RB_LeaveDepthHack();
		fhRenderProgram::SetProjectionMatrix( GL_ProjectionMatrix.Top() );
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

void InteractionList::AddDrawSurfacesOnLight(const viewLight_t& vLight, const drawSurf_t *surf ) {
	RB_GLSL_CreateDrawInteractions(vLight, surf, *this);
}

void InteractionList::Submit(const viewLight_t& vLight) {
	RB_GLSL_SubmitDrawInteractions(vLight, *this);
}
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

struct fhUniform {
	enum Value
	{
		ModelMatrix,
		ViewMatrix,
		ModelViewMatrix,
		ProjectionMatrix,
		LocalLightOrigin,
		LocalViewOrigin,
		LightProjectionMatrixS,
		LightProjectionMatrixT,
		LightProjectionMatrixQ,
		LightFallOff,
		BumpMatrixS,
		BumpMatrixT,
		DiffuseMatrixS,
		DiffuseMatrixT,
		SpecularMatrixS,
		SpecularMatrixT,
		ColorModulate,
		ColorAdd,
		DiffuseColor,
		SpecularColor,
		AmbientLight,
		ShaderParm0,
		ShaderParm1,
		ShaderParm2,
		ShaderParm3,
		TextureMatrix0,
		AlphaTestEnabled,
		AlphaTestThreshold,
		CurrentRenderSize,
		ClipRange,
		DepthBlendMode,
		DepthBlendRange,
		PomMaxHeight,
		Shading,
		specularExp,
		ShadowMappingMode,
		SpotLightProjection,
		PointLightProjection,
		GlobalLightOrigin,
		ShadowParams,
		ShadowCoords,
		CascadeDistances,
		ShadowMapSize,
		InverseLightRotation,
		NormalMapEncoding,
		RealtimeMode,
		SsrColor,
		RtCmFrColor,
		RtCmBkColor,
		RtCmLtColor,
		RtCmRtColor,
		RtCmUpColor,
		RtCmDnColor,
		NUM
	};

	Value value;
};

struct shadowCoord_t {
	idVec2 scale;
	idVec2 offset;
};

struct fhRenderProgram {
public:
	static const int vertex_attrib_position = 0;
	static const int vertex_attrib_texcoord = 1;
	static const int vertex_attrib_normal = 2;
	static const int vertex_attrib_color = 3;
	static const int vertex_attrib_binormal = 4;
	static const int vertex_attrib_tangent = 5;
	static const int vertex_attrib_position_shadow = 6;

	static const int normal_map_encoding_rgb = 0;
	static const int normal_map_encoding_dxrg = 1;
	static const int normal_map_encoding_ag = 2;
	static const int normal_map_encoding_rg = 3;

public:
	fhRenderProgram();
	~fhRenderProgram();

	void Load(const char* vertexShader, const char* fragmentShader);
	void Reload();
	void Purge();
	bool IsLoaded() const { return ident != 0; }
	bool Bind(bool force = false) const;

	const char* vertexShader() const;
	const char* fragmentShader() const;

	static void Unbind();
	static void ReloadAll();
	static void PurgeAll();
	static void Init();

	static void SetModelMatrix(const float* m);
	static void SetViewMatrix(const float* m);
	static void SetModelViewMatrix(const float* m);
	static void SetProjectionMatrix(const float* m);

	static void SetLocalLightOrigin(const idVec4& v);
	static void SetLocalViewOrigin(const idVec4& v);
	static void SetLightProjectionMatrix(const idVec4& s, const idVec4& t, const idVec4& q);
	static void SetLightFallOff(const idVec4& v);
	static void SetBumpMatrix(const idVec4& s, const idVec4& t);
	static void SetDiffuseMatrix(const idVec4& s, const idVec4& t);
	static void SetSpecularMatrix(const idVec4& s, const idVec4& t);
	static void SetTextureMatrix(const float* m);

	static void SetColorModulate(const idVec4& c);
	static void SetColorAdd(const idVec4& c);
	static void SetDiffuseColor(const idVec4& c);
	static void SetSpecularColor(const idVec4& c);

	static void SetShaderParm(int index, const idVec4& v);
	static void SetAlphaTestEnabled(bool enabled);
	static void SetAlphaTestThreshold(float threshold);
	static void SetCurrentRenderSize(const idVec2& uploadSize, const idVec2& viweportSize);
	static void SetClipRange(float nearClip, float farClip);
	static void SetDepthBlendMode(int m);
	static void SetDepthBlendRange(float range);
	static void SetPomMaxHeight(float h);
	static void SetShading(int shading);
	static void SetSpecularExp(float e);

	static void SetShadowMappingMode(int m);
	static void SetSpotLightProjectionMatrix(const float* m);
	static void SetPointLightProjectionMatrices(const float* m);
	static void SetShadowParams(const idVec4& v);
	static void SetGlobalLightOrigin(const idVec4& v);

	static void SetShadowCoords(const shadowCoord_t* coords, int num);
	static void SetCascadeDistances(float d1, float d2, float d3, float d4, float d5);
	static void SetShadowMapSize(const idVec4* sizes, int numSizes);

	static void SetInverseLightRotation(const float* m);
	static void SetNormalMapEncoding( int encoding );
	static void SetAmbientLight(int amb);

	static void SetRealtimeMode( int mode );

private:
	static bool dirty[fhUniform::NUM];
	static idVec4 currentColorModulate;
	static idVec4 currentColorAdd;
	static idVec4 currentDiffuseColor;
	static idVec4 currentSpecularColor;
	static idVec4 currentDiffuseMatrix[2];
	static idVec4 currentSpecularMatrix[2];
	static idVec4 currentBumpMatrix[2];
	static bool   currentAlphaTestEnabled;
	static float  currentAlphaTestThreshold;
	static float  currentPomMaxHeight;
	static int    currentNormalMapEncoding;

	static int	  currentRealtimeMode; // 0 = no realtime, 1 = screenspace, 2 = realtime cubemap

	static const GLint* currentUniformLocations;
	void Load();

	char   vertexShaderName[64];
	char   fragmentShaderName[64];
	GLuint ident;
	GLint  uniformLocations[fhUniform::NUM];
};

extern const fhRenderProgram* shadowProgram;
extern const fhRenderProgram* interactionProgram;
extern const fhRenderProgram* depthProgram;
extern const fhRenderProgram* shadowmapProgram;
extern const fhRenderProgram* defaultProgram;
extern const fhRenderProgram* skyboxProgram;
extern const fhRenderProgram* bumpyEnvProgram;
extern const fhRenderProgram* fogLightProgram;
extern const fhRenderProgram* vertexColorProgram;
extern const fhRenderProgram* flatColorProgram;
extern const fhRenderProgram* intensityProgram;
extern const fhRenderProgram* blendLightProgram;
extern const fhRenderProgram* depthblendProgram;
extern const fhRenderProgram* debugDepthProgram;
extern const fhRenderProgram* postprocessProgram;
extern const fhRenderProgram* bloomProgram;
extern const fhRenderProgram* blurProgram;
extern const fhRenderProgram* brightnessGammaProgram;

extern const fhRenderProgram* rtcmProgram;


ID_INLINE void fhRenderProgram::SetModelMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::ModelMatrix], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetViewMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::ViewMatrix], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetModelViewMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::ModelViewMatrix], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetProjectionMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::ProjectionMatrix], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetLocalLightOrigin( const idVec4& v ) {
	glUniform4fv(currentUniformLocations[fhUniform::LocalLightOrigin], 1, v.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetLocalViewOrigin( const idVec4& v ) {
	glUniform4fv(currentUniformLocations[fhUniform::LocalViewOrigin], 1, v.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetLightProjectionMatrix( const idVec4& s, const idVec4& t, const idVec4& q ) {
	glUniform4fv(currentUniformLocations[fhUniform::LightProjectionMatrixS], 1, s.ToFloatPtr());
	glUniform4fv(currentUniformLocations[fhUniform::LightProjectionMatrixT], 1, t.ToFloatPtr());
	glUniform4fv(currentUniformLocations[fhUniform::LightProjectionMatrixQ], 1, q.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetLightFallOff( const idVec4& v ) {
	glUniform4fv(currentUniformLocations[fhUniform::LightFallOff], 1, v.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetBumpMatrix( const idVec4& s, const idVec4& t ) {
	if (dirty[fhUniform::BumpMatrixS] || !currentBumpMatrix[0].Compare( s, 0.001 ) || !currentBumpMatrix[1].Compare( t, 0.001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::BumpMatrixS], 1, s.ToFloatPtr() );
		glUniform4fv( currentUniformLocations[fhUniform::BumpMatrixT], 1, t.ToFloatPtr() );

		currentBumpMatrix[0] = s;
		currentBumpMatrix[1] = t;
		dirty[fhUniform::BumpMatrixS] = false;
	}
}

ID_INLINE void fhRenderProgram::SetDiffuseMatrix( const idVec4& s, const idVec4& t ) {
	if (dirty[fhUniform::DiffuseMatrixS] || !currentDiffuseMatrix[0].Compare( s, 0.001 ) || !currentDiffuseMatrix[1].Compare( t, 0.001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::DiffuseMatrixS], 1, s.ToFloatPtr() );
		glUniform4fv( currentUniformLocations[fhUniform::DiffuseMatrixT], 1, t.ToFloatPtr() );

		currentDiffuseMatrix[0] = s;
		currentDiffuseMatrix[1] = t;
		dirty[fhUniform::DiffuseMatrixS] = false;
	}
}

ID_INLINE void fhRenderProgram::SetSpecularMatrix( const idVec4& s, const idVec4& t ) {
	if (dirty[fhUniform::SpecularMatrixS] || !currentSpecularMatrix[0].Compare( s, 0.001 ) || !currentSpecularMatrix[1].Compare( t, 0.001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::SpecularMatrixS], 1, s.ToFloatPtr() );
		glUniform4fv( currentUniformLocations[fhUniform::SpecularMatrixT], 1, t.ToFloatPtr() );

		currentSpecularMatrix[0] = s;
		currentSpecularMatrix[1] = t;
		dirty[fhUniform::SpecularMatrixS] = false;
	}
}

ID_INLINE void fhRenderProgram::SetTextureMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::TextureMatrix0], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetColorModulate( const idVec4& c ) {
	if(dirty[fhUniform::ColorModulate] || !currentColorModulate.Compare(c, 0.000001)) {
		glUniform4fv( currentUniformLocations[fhUniform::ColorModulate], 1, c.ToFloatPtr() );
		currentColorModulate = c;
		dirty[fhUniform::ColorModulate] = false;
	}
}

ID_INLINE void fhRenderProgram::SetColorAdd( const idVec4& c ) {
	if (dirty[fhUniform::ColorAdd] || !currentColorAdd.Compare( c, 0.000001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::ColorAdd], 1, c.ToFloatPtr() );
		currentColorAdd = c;
		dirty[fhUniform::ColorAdd] = false;
	}
}

ID_INLINE void fhRenderProgram::SetDiffuseColor( const idVec4& c ) {
	if (dirty[fhUniform::DiffuseColor] || !currentDiffuseColor.Compare( c, 0.001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::DiffuseColor], 1, c.ToFloatPtr() );
		currentDiffuseColor = c;
		dirty[fhUniform::DiffuseColor] = false;
	}
}

ID_INLINE void fhRenderProgram::SetSpecularColor( const idVec4& c ) {
	if (dirty[fhUniform::SpecularColor] || !currentSpecularColor.Compare( c, 0.001 )) {
		glUniform4fv( currentUniformLocations[fhUniform::SpecularColor], 1, c.ToFloatPtr() );
		currentSpecularColor = c;
		dirty[fhUniform::SpecularColor] = false;
	}
}

ID_INLINE void fhRenderProgram::SetShaderParm( int index, const idVec4& v ) {
	assert(index >= 0 && index < 4);
	glUniform4fv( currentUniformLocations[fhUniform::ShaderParm0 + index], 1, v.ToFloatPtr() );
}

ID_INLINE void fhRenderProgram::SetAlphaTestEnabled( bool enabled ) {
	if (dirty[fhUniform::AlphaTestEnabled] || currentAlphaTestEnabled != enabled) {
		glUniform1i(currentUniformLocations[fhUniform::AlphaTestEnabled], static_cast<int>(enabled));
		currentAlphaTestEnabled = enabled;
		dirty[fhUniform::AlphaTestEnabled] = false;
	}
}

ID_INLINE void fhRenderProgram::SetAlphaTestThreshold( float threshold ) {
	if (dirty[fhUniform::AlphaTestThreshold] || std::abs(currentAlphaTestThreshold - threshold) > 0.001) {
		glUniform1f(currentUniformLocations[fhUniform::AlphaTestThreshold], threshold);
		currentAlphaTestThreshold = threshold;
		dirty[fhUniform::AlphaTestThreshold] = false;
	}
}

ID_INLINE void fhRenderProgram::SetCurrentRenderSize( const idVec2& uploadSize, const idVec2& viewportSize ) {
	glUniform4f(currentUniformLocations[fhUniform::CurrentRenderSize], uploadSize.x, uploadSize.y, viewportSize.x, viewportSize.y);
}

ID_INLINE void fhRenderProgram::SetClipRange( float nearClip, float farClip ) {
	glUniform2f(currentUniformLocations[fhUniform::ClipRange], nearClip, farClip);
}

ID_INLINE void fhRenderProgram::SetDepthBlendMode( int m ) {
	glUniform1i(currentUniformLocations[fhUniform::DepthBlendMode], m);
}

ID_INLINE void fhRenderProgram::SetDepthBlendRange( float range ) {
	glUniform1f(currentUniformLocations[fhUniform::DepthBlendRange], range);
}

ID_INLINE void fhRenderProgram::SetPomMaxHeight( float h ) {
	if (dirty[fhUniform::PomMaxHeight] || std::abs( currentPomMaxHeight - h ) > 0.001) {
		glUniform1f( currentUniformLocations[fhUniform::PomMaxHeight], h );
		currentPomMaxHeight = h;
		dirty[fhUniform::PomMaxHeight] = false;
	}
}

ID_INLINE void fhRenderProgram::SetShading( int shading ) {
	glUniform1i(currentUniformLocations[fhUniform::Shading], shading);
}

ID_INLINE void fhRenderProgram::SetSpecularExp( float e ) {
	glUniform1f(currentUniformLocations[fhUniform::specularExp], e);
}

ID_INLINE void fhRenderProgram::SetShadowMappingMode( int m ) {
	glUniform1i(currentUniformLocations[fhUniform::ShadowMappingMode], m);
}

ID_INLINE void fhRenderProgram::SetAmbientLight(int amb) {
	glUniform1i(currentUniformLocations[fhUniform::AmbientLight], amb);
}

ID_INLINE void fhRenderProgram::SetSpotLightProjectionMatrix( const float* m ) {
	glUniformMatrix4fv(currentUniformLocations[fhUniform::SpotLightProjection], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetPointLightProjectionMatrices( const float* m ) {
	if(currentUniformLocations[fhUniform::PointLightProjection] != -1) {
		glUniformMatrix4fv(currentUniformLocations[fhUniform::PointLightProjection], 6, GL_FALSE, m);
	}
}

ID_INLINE void fhRenderProgram::SetShadowParams( const idVec4& v ) {
	glUniform4fv(currentUniformLocations[fhUniform::ShadowParams], 1, v.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetGlobalLightOrigin( const idVec4& v ) {
	glUniform4fv(currentUniformLocations[fhUniform::GlobalLightOrigin], 1, v.ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetShadowCoords(const shadowCoord_t* coords, int num) {
	static_assert(sizeof(shadowCoord_t) == sizeof(float)*4, "");
	assert(num > 0 && num <= 6); //num==0 is probably ok technically, but it seems like a bug, so assert num>0
	glUniform4fv(currentUniformLocations[fhUniform::ShadowCoords], num, reinterpret_cast<const float*>(coords));
}

ID_INLINE void fhRenderProgram::SetCascadeDistances(float d1, float d2, float d3, float d4, float d5) {
	float distances[] = {
		d1, d2, d3, d4, d5
	};

	glUniform1fv(currentUniformLocations[fhUniform::CascadeDistances], 5, distances);
}

ID_INLINE void fhRenderProgram::SetShadowMapSize(const idVec4* sizes, int numSizes) {
	glUniform4fv(currentUniformLocations[fhUniform::ShadowMapSize], numSizes, sizes[0].ToFloatPtr());
}

ID_INLINE void fhRenderProgram::SetInverseLightRotation(const float* m) {
	glUniformMatrix4fv( currentUniformLocations[fhUniform::InverseLightRotation], 1, GL_FALSE, m);
}

ID_INLINE void fhRenderProgram::SetNormalMapEncoding( int encoding ) {
	glUniform1i( currentUniformLocations[fhUniform::NormalMapEncoding], encoding );
}

ID_INLINE void fhRenderProgram::SetRealtimeMode( int mode )
{
	glUniform1i( currentUniformLocations[ fhUniform::RealtimeMode ], mode );
}
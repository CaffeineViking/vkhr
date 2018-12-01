//--------------------------------------------------------------------------------------
// Holds all common declarations.
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
cbuffer cbPerFrame : register(b0)
{
	matrix      g_mViewProj						: packoffset(c0);

	matrix		g_mClipToViewport				: packoffset(c4);
	matrix		g_mInvViewProjViewport			: packoffset(c8);

	matrix		g_mClipSpace					: packoffset(c12);

	// float4(g_ScreenWidth, g_ScreenHeight, g_NearPlane, g_FarPlane);
	float4		g_WinSize						: packoffset(c16);

	uint        g_NumVerticesPerStrand			: packoffset(c17.x);
	uint        g_NumStrands					: packoffset(c17.y);
	uint2		g_TileDim						: packoffset(c17.z);

	float		g_HalfHairThickness				: packoffset(c18.x);
	float		g_MinHairThickness				: packoffset(c18.y);
	float		g_RcpMinHairThickness			: packoffset(c18.z);

	float		g_FieldOfView					: packoffset(c18.w);

	uint2		g_TileDim64						: packoffset(c19);
	uint2		g_TileDim32						: packoffset(c19.z);
	uint2		g_TileDim16						: packoffset(c20);
	uint2		g_TileDim8						: packoffset(c20.z);
	uint2		g_TileDim4						: packoffset(c21);

	// Material parameters for hair shading
	float2		g_SpecularExponents				: packoffset(c21.z); // Ex1, Ex2
	float4		g_EyePos						: packoffset(c22);
	float4		g_PointLightPos					: packoffset(c23);
	float4		g_PointLightColor				: packoffset(c24);
	float4		g_AmbientLightColor				: packoffset(c25);
	float4		g_HairBaseColor					: packoffset(c26);
	float4		g_MatKValue						: packoffset(c27); // Ka, Kd, Ks1, Ks2

};

//--------------------------------------------------------------------------------------
// Shader Resource Buffers
//--------------------------------------------------------------------------------------
StructuredBuffer<float4>    g_HairVertexPositions		: register(t0);
#include "Curves.hlsl"

#define NUM_ELEMENTS_TO_PRELOAD 64
#define FLOAT_MAX 3.402823466e+38F

//--------------------------------------------------------------------------------------
// Constant Buffer
//--------------------------------------------------------------------------------------
cbuffer cbCounts : register(b1)
{
	uint		g_TileCurvePointersCount					: packoffset(c0.x);
};

//--------------------------------------------------------------------------------------
//		Structs
//--------------------------------------------------------------------------------------

struct CurvePointer {
	uint curvePointer;
	uint depth;
	uint screenClipParams; // Used to clip in screen space
	uint viewPortClipParams; // Used to clip in homogeneous clip space
};

struct TileCurvePointer {
	uint linearTilePointer;
	CurvePointer curvePointer; // Contains curve id and clip coordinates 
};

//--------------------------------------------------------------------------------------
//		Groupshared memory.
//--------------------------------------------------------------------------------------
groupshared uint	g_StartIndex;
groupshared uint	g_CurveCount;

//--------------------------------------------------------------------------------------
//		Unordered Access Buffers
//--------------------------------------------------------------------------------------
// Tile 4
RWStructuredBuffer<TileCurvePointer>					g_TileCurvePointers						: register(u2);
RWStructuredBuffer<uint>								g_PerBinCounter							: register(u3);
RWTexture2D<float>										g_DepthBuffer8							: register(u4);

// Reordering
RWStructuredBuffer<uint>								g_PrefixSumInc							: register(u5);
RWStructuredBuffer<CurvePointer>						g_CurvePointers							: register(u6);
RWTexture2D<uint>										g_PerBinCounterTex						: register(u7);

// Dispatch indirect
RWStructuredBuffer<uint>								g_FilledTiles							: register(u8);

// Draw stage
RWStructuredBuffer<float4>								g_NearestColors							: register(u9);
RWStructuredBuffer<float>								g_NearestDepths							: register(u10);


//--------------------------------------------------------------------------------------
//		Shader Resource Buffers
//--------------------------------------------------------------------------------------
Texture2D<float>		g_DepthBuffer												: register(t1);

//--------------------------------------------------------------------------------------
//
//		GetDepthAsUint
//
//		Converts the floating point depth to an unsigned integer.
//
//--------------------------------------------------------------------------------------
inline uint GetDepthAsUint(float depth) {
	return  (uint)(depth * UINT_MAX);
}

//--------------------------------------------------------------------------------------
//
//		LinearizeDepth
//
//		Converts the exponential depth to a linear depth.
//
//--------------------------------------------------------------------------------------
inline float LinearizeDepth(float depth) {
	float n = g_WinSize.z;
	float f = g_WinSize.w;
	return (2 * n) / (f + n - depth * (f - n));
}



//--------------------------------------------------------------------------------------
//
//		Pack and Unpack CurvePointer
//
//		Constructor method for a new tile curve pointer.
//
//--------------------------------------------------------------------------------------
inline CurvePointer Pack(uint curvePointer, uint depth,
	float t0ViewPort, float t1ViewPort, float t0Screen, float t1Screen)
{
	CurvePointer cp;
	cp.curvePointer = curvePointer;
	cp.depth = depth;
	cp.viewPortClipParams = ((uint)(0xFFFF * t0ViewPort) << 16) | ((uint)(0xFFFF * t1ViewPort) & 0xFFFF);
	cp.screenClipParams = ((uint)(0xFFFF * t0Screen) << 16) | ((uint)(0xFFFF * t1Screen) & 0xFFFF);
	return cp;
}

inline void Unpack(CurvePointer cp, out uint curvePointer, out uint depth,
	out float t0ViewPort, out float t1ViewPort, out float t0Screen, out float t1Screen)
{
	curvePointer = cp.curvePointer;
	depth = cp.depth;
	t0ViewPort = (cp.viewPortClipParams >> 16) / (float)0xFFFF;
	t1ViewPort = (cp.viewPortClipParams & 0xFFFF) / (float)0xFFFF;
	t0Screen = (cp.screenClipParams >> 16) / (float)0xFFFF;
	t1Screen = (cp.screenClipParams & 0xFFFF) / (float)0xFFFF;
}

//--------------------------------------------------------------------------------------
//
//		Pack and Unpack TileCurvePointer
//
//		Constructor method for a new tile curve pointer.
//
//--------------------------------------------------------------------------------------
inline TileCurvePointer Pack(uint curvePointer, uint linearTileIndex, uint depth,
	float t0ViewPort, float t1ViewPort, float t0Screen, float t1Screen)
{
	TileCurvePointer cp;
	cp.linearTilePointer = linearTileIndex;
	cp.curvePointer = Pack(curvePointer, depth, t0ViewPort, t1ViewPort, t0Screen, t1Screen);
	return cp;
}

inline void Unpack(TileCurvePointer cp, out uint curvePointer, out uint linearTileIndex, out uint depth, 
	out float t0ViewPort, out float t1ViewPort, out float t0Screen, out float t1Screen)
{
	Unpack(cp.curvePointer, curvePointer, depth, t0ViewPort, t1ViewPort, t0Screen, t1Screen);
	linearTileIndex = cp.linearTilePointer;
}


//--------------------------------------------------------------------------------------
//
//		GetRadiusWeights
//
//		Computes the radius weights �(0, 1) for each control point given the 
//		index of the first control point vertex.
//
//--------------------------------------------------------------------------------------
float3 GetRadiusWeights(uint vertexIndex) {
	uint localVertexIndex = vertexIndex % g_NumVerticesPerStrand;
	// Map (0, g_NumVerticesPerStrand - 1) to (1, 0):
	return 1 - (localVertexIndex + uint3(0, 1, 2)) * rcp(g_NumVerticesPerStrand - 1);
}

//--------------------------------------------------------------------------------------
//
//      SetupGlobals
//
//		Sets g_CurveCount, g_StartIndex.
//
//--------------------------------------------------------------------------------------
inline void SetupGlobals(uint linearTilePointer, uint GIndex)
{
	if (GIndex == 0) {
		g_StartIndex = g_PerBinCounter[linearTilePointer];
		g_CurveCount = max(g_PerBinCounter[linearTilePointer + 1], g_StartIndex) - g_StartIndex;
	}
	GroupMemoryBarrierWithGroupSync();
}

//--------------------------------------------------------------------------------------
//
//      LoadCurvePrimitive
//
//		Helper method to load the respective curve.
//
//--------------------------------------------------------------------------------------
inline CurvePrimitive LoadCurvePrimitive(uint curvePointer, float t0ViewPort, float t1ViewPort, float t0Screen, float t1Screen)
{
	// curvePointer points to the first control point
	CurvePrimitive curve;
	curve.p0 = g_HairVertexPositions[curvePointer];
	curve.p1 = g_HairVertexPositions[curvePointer + 1];
	curve.p2 = g_HairVertexPositions[curvePointer + 2];

	ToClipSpace(curve);
	float3 weights = GetRadiusWeights(curvePointer);
	ToScreenSpace(curve, t0ViewPort, t1ViewPort, weights);
	curve = GetSubCurve(curve, t0Screen, t1Screen);

	return curve;
}

//--------------------------------------------------------------------------------------
//
//      LoadCurvePrimitive
//
//		Helper method to load the respective curve.
//
//--------------------------------------------------------------------------------------
inline CurvePrimitive LoadCurvePrimitive(uint index)
{
	uint curvePointer, nearestZ;
	float t0ViewPort, t1ViewPort, t0Screen, t1Screen;
	Unpack(g_CurvePointers[index], curvePointer, nearestZ, t0ViewPort, t1ViewPort, t0Screen, t1Screen);
	return LoadCurvePrimitive(curvePointer, t0ViewPort, t1ViewPort, t0Screen, t1Screen);
}

//--------------------------------------------------------------------------------------
//
//      ComputeCoverage
//
//		Compute the pixel coverage by looking up the contents of LDS at index.
//		If the coverage value is zero, no fragment has to be created.
//		Because most of the necessary computations are equal, this method
//		also computes the fragment depth.
//		The return value is the interpolation parameter of the pixel center 
//		projected onto the connecting line of the curves first and last control point.
//
//--------------------------------------------------------------------------------------
inline void ComputeCoverage(CurvePrimitive curve, float2 normal, float2 pixelCenter, out float coverage, out float depth, out float t)
{
	float2 p0ToPixelCenter = pixelCenter - float2(curve.p0.xy);
	float2 tangentStart = float2(curve.p1.xy - curve.p0.xy);
	float2 tangentEnd = float2(curve.p1.xy - curve.p2.xy);

	coverage = 0;
	depth = -1;
    t = 0;

	// The curves are not properly cut if the current segment looks like in this case.
	//		_
	//     - -
	//    /   \
    //   /
    // This is caused by tangents pointing into the same directon. 
    if (dot(tangentStart, tangentEnd) > 0) {
		// So we do not want to create fragments in this case.
		return;
	}

	float coverageStart = dot(tangentStart, p0ToPixelCenter);
	float coverageEnd = dot(tangentEnd, pixelCenter - float2(curve.p2.xy));
	// Hard cut off for pixel centers that do not project onto the curve.
	// For the spline connections this part is essential.
	// However at the root and the tip we have aliasing.
	// Fortunately this is only visible when zooming in to the tip.
	if ((coverageStart < 0) || (coverageEnd <= 0)) {
		return;
	}

	// Determine interpolation parameter t.
	// It holds how far along the connecting line the projected pixel center resides.
	float2 v = float2(curve.p2.xy - curve.p0.xy);
	t = dot(pixelCenter - float2(curve.p0.xy), v.xy) / dot(v.xy, v.xy);
	t = saturate(t);

	// Use t to determine the inverse depth (1 / z).
	depth = curve.p0.z + t * (curve.p2.z - curve.p0.z);

	// In the same manner determine the screen space radius.
	float curveRadius = curve.p0.w + t * (curve.p2.w - curve.p0.w);
	curveRadius *= rcp(depth);

	// Compute the absolute distance value of the pixel center to the connecting line 
	// between the first and last control point of the curve.
	// Combining the distance and the curve radius yields
	// an approximation for the pixel overlap.
	// Here a single pixel is approximated by a circle with radius 0.5.
	float distance = abs(dot(p0ToPixelCenter, normal));

	if (curveRadius < g_MinHairThickness) {
        // Hair is thinner than we need it to fully cover a pixel.
        // Thus, further reduce coverage.
		coverage = 1 - saturate(distance - float(g_MinHairThickness) + 0.5);
		coverage *= float(g_RcpMinHairThickness * curveRadius);
	}
	else {
        // Hair can cover a whole pixel, 
        // but it might still only covers part of it due to being off center.
        // Reduce the coverage to achieve antialiased edges.
		coverage = 1 - saturate(distance - float(curveRadius) + 0.5);
	}
}

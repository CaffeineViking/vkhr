#include "Commons.hlsl"

#define NUM_NEAREST_FRAGMENTS	4
#define CR_SUBPIXEL_LOG2		4       // PixelSize / SubpixelSize.

struct Mask64 {
	uint lo;
	uint hi;
};

//--------------------------------------------------------------------------------------
//		Groupshared memory.
//--------------------------------------------------------------------------------------
groupshared CurvePrimitive					g_CurvePrimitives[64];

//--------------------------------------------------------------------------------------
//
//      Add64
//
//		64bit addition with overflow into the higher 32 bits.
//
//--------------------------------------------------------------------------------------
inline void Add64(inout uint alo, uint blo, inout uint ahi, uint bhi)
{
	uint t = alo;
	alo += blo;
	ahi += bhi;
    ahi += (alo < t) ? 1 : 0;
}

//--------------------------------------------------------------------------------------
//
//      GenerateMask
//
//		Generate the mask entry for a specific edge.
//		Modified "High-Performance Software Rasterization on GPUs"
//		https://mediatech.aalto.fi/~samuli/publications/laine2011hpg_paper.pdf
//
//--------------------------------------------------------------------------------------
Mask64 GenerateMask(int curr, int dx, int dy)
{
	curr += (dx - dy) * (7 << CR_SUBPIXEL_LOG2);
	int stepX = dy << (CR_SUBPIXEL_LOG2 + 1);
	int stepYorig = -dx - dy * 7;
	int stepY = stepYorig << (CR_SUBPIXEL_LOG2 + 1);

	uint hi = (curr >= 0) ? 1 : 0;
	uint frac = curr + curr;
	int i;
	for (i = 62; i >= 32; i--)
		Add64(frac, ((i & 7) == 7) ? stepY : stepX, hi, hi);

	uint lo = 0;
	for (i = 31; i >= 0; i--)
		Add64(frac, ((i & 7) == 7) ? stepY : stepX, lo, lo);

	lo ^= lo >> 1, hi ^= hi >> 1;
	lo ^= lo >> 2, hi ^= hi >> 2;
	lo ^= lo >> 4, hi ^= hi >> 4;
	lo ^= lo >> 8, hi ^= hi >> 8;
	lo ^= lo >> 16, hi ^= hi >> 16;

	if (dy < 0)
	{
		lo ^= 0x55AA55AA;
		hi ^= 0x55AA55AA;
	}
	if (stepYorig < 0)
	{
		lo ^= 0xFF00FF00;
		hi ^= 0x00FF00FF;
	}
	if ((hi & 1) != 0)
		lo = ~lo;

	Mask64 mask;
	mask.lo = lo;
	mask.hi = hi;
	return mask;
}

Mask64 GetConservativeMask(float2 origin, float2 delta)
{
	// The algorithm for the mask generation relies on 
	// integer coordinates with a subpixel precision of 
	// CR_SUBPIXEL_LOG2 bits.
	// So we transform the input parameters accordingly.
	int ox = (int)(origin.x * (1 << CR_SUBPIXEL_LOG2));
	int oy = (int)(origin.y * (1 << CR_SUBPIXEL_LOG2));
	int dx = (int)(delta.x * (1 << CR_SUBPIXEL_LOG2));
	int dy = (int)(delta.y * (1 << CR_SUBPIXEL_LOG2));

	int curr = ox * dy - oy * dx;
	if (dy > 0 || (dy == 0 && dx <= 0)) curr--; // exclusive
	curr += (abs(dx) + abs(dy)) << (CR_SUBPIXEL_LOG2 - 1); // Conservative part
	return GenerateMask(curr, dx, dy);
}

//--------------------------------------------------------------------------------------
//
//      GeneratePixelCoverageMasks
//
//--------------------------------------------------------------------------------------
void SetupBlockLDS(uint2 tileCorner, uint curveIndex, uint GIndex)
{	
	CurvePrimitive curve = LoadCurvePrimitive(curveIndex);
	curve.p0.xy -= tileCorner;
	curve.p1.xy -= tileCorner;
	curve.p2.xy -= tileCorner;
	g_CurvePrimitives[GIndex] = curve;

	// Generate the mask for each edge
	//					Mask Left
	//				x -------------> x
	// Mask Start	|                | Mask End
	//              x <------------- x
	//                 Mask Right

	// Expand the connecting line to generate four edges.
	//float2 normal = normalize(float2(curve.p2.y - curve.p0.y, curve.p0.x - curve.p2.x));
	//float radiusStart = 0.5 + float(max(curve.p0.w * rcp(curve.p0.z), g_MinHairThickness));
	//float radiusEnd = 0.5 + float(max(curve.p2.w * rcp(curve.p2.z), g_MinHairThickness));

	//float2 normalStart = normal * radiusStart;
	//float2 normalEnd = normal * radiusEnd;

	//float2 originEdgeLeft = float2(curve.p0.xy + normalStart);
	//float2 deltaEdgeLeft = float2(curve.p2.xy + normalEnd - originEdgeLeft);
	//Mask64 maskLeft = GetConservativeMask(originEdgeLeft, deltaEdgeLeft);

	//float2 originEdgeRight = float2(curve.p2.xy - normalEnd);
	//float2 deltaEdgeRight = float2(curve.p0.xy - normalEnd - originEdgeRight);
	//Mask64 maskRight = GetConservativeMask(originEdgeRight, deltaEdgeRight);

	//// The edges at start and end depend on the tangents at p0 and p2.
	//// This is necessary to properly set the curve interconnections later on.
	//// Overwrite the normal vectors from before.
	//normalStart = radiusStart * normalize(float2(curve.p1.y - curve.p0.y, curve.p0.x - curve.p1.x));
	//normalEnd = radiusEnd * normalize(float2(curve.p2.y - curve.p1.y, curve.p1.x - curve.p2.x));

	//float2 originEdgeStart = float2(curve.p0.xy - normalStart);
	//Mask64 maskStart = GetConservativeMask(originEdgeStart, normalStart);

	//float2 originEdgeEnd = float2(curve.p2.xy + normalEnd);
	//Mask64 maskEnd = GetConservativeMask(originEdgeEnd, -normalEnd);

	//g_CoverageMasks[GIndex].lo = maskLeft.lo & maskRight.lo & maskStart.lo & maskEnd.lo;
	//g_CoverageMasks[GIndex].hi = maskLeft.hi & maskRight.hi & maskStart.hi & maskEnd.hi;
}

//--------------------------------------------------------------------------------------
//
//      GetPixelBitForThread
//
//		Compute for a particular thread the corresponding bit in the pixel
//		coverage mask.
//
//--------------------------------------------------------------------------------------
inline Mask64 GetPixelBitForThread(uint2 GTid) {
	uint bitShifts = GTid.y * 8 + GTid.x;
	Mask64 bit;
	bit.lo = (bitShifts < 32) ? (1 << bitShifts) : 0;
	bit.hi = (bitShifts > 31) ? (1 << (bitShifts - 32)) : 0;
	return bit;
}

//--------------------------------------------------------------------------------------
//
//      BlendNearest
//
//		Sort and alpha blend the nearest fragments, OIT for the rest.
//
//--------------------------------------------------------------------------------------
void BlendNearest(uint2 pixelCoordinate, float2 tileLocalPixelCenter, uint GIndex, uint DIndex)
{
	// Greater depth means nearer to the viewer
	float farthestDepth = rcp(g_DepthBuffer[pixelCoordinate]);
    float2 tileCorner = float2(pixelCoordinate & (~7));

    float depths[4];
    depths[0] = farthestDepth;
    depths[1] = farthestDepth;
    depths[2] = farthestDepth;
    depths[3] = farthestDepth;

	[unroll]
	for (uint idx = 0; idx < NUM_NEAREST_FRAGMENTS; ++idx) 
    {
		g_NearestColors[DIndex + idx] = float4(0, 0, 0, 0); // Don't want to keep them in VGPRs
	}

	// The accumulated background color
	float4 accumulatedBackgroundColor = float4(0, 0, 0, 0);
    
    // Tracks the accumulated translucency that can pass through the combined layers. 
    // The actual alpha value of the fragment is thus (1 - accumulatedTranslucency).
    float accumulatedTranslucency = 1;

    // The bit that indentifies the pixel to test
    GroupMemoryBarrierWithGroupSync();

	// Go through all blocks with 64 strands at once
	for (uint curveIndex = g_StartIndex; curveIndex < (g_StartIndex + g_CurveCount); curveIndex += 64) 
    {
        // Setup the next (up to) 64 curves in LDS
        SetupBlockLDS(tileCorner, curveIndex + GIndex, GIndex);
		GroupMemoryBarrierWithGroupSync();
		uint remainingElements = min(64, g_StartIndex + g_CurveCount - curveIndex);

        // Cycle through the block and implicitly test against the curve
        for (int ldsIndex = 0; ldsIndex < remainingElements; ++ldsIndex)
        {
            // Compute fragment coverage
            CurvePrimitive curve = g_CurvePrimitives[ldsIndex];
            float2 normal = normalize(float2(curve.p2.y - curve.p0.y, curve.p0.x - curve.p2.x));

            float coverage;
            float inverseDepth;
            float t;
            ComputeCoverage(curve, normal, tileLocalPixelCenter, coverage, inverseDepth, t);

            // If coverage and depth is sufficient, compute the color
            if ((coverage > 0.06) && (inverseDepth > farthestDepth)) 
            {
                // Find farthest of the nearest == the ones with the smallest inverse depth
                uint farthest = 0;
                farthest = (depths[1] < depths[farthest]) ? 1 : farthest;
                farthest = (depths[2] < depths[farthest]) ? 2 : farthest;
                farthest = (depths[3] < depths[farthest]) ? 3 : farthest;

                float3 fragmentPos = float3(tileLocalPixelCenter + tileCorner, inverseDepth);
                float3 fragmentTangent = ComputeTangent(curve, t);
                float4 color = coverage * float4(ComputeShading(fragmentPos, fragmentTangent), 1);
                
                // Check if the current fragment should be part of the nearest 4 fragments.
                // The coverage check makes sure that low opacity hair doesn't overrule 
                // prominent features, thus keeping hairs from loosing their detial / vanishing in the distance.
                if (inverseDepth > depths[farthest] && coverage > 0.2)
                {
                	// Swap it into the nearest fragments.
                	Swap(depths[farthest], inverseDepth);
                	Swap(g_NearestColors[DIndex + farthest], color);
                }

                // The leftover fragment is blended with the background.
                // This (wrongly) assumes that all background fragments are at the same depth and contribute to the final color
                // according to their individual alpha values
                accumulatedBackgroundColor += color; // color has premultiplied alpha

                // We still need to track the final alpha
                accumulatedTranslucency *= (1 - color.w);
            }
        }
	}

	// Final Blending
	GroupMemoryBarrierWithGroupSync();

    // First prepare the background:
    if (accumulatedBackgroundColor.w > 0)
    {
        accumulatedBackgroundColor /= accumulatedBackgroundColor.w; // Normalize
        accumulatedBackgroundColor *= (1 - accumulatedTranslucency);
    }

    // Blend all 4 nearest fragments over the background color. back to front.
    for (int i = 0; i < 4; ++i)
    {
        // Again, determine farthest fragment
        uint farthest = 0;
        farthest = (depths[1] < depths[farthest]) ? 1 : farthest;
        farthest = (depths[2] < depths[farthest]) ? 2 : farthest;
        farthest = (depths[3] < depths[farthest]) ? 3 : farthest;

        // Make sure it won't be used anymore
        depths[farthest] = FLOAT_MAX;

        // Do the blending
        float4 color = g_NearestColors[DIndex + farthest];
        accumulatedBackgroundColor = color + (1 - color.w) * accumulatedBackgroundColor;
    }

    g_BackBuffer[pixelCoordinate] = accumulatedBackgroundColor;
}

//--------------------------------------------------------------------------------------
//
//      DrawStrands
//
//		Draws the content of four 8x8 pixel tiles.
//		-> One thread per pixel.
//
//--------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void DrawStrands(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID, uint GIndex : SV_GroupIndex)
{
	// Lookup for which tile we are responsible.
	uint linearTilePointer = g_FilledTiles[Gid.x];
	SetupGlobals(linearTilePointer, GIndex);

	// Compute the pixel coordinates of this thread.
	uint2 pixelCoordinate = 8 * uint2(linearTilePointer % g_TileDim8.x, linearTilePointer / g_TileDim8.x) + GTid.xy;
	uint DIndex = NUM_NEAREST_FRAGMENTS * (linearTilePointer * 64 + GIndex);
	BlendNearest(pixelCoordinate, float2(GTid.xy + 0.5), GIndex, DIndex);
}

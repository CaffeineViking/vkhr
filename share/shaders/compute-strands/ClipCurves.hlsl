#include "Commons.hlsl"

//--------------------------------------------------------------------------------------
//
//      ClipBezierCurve
//
//		Clips a bezier curve to the view frustum defined in clip space. 
//		The intervals t0Out and t1Out may only be used if b0 and respectively b1 are true.
//
//--------------------------------------------------------------------------------------
void ClipBezierCurve(in CurvePrimitive segIn, out bool b0, out float2 t0Out, out bool b1, out float2 t1Out) {

	// Sensible default values
	t0Out = t1Out = float2(0, 1);

	b0 = false;
	b1 = false;

	// Boundary coordinates
	// Essentially holds the dot product of each point to each boundary plane equation. 
	// If the sign is negative, then this means, that the point is outside of the plane.
	float bc0[6] = { segIn.p0.x, segIn.p0.w - segIn.p0.x, segIn.p0.y, segIn.p0.w - segIn.p0.y, segIn.p0.z, segIn.p0.w - segIn.p0.z };
	float bc1[6] = { segIn.p1.x, segIn.p1.w - segIn.p1.x, segIn.p1.y, segIn.p1.w - segIn.p1.y, segIn.p1.z, segIn.p1.w - segIn.p1.z };
	float bc2[6] = { segIn.p2.x, segIn.p2.w - segIn.p2.x, segIn.p2.y, segIn.p2.w - segIn.p2.y, segIn.p2.z, segIn.p2.w - segIn.p2.z };

	int outcode0 = 0;
	int outcode1 = 0;
	int outcode2 = 0;

	[unroll]
	for (int bit = 0; bit < 6; ++bit) {
		outcode0 += (bc0[bit] < 0 ? 1 : 0) << bit; // -1 == outside of the frustum -> set bit
		outcode1 += (bc1[bit] < 0 ? 1 : 0) << bit;
		outcode2 += (bc2[bit] < 0 ? 1 : 0) << bit;
	}

	if ((outcode0 & outcode1 & outcode2) == 0) {
		if ((outcode0 | outcode1 | outcode2) == 0) {
			// Only reached if all boundary conditions are positive for every i.
			// This means that all control points are within
			// the view frustum and no clipping has to be done.
			b0 = true; // trivial accept of the whole curve
			return;
		}
		// No trivial accept or trivial reject was possible.
		// -> Actually clip the line
	}
	else {
		// Only reached if bc0[i], bc1[i] and bc2[i] are 1 for at least one i.
		// This means the whole segment is rejected, as start point, center point and
		// end point lie outside of the same boundary plane.
		return;
	}

	// TODO: Clipping at the view frustum planes (especially x & y) could make (small) problems for thick curves.
	// e.g. The center curve of a thick curve is culled although the offset curves would be visible.
	// This could be solved for example by expanding the viewport artificially (See clip space transformation matrix).

	// Actually clip the line
	float4 a = segIn.p0 - 2 * segIn.p1 + segIn.p2;
	float4 b = 2 * (segIn.p1 - segIn.p0);
	float4 c = segIn.p0;

	// Coefficients for "Mitternachtsformel"
	float3 coef[6] = { float3(a.x, b.x, c.x), float3(a.w - a.x, b.w - b.x, c.w - c.x),
		float3(a.y, b.y, c.y), float3(a.w - a.y, b.w - b.y, c.w - c.y),
		float3(a.z, b.z, c.z), float3(a.w - a.z, b.w - b.z, c.w - c.z) };


	int straddledBounds = outcode0 | outcode1 | outcode2;
	// For each plane
	for (int i = 0; i < 6; ++i)
	{
		int mask = 1 << i; // Shift mask to the bound.

		if ((straddledBounds & mask) == 0) continue; // Skip plane if all points are inside

		coef[i].x += coef[i].x == 0 ? 0.0001 : 0; // To avoid division by zero.

		float disc = coef[i].y * coef[i].y - 4 * coef[i].x * coef[i].z; //Discriminant

		if (disc < 0) {
			// Curve can be rejected, if p1 is inside and there are no intersections -> In this case the whole curve lies outside.
			// There are two cases where this is wrong (all out or all in), those have already been rejected or accepted before, though.
			if ((outcode1 & mask) == 0) {// If segIn.p1 is inside.
				b0 = false;
				b1 = false;
				return;
			}
			continue; // Continue with next plane, as there is no intersection 

		}

		float sqrtDisc = sqrt(disc);

		float t0 = (-coef[i].y + sqrtDisc) / (2 * coef[i].x);
		float t1 = (-coef[i].y - sqrtDisc) / (2 * coef[i].x);

		if (t0 > t1) { // Make sure t0 is smaller than t1
			Swap(t0, t1);
		}

		float2 t0Tmp = float2(0, 1);
		float2 t1Tmp = float2(0, 1);

		if (disc == 0) { // 1 Solution (t0 == t1) -> 1 Intersection with plane
			if ((outcode0 & mask) != 0) { // If segIn.p0 is outside
				t0Tmp = float2(t0, 1);
				t1Tmp = float2(t0, 1);
			}
			else {
				t0Tmp = float2(0, t0);
				t1Tmp = float2(0, t0);
			}
		}
		else { // disc > 0 -> 2 Solutions -> 2 Intersections with plane
			bool t0In = 0 <= t0 && t0 <= 1;
			bool t1In = 0 <= t1 && t1 <= 1;

			if (t0In && t1In) {
				if ((outcode1 & mask) == 0) { // p1 is inside
					t0Tmp = float2(t0, t1);
					t1Tmp = float2(t0, t1);
				}
				else {
					t0Tmp = float2(0, t0);
					t1Tmp = float2(t1, 1);
					b1 = true;
				}

			}
			else if (!t0In && t1In) {
				if ((outcode0 & mask) == 0) { // p0 is inside & p2 is outside
					t0Tmp = float2(0, t1);
					t1Tmp = float2(0, t1);
				}
				else {// p2 is inside & p0 is outside
					t0Tmp = float2(t1, 1);
					t1Tmp = float2(t1, 1);
				}
			}
			else if (t0In && !t1In) {
				if ((outcode0 & mask) == 0) { // p0 is inside & p2 is outside
					t0Tmp = float2(0, t0);
					t1Tmp = float2(0, t0);
				}
				else {// p2 is inside & p0 is outside
					t0Tmp = float2(t0, 1);
					t1Tmp = float2(t0, 1);
				}
			}
			else {
				if ((outcode0 & mask) == 0) { // p0 is inside & p2 is inside
					continue;// non trivial accept
				}
				else {
					b1 = false;
					return;// non trivial reject
				}
			}
		}

		// Truncate intervals
		t0Out = float2(max(t0Out.x, t0Tmp.x), min(t0Out.y, t0Tmp.y));
		t1Out = float2(max(t1Out.x, t1Tmp.x), min(t1Out.y, t1Tmp.y));
	}


	// Set the valid segments
	if (t0Out.x < t0Out.y) {
		b0 = true;
	}

	if (t1Out.x >= t1Out.y) {
		b1 = false; // Set b1 back to false, if the interval borders overlap.
	}
}

//--------------------------------------------------------------------------------------
//
//      AppendCurve
//
//		Appends the pointer to the first control point together with its 
//		clipping parameters in clip space at the end of g_TileCurvePointers.
//
//--------------------------------------------------------------------------------------
inline void AppendCurvePrimitive(uint indexOfFirstControlpoint, uint2 tile, float depth,
	float t0ViewPort, float t1ViewPort, float t0Screen, float t1Screen) {
	// Safe the clipped segment.
	uint linearTileIndex = g_TileDim8.x * tile.y + tile.x;
	TileCurvePointer tileCurvePointer = Pack(indexOfFirstControlpoint, 
		linearTileIndex, GetDepthAsUint(LinearizeDepth(depth)),
		t0ViewPort, t1ViewPort, 
		t0Screen, t1Screen);
	uint idx = g_TileCurvePointers.IncrementCounter();
	g_TileCurvePointers[idx] = tileCurvePointer;
	InterlockedAdd(g_PerBinCounter[linearTileIndex], 1);
}

//--------------------------------------------------------------------------------------
//
//      RestoreDepthRadius
//
//		Restore the depth and radius information of the z and w components.
//
//--------------------------------------------------------------------------------------
inline void RestoreDepthRadius(inout CurvePrimitive curve) {
	curve.p0.z = rcp(curve.p0.z);
	curve.p1.z = rcp(curve.p1.z);
	curve.p2.z = rcp(curve.p2.z);

	curve.p0.w *= curve.p0.z;
	curve.p1.w *= curve.p1.z;
	curve.p2.w *= curve.p2.z;
}

//--------------------------------------------------------------------------------------
//
//      ComputeAABB
//
//		Compute the minimum pixel as well as the maximum pixel
//		possibly overlapped by the curve.
//
//--------------------------------------------------------------------------------------
inline void ComputeAABB(CurvePrimitive curve, float maxRadius, out int2 minTile, out int2 maxTile) {
	float2 minPoint = min(curve.p0.xy, curve.p2.xy);
	float2 maxPoint = max(curve.p0.xy, curve.p2.xy);

	// Compute the screen space radius.
	minPoint -= maxRadius;
	maxPoint += maxRadius;

	minTile = (int2) floor(minPoint);
	maxTile = (int2) floor(maxPoint);

	minTile >>= 3; 
	maxTile >>= 3;

	minTile = max(minTile, 0);
	maxTile = min(maxTile, g_TileDim8 - 1);
}

//--------------------------------------------------------------------------------------
//
//      Overlaps
//
//		Check if the tile is overlapped by a straight line with a certain thickness.
//
//--------------------------------------------------------------------------------------
inline bool Overlaps(uint2 tile, float2 referencePoint, float2 normal, float thickness) {
	// Project every extended tile corner onto the line. 
	// If all corners are on the same side, the overlap test fails.
	// p1 --- p2
	// |      |
	// p4 --- p3
	float2 p1 = (8 * tile) - referencePoint - thickness;
	float offset = 8 + 2 * thickness;
	float2 p2 = float2(p1.x + offset, p1.y);
	float2 p3 = float2(p2.x, p2.y + offset);
	float2 p4 = float2(p1.x, p3.y);

	int sign = (dot(p1, normal) > 0) ? 1 : -1;
	sign += (dot(p2, normal) > 0) ? 1 : -1;
	sign += (dot(p3, normal) > 0) ? 1 : -1;
	sign += (dot(p4, normal) > 0) ? 1 : -1;

	// Not all left or all right
	return (sign != -4) && (sign != 4);
}


void BinCurve(CurvePrimitive curve, float t0ViewPort, float t1ViewPort, 
	float t0Screen, float t1Screen, uint firstVertexIndex) {
	// Now that we clipped the curve, we can safely compute 
	// the actual depth and radius values.
	RestoreDepthRadius(curve);
	float maxRadius = max(curve.p0.w, curve.p2.w);

	// For antialiasing we tune down the opacity instead of the radius.
	// We have to consider this minimum radius when creating the AABB.
	maxRadius = max(maxRadius, g_MinHairThickness); 

	int2 minTile, maxTile;
	ComputeAABB(curve, maxRadius, minTile, maxTile);

	float2 normal = float2(curve.p2.y - curve.p0.y, curve.p0.x - curve.p2.x);
	float nearestZ = min(curve.p0.z, min(curve.p1.z, curve.p2.z));

	for (int i = minTile.x; i <= maxTile.x; ++i) {
		for (int j = minTile.y; j <= maxTile.y; ++j) {
			uint2 tile = uint2(i, j);
			if (g_DepthBuffer8[tile] <= nearestZ) {
				// Depth Test failed
				continue;
			}
			
			// Tile overlap tests
			if (Overlaps(tile, curve.p0.xy, normal, maxRadius)) {
				AppendCurvePrimitive(firstVertexIndex, tile, nearestZ,
					t0ViewPort, t1ViewPort, t0Screen, t1Screen);
			}
			
		}
	}
}

//--------------------------------------------------------------------------------------
//
//      SubdivisionSteps
//
//		Determine the number of splits until we expect sufficient smoothness
//		taking into account that the sum of the distances from p0 to p1 
//		and p1 to p2 serves as upper bound.
//
//--------------------------------------------------------------------------------------
inline int SubdivisionSteps(CurvePrimitive curve) {
	float maxLength = length(curve.p1.xy - curve.p0.xy) + length(curve.p2.xy - curve.p1.xy);
	return min(5, (int)log2(maxLength * 0.25));
}

//--------------------------------------------------------------------------------------
//
//      ProcessCurve
//
//		Determine all potentially overlapping tiles.
//
//--------------------------------------------------------------------------------------
inline void ProcessCurve(CurvePrimitive curve, float t0ViewPort, float t1ViewPort, uint vertexIdx) {
	float3 radiusWeights = GetRadiusWeights(vertexIdx);
	ToScreenSpace(curve, t0ViewPort, t1ViewPort, radiusWeights);

	float4 a = curve.p0 - 2 * curve.p1 + curve.p2;
	float4 b = 2 * (curve.p1 - curve.p0);

	// Determine maximum number of subdivision steps
	int subdivisionSteps = SubdivisionSteps(curve);

	// Binary search with h for the first straight line approximation
	float t0Screen = 0;
	while (t0Screen < 1) {
		float h = 1 - t0Screen;

		// Maximum number of splits determined by initial curve length from t0Screen to t1Screen
		for (int i = 0; i < subdivisionSteps; ++i) {
			curve = GetSubCurve(curve.p0, t0Screen, h, a, b);
			if (IsFlat(curve)) {
				break;
			}
			// If the curve is not flat yet, 
			// we have to split it.
			h *= 0.5;
		}

		curve = GetSubCurve(curve.p0, t0Screen, h, a, b);
		BinCurve(curve, t0ViewPort, t1ViewPort, t0Screen, t0Screen + h, vertexIdx);

		// Proceed with the rest of the interval.
		curve.p0 = curve.p2;
		t0Screen += h;
	}
}

//--------------------------------------------------------------------------------------
//
//      ClipCurves
//
//		Clip each curve and store them in g_CurvePrimitives.
//
//--------------------------------------------------------------------------------------
[numthreads(256, 1, 1)]
void ClipCurves(uint3 DTid : SV_DispatchThreadID)
{
	uint numCurvesPerStrand = g_NumVerticesPerStrand / 2;
	if (DTid.x >= g_NumStrands * numCurvesPerStrand) return;

	// Index into the vertex positions.
	uint vertexIdx = DTid.x * 2 + DTid.x / numCurvesPerStrand;

	// Prepare the correct control point positions.
	CurvePrimitive curve;
	curve.p0 = g_HairVertexPositions[vertexIdx];
	curve.p1 = g_HairVertexPositions[vertexIdx + 1];
	curve.p2 = g_HairVertexPositions[vertexIdx + 2];

	ToClipSpace(curve);

	// Compute clipping	parameters.										  
	float2 firstInterval, secondInterval;
	bool drawFirst, drawSecond;
	ClipBezierCurve(curve, drawFirst, firstInterval, drawSecond, secondInterval);

	// Store each successful clipping result.
	if (drawFirst) {
		ProcessCurve(curve, firstInterval.x, firstInterval.y, vertexIdx);
	}

	if (drawSecond) { // Only executed rarely
		ProcessCurve(curve, secondInterval.x, secondInterval.y, vertexIdx);
	}
}

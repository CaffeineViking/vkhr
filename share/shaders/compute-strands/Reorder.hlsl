#include "Commons.hlsl"

//--------------------------------------------------------------------------------------
//
//      Reorder
//
//		Pass to reorder the explicitly stored TileCurvePointers into 
//		g_CurvePointers so that they are placed continuously in memory.
//
//--------------------------------------------------------------------------------------
[numthreads(256, 1, 1)]
void Reorder(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	// DTid.x == index into g_TileCurvePointers4
	if (DTid.x >= g_TileCurvePointersCount) return;

	TileCurvePointer tileCurvePointer = g_TileCurvePointers[DTid.x];

	// Compute correct offset to safely place segment.
	uint targetIndex;
	InterlockedAdd(g_PerBinCounter[tileCurvePointer.linearTilePointer], -1, targetIndex);
	
	// Place new element at the correct position.
	g_CurvePointers[targetIndex - 1] = tileCurvePointer.curvePointer;
}

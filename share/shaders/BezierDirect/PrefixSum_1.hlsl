#include "Commons.hlsl"
#include "ScanBlockBuffer.hlsl"

//--------------------------------------------------------------------------------------
//
//		PrefixSum_1
//
//		Does an inclusive prefix sum on g_PerBinCounter and safes the highest results in g_PrefixSumInc.
//		Additionally write out all tiles that have curves assigned to g_FilledTiles.
//
//--------------------------------------------------------------------------------------
[numthreads(NUM_THREADS_PER_GROUP, 1, 1)]
void PrefixSum_1(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	uint numCurves = g_PerBinCounter[DTid.x];
	g_BlockBuffer[GTid.x] = numCurves;

	// Although this is not part of the prefix sum, 
	// we can avoid another dispatch by doing additional work here.
	// Write out all tile ids of tiles that have at least one curve assigned
	// to the end of g_FilledTiles.
	if (numCurves > 0) {
		uint index = g_FilledTiles.IncrementCounter();
		g_FilledTiles[index] = DTid.x;
	}

	uint wavefrontIndex = GTid.x >> NUM_BITSHIFTS;
	uint localThreadIndex = GTid.x - (wavefrontIndex << NUM_BITSHIFTS);

	GroupMemoryBarrierWithGroupSync();
	ScanBlockBuffer(GTid.x, wavefrontIndex, localThreadIndex);

	GroupMemoryBarrierWithGroupSync();
	g_PerBinCounter[DTid.x] = g_BlockBuffer[GTid.x]; // Write back local prefix sums
	if (GTid.x == NUM_THREADS_PER_GROUP - 1) {
		g_PrefixSumInc[Gid.x] = g_BlockBuffer[GTid.x]; // Save highest value
	}
}

#include "Commons.hlsl"
#include "ScanBlockBuffer.hlsl"

//--------------------------------------------------------------------------------------
//
//		PrefixSum_2
//
//		Does an inclusive prefix sum on g_PrefixSumInc and safes thes results back into g_PerBinCounter.
//
//--------------------------------------------------------------------------------------
[numthreads(NUM_THREADS_PER_GROUP, 1, 1)]
void PrefixSum_2(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{
	g_BlockBuffer[GTid.x] = g_PrefixSumInc[GTid.x]; // This restricts the number of tiles to 1024 * 1024

	uint wavefrontIndex = GTid.x >> NUM_BITSHIFTS;
	uint localThreadIndex = GTid.x - (wavefrontIndex << NUM_BITSHIFTS);

	GroupMemoryBarrierWithGroupSync();
	ScanBlockBuffer(GTid.x, wavefrontIndex, localThreadIndex);

	GroupMemoryBarrierWithGroupSync();
	g_PerBinCounter[DTid.x] += Gid.x > 0 ? g_BlockBuffer[Gid.x - 1] : 0;
}

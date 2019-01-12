#include "Commons.hlsl"

groupshared float g_DepthValues[8][8];

//--------------------------------------------------------------------------------------
//
//      Reduce4
//
//		Reduces the depth values in g_DepthBuffer and stores the highest values
//		per 4x4 pixel tile in g_DepthBuffer4.
//
//--------------------------------------------------------------------------------------
[numthreads(8, 8, 1)]
void ReduceDepthBuffer(uint3 DTid : SV_DispatchThreadID, uint3 Gid : SV_GroupID, uint3 GTid : SV_GroupThreadID)
{
	g_DepthValues[GTid.x][GTid.y] = g_DepthBuffer[DTid.xy];

	g_DepthValues[GTid.x][GTid.y] = (GTid.x % 2 == 1) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x - 1][GTid.y]) : g_DepthValues[GTid.x][GTid.y];
	g_DepthValues[GTid.x][GTid.y] = (GTid.y % 2 == 1) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x][GTid.y - 1]) : g_DepthValues[GTid.x][GTid.y];

	g_DepthValues[GTid.x][GTid.y] = (GTid.x % 4 == 3) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x - 2][GTid.y]) : g_DepthValues[GTid.x][GTid.y];
	g_DepthValues[GTid.x][GTid.y] = (GTid.y % 4 == 3) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x][GTid.y - 2]) : g_DepthValues[GTid.x][GTid.y];

	g_DepthValues[GTid.x][GTid.y] = (GTid.x % 8 == 7) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x - 4][GTid.y]) : g_DepthValues[GTid.x][GTid.y];
	g_DepthValues[GTid.x][GTid.y] = (GTid.y % 8 == 7) ? max(g_DepthValues[GTid.x][GTid.y], g_DepthValues[GTid.x][GTid.y - 4]) : g_DepthValues[GTid.x][GTid.y];

	if (GTid.x == 7 && GTid.y == 7) {
		g_DepthBuffer8[Gid.xy] = g_DepthValues[GTid.x][GTid.y];
	}
}

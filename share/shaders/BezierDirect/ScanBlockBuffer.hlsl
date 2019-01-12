#define NUM_THREADS_PER_GROUP 1024
#define NUM_THREADS_PER_WAVEFRONT 64
#define NUM_BITSHIFTS 6
#define NUM_WAVEFRONTS (NUM_THREADS_PER_GROUP / NUM_THREADS_PER_WAVEFRONT)

//--------------------------------------------------------------------------------------
//		Groupshared memory.
//--------------------------------------------------------------------------------------
groupshared uint g_BlockBuffer[NUM_THREADS_PER_GROUP];

//--------------------------------------------------------------------------------------
//
//		ScanBlockBuffer
//
//		Does an inclusive prefix sum on g_BlockBuffer.
//
//--------------------------------------------------------------------------------------
inline void ScanBlockBuffer(uint index, uint wavefrontIndex, uint localThreadIndex) {
	// Step 1
	[unroll]
	for (uint stride = 1; stride < NUM_THREADS_PER_WAVEFRONT; stride <<= 1) {
		g_BlockBuffer[index] += localThreadIndex >= stride ? g_BlockBuffer[index - stride] : 0;
	}

	// Step 2 + 3
	GroupMemoryBarrierWithGroupSync();
	uint groupScanResult = 0;
	[unroll]
	for (uint i = 1; i < NUM_WAVEFRONTS; ++i) {
		groupScanResult += wavefrontIndex >= i ? g_BlockBuffer[i * NUM_THREADS_PER_WAVEFRONT - 1] : 0;
	}

	// Step 4
	GroupMemoryBarrierWithGroupSync();
	g_BlockBuffer[index] += groupScanResult;
}

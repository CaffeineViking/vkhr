//--------------------------------------------------------------------------------------
//
//		Random
//
//		XTEA implementation that returns two random numbers using the input as seed.
//
//--------------------------------------------------------------------------------------
inline uint2 Random(uint2 v) {
	uint4 k = { 0xbe168aa1, 0x16c498a3, 0x5e87b018, 0x56de7805 }; // Key
	uint sum = 0;
	uint delta = 0x9e3779b9;

	[unroll]
	for (uint i = 0; i < 2; ++i) {
		sum += delta;
		v.x += ((v.y << 4) + k.x) ^ (v.y + sum) ^ ((v.y >> 5) + k.y);
		v.y += ((v.x << 4) + k.z) ^ (v.x + sum) ^ ((v.x >> 5) + k.w);
	}

	return v;
}

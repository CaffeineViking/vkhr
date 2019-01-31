#ifndef VKHR_IMAGE_ATOMIC_AVERAGE_RGBA8_GLSL
#define VKHR_IMAGE_ATOMIC_AVERAGE_RGBA8_GLSL

// Based on the "Octree-Based Sparse Voxelization Using the GPU Hardware Rasterizer" by Crassin and Greene,
// this is the "modified" version (that works) from: https://rauwendaal.net/2013/02/07/glslrunningaverage/.
void imageAtomicAverageRGBA8(layout(r32ui) coherent volatile uimage3D voxels, ivec3 coord, vec3 nextVec3) {
    uint nextUint = packUnorm4x8(vec4(nextVec3, 1.0f / 255.0f));
    uint prevUint = 0;
    uint currUint;

    vec4 currVec4;

    vec3 average;
    uint count;

    // "Spin"-lock while the threads are trying to change the voxel (one at a time please)
    while((currUint = imageAtomicCompSwap(voxels, coord, prevUint, nextUint)) != prevUint)
    {
        prevUint = currUint;                 // store: packed rgb average and count
        currVec4 = unpackUnorm4x8(currUint); // unpack stored rgb average and count

        average =      currVec4.rgb;         // extract rgb average
        count   = uint(currVec4.a*255.0f);   // extract count

        // Compute the running average
        average = (average*count + nextVec3) / (count+1);

        // Pack new average and incremented count back into a int
        nextUint = packUnorm4x8(vec4(average, (count+1)/255.0f));
    }
}

#endif

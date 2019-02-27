#ifndef VKHR_SCHEME_GLSL
#define VKHR_SCHEME_GLSL

#include "../scene_graph/params.glsl"

// Simple LoD scheme that blends magnified and minified case using a Hermite function.
float lod(float magnified_distance, float minified_distance, float current_distance) {
    if      (renderer == RASTERIZER) return 0.0f;
    else if (renderer == RAYMARCHER) return 1.0f;
    else return smoothstep(magnified_distance, minified_distance, current_distance);
}

#endif

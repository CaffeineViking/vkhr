#ifndef VKHR_SCHEME_GLSL
#define VKHR_SCHEME_GLSL

#include "../scene_graph/params.glsl"

// Simple LoD scheme that blends magnified and minified case using a Hermite function.
float lod(float magnified_distance, float minified_distance, float current_distance) {
    if      (renderer == 0) return 0.0f; // i.e.: we're using the strand rasterizer,
    else if (renderer == 2) return 1.0f; // and here we're using the raymarcher one.
    else return smoothstep(magnified_distance, minified_distance, current_distance);
}

#endif

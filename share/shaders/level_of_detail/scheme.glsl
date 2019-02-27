#ifndef VKHR_SCHEME_GLSL
#define VKHR_SCHEME_GLSL

// Simple LoD scheme that blends magnified and minified case using a Hermite function.
float lod(float magnified_distance, float minified_distance, float current_distance) {
    return smoothstep(magnified_distance, minified_distance, current_distance);
}

#endif

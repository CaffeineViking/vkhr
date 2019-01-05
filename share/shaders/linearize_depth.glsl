#ifndef VKHR_LINEARIZE_DEPTH_GLSL
#define VKHR_LINEARIZE_DEPTH_GLSL

// Linearizes values in the shadow maps using near and far.
float linearize_depth(float depth, float near, float far) {
    return (2.0f * near) / (far+near - depth * (far-near));
}

#endif

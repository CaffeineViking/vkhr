#ifndef VKHR_RAYMARCH_GLSL
#define VKHR_RAYMARCH_GLSL

#include "sample_volume.glsl"

vec4 raymarch(sampler3D volume, vec3 start, vec3 end, vec3 origin, vec3 size, uint samples) {
    vec3 direction = end - start;
    vec4 accumulator = vec4(0.0);
    float steps = 1.0f / samples;

    for (float t = 0.0f; t < 1.0f; t += steps) {
        vec3 position = start + direction * t;
        accumulator += sample_volume(volume,
                                     position,
                                     origin,
                                     size);
    }

    return accumulator;
}

#endif

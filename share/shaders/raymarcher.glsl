#ifndef VKHR_RAYMARCH_GLSL
#define VKHR_RAYMARCH_GLSL

#include "volume.glsl"

vec4 raymarch(sampler3D volume, vec3 start, vec3 end, vec3 origin, vec3 size) {
    vec3 direction = end - start;
    vec4 accumulator = vec4(0.0);

    for (float t = 0.0f; t < 1.0f; t += 0.01f) {
        vec3 position = start + direction * t;
        accumulator += sample_volume(volume,
                                     position,
                                     origin,
                                     size);
    }

    return accumulator;
}

#endif

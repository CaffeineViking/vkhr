#ifndef VKHR_RAYMARCH_GLSL
#define VKHR_RAYMARCH_GLSL

#include "volume.glsl"

float raymarch(sampler3D volume, vec3 start, vec3 end, vec3 origin, vec3 size) {
    vec3 direction = end - start;
    float gathered_density = 0.0;

    for (float t = 0.0f; t < 1.0f; t += 0.1f) {
        vec3 position = start + direction * t;
        gathered_density += sample_volume(volume,
                                          position,
                                          origin,
                                          size);
    }

    return gathered_density;
}

#endif

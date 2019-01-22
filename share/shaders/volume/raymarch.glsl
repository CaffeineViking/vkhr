#ifndef VKHR_RAYMARCH_GLSL
#define VKHR_RAYMARCH_GLSL

#include "sample_volume.glsl"

// Simple raymarcher that samples the volume in equal-sized steps from the 'start' to the 'end' of the ray.
vec4 raymarch(sampler3D volume, vec3 start, vec3 end, vec3 volume_origin, vec3 volume_size, uint samples) {
    vec4 accumulator = vec4(0.0);
    float steps = 1.0f / samples;

    for (float t = 0.0f; t < 1.0f; t += steps) {
        vec3 point = mix(start, end, t);
        accumulator += sample_volume(volume, point,
                                     volume_origin,
                                     volume_size);
    }

    return accumulator;
}

#endif

#ifndef VKHR_FILTERED_RAYMARCH_GLSL
#define VKHR_FILTERED_RAYMARCH_GLSL

#include "filter_volume.glsl"

// Filtered raymarcher that samples the volume 'samples' times along the 'start' toward the 'end' of the ray with a `kernel` size.
vec4 filtered_raymarch(sampler3D volume, vec3 start, vec3 end, vec3 volume_origin, vec3 volume_size, uint samples, float kernel) {
    vec4 accumulator = vec4(0.0);
    float steps = 1.0f / samples;

    for (float t = 0.0f; t < 1.0f; t += steps) {
        vec3 point = mix(start, end, t);
        accumulator += filter_volume(volume, kernel,
                                     point,
                                     volume_origin,
                                     volume_size);
    }

    return accumulator;
}

#endif

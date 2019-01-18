#ifndef VKHR_AMBIENT_OCCLUSION_GLSL
#define VKHR_AMBIENT_OCCLUSION_GLSL

#include "raymarch.glsl"

// Raycast pseudo-randomly within the sphere.
float ambient_occlusion(sampler3D density,
                        vec3 position,
                        vec3 volume_origin,
                        vec3 volume_size,
                        uint rays,
                        uint samples_per_ray,
                        float radius,
                        float intensity) {
    float total_density = 0.0f;

    uint total_samples = rays * samples_per_ray;

    for (uint r = 0; r < rays; ++r) {
        vec3 direction = vec3(1.0, 1.0, 1.0); // TODO: sample pseudo-randomly sphere.
        total_density += raymarch(density,
                                  position, position + normalize(direction) * radius,
                                  volume_origin, volume_size,
                                  samples_per_ray).r;
    }

    return pow(1.0f - total_density / total_samples, intensity);
}

#endif

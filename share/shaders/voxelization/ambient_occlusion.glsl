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
                        float radius) {
    float total_density = 0.0f;

    uint total_samples = rays * samples_per_ray;

    for (uint r = 0; r < rays; ++r) {
        vec3 direction = vec3(1.0, 1.0, 1.0); // TODO: sample pseudo-randomly sphere.
        total_density += raymarch(density,
                                  position, position + normalize(direction) * radius,
                                  volume_origin, volume_size,
                                  samples_per_ray).r;
    }

    return 1.0f - total_density / total_samples;
}

// "Local Ambient Occlusion in Direct Volume Rendering" by Hernell et al (2010).
float local_ambient_occlusion(sampler3D density,
                              vec3 position,
                              vec3 origin,
                              vec3 size,
                              float samples,
                              float radius,
                              float intensity) {
    vec3 density_grid_size = textureSize(density, 0);
    vec3 texture_space = size / density_grid_size;
    float sampling_range = (samples - 1.00f) / 2.00f;

    float total_density = 0.0f;

    for (float y = -sampling_range; y <= +sampling_range; y += 1.0f)
    for (float x = -sampling_range; x <= +sampling_range; x += 1.0f)
    for (float z = -sampling_range; z <= +sampling_range; z += 1.0f) {
        vec3 sample_offset = position + vec3(x, y, z) * texture_space * radius;
        total_density += sample_volume(density, sample_offset, origin, size).r;
    }

    return pow(1.0f - total_density / pow(samples, 3), intensity);
}

#endif

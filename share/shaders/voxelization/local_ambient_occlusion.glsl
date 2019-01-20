#ifndef VKHR_LOCAL_AMBIENT_OCCLUSION_GLSL
#define VKHR_LOCAL_AMBIENT_OCCLUSION_GLSL

#include "sample_volume.glsl"

// "Local Ambient Occlusion in Direct Volume Rendering" by Hernell et al (2010).
float local_ambient_occlusion(sampler3D density,
                              vec3 position,
                              vec3 volume_origin,
                              vec3 volume_size,
                              float samples,
                              float radius,
                              float intensity) {
    vec3 density_grid_size = textureSize(density, 0);
    vec3 texture_space = volume_size / density_grid_size;
    float sampling_range = (samples - 1.00f) / 2.00f;
    float sampling_scale = (radius / sampling_range);

    float total_density = 0.0f;

    for (float z = -sampling_range; z <= +sampling_range; z += 1.0f)
    for (float y = -sampling_range; y <= +sampling_range; y += 1.0f)
    for (float x = -sampling_range; x <= +sampling_range; x += 1.0f) {
        vec3 sample_offset = position + vec3(x, y, z) * texture_space * sampling_scale;
        total_density += sample_volume(density, sample_offset, volume_origin,
                                                               volume_size).r;
    }

    return pow(1.0f - total_density / pow(samples, 3), intensity);
}

#endif

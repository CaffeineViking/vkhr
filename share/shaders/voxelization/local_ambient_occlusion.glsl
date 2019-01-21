#ifndef VKHR_LOCAL_AMBIENT_OCCLUSION_GLSL
#define VKHR_LOCAL_AMBIENT_OCCLUSION_GLSL

#include "sample_volume.glsl"

// Based on "Local Ambient Occlusion in Direct Volume Rendering" by Hernell et al. (2010)
float local_ambient_occlusion(sampler3D volume,
                              vec3 fragment_position,
                              vec3 volume_origin, vec3 volume_size,
                              float kernel_size,
                              float radius,
                              float intensity, float min_intensity) {
    float density = 0.0f;

    float kernel_radius = (kernel_size - 1.0f) / 2.0f;
    vec3 voxel_space = volume_size / textureSize(volume, 0);
    float voxel_scaling = radius / kernel_radius;
    vec3 voxel_sample_scaling = voxel_scaling * voxel_space;

    for (float z = -kernel_radius; z <= +kernel_radius; z += 1.0f)
    for (float y = -kernel_radius; y <= +kernel_radius; y += 1.0f)
    for (float x = -kernel_radius; x <= +kernel_radius; x += 1.0f) {
        vec3 sample_position = fragment_position + vec3(x, y, z) * voxel_sample_scaling;
        density += min(sample_volume(volume, sample_position, volume_origin, volume_size).r, min_intensity);
    }

    return pow(1.0f - density / pow(kernel_size, 3.0f), intensity);
}

#endif

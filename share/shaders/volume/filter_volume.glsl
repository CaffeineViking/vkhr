#ifndef VKHR_FILTER_VOLUME
#define VKHR_FILTER_VOLUME

#include "../utils/math.glsl"
#include "sample_volume.glsl"

// High-quality volume filter that takes the Gaussian of the local N*N*N neighborhood centered at 'fragment_position'.
vec4 filter_volume(sampler3D volume, float kernel_width, vec3 fragment_position, vec3 volume_origin, vec3 volume_size) {
    vec3 volume_resolution = textureSize(volume, 0);
    vec3 volume_space = (volume_size / volume_resolution);

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float sigma_stddev = (kernel_width / 2.0f) / 2.4f;
    float sigma_squared = sigma_stddev * sigma_stddev;

    vec4 density = vec4(0.0f);
    float total_weight = 0.0f;

    for (float z = -kernel_range; z <= +kernel_range; z += 1.0f)
    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float exponent = -1.0f * (x*x + y*y + z*z) / 2.0f*sigma_squared;
        float local_weight = 1.0f / (2.0f*M_PI*sigma_squared) * pow(M_E, exponent);
        density += sample_volume(volume,
                                 fragment_position + vec3(x, y, z) * volume_space,
                                 volume_origin, volume_size) * local_weight;
        total_weight += local_weight;
    }

    return density / total_weight;
}

#endif

#ifndef VKHR_AMBIENT_OCCLUSION_GLSL
#define VKHR_AMBIENT_OCCLUSION_GLSL

#include "raymarch.glsl"

float ambient_occlusion(sampler3D density,
                        vec3 position,
                        vec3 origin,
                        vec3 size,
                        float samples,
                        float radius) {
    vec3 density_resolution = textureSize(density,0);
    vec3 texture_space = (size / density_resolution);
    float sampling_range = (samples - 1.00f) / 2.00f;

    float total_density = 0.0f;

    for (float y = -sampling_range; y <= +sampling_range; y += 1.0f)
    for (float x = -sampling_range; x <= +sampling_range; x += 1.0f)
    for (float z = -sampling_range; z <= +sampling_range; z += 1.0f) {
        vec3 sample_offset = position + vec3(x, y, z) * texture_space * radius;
        total_density += sample_volume(density, sample_offset, origin, size).r;
    }

    return total_density;
}

#endif

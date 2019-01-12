#ifndef VKHR_SAMPLE_VOLUME_GLSL
#define VKHR_SAMPLE_VOLUME_GLSL

#include "../math.glsl"

vec4 sample_volume(sampler3D volume, vec3 position, vec3 origin, vec3 size) {
    return texture(volume, (position - origin) / size);
}

vec4 filter_volume(sampler3D volume, float kernel_width, vec3 position, vec3 origin, vec3 size, vec3 resolution) {
    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float sigma_stddev = (kernel_width / 2.0f) / 2.4f;
    float sigma_squared = sigma_stddev * sigma_stddev;

    vec4 filtered_density = vec4(0.0f);
    float total_weight = 0.0f;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f)
    for (float z = -kernel_range; z <= +kernel_range; z += 1.0f) {
        float weight_power = -1.0f * (x*x + y*y + z*z) / 2.0f*sigma_squared;
        float local_weight =  1.0f / (2.0f*M_PI*sigma_squared) * pow(M_E, weight_power);
        filtered_density += sample_volume(volume, position + vec3(x, y, z) * (size / resolution), origin, size) * local_weight;
        total_weight += local_weight;
    }

    return filtered_density / total_weight;
}

#endif

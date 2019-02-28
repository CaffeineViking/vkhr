#ifndef VKHR_FILTER_SHADOWS_GLSL
#define VKHR_FILTER_SHADOWS_GLSL

#define CSM 2

#include "tex2Dproj.glsl"
#include "linearize_depth.glsl"
#include "../scene_graph/params.glsl"
#include "../utils/rand.glsl"

// Standard shadow mapping with some filters.
float filter_shadows(sampler2D shadow_map, // the non-linearized shadow map.
                     vec4 light_space_frag, // fragment in light coordiante.
                     float kernel_width, // size of the uniform PCF kernels.
                     float shadow_map_bias) { // bias to remove shadow acne.
    float visibility = 1.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0) / 2;

    float light_depth = light_space_frag.z / light_space_frag.w;

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float kernel_weight = kernel_width * kernel_width;

    int s = 0; // For sampling from a Poisson Disk :-)

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        vec2 sample_position = vec2(x,  y);
        if (pcf_shadows_sampling_type == 1) {
            sample_position += poisson_disk[s];
        } sample_position /= shadow_map_size;

        float shadow_depth = tex2Dproj(shadow_map, light_space_frag, sample_position).r;
        if (shadow_depth > light_depth - shadow_map_bias)
            visibility -= 1.0f / kernel_weight;
        ++s; // Fetch the next sample position.
    }

    return 1.0f - visibility;
}

#endif

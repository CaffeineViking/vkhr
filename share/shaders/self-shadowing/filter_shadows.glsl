#ifndef VKHR_FILTER_SHADOWS_GLSL
#define VKHR_FILTER_SHADOWS_GLSL

#include "tex2Dproj.glsl"
#include "linearize_depth.glsl"

// Standard shadow mapping with some filters.
float filter_shadows(sampler2D shadow_map, // the non-linearized shadow map.
                     vec4 light_space_frag, // fragment in light coordiante.
                     float kernel_width, // size of the uniform PCF kernels.
                     float shadow_map_bias) { // bias to remove shadow acne.
    float visibility = 1.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0);

    float light_depth = light_space_frag.z / light_space_frag.w;

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float kernel_weight = kernel_width * kernel_width;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float shadow_depth = tex2Dproj(shadow_map, light_space_frag,
                                       vec2(x, y) / shadow_map_size).r;
        if (shadow_depth > light_depth - shadow_map_bias)
            visibility -= 1.0f / kernel_weight;
    }

    return 1.0f - visibility;
}

#endif

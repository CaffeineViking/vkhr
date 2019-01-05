#ifndef VKHR_SHADOW_MAP_GLSL
#define VKHR_SHADOW_MAP_GLSL

#include "light.glsl"

layout(binding = 5) uniform sampler2D shadow_maps[lights_size];

layout(binding = 4) uniform ShadowMap {
    int deep_shadows_kernel_size;
    int deep_shadows_sampling_type;
    int deep_shadows_stride_size;
    int deep_shadows_on;

    int shadow_map_kernel_size;
    int shadow_map_sampling_type;
    float shadow_map_bias;
    int shadow_map_on;
};

#endif

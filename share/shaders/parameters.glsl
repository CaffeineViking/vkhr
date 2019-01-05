#ifndef VKHR_PARAMETERS_GLSL
#define VKHR_PARAMETERS_GLSL

layout(binding = 4) uniform Parameters {
    int shading_model;

    int deep_shadows_kernel_size;
    int deep_shadows_sampling_type;
    int deep_shadows_stride_size;
    int deep_shadows_on;

    int shadow_map_kernel_size;
    int shadow_map_sampling_type;
    float shadow_map_bias;
    int shadow_map_on;

    int shadow_technique;
};

#endif

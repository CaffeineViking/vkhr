#ifndef VKHR_PARAMETERS_GLSL
#define VKHR_PARAMETERS_GLSL

layout(binding = 4) uniform Parameters {
    int shading_model;

    int deep_shadows_kernel_size;
    int deep_shadows_sampling_type;
    int deep_shadows_stride_size;
    int deep_shadows_on;

    int pcf_shadows_kernel_size;
    int pcf_shadows_sampling_type;
    float pcf_shadows_bias;
    int pcf_shadows_on;

    int shadow_technique;
};

#endif

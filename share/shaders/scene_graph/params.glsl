#ifndef VKHR_PARAMS_GLSL
#define VKHR_PARAMS_GLSL

#define YES  1
#define NO   0

layout(binding = 4) uniform Params {
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

    float isosurface;
    float raycast_steps;
    float occlusion_radius;
    float ao_exponent;
    float ao_max;

    float magnified_distance;
    int renderer;
    float minified_distance;
};

#endif

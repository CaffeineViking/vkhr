#ifndef VKHR_LIGHTS_GLSL
#define VKHR_LIGHTS_GLSL

struct Light {
    vec3 vector;
    float type;
    vec3 intensity;
    float cutoff;
    mat4 matrix;
    float near;
    float far;
};

layout(constant_id = 0) const uint lights_size = 1;

layout(binding = 1) uniform Lights {
    Light lights[lights_size];
};

#endif

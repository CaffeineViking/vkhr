#ifndef VKHR_CAMERA_GLSL
#define VKHR_CAMERA_GLSL

layout(binding = 0) uniform Camera {
    mat4 view;
    mat4 projection;
    vec3 position;
    float padding_0;
    float near, far;
    vec2 resolution;
} camera;

#endif

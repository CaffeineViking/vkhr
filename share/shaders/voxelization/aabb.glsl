#ifndef VKHR_AABB_GLSL
#define VKHR_AABB_GLSL

struct AABB {
    vec3 origin;
    float radius;
    vec3 size;
    float volume;
};

#endif

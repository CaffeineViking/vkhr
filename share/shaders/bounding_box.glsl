#ifndef VKHR_BOUNDING_BOX_GLSL
#define VKHR_BOUNDING_BOX_GLSL

struct AABB {
    vec3 origin;
    float radius;
    vec3 size;
    float volume;
};

#endif

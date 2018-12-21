#ifndef VKHR_VOLUME_GLSL
#define VKHR_VOLUME_GLSL

struct AABB {
    vec3 min;
    vec3 max;
};

struct Volume {
    vec3 resolution;
    AABB bounds;
};

#endif

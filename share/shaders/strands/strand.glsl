#ifndef VKHR_STRAND_GLSL
#define VKHR_STRAND_GLSL

#include "../volumes/bounding_box.glsl"

#define STRAND_SCALING (1.0 / 0.042)

layout(binding = 2) uniform Strand {
    AABB volume_bounds;
    vec3 volume_resolution;
    float strand_width;
    vec3 hair_color;
    float hair_alpha;
    float hair_exponent;
    float strand_ratio;
};

#endif

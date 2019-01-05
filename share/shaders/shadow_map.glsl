#ifndef VKHR_SHADOW_MAP_GLSL
#define VKHR_SHADOW_MAP_GLSL

#include "light.glsl"

#define VISUALIZE_SHADOW_MAP 2

layout(binding = 5) uniform sampler2D shadow_maps[lights_size];

#endif

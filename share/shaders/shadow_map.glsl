#ifndef VKHR_SHADOW_MAP_GLSL
#define VKHR_SHADOW_MAP_GLSL

#include "lights.glsl"

layout(binding = 5) uniform sampler2D shadow_maps[lights_size];

#endif

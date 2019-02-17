#ifndef VKHR_SHADOW_MAPS_GLSL
#define VKHR_SHADOW_MAPS_GLSL

#include "lights.glsl"

layout(binding = 7) uniform sampler2D shadow_maps[lights_size];

#endif

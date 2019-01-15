#ifndef VKHR_AMBIENT_OCCLUSION_GLSL
#define VKHR_AMBIENT_OCCLUSION_GLSL

#include "raymarch.glsl"

float ambient_occlusion(sampler3D density,
                        vec3 position,
                        vec3 origin,
                        vec3 size,
                        float radius,
                        uint samples) {
    return 0.0f;
}

#endif

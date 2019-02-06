#ifndef VKHR_NAIVE_AMBIENT_OCCLUSION_GLSL
#define VKHR_NAIVE_AMBIENT_OCCLUSION_GLSL

#include "raymarch.glsl"

#define AO 3

// Brute force AO calculation by sampling the six sides of a cube and its eight corners (limited to a certain maximum radius).
float ambient_occlusion(sampler3D density,
                        vec3 position,
                        vec3 volume_origin,
                        vec3 volume_size,
                        float radius,
                        uint samples) {
    float occlusion = 0.00f;

    occlusion += raymarch(density, position, position + vec3(radius, 0.0f,     0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(radius, 0.0f,     0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(0.0f,   radius,   0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(0.0f,   radius,   0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(radius, radius,   0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(radius, radius,   0.0f), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(0.0f,   0.0f,   radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(0.0f,   0.0f,   radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(radius, 0.0f,   radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(radius, 0.0f,   radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(0.0f,   radius, radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(0.0f,   radius, radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position + vec3(radius, radius, radius), volume_origin, volume_size, samples).r;
    occlusion += raymarch(density, position, position - vec3(radius, radius, radius), volume_origin, volume_size, samples).r;

    occlusion /= 14.0f * samples;

    return 1.0f - occlusion;
}

#endif

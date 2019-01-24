#ifndef VKHR_VOLUME_RENDERING_GLSL
#define VKHR_VOLUME_RENDERING_GLSL

#include "sample_volume.glsl"

// Find the normal of the surface at 'position' by taking the finite difference (with scaling) of a point.
vec3 volume_gradient(sampler3D volume, vec3 position, vec3 volume_origin, vec3 volume_size, float scale) {
    vec3 epsilon = volume_size / textureSize(volume, 0) * scale;
    float dx = sample_volume(volume, position + vec3(epsilon.x, 0, 0), volume_origin, volume_size).r - sample_volume(volume, position - vec3(epsilon.x, 0, 0), volume_origin, volume_size).r;
    float dy = sample_volume(volume, position + vec3(0, epsilon.y, 0), volume_origin, volume_size).r - sample_volume(volume, position - vec3(0, epsilon.y, 0), volume_origin, volume_size).r;
    float dz = sample_volume(volume, position + vec3(0, 0, epsilon.z), volume_origin, volume_size).r - sample_volume(volume, position - vec3(0, 0, epsilon.z), volume_origin, volume_size).r;
    return normalize(vec3(dx, dy, dz)); // gradient estimation, i.e. find isosurface normals.
}

// Finds the surface of a volume with at least 'surface_density' starting from 'volume_start' to 'volume_end' when it has been sampled 'step' times.
vec4 volume_surface(sampler3D volume, vec3 volume_start, vec3 volume_end, uint steps, float surface_density, vec3 volume_origin, vec3 volume_size) {
    float density = 0.0f; // current density values.
    float step_size = 1.0f / steps; // for raymarch.
    for (float t = 0.0f; t < 1.0f; t += step_size) {
        vec3 P = mix(volume_start, volume_end, t);
        density += sample_volume(volume, P,
                                 volume_origin,
                                 volume_size).r;
        if (density >= surface_density)
            return vec4(P, 1.0f);
    }

    return vec4(0.0f, 0.0f, 0.0f, 0.0f);
}

#endif

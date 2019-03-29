#ifndef VKHR_VOLUME_RENDERING_GLSL
#define VKHR_VOLUME_RENDERING_GLSL

#include "sample_volume.glsl"

// Find the normal of the surface at 'position' by taking the finite difference of a point.
vec3 volume_normal(sampler3D volume, vec3 position, vec3 volume_origin, vec3 volume_size) {
    vec3 epsilon = volume_size / textureSize(volume, 0);
    float dx = sample_volume(volume, position + vec3(epsilon.x, 0, 0), volume_origin, volume_size).r -
               sample_volume(volume, position - vec3(epsilon.x, 0, 0), volume_origin, volume_size).r;
    float dy = sample_volume(volume, position + vec3(0, epsilon.y, 0), volume_origin, volume_size).r -
               sample_volume(volume, position - vec3(0, epsilon.y, 0), volume_origin, volume_size).r;
    float dz = sample_volume(volume, position + vec3(0, 0, epsilon.z), volume_origin, volume_size).r -
               sample_volume(volume, position - vec3(0, 0, epsilon.z), volume_origin, volume_size).r;
    return -normalize(vec3(dx, dy, dz)); // the normals!
}

// Finds the isosurface of a volume with at least 'surface_density' starting from 'volume_start' to 'volume_end' when it has been sampled 'step' times.
vec4 volume_surface(sampler3D volume, vec3 volume_start, vec3 volume_end, float steps, float surface_density, vec3 volume_origin, vec3 volume_size, float depth_buffer) {
    float accumulated_density = 0.0f;
    float step_size = (1.0f / steps);

    vec3 surface_point = vec3(0.0f);
    bool surface_point_found = false;
    bool entry_point_found   = false;
    vec3 entry_point   = vec3(0.0f);

    for (float t = 0.0f; t < 1.0f; t += step_size) {
        vec3 P = mix(volume_start, volume_end, t);
        vec4 projection = camera.projection * camera.view * vec4(P, 1.0f);
        float depth = projection.z / projection.w;

        if (depth_buffer < depth)
            break;

        float density = sample_volume(volume, P, volume_origin, volume_size).r;
        accumulated_density += density; // total amount of screen-space density

        if (density != 0.0f) {
            if (accumulated_density <= surface_density) surface_point = P;
            if (accumulated_density >= surface_density) surface_point_found = true;
            if (!entry_point_found) {
                entry_point = P;
                entry_point_found = true;
            }
        }
    }

    if (!surface_point_found && entry_point_found)
        surface_point = entry_point;
    return vec4(surface_point, accumulated_density / surface_density);
}

#endif

#version 460 core

#include "../scene_graph/camera.glsl"
#include "../scene_graph/lights.glsl"
#include "../volume/volume_rendering.glsl"
#include "../volume/local_ambient_occlusion.glsl"
#include "../volume/raymarch.glsl"

#include "volume.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
} fs_in;

layout(binding = 3) uniform sampler3D density_volume;

layout(location = 0) out vec4 color;

void main() {
    vec3  raycast_start  = fs_in.position.xyz;
    float raycast_length = volume_bounds.radius;
    vec3  raycast_end    = raycast_start + normalize(raycast_start - camera.position) * raycast_length;

    vec4 surface_position = volume_surface(density_volume,
                                           raycast_start, raycast_end,
                                           255, 0.01f,
                                           volume_bounds.origin, volume_bounds.size);

    if (surface_position.a == 0.0f)
        discard;

    vec3 surface_normal = volume_gradient(density_volume,
                                          surface_position.xyz,
                                          volume_bounds.origin,
                                          volume_bounds.size);

    vec3 light_direction = normalize(lights[0].vector - surface_position.xyz);

    vec3 shading = hair_color * dot(surface_normal, light_direction);

    float occlusion = local_ambient_occlusion(density_volume,
                                              surface_position.xyz,
                                              volume_bounds.origin,
                                              volume_bounds.size,
                                              2, 2.50f, 16, 0.1f);

    color = vec4(shading * occlusion, 1.0f);
}

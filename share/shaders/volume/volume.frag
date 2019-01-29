#version 460 core

#include "../scene_graph/camera.glsl"
#include "../scene_graph/lights.glsl"
#include "../volume/volume_rendering.glsl"
#include "../volume/local_ambient_occlusion.glsl"
#include "../self-shadowing/approximate_deep_shadows.glsl"
#include "../shading_models/kajiya-kay.glsl"
#include "../volume/raymarch.glsl"

#include "../scene_graph/params.glsl"

#include "volume.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
} fs_in;

layout(binding = 3) uniform sampler3D density_volume;

layout(input_attachment_index = 1, binding = 5) uniform subpassInput depth_buffer;

layout(location = 0) out vec4 color;

void main() {
    vec3  raycast_start  = fs_in.position.xyz;
    float raycast_length = volume_bounds.radius;
    vec3  raycast_end    = raycast_start + normalize(raycast_start - camera.position) * raycast_length;

    vec4 surface_position = volume_surface(density_volume,
                                           raycast_start, raycast_end,
                                           512, 0.003,
                                           volume_bounds.origin,
                                           volume_bounds.size);

    if (surface_position.a == 0.0f)
        discard;

    float depth_buffer = subpassLoad(depth_buffer).r;

    vec4 projected_surface = camera.projection * camera.view * vec4(surface_position.xyz, 1.0f);

    float surface_depth = projected_surface.z / projected_surface.w;

    if (depth_buffer < surface_depth)
        discard;

    vec3 surface_normal = volume_gradient(density_volume,
                                          surface_position.xyz,
                                          volume_bounds.origin,
                                          volume_bounds.size);

    vec3 shading = vec3(1.0);

    vec3 light_direction = normalize(lights[0].vector - surface_position.xyz);
    vec3 eye_direction   = normalize(camera.position  - surface_position.xyz);

    if (shading_model == KAJIYA_KAY) {
        shading = kajiya_kay(hair_color, lights[0].intensity, hair_shininess,
                             surface_normal, light_direction, eye_direction);
        shading = hair_color * max(dot(surface_normal, light_direction), 0.);
    }

    float occlusion = 1.000f;

    vec4 light_position = lights[0].matrix * vec4(0, 0, 0, 1);

    if (deep_shadows_on == YES && shading_model != LAO) {
        occlusion = volume_approximated_deep_shadows(density_volume,
                                                     surface_position.xyz, light_position.xyz,
                                                     128, 0.8f,
                                                     volume_bounds.origin, volume_bounds.size);
    }

    if (shading_model != ADSM) {
        occlusion *= local_ambient_occlusion(density_volume,
                                             surface_position.xyz,
                                             volume_bounds.origin,
                                             volume_bounds.size,
                                             2, 2.50f, 16, 0.1f);
    }

    color = vec4(shading * occlusion, 1.0f);
}

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

layout(binding = 3) uniform sampler3D strand_density;

layout(input_attachment_index = 1, binding = 5) uniform subpassInput depth_buffer;

layout(location = 0) out vec4 color;

void main() {
    vec3  raycast_start  = fs_in.position.xyz;
    float raycast_length = volume_bounds.radius;
    vec3  raycast_end    = raycast_start + normalize(raycast_start - camera.position) * raycast_length;

    vec4 surface_position = volume_surface(strand_density,
                                           raycast_start, raycast_end,
                                           raymarch_size, volume_isosurface,
                                           volume_bounds.origin,
                                           volume_bounds.size);

    if (surface_position.a == 0.0f)
        discard;

    float depth_buffer = subpassLoad(depth_buffer).r;

    vec4 projected_surface = camera.projection * camera.view * vec4(surface_position.xyz, 1.0f);

    float surface_depth = projected_surface.z / projected_surface.w;

    if (depth_buffer < surface_depth)
        discard;

    vec3 surface_normal = volume_normal(strand_density,
                                        surface_position.xyz,
                                        volume_bounds.origin,
                                        volume_bounds.size);

    vec3 shading = vec3(1.0);

    vec3 light_direction = normalize(lights[0].origin - surface_position.xyz);
    vec3 eye_direction   = normalize(surface_position.xyz  - camera.position);

    vec3 light_bulb_intensity = lights[0].intensity;

    vec3 hacked_tangent = normalize(cross(surface_normal, -light_direction));

    if (shading_model == KAJIYA_KAY) {
        shading = kajiya_kay(hair_color, light_bulb_intensity, hair_exponent,
                             hacked_tangent, light_direction, eye_direction);
    }

    float occlusion = 1.000f;

    if (deep_shadows_on == YES && shading_model != LAO) {
        occlusion *= volume_approximated_deep_shadows(strand_density,
                                                      surface_position.xyz,
                                                      lights[0].origin,
                                                      raymarch_size, hair_opacity,
                                                      volume_bounds.origin,
                                                      volume_bounds.size);
    }

    if (shading_model != ADSM) {
        occlusion *= local_ambient_occlusion(strand_density,
                                             surface_position.xyz,
                                             volume_bounds.origin,
                                             volume_bounds.size,
                                             2, 2.50f, 16, 0.1f);
    }

    color = vec4(shading, 1.0f);
}

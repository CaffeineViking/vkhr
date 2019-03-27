#version 460 core

#include "../scene_graph/camera.glsl"
#include "../scene_graph/lights.glsl"
#include "../self-shadowing/approximate_deep_shadows.glsl"
#include "../shading/kajiya-kay.glsl"

#include "../transparency/ppll.glsl"
#include "../level_of_detail/scheme.glsl"

#include "raymarch.glsl"
#include "sample_volume.glsl"
#include "local_ambient_occlusion.glsl"
#include "volume_rendering.glsl"

#include "../scene_graph/params.glsl"

#include "volume.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
} fs_in;

layout(binding = 3)  uniform sampler3D strand_density;
layout(binding = 10) uniform sampler3D strand_tangent;

layout(input_attachment_index = 1, binding = 9) uniform subpassInput depth_buffer;

layout(location = 0) out vec4 color;

void main() {
    vec3  raycast_start  = fs_in.position.xyz;
    float raycast_length = volume_bounds.radius;
    vec3  raycast_end    = raycast_start + normalize(raycast_start - camera.position) * raycast_length;

    float depth_buffer = subpassLoad(depth_buffer).r;

    vec4 surface_position = volume_surface(strand_density,
                                           raycast_start, raycast_end,
                                           raycast_steps, isosurface,
                                           volume_bounds.origin,
                                           volume_bounds.size,
                                           depth_buffer);

    if (surface_position.a == 0.0f)
        discard;

    float coverage = lod(magnified_distance, minified_distance, camera.look_at_distance) * surface_position.a * (1.0f - hair_alpha);

    vec3 shading = vec3(1.0);

    vec3 light_direction = normalize(lights[0].origin - surface_position.xyz);
    vec3 eye_direction   = normalize(surface_position.xyz  - camera.position);

    vec3 light_bulb_intensity = lights[0].intensity;

    vec3 surface_tangent = sample_volume(strand_tangent,
                                         surface_position.xyz,
                                         volume_bounds.origin,
                                         volume_bounds.size).xyz;

    surface_tangent = normalize(surface_tangent);

    if (shading_model == KAJIYA_KAY) {
        shading = kajiya_kay(hair_color, light_bulb_intensity, hair_exponent,
                             surface_tangent, light_direction, eye_direction);
    }

    float occlusion = 1.000f;

    if (deep_shadows_on == YES && shading_model != LAO) {
        occlusion *= volume_approximated_deep_shadows(strand_density,
                                                      surface_position.xyz,
                                                      lights[0].origin,
                                                      raycast_steps, hair_alpha,
                                                      volume_bounds.origin,
                                                      volume_bounds.size,
                                                      6.0f);
    }

    if (shading_model != ADSM) {
        occlusion *= local_ambient_occlusion(strand_density,
                                             surface_position.xyz,
                                             volume_bounds.origin,
                                             volume_bounds.size,
                                             2, occlusion_radius,
                                             ao_exponent, ao_max);
    }

    vec4 surface = camera.projection * camera.view * vec4(surface_position.xyz, 1.0f);
    float depth  = surface.z / surface.w;

    color = vec4(shading * occlusion, coverage);

    ivec2 pixel = ivec2(gl_FragCoord.xy);

    uint node = ppll_next_node();
    if (node == PPLL_NULL_NODE) discard;
    ppll_node_data(node, color, depth);
    ppll_link_node(pixel, node);

    discard; // Fragments resolved in next pass.
}

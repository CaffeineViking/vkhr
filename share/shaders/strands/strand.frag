#version 460 core

#include "../scene_graph/camera.glsl"
#include "../shading/kajiya-kay.glsl"
#include "../self-shadowing/approximate_deep_shadows.glsl"
#include "../volumes/local_ambient_occlusion.glsl"

#include "../transparency/ppll.glsl"
#include "../level_of_detail/scheme.glsl"
#include "../anti-aliasing/gpaa.glsl"

#include "../scene_graph/lights.glsl"
#include "../scene_graph/shadow_maps.glsl"
#include "../scene_graph/params.glsl"

#include "strand.glsl"

layout(early_fragment_tests) in;

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
} fs_in;

layout(push_constant) uniform Object {
    mat4 model;
} object;

layout(binding = 3) uniform sampler3D strand_density;

layout(location = 0) out vec4 color;

void main() {
    float coverage = gpaa(gl_FragCoord.xy, fs_in.position,
                          camera.projection * camera.view,
                          camera.resolution, strand_width);

    coverage *= hair_alpha; // Alpha used for transparency.
    if (coverage < 0.001) discard; // Shading not worth it!

    vec3 eye_normal = normalize(fs_in.position.xyz - camera.position);
    vec3 light_direction = normalize(lights[0].origin - fs_in.position.xyz);
    vec3 light_bulb_color = lights[0].intensity; // add attenutations?

    vec3 shading = vec3(1.0);

    if (shading_model == KAJIYA_KAY) {
        shading = kajiya_kay(hair_color, light_bulb_color, hair_exponent,
                             fs_in.tangent, light_direction, eye_normal);
    }

    vec4 shadow_space_fragment = lights[0].matrix * fs_in.position;

    float occlusion = 1.000f;

    if (deep_shadows_on == YES && shading_model != LAO) {
        occlusion *= approximate_deep_shadows(shadow_maps[0],
                                              shadow_space_fragment,
                                              deep_shadows_kernel_size,
                                              deep_shadows_stride_size,
                                              30000.0f, 0.3);
    }

    if (shading_model != ADSM) {
        occlusion *= local_ambient_occlusion(strand_density,
                                             fs_in.position.xyz,
                                             volume_bounds.origin,
                                             volume_bounds.size,
                                             2, occlusion_radius,
                                             ao_exponent, ao_max);
    }

    color = vec4(shading * occlusion, coverage);

    ivec2 pixel = ivec2(gl_FragCoord.xy);

    uint node = ppll_next_node();
    ppll_node_data(node, color, gl_FragCoord.z);
    ppll_link_node(pixel, node);

    discard; // Fragments resolved in next pass.
}

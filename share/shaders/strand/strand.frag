#version 460 core

#include "../scene_graph/camera.glsl"
#include "../shading_models/kajiya-kay.glsl"
#include "../self-shadowing/approximate_deep_shadows.glsl"
#include "../voxelization/ambient_occlusion.glsl"
#include "../scene_graph/shadow_maps.glsl"
#include "../scene_graph/lights.glsl"

#include "../scene_graph/params.glsl"

#include "strand.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec4 origin;
    vec3 tangent;
} fs_in;

layout(binding = 3) uniform sampler3D density_volume;

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 camera_space_light  = (camera.view     * light_position).xyz;
    vec4 shadow_space_strand = lights[0].matrix * fs_in.position;

    vec3 shading = vec3(1.0);

    if (shading_model == 0) {
        shading = kajiya_kay(hair_color, light_color, hair_shininess,
                             fs_in.tangent, camera_space_light,
                             vec3(0, 0, -1));
    }

    float visibility = 1.00f;

    if (deep_shadows_on == 1 && shading_model != 3) {
        visibility = approximate_deep_shadows(shadow_maps[0],
                                              shadow_space_strand,
                                              deep_shadows_kernel_size,
                                              deep_shadows_stride_size,
                                              1136.0f, 0.8f);
    }

    if (shading_model != 2) {
        float density = ambient_occlusion(density_volume,
                                          fs_in.position.xyz,
                                          fs_in.origin.xyz,
                                          volume_bounds.size,
                                          2.0f, 2.50f);
        visibility *= pow(1.0f - density, 16.0f);
    }

    color = vec4(shading * visibility, 1.0f);
}

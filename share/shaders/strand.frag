#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "volume.glsl"
#include "shadow_map.glsl"
#include "approximate_deep_shadows.glsl"
#include "light.glsl"
#include "parameters.glsl"
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

    if (shading_model == KAJIYA_KAY) {
        shading = kajiya_kay(hair_color, light_color, hair_shininess,
                             fs_in.tangent, camera_space_light,
                             vec3(0, 0, -1));
    }

    float occlusion = 1.0f;

    if (deep_shadows_on == 1) {
        occlusion = approximate_deep_shadows(shadow_maps[0],
                                             shadow_space_strand,
                                             deep_shadows_kernel_size,
                                             deep_shadows_stride_size,
                                             1136.0f, strand_radius);
    }

    color = vec4(shading * occlusion, 1.0f);
}

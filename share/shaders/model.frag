#version 460 core

#include "camera.glsl"
#include "blinn-phong.glsl"
#include "filter_shadows.glsl"
#include "shadow_map.glsl"
#include "parameters.glsl"
#include "lights.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 normal;
    vec2 texcoord;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);

    vec3 camera_space_light = (camera.view     * light_position).xyz;
    vec4 light_space_vertex = lights[0].matrix * fs_in.position;

    vec3 shading = vec3(1.0f);

    if (shading_model == 0) {
        shading = vec3(dot(camera_space_light, fs_in.normal));
    }

    float occlusion = 1.0f;

    if (pcf_shadows_on == 1 && shading_model != 3) {
        occlusion = filter_shadows(shadow_maps[0],
                                   light_space_vertex,
                                   pcf_shadows_kernel_size,
                                   pcf_shadows_bias);
    }

    color = vec4(shading * occlusion, 1.0f);
}

#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "shadow_map.glsl"
#include "light.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.4f;
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 view_space_light = (camera.view     * light_position).xyz;
    vec4 light_space_frag = lights[0].matrix * fs_in.position;

    vec3 shading = kajiya_kay(hair_color, light_color, 50.00f,
                              fs_in.tangent, view_space_light,
                              vec3(0, 0, -1)); // camera space

    float visibility = 0.0f;

    visibility = approximate_deep_shadows(shadow_maps[0],
                                          shadows.kernel_size,
                                          shadows.stride_size,
                                          light_space_frag,
                                          1136.0f, 0.8f);

    color = vec4(shading * visibility, 1.0f);
}

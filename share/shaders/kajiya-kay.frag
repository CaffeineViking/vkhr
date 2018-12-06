#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "shadow_map.glsl"
#include "lights.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.40f;
    vec4 light_position = vec4(lights.data[0].vector, 0.0f);
    vec3 light_color = lights.data[0].intensity;

    vec3 view_space_light = (camera.view          * light_position).xyz;
    vec4 light_space_frag = lights.data[0].matrix * fs_in.position;

    vec3 shading = kajiya_kay(hair_color, light_color, 50.00f,
                              fs_in.tangent, view_space_light,
                              vec3(0, 0, -1)); // camera space

    float depth = tex2Dproj(shadow_maps[0], light_space_frag, vec2(0.0f)).r;

    float visibility = 1.0f;
    if (depth < light_space_frag.z / light_space_frag.w)
        visibility  = 0.0f;

    color = vec4(shading * visibility, 1.0f);
}

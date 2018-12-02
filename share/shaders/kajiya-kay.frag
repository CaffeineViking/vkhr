#version 460 core

#include "light-data.glsl"
#include "shadow-map.glsl"
#include "kajiya-kay.glsl"

layout(location = 0) in PipelineInput {
    vec3 tangent;
} fs_in;

layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

layout(binding = 1) uniform LightData {
    Light lights[16];
    int light_number;
} light_data;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.4;
    vec3 light_position = light_data.lights[0].vector;
    vec3 light_color = light_data.lights[0].intensity;

    vec3 shading = kajiya_kay(hair_color, light_color, 80.0,
                              fs_in.tangent, light_position,
                              vec3(0, 0, -1)); // view space

    color = vec4(shading, 1.0f);
}

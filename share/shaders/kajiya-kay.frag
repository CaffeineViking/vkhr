#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "shadow_map.glsl"
#include "lights.glsl"

layout(location = 0) in PipelineIn {
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.4;
    vec4 light_position = vec4(lights.data[0].vector, 0.0f);
    vec3 light_color = lights.data[0].intensity;

    vec4 light_view = camera.view * light_position;

    vec3 shading = kajiya_kay(hair_color, light_color, 80.0,
                              fs_in.tangent, light_view.xyz,
                              vec3(0, 0, -1)); // view space

    color = vec4(shading, 1.0f);
}

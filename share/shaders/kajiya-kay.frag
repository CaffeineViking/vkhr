#version 460 core

#include "kajiya-kay.glsl"

layout(location = 0) in PipelineData {
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.40f;
    vec3 light = normalize(vec3(1.0f, 2.0f, 1.0f));
    vec3 light_color = vec3(1.0f, 0.772f, 0.56f) * 0.2f;

    vec3 shading = kajiya_kay(hair_color, light_color, 80.00f,
                              normalize(fs_in.tangent), light,
                              vec3(0, 0, 0)); // in view space

    color = vec4(shading, 1.0f);
}

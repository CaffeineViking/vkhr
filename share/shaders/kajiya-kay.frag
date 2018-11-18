#version 460 core

#include "kajiya-kay.glsl"

layout(location = 0) in PipelineData {
    flat vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    float cut = length(fs_in.tangent);

    if (cut == 0.0f) discard;

    vec3 hair_color = vec3(0.8, 0.57, 0.32) * 0.40;
    vec3 light = normalize(vec3(1.0f, 2.0f, 1.0f));
    vec3 light_color = vec3(1, 0.772, 0.56) * 0.20;

    vec3 shading = kajiya_kay(hair_color, light_color, 80.00f,
                              normalize(fs_in.tangent), light,
                              vec3(0, 0, 0)); // in view space

    color = vec4(normalize(fs_in.tangent), 1.0f);
}

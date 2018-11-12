#version 460 core

#include "kajiya-kay.glsl"

layout(location = 0) in PipelineData {
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    float cut = length(fs_in.tangent);

    if (cut == 0.0f) discard;

    vec3 tangent = normalize(fs_in.tangent);
    vec3 hair_color = vec3(0.8, 0.57, 0.32);
    vec3 light = normalize(vec3(0, 1.0, 0));

    vec3 diffuse_color = kajiya_kay_diffuse(hair_color, tangent, light);

    color = vec4(diffuse_color, 1.0f);
}

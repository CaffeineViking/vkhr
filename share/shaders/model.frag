#version 460 core

#include "camera.glsl"
#include "blinn-phong.glsl"
#include "shadow_map.glsl"
#include "light.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 normal;
    vec2 texcoord;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(vec3(0.5f), 1.0f);
}

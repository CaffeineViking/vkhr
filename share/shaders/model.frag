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
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 view_space_light = (camera.view * light_position).xyz;
    color = vec4(vec3(dot(fs_in.normal, view_space_light)), 1);
}

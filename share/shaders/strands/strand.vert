#version 460 core

#include "../scene_graph/camera.glsl"

#include "strand.glsl"

layout(location = 0) in vec3  position;
layout(location = 1) in vec3  tangent;
layout(location = 2) in float thickness;

layout(push_constant) uniform Object {
    mat4 model;
} object;

layout(location = 0) out PipelineOut {
    vec4 position;
    vec3 tangent;
    float thickness;
} vs_out;

void main() {
    mat4 projection_view = camera.projection * camera.view;

    vec4 world_position = object.model * vec4(position, 1.0f);
    vec4 world_tangent  = object.model * vec4(tangent,  0.0f);

    vs_out.position  = world_position;
    vs_out.tangent   = world_tangent.xyz;
    vs_out.thickness = thickness;

    gl_Position = projection_view * world_position;
}

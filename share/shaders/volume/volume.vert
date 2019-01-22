#version 460 core

#include "../scene_graph/camera.glsl"

#include "volume.glsl"

layout(location = 0) in vec3 position;

layout(push_constant) uniform Object {
    mat4 model;
} object;

layout(location = 0) out PipelineOut {
    vec4 position;
} vs_out;

void main() {
    mat4 projection_view = camera.projection * camera.view;

    vec4 world_position = object.model * vec4(position, 1.0f);

    vs_out.position = world_position;

    gl_Position = projection_view * world_position;
}

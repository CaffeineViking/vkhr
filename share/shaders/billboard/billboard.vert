#version 460 core

#include "../scene_graph/camera.glsl"

#include "billboard.glsl"

layout(push_constant) uniform Object {
    mat4 model;
} object;

layout(location = 0) out PipelineOut {
    vec2 texcoord;
} vs_out;

void main() {
    mat4 projection_view = camera.projection * camera.view;

    vec2 texcoord = texcoords[gl_VertexIndex];
    vec3 position = vec3(positions[gl_VertexIndex], 0.0f);

    vec4 world_position = object.model * vec4(position, 1.0f);

    vs_out.texcoord = texcoord;

    gl_Position = projection_view * world_position;
}

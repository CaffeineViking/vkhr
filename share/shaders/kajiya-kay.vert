#version 460 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 tangent;

layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

layout(location = 0) out PipelineData {
    vec3 tangent;
} vs_out;

void main() {
    mat4 projection_view = transform.projection * transform.view;

    vec4 world_position = transform.model * vec4(position, 1.0f);
    vec4 world_tangent  = transform.model * vec4(tangent,  1.0f);

    vs_out.tangent = world_tangent.xyz;

    gl_Position =  projection_view * world_position;
}

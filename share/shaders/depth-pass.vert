#version 460 core

layout(location = 0) in vec3 position;

layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

void main() {
    mat4 projection_view = transform.projection * transform.view;
    vec4 world_position = transform.model * vec4(position, 1.0f);
    gl_Position = projection_view * world_position; // the depth.
}

#version 460 core

layout(push_constant) uniform Object {
    mat4 mvp;
} object;

layout(location = 0) in vec3 position;

void main() {
    vec4 world_position = object.mvp * vec4(position, 1.0f);
    gl_Position = world_position; // pre-multiplied on host.
}

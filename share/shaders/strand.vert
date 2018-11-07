#version 450 core

layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 tangents;

layout(location = 0) out vec3 tangent;

void main() {
    mat4 projection = transform.projection;
    mat4 model_view = transform.view * transform.model;
    gl_Position = projection * model_view * vec4(position, 1.0);
    tangent = (model_view * vec4(tangents, 0.0)).xyz;
}

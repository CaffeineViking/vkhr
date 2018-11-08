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
    gl_Position = transform.projection *
                  transform.view *
                  transform.model * vec4(position, 1.0);
    tangent = (transform.model * vec4(tangents, 0.0)).xyz;
}

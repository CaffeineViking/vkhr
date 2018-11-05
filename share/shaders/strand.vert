#version 450 core

out gl_PerVertex {
    vec4 gl_Position;
};

layout(location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position, 1.0);
}

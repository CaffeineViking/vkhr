#version 450 core

layout(location = 0) in vec3 tangent;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.8f, 0.57f, 0.32f);
    color = vec4(tangent, length(tangent)*1.0);
}

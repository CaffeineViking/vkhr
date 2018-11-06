#version 450 core

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.8f, 0.57f, 0.32f);
    color = vec4(hair_color * 0.8, 0.08);
}

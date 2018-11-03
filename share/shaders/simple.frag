#version 450 core

#include "simple.glsl"

layout(location = 0) out vec4 color;

void main() {
    color = simple_color();
}

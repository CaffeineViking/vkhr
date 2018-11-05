#version 450 core

#include "strand.glsl"

layout(location = 0) out vec4 color;

void main() {
    color = vec4(get_strand_color(), 0.4);
}

#version 450 core

#include "simple.glsl"

out gl_PerVertex {
    vec4 gl_Position;
};

vec2 positions[3] = vec2[](
    vec2(0.0,  0.5),
    vec2(0.5,  0.5),
    vec2(-0.5, 0.5)
);

layout(location = 0) out vec3 frag_color;

void main() {
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    frag_color = simple_color();
}

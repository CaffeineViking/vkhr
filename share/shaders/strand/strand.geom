#version 460 core

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
} gs_in[];

layout(location = 0) out PipelineOut {
    vec4 position;
    vec3 tangent;
} gs_out;

void main() {
    float r = 1.0f;

    gs_out.position = gs_in[0].position;
    gs_out.tangent  = gs_in[0].tangent;

    gl_Position = gl_in[0].gl_Position;

    EmitVertex();

    gs_out.position = gs_in[1].position;
    gs_out.tangent  = gs_in[1].tangent;

    gl_Position = gl_in[1].gl_Position;

    EmitVertex();

    EndPrimitive();
}

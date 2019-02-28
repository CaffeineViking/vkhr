#version 460 core

#include "../scene_graph/camera.glsl"

#include "strand.glsl"

layout(lines) in;
layout(line_strip, max_vertices = 2) out;

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
    float thickness;
} gs_in[];

layout(location = 0) out PipelineOut {
    vec4 position;
    vec3 tangent;
    float thickness;
} gs_out;

void main() {
    vec3 normal = normalize(camera.position - gs_in[0].position.xyz);
    vec3 tangent = gs_in[0].tangent;
    vec3 direction = normalize(cross(normal, tangent));

    gs_out.tangent   = gs_in[0].tangent;
    gs_out.position  = gs_in[0].position;
    gs_out.thickness = gs_in[0].thickness;
    gl_Position = gl_in[0].gl_Position;

    EmitVertex();

    gs_out.tangent   = gs_in[1].tangent;
    gs_out.position  = gs_in[1].position;
    gs_out.thickness = gs_in[1].thickness;
    gl_Position = gl_in[1].gl_Position;

    EmitVertex();

    EndPrimitive();
}

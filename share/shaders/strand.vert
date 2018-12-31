#version 460 core

#include "strand.glsl"
#include "camera.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 tangent;

layout(push_constant) uniform Object {
    mat4 model;
} object;

layout(location = 0) out PipelineOut {
    vec4 position;
    vec4 origin;
    vec3 tangent;
} vs_out;

void main() {
    mat4 projection_view = camera.projection * camera.view;

    vec4 world_position = object.model * vec4(position, 1.0f);
    vec4 origin = object.model * vec4(volume_bounds.origin,1);
    vec4 camera_tangent = camera.view  * object.model * vec4(tangent, 0.0f);

    vs_out.position = world_position;
    vs_out.origin   = origin;
    vs_out.tangent  = camera_tangent.xyz;

    gl_Position =  projection_view * world_position;
}

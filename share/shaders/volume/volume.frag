#version 460 core

#include "../scene_graph/camera.glsl"
#include "../shading_models/kajiya-kay.glsl"
#include "../volume/local_ambient_occlusion.glsl"
#include "../scene_graph/lights.glsl"

#include "volume.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
} fs_in;

layout(binding = 3) uniform sampler3D density_volume;

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 eye = vec3(0, 0, -1);

    vec3 shading = vec3(1.0f, 0.0f, 0.0f);

    float occlusion = 1.0f;

    color = vec4(shading*occlusion, 1.0f);
}

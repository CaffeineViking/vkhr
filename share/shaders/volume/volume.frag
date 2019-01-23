#version 460 core

#include "../scene_graph/camera.glsl"
#include "../scene_graph/lights.glsl"
#include "../volume/ambient_occlusion.glsl"
#include "../volume/raymarch.glsl"

#include "volume.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
} fs_in;

layout(binding = 3) uniform sampler3D density_volume;

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 eye = fs_in.position.xyz;

    vec3 direction = normalize(eye - camera.position);

    float t = volume_bounds.radius;

    vec3 shading = vec3(1.0f, 1.0f, 1.0f);

    float density = raymarch(density_volume,
                             eye, eye + direction * t,
                             volume_bounds.origin, volume_bounds.size,
                             256).r;

    if (density == 0.0f)
        discard;

    float occlusion = 1.0f;

    color = vec4(shading*density, 1.0);
}

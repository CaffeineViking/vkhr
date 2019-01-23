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

    vec3 shading = vec3(1.0f);

    // Find isosurface + shade

    float occlusion = 1.0000f;

    float volume_max = volume_bounds.radius;

    float density = raymarch(density_volume,
                             fs_in.position.xyz,
                             fs_in.position.xyz + normalize(fs_in.position.xyz - camera.position) * volume_max,
                             volume_bounds.origin, volume_bounds.size,
                             128).r / 5.0f;

    color = vec4(vec3(density), 1.0f);
}

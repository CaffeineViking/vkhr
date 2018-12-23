#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "volume.glsl"
#include "shadow_map.glsl"
#include "light.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec4 origin;
    vec3 tangent;
} fs_in;

layout(binding = 2) uniform Strand {
    AABB volume_bounds;
    vec3 volume_resolution;
    float strand_radius;
    vec3 hair_color;
    float hair_shininess;
};

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 camera_space_lights = (camera.view     * light_position).xyz;
    vec4 shadow_space_strand = lights[0].matrix * fs_in.position;

    vec3 shading = kajiya_kay(hair_color, light_color, hair_shininess,
                              fs_in.tangent, camera_space_lights,
                              vec3(0, 0, -1)); // we're in view space.

    float visibility = 0.0f;

    visibility = approximate_deep_shadows(shadow_maps[0],
                                          shadow_space_strand,
                                          deep_shadows_kernel_size,
                                          deep_shadows_stride_size,
                                          1136.0f, strand_radius);

    float density = sample_volume(density_texture, fs_in.position.xyz,
                                  volume_resolution, fs_in.origin.xyz,
                                  volume_bounds.size).r * 8;

    color = vec4(shading * (1 - density) * visibility, 1.0f);
}

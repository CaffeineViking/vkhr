#version 460 core

#include "camera.glsl"
#include "blinn-phong.glsl"
#include "shadow_map.glsl"
#include "light.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 normal;
    vec2 texcoord;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec4 light_position = vec4(lights[0].vector, 0.0f);

    vec3 camera_space_light  = (camera.view     * light_position).xyz;
    vec4 light_space_vertex  = lights[0].matrix * fs_in.position;

    float visibility = 1.0f;

    visibility = filtered_shadows(shadow_maps[0],
                                  light_space_vertex,
                                  shadows.kernel_size,
                                  0.0001f);

    float diffuse = dot(camera_space_light, fs_in.normal);

    color = vec4(vec3(diffuse * visibility), 1.0f);
}

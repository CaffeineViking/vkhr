#version 460 core

#include "../scene_graph/camera.glsl"
#include "../shading/lambertian.glsl"
#include "../self-shadowing/filter_shadows.glsl"

#include "../scene_graph/params.glsl"
#include "../scene_graph/shadow_maps.glsl"
#include "../scene_graph/lights.glsl"

#include "model.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 normal;
    vec2 texcoord;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 shading = vec3(1.0);

    vec3 light_normal = normalize(lights[0].origin - fs_in.position.xyz);

    vec3 white = vec3(1,1,1);

    if (shading_model == LAMBERTIAN) {
        shading = white * lambertian(fs_in.normal, light_normal);
    }

    float occlusion = 1.000f;

    vec4 shadow_space_fragment = lights[0].matrix * fs_in.position;

    if (pcf_shadows_on == YES && shading_model != 3) {
        occlusion *= filter_shadows(shadow_maps[0],
                                    shadow_space_fragment,
                                    pcf_shadows_kernel_size,
                                    pcf_shadows_bias);
    }

    float visibility = 1.0f;
    if (benchmarking == 1) {
        visibility   = 0.0f;
    }

    color = vec4(shading * occlusion, visibility);
}

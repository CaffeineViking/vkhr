#version 460 core

#include "camera.glsl"
#include "kajiya-kay.glsl"
#include "shadow_map.glsl"
#include "lights.glsl"

layout(location = 0) in PipelineIn {
    vec4 position;
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    vec3 hair_color = vec3(0.80f, 0.57f, 0.32f) * 0.4f;
    vec4 light_position = vec4(lights[0].vector, 0.0f);
    vec3 light_color    = lights[0].intensity;

    vec3 view_space_light = (camera.view     * light_position).xyz;
    vec4 light_space_frag = lights[0].matrix * fs_in.position;

    vec3 shading = kajiya_kay(hair_color, light_color, 50.00f,
                              fs_in.tangent, view_space_light,
                              vec3(0, 0, -1)); // camera space

    vec2 poisson_disk[4] = {
        vec2(-0.94201624,  -0.39906216 ),
        vec2(0.94558609,   -0.76890725 ),
        vec2(-0.094184101, -0.92938870 ),
        vec2(0.34495938,    0.29387760 )
    };

    float visibility = 1.0f;
    for (int i = 0; i < 4; ++i) {
        float depth = tex2Dproj(shadow_maps[0], light_space_frag,
                                poisson_disk[i] / 256.0f).r;
        if (depth < light_space_frag.z / light_space_frag.w)
            visibility -= 0.225f;
    }

    color = vec4(shading * visibility, 1.0f);
}

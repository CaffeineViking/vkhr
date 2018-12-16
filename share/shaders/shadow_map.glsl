#ifndef VKHR_SHADOW_MAPS_GLSL
#define VKHR_SHADOW_MAPS_GLSL

#include "light.glsl"
#include "math.glsl"

layout(binding = 3) uniform sampler2D shadow_maps[lights_size];

layout(binding = 2) uniform ShadowMap {
    int kernel_size;
    int type;
    int sampling_type;
    int stride_size;
    int enabled;
} shadows;

// Projects a 3-D position onto a 2-D plane for sampling a texture.
vec4 tex2Dproj(sampler2D image, vec4 position, vec2 displacement) {
    vec4 texel = vec4(1.0f);
    vec3 projected_position = position.xyz / position.w;
    if (projected_position.z > -1.0f && projected_position.z < 1.0f)
        texel = texture(image, projected_position.st + displacement);
    return texel;
}

// Linearizes values in the shadow maps using near and far.
float linearize_depth(float depth, float near, float far) {
    return (2.0f * near) / (far+near - depth * (far-near));
}

// Based on the "A Survivor Reborn: Tomb Raider on DX11" talk at GDC 2013 by Jason Lacroix and his pseudo-code.
float approximate_deep_shadow(float shadow_depth, float light_depth, float strand_radius, float strand_alpha) {
    float strand_depth = max(light_depth - shadow_depth, 0.0f); // depth of the current strand inside geometry.
    float strand_count = strand_depth * strand_radius; // expected number of hair strands occluding the strand.

    if (strand_depth > 1e-5) strand_count += 1; // assume we have passed some strand if the depth is above some
    // floating point error threshold (e.g from the given shadow map) and add it to the occluding strand count.

    // We also take into account the transparency of the hair strand to determine how much light might scatter.
    return pow(1.0f - strand_alpha, strand_count); // this gives us "stronger" shadows with deeper hair strand.
}

// Applies Gaussian PCF to the function above to create
// the final fragment visibility. It also features some
// "jitter" which creates high-quality "smooth" shadow.
float approximate_deep_shadows(sampler2D shadow_map, // the non-linearized shadow map itself of the hair style.
                               vec4 light_space_strand, // fragment in the shadow maps light coordinate system.
                               float kernel_width, // size of the PCF kernel, common values are 3x3 or 5x5 too.
                               float smoothing, // this jitter/stride parameter which creates smoother shadows.
                               float strand_radius, // the radius of the hair strands to calculate the density.
                               float strand_opacity) { // inv. proportional to amount of light passing through.
    float visibility = 0.0f;

    if (shadows.enabled == 0)
        return 1.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0);
    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float sigma_stddev = (kernel_width / 2.0f) / 2.4f;
    float sigma_squared = sigma_stddev * sigma_stddev;

    float light_depth = light_space_strand.z / light_space_strand.w;
    vec2 shadow_map_stride = shadow_map_size / smoothing; // stride.

    float total_weight = 0.0f;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float weight_power = -1.0f * (x*x + y*y) / 2.0f*sigma_squared;
        float local_weight =  1.0f / (2.0f*M_PI*sigma_squared) * pow(M_E, weight_power);

        float shadow_depth = tex2Dproj(shadow_map, light_space_strand, vec2(x, y) / shadow_map_stride).r;
        float shadow = approximate_deep_shadow(shadow_depth, light_depth, strand_radius, strand_opacity);

        visibility += shadow * local_weight;
        total_weight += local_weight;
    }

    return visibility / total_weight;
}

// Standard shadow mapping with some filtering.
float filtered_shadows(sampler2D shadow_map, // the non-linearized shadow map.
                       vec4 light_space_frag, // fragment in light coordiante.
                       float kernel_width, // size of the uniform PCF kernels.
                       float shadow_map_bias) { // bias to remove shadow acne.
    float visibility = 1.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0);

    float light_depth = light_space_frag.z / light_space_frag.w;

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float kernel_weight = kernel_width * kernel_width;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float shadow_depth = tex2Dproj(shadow_map, light_space_frag,
                                       vec2(x, y) / shadow_map_size).r;
        if (shadow_depth > light_depth - shadow_map_bias)
            visibility -= 1.0f / kernel_weight;
    }

    return 1.0f - visibility;
}

#endif

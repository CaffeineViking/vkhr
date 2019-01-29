#ifndef VKHR_APPROXIMATE_DEEP_SHADOWS_GLSL
#define VKHR_APPROXIMATE_DEEP_SHADOWS_GLSL

#include "../utils/math.glsl"
#include "../volume/sample_volume.glsl"
#include "linearize_depth.glsl"
#include "tex2Dproj.glsl"

// Based on the "A Survivor Reborn: Tomb Raider on DX11" talk at GDC 2013 by Jason Lacroix and his pseudo-code.
float approximate_deep_shadow(float shadow_depth, float light_depth, float strand_radius, float strand_alpha) {
    float strand_depth = max(light_depth - shadow_depth, 0.0f); // depth of the current strand inside geometry.
    float strand_count = strand_depth * strand_radius; // expected number of hair strands occluding the strand.

    if (strand_depth > 1e-5) strand_count += 1; // assume we have passed some strand if the depth is above some
    // floating point error threshold (e.g from the given shadow map) and add it to the occluding strand count.

    // We also take into account the transparency of the hair strand to determine how much light might scatter.
    return pow(1.0f - strand_alpha, strand_count); // this gives us "stronger" shadows with deeper hair strand.
}

// Instead of "guessing" the amount of strands in the way, we can find the amount from the strand voxelization.
float volume_approximated_deep_shadows(sampler3D volume, vec3 strand_position, vec3 light_position, uint steps,
                                       float strand_alpha, vec3 volume_origin, vec3 volume_size) { // Vol-ADSM.
    float strands = 0;
    float step_size = 1.0f / steps; // for raymarch.
    for (float t = 0.0f; t < 1.0f; t += step_size) {
        vec3 point = mix(strand_position, light_position, t);
        strands += sample_volume(volume, point,
                                 volume_origin,
                                 volume_size).r;
    }

    return pow(1.0f - strand_alpha, strands);
}

// Applies Gaussian PCF to the function above to create
// the final fragment visibility. It also features some
// "jitter" which create high-quality "smooth" shadows.
float approximate_deep_shadows(sampler2D shadow_map, // the non-linearized shadow map itself of the hair style.
                               vec4 light_space_strand, // fragment in the shadow maps light coordinate system.
                               float kernel_width, // size of the PCF kernel, common values are 3x3 or 5x5 too.
                               float smoothing, // this jitter/stride parameter which creates smoother shadows.
                               float strand_radius, // the radius of the hair strands to calculate the density.
                               float strand_opacity) { // inv. proportional to amount of light passing through.
    float visibility = 0.0f;

    vec2 shadow_map_size = textureSize(shadow_map, 0);

    float kernel_range = (kernel_width - 1.0f) / 2.0f;
    float sigma_stddev = (kernel_width / 2.0f) / 2.4f;
    float sigma_squared = sigma_stddev * sigma_stddev;

    float light_depth = light_space_strand.z / light_space_strand.w;
    vec2 shadow_map_stride = shadow_map_size / smoothing; // stride.

    float total_weight = 0.0f;

    for (float y = -kernel_range; y <= +kernel_range; y += 1.0f)
    for (float x = -kernel_range; x <= +kernel_range; x += 1.0f) {
        float exponent = -1.0f * (x*x + y*y) / 2.0f*sigma_squared; // Gaussian RBDF.
        float local_weight =  1.0f / (2.0f*M_PI*sigma_squared) * pow(M_E, exponent);

        float shadow_depth = tex2Dproj(shadow_map, light_space_strand, vec2(x, y) / shadow_map_stride).r;
        float shadow = approximate_deep_shadow(shadow_depth, light_depth, strand_radius, strand_opacity);

        visibility   += shadow * local_weight;
        total_weight += local_weight;
    }

    return visibility / total_weight;
}

#endif

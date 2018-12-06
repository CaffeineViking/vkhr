layout(binding = 2) uniform sampler2D shadow_maps[16];

// Based on the "A Survivor Reborn: Tomb Raider on DX11" talk at GDC 2013 by Jason Lacroix and his pseudo-code.
float approximate_deep_shadow(float shadow_depth, float light_depth, float strand_radius, float strand_alpha) {
    float strand_depth = max(light_depth - shadow_depth, 0.0f); // depth of the current strand inside geometry.
    float strand_count = strand_depth * strand_radius; // expected number of hair strands occluding the strand.

    if (strand_depth > 1e-5) strand_count += 1; // assume we have passed some strand if the depth is above some
    // floating point error threshold (e.g from the given shadow map) and add it to the occluding strand count.

    // We also take into account the transparency of the hair strand to determine how much light might scatter.
    return pow(1.0f - strand_alpha, strand_count); // this gives us "stronger" shadows with deeper hair strand.
}

vec4 tex2Dproj(sampler2D image, vec4 position, vec2 uv_offset) {
    vec4 texel = vec4(1.0f);
    vec3 projected_position = position.xyz / position.w;
    if (projected_position.z > -1.0 && projected_position.z < 1.0)
        texel = texture(image, projected_position.st + uv_offset);
    return texel;
}

float approximate_deep_shadow_4x4_pcf() {
    float shadow = 0.00f;

    for (float y = -1.5f; y <= +1.5f; y += 1.0f)
    for (float x = -1.5f; x <= +1.5f; x += 1.0f) {
    }

    return shadow / 16.0;
}

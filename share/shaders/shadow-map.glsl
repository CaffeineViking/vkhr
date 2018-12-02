// Based on the "A Survivor Reborn: Tomb Raider on DX11" talk at GDC 2013 by Jason Lacroix and his pseudo-code.
float approximate_deep_shadow(float shadow_depth, float light_depth, float strand_radius, float strand_alpha) {
    float strand_depth = max(light_depth - shadow_depth, 0.0f); // depth of the current strand inside geometry.
    float strand_count = strand_depth * strand_radius; // expected number of hair strands occluding the strand.

    if (strand_depth > 1e-5) strand_count += 1; // assume we have passed some strand if the depth is above some
    // floating point error threshold (e.g from the given shadow map) and add it to the occluding strand count.

    // We also take into account the transparency of the hair strand to determine how much light might scatter.
    return pow(1.0f - strand_alpha, strand_count); // this gives us "stronger" shadows with deeper hair strand.
}

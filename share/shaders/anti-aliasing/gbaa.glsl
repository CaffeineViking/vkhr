#ifndef VKHR_GBAA_GLSL
#define VKHR_GBAA_GLSL

// This coverage calculation is based on Emil Persson's:
// "Geometry Buffer Anti-Aliasing (GBAA)" AA techniques.
// But implementation is similar to the code in TressFX.

float gbaa(vec2 p, vec2 q, vec2 fragment, vec2 screen) {
    // p, q, and fragment should be in view coordinates.

    p        *= screen; // Scale positions so 1 is sized
    q        *= screen; // as a half-pixel size by using
    fragment *= screen; // the size of the resolution...

    float geom_width = length(p - q);
    float p_d = length(p - fragment);
    float q_d = length(q - fragment);

    bool outside = any(bvec2(step(geom_width, p_d), step(geom_width, q_d)));

    float direction = outside ? -1.0f : +1.0f;

    // Signed distance field (positive if inside hair, negative outside).
    float signed_distance = direction * clamp(min(p_d, q_d), 0.0f, 1.0f);

    // Returns the coverage by the distance
    //     - 0 if we're outside the strand,
    //     - 1, if we're inside the strand.
    return (signed_distance + 1.0f) + 0.5f;
}

#endif

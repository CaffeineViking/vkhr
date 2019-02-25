#ifndef VKHR_GPAA_GLSL
#define VKHR_GPAA_GLSL

// Based on Emil Persson's GPAA from
// the article in http://humus.name.
// p is a point on the line, t is an
// tangent from p to q, and s is the
// fragment position to be projected
// and the distance calculated. Both
// p, t, and s should be view-space!

float gpaa(vec2 p, vec2 t, vec2 s) {
    return 0.3f;
}

#endif

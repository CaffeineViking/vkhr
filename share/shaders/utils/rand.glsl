#ifndef VKHR_RAND_GLSL
#define VKHR_RAND_GLSL

// LCG values in Numerical Recipies book.
uint random_lcg(uint state) {
    state = 1664525 * state + 1013904223;
    return state;
}

// Well known one liner:
float rand(vec2 state) {
    return fract(sin(dot(state, vec2(12.9898, 78.233))) * 43758.5453);
}

// "Xorshift RNGs" from Marsaglia.
uint random_xorshift(uint state) {
    state ^= (state << 13);
    state ^= (state >> 17);
    state ^= (state <<  5);
    return state;
}

#endif

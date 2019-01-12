#ifndef VKHR_RANDOM_GLSL
#define VKHR_RANDOM_GLSL

layout(constant_id = 42) const uint random_seed = 42;

// LCG values taken from the Numerical Recipies book.
uint random_lcg() {
    random_seed = 1664525 * random_seed + 1013904223;
    return random_seed;
}

// "Xorshift RNGs" by George Marsaglia.
uint random_xorshift() {
    random_seed ^= (random_seed << 13);
    random_seed ^= (random_seed >> 17);
    random_seed ^= (random_seed <<  5);
    return random_seed;
}

#endif

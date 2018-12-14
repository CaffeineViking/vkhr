#ifndef VKHR_MATH_GLSL
#define VKHR_MATH_GLSL

#define M_PI 3.14159265358979323846
#define M_E  2.71828182845904523536

uint random_seed; // Use Wang hash.

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

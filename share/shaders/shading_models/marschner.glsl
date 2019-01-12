#ifndef VKHR_MARSCHNER_GLSL
#define VKHR_MARSCHNER_GLSL

// Based on the "Hair Rendering and Shading" ATI slides by Thorsten Scheuermann,
// from Marschner's observations with "Light Scattering from Human Hair Fibers".
vec3 marschner(vec3 diffuse, // specular was split into two parts:
               vec3 specular_reflection,   float reflection_power,
               vec3 specular_transmission, float transmission_power,
               vec3 tangent, vec3 light, vec3 eye) {
    float cosTL = dot(tangent, light);

    float cosTL_squared = cosTL*cosTL;

    float one_minus_cosTL_squared = 1.0f - cosTL_squared;

    float sinTL = sqrt(one_minus_cosTL_squared);

    vec3 diffuse_colors = diffuse * sinTL;

    return diffuse_colors;
}

#endif

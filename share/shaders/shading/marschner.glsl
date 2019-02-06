#ifndef VKHR_MARSCHNER_GLSL
#define VKHR_MARSCHNER_GLSL

#define MARSCHNER 0

// Based on the "Hair Rendering and Shading" ATI slides by Thorsten Scheuermann,
// from Marschner's observations with "Light Scattering from Human Hair Fibers".
vec3 marschner(vec3 diffuse, // specular was split into two parts:
               vec3 specular_reflection,   float reflection_power,
               vec3 specular_transmission, float transmission_power,
               vec3 tangent, vec3 light, vec3 eye) {
    return vec3(1.0f); // Need to find good shaders.
}

#endif

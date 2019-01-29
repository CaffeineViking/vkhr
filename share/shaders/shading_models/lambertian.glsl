#ifndef VKHR_LAMBERTIAN_GLSL
#define VKHR_LAMBERTIAN_GLSL

#define LAMBERTIAN 0

float lambertian(vec3 surface_normal, vec3 light_normal) {
    return max(dot(surface_normal, light_normal), 0.0f);
}

#endif

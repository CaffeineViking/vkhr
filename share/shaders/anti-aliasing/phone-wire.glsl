#ifndef VKHR_PHONE_WIRE_GLSL
#define VKHR_PHONE_WIRE_GLSL

float phone_wire(vec3 position, mat4 view, float thickness, float pixel_scale) {
    float w = dot(view[3], vec4(position, 1.0));
    float pixel_radius = w * pixel_scale;
    float radius = max(thickness, pixel_radius);
    float fade = thickness / radius;
    return fade; // position same size as pixel!
}

#endif

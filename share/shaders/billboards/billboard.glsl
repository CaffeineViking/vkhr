#ifndef VKHR_BILLBOARD_GLSL
#define VKHR_BILLBOARD_GLSL

vec2 positions[] = {
    { -1.0f, -1.0f },
    { +1.0f, +1.0f },
    { +1.0f, -1.0f },
    { +1.0f, +1.0f },
    { -1.0f, -1.0f },
    { -1.0f, +1.0f }
};

vec2 texcoords[] = {
    { 0.0f, 0.0f },
    { 1.0f, 1.0f },
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 0.0f },
    { 0.0f, 1.0f }
};

#endif

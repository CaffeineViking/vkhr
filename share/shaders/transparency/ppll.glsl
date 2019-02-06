#ifndef VKHR_PPLL_GLSL
#define VKHR_PPLL_GLSL

#define PPLL_NULL 0xffffffff

struct Node {
    vec4 color;
    float depth;
    uint next;
};

layout(binding = 6, r32ui)  uniform uimage2D heads;
layout(binding = 7, std430) buffer LinkedList {
    uint counter;
    Node nodes[];
};

#endif

#ifndef VKHR_PPLL_GLSL
#define VKHR_PPLL_GLSL

#define PPLL_NULL_NODE 0xffffffff

struct Node {
    vec4 color;
    float depth;
    uint next;
};

layout(binding = 6, r32ui)  uniform uimage2D ppll_heads;
layout(binding = 7, std430) buffer LinkedList {
    uint ppll_counter;
    Node ppll_nodes[];
};

layout(constant_id = 1) const uint ppll_size = 0;

uint ppll_next() {
    uint next_node = atomicAdd(ppll_counter, 1u);
    if (next_node > ppll_size)
        return PPLL_NULL_NODE;
    return next_node;
}

void ppll_put(vec4 color, float depth, uint node) {
    ppll_nodes[node].color = color;
    ppll_nodes[node].depth = depth;
}

uint ppll_head(ivec2 pixel) {
    return imageLoad(ppll_heads, pixel).r;
}

uint ppll_link(uint old, /* to the */ uint new) {
    return atomicExchange(ppll_nodes[old].next,
                          new); // returns old.
}

#endif

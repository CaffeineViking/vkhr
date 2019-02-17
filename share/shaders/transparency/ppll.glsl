#ifndef VKHR_PPLL_GLSL
#define VKHR_PPLL_GLSL

#define PPLL_EMPTY_NODE 0xffffffff

struct Node {
    vec4 color;
    float depth;
    uint prev;
};

layout(binding = 5, r32ui) uniform uimage2D ppll_heads;
layout(binding = 6, std430) buffer LinkedList {
    uint ppll_counter;
    uint ppll_size;
    Node ppll_nodes[];
};

uint ppll_next_node() {
    uint next_node = atomicAdd(ppll_counter, 1u);
    if (next_node >= ppll_size)
        return PPLL_EMPTY_NODE;
    ppll_nodes[next_node].prev = PPLL_EMPTY_NODE;
    return next_node;
}

void ppll_node_data(uint node, vec4 color, float depth, uint phead) {
    ppll_nodes[node].color = color;
    ppll_nodes[node].depth = depth;
    ppll_nodes[node].prev  = phead;
}

uint ppll_find_head(ivec2 pixel) {
    return imageLoad(ppll_heads, pixel).r;
}

uint ppll_link_node(ivec2 pixel, uint new_node) {
    return imageAtomicExchange(ppll_heads, pixel,
                               new_node);
}

#endif

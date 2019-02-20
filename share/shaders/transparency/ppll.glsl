#ifndef VKHR_PPLL_GLSL
#define VKHR_PPLL_GLSL

#define PPLL_NULL_NODE 0xffffffff

struct Node {
    vec4 color;
    float depth;
    uint prev;
    uint _p[2];
};

layout(binding = 5, r32ui) uniform uimage2D ppll_heads;
layout(binding = 6, std430) buffer LinkedList {
    uint ppll_counter;
    uint ppll_size;
    float _padding[2];
    Node ppll_nodes[];
};

uint ppll_next_node() {
    uint next_node = atomicAdd(ppll_counter, 1u);
    if (next_node >= 3686400) // TODO: ppll_size!
        return PPLL_NULL_NODE;
    ppll_nodes[next_node].prev = PPLL_NULL_NODE;
    return next_node;
}

void ppll_node_data(uint node, vec4 color, float depth) {
    ppll_nodes[node].color = color;
    ppll_nodes[node].depth = depth;
}

Node ppll_node(uint node) {
    return ppll_nodes[node];
}

uint ppll_head_node(ivec2 pixel) {
    return imageLoad(ppll_heads, pixel).r;
}

void ppll_link_node(ivec2 pixel, uint next_node) {
    uint prev_node = imageAtomicExchange(ppll_heads, pixel,
                                         next_node);
    ppll_nodes[next_node].prev = prev_node;
}

#endif

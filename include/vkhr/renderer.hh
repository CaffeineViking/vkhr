#ifndef VKHR_RENDERER_HH
#define VKHR_RENDERER_HH

#include <vkhr/scene_graph.hh>

namespace vkhr {
    class Renderer {
    public:
        virtual ~Renderer() = default;
        virtual void draw(const SceneGraph&) = 0;
    };
}

#endif

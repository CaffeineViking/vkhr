#ifndef VKHR_RAY_TRACER_HH
#define VKHR_RAY_TRACER_HH

#include <vkhr/renderer.hh>

namespace vkhr {
    class Raytracer final : public Renderer {
    public:
        void draw(const SceneGraph& scene) override;
    };
}

#endif

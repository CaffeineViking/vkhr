#ifndef VKHR_RAY_TRACER_HH
#define VKHR_RAY_TRACER_HH

#include <vkhr/renderer.hh>

#include <embree3/rtcore.h>

namespace vkhr {
    class Raytracer final : public Renderer {
    public:
        Raytracer();
        void draw(const SceneGraph& scene) override;
    };
}

#endif

#ifndef VKHR_RASTERIZER_HH
#define VKHR_RASTERIZER_HH

#include <vkhr/renderer.hh>

namespace vkhr {
    class Rasterizer final : public Renderer {
    public:
        Rasterizer();

        void load(const SceneGraph& scene) override;
        void draw(const SceneGraph& scene) override;

    private:
    };
}

#endif

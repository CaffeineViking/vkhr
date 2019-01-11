#ifndef VKHR_RAY_TRACER_HH
#define VKHR_RAY_TRACER_HH

#include <vkhr/renderer.hh>

#include <vkhr/embree/hair_style.hh>
#include <vkhr/embree/hair_style.hh>

#include <vkhr/ray.hh>

#include <embree3/rtcore.h>

#include <random>

namespace vkhr {
    class Interface;
    class Raytracer final : public Renderer {
    public:
        Raytracer(const SceneGraph& scene_graph);

        ~Raytracer() noexcept;

        void load(const SceneGraph& scene_graph) override;
        void draw(const SceneGraph& scene_graph) override;

        Raytracer(Raytracer&& raytracer) noexcept;
        Raytracer& operator=(Raytracer&& raytracer) noexcept;
        friend void swap(Raytracer& lhs, Raytracer& rhs);

        void toggle_shadows();

        Image& get_framebuffer();
        const Image& get_framebuffer() const;

    private:
        void set_flush_to_zero();
        void set_denormal_zero();

        bool shadows_on { true };
        int sampling_count { 1 };

        mutable RTCDevice device { nullptr };
        mutable RTCScene  scene  { nullptr };
        std::mt19937 random_number_generator;

        Image framebuffer;

        std::vector<embree::HairStyle> hair_style_geometry;

        friend class embree::HairStyle;

        friend class Interface;
    };
}

#endif

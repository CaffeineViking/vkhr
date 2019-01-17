#ifndef VKHR_RAY_TRACER_HH
#define VKHR_RAY_TRACER_HH

#include <vkhr/renderer.hh>

#include <vkhr/ray_tracer/hair_style.hh>
#include <vkhr/ray_tracer/ray.hh>

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

        glm::vec3 light_shading(const Ray& ray, const Camera& camera,
                                const LightSource& light,
                                RTCIntersectContext& context);
        float ambient_occlusion(const glm::vec3& point, RTCIntersectContext& context);

        Raytracer(Raytracer&& raytracer) noexcept;
        Raytracer& operator=(Raytracer&& raytracer) noexcept;
        friend void swap(Raytracer& lhs, Raytracer& rhs);

        void toggle_shadows();

        Image& get_framebuffer();
        void set_framebuffer(const Image& framebuffer);
        const Image& get_framebuffer() const;

        void clear();

        enum VisualizationMethod {
            Shaded           = 0,
            CombinedShadows  = 1,
            DirectShadows    = 2,
            AmbientOcclusion = 3
        };

    private:
        void set_flush_to_zero();
        void set_denormal_zero();

        bool shadows_on { true };
        bool now_dirty { false };

        VisualizationMethod visualization_method { Shaded };

        mutable RTCDevice device { nullptr };
        mutable RTCScene  scene  { nullptr };

        float ao_radius { 3.50f };
        std::size_t samples { 0 };

        std::vector<glm::dvec3> back_buffer;

        std::uint32_t seed { 0 };
        float sample(float min,  float max);
        std::uint32_t xorshift();

        // "Correlated Multi-Jittered Sampling":
        float rand_float(unsigned i, unsigned p);
        unsigned permute(unsigned i, unsigned l, unsigned p);
        glm::vec2 cmj(int s, int m, int n, int p);

        Image framebuffer;

        std::vector<embree::HairStyle> hair_styles;

        friend class embree::HairStyle;
        friend class Interface;
    };
}

#endif

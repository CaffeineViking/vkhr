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
        const Image& get_framebuffer() const;

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
        int sampling_count { 1 };

        VisualizationMethod visualization_method { Shaded };

        mutable RTCDevice device { nullptr };
        mutable RTCScene  scene  { nullptr };

        std::mt19937 prng; // For sampling :)
        std::uniform_real_distribution<float> sample { -1.0f, +1.0f };

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

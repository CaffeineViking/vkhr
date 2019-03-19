#ifndef VKHR_EMBREE_HAIR_STYLE_HH
#define VKHR_EMBREE_HAIR_STYLE_HH

#include <vkhr/scene_graph/hair_style.hh>

#include <vkhr/ray_tracer/shadable.hh>

#include <glm/glm.hpp>

#include <embree3/rtcore.h>

#include <vector>

#include <vkhr/rasterizer/hair_style.hh>

namespace vkhr {
    class Raytracer;
    namespace embree {
        class HairStyle final : public Shadable {
        public:
            HairStyle() = default;

            HairStyle(const vkhr::HairStyle& hair_style, const vkhr::Raytracer& raytracer);
            void load(const vkhr::HairStyle& hair_style, const vkhr::Raytracer& raytracer);

            glm::vec3 shade(const Ray& surface_intersection,
                            const LightSource& light_source,
                            const Camera& projection_camera) override;
            glm::vec4 get_tangent(const Ray& position) const;

            unsigned get_geometry() const;

            void update_parameters(const vkhr::vulkan::HairStyle& hair_style);

            const vkhr::HairStyle* get_pointer() const;

        private:
            glm::vec3 kajiya_kay(const glm::vec3& diffuse,
                                 const glm::vec3& specular,
                                 float p,
                                 const glm::vec3& tangent,
                                 const glm::vec3& light,
                                 const glm::vec3& eye);

            unsigned geometry { RTC_INVALID_GEOMETRY_ID };

            const vkhr::HairStyle* pointer { nullptr };

            RTCScene scene { nullptr };

            glm::vec3 hair_diffuse;
            float     hair_exponent;

            std::vector<glm::vec4> position_thickness;
        };
    }
}

#endif
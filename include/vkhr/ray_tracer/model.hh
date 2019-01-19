#ifndef VKHR_EMBREE_MODEL_HH
#define VKHR_EMBREE_MODEL_HH

#include <vkhr/scene_graph/model.hh>

#include <vkhr/ray_tracer/shadable.hh>

namespace vkhr {
    class Raytracer;
    namespace embree {
        class Model final : public Shadable {
        public:
            Model(const vkhr::Model& model,     const vkhr::Raytracer& raytracer);
            void load(const vkhr::Model& model, const vkhr::Raytracer& raytracer);

            Model() = default;

            unsigned get_geometry() const;

            glm::vec3 shade(const Ray& surface_intersection,
                            const LightSource& light_source,
                            const Camera& projection_camera);
        private:
            glm::vec3 blinn_phong(const glm::vec3& diffuse,
                                  const glm::vec3& specular,
                                  float shininess,
                                  const glm::vec3& normal,
                                  const glm::vec3& light,
                                  const glm::vec3& eye);

            RTCScene scene { nullptr };

            unsigned geometry { RTC_INVALID_GEOMETRY_ID };
        };
    }
}

#endif
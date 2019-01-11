#ifndef VKHR_EMBREE_SHADABLE_HH
#define VKHR_EMBREE_SHADABLE_HH

#include <vkhr/ray.hh>
#include <vkhr/light_source.hh>
#include <vkhr/camera.hh>
#include <glm/glm.hpp>

namespace vkhr {
    namespace embree {
        class Shadable {
        public:
            virtual ~Shadable() noexcept = default;
            virtual glm::vec3 shade(const Ray& surface_intersection,
                                    const LightSource& light_source,
                                    const Camera& projection_camera) = 0;
        };
    }
}

#endif
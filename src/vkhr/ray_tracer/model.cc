#include <vkhr/ray_tracer/model.hh>

namespace vkhr {
    namespace embree {
        glm::vec3 Model::shade(const Ray& surface_intersection,
                               const LightSource& light_source,
                               const Camera& projection_camera) {
            return glm::vec3 { 0.0 };
        }
    }
}

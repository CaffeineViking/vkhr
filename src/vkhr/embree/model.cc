#include <vkhr/embree/model.hh>

namespace vkhr {
    namespace embree {
        glm::vec4 Model::shade(const Ray& surface_intersection,
                               const LightSource& light_source,
                               const Camera& projection_camera) {
            return glm::vec4 { 0.0 };
        }
    }
}

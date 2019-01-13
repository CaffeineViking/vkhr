#ifndef VKHR_EMBREE_MODEL_HH
#define VKHR_EMBREE_MODEL_HH

#include <vkhr/ray_tracer/shadable.hh>

namespace vkhr {
    namespace embree {
        class Model final : public Shadable {
        public:
            glm::vec3 shade(const Ray& surface_intersection,
                            const LightSource& light_source,
                            const Camera& projection_camera);
        };
    }
}

#endif
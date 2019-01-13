#ifndef VKHR_EMBREE_BILLBOARD_HH
#define VKHR_EMBREE_BILLBOARD_HH

#include <vkhr/ray_tracer/shadable.hh>

namespace vkhr {
    namespace embree {
        class Billboard final : public Shadable {
        public:
            glm::vec3 shade(const Ray& surface_intersection,
                            const LightSource& light_source,
                            const Camera& projection_camera);
        };
    }
}

#endif
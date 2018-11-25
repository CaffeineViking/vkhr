#ifndef VKHR_EMBREE_RAY_HH
#define VKHR_EMBREE_RAY_HH

#include <embree3/rtcore.h>

#include <glm/glm.hpp>

namespace vkhr {
    class Ray final {
    public:
        Ray(const glm::vec3& origin,
            const glm::vec3& direction,
            float near_plane_t_value);

        static constexpr float Epsilon { 0.000001 };

        glm::vec3 get_origin() const;
        glm::vec3 get_direction() const;

        RTCRay& get_ray();
        RTCHit& get_hit();

        bool hit_surface() const;
        bool is_occluded() const;

        glm::vec2 get_uv() const;
        glm::vec3 get_tangent() const;
        glm::vec3 get_normal() const;

        glm::vec4 get_uniform_tangent() const;
        glm::vec4 get_uniform_normal()  const;

        unsigned get_primitive_id() const;
        unsigned get_geometry_id()  const;
        bool hit_geometry(unsigned) const;

        glm::vec3 get_intersection_point() const;

        bool intersects(RTCScene& scene,  RTCIntersectContext& context);
        bool occluded_by(RTCScene& scene, RTCIntersectContext& context);

    private:
        RTCRayHit ray_hit { };
    };
}

#endif
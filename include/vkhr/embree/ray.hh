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

        unsigned get_primitive_id() const;
        unsigned get_geometry_id()  const;

        glm::vec3 get_intersection_point() const;

        bool intersects(RTCScene& scene,  RTCIntersectContext& context);
        bool occluded_by(RTCScene& scene, RTCIntersectContext& context);

    private:
        RTCRayHit ray_hit { };
    };

    Ray::Ray(const glm::vec3& origin, const glm::vec3& direction, float tnear_plane) {
        ray_hit.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        ray_hit.ray.org_x = origin.x;
        ray_hit.ray.org_y = origin.y;
        ray_hit.ray.org_z = origin.z;

        ray_hit.ray.dir_x = direction.x;
        ray_hit.ray.dir_y = direction.y;
        ray_hit.ray.dir_z = direction.z;

        ray_hit.ray.tnear = tnear_plane;
        ray_hit.ray.tfar  = std::numeric_limits<float>::infinity();
    }

    RTCRay& Ray::get_ray() {
        return ray_hit.ray;
    }

    RTCHit& Ray::get_hit() {
        return ray_hit.hit;
    }

    glm::vec3 Ray::get_origin() const {
        return { ray_hit.ray.org_x,
                 ray_hit.ray.org_y,
                 ray_hit.ray.org_z };
    }

    glm::vec3 Ray::get_direction() const {
        return { ray_hit.ray.dir_x,
                 ray_hit.ray.dir_y,
                 ray_hit.ray.dir_z };
    }

    bool Ray::hit_surface() const {
        return ray_hit.hit.geomID != RTC_INVALID_GEOMETRY_ID;
    }

    bool Ray::is_occluded() const {
        return ray_hit.ray.tfar < 0.0;
    }

    glm::vec2 Ray::get_uv() const {
        return { ray_hit.hit.u,
                 ray_hit.hit.v };
    }

    glm::vec3 Ray::get_normal() const {
        return { ray_hit.hit.Ng_x,
                 ray_hit.hit.Ng_y,
                 ray_hit.hit.Ng_z };
    }

    glm::vec3 Ray::get_tangent() const {
        return get_normal();
    }

    unsigned Ray::get_primitive_id() const {
        return ray_hit.hit.primID;
    }

    unsigned Ray::get_geometry_id() const {
        return ray_hit.hit.geomID;
    }

    glm::vec3 Ray::get_intersection_point() const {
        return get_origin() + get_direction() * ray_hit.ray.tfar;
    }

    bool Ray::intersects(RTCScene& scene,  RTCIntersectContext& context) {
        rtcIntersect1(scene, &context, &ray_hit);
        return hit_surface();
    }

    bool Ray::occluded_by(RTCScene& scene, RTCIntersectContext& context) {
        rtcOccluded1(scene, &context, &ray_hit.ray);
        return is_occluded();
    }
}

#endif
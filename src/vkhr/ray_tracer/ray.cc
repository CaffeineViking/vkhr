#include <vkhr/ray_tracer/ray.hh>

namespace vkhr {
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

    glm::vec4 Ray::get_uniform_normal() const {
        return { ray_hit.hit.Ng_x,
                 ray_hit.hit.Ng_y,
                 ray_hit.hit.Ng_z,
                 0.0f };
    }

    glm::vec3 Ray::get_tangent() const {
        return get_normal();
    }

    glm::vec4 Ray::get_uniform_tangent() const {
        return get_uniform_normal();
    }

    unsigned Ray::get_primitive_id() const {
        return ray_hit.hit.primID;
    }

    unsigned Ray::get_geometry_id() const {
        return ray_hit.hit.geomID;
    }

    bool Ray::hit_geometry(unsigned  id) const {
        return ray_hit.hit.geomID == id;
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
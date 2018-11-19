#ifndef VKHR_RAY_TRACER_HH
#define VKHR_RAY_TRACER_HH

#include <vkhr/renderer.hh>

#include <embree3/rtcore.h>

namespace vkhr {
    class Raytracer final : public Renderer {
    public:
        Raytracer(const Camera& camera, HairStyle& hair_style);

        void draw(const Camera& camera);

        void load(const SceneGraph& scene) override;
        void draw(const SceneGraph& scene) override;

        ~Raytracer() noexcept;

        Raytracer(Raytracer&& raytracer) noexcept;
        Raytracer& operator=(Raytracer&& raytracer) noexcept;
        friend void swap(Raytracer& lhs, Raytracer& rhs);

        void toggle_shadows();

    private:
        void set_flush_to_zero();
        void set_denormal_zero();

        glm::vec3 kajiya_kay(const glm::vec3& diffuse,
                             const glm::vec3& specular,
                             float p,
                             const glm::vec3& tangent,
                             const glm::vec3& light,
                             const glm::vec3& eye);

        bool shadows_off { true };

        Image back_buffer;

        RTCGeometry hair;

        std::vector<glm::vec4> hair_vertices;

        RTCScene scene;

        RTCDevice device;
    };

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

}

#endif

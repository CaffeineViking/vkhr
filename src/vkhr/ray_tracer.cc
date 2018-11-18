#include <vkhr/ray_tracer.hh>

#include <utility>
#include <iostream>
#include <string>

#include <xmmintrin.h>
#include <pmmintrin.h>

#include <limits>
#include <vector>
#include <cmath>

namespace vkhr {
    static void embree_debug_callback(void*, const RTCError code,
                                             const char* message) {
        if (code == RTC_ERROR_UNKNOWN)
            return;
        if (message) {
            std::cerr << '\n'
                      << message
                      << std::endl;
        }
    }

    Raytracer::Raytracer(const Camera& camera, vkhr::HairStyle& hair_style) {
        set_flush_to_zero();
        set_denormal_zero();

        device = rtcNewDevice("verbose=1");

        rtcSetDeviceErrorFunction(device, embree_debug_callback, nullptr);

        scene = rtcNewScene(device);

        auto hair_vertices  =  hair_style.create_position_thickness_data();
        hair_style.generate_control_points_for(HairStyle::CurveType::Line);

        const auto& hair_indices = hair_style.get_control_points();

        RTCGeometry hair { rtcNewGeometry(device, RTC_GEOMETRY_TYPE_FLAT_LINEAR_CURVE) };

        rtcSetSharedGeometryBuffer(hair, RTC_BUFFER_TYPE_VERTEX, 0, RTC_FORMAT_FLOAT4,
                                   hair_vertices.data(), 0, sizeof(hair_vertices[0]),
                                   hair_vertices.size());
        rtcSetSharedGeometryBuffer(hair, RTC_BUFFER_TYPE_INDEX, 0, RTC_FORMAT_UINT,
                                   hair_indices.data(), 0, sizeof(hair_indices[0]),
                                   hair_indices.size());

        rtcCommitGeometry(hair);
        rtcAttachGeometry(scene, hair);
        rtcReleaseGeometry(hair);

        rtcCommitScene(scene);

        vkhr::Image framebuffer { 1280, 720 };

        framebuffer.clear();

        auto hair_color = glm::vec3(0.80f, 0.57f, 0.32f) * 0.40f;
        auto light = glm::normalize(glm::vec3(1.0f, 2.0f, 1.0f));
        auto light_color = glm::vec3(1.0f, 0.77f, 0.56f) * 0.20f;

        auto viewing_plane = camera.get_viewing_plane();

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < static_cast<int>(framebuffer.get_height()); ++j)
        for (int i = 0; i < static_cast<int>(framebuffer.get_width()); ++i) {
            float x { static_cast<float>(i) }, y { static_cast<float>(j) };

            RTCIntersectContext      context;
            rtcInitIntersectContext(&context);

            auto eye_direction = glm::normalize(x * viewing_plane.x +
                                                y * viewing_plane.y +
                                                    viewing_plane.z);
            Ray ray { viewing_plane.point, eye_direction , 0.0000f };

            if (ray.intersects(scene, context)) {
                glm::vec4 color { 0.0, 0.0, 0.0, 1.0 };

                Ray shadow_ray { ray.get_intersection_point(), light, Ray::Epsilon };

                auto tangent = glm::vec4 { ray.get_tangent(), 0 };
                tangent = camera.get_view_matrix() * tangent;

                // if (shadow_ray.occluded_by(scene, context)) {
                    auto shading = kajiya_kay(hair_color, light_color, 80.0f,
                                              glm::normalize(tangent), light,
                                              glm::vec3(0.00f, 0.0f, 0.00f));
                    color = glm::vec4 { shading, 1.0f };
                // }

                framebuffer.set_pixel(i, j, { std::clamp(color.r, 0.0f, 1.0f) * 255,
                                              std::clamp(color.g, 0.0f, 1.0f) * 255,
                                              std::clamp(color.b, 0.0f, 1.0f) * 255,
                                              std::clamp(color.a, 0.0f, 1.0f) * 255 });
            }
        }

        framebuffer.horizontal_flip();
        framebuffer.save("render.png");
    }

    void Raytracer::draw(const SceneGraph&) {
    }

    Raytracer::~Raytracer() noexcept {
        rtcReleaseScene(scene);
        rtcReleaseDevice(device);
    }

    Raytracer::Raytracer(Raytracer&& raytracer) noexcept {
        swap(*this, raytracer);
    }

    Raytracer& Raytracer::operator=(Raytracer&& raytracer) noexcept {
        swap(*this, raytracer);
        return *this;
    }

    void swap(Raytracer& lhs, Raytracer& rhs) {
        using std::swap;
        std::swap(lhs.device, rhs.device);
    }

    void Raytracer::set_flush_to_zero() {
        _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    }

    void Raytracer::set_denormal_zero() {
        _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
    }

    glm::vec3 Raytracer::kajiya_kay(const glm::vec3& diffuse,
                                    const glm::vec3& specular,
                                    float p,
                                    const glm::vec3& tangent,
                                    const glm::vec3& light,
                                    const glm::vec3& eye) {
        float cosTL = glm::dot(tangent, light);
        float cosTE = glm::dot(tangent, eye);

        float cosTL_squared = cosTL*cosTL;
        float cosTE_squared = cosTE*cosTE;

        float one_minus_cosTL_squared = 1.0f - cosTL_squared;
        float one_minus_cosTE_squared = 1.0f - cosTE_squared;

        float sinTL = one_minus_cosTL_squared / std::sqrt(one_minus_cosTL_squared);
        float sinTE = one_minus_cosTE_squared / std::sqrt(one_minus_cosTE_squared);

        glm::vec3 diffuse_colors = diffuse  * sinTL;
        glm::vec3 specular_color = specular * std::pow((cosTL * cosTE + sinTL * sinTE), p);

        return diffuse_colors + specular_color;
    }

    Ray::Ray() {
    }

    Ray::Ray(const glm::vec3& origin, const glm::vec3& direction, float tnear_plane) {
        rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;

        rh.ray.org_x = origin.x;
        rh.ray.org_y = origin.y;
        rh.ray.org_z = origin.z;

        rh.ray.dir_x = direction.x;
        rh.ray.dir_y = direction.y;
        rh.ray.dir_z = direction.z;

        rh.ray.tnear = tnear_plane;
        rh.ray.tfar  = std::numeric_limits<float>::infinity();
    }

    RTCRay& Ray::get_ray() {
        return rh.ray;
    }

    RTCHit& Ray::get_hit() {
        return rh.hit;
    }

    glm::vec3 Ray::get_origin() const {
        return { rh.ray.org_x,
                 rh.ray.org_y,
                 rh.ray.org_z };
    }

    glm::vec3 Ray::get_direction() const {
        return { rh.ray.dir_x,
                 rh.ray.dir_y,
                 rh.ray.dir_z };
    }

    bool Ray::hit_surface() const {
        return rh.hit.geomID != RTC_INVALID_GEOMETRY_ID;
    }

    bool Ray::is_occluded() const {
        return rh.ray.tfar >= 0.0;
    }

    glm::vec2 Ray::get_uv() const {
        return { rh.hit.u,
                 rh.hit.v };
    }

    glm::vec3 Ray::get_normal() const {
        return { rh.hit.Ng_x,
                 rh.hit.Ng_y,
                 rh.hit.Ng_z };
    }

    glm::vec3 Ray::get_tangent() const {
        return get_normal();
    }

    unsigned Ray::get_primitive_id() const {
        return rh.hit.primID;
    }

    unsigned Ray::get_geometry_id() const {
        return rh.hit.geomID;
    }

    glm::vec3 Ray::get_intersection_point() const {
        return get_origin() + get_direction() * rh.ray.tfar;
    }

    bool Ray::intersects(RTCScene& scene,  RTCIntersectContext& context) {
        rtcIntersect1(scene, &context, &rh);
        return hit_surface();
    }

    bool Ray::occluded_by(RTCScene& scene, RTCIntersectContext& context) {
        rtcOccluded1(scene, &context, &rh.ray);
        return is_occluded();
    }
}

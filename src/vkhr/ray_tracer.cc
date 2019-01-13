#include <vkhr/ray_tracer.hh>

#include <utility>
#include <iostream>
#include <string>

#include <xmmintrin.h>
#include <pmmintrin.h>

#include <glm/gtx/rotate_vector.hpp>

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

    Raytracer::Raytracer(const SceneGraph& scene_graph) {
        set_flush_to_zero();
        set_denormal_zero();

        device = rtcNewDevice("verbose=1");

        rtcSetDeviceErrorFunction(device, embree_debug_callback, nullptr);

        std::random_device seed;
        prng.seed(seed());
        load(scene_graph);
    }

    Raytracer::~Raytracer() noexcept {
        rtcReleaseScene(scene);
        rtcReleaseDevice(device);
    }

    void Raytracer::load(const SceneGraph& scene_graph) {
        if (scene != nullptr) {
            rtcReleaseScene(scene);
            scene = nullptr;
        }

        scene = rtcNewScene(device);

        for (const auto& hair_style_node : scene_graph.get_nodes_with_hair_styles()) {
            for (const auto hair_style : hair_style_node->get_hair_styles()) {
                auto hair = embree::HairStyle { *hair_style, *this };
                if (hair.get_geometry() >= hair_styles.size())
                    hair_styles.resize(hair.get_geometry()+1);
                hair_styles[hair.get_geometry()] = std::move(hair);
            }
        }

        rtcCommitScene(scene);

        framebuffer = Image {
            scene_graph.get_camera().get_width(),
            scene_graph.get_camera().get_height()
        };

        framebuffer.clear();
    }

    void Raytracer::draw(const SceneGraph& scene_graph) {
        framebuffer.clear();

        auto& viewing_plane = scene_graph.get_camera().get_viewing_plane();

        auto& camera = scene_graph.get_camera();
        auto& light  = scene_graph.get_light_sources().front();

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < static_cast<int>(framebuffer.get_height()); ++j)
        for (int i = 0; i < static_cast<int>(framebuffer.get_width()); ++i) {
            float x { static_cast<float>(i) }, y { static_cast<float>(j) };

            RTCIntersectContext      context;
            rtcInitIntersectContext(&context);

            auto eye_direction = (x * viewing_plane.x +
                                  y * viewing_plane.y +
                                      viewing_plane.z);

            Ray ray {
                viewing_plane.point,
                eye_direction,
                0.0f
            };

            glm::vec3 pixel_color { 0.0f, 0.0f, 0.0f };

            if (ray.intersects(scene, context)) {
                auto surface_point = ray.get_intersection_point();

                pixel_color = light_shading(ray, camera, light, context);

                if (visualization_method != DirectShadows) {
                    pixel_color *= ambient_occlusion(surface_point, context);
                }

                framebuffer.set_pixel(framebuffer.get_width() - i - 1, j, {
                    glm::clamp(pixel_color.r, 0.0f, 1.0f) * 255,
                    glm::clamp(pixel_color.g, 0.0f, 1.0f) * 255,
                    glm::clamp(pixel_color.b, 0.0f, 1.0f) * 255,
                    255 // for now assume that there's no alpha.
              });
            }
        }
    }

    glm::vec3 Raytracer::light_shading(const Ray& ray, const Camera& camera, const LightSource& light, RTCIntersectContext& context) {
        Ray shadow_ray {
            ray.get_intersection_point(),
            light.get_direction(),
            Ray::Epsilon
        };

        if (visualization_method == AmbientOcclusion) {
            return glm::vec3 { 1.0f };
        } else if (!shadow_ray.occluded_by(scene, context) || !shadows_on) {
            if (visualization_method == Shaded) {
                return hair_styles[ray.get_geometry_id()].shade(ray, light, camera);
            } else {
                return glm::vec3 { 1.0f };
            }
        } else {
            return glm::vec3 { 0.0f };
        }
    }

    float Raytracer::ambient_occlusion(const glm::vec3& position, RTCIntersectContext& context) {
        std::size_t occluded_rays { 0 };

        for (std::size_t s { 0 }; s < sampling_count; ++s) {
            auto random_direction = glm::vec3 {
                sample(prng),
                sample(prng),
                sample(prng)
            };

            Ray random_ray {
                position,
                random_direction,
                Ray::Epsilon
            };

            if (!random_ray.occluded_by(scene, context))
                ++occluded_rays;
        }

        // We divide here since we are sampling in a sphere, and not in a hemisphere as usual...
        return static_cast<float>(occluded_rays) / (static_cast<float>(sampling_count) / 2.00f);
    }

    Image& Raytracer::get_framebuffer() {
        return framebuffer;
    }

    const Image& Raytracer::get_framebuffer() const {
        return framebuffer;
    }

    void Raytracer::toggle_shadows() {
        shadows_on = !shadows_on;
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
}

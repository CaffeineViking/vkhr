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

        std::random_device rng; seed=rng(); 

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

        // Load only the set of hair styles which are within the actual scene graph.
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

        back_buffer.resize(framebuffer.get_pixel_count(), glm::dvec3 { 0.0, 0.0, 0.0 });

        clear();
    }

    void Raytracer::draw(const SceneGraph& scene_graph) {
        if (now_dirty)
            clear();

        auto& viewing_plane = scene_graph.get_camera().get_viewing_plane();

        auto& camera = scene_graph.get_camera();
        auto& light  = scene_graph.get_light_sources().front();

        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < static_cast<int>(framebuffer.get_height()); ++j)
        for (int i = 0; i < static_cast<int>(framebuffer.get_width());  ++i) {
            float x { static_cast<float>(i) },
                  y { static_cast<float>(j) };

            glm::dvec3 sample_color { 1.000, 1.000, 1.000 };

            RTCIntersectContext      context;
            rtcInitIntersectContext(&context);

            glm::vec2 jitter {
                sample(0.0f, 1.0f),
                sample(0.0f, 1.0f)
            };

            auto direction = ((x + jitter.x) * viewing_plane.x +
                              (y + jitter.y) * viewing_plane.y +
                                               viewing_plane.z);

            Ray ray { viewing_plane.point, direction, 0.0000f };

            if (ray.intersects(scene, context)) {
                glm::vec3 position { ray.get_intersection_point() };

                sample_color = light_shading(ray, camera, light, context);

                if (visualization_method != DirectShadows) {
                    sample_color *= ambient_occlusion(position,  context);
                }
            }

            back_buffer[i + j * framebuffer.get_width()] += sample_color;
        }

        ++samples;

        framebuffer.clear();
        framebuffer.copy(back_buffer, samples);
    }

    glm::vec3 Raytracer::light_shading(const Ray& ray, const Camera& camera, const LightSource& light, RTCIntersectContext& context) {
        Ray shadow_ray {
            ray.get_intersection_point(),
            light.get_spotlight_origin(),
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
        auto random_direction = glm::vec3 {
            sample(-1.0f, +1.0f),
            sample(-1.0f, +1.0f),
            sample(-1.0f, +1.0f)
        };

        Ray random_ray {
            position,
            random_direction,
            Ray::Epsilon
        };

        if (!random_ray.occluded_by(scene, context, ao_radius))
            return 2.0f;
        else
            return 0.0f;
    }

    Image& Raytracer::get_framebuffer() {
        return framebuffer;
    }

    const Image& Raytracer::get_framebuffer() const {
        return framebuffer;
    }

    void Raytracer::set_framebuffer(const Image& framebuffer) {
        this->framebuffer = framebuffer;
    }

    void Raytracer::clear() {
        samples = 0;
        std::fill(back_buffer.begin(),
                  back_buffer.end(),
                  glm::vec3 { 0.0 });
        now_dirty = false;
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

    std::uint32_t Raytracer::xorshift() {
        std::uint32_t x = seed;
        x ^= x << 13;
        x ^= x >> 7;
        x ^= x << 17;
        seed = x;
        return x;
    }

    float Raytracer::sample(float min, float max) {
        float random_number = xorshift();
        float range = max - min;
        float normalization = std::numeric_limits<std::uint32_t>::max();
        return (random_number / normalization) * range + min;
    }

    // From: "Correlated Multi-Jitter Sampling" by Pixar:

    float Raytracer::rand_float(unsigned i, unsigned p) {
        i ^= p;
        i ^= i >> 17;
        i ^= i >> 10;
        i *= 0xb36534e5;
        i ^= i >> 12;
        i ^= i >> 21;
        i *= 0x93fc4795;
        i ^= 0xdf6e307f;
        i ^= i >> 17;
        i *= 1 | p >> 18;
        return i * (1.0f / 4294967808.0f);
    }

    unsigned Raytracer::permute(unsigned i, unsigned l, unsigned p) {
        unsigned w = l - 1;

        w |= w >> 1;
        w |= w >> 2;
        w |= w >> 4;
        w |= w >> 8;
        w |= w >> 16;

        do {
            i ^= p;
            i *= 0xe170893d;
            i ^= p >> 16;
            i ^= (i & w) >> 4;
            i ^= p >> 8;
            i *= 0x0929eb3f;
            i ^= p >> 23;
            i ^= (i & w) >> 1;
            i *= 1 | p >> 27;
            i *= 0x6935fa69;
            i ^= (i & w) >> 11;
            i *= 0x74dcb303;
            i ^= (i & w) >> 2;
            i *= 0x9e501cc3;
            i ^= (i & w) >> 2;
            i *= 0xc860a3df;
            i &= w;
            i ^= i >> 5;
        } while (i >= l);

        return (i + p) % l;
    }

    glm::vec2 Raytracer::cmj(int s, int m, int n, int p) {
        int sx = permute(s % m, m, p * 0xa511e9b3),
            sy = permute(s / m, n, p * 0x63d83595);

        float jx = rand_float(s, p * 0xa399d265),
              jy = rand_float(s, p * 0x711ad6a5);

        glm::vec2 r = {
            (s % m + (sy + jx) / n) / m,
            (s / m + (sx + jy) / m) / n
        };

        return r;
    }
}

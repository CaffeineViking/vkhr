#include <vkhr/ray_tracer.hh>

#include <xmmintrin.h>
#include <pmmintrin.h>

#include <utility>
#include <iostream>
#include <string>

#include <limits>
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

    glm::vec3* face_colors   = nullptr;
    glm::vec3* vertex_colors = nullptr;

    unsigned int addCube(RTCDevice device, RTCScene scene_i) {
        /* create a triangulated cube with 12 triangles and 8 vertices */
        RTCGeometry mesh = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        /* create face and vertex color arrays */
        face_colors   = new glm::vec3[12];
        vertex_colors = new glm::vec3[8];

        /* set vertices and vertex colors */
        glm::vec3* vertices = (glm::vec3*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(glm::vec3),8);
        vertex_colors[0] = glm::vec3(0,0,0); vertices[0][0] = -1; vertices[0][1] = -1; vertices[0][2] = -1;
        vertex_colors[1] = glm::vec3(0,0,1); vertices[1][0] = -1; vertices[1][1] = -1; vertices[1][2] = +1;
        vertex_colors[2] = glm::vec3(0,1,0); vertices[2][0] = -1; vertices[2][1] = +1; vertices[2][2] = -1;
        vertex_colors[3] = glm::vec3(0,1,1); vertices[3][0] = -1; vertices[3][1] = +1; vertices[3][2] = +1;
        vertex_colors[4] = glm::vec3(1,0,0); vertices[4][0] = +1; vertices[4][1] = -1; vertices[4][2] = -1;
        vertex_colors[5] = glm::vec3(1,0,1); vertices[5][0] = +1; vertices[5][1] = -1; vertices[5][2] = +1;
        vertex_colors[6] = glm::vec3(1,1,0); vertices[6][0] = +1; vertices[6][1] = +1; vertices[6][2] = -1;
        vertex_colors[7] = glm::vec3(1,1,1); vertices[7][0] = +1; vertices[7][1] = +1; vertices[7][2] = +1;

        /* set triangles and face colors */
        int tri = 0;
        glm::uvec3* triangles = (glm::uvec3*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(glm::uvec3),12);

        // left side
        face_colors[tri] = glm::vec3(1,0,0); triangles[tri][0] = 0; triangles[tri][1] = 1; triangles[tri][2] = 2; tri++;
        face_colors[tri] = glm::vec3(1,0,0); triangles[tri][0] = 1; triangles[tri][1] = 3; triangles[tri][2] = 2; tri++;

        // right side
        face_colors[tri] = glm::vec3(0,1,0); triangles[tri][0] = 4; triangles[tri][1] = 6; triangles[tri][2] = 5; tri++;
        face_colors[tri] = glm::vec3(0,1,0); triangles[tri][0] = 5; triangles[tri][1] = 6; triangles[tri][2] = 7; tri++;

        // bottom side
        face_colors[tri] = glm::vec3(0.5f);  triangles[tri][0] = 0; triangles[tri][1] = 4; triangles[tri][2] = 1; tri++;
        face_colors[tri] = glm::vec3(0.5f);  triangles[tri][0] = 1; triangles[tri][1] = 4; triangles[tri][2] = 5; tri++;

        // top side
        face_colors[tri] = glm::vec3(1.0f);  triangles[tri][0] = 2; triangles[tri][1] = 3; triangles[tri][2] = 6; tri++;
        face_colors[tri] = glm::vec3(1.0f);  triangles[tri][0] = 3; triangles[tri][1] = 7; triangles[tri][2] = 6; tri++;

        // front side
        face_colors[tri] = glm::vec3(0,0,1); triangles[tri][0] = 0; triangles[tri][1] = 2; triangles[tri][2] = 4; tri++;
        face_colors[tri] = glm::vec3(0,0,1); triangles[tri][0] = 2; triangles[tri][1] = 6; triangles[tri][2] = 4; tri++;

        // back side
        face_colors[tri] = glm::vec3(1,1,0); triangles[tri][0] = 1; triangles[tri][1] = 5; triangles[tri][2] = 3; tri++;
        face_colors[tri] = glm::vec3(1,1,0); triangles[tri][0] = 3; triangles[tri][1] = 5; triangles[tri][2] = 7; tri++;

        rtcSetGeometryVertexAttributeCount(mesh,1);
        rtcSetSharedGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,vertex_colors,0,sizeof(glm::vec3),8);

        rtcCommitGeometry(mesh);
        unsigned int geomID = rtcAttachGeometry(scene_i,mesh);
        rtcReleaseGeometry(mesh);
        return geomID;
    }

    unsigned int addGroundPlane(RTCDevice device, RTCScene scene_i) {
        RTCGeometry mesh = rtcNewGeometry (device, RTC_GEOMETRY_TYPE_TRIANGLE);

        glm::vec3* vertices = (glm::vec3*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,sizeof(glm::vec3),4);
        vertices[0][0] = -10; vertices[0][1] = -2; vertices[0][2] = -10;
        vertices[1][0] = -10; vertices[1][1] = -2; vertices[1][2] = +10;
        vertices[2][0] = +10; vertices[2][1] = -2; vertices[2][2] = -10;
        vertices[3][0] = +10; vertices[3][1] = -2; vertices[3][2] = +10;

        glm::uvec3* triangles = (glm::uvec3*) rtcSetNewGeometryBuffer(mesh,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,sizeof(glm::uvec3),2);
        triangles[0][0] = 0; triangles[0][1] = 1; triangles[0][2] = 2;
        triangles[1][0] = 1; triangles[1][1] = 3; triangles[1][2] = 2;

        rtcCommitGeometry(mesh);
        unsigned int geomID = rtcAttachGeometry(scene_i,mesh);
        rtcReleaseGeometry(mesh);
        return geomID;
    }

    Raytracer::Raytracer() {
        set_flush_to_zero();
        set_denormal_zero();

        device = rtcNewDevice("verbose=1");

        rtcSetDeviceErrorFunction(device, embree_debug_callback, nullptr);

        scene = rtcNewScene(device);

        addCube(device, scene);

        addGroundPlane(device, scene);

        rtcCommitScene(scene);

        vkhr::Image framebuffer { 1280, 720 };

        vkhr::Camera camera { glm::radians(100.0f), framebuffer.get_width(),
                                                   framebuffer.get_height()};
        camera.look_at({ +0.00f, +0.00f, +0.00f }, { +1.5f, +1.5f, -1.5f });

        for (float y { 0.0f }; y < framebuffer.get_height(); ++y) {
            for (float x { 0.0f }; x < framebuffer.get_width(); ++x) {
                RTCIntersectContext context;

                rtcInitIntersectContext(&context);

                RTCRayHit ray;

                auto viewing_plane = camera.get_viewing_plane();

                ray.ray.org_x = viewing_plane.point.x;
                ray.ray.org_y = viewing_plane.point.y;
                ray.ray.org_z = viewing_plane.point.z;
                ray.ray.tnear = 0.0f;

                auto ray_direction = glm::normalize(x * viewing_plane.x +
                                                    y * viewing_plane.y +
                                                        viewing_plane.z);

                ray.ray.dir_x = ray_direction.x;
                ray.ray.dir_y = ray_direction.y;
                ray.ray.dir_z = ray_direction.z;

                ray.ray.flags  = 0;
                ray.hit.geomID = RTC_INVALID_GEOMETRY_ID;

                ray.ray.tfar = std::numeric_limits<float>::infinity();

                rtcIntersect1(scene, &context, &ray);

                glm::vec3 color { 0.0, 0.0, 0.0 };

                if (ray.hit.geomID != RTC_INVALID_GEOMETRY_ID) {
                    glm::vec3 diffuse = face_colors[ray.hit.primID];
                    color += diffuse*0.5f;

                    glm::vec3 light = glm::normalize(glm::vec3 { -1, -1, -1 });

                    RTCRay shadow_ray;

                    auto intersection = viewing_plane.point + ray_direction * ray.ray.tfar;

                    shadow_ray.org_x = intersection.x;
                    shadow_ray.org_y = intersection.y;
                    shadow_ray.org_z = intersection.z;
                    shadow_ray.tnear = 0.001f;

                    shadow_ray.dir_x = -light.x;
                    shadow_ray.dir_y = -light.y;
                    shadow_ray.dir_z = -light.z;

                    shadow_ray.flags = 0;
                    shadow_ray.tfar = std::numeric_limits<float>::infinity();

                    rtcOccluded1(scene, &context, &shadow_ray);

                    if (shadow_ray.tfar >= 0.0f) {
                        color += diffuse * std::clamp(-glm::dot(light, glm::normalize(glm::vec3 { ray.hit.Ng_x, ray.hit.Ng_y, ray.hit.Ng_z })), 0.0f, 1.0f);
                    }
                }

                framebuffer.set_pixel(x, y, { std::clamp(color.r, 0.0f, 1.0f) * 255,
                                              std::clamp(color.g, 0.0f, 1.0f) * 255,
                                              std::clamp(color.b, 0.0f, 1.0f) * 255,
                                              255 });
            }
        }

        framebuffer.save("render.png");
    }

    void Raytracer::draw(const SceneGraph&) {
    }

    Raytracer::~Raytracer() noexcept {
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
}

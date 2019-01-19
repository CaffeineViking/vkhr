#include <vkhr/ray_tracer/model.hh>

#include <vkhr/ray_tracer.hh>

namespace vkhr {
    namespace embree {
        Model::Model(const vkhr::Model& model, const vkhr::Raytracer& raytracer) {
            load(model, raytracer);
        }

        void Model::load(const vkhr::Model& model, const vkhr::Raytracer& raytracer) {
            auto model_geometry = rtcNewGeometry(raytracer.device, RTC_GEOMETRY_TYPE_TRIANGLE);

            // Upload geometry here.

            scene = raytracer.scene;

            rtcCommitGeometry(model_geometry);
            geometry = rtcAttachGeometry(raytracer.scene, model_geometry);
            rtcReleaseGeometry(model_geometry);
        }

        glm::vec3 Model::shade(const Ray& surface_intersection,
                               const LightSource& light_source,
                               const Camera& projection_camera) {
            return glm::vec3 { 1.0f };
        }

        unsigned Model::get_geometry() const {
            return geometry;
        }

        glm::vec3 Model::blinn_phong(const glm::vec3& diffuse,
                                     const glm::vec3& specular,
                                     float shininess,
                                     const glm::vec3& normal,
                                     const glm::vec3& light,
                                     const glm::vec3& eye) {
            return glm::vec3 { 1.0f };
        }
    }
}

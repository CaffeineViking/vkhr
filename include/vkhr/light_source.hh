#ifndef VKHR_LIGHT_SOURCE_HH
#define VKHR_LIGHT_SOURCE_HH

#include <vkhr/camera.hh>

#include <glm/glm.hpp>

namespace vkhr {
    class LightSource {
    public:
        enum class Type {
            Point,
            Directional
        };

        struct Buffer {
            glm::vec4 vector;
            glm::vec4 intensity;
        };

        LightSource() = default;

        LightSource(const glm::vec3& vector, Type type,
                    const glm::vec3& intensity,
                    float light_cutoff = 0.0);

        const Buffer& get_buffer() const;

        Type get_type() const;
        void set_type(Type light_source_type);

        glm::vec3 get_position() const;
        void set_position(const glm::vec3& position);
        const glm::vec4& get_vector() const;
        void set_direction(const glm::vec3& direction);
        glm::vec3 get_direction() const;

        void set_origin(const glm::vec3& scene_origin, float lengths = 0.0f);
        void update_view_matrix();
        void set_projection(float far, float fov = 45.0f, float near = 1.0f);

        MVP& get_mvp(const glm::mat4& model_matrix) const;

        glm::vec3 get_intensity() const;
        void set_intensity(const glm::vec3& intensity);
        void set_cutoff_factor(float cutoff);
        float get_cutoff_factor() const;

    private:
        void set_vector(glm::vec3 vector);

        glm::vec3 point { 0.0f, 0.0f, 0.0f };
        float distance { 1.0f }; // for light

        mutable MVP transform;

        Type type;
        Buffer buffer;
    };

    struct Lights {
        LightSource::Buffer lights[16];
        int lights_enabled_count { 0 };
    };
}

#endif

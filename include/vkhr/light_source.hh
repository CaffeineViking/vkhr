#ifndef VKHR_LIGHT_SOURCE_HH
#define VKHR_LIGHT_SOURCE_HH

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

        LightSource(const glm::vec3& vector, Type type,
                    const glm::vec3& intensity,
                    float cutoff = 0.0);

        const Buffer& get_buffer() const;

        Type get_type() const;
        void set_type(Type light_source_type);

        glm::vec3 get_position() const;
        void set_position(const glm::vec3& position);

        const glm::vec4& get_vector() const;

        void set_direction(const glm::vec3& direction);
        glm::vec3 get_direction() const;

        glm::vec3 get_intensity() const;
        void set_intensity(const glm::vec3& intensity);

        void set_cutoff(float cutoff);
        float get_cutoff() const;

    private:
        void set_vector(const glm::vec3& vector);

        Type type;
        Buffer buffer;
    };
}

#endif

#include <vkhr/light_source.hh>

namespace vkhr {
    LightSource::LightSource(const glm::vec3& vector, Type type,
                             const glm::vec3& intensity,
                             float cutoff_value) {
        set_type(type);
        set_vector(vector);
        set_intensity(intensity);
        set_cutoff(cutoff_value);
    }

    const LightSource::Buffer& LightSource::get_buffer() const {
        return buffer;
    }

    LightSource::Type LightSource::get_type() const {
        return type;
    }

    void LightSource::set_type(Type light_source_type) {
        type = light_source_type;
        switch (type) {
        case Type::Point:
            buffer.vector[3] = 1.0;
            break;
        case Type::Directional:
            buffer.vector[3] = 0.0;
            break;
        default: break;
        }
    }

    glm::vec3 LightSource::get_position() const {
        return glm::vec3 { buffer.vector };
    }

    void LightSource::set_position(const glm::vec3& position) {
        set_vector(position);
        set_type(Type::Point);
    }

    const glm::vec4& LightSource::get_vector() const {
        return buffer.vector;
    }

    void LightSource::set_direction(const glm::vec3& direction) {
        set_vector(direction);
        set_type(Type::Directional);
    }

    glm::vec3 LightSource::get_direction() const {
        return glm::vec3 { buffer.vector };
    }

    glm::vec3 LightSource::get_intensity() const {
        return buffer.intensity;
    }

    void LightSource::set_intensity(const glm::vec3& intensity) {
        buffer.intensity[0] = intensity[0];
        buffer.intensity[1] = intensity[1];
        buffer.intensity[2] = intensity[2];
    }

    void LightSource::set_cutoff(float cutoff) {
        buffer.intensity[3] = cutoff;
    }

    float LightSource::get_cutoff() const {
        return buffer.intensity[3];
    }

    void LightSource::set_vector(const glm::vec3& vector) {
        buffer.vector[0] = vector[0];
        buffer.vector[1] = vector[1];
        buffer.vector[2] = vector[2];
    }
}

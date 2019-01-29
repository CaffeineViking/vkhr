#include <vkhr/scene_graph/light_source.hh>

namespace vkhr {
    LightSource::LightSource(const glm::vec3& vector, Type type,
                             const glm::vec3& intensity,
                             float cutoff_value) {
        set_type(type);
        set_vector(vector);
        set_intensity(intensity);
        set_cutoff_factor(cutoff_value);
        update_view_matrix();
    }

    const LightSource::Buffer& LightSource::get_buffer() const {
        return buffer;
    }

    LightSource::Type LightSource::get_type() const {
        return type;
    }

    const std::string& LightSource::get_type_name() const {
        switch (type) {
        case Type::Point:
            type_string = "Point";
            break;
        case Type::Directional:
            type_string = "Directional";
            break;
        default:
            type_string = "What?";
            break;
        }

        return type_string;
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
        set_type(Type::Point);
        set_vector(position);
        update_view_matrix();
    }

    void LightSource::set_projection(float far, float fov, float near) {
        projection = glm::perspective(glm::radians(fov), 1.0f,
                                      near, far);
        projection[1][1] *= -1;
        view_projection        = projection * view;
        buffer.view_projection = bias * view_projection;
        buffer.near = near;
        buffer.far = far;
    }

    const glm::vec4& LightSource::get_vector() const {
        return buffer.vector;
    }

    void LightSource::set_direction(const glm::vec3& direction) {
        set_type(Type::Directional);
        set_vector(direction);
        update_view_matrix();
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

    void LightSource::set_cutoff_factor(float cutoff) {
        buffer.intensity[3] = cutoff;
    }

    float LightSource::get_cutoff_factor() const {
        return buffer.intensity[3];
    }

    void LightSource::set_vector(glm::vec3 vector) {
        if (type == LightSource::Type::Directional)
            vector = glm::normalize(vector);

        buffer.vector[0] = vector[0];
        buffer.vector[1] = vector[1];
        buffer.vector[2] = vector[2];
    }

    void LightSource::set_origin(const glm::vec3& scene_origin, float distance) {
        point = scene_origin;
        if (type == Type::Point) {
            this->distance = 1.0f;
        } else {
            this->distance = distance;
        }
        update_view_matrix();
    }

    const glm::vec3& LightSource::get_spotlight_origin() const {
        return spotlight_origin;
    }

    void LightSource::update_view_matrix() {
        if (type == Type::Point) {
            view = glm::lookAt(get_position(), point, glm::vec3 { 0.0, 1.0, 0.0 });
            buffer.origin = get_position();
        } else {
            spotlight_origin = point + get_direction() * distance;
            view = glm::lookAt(spotlight_origin, point,
                               glm::vec3 { 0, 1, 0 });
            buffer.origin = spotlight_origin;
        }

        view_projection        = projection * view;
        buffer.view_projection = bias * view_projection;
    }

    const glm::mat4& LightSource::get_view_projection() const {
        return view_projection;
    }

    glm::mat4 LightSource::bias {
        0.5, 0.0, 0.0, 0.0,
        0.0, 0.5, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.5, 0.5, 0.0, 1.0
    };
}

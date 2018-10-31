#include <vkhr/camera.hh>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vkhr {
    Camera::Camera(const float field_of_view, const float aspect_ratio,
                   const float znear, const float zfar)
                  : near_distance { znear }, far_distance { zfar },
                    aspect_ratio { aspect_ratio },
                    field_of_view { field_of_view } {
        recalculate_projection_matrix();
        recalculate_view_matrix();
    }

    void Camera::rotate(const glm::vec3& around_axis, const float angle) {
        auto look_at_vector = look_at_point - position;
        glm::rotate(look_at_vector, angle, around_axis);
        set_look_at_point(position + look_at_vector);
    }

    void Camera::translate(const glm::vec3& translation) {
        set_position(this->position + translation);
    }

    void  Camera::set_aspect_ratio(const float aspect_ratio) {
        this->aspect_ratio = aspect_ratio;
        recalculate_projection_matrix();
    }

    float Camera::get_aspect_ratio() const {
        return aspect_ratio;
    }

    void  Camera::set_field_of_view(const float field_of_view) {
        this->field_of_view = field_of_view;
        recalculate_projection_matrix();
    }

    float Camera::get_field_of_view() const {
        return field_of_view;
    }

    const glm::vec3& Camera::get_position() const {
        return position;
    }

    void Camera::set_position(const glm::vec3& position) {
        this->position = position;
        recalculate_view_matrix();
    }

    void Camera::set_look_at_point(const glm::vec3& look_at_point) {
        this->look_at_point = look_at_point;
        recalculate_view_matrix();
    }

    const glm::vec3& Camera::get_look_at_point() const {
        return look_at_point;
    }

    void Camera::set_up_direction(const glm::vec3& up_direction) {
        this->up_direction = glm::normalize(up_direction);
        recalculate_view_matrix();
    }

    const glm::vec3& Camera::get_up_direction() const {
        return up_direction;
    }

    const glm::mat4& Camera::get_view_matrix() const {
        return view_matrix;
    }

    const glm::mat4& Camera::get_projection_matrix() const {
        return projection_matrix;
    }

    glm::vec3 Camera::get_left_direction() const {
        return glm::cross(get_forward_direction(),
                          get_up_direction());
    }

    glm::vec3 Camera::get_forward_direction() const {
        return glm::normalize(position - look_at_point);
    }

    void Camera::recalculate_projection_matrix() {
        projection_matrix = glm::perspective(field_of_view, aspect_ratio,
                                             near_distance, far_distance);
    }

    void Camera::recalculate_view_matrix() {
        view_matrix = glm::lookAt(position, look_at_point, up_direction);
    }
}

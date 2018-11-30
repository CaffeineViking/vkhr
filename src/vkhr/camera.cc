#include <vkhr/camera.hh>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

namespace vkhr {
    Camera::Camera(const float field_of_view,
                   const unsigned width, const unsigned height,
                   const float znear, const float zfar)
                  : near_distance { znear }, far_distance { zfar },
                    width { width }, height { height },
                    field_of_view { field_of_view } { }

    void Camera::rotate(const glm::vec3& around_axis, const float angle) {
        auto look_at_vector = look_at_point - position;
        look_at_vector = glm::rotate(look_at_vector, angle, around_axis);
        set_look_at_point(position + look_at_vector);
    }

    void Camera::translate(const glm::vec3& translation) {
        this->look_at_point += translation;
        this->position += translation;
        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    unsigned Camera::get_width() const {
        return width;
    }

    void Camera::set_width(unsigned width) {
        this->width = width;
        projection_matrix_dirty = true;
        viewing_plane_dirty     = true;
    }

    void Camera::set_height(unsigned height) {
        this->height = height;
        projection_matrix_dirty = true;
        viewing_plane_dirty     = true;
    }

    void Camera::set_resolution(unsigned width, unsigned height) {
        set_width(width); set_height(height);
    }

    unsigned Camera::get_height() const {
        return height;
    }

    float Camera::get_aspect_ratio() const {
        return static_cast<float>(width) /
               static_cast<float>(height);
    }

    void Camera::set_field_of_view(const float field_of_view) {
        this->field_of_view = field_of_view;
        projection_matrix_dirty = true;
        viewing_plane_dirty     = true;
    }

    float Camera::get_field_of_view() const {
        return field_of_view;
    }

    const glm::vec3& Camera::get_position() const {
        return position;
    }

    void Camera::set_position(const glm::vec3& position) {
        this->position = position;
        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    void Camera::set_look_at_point(const glm::vec3& look_at_point) {
        this->look_at_point = look_at_point;
        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    const glm::vec3& Camera::get_look_at_point() const {
        return look_at_point;
    }

    void Camera::set_up_direction(const glm::vec3& up_direction) {
        this->up_direction = glm::normalize(up_direction);
        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    const glm::vec3& Camera::get_up_direction() const {
        return up_direction;
    }

    void Camera::control(InputMap& input_map, const float delta_time, bool imgui_focused) {
        if (input_map.just_released("grab")) {
            input_map.unlock_cursor();
        } else if (!imgui_focused) {
            if (input_map.just_pressed("grab")) {
                input_map.freeze_cursor();
                last_mouse_position = input_map.get_mouse_position();
            } else if (input_map.pressed("grab")) {
                glm::vec2 cursor_movement { 0.0f, 0.0f };
                auto mouse_position = input_map.get_mouse_position();
                cursor_movement = mouse_position - last_mouse_position;
                last_mouse_position = mouse_position;
                cursor_movement *= delta_time * 0.2f;
                arcball_relative_to(cursor_movement);
            }
        }
    }

    void Camera::arcball_relative_to(const glm::vec2& cursor, const float scroll) {
        glm::vec2 cursor_delta { cursor };

        position = glm::rotate(position - look_at_point, -cursor_delta.x, +(get_up_direction())) + look_at_point;
        position = glm::rotate(position - look_at_point, -cursor_delta.y, -get_left_direction()) + look_at_point;

        up_direction = glm::rotate(up_direction, -cursor_delta.x, +(get_up_direction()));
        up_direction = glm::rotate(up_direction, -cursor_delta.y, -get_left_direction());

        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    void Camera::look_at(const glm::vec3& point, const glm::vec3& eye,
                         const glm::vec3& up) {
        this->position = eye;
        this->up_direction = glm::normalize(up);
        this->look_at_point = point;
        view_matrix_dirty   = true;
        viewing_plane_dirty = true;
    }

    const Camera::ViewingPlane& Camera::get_viewing_plane() const {
        if (viewing_plane_dirty)
            recalculate_viewing_plane();
        return viewing_plane;
    }

    const glm::mat4& Camera::get_view_matrix() const {
        if (view_matrix_dirty)
            recalculate_view_matrix();
        return view_matrix;
    }

    const glm::mat4& Camera::get_projection_matrix() const {
        if (projection_matrix_dirty)
            recalculate_projection_matrix();
        return projection_matrix;
    }

    MVP& Camera::get_mvp(const glm::mat4& mdl) const {
        mvp_matrix.view = get_view_matrix();
        mvp_matrix.projection = get_projection_matrix();
        mvp_matrix.model = mdl;
        return mvp_matrix;
    }

    glm::vec3 Camera::get_left_direction() const {
        return glm::cross(get_up_direction(),
                          get_forward_direction());
    }

    glm::vec3 Camera::get_forward_direction() const {
        return glm::normalize(look_at_point - position);
    }

    void Camera::recalculate_projection_matrix() const {
        float aspect_ratio = get_aspect_ratio();
        projection_matrix = glm::perspective(field_of_view, aspect_ratio,
                                             near_distance, far_distance);
        projection_matrix[1][1] *= -1; // Since Vulkan uses RHS coord-sys.
        projection_matrix_dirty = false;
    }

    void Camera::recalculate_view_matrix() const {
        view_matrix = glm::lookAt(position, look_at_point, up_direction);
        view_matrix_dirty = false;
    }

    void Camera::recalculate_viewing_plane() const {
        const float yfov_scale { 1.0f / std::tan(0.5f * field_of_view) };

        viewing_plane.z = get_forward_direction();
        viewing_plane.x = get_left_direction();
        viewing_plane.y = glm::cross(viewing_plane.x,
                                     viewing_plane.z);

        viewing_plane.point = position;

        viewing_plane.z = -0.5f * width  * viewing_plane.x +
                          -0.5f * height * viewing_plane.y +
                           0.5f * height * viewing_plane.z * yfov_scale;

        viewing_plane_dirty = false;
    }

    MVP Camera::Identity_Transform {
        glm::mat4 { 1.0f },
        glm::mat4 { 1.0f },
        glm::mat4 { 1.0f }
    };
}

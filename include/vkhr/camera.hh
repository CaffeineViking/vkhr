#ifndef VKHR_CAMERA_HH
#define VKHR_CAMERA_HH

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

namespace vkhr {
    class Camera final {
    public:
        Camera() = default;
        Camera(const float field_of_view, const float aspect_ratio,
               const float znear = 0.1, const float zfar = 100000);

        void rotate(const glm::vec3& around_axis, const float angle);
        void translate(const glm::vec3& translation);

        float get_aspect_ratio() const;
        void  set_aspect_ratio(const float aspect_ratio);
        void  set_field_of_view(const float field_of_view);
        float get_field_of_view() const;

        void set_move_speed(float speed);
        float get_move_speed() const;
        float get_look_speed() const;
        void set_look_speed(float speed);

        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);
        void set_look_at_point(const glm::vec3& look_at_point);
        const glm::vec3& get_look_at_point() const;
        void set_up_direction(const glm::vec3& up_direction);
        const glm::vec3& get_up_direction() const;

        void look_at(const glm::vec3& point, const glm::vec3& eye,
                     const glm::vec3& up = { 0.0f, 1.0f, 0.0f });

        glm::vec3 get_left_direction() const;
        glm::vec3 get_forward_direction() const;

        // TODO: convert to Embree-based camera.

        const glm::mat4& get_view_matrix() const;
        const glm::mat4& get_projection_matrix() const;

    private:
        void recalculate_projection_matrix();
        void recalculate_view_matrix();

        float move_speed { 4.0 };
        float look_speed { 0.2 };

        float near_distance { 0.1 };
        float far_distance { 1000 };

        float aspect_ratio { 16.0 / 9.0 };
        float field_of_view { glm::quarter_pi<float>() };

        glm::vec3 position      { 0.0, 0.0, 4.0 };
        glm::vec3 look_at_point { 0.0, 0.0, 0.0 };
        glm::vec3 up_direction  { 0.0, 1.0, 0.0 };

        glm::mat4 view_matrix;
        glm::mat4 projection_matrix;
    };
}

#endif

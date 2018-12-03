#ifndef VKHR_CAMERA_HH
#define VKHR_CAMERA_HH

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <vkhr/input_map.hh>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/glm.hpp>

namespace vkhr {
    struct MVP {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    };

    class Camera final {
    public:
        Camera() = default;
        Camera(const float field_of_view,
               const unsigned width, const unsigned height,
               const float znear = 0.01,
               const float zfar = 1000);

        void rotate(const glm::vec3& around_axis, const float angle);
        void translate(const glm::vec3& translation);

        unsigned get_width() const;
        void set_width(unsigned width);
        void set_height(unsigned height);
        unsigned get_height() const;

        void set_resolution(unsigned width, unsigned height);

        float get_aspect_ratio() const;
        void  set_field_of_view(const float field_of_view);
        float get_field_of_view() const;

        const glm::vec3& get_position() const;
        void set_position(const glm::vec3& position);
        void set_look_at_point(const glm::vec3& look_at_point);
        const glm::vec3& get_look_at_point() const;
        void set_up_direction(const glm::vec3& up_direction);
        const glm::vec3& get_up_direction() const;

        void control(InputMap& input_map, const float delta_time, const bool imgui_focused);
        void arcball_relative_to(const glm::vec2& mouse_movement, const float scroll = 0.0);

        void look_at(const glm::vec3& point, const glm::vec3& eye,
                     const glm::vec3& up = { 0.0f, 1.0f, 0.0f });

        glm::vec3 get_left_direction() const;
        glm::vec3 get_forward_direction() const;

        struct ViewingPlane {
            glm::vec3 x;
            glm::vec3 y;
            glm::vec3 z;
            glm::vec3 point;
        };

        const ViewingPlane& get_viewing_plane() const;

        const glm::mat4& get_view_matrix() const;
        const glm::mat4& get_projection_matrix() const;

        MVP& get_vp() const;

        MVP& get_mvp(const glm::mat4& model_mat) const;

        static MVP IdentityVPMatrix;

    private:
        void recalculate_view_matrix() const;
        void recalculate_projection_matrix() const;
        void recalculate_viewing_plane() const;

        float near_distance { .01 };
        float far_distance { 1000 };

        unsigned width { 1280 }, height { 720 };

        float field_of_view { glm::quarter_pi<float>() };

        glm::vec3 position      { 0.0, 0.0, 4.0 };
        glm::vec3 look_at_point { 0.0, 0.0, 0.0 };
        glm::vec3 up_direction  { 0.0, 1.0, 0.0 };

        glm::vec2 last_mouse_position;

        mutable ViewingPlane viewing_plane;

        mutable bool view_matrix_dirty { true };
        mutable bool projection_matrix_dirty { true };
        mutable bool viewing_plane_dirty { true };

        mutable MVP mvp_matrix;

        mutable glm::mat4 view_matrix;
        mutable glm::mat4 projection_matrix;
    };
}

#endif

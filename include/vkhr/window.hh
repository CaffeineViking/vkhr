#ifndef VKHR_WINDOW_HH
#define VKHR_WINDOW_HH

#include <vkhr/image.hh>

#include <vkpp/surface.hh>
#include <vkpp/extension.hh>
#include <vkpp/instance.hh>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>

namespace vkhr {
    class Window final {
    public:
        Window(const int width, const int height, const std::string& title,
               const Image& icon, const bool fullscreen = false,
               const bool vsync = false); // Will be Vulkan set.
        ~Window() noexcept;

        bool is_open() const;
        void close(); // kill

        bool is_fullscreen() const;

        void toggle_fullscreen();
        void hide(); void show();

        int get_width()  const;
        float get_aspect_ratio() const;
        int get_height() const;

        VkExtent2D get_extent() const;

        int get_refresh_rate() const;

        bool vsync_requested() const;
        void enable_vsync(bool sync);

        std::vector<vkpp::Extension> get_vulkan_surface_extensions() const;
        vkpp::Surface create_vulkan_surface_with(vkpp::Instance& instance);

        void resize(const int width, const int height);

        GLFWwindow* get_handle();

        void set_icon(const Image& icon);

        void change_title(const std::string& title);
        void append_string(const std::string& text) const;

        void poll_events(); // Called once per frame.

        float get_vertical_dpi()   const;
        float get_horizontal_dpi() const;

        void set_time(const float time);
        float get_current_time() const;
        float delta_time() const;

        float update_delta_time() const;

    private:
        int width, height;
        std::string title;
        bool fullscreen { false },
             vsync;

        GLFWwindow* handle { nullptr };

        GLFWmonitor* monitor;
        int window_x, window_y;
        int monitor_width, monitor_height;
        int monitor_refresh_rate;
        float horizontal_dpi,
              vertical_dpi;

        mutable std::string append;

        std::size_t frames { 0 };

        float frame_time { -1 },
              last_frame_time { 0 },
              fps_update { 0 };
    };
}

#endif

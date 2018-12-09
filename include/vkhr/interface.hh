#ifndef VKHR_INTERFACE_HH
#define VKHR_INTERFACE_HH

#include <vkhr/window.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/ray_tracer.hh>

#include <vkpp/instance.hh>
#include <vkpp/device.hh>
#include <vkpp/descriptor_set.hh>
#include <vkpp/render_pass.hh>
#include <vkpp/command_buffer.hh>
#include <vkpp/query.hh>

#include <vulkan/vulkan.h>

#include <glm/glm.hpp>

#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

namespace vkhr {
    class Rasterizer;
    class Interface final {
    public:
        Interface() = default;

        Interface(Window& w, Rasterizer* rasterizer);

        Interface(Interface&& interface) noexcept;
        Interface& operator=(Interface&& interface) noexcept;
        friend void swap(Interface& lhs, Interface& rhs);

        ~Interface() noexcept;

        void make_custom_style(ImGuiStyle&);

        void load(vkhr::Rasterizer& vulkan_renderer);
        void transform(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& raytracer);
        void draw(vkpp::CommandBuffer& command_list, vkpp::QueryPool& query_pool);
        void draw(vkpp::CommandBuffer& command_list);

        bool wants_focus() const;
        bool typing_text() const;

        bool hide();
        void toggle_visibility();
        void set_visibility(bool visible);
        bool raytracing_enabled();
        void toggle_raytracing();
        bool show();

        void store_shader_performance_timestamp(const std::unordered_map<std::string, float>& timestamps);

    private:
        int scene_file { 0 };
        int previous_scene_file { 0 };
        std::vector<std::string> scene_files;
        std::vector<std::string> renderers;
        std::vector<std::string> simulations;
        int simulation { 0 };
        int renderer { 0 };

        struct ProfilePair {
            std::vector<float> timestamps;
            int offset;
            std::string output { "??.??ms" };
        };

        static constexpr int profile_limit { 60 };
        std::unordered_map<std::string, ProfilePair> profiles;

        static bool get_string_from_vector(void*, int, const char**);

        bool raytrace_scene { false };
        ImGuiContext* ctx { nullptr };
        bool gui_visibility { false };
    };
}

#endif

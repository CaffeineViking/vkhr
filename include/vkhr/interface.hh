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

        void load(vkhr::Rasterizer& vulkan_renderer);
        void transform(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& raytracer);
        void draw(vkpp::CommandBuffer& command_list);

        bool wants_focus() const;
        bool typing_text() const;

        void hide();
        void toggle_visibility();
        bool raytracing_enabled();
        void toggle_raytracing();
        void show();

        void make_style(ImVec4*);

    private:
        int scene_file { 0 };
        std::vector<std::string> scene_files;
        std::vector<std::string> renderers;
        int renderer { 0 };

        static bool get_string_from_vector(void*, int, const char**);

        bool raytrace_scene { false };
        ImGuiContext* ctx { nullptr };
        bool gui_visibility { false };
    };
}

#endif

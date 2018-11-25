#ifndef VKHR_INTERFACE_HH
#define VKHR_INTERFACE_HH

#include <vkhr/window.hh>
#include <vkhr/scene_graph.hh>

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

        Interface(Window& w, Rasterizer& rasterizer);

        Interface(Interface&& interface) noexcept;
        Interface& operator=(Interface&& interface) noexcept;
        friend void swap(Interface& lhs, Interface& rhs);

        ~Interface() noexcept;

        void load(vkhr::Rasterizer& vulkan_renderer);
        void update(SceneGraph& scene_graph);
        void draw(vkpp::CommandBuffer& command_list);

        bool wants_focus() const;
        bool typing_text() const;

        void hide();
        void toggle_visibility();
        void toggle_raytracing();
        void show();

        bool do_raytrace() const;


    private:
        bool raytrace_scene { false };
        ImGuiContext* ctx { nullptr };
        bool gui_visibility { false };
    };
}

#endif

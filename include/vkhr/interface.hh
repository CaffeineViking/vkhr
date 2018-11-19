#ifndef VKHR_INTERFACE_HH
#define VKHR_INTERFACE_HH

#include <vkhr/window.hh>

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
    class Interface final {
    public:
        Interface() = default;

        Interface(vkhr::Window& window,
                  vkpp::Instance& instance,
                  vkpp::Device& device,
                  vkpp::DescriptorPool& descriptor_pool,
                  vkpp::RenderPass& render_pass,
                  vkpp::CommandPool& command_pool);

        ~Interface() noexcept;

        bool want_focus();

        void hide();
        void toggle_visibility();
        void show();

        void render(vkpp::CommandBuffer& command_buffer);

        void update();

    private:
        bool hidden { true };
    };
}

#endif

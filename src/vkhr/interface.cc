#include <vkhr/interface.hh>

#include <iostream>
#include <utility>

namespace vkhr {
    static void imgui_debug_callback(VkResult error) {
        if (error == 0) return; // VK_SUCCESS
        std::cerr << "ImGui error: " << error
                  << std::endl;
    }

    Interface::Interface(vkhr::Window& window,
                         vkpp::Instance& instance,
                         vkpp::Device& device,
                         vkpp::DescriptorPool& descriptor_pool,
                         vkpp::RenderPass& render_pass,
                         vkpp::CommandPool& command_pool) {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window.get_handle(), false);

        auto& physical_device = device.get_physical_device();

        ImGui_ImplVulkan_InitInfo init_info;

        init_info.Instance = instance.get_handle();
        init_info.PhysicalDevice = physical_device.get_handle();
        init_info.Device = device.get_handle();
        init_info.QueueFamily = physical_device.get_graphics_queue_family_index();
        init_info.Queue = device.get_graphics_queue().get_handle();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = descriptor_pool.get_handle();
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = imgui_debug_callback;

        ImGui_ImplVulkan_Init(&init_info, render_pass.get_handle());

        ImGui::StyleColorsDark();

        auto command_buffer = command_pool.allocate_and_begin();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get_handle());
        command_buffer.end();

        device.get_graphics_queue().submit(command_buffer)
                                   .wait_idle();
        ImGui_ImplVulkan_InvalidateFontUploadObjects();
    }

    void Interface::hide() {
        hidden = true;
    }

    void Interface::toggle_visibility(unsigned) {
        hidden = !hidden;
    }

    void Interface::show() {
        hidden = false;
    }

    Interface::~Interface() noexcept {
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void Interface::render(vkpp::CommandBuffer& command_buffer) {
        auto command_buffer_list { command_buffer.get_handle() };
        if (!hidden) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                            command_buffer_list);
        }
    }

    void Interface::update() {
        if (!hidden) {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();

            ImGui::Render();
        }
    }
}

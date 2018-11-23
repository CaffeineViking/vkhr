#include <vkhr/interface.hh>

#include <vkhr/window.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/rasterizer.hh>

#include <iostream>
#include <utility>

namespace vkhr {
    static void imgui_debug_callback(VkResult error) {
        if (error == 0) return; // VK_SUCCESS
        std::cerr << "ImGui error: " << error
                  << std::endl;
    }

    Interface::Interface(Window& window, Rasterizer& vulkan_renderer) {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();
        ImGui_ImplGlfw_InitForVulkan(window.get_handle(), false);
        ImGui::GetIO().IniFilename = nullptr;
        load(vulkan_renderer);
    }

    Interface::~Interface() noexcept {
        if (ctx != nullptr) {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext(ctx);
        }
    }

    void Interface::load(Rasterizer& vulkan_renderer) {
        ImGui_ImplVulkan_InitInfo init_info;

        init_info.Instance = vulkan_renderer.instance.get_handle();
        init_info.PhysicalDevice = vulkan_renderer.physical_device.get_handle();
        init_info.Device = vulkan_renderer.device.get_handle();
        init_info.QueueFamily = vulkan_renderer.physical_device.get_graphics_queue_family_index();
        init_info.Queue = vulkan_renderer.device.get_graphics_queue().get_handle();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPool = vulkan_renderer.descriptor_pool.get_handle();
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = imgui_debug_callback;

        ImGui_ImplVulkan_Init(&init_info, vulkan_renderer.render_pass.get_handle());

        ImGui::StyleColorsDark();

        auto command_buffer = vulkan_renderer.command_pool.allocate_and_begin();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get_handle());
        command_buffer.end();

        vulkan_renderer.device.get_graphics_queue().submit(command_buffer)
                                                   .wait_idle();
        ImGui_ImplVulkan_InvalidateFontUploadObjects();
    }

    void Interface::update(const SceneGraph& scene_graph) {
        if (!gui_visibility) {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::ShowDemoWindow();

            ImGui::Render();
        }
    }

    void Interface::draw(vkpp::CommandBuffer& cmdlst) {
        if (!gui_visibility) {
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                            cmdlst.get_handle());
        }
    }

    bool Interface::wants_focus() const {
        return ImGui::GetIO().WantCaptureMouse && !gui_visibility;
    }

    void Interface::hide() {
        gui_visibility = true;
    }

    void Interface::toggle_visibility() {
        gui_visibility = !gui_visibility;
    }

    void Interface::show() {
        gui_visibility = false;
    }

    Interface::Interface(Interface&& interface) noexcept {
        swap(*this, interface);
    }

    Interface& Interface::operator=(Interface&& interface) noexcept {
        swap(*this, interface);
        return *this;
    }

    void swap(Interface& lhs, Interface& rhs) {
        using std::swap;
        swap(lhs.ctx, rhs.ctx);
        swap(lhs.gui_visibility, rhs.gui_visibility);
    }
}

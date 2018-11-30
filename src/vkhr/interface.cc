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
        ImGui::GetIO().IniFilename = nullptr;
        ImGui_ImplGlfw_InitForVulkan(window.get_handle(), false);
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

        ImGui_ImplVulkan_Init(&init_info, vulkan_renderer.color_pass.get_handle());

        ImGui::StyleColorsDark();
        auto& style = ImGui::GetStyle();
        make_style(style.Colors);

        auto command_buffer = vulkan_renderer.command_pool.allocate_and_begin();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get_handle());
        command_buffer.end();

        vulkan_renderer.device.get_graphics_queue().submit(command_buffer)
                                                   .wait_idle();
        ImGui_ImplVulkan_InvalidateFontUploadObjects();

        scene_files.push_back("share/scenes/ponytail.vkhr");
        scene_file = 0;

        renderers.push_back("Rasterizer");
        renderers.push_back("Ray Tracer");
        renderer = 0;
    }

    void Interface::transform(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& raytracer) {
        if (!gui_visibility) {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            auto& window = rasterizer.window_surface.get_glfw_window();

            ImGui::Begin("VKHR - Scalable Strand-Based Hair Renderer.",
                         nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            if (ImGui::Button("Toggle Fullscreen"))
                window.toggle_fullscreen();

            ImGui::SameLine();

            if (ImGui::Button("Take Screenshot"))
                rasterizer.get_screenshot()
                          .save("out.png");

            ImGui::SameLine();

            if (ImGui::Button("Hide UI"))
                toggle_visibility();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Combo("", &renderer, get_string_from_vector, static_cast<void*>(&renderers), renderers.size());

            if (renderer == 0) raytrace_scene = 0;
            else               raytrace_scene = 1;

            ImGui::SameLine(0.0, 4.0);

            if (ImGui::Button("Toggle Render"))
                toggle_raytracing();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Render Settings")) {
                if (ImGui::TreeNode("Rasterizer")) {
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Ray Tracer")) {
                    ImGui::TreePop();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Combo("Rendered Scene", &scene_file, get_string_from_vector, static_cast<void*>(&scene_files), scene_files.size());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Scene Hierarchy")) {
                if (ImGui::TreeNode("Camera")) {
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Light Sources")) {
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Node")) {
                    ImGui::TreePop();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Recompile Shaders"))
                rasterizer.recompile();

            ImGui::SameLine(0.0, 9.0);

            ImGui::Button("Toggle Shadow Map Viewer");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Shader Profiler")) {
            }

            ImGui::End();

            ImGui::Render();
        }
    }

    void Interface::draw(vkpp::CommandBuffer& command_buffer) {
        if (!gui_visibility) {
            vk::DebugMarker::begin(command_buffer, "Render Interface");
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                            command_buffer.get_handle());
            vk::DebugMarker::end(command_buffer);
        }
    }

    bool Interface::wants_focus() const {
        return ImGui::GetIO().WantCaptureMouse && !gui_visibility;
    }

    bool Interface::typing_text() const {
        return ImGui::GetIO().WantCaptureKeyboard && !gui_visibility;
    }

    void Interface::hide() {
        gui_visibility = true;
    }

    void Interface::toggle_visibility() {
        gui_visibility = !gui_visibility;
    }

    void Interface::toggle_raytracing() {
        raytrace_scene = !raytrace_scene;
        renderer       =  raytrace_scene;
    }

    void Interface::show() {
        gui_visibility = false;
    }

    bool Interface::raytracing_enabled() {
        return raytrace_scene;
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
        swap(lhs.raytrace_scene, rhs.raytrace_scene);

        swap(lhs.renderer, rhs.renderer);
        swap(lhs.scene_file, rhs.scene_file);
        swap(lhs.scene_files, rhs.scene_files);
        swap(lhs.renderers, rhs.renderers);
    }

    void Interface::make_style(ImVec4* color) {
    }

    bool Interface::get_string_from_vector(void* data, int n, const char** str) {
        std::vector<std::string>* v = (std::vector<std::string>*) data;
        *str = v->at(n).c_str();
        return true;
    }
}

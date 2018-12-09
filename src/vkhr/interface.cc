#include <vkhr/interface.hh>

#include <vkhr/window.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/rasterizer.hh>

#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sstream>

#include <iostream>
#include <utility>

namespace vkhr {
    static void imgui_debug_callback(VkResult error) {
        if (error == 0) return; // VK_SUCCESS
        std::cerr << "ImGui error: " << error
                  << std::endl;
    }

    Interface::Interface(Window& window, Rasterizer* vulkan_renderer) {
        IMGUI_CHECKVERSION();
        ctx = ImGui::CreateContext();
        ImGui::GetIO().IniFilename = nullptr;
        ImGui_ImplGlfw_InitForVulkan(window.get_handle(), false);
        load(*vulkan_renderer);
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
        make_custom_style(style);

        auto command_buffer = vulkan_renderer.command_pool.allocate_and_begin();
        ImGui_ImplVulkan_CreateFontsTexture(command_buffer.get_handle());
        command_buffer.end();

        vulkan_renderer.device.get_graphics_queue().submit(command_buffer)
                                                   .wait_idle();
        ImGui_ImplVulkan_InvalidateFontUploadObjects();

        renderers.push_back("Rasterizer");
        renderers.push_back("Ray Tracer");

        scene_files.push_back(SCENE("ponytail.vkhr"));

        simulations.push_back("No Effects");

        renderer = scene_file = simulation = 0;
    }

    void Interface::transform(SceneGraph& scene_graph, Rasterizer& rasterizer, Raytracer& ray_tracer) {
        auto direction = scene_graph.light_sources.front().get_direction();
        direction = glm::rotateY(direction, 0.005f);
        direction = glm::rotateX(direction, 0.008f);
        direction = glm::rotateZ(direction, 0.002f);
        scene_graph.light_sources.front().set_direction(direction);

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (gui_visible) {
            auto& window = rasterizer.window_surface.get_glfw_window();

            ImGui::Begin(" VKHR - a Scalable Strand-Based Hair Renderer",
                         &gui_visible, ImGuiWindowFlags_AlwaysAutoResize |
                                       ImGuiWindowFlags_NoCollapse);

            // ImGui::TextWrapped("");
            // ImGui::Spacing();

            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Checkbox("Fullscreen", &window.fullscreen);

            ImGui::SameLine(0.0f, 10.0f);

            ImGui::Checkbox("VSync", &window.vsync);

            ImGui::SameLine(0.0f, 10.0f);

            if (ImGui::Button("Take Screenshot"))
                rasterizer.get_screenshot(scene_graph, ray_tracer)
                          .save_time();

            ImGui::SameLine();

            ImGui::Button("Help");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Combo("", &renderer, get_string_from_vector, static_cast<void*>(&renderers), renderers.size());

            if (renderer == 0) raytrace_scene = 0;
            else               raytrace_scene = 1;

            ImGui::SameLine(0.0, 4.0);

            if (ImGui::Button("Toggle Renderer"))
                toggle_raytracing();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Render Settings")) {
                if (ImGui::TreeNode("Rasterizer")) {
                    ImGui::Checkbox("Shadows", &rasterizer.shadows_on);
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Ray Tracer")) {
                    ImGui::Checkbox("Shadows", &ray_tracer.shadows_on);
                    ImGui::TreePop();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Combo("Scene Graph File", &scene_file, get_string_from_vector, static_cast<void*>(&scene_files), scene_files.size());

            // Switch to the new scene by loading it
            if (scene_file != previous_scene_file) {
                scene_graph.load(scene_files[scene_file]);
                rasterizer.load(scene_graph);
                previous_scene_file = scene_file;
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Scene Hierarchy")) {
                if (ImGui::TreeNode("Camera")) {
                    if (ImGui::SliderAngle("Field of View", &scene_graph.camera.field_of_view, 0.0f, 180.0f))
                        scene_graph.camera.recalculate_projection_matrix();
                    if (ImGui::SliderFloat("Screen Aspect", &scene_graph.camera.aspect_ratio, 1.00f, 2.370f, "%.4f"))
                        scene_graph.camera.recalculate_projection_matrix();
                    if (ImGui::DragFloat3("View Position", glm::value_ptr(scene_graph.camera.position)))
                        scene_graph.camera.recalculate_view_matrix();
                    if (ImGui::DragFloat3("Look at Point", glm::value_ptr(scene_graph.camera.look_at_point)))
                        scene_graph.camera.recalculate_view_matrix();

                    ImGui::TreePop();
                }

                if (ImGui::TreeNode("Lights")) {
                    ImGui::TreePop();
                }

                if (ImGui::TreeNode(scene_graph.get_root_node().node_name.c_str())) {
                    ImGui::TreePop();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::Button("Recompile Shaders"))
                rasterizer.recompile();

            ImGui::SameLine();

            ImGui::Checkbox("Display Sources of Light", &light_debugger);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Combo("Animation Effect", &simulation, get_string_from_vector, static_cast<void*>(&simulations), simulations.size());

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (ImGui::CollapsingHeader("Shader Profiler")) {
                for (auto& profile : profiles)
                    ImGui::PlotLines(profile.first.c_str(),
                                     profile.second.timestamps.data(),
                                     profile.second.timestamps.size(),
                                     profile.second.offset,
                                     profile.second.output.c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();

            // ImGui::Spacing();
            // ImGui::TextWrapped("");

            ImGui::End();
        }

        ImGui::Render();
    }

    void Interface::draw(vkpp::CommandBuffer& command_buffer, vkpp::QueryPool& query_pool) {
        if (gui_visible) {
            vk::DebugMarker::begin(command_buffer, "Draw GUI Overlay", query_pool);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                            command_buffer.get_handle());
            vk::DebugMarker::close(command_buffer, "Draw GUI Overlay", query_pool);
        }
    }

    void Interface::draw(vkpp::CommandBuffer& command_buffer) {
        if (gui_visible) {
            vk::DebugMarker::begin(command_buffer, "Draw GUI Overlay");
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                            command_buffer.get_handle());
            vk::DebugMarker::close(command_buffer);
        }
    }

    bool Interface::wants_focus() const {
        return ImGui::GetIO().WantCaptureMouse    && gui_visible;
    }

    bool Interface::typing_text() const {
        return ImGui::GetIO().WantCaptureKeyboard && gui_visible;
    }

    void Interface::set_visibility(bool visible) {
        gui_visible = visible;
    }

    bool Interface::hide() {
        auto previous_visibility = gui_visible;
        gui_visible = false;
        return previous_visibility;
    }

    void Interface::toggle_visibility() {
        gui_visible = !gui_visible;
    }

    void Interface::toggle_raytracing() {
        raytrace_scene = !raytrace_scene;
        renderer       =  raytrace_scene;
    }

    bool Interface::show() {
        auto previous_visibility = gui_visible;
        gui_visible = true;
        return previous_visibility;
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
        swap(lhs.gui_visible, rhs.gui_visible);
        swap(lhs.raytrace_scene, rhs.raytrace_scene);

        swap(lhs.renderer, rhs.renderer);
        swap(lhs.scene_file, rhs.scene_file);
        swap(lhs.scene_files, rhs.scene_files);
        swap(lhs.renderers, rhs.renderers);
        swap(lhs.simulations, rhs.simulations);
        swap(lhs.simulation, rhs.simulation);
        swap(lhs.profiles, rhs.profiles);

        swap(lhs.light_debugger, rhs.light_debugger);
    }

    void Interface::make_custom_style(ImGuiStyle& style) {
        style.FrameRounding  = 2.0f;
        style.GrabRounding   = 2.0f;
    }

    bool Interface::get_string_from_vector(void* data, int n, const char** str) {
        std::vector<std::string>* v = (std::vector<std::string>*) data;
        *str = v->at(n).c_str();
        return true;
    }

    void Interface::store_shader_performance_timestamp(const std::unordered_map<std::string, float>& timestamps) {
        for (const auto& timestamp : timestamps) {
            auto profile = profiles.find(timestamp.first);
            if (profile == profiles.end()) {
                profiles[timestamp.first] = ProfilePair { {}, 0 };
                profile = profiles.find(timestamp.first);
                profile->second.timestamps.resize(profile_limit);
                std::fill(profile->second.timestamps.begin(),
                          profile->second.timestamps.end(), 0.0f);
            }

            profile->second.timestamps[profile->second.offset] = timestamp.second;

            if (profile->second.offset == (profile_limit - 1)) {
                auto sum = std::accumulate(profile->second.timestamps.begin(),
                                           profile->second.timestamps.end(), 0.0f);
                auto average = sum / profile->second.timestamps.size();
                profile->second.output.resize(10);
                std::snprintf(&profile->second.output[0], 10, "%5.2fms", average);
            }

            profile->second.offset = (profile->second.offset + 1) % profile_limit;
        }
    }
}

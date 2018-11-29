#include <vkhr/rasterizer.hh>

namespace vkhr {
    Rasterizer::Rasterizer(Window& window, const SceneGraph& scene_graph) {
        vk::Version target_vulkan_loader { 1,1 };
        vk::Application application_information {
            "VKHR", { 1, 0, 0 },
            "None", { 0, 0, 0 },
            target_vulkan_loader
        };

        std::vector<vk::Layer> required_layers {
        #ifdef DEBUG
            "VK_LAYER_LUNARG_standard_validation"
        #endif
        };

        std::vector<vk::Extension> required_extensions {
        #ifdef DEBUG
            "VK_EXT_debug_utils"
        #endif
        };

        vk::append(window.get_vulkan_surface_extensions(),
                   required_extensions); // VK_surface_KHR

        instance = vk::Instance {
            application_information,
            required_layers,
            required_extensions
        };

        window_surface = window.create_vulkan_surface_with(instance);

        // Find physical devices that seem most promising of the lot.
        auto score = [&](const vk::PhysicalDevice& physical_device) {
            short gpu_suitable = 2*physical_device.is_discrete_gpu()+
                                 physical_device.is_integrated_gpu();
            return physical_device.has_every_queue() * gpu_suitable *
                   physical_device.has_present_queue(window_surface);
        };

        physical_device = instance.find_physical_devices_with(score);
        window.append_string(physical_device.get_name()); // our GPU.
        physical_device.assign_present_queue_indices(window_surface);

        std::vector<vk::Extension> device_extensions {
            "VK_KHR_swapchain"
        };

        // Just enable every device feature we have right now.
        auto device_features = physical_device.get_features();

        device = vk::Device {
            physical_device,
            required_layers,
            device_extensions,
            device_features
        };

        instance.label(device.get_handle());

        command_pool = vk::CommandPool { device, device.get_graphics_queue() };

        auto vsync = window.vsync_requested();

        auto presentation_mode = vsync ? vk::SwapChain::PresentationMode::Fifo
                                   : vk::SwapChain::PresentationMode::MailBox;

        swap_chain = vk::SwapChain {
            device,
            window_surface,
            command_pool,
            {
                VK_FORMAT_B8G8R8A8_UNORM,
                VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
            },
            presentation_mode,
            window.get_extent()
        };

        window_surface.label(device.get_handle());

        descriptor_pool = vkpp::DescriptorPool {
            device,
            {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 }
            }
        };

        build_render_passes();

        framebuffers = swap_chain.create_framebuffers(color_pass);

        image_available = vk::Semaphore { device };
        render_complete = vk::Semaphore { device };
        command_buffer_done = vk::Fence { device };

        vk::DebugMarker::object_name(device, image_available, VK_OBJECT_TYPE_SEMAPHORE, "Image Available Semaphore");
        vk::DebugMarker::object_name(device, render_complete, VK_OBJECT_TYPE_SEMAPHORE, "Render Complete Semaphore");
        vk::DebugMarker::object_name(device, command_buffer_done, VK_OBJECT_TYPE_FENCE, "Command Buffer Done Fence");

        build_pipelines();

        load(scene_graph);

        command_buffers = command_pool.allocate(framebuffers.size());
    }

    void Rasterizer::load(const SceneGraph& scene_graph) {
        imgui = Interface { window_surface.get_glfw_window(), *this };

        for (const auto& hair_style : scene_graph.get_hair_styles()) {
            const auto& hair_style_geometry = hair_style.second;
            hair_styles[&hair_style_geometry] = vulkan::HairStyle {
                hair_style_geometry,
                *this,
                hair_style_pipeline
            };
        }

        auto& camera = scene_graph.get_camera();

        raytraced_image = vulkan::Billboard {
            camera.get_width(),
            camera.get_height(),
            *this,
            billboards_pipeline
        };
    }

    void Rasterizer::draw(const SceneGraph& scene_graph) {
        auto next_image = swap_chain.acquire_next_image(image_available);

        auto& cam = scene_graph.get_camera();

        command_buffer_done.wait_and_reset();

        for (std::size_t i { 0 }; i < swap_chain.size(); ++i) {
            command_buffers[i].begin();
            vk::DebugMarker::begin(command_buffers[i], "Render Scene Graph");
            command_buffers[i].begin_render_pass(color_pass, framebuffers[i],
                                                 { 1.0f, 1.0f, 1.0f, 1.0f });

            vk::DebugMarker::begin(command_buffers[i], "Render Hair Styles");
            for (auto& hair_node : scene_graph.get_nodes_with_hair_styles()) {
                auto& model_view_projection = cam.get_mvp(hair_node->get_m());
                draw(hair_node, command_buffers[i], i, model_view_projection);
            } vk::DebugMarker::end(command_buffers[i]);

            imgui.draw(command_buffers[i]);

            command_buffers[i].end_render_pass();
            vk::DebugMarker::end(command_buffers[i]);
            command_buffers[i].end();
        }

        device.get_graphics_queue().submit(command_buffers[next_image], image_available,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete, command_buffer_done);

        device.get_present_queue().present(swap_chain, next_image, render_complete);
    }

    void Rasterizer::draw(const SceneGraph::Node* node,
                          vk::CommandBuffer& cmd_lists,
                          std::size_t fb, MVP& mvp_mat) {
        for (auto& hair_style : node->get_hair_styles()) {
            hair_styles[hair_style].update(mvp_mat, fb);
            hair_styles[hair_style].draw(cmd_lists, fb);
        }
    }

    void Rasterizer::draw(Image& raytrace_framebuffer) {
        auto next_image = swap_chain.acquire_next_image(image_available);

        command_buffer_done.wait_and_reset();

        for (std::size_t i { 0 }; i < swap_chain.size(); ++i) {
            command_buffers[i].begin();
            raytraced_image.update(raytrace_framebuffer, command_buffers[i]);
            vk::DebugMarker::begin(command_buffers[i], "Render Framebuffer");
            command_buffers[i].begin_render_pass(color_pass, framebuffers[i],
                                                 { 1.0f, 1.0f, 1.0f, 1.0f });

            raytraced_image.update(Camera::Identity, i);
            raytraced_image.draw(command_buffers[i], i);

            imgui.draw(command_buffers[i]);

            command_buffers[i].end_render_pass();
            vk::DebugMarker::end(command_buffers[i]);
            command_buffers[i].end();
        }

        device.get_graphics_queue().submit(command_buffers[next_image], image_available,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete, command_buffer_done);

        device.get_present_queue().present(swap_chain, next_image, render_complete);
    }

    void Rasterizer::build_pipelines() {
        vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);
        vulkan::Billboard::build_pipeline(billboards_pipeline, *this);
    }

    void Rasterizer::build_render_passes() {
        vk::RenderPass::mk_color_pass(color_pass, device, swap_chain);
        vk::RenderPass::mk_depth_pass(depth_pass, device, shadow_map);
    }

    void Rasterizer::recreate_swapchain(Window& window) {
    }

    Interface& Rasterizer::get_imgui() {
        return imgui;
    }

    Image Rasterizer::get_screenshot() {
        return Image { swap_chain.get_width(),
                       swap_chain.get_height() };
    }

    void Rasterizer::recompile_spirv() {
        device.wait_idle(); // Flush it.

        bool hair_style_pipeline_dirty { false };
        for (auto& hair_style_shader : hair_style_pipeline.shader_stages)
            hair_style_pipeline_dirty |= hair_style_shader.recompile();
        if (hair_style_pipeline_dirty)
            vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);

        bool billboards_pipeline_dirty  { false };
        for (auto& billboard_shader : billboards_pipeline.shader_stages)
            billboards_pipeline_dirty |= billboard_shader.recompile();
        if (billboards_pipeline_dirty)
            vulkan::Billboard::build_pipeline(billboards_pipeline, *this);
    }
}

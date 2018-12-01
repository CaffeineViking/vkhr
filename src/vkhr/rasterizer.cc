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

        image_available = vk::Semaphore::create(device, swap_chain.size(), "Image Available Semaphore");
        render_complete = vk::Semaphore::create(device, swap_chain.size(), "Render Complete Semaphore");
        command_buffer_done = vk::Fence::create(device, swap_chain.size(), "Command Buffer Done Fence");
        light_data = vk::UniformBuffer::create(device, sizeof(Lights), swap_chain.size(), "Light Data");
        transform = vk::UniformBuffer::create(device, sizeof(MVP), swap_chain.size(), "Transform Data");

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
                *this
            };
        }

        shadow_map = vulkan::DepthView {
            1024, 1024,
            1,
            *this
        };

        raytraced_image = vulkan::Billboard {
            scene_graph.get_camera().get_width(),
            scene_graph.get_camera().get_height(),
            *this
        };
    }

    std::uint32_t Rasterizer::fetch_next_frame() {
        return (frame + 1) % swap_chain.size();
    }

    void Rasterizer::draw(const SceneGraph& scene_graph) {
        auto next_image = swap_chain.acquire_next_image(image_available[frame]);

        light_data[frame].update(scene_graph.light);
        command_buffer_done[frame].wait_and_reset();

        command_buffers[frame].begin();
        vk::DebugMarker::begin(command_buffers[frame], "Render into Shadow Map");
        vk::DebugMarker::begin(command_buffers[frame], "Render the Scene Graph");
        command_buffers[frame].begin_render_pass(color_pass, framebuffers[frame],
                                                 { 1.00f, 1.00f, 1.00f, 1.00f });

        auto camera_node = scene_graph.get_camera();

        vk::DebugMarker::begin(command_buffers[frame], "Render the Hair Styles");
        hair_style_pipeline.make_current_pipeline(command_buffers[frame], frame);
        for (auto& hair_styles_node : scene_graph.get_nodes_with_hair_styles()) {
            auto& mvp = camera_node.get_mvp(hair_styles_node->get_matrix());
            draw_hair(hair_styles_node, command_buffers[frame], frame, mvp);
        } vk::DebugMarker::end(command_buffers[frame]);

        imgui.draw(command_buffers[frame]);

        command_buffers[frame].end_render_pass();
        vk::DebugMarker::end(command_buffers[frame]);
        command_buffers[frame].end();

        device.get_graphics_queue().submit(command_buffers[frame], image_available[frame],
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete[frame], command_buffer_done[frame]);
        device.get_present_queue().present(swap_chain, next_image, render_complete[frame]);
        frame = fetch_next_frame();
    }

    void Rasterizer::draw_hair(const SceneGraph::Node* hair_node,
                               vk::CommandBuffer& command_buffer,
                               std::size_t frame, MVP& transform) {
        this->transform[frame].update(transform); // MVP stuff.
        for (auto& hair_style : hair_node->get_hair_styles()) {
            hair_styles[hair_style].draw(command_buffer, frame);
        }
    }

    void Rasterizer::draw(Image& image) {
        auto next_image = swap_chain.acquire_next_image(image_available[frame]);

        command_buffer_done[frame].wait_and_reset();

        command_buffers[frame].begin();
        raytraced_image.update(billboards_pipeline.descriptor_sets[frame], image,
                               command_buffers[frame]); // Staged image upload...
        vk::DebugMarker::begin(command_buffers[frame], "Render the Framebuffer");
        billboards_pipeline.make_current_pipeline(command_buffers[frame], frame);
        command_buffers[frame].begin_render_pass(color_pass, framebuffers[frame],
                                                 { 1.00f, 1.00f, 1.00f, 1.00f });

        transform[frame].update(Camera::Identity_Transform);
        raytraced_image.draw(command_buffers[frame], frame);

        imgui.draw(command_buffers[frame]);

        command_buffers[frame].end_render_pass();
        vk::DebugMarker::end(command_buffers[frame]);
        command_buffers[frame].end();

        device.get_graphics_queue().submit(command_buffers[next_image], image_available[frame],
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete[frame], command_buffer_done[frame]);
        device.get_present_queue().present(swap_chain, next_image, render_complete[frame]);
        frame = fetch_next_frame();
    }

    void Rasterizer::build_pipelines() {
        vulkan::DepthView::build_pipeline(depth_view_pipeline, *this);
        vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);
        vulkan::Billboard::build_pipeline(billboards_pipeline, *this);
    }

    void Rasterizer::build_render_passes() {
        vk::RenderPass::mk_color_pass(color_pass, device, swap_chain);
        vk::RenderPass::mk_depth_pass(depth_pass, device, shadow_map);
    }

    void Rasterizer::recreate_swapchain(Window&) {
    }

    Interface& Rasterizer::get_imgui() {
        return imgui;
    }

    Image Rasterizer::get_screenshot() {
        return Image { swap_chain.get_width(),
                       swap_chain.get_height() };
    }

    void Rasterizer::recompile() {
        device.wait_idle(); // GPU

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

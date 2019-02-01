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

        command_pool = vk::CommandPool { device, device.get_graphics_queue() };

        auto vsync = window.vsync_requested();

        auto presentation_mode = vsync ? vk::SwapChain::PresentationMode::Fifo
                                 : vk::SwapChain::PresentationMode::Immediate;

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

        depth_sampler = vk::Sampler {
            device, // for sampling depth buffer.
            VK_FILTER_LINEAR,    VK_FILTER_LINEAR,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
        };

        descriptor_pool = vkpp::DescriptorPool {
            device,
            {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         64 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 64 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         64 },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,       64 }
            }
        };

        build_render_passes();

        framebuffers = swap_chain.create_framebuffers(color_pass);

        image_available = vk::Semaphore::create(device, swap_chain.size(), "Image Available Semaphore");
        render_complete = vk::Semaphore::create(device, swap_chain.size(), "Render Complete Semaphore");
        command_buffer_finished = vk::Fence::create(device, swap_chain.size(), "Buffer Finished Fence");

        camera = vk::UniformBuffer::create(device, sizeof(vkhr::ViewProjection),  swap_chain.size(), "Camera Matrix Data");
        params = vk::UniformBuffer::create(device, sizeof(Interface::Parameters), swap_chain.size(), "Rendering Settings");

        load(scene_graph);

        fullscreen_billboard = vulkan::Billboard {
            swap_chain.get_width(),
            swap_chain.get_height(),
            *this
        };

        imgui = Interface { window_surface.get_glfw_window(), this };

        query_pools = vk::QueryPool::create(framebuffers.size(), device, VK_QUERY_TYPE_TIMESTAMP, 128);

        command_buffers = command_pool.allocate(framebuffers.size());
    }

    void Rasterizer::load(const SceneGraph& scene_graph) {
        device.wait_idle();

        hair_styles.clear();
        models.clear();
        shadow_maps.clear();

        for (const auto& model : scene_graph.get_models())
            models[&model.second] = vulkan::Model {
                model.second, *this
            };

        for (const auto& hair_style : scene_graph.get_hair_styles())
            hair_styles[&hair_style.second] = vulkan::HairStyle {
                hair_style.second, *this
            };

        lights = vk::UniformBuffer::create(device, scene_graph.get_light_sources().size() * sizeof(LightSource::Buffer),
                                           swap_chain.size(), "Light Source Buffer Data"); // e.g.: position, intensity.

        for (auto& light_source : scene_graph.get_light_sources())
            shadow_maps.emplace_back(1024, *this, light_source);

        build_pipelines();
    }

    void Rasterizer::update(const SceneGraph& scene_graph) {
        camera[frame].update(scene_graph.get_camera().get_transform());
        lights[frame].update(scene_graph.fetch_light_source_buffers());
        params[frame].update(imgui.parameters); // Rendering parameter.
    }

    void Rasterizer::draw(const SceneGraph& scene_graph) {
        command_buffer_finished[frame].wait_and_reset();
        imgui.record_performance(query_pools[frame].request_timestamp_queries());
        update(scene_graph); // updates descriptor sets.

        auto frame_image = swap_chain.acquire_next_image(image_available[frame]);

        command_buffers[frame].begin();

        command_buffers[frame].reset_query_pool(query_pools[frame], 0, // performance.
                                                query_pools[frame].get_query_count());

        vk::DebugMarker::begin(command_buffers[frame], "Total Frame Time", query_pools[frame]);

        if (!imgui.raymarcher_enabled()) {
            draw_depth(scene_graph, command_buffers[frame]);
        }

        voxelize(scene_graph,   command_buffers[frame]);

        draw_color(scene_graph, command_buffers[frame]);

        vk::DebugMarker::close(command_buffers[frame], "Total Frame Time", query_pools[frame]);

        command_buffers[frame].end();

        device.get_graphics_queue().submit(command_buffers[frame], image_available[frame],
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete[frame], command_buffer_finished[frame]);
        device.get_present_queue().present(swap_chain, frame_image, render_complete[frame]);

        latest_drawn_frame = frame;
        frame = fetch_next_frame();
    }

    std::uint32_t Rasterizer::fetch_next_frame() {
        return (frame + 1) % swap_chain.size();
    }

    void Rasterizer::voxelize(const SceneGraph& scene_graph, vk::CommandBuffer& command_buffer) {
        vk::DebugMarker::begin(command_buffers[frame], "Voxelize Strands", query_pools[frame]);

        command_buffer.bind_pipeline(hair_voxel_pipeline);

        for (auto& hair_node : scene_graph.get_nodes_with_hair_styles()) {
            for (auto& hair : hair_node->get_hair_styles()) {
                hair_styles[hair].voxelize(hair_voxel_pipeline,
                                           hair_voxel_pipeline.descriptor_sets[frame],
                                           command_buffer);
            }
        }

        vk::DebugMarker::close(command_buffers[frame], "Voxelize Strands", query_pools[frame]);
    }

    void Rasterizer::draw_color(const SceneGraph& scene_graph, vk::CommandBuffer& command_buffer) {
        vk::DebugMarker::begin(command_buffers[frame], "Color Pass");
        command_buffers[frame].begin_render_pass(color_pass, framebuffers[frame],
                                                 { 1.00f, 1.00f, 1.00f, 1.00f });

        vk::DebugMarker::begin(command_buffers[frame], "Draw Mesh Models", query_pools[frame]);
        draw_model(scene_graph, model_mesh_pipeline, command_buffers[frame]);
        vk::DebugMarker::close(command_buffers[frame], "Draw Mesh Models", query_pools[frame]);

        if (!imgui.raymarcher_enabled()) {
            vk::DebugMarker::begin(command_buffers[frame], "Draw Hair Styles", query_pools[frame]);
            draw_hairs(scene_graph, hair_style_pipeline, command_buffers[frame]);
            vk::DebugMarker::close(command_buffers[frame], "Draw Hair Styles", query_pools[frame]);
        }

        command_buffers[frame].next_subpass(); // Next sub-pass which uses depth buffer values.

        if (imgui.raymarcher_enabled()) {
            vk::DebugMarker::begin(command_buffers[frame], "Raymarch Strands", query_pools[frame]);
            strand_dvr(scene_graph, strand_dvr_pipeline, command_buffers[frame]);
            vk::DebugMarker::close(command_buffers[frame], "Raymarch Strands", query_pools[frame]);
        }

        vk::DebugMarker::begin(command_buffers[frame], "Draw GUI Overlay", query_pools[frame]);
        imgui.draw(command_buffers[frame]);
        vk::DebugMarker::close(command_buffers[frame], "Draw GUI Overlay", query_pools[frame]);

        command_buffers[frame].end_render_pass();
        vk::DebugMarker::close(command_buffers[frame]);
    }

    void Rasterizer::draw_model(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer, glm::mat4 projection) {
        command_buffer.bind_pipeline(pipeline); // Color / Depth Pass.
        for (auto& model_node : scene_graph.get_nodes_with_models()) {
            command_buffer.push_constant(pipeline, 0, projection * model_node->get_model_matrix());
            for (auto& model_mesh : model_node->get_models())
                models[model_mesh].draw(pipeline, pipeline.descriptor_sets[frame], command_buffer);
        }
    }

    void Rasterizer::draw_depth(const SceneGraph& scene_graph, vk::CommandBuffer& command_buffer) {
        vk::DebugMarker::begin(command_buffers[frame], "Depth Pass");

        if (!imgui.parameters.adsm_on && !imgui.parameters.ctsm_on) {
            vk::DebugMarker::close(command_buffers[frame]);
            return;
        }

        vk::DebugMarker::begin(command_buffers[frame], "Bake Shadow Maps", query_pools[frame]);
        for (auto& shadow_map : shadow_maps) {
            auto& vp = shadow_map.light->get_view_projection();
            command_buffer.begin_render_pass(depth_pass, shadow_map);
            shadow_map.update_dynamic_viewport_scissor_depth(command_buffer);

            if (imgui.parameters.adsm_on) draw_hairs(scene_graph, hair_depth_pipeline, command_buffer, vp);
            if (imgui.parameters.ctsm_on) draw_model(scene_graph, mesh_depth_pipeline, command_buffer, vp);

            command_buffer.end_render_pass();
        }
        vk::DebugMarker::close(command_buffers[frame], "Bake Shadow Maps", query_pools[frame]);

        vk::DebugMarker::close(command_buffers[frame]);
    }

    void Rasterizer::draw_hairs(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer, glm::mat4 projection) {
        command_buffer.bind_pipeline(pipeline); // Color / Depth / Voxels.
        for (auto& hair_node : scene_graph.get_nodes_with_hair_styles()) {
            command_buffer.push_constant(pipeline, 0, projection * hair_node->get_model_matrix());
            for (auto& hair_style : hair_node->get_hair_styles())
                hair_styles[hair_style].draw(pipeline, pipeline.descriptor_sets[frame], command_buffer);
        }
    }

    void Rasterizer::strand_dvr(const SceneGraph& scene_graph, Pipeline& pipeline, vk::CommandBuffer& command_buffer) {
        command_buffer.bind_pipeline(pipeline);
        for (auto& hair_node : scene_graph.get_nodes_with_hair_styles()) {
            command_buffer.push_constant(pipeline, 0, hair_node->get_model_matrix());
            for (auto& hair_style : hair_node->get_hair_styles())
                hair_styles[hair_style].draw_volume(pipeline, pipeline.descriptor_sets[frame], command_buffer);
        }
    }

    void Rasterizer::draw(Image& fullscreen_image) {
        command_buffer_finished[frame].wait_and_reset();
        imgui.record_performance(query_pools[frame].request_timestamp_queries());

        auto frame_image = swap_chain.acquire_next_image(image_available[frame]);

        command_buffers[frame].begin();

        command_buffers[frame].reset_query_pool(query_pools[frame], 0, // performance.
                                                query_pools[frame].get_query_count());

        fullscreen_billboard.send_img(billboards_pipeline.descriptor_sets[frame],
                                      fullscreen_image, command_buffers[frame]);

        vk::DebugMarker::begin(command_buffers[frame], "Blit Framebuffer", query_pools[frame]);
        command_buffers[frame].begin_render_pass(color_pass, framebuffers[frame],
                                                 { 1.00f, 1.00f, 1.00f, 1.00f });

        command_buffers[frame].bind_pipeline(billboards_pipeline);
        camera[frame].update(Camera::IdentityVPMatrix);
        command_buffers[frame].push_constant(billboards_pipeline, 0, Identity);
        fullscreen_billboard.draw(billboards_pipeline, billboards_pipeline.descriptor_sets[frame],
                                  command_buffers[frame]);

        imgui.draw(command_buffers[frame]);

        command_buffers[frame].end_render_pass();
        vk::DebugMarker::close(command_buffers[frame], "Blit Framebuffer", query_pools[frame]);

        command_buffers[frame].end();

        device.get_graphics_queue().submit(command_buffers[frame], image_available[frame],
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete[frame], command_buffer_finished[frame]);
        device.get_present_queue().present(swap_chain, frame_image, render_complete[frame]);

        latest_drawn_frame = frame;
        frame = fetch_next_frame();
    }

    void Rasterizer::build_pipelines() {
        vulkan::HairStyle::depth_pipeline(hair_depth_pipeline, *this);
        vulkan::Model::depth_pipeline(mesh_depth_pipeline, *this);
        vulkan::HairStyle::voxel_pipeline(hair_voxel_pipeline, *this);
        vulkan::Volume::build_pipeline(strand_dvr_pipeline, *this);
        vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);
        vulkan::Model::build_pipeline(model_mesh_pipeline, *this);
        vulkan::Billboard::build_pipeline(billboards_pipeline, *this);
    }

    void Rasterizer::build_render_passes() {
        vk::RenderPass::create_modified_color_pass(color_pass, device, swap_chain);
        vk::RenderPass::create_standard_depth_pass(depth_pass, device);
    }

    void Rasterizer::recreate_swapchain(Window&) {
        // TODO: still need to do this, one day.
    }

    Interface& Rasterizer::get_imgui() {
        return imgui;
    }

    Image Rasterizer::get_screenshot(SceneGraph& scene_graph, Raytracer& ray_tracer) {
        bool previous_visibility { imgui.hide() };

        if (imgui.raytracing_enabled())
            draw(ray_tracer.get_framebuffer());
        else draw(scene_graph); // hides ImGui.

        imgui.set_visibility(previous_visibility);

        device.wait_idle();

        vk::Image screenshot_image {
            device,
            swap_chain.get_width(),
            swap_chain.get_height(),
            VK_FORMAT_R8G8B8A8_SRGB,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT,
            1, VK_SAMPLE_COUNT_1_BIT,
            VK_IMAGE_TILING_LINEAR
        };

        vk::DeviceMemory screenshot_memory {
            device,
            screenshot_image.get_memory_requirements(),
            vk::DeviceMemory::Type::HostVisible
        };

        screenshot_image.bind(screenshot_memory);

        auto command_buffer = command_pool.allocate_and_begin();
        screenshot_image.transition(command_buffer, 0, VK_ACCESS_TRANSFER_WRITE_BIT,
                                    VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        swap_chain.get_images()[latest_drawn_frame].transition(command_buffer, VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
                                                               VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                                               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        command_buffer.copy_image(swap_chain.get_images()[latest_drawn_frame], screenshot_image);

        swap_chain.get_images()[latest_drawn_frame].transition(command_buffer, VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                                                               VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

        screenshot_image.transition(command_buffer, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_MEMORY_READ_BIT,
                                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
                                    VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
        command_buffer.end();
        device.get_graphics_queue().submit(command_buffer)
                                   .wait_idle();

        vkhr::Image screenshot { swap_chain.get_width(), swap_chain.get_height() };

        const char* buffer { nullptr };
        screenshot_memory.map(0, screenshot.get_size_in_bytes(), (void**) &buffer);
        std::memcpy(screenshot.get_data(), buffer, screenshot.get_size_in_bytes());
        screenshot_memory.unmap();

        screenshot.flip_channels();

        return screenshot;
    }

    void Rasterizer::recompile() {
        device.wait_idle(); // If any pipeline is still in use we need to wait until execution is complete to recompile it.

        if (recompile_pipeline_shaders(hair_depth_pipeline)) vulkan::HairStyle::depth_pipeline(hair_depth_pipeline, *this);
        if (recompile_pipeline_shaders(mesh_depth_pipeline)) vulkan::Model::depth_pipeline(mesh_depth_pipeline, *this);
        if (recompile_pipeline_shaders(hair_voxel_pipeline)) vulkan::HairStyle::voxel_pipeline(hair_voxel_pipeline, *this);

        if (recompile_pipeline_shaders(strand_dvr_pipeline)) vulkan::Volume::build_pipeline(strand_dvr_pipeline, *this);

        if (recompile_pipeline_shaders(hair_style_pipeline)) vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);
        if (recompile_pipeline_shaders(model_mesh_pipeline)) vulkan::Model::build_pipeline(model_mesh_pipeline, *this);
        if (recompile_pipeline_shaders(billboards_pipeline)) vulkan::Billboard::build_pipeline(billboards_pipeline, *this);
    }

    bool Rasterizer::recompile_pipeline_shaders(Pipeline& pipeline) {
        bool pipeline_dirty { false };
        for (auto& shader_module : pipeline.shader_stages)
            pipeline_dirty |= shader_module.recompile();
        return pipeline_dirty;
    }
}

#include <vkhr/rasterizer.hh>

#include <vkpp/version.hh>
#include <vkpp/application.hh>
#include <vkpp/layer.hh>
#include <vkpp/extension.hh>
#include <vkpp/append.hh>

namespace vk = vkpp;

namespace vkhr {
    Rasterizer::Rasterizer(Window& window, const SceneGraph& scene_graph, bool vsync = true) {
        vk::Version target_vulkan_loader { 1,1 };
        vk::Application application_information {
            "VKHR", { 1, 0, 0 },
            "None", { 0, 0, 0 },
            target_vulkan_loader
        };

        std::vector<vk::Layer> required_layers {
            "VK_LAYER_LUNARG_standard_validation"
        };

        std::vector<vk::Extension> required_extensions {
            "VK_EXT_debug_utils"
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

        descriptor_pool = vkpp::DescriptorPool {
            device,
            {
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024 },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024 },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,         1024 }
            }
        };

        render_pass = default_render_pass();

        framebuffers = swap_chain.create_framebuffers(render_pass);

        image_available = vk::Semaphore { device };
        render_complete = vk::Semaphore { device };
        command_buffer_done = vk::Fence { device };

        build_pipelines();

        load(scene_graph);

        command_buffers = command_pool.allocate(framebuffers.size());
    }

    void Rasterizer::load(const SceneGraph& scene_graph) {
        imgui = Interface { window_surface.get_hwnd(), *this };
        for (const auto& hair_style : scene_graph.get_hair_styles()) {
            const auto& hair_style_geometry = hair_style.second;
            hair_styles[&hair_style_geometry] = vulkan::HairStyle {
                hair_style_geometry,
                *this,
                hair_style_pipeline
            };
        }
    }

    void Rasterizer::update(const SceneGraph& scene_graph) {
        imgui.update(scene_graph);
    }

    void Rasterizer::draw(const SceneGraph& scene_graph) {
        auto next_image = swap_chain.acquire_next_image(image_available);

        update(scene_graph); // e.g. buffers.

        auto& cam = scene_graph.get_camera();

        command_buffer_done.wait_and_reset();

        for (std::size_t i { 0 }; i < swap_chain.size(); ++i) {
            command_buffers[i].begin();
            command_buffers[i].begin_render_pass(render_pass, framebuffers[i],
                                                 { 1.0f, 1.0f, 1.0f, 1.0f });

            for (auto& hair_node : scene_graph.get_nodes_with_hair_styles()) {
                auto& model_view_projection = cam.get_mvp(hair_node->get_m());
                draw(hair_node, command_buffers[i], i, model_view_projection);
            }

            imgui.draw(command_buffers[i]);

            command_buffers[i].end_render_pass();
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

    void Rasterizer::build_pipelines() {
        vulkan::HairStyle::build_pipeline(hair_style_pipeline, *this);
    }

    vk::RenderPass Rasterizer::default_render_pass() {
        std::vector<vk::RenderPass::Attachment> attachments {
            {
                swap_chain.get_color_attachment_format(),
                swap_chain.get_khr_presentation_layout()
            },
            {
                swap_chain.get_depth_attachment_format(),
                swap_chain.get_depth_attachment_layout()
            }
        };

        std::vector<vk::RenderPass::Subpass> render_subpasses {
            {
                { 0, swap_chain.get_color_attachment_layout() },
                { 1, swap_chain.get_depth_attachment_layout() }
            }
        };

        std::vector<vk::RenderPass::Dependency> dependencies {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
            }
        };

        return vkpp::RenderPass {
            device,
            attachments,
            render_subpasses,
            dependencies
        };
    }

    Interface& Rasterizer::get_imgui() {
        return imgui;
    }
}

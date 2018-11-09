#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

int main(int argc, char** argv) {
    vkhr::ArgParser argp { vkhr::arguments };
    auto scene_file = argp.parse(argc, argv);

    // TODO: start shoveling this code around
    // to somewhere else. e.g. Rasterizer :-)

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

    const vkhr::Image vulkan_icon { IMAGE("vulkan.icon") };
    vkhr::Window window { 1280, 720, "VKHR", vulkan_icon };

    vkhr::InputMap input_map { window };

    input_map.bind("quit",       vkhr::Input::Key::Escape);
    input_map.bind("fullscreen", vkhr::Input::Key::F);

    vk::append(window.get_vulkan_surface_extensions(),
               required_extensions); // VK_surface_KHR

    vk::Instance instance {
        application_information,
        required_layers,
        required_extensions
    };

    auto window_surface = window.create_vulkan_surface(instance);

    // Find physical devices that seem most promising of the lot.
    auto score = [&](const vk::PhysicalDevice& physical_device) {
        short gpu_suitable = physical_device.is_discrete_gpu()*2+
                             physical_device.is_integrated_gpu();
        return physical_device.has_every_queue() * gpu_suitable *
               physical_device.has_present_queue(window_surface);
    };

    auto physical_device = instance.find_physical_devices(score);

    window.append_string(physical_device.get_name());

    physical_device.assign_present_queue_indices(window_surface);

    std::vector<vk::Extension> device_extensions {
        "VK_KHR_swapchain"
    };

    // Just enable every device feature we have right now.
    auto device_features = physical_device.get_features();

    vk::Device device {
        physical_device,
        required_layers,
        device_extensions,
        device_features
    };

    vk::CommandPool command_pool { device, device.get_graphics_queue() };

    vk::SwapChain swap_chain {
        device,
        window_surface,
        {
            VK_FORMAT_B8G8R8A8_UNORM,
            VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
        },
        vk::SwapChain::PresentationMode::Fifo,
        window.get_extent()
    };

    std::vector<vk::RenderPass::Attachment> attachments {
        {
            swap_chain.get_format(),
            swap_chain.get_sample_count(),
            swap_chain.get_layout()
        }
    };

    std::vector<vk::RenderPass::Subpass> subpasses {
        {
            {
                { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
            }
        }
    };

    std::vector<vk::RenderPass::SubpassDependency> dependencies {
        {
            0,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
        }
    };

    vk::RenderPass render_pass {
        device,
        attachments,
        subpasses,
        dependencies
    };

    vkhr::HairStyle hair_style { STYLE("ponytail.hair") };

    hair_style.generate_tangents(); // TODO: pre-bake this

    vk::VertexBuffer vertex_buffer {
        device,
        command_pool,
        hair_style.vertices,
        0,
        {
            { 0, VK_FORMAT_R32G32B32_SFLOAT }
        }
    };

    vk::VertexBuffer normal_buffer {
        device,
        command_pool,
        hair_style.tangents,
        1,
        {
            { 1, VK_FORMAT_R32G32B32_SFLOAT }
        }
    };

    vk::GraphicsPipeline::FixedFunction fixed_functions;

    fixed_functions.add_vertex_input(vertex_buffer);
    fixed_functions.add_vertex_input(normal_buffer);

    fixed_functions.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);

    fixed_functions.set_scissor({ 0, 0, swap_chain.get_extent() });
    fixed_functions.set_viewport({ 0.0, 0.0,
                                   static_cast<float>(swap_chain.get_width()),
                                   static_cast<float>(swap_chain.get_height()),
                                   0.0, 1.0 });

    fixed_functions.set_line_width(1.0);

    fixed_functions.enable_alpha_blending_for(0);

    std::vector<vk::ShaderModule> shading_stages;

    shading_stages.emplace_back(device, SHADER("strand.vert"));
    shading_stages.emplace_back(device, SHADER("strand.frag"));

    struct Transform {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 projection;
    } mvp;

    std::vector<vk::UniformBuffer> uniform_buffers;

    for (std::size_t i { 0 } ; i < swap_chain.size(); ++i)
        uniform_buffers.emplace_back(device, mvp,
                                     sizeof(mvp));

    vk::DescriptorSet::Layout descriptor_layout {
        device,
        {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
        }
    };

    vk::DescriptorPool descriptor_pool {
        device,
        {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, swap_chain.size() }
        }
    };

    auto descriptor_sets = descriptor_pool.allocate(swap_chain.size(), descriptor_layout);

    for (std::size_t i { 0 }; i < descriptor_sets.size(); ++i)
        descriptor_sets[i].write(0, uniform_buffers[i]);

    vk::Pipeline::Layout layout {
        device,
        descriptor_layout
    };

    vk::GraphicsPipeline graphics_pipeline {
        device,
        shading_stages,
        fixed_functions,
        layout,
        render_pass
    };

    auto framebuffers = swap_chain.create_framebuffers(render_pass);

    vk::Semaphore image_available { device };
    vk::Semaphore render_complete { device };

    auto command_buffers = command_pool.allocate(framebuffers.size());

    for (std::size_t i { 0 }; i < framebuffers.size(); ++i) {
        command_buffers[i].begin();
        command_buffers[i].begin_render_pass(render_pass, framebuffers[i],
                                             { 1.0f, 1.0f, 1.0f, 1.0f });
        command_buffers[i].bind_pipeline(graphics_pipeline);
        command_buffers[i].bind_descriptor_set(descriptor_sets[i],
                                               graphics_pipeline);
        command_buffers[i].bind_vertex_buffer(vertex_buffer);
        command_buffers[i].bind_vertex_buffer(normal_buffer);
        command_buffers[i].draw(vertex_buffer.elements(), 1);
        command_buffers[i].end_render_pass();
        command_buffers[i].end();
    }

    vkhr::Camera camera { glm::radians(45.0f), swap_chain.get_aspect_ratio() };

    camera.look_at({ 0.000f, 60.0f, 0.000f },
                   { 200.0f, 35.0f, 200.0f });

    window.show();

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("fullscreen")) {
            window.toggle_fullscreen();
        }

        auto next_image = swap_chain.acquire_next_image(image_available);

        mvp.model = glm::rotate(glm::mat4(1.0f),
                                window.get_current_time() * glm::radians(45.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
        mvp.projection = camera.get_projection_matrix();
        mvp.view = camera.get_view_matrix();

        uniform_buffers[next_image].update(mvp);

        device.get_graphics_queue().submit(command_buffers[next_image],
                                           image_available,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete);

        device.get_present_queue().present(swap_chain, next_image, render_complete);

        window.poll_events();
    }

    return 0;
}

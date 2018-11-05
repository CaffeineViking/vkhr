#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

int main(int argc, char** argv) {
    vkhr::ArgParser argp { vkhr::arguments };
    auto scene_file = argp.parse(argc, argv);

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
        return physical_device.is_discrete_gpu() *
               physical_device.has_every_queue() *
               physical_device.has_present_queue(window_surface);
    };

    auto physical_device = instance.find_physical_devices(score);

    window.append_string(physical_device.get_details());

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

    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;
    };

    const std::vector<Vertex> vertices {
        { { -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f } },
        { {  0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f } },
        { {  0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f } },
        { { -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f } }
    };

    const std::vector<std::uint16_t> indices {
        0, 1, 2,
        2, 3, 0
    };

    vk::CommandPool command_pool { device, device.get_graphics_queue() };

    vk::Buffer staging_buffer {
        device,
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    auto staging_memory_requirements = staging_buffer.get_memory_requirements();

    vk::DeviceMemory staging_buffer_memory {
        device,
        staging_memory_requirements,
        vk::DeviceMemory::Type::HostVisible
    };

    staging_buffer.bind(staging_buffer_memory);
    staging_buffer_memory.copy(vertices, 0ul);

    vk::Buffer vertex_buffer {
        device,
        sizeof(vertices[0]) * vertices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
    };

    auto vertex_memory_requirements = vertex_buffer.get_memory_requirements();

    vk::DeviceMemory vertex_buffer_memory {
        device,
        vertex_memory_requirements,
        vk::DeviceMemory::Type::DeviceLocal
    };

    vertex_buffer.bind(vertex_buffer_memory);

    {
        auto command_buffer = command_pool.allocate();

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        command_buffer.copy_buffer(staging_buffer, vertex_buffer);
        command_buffer.end();

        device.get_transfer_queue().submit(command_buffer).wait_idle();
    }

    staging_buffer = vk::Buffer {
        device,
        sizeof(indices[0]) * indices.size(),
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT
    };

    staging_memory_requirements = staging_buffer.get_memory_requirements();

    staging_buffer_memory = vk::DeviceMemory {
        device,
        staging_memory_requirements,
        vk::DeviceMemory::Type::HostVisible
    };

    staging_buffer.bind(staging_buffer_memory);
    staging_buffer_memory.copy(indices, 0ul);

    vk::Buffer index_buffer {
        device,
        sizeof(indices[0]) * indices.size(),
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT
    };

    auto index_memory_requirements = index_buffer.get_memory_requirements();

    vk::DeviceMemory index_buffer_memory {
        device,
        index_memory_requirements,
        vk::DeviceMemory::Type::DeviceLocal
    };

    index_buffer.bind(index_buffer_memory);

    {
        auto command_buffer = command_pool.allocate();

        command_buffer.begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
        command_buffer.copy_buffer(staging_buffer, index_buffer);
        command_buffer.end();

        device.get_transfer_queue().submit(command_buffer).wait_idle();
    }

    std::vector<vk::VertexBinding> vertex_bindings {
        {
            0,
            sizeof(Vertex),
            VK_VERTEX_INPUT_RATE_VERTEX
        }
    };

    std::vector<vk::VertexAttribute> vertex_attributes {
        {
            0,
            0,
            VK_FORMAT_R32G32_SFLOAT,
            offsetof(Vertex, position)
        },
        {
            1,
            0,
            VK_FORMAT_R32G32B32_SFLOAT,
            offsetof(Vertex, color)
        }
    };

    vk::GraphicsPipeline::FixedFunction fixed_functions;

    fixed_functions.set_vertex_bindings(vertex_bindings);
    fixed_functions.set_vertex_attributes(vertex_attributes);

    fixed_functions.disable_depth_testing();

    fixed_functions.set_scissor({ 0, 0, window.get_extent() });
    fixed_functions.set_viewport({ 0.0, 0.0,
                                   static_cast<float>(window.get_width()),
                                   static_cast<float>(window.get_height()),
                                   0.0, 1.0 });

    fixed_functions.enable_alpha_blending_for(0);

    std::vector<vk::ShaderModule> shading_stages;

    shading_stages.emplace_back(device, SPIRV("strand.vert"));
    shading_stages.emplace_back(device, SPIRV("strand.frag"));

    // Just the empty layout now.
    vk::Pipeline::Layout layout {
        device
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

    auto command_buffers = command_pool.allocate(2);

    for (std::size_t i { 0 }; i < command_buffers.size(); ++i) {
        command_buffers[i].begin();
        command_buffers[i].begin_render_pass(render_pass, framebuffers[i],
                                             { 1.0f, 1.0f, 1.0f, 1.0f });
        command_buffers[i].bind_pipeline(graphics_pipeline);
        command_buffers[i].bind_vertex_buffer(0, 1, vertex_buffer);
        command_buffers[i].bind_index_buffer(index_buffer, VK_INDEX_TYPE_UINT16);
        command_buffers[i].draw_indexed(indices.size(), 1, 0);
        command_buffers[i].end_render_pass();
        command_buffers[i].end();
    }

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("fullscreen")) {
            window.toggle_fullscreen();
        }

        auto next_image = swap_chain.acquire_next_image(image_available);

        device.get_graphics_queue().submit(command_buffers[next_image],
                                           image_available,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           render_complete);

        device.get_present_queue().present(swap_chain, next_image, render_complete);

        window.poll_events();
    }

    return 0;
}

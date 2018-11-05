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
               required_extensions);

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

    vk::RenderPass::Attachment attachment {
        swap_chain.get_format(),
        swap_chain.get_sample_count(),
        swap_chain.get_layout()
    };

    vk::RenderPass::Subpass subpass {
        {
            { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }
        }
    };

    vk::RenderPass::Dependency dependency {
        0,
        VK_SUBPASS_EXTERNAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    vk::RenderPass render_pass {
        device,
        attachment,
        subpass,
        dependency
    };

    vkhr::HairStyle curly_hair { STYLE("ponytail.hair") };

    auto& vertices = curly_hair.vertices;

    VkVertexInputBindingDescription binding_description { };

    binding_description.binding = 0;

    binding_description.stride = sizeof(glm::vec2);
    binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attribute_description { };

    attribute_description.binding = 0;
    attribute_description.location = 0;
    attribute_description.format = VK_FORMAT_R32G32_SFLOAT;
    attribute_description.offset = 0;

    VkDeviceSize vertex_offset { 0 };

    vk::GraphicsPipeline::FixedFunction fixed_functions;

    fixed_functions.set_line_width(2.0);

    fixed_functions.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_STRIP);

    fixed_functions.vertex_input_state.vertexBindingDescriptionCount = 1;
    fixed_functions.vertex_input_state.pVertexBindingDescriptions = &binding_description;
    fixed_functions.vertex_input_state.vertexAttributeDescriptionCount = 1;
    fixed_functions.vertex_input_state.pVertexAttributeDescriptions = &attribute_description;

    VkBufferCreateInfo buffer_info = {};
    buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_info.size = sizeof(vertices[0]) * vertices.size();

    buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

    buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VkBuffer vertex_buffer;

    if (vkCreateBuffer(device.get_handle(), &buffer_info, nullptr, &vertex_buffer)) {
        throw std::runtime_error { "Failed to create the hair strand buffer data!" };
    }

    VkMemoryRequirements memory_requirements;
    vkGetBufferMemoryRequirements(device.get_handle(), vertex_buffer, &memory_requirements);

    VkPhysicalDeviceMemoryProperties memory_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_device.get_handle(), &memory_properties);

    std::uint32_t mindex;
    std::uint32_t filter { memory_requirements.memoryTypeBits };
    std::uint32_t properties { VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT };
    for (uint32_t i = 0; i < memory_properties.memoryTypeCount; i++) {
        if ((filter & (1 << i)) && (memory_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            mindex = i;
        }
    }

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = memory_requirements.size;
    alloc_info.memoryTypeIndex = mindex;

    VkDeviceMemory vertex_buffer_memory;

    if (vkAllocateMemory(device.get_handle(), &alloc_info, nullptr, &vertex_buffer_memory)) {
        throw std::runtime_error { "Failed to alloc strand data!" };
    }

    vkBindBufferMemory(device.get_handle(), vertex_buffer, vertex_buffer_memory, 0);

    void* memory_map;
    vkMapMemory(device.get_handle(), vertex_buffer_memory, 0, buffer_info.size, 0, &memory_map);
    memcpy(memory_map, vertices.data(), static_cast<size_t>(buffer_info.size));
    vkUnmapMemory(device.get_handle(), vertex_buffer_memory);

    fixed_functions.disable_depth_testing(); // soon(tm)

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

    vk::CommandPool command_pool { device, device.get_graphics_queue() };

    auto command_buffers = command_pool.allocate(2);

    for (std::size_t i { 0 }; i < command_buffers.size(); ++i) {
        command_buffers[i].begin();
        command_buffers[i].begin_render_pass(render_pass, framebuffers[i],
                                             { 1.0f, 1.0f, 1.0f, 1.0f });
        command_buffers[i].bind_pipeline(graphics_pipeline);

        vkCmdBindVertexBuffers(command_buffers[i].get_handle(), 0, 1,
                               &vertex_buffer, &vertex_offset);

        command_buffers[i].draw(vertices.size(), 1, 0, 0);
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

    device.wait_idle();

    vkDestroyBuffer(device.get_handle(), vertex_buffer, nullptr);
    vkFreeMemory(device.get_handle(), vertex_buffer_memory, nullptr);

    return 0;
}

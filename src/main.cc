#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

#include <iostream>

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
        VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
    };

    std::vector<vk::RenderPass::Attachment> attachments {
        {
            swap_chain.get_format(),
            swap_chain.get_sample_count(), // attachment #0
            swap_chain.get_layout()
        }
    };

    vk::RenderPass render_pass {
        device,
        attachments,
        subpass,
        dependency
    };

    vk::GraphicsPipeline::FixedFunction fixed_functions;

    fixed_functions.disable_depth_testing(); // soon(tm)

    fixed_functions.set_scissor({ 0,0, window.get_extent() });
    fixed_functions.set_viewport({ 0.0, 0.0,
                                   static_cast<float>(window.get_width()),
                                   static_cast<float>(window.get_height()),
                                   0.0, 1.0 });

    fixed_functions.enable_alpha_blending_for(0);

    std::vector<vk::ShaderModule> shading_stages;

    shading_stages.emplace_back(device, SPIRV("simple.vert"));
    shading_stages.emplace_back(device, SPIRV("simple.frag"));

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

    // TODO: move the code below to a better abstraction level.

    std::vector<VkFramebuffer> framebuffers(swap_chain.size());

    for (std::size_t i { 0 }; i < framebuffers.size(); ++i) {
        VkImageView attachments[1] {
            swap_chain.get_attachment(i)
        };

        VkFramebufferCreateInfo create_info { };
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = render_pass.get_handle();
        create_info.attachmentCount = 1;
        create_info.pAttachments = attachments;
        create_info.width = swap_chain.get_width();
        create_info.height = swap_chain.get_height();
        create_info.layers = 1;

        if (vkCreateFramebuffer(device.get_handle(), &create_info, nullptr, &framebuffers[i])) {
            std::cerr << "Couldn't create framebuffer!" << std::endl;
        }
    }

    VkCommandPool command_pool;

    VkCommandPoolCreateInfo command_pool_create_info { };
    command_pool_create_info.sType =  VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.queueFamilyIndex = physical_device.get_graphics_queue_family_index();
    command_pool_create_info.flags = 0;

    if (vkCreateCommandPool(device.get_handle(), &command_pool_create_info, nullptr, &command_pool)) {
        std::cerr << "Couldn't create the command pool!" << std::endl;
    }

    std::vector<VkCommandBuffer> command_buffers(swap_chain.size());

    VkCommandBufferAllocateInfo command_pool_alloc_info { };

    command_pool_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_pool_alloc_info.commandPool = command_pool;
    command_pool_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_pool_alloc_info.commandBufferCount = static_cast<std::uint32_t>(command_buffers.size());

    if (vkAllocateCommandBuffers(device.get_handle(), &command_pool_alloc_info, command_buffers.data())) {
        std::cerr << "Couldn't allocate command buffers!" << std::endl;
    }

    for (std::size_t i { 0 }; i < command_buffers.size(); ++i) {
        VkCommandBufferBeginInfo command_buffer_begin {  };
        command_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        command_buffer_begin.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin)) {
            std::cerr << "Couldn't begin command buffer!" << std::endl;
        }

        VkRenderPassBeginInfo render_pass_begin {  };
        render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_begin.renderPass = render_pass.get_handle();
        render_pass_begin.framebuffer = framebuffers[i];
        render_pass_begin.renderArea.offset = { 0, 0 };
        render_pass_begin.renderArea.extent = swap_chain.get_extent();

        VkClearValue clear_color { 0.0, 0.0, 0.0, 1.0 };

        render_pass_begin.clearValueCount = 1;
        render_pass_begin.pClearValues = &clear_color;

        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(command_buffers[i], graphics_pipeline.get_bind_point(), graphics_pipeline.get_handle());
        vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
        vkCmdEndRenderPass(command_buffers[i]);

        if (vkEndCommandBuffer(command_buffers[i])) {
            std::cerr << "Couldn't end command buffer!" << std::endl;
        }
    }

    vk::Semaphore image_available { device };
    vk::Semaphore render_complete { device };

    vkhr::HairStyle curly_hair { STYLE("wCurly.hair") };

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("fullscreen")) {
            window.toggle_fullscreen();
        }

        auto next_image = swap_chain.acquire_next_image(image_available);

        VkSubmitInfo submit_info {  };
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        VkPipelineStageFlags wait_stages { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &image_available.get_handle();
        submit_info.pWaitDstStageMask = &wait_stages;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffers[next_image];

        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &render_complete.get_handle();

        if (vkQueueSubmit(device.get_graphics_queue()->get_handle(), 1, &submit_info, VK_NULL_HANDLE)) {
            std::cerr << "Failed to submit draw commands to graphics queue!" << std::endl;
        }

        VkPresentInfoKHR present_info {  };
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_complete.get_handle();
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain.get_handle();
        present_info.pImageIndices = &next_image;
        present_info.pResults = nullptr;

        vkQueuePresentKHR(device.get_present_queue()->get_handle(), &present_info);

        window.poll_events();
    }

    device.wait_idle();

    vkDestroyCommandPool(device.get_handle(), command_pool, nullptr);

    for (auto& framebuffer : framebuffers) {
        vkDestroyFramebuffer(device.get_handle(), framebuffer, nullptr);
    }

    return 0;
}

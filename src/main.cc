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

    std::vector<vk::RenderPass::Attachment> attachments {
        {
            swap_chain.get_format(),
            swap_chain.get_sample_count(),
            swap_chain.get_layout()
        }
    };

    vk::RenderPass render_pass {
        device,
        attachments,
        subpass
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
        render_pass, 0u
    };

    vkhr::HairStyle curly_hair { STYLE("wCurly.hair") };

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("fullscreen")) {
            window.toggle_fullscreen();
        }

        window.poll_events();
    }

    return 0;
}

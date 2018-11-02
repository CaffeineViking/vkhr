#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

int main(int argc, char** argv) {
    vkhr::ArgParser argp { vkhr::arguments };
    auto scene_file = argp.parse(argc, argv);

    // TODO: move over lots of boilerplate to
    // some rasterizer abstractions (Vulkan).

    vk::Version target_vulkan_loader { 1,1 };
    vk::Application application_information {
        "VKHR", { 1, 0, 0 },
        "None", { 0, 0, 0 },
        target_vulkan_loader
    };

    std::vector<vk::Layer> required_layers {
        vk::DebugMessenger::InstanceLayer
    };

    std::vector<vk::Extension> required_extensions {
        vk::DebugMessenger::InstanceExtension
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
        vk::SwapChain::DeviceExtension
    };

    auto device_features = physical_device.get_features();

    vk::Device device {
        physical_device,
        required_layers,
        device_extensions,
        device_features
    };

    VkSurfaceFormatKHR preferred_format {
        VK_FORMAT_B8G8R8A8_UNORM,
        VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
    };

    auto preferred_present_mode = vk::SwapChain::PresentMode::MailBox;

    vk::SwapChain swap_chain {
        device,
        window_surface,
        preferred_format,
        preferred_present_mode,
        window.get_extent()
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

#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

int main(int argc, char** argv) {
    vkhr::ArgParser argp { argc, argv };
    vk::Version target_vulkan_loader { 1, 1 };
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

    // Append the required Vulkan surface extensions as well.
    auto surface_extensions = window.get_surface_extension();
    required_extensions.insert(required_extensions.begin(),
                               surface_extensions.begin(),
                               surface_extensions.end());

    vk::Instance instance {
        application_information,
        required_layers,
        required_extensions
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

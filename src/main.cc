#include <iostream>

#include <chrono>

#include <vkhr/paths.hh>
#include <vkhr/hair_style.hh>
#include <vkhr/image.hh>

#include <vkhr/logger.hh>
#include <vkhr/input_maps.hh>
#include <vkhr/window.hh>

#include <vkpp/vkpp.hh>
namespace vk = vkpp;

int main(int, char**) {
    auto start_time = std::chrono::system_clock::now();

    vkhr::HairStyle curly_hair { STYLE("wCurly.hair") };

    auto end_time = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Loading hair geometry took: " << elapsed.count() << " ms" << std::endl;

    if (!curly_hair) std::cerr << "Something is broken?: ";

    // NOT how to handle errors. Just for testing purposes!

    switch (curly_hair.get_last_error_state()) {
    case vkhr::HairStyle::Error::None: break;
    case vkhr::HairStyle::Error::OpeningFile:
        std::cerr << "failed to open a file!" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingFileHeader:
        std::cerr << "failed to read header!" << std::endl;
        break;
    case vkhr::HairStyle::Error::InvalidSignature:
        std::cerr << "this isn't a hair file" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingSegments:
        std::cerr << "failed to get segments" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingVertices:
        std::cerr << "failed to get vertices" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingThickness:
        std::cerr << "fails to get thickness" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingTransparency:
        std::cerr << "didnt get transparency" << std::endl;
        break;
    case vkhr::HairStyle::Error::ReadingColor:
        std::cerr << "failed to get a color!" << std::endl;
        break;
    case vkhr::HairStyle::Error::InvalidFormat:
        std::cerr << "format is non-standard" << std::endl;
        break;
    default: std::cerr << "something else :(" << std::endl;
    }

    vk::Application application_information {
        "VKHR", { 1, 0, 0 },
        "None", { 0, 0, 0 },
        vk::Instance::get_api_version()
    };

    std::vector<vk::Layer> required_layers {
        "VK_LAYER_LUNARG_standard_validation"
    };

    std::vector<vk::Extension> required_extensions {
        "VK_EXT_debug_utils"
    };

    std::cout << "\nAvailable layers:\n" << std::endl;
    for (auto layers : vk::Instance::get_available_layers()) {
        std::cout << layers.name << '\n';
    }

    std::cout << std::endl;

    std::cout << "Available extensions:\n" << std::endl;
    for (auto extensions : vk::Instance::get_available_extensions()) {
        std::cout << extensions.name << '\n';
    }

    std::cout << std::endl;

    const vkhr::Image vulkan_icon { IMAGE("vulkan.icon") };
    vkhr::Window window { 1280, 720, "VKHR", vulkan_icon };

    vkhr::InputMapper input_map { window };

    input_map.bind("quit",       vkhr::Input::Key::Escape);
    input_map.bind("fullscreen", vkhr::Input::Key::F);

    // Append the required Vulkan surface extensions as well.
    auto surface_extensions = window.get_surface_extensions();
    required_extensions.insert(required_extensions.begin(),
                               surface_extensions.begin(),
                               surface_extensions.end());

    vk::Instance instance {
        application_information,
        required_layers,
        required_extensions
    };

    std::cout << "Layers enabled:\n" << std::endl;
    for (auto layer : instance.get_enabled_layers()) {
        std::cout << layer.name << '\n';
    }

    std::cout << std::endl;

    std::cout << "Extensions enabled:\n" << std::endl;
    for (auto extension : instance.get_enabled_extensions()) {
        std::cout << extension.name << '\n';
    }

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

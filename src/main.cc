#include <iostream>

#include <chrono>

#include <vkhr/paths.hh>
#include <vkhr/hair_style.hh>
#include <vkhr/image.hh>

#include <vkhr/input_maps.hh>
#include <vkhr/window.hh>

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

    const vkhr::Image vulkan_icon { IMAGE("vulkan.icon") };
    vkhr::Window window { 1280, 720, "VKHR", vulkan_icon };

    vkhr::InputMapper input_map { window };

    input_map.bind("quit",       vkhr::Input::Key::Escape);
    input_map.bind("fullscreen", vkhr::Input::Key::F);

    while (window.is_open()) {
        if (input_map.pressed("quit")) {
            window.close();
        }

        window.poll_events();
    }

    return 0;
}

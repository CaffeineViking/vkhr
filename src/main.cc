#include <iostream>

#include <chrono>

#include <vkhr/paths.hh>
#include <vkhr/hair_style.hh>

#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main(int, char**) {
    auto start_time = std::chrono::system_clock::now();

    vkhr::HairStyle curly_hair { STYLE("wCurly.hair") };

    auto end_time = std::chrono::system_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    std::cout << "Loading hair geometry took: " << elapsed.count() << " ms" << std::endl;

    if (!curly_hair) std::cerr << "Something is broken?: ";

    // NOT how to handle errors. Just for testing purposes!

    switch (curly_hair.get_previous_failure_code()) {
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

    // Copy-paste from the GLFW examples. To test building.

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    /* Create a windowed mode window and no OpenGL context */
    window = glfwCreateWindow(640, 480, "Test!", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    GLFWimage vulkan_icon;

    /* Testing out if the STB image loading is working correctly! */

    vulkan_icon.pixels = stbi_load(IMAGE("vulkan.png"), &vulkan_icon.width,
                                   &vulkan_icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &vulkan_icon);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

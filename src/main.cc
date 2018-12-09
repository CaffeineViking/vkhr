#include <vkhr/arg_parser.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/paths.hh>
#include <vkhr/image.hh>
#include <vkhr/window.hh>
#include <vkhr/input_map.hh>
#include <vkhr/rasterizer.hh>
#include <vkhr/ray_tracer.hh>

#include <glm/glm.hpp>

int main(int argc, char** argv) {
    vkhr::ArgParser argp { vkhr::arguments };
    auto scene_file = argp.parse(argc, argv);

    if (scene_file.empty()) scene_file = SCENE("ponytail.vkhr");

    vkhr::SceneGraph scene_graph { scene_file };
    auto& camera { (scene_graph.get_camera()) };

    int width  = argp["x"].value.integer,
        height = argp["y"].value.integer;

    camera.set_resolution(width, height);

    vkhr::Raytracer ray_tracer { scene_graph };

    const vkhr::Image vulkan_icon { IMAGE("vulkan-icon.png") };
    vkhr::Window window { width, height, "VKHR", vulkan_icon };

    if (argp["fullscreen"].value.boolean)
        window.toggle_fullscreen();

    window.enable_vsync(argp["vsync"].value.boolean);

    vkhr::InputMap input_map { window };

    input_map.bind("quit", vkhr::Input::Key::Escape);
    input_map.bind("toggle_ui", vkhr::Input::Key::GraveAccent);
    input_map.bind("grab", vkhr::Input::MouseButton::Left);
    input_map.bind("toggle_fullscreen", vkhr::Input::Key::F11);
    input_map.bind("take_screenshot", vkhr::Input::Key::PrintScreen);
    input_map.bind("recompile", vkhr::Input::Key::R);

    vkhr::Rasterizer rasterizer { window, scene_graph };

    if (argp["ui"].value.boolean == 0)
        rasterizer.get_imgui().hide();

    auto& imgui = rasterizer.get_imgui();

    window.show();

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("toggle_ui")) {
            imgui.toggle_visibility();
        } else if (input_map.just_pressed("toggle_fullscreen")) {
            window.toggle_fullscreen();
        } else if (input_map.just_pressed("take_screenshot")) {
            rasterizer.get_screenshot(scene_graph, ray_tracer).save_time();
        } else if (input_map.just_pressed("recompile")) {
            rasterizer.recompile();
        }

        camera.control(input_map, window.update_delta_time(),
                       rasterizer.get_imgui().wants_focus());

        scene_graph.traverse_nodes();

        imgui.transform(scene_graph, rasterizer, ray_tracer);

        if (imgui.raytracing_enabled()) {
            ray_tracer.draw(scene_graph);
            auto& framebuffer = ray_tracer.get_framebuffer();
            rasterizer.draw(framebuffer);
        } else {
            rasterizer.draw(scene_graph);
        }

        window.poll_events();
    }

    return 0;
}

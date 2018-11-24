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

    int width  = argp["x"].value.integer,
        height = argp["y"].value.integer;

    const vkhr::Image vulkan_icon { IMAGE("vulkan-icon.png") };
    vkhr::Window window { width, height, "VKHR", vulkan_icon };

    if (argp["fullscreen"].value.boolean)
        window.toggle_fullscreen();

    vkhr::InputMap input_map { window };

    input_map.bind("quit", vkhr::Input::Key::Escape);
    input_map.bind("grab", vkhr::Input::MouseButton::Left);
    input_map.bind("toggle_ui", vkhr::Input::Key::U);
    input_map.bind("recompile", vkhr::Input::Key::R);

    bool vsync_enabled = argp["vsync"].value.boolean;

    vkhr::Rasterizer rasterizer { window, scene_graph, vsync_enabled };

    if (argp["ui"].value.boolean == 0)
        rasterizer.get_imgui().hide();

    auto last_mouse_point = input_map.get_mouse_position();

    window.show();

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("toggle_ui")) {
            rasterizer.get_imgui().toggle_visibility();
        } else if (input_map.just_pressed("recompile")) {
            // TODO: recompile shaders by calling glslc
        }

        glm::vec2 cursor_delta { 0.0f, 0.0f };

        if (input_map.just_released("grab")) {
            input_map.unlock_cursor();
        } else if (!rasterizer.get_imgui().wants_focus()) {
            if (input_map.just_pressed("grab")) {
                input_map.freeze_cursor();
                last_mouse_point = input_map.get_mouse_position();
            } else if (input_map.pressed("grab")) {
                auto mouse_point = input_map.get_mouse_position();
                cursor_delta = mouse_point - last_mouse_point;
                last_mouse_point = mouse_point;
                cursor_delta *= window.delta_time() * 0.3117f;
                scene_graph.get_camera().arcball_by(cursor_delta);
            }
        }

        scene_graph.traverse_nodes();
        rasterizer.draw(scene_graph);

        window.poll_events();
    }

    return 0;
}

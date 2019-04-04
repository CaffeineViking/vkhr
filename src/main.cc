#include <vkhr/arg_parser.hh>
#include <vkhr/paths.hh>
#include <vkhr/image.hh>
#include <vkhr/window.hh>
#include <vkhr/input_map.hh>

#include <vkhr/rasterizer.hh>
#include <vkhr/scene_graph.hh>
#include <vkhr/ray_tracer.hh>

#include <glm/glm.hpp>

void build_benchmarks(vkhr::Rasterizer& dut);

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

    const vkhr::Image vulkan_icon { IMAGE("vulkan_icon.png") };
    vkhr::Window window { width, height, "VKHR", vulkan_icon };

    if (argp["fullscreen"].value.boolean)
        window.toggle_fullscreen();

    window.enable_vsync(argp["vsync"].value.boolean);

    if (argp["benchmark"].value.boolean == 1)
        window.enable_vsync(false);

    vkhr::InputMap input_map { window };

    input_map.bind("toggle_ui", vkhr::Input::Key::U);
    input_map.bind("grab", vkhr::Input::MouseButton::Left);
    input_map.bind("toggle_fullscreen", vkhr::Input::Key::F11);
    input_map.bind("take_screenshot", vkhr::Input::Key::S);
    input_map.bind("quit", std::vector<vkhr::Input::Key> { vkhr::Input::Key::Escape, vkhr::Input::Key::Q });
    input_map.bind("toggle_renderer", vkhr::Input::Key::T);
    input_map.bind("pan", vkhr::Input::MouseButton::Middle);
    input_map.bind("rotate_light", vkhr::Input::Key::L);
    input_map.bind("recompile", vkhr::Input::Key::R);

    vkhr::Rasterizer rasterizer { window, scene_graph };

    if (argp["ui"].value.boolean == 0)
        rasterizer.get_imgui().hide();

    auto& imgui = rasterizer.get_imgui();

    window.show();

    if (argp["benchmark"].value.boolean == 1) {
        build_benchmarks(rasterizer);
        rasterizer.start_benchmark(scene_graph);
    }

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("toggle_ui")) {
            imgui.toggle_visibility();
        } else if (input_map.just_pressed("toggle_fullscreen")) {
            window.toggle_fullscreen();
        } else if (input_map.just_pressed("take_screenshot")) {
            rasterizer.get_screenshot(scene_graph, ray_tracer)
                      .save_time(); // label using date/time.
        } else if (input_map.just_pressed("toggle_renderer")) {
            imgui.toggle_renderer();
        } else if (input_map.just_pressed("rotate_light")) {
            imgui.toggle_light_rotation();
        } else if (input_map.just_pressed("recompile")) {
            rasterizer.recompile();
        }

        camera.control(input_map, window.update_delta_time(),
                       rasterizer.get_imgui().wants_focus());

        scene_graph.traverse_nodes();

        imgui.transform(scene_graph, rasterizer, ray_tracer);

        if (window.surface_is_dirty() || rasterizer.swapchain_is_dirty()) {
            ray_tracer.recreate(window.get_width(), window.get_height());
            rasterizer.recreate_swapchain(window, scene_graph); // slow!?
        }

        if (imgui.raytracing_enabled()) {
            ray_tracer.draw(scene_graph);
            auto& framebuffer = ray_tracer.get_framebuffer();
            rasterizer.draw(framebuffer);
        } else {
            rasterizer.draw(scene_graph);
        }

        // Benchmark the renderer and dump timings.
        if (argp["benchmark"].value.boolean == 1) {
            if (!rasterizer.benchmark(scene_graph))
                return 0; // benchmark is complete!
        }

        window.poll_events();
    }

    return 0;
}

void build_benchmarks(vkhr::Rasterizer& rasterizer) {
    vkhr::Rasterizer::Benchmark default_parameter {
        "Benchmark Scenario",
        SCENE("ponytail.vkhr"),
        1280,
        720,
        vkhr::Renderer::Rasterizer,
        226,
        1.0,
        512
    };

    rasterizer.append_benchmark({ "Rasterizer (Ponytail)", SCENE("ponytail.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Rasterizer, 226, default_parameter.strand_reduction, default_parameter.raymarch_steps });

    for (float distance { 200.0f }; distance < 2000.0f; distance += 1800.0f / 64.0f) {
        rasterizer.append_benchmark({ "Rasterized Time vs. Distance (Ponytail)",
                                      SCENE("ponytail.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Rasterizer,
                                      distance,
                                      default_parameter.strand_reduction,
                                      default_parameter.raymarch_steps });
    }

    for (float strands { 1.0f }; strands >= 0.0f; strands -= 1.0f / 64.0f) {
        rasterizer.append_benchmark({ "Rasterized Time vs. Strands (Ponytail)",
                                      SCENE("ponytail.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Rasterizer,
                                      default_parameter.viewing_distance,
                                      strands,
                                      default_parameter.raymarch_steps });
    }

    rasterizer.append_benchmark({ "Raymarcher (Ponytail)", SCENE("ponytail.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Raymarcher, 226, default_parameter.strand_reduction, default_parameter.raymarch_steps });

    for (float distance { 200.0f }; distance < 2000.0f; distance += 1800.0f / 64.0f) {
        rasterizer.append_benchmark({ "Raymarched Time vs. Distance (Ponytail)",
                                      SCENE("ponytail.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Raymarcher,
                                      distance,
                                      default_parameter.strand_reduction,
                                      default_parameter.raymarch_steps });
    }

    for (int samples { 512 }; samples >= 64; samples -= 448 / 64) {
        rasterizer.append_benchmark({ "Raymarched Time vs. Samples (Ponytail)",
                                      SCENE("ponytail.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Raymarcher,
                                      default_parameter.viewing_distance,
                                      default_parameter.strand_reduction,
                                      samples });
    }

    rasterizer.append_benchmark({ "Rasterizer (Bear)", SCENE("bear.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Rasterizer, 385, default_parameter.strand_reduction, default_parameter.raymarch_steps });

    for (float distance { 300.0f }; distance < 3000.0f; distance += 2700.0f / 64.0f) {
        rasterizer.append_benchmark({ "Rasterized Time vs. Distance (Bear)",
                                      SCENE("bear.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Rasterizer,
                                      distance,
                                      default_parameter.strand_reduction,
                                      default_parameter.raymarch_steps });
    }

    for (float strands { 1.0f }; strands >= 0.0f; strands -= 1.0f / 64.0f) {
        rasterizer.append_benchmark({ "Rasterized Time vs. Strands (Bear)",
                                      SCENE("bear.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Rasterizer,
                                      default_parameter.viewing_distance,
                                      strands,
                                      default_parameter.raymarch_steps });
    }

    rasterizer.append_benchmark({ "Raymarcher (Bear)", SCENE("bear.vkhr"), default_parameter.width, default_parameter.height, vkhr::Renderer::Raymarcher, 385, default_parameter.strand_reduction, default_parameter.raymarch_steps });

    for (float distance { 300.0f }; distance < 3000.0f; distance += 2700.0f / 64.0f) {
        rasterizer.append_benchmark({ "Raymarched Time vs. Distance (Bear)",
                                      SCENE("bear.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Raymarcher,
                                      distance,
                                      default_parameter.strand_reduction,
                                      default_parameter.raymarch_steps });
    }

    for (int samples { 512 }; samples >= 64; samples -= 448 / 64) {
        rasterizer.append_benchmark({ "Raymarched Time vs. Samples (Bear)",
                                      SCENE("bear.vkhr"),
                                      default_parameter.width,
                                      default_parameter.height,
                                      vkhr::Renderer::Raymarcher,
                                      default_parameter.viewing_distance,
                                      default_parameter.strand_reduction,
                                      samples });
    }
}

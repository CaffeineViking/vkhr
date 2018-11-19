#include <vkhr/vkhr.hh>
#include <vkpp/vkpp.hh>

namespace vk = vkpp;

int main(int argc, char** argv) {
    vkhr::ArgParser argp { vkhr::arguments };
    auto scene_file = argp.parse(argc, argv);

    int width  = argp["x"].value.integer,
        height = argp["y"].value.integer;

    const vkhr::Image vulkan_icon { IMAGE("vulkan-icon.png") };
    vkhr::Window window { width, height, "VKHR", vulkan_icon };

    if (argp["fullscreen"].value.boolean)
        window.toggle_fullscreen();

    vkhr::InputMap input_map { window };

    input_map.bind("quit", vkhr::Input::Key::Escape);
    input_map.bind("fullscreen", vkhr::Input::Key::F);
    input_map.bind("grab", vkhr::Input::MouseButton::Left);
    input_map.bind("toggle_ui", vkhr::Input::Key::H);
    input_map.bind("recompile", vkhr::Input::Key::R);

    vkhr::SceneGraph scene_graph { scene_file };

    vkhr::Rasterizer rasterizer { window, scene_graph };

    vkhr::HairStyle hair_style { STYLE("ponytail.hair") };

    vk::VertexBuffer vertex_buffer {
        rasterizer.device,
        rasterizer.graphics_pool,
        hair_style.get_vertices(),
        0,
        {
            { 0, VK_FORMAT_R32G32B32_SFLOAT }
        }
    };

    hair_style.generate_tangents();

    vk::VertexBuffer tangent_buffer {
        rasterizer.device,
        rasterizer.graphics_pool,
        hair_style.get_tangents(),
        1,
        {
            { 1, VK_FORMAT_R32G32B32_SFLOAT }
        }
    };

    hair_style.generate_indices();

    vk::IndexBuffer index_buffer {
        rasterizer.device,
        rasterizer.graphics_pool,
        hair_style.get_indices()
    };

    vk::GraphicsPipeline::FixedFunction fixed_functions;

    fixed_functions.add_vertex_input(vertex_buffer);
    fixed_functions.add_vertex_input(tangent_buffer);

    fixed_functions.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

    fixed_functions.set_scissor({ 0, 0, rasterizer.swap_chain.get_extent() });
    fixed_functions.set_viewport({ 0.0, 0.0,
                                   static_cast<float>(rasterizer.swap_chain.get_width()),
                                   static_cast<float>(rasterizer.swap_chain.get_height()),
                                   0.0, 1.0 });

    fixed_functions.set_line_width(1.0);
    fixed_functions.enable_depth_test();

    fixed_functions.enable_alpha_blending_for(0);

    std::vector<vk::ShaderModule> shading_stages;

    shading_stages.emplace_back(rasterizer.device, SHADER("kajiya-kay.vert"));
    shading_stages.emplace_back(rasterizer.device, SHADER("kajiya-kay.frag"));

    struct Transform {
        glm::mat4 model      { 1.0f };
        glm::mat4 view       { 1.0f };
        glm::mat4 projection { 1.0f };
    } mvp;

    std::vector<vk::UniformBuffer> uniform_buffers;

    for (std::size_t i { 0 } ; i < rasterizer.swap_chain.size(); ++i)
        uniform_buffers.emplace_back(rasterizer.device, mvp,
                                     sizeof(mvp));

    vk::DescriptorSet::Layout descriptor_layout {
        rasterizer.device,
        {
            { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
        }
    };

    auto descriptor_sets = rasterizer.descriptor_pool.allocate(rasterizer.swap_chain.size(), descriptor_layout);

    for (std::size_t i { 0 }; i < descriptor_sets.size(); ++i)
        descriptor_sets[i].write(0, uniform_buffers[i]);

    vk::Pipeline::Layout layout {
        rasterizer.device,
        descriptor_layout
    };

    vk::GraphicsPipeline graphics_pipeline {
        rasterizer.device,
        shading_stages,
        fixed_functions,
        layout,
        rasterizer.render_pass
    };

    vkhr::Interface imgui {
        window,
        rasterizer.instance,
        rasterizer.device,
        rasterizer.descriptor_pool,
        rasterizer.render_pass,
        rasterizer.graphics_pool
    };

    vkhr::Camera camera { glm::radians(45.0f), rasterizer.swap_chain.get_width(),
                                               rasterizer.swap_chain.get_height() };
    camera.look_at({ 0.000f, 60.0f, 0.000f }, { 200.0f, 35.0f, 200.0f });

    mvp.projection  = camera.get_projection_matrix();

    glm::vec2 previous_mouse_position { input_map.get_mouse_position() };

    vkhr::Raytracer raytracer { camera, hair_style };

    window.show();

    while (window.is_open()) {
        if (input_map.just_pressed("quit")) {
            window.close();
        } else if (input_map.just_pressed("toggle_ui")) {
            imgui.toggle_visibility();
        } else if (input_map.just_pressed("fullscreen")) {
            window.toggle_fullscreen();
        } else if (input_map.just_pressed("recompile")) {
            // TODO: recompile shader.
        }

        glm::vec2 cursor_delta { 0.0f, 0.0f };
        if (input_map.just_released("grab")) {
            input_map.unlock_cursor();
        } else if (input_map.just_pressed("grab")) {
            input_map.freeze_cursor();
            previous_mouse_position  = input_map.get_mouse_position();
        } else if (input_map.pressed("grab") && !imgui.want_focus()) {
            glm::vec2 mouse_position = input_map.get_mouse_position();
            cursor_delta = (mouse_position - previous_mouse_position);
            previous_mouse_position = mouse_position; // single frame.
            camera.arcball_by(cursor_delta*window.delta_time()*0.32f);
        }

        imgui.update();

        auto next_image = rasterizer.swap_chain.acquire_next_image(rasterizer.image_available);

        rasterizer.command_buffer_done.wait_and_reset();

        for (std::size_t i = 0; i < rasterizer.framebuffers.size(); ++i) {
            rasterizer.command_buffers[i].begin();
            rasterizer.command_buffers[i].begin_render_pass(rasterizer.render_pass, rasterizer.framebuffers[i],
                                                            { 1.0f, 1.0f, 1.0f, 1.0f });
            rasterizer.command_buffers[i].bind_pipeline(graphics_pipeline);
            rasterizer.command_buffers[i].bind_descriptor_set(descriptor_sets[i],
                                                              graphics_pipeline);
            rasterizer.command_buffers[i].bind_index_buffer(index_buffer);
            rasterizer.command_buffers[i].bind_vertex_buffer(tangent_buffer);
            rasterizer.command_buffers[i].bind_vertex_buffer(vertex_buffer);
            rasterizer.command_buffers[i].draw_indexed(index_buffer.count());
            imgui.render(rasterizer.command_buffers[i]); // Re-uploads data.
            rasterizer.command_buffers[i].end_render_pass();
            rasterizer.command_buffers[i].end();
        }

        mvp.view   =   camera.get_view_matrix();
        uniform_buffers[next_image].update(mvp);

        rasterizer.device.get_graphics_queue().submit(rasterizer.command_buffers[next_image], rasterizer.image_available,
                                           VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                                           rasterizer.render_complete, rasterizer.command_buffer_done);

        rasterizer.device.get_present_queue().present(rasterizer.swap_chain, next_image, rasterizer.render_complete);

        window.poll_events();
    }

    return 0;
}

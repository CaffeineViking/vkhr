#include <vkhr/vulkan/hair_style.hh>

#include <vkhr/camera.hh>
#include <vkhr/light_source.hh>
#include <vkhr/rasterizer.hh>

namespace vk = vkpp;

namespace vkhr {
    namespace vulkan {
        HairStyle::HairStyle(const vkhr::HairStyle& hair_style,
                             vkhr::Rasterizer& vulkan_renderer,
                             Pipeline& hair_style_pipeline)
                            : pipeline { &hair_style_pipeline } {
            load(hair_style, vulkan_renderer);
        }

        void HairStyle::load(const vkhr::HairStyle& hair_style,
                             vkhr::Rasterizer& vulkan_renderer) {
            positions = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_vertices()
            };

            tangents = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_tangents()
            };

            vertices = vk::IndexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_indices()
            };
        }

        void HairStyle::update(MVP& transform, std::size_t i) {
            pipeline->descriptor_states[i].uniform_buffers[0].update(transform);
        }

        void HairStyle::draw(vk::CommandBuffer& command_list, std::size_t i) {
            command_list.bind_pipeline(pipeline->pipeline);

            auto& descriptor_sets = pipeline->descriptor_sets[i];

            command_list.bind_descriptor_set(descriptor_sets,
                                             pipeline->pipeline);

            command_list.bind_vertex_buffer(0, positions, 0);
            command_list.bind_vertex_buffer(1, tangents,  0);

            command_list.bind_index_buffer(vertices);

            command_list.draw_indexed(vertices.count());
        }

        void HairStyle::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline.fixed_stages.add_vertex_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)});
            pipeline.fixed_stages.add_vertex_binding({ 1, 1, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)});

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.set_line_width(1.0);
            pipeline.fixed_stages.enable_depth_test();
            pipeline.fixed_stages.enable_alpha_mix(0);

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("kajiya-kay.vert"));
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("kajiya-kay.frag"));

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
                }
            };

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout);
            pipeline.descriptor_states.resize(pipeline.descriptor_sets.size());
            for (std::size_t i = 0; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.write_uniform_buffer(i, 0, sizeof(MVP), vulkan_renderer.device);
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout
            };

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.render_pass
            };
        }
    }
}

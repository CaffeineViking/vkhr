#include <vkhr/vulkan/hair_style.hh>

#include <vkhr/rasterizer.hh>

namespace vk = vkpp;

namespace vkhr {
    namespace vulkan {
        HairStyle::HairStyle(const vkhr::HairStyle& hair_style,
                             vkhr::Rasterizer& vulkan_renderer) {
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

        void HairStyle::draw(vk::CommandBuffer& command_list,
                             vk::DescriptorSet& descriptor_set,
                             vk::GraphicsPipeline& pipeline) {
            command_list.bind_pipeline(pipeline);

            command_list.bind_descriptor_set(descriptor_set,
                                             pipeline);

            command_list.bind_vertex_buffer(positions, 0);
            command_list.bind_vertex_buffer(tangents,  1);

            command_list.bind_index_buffer(vertices);

            command_list.draw_indexed(vertices.count());
        }

        Pipeline HairStyle::build_pipeline(Rasterizer& vulkan_renderer) {
            Pipeline pipeline;

            pipeline.fixed_stages.add_vertex_attribute_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT });
            pipeline.fixed_stages.add_vertex_attribute_binding({ 1, 1, VK_FORMAT_R32G32B32_SFLOAT });

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

            pipeline.descriptors = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                            pipeline.descriptor_set_layout);

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

            return pipeline;
        }
    }
}

#include <vkhr/vulkan/hair_style.hh>

#include <vkhr/camera.hh>
#include <vkhr/light_source.hh>
#include <vkhr/rasterizer.hh>

#include <vkpp/debug_marker.hh>

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

            vk::DebugMarker::object_name(vulkan_renderer.device, positions, VK_OBJECT_TYPE_BUFFER, "Hair Style Position Vertex Buffer");
            vk::DebugMarker::object_name(vulkan_renderer.device, positions.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Style Position Device Memory");

            tangents = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_tangents()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangents, VK_OBJECT_TYPE_BUFFER, "Hair Style Tangent Vertex Buffer");
            vk::DebugMarker::object_name(vulkan_renderer.device, tangents.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Style Tangent Device Memory");

            vertices = vk::IndexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_indices()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, vertices, VK_OBJECT_TYPE_BUFFER, "Hair Style Index Buffer");
            vk::DebugMarker::object_name(vulkan_renderer.device, vertices.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Style Index Device Memory");
        }

        void HairStyle::draw(vk::CommandBuffer& command_buffer) {
            command_buffer.bind_vertex_buffer(0, positions, 0);
            command_buffer.bind_vertex_buffer(1, tangents,  0);

            command_buffer.bind_index_buffer(vertices);

            command_buffer.draw_indexed(vertices.count());
        }

        void HairStyle::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

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
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Kajiya-Kay Vertex Shader");
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("kajiya-kay.frag"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[1],
                                       VK_OBJECT_TYPE_SHADER_MODULE, "Kajiya-Kay Fragment Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                    { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Style Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Style Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.camera_vp[i]);
                pipeline.descriptor_sets[i].write(1, vulkan_renderer.light_buf[i]);
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // model.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Hair Style Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.color_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Hair Style Graphics Pipeline");
        }
    }
}

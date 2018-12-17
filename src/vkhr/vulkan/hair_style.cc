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

            vk::DebugMarker::object_name(vulkan_renderer.device, positions, VK_OBJECT_TYPE_BUFFER, "Hair Position Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, positions.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Position Device Memory", id);

            tangents = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_tangents()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, tangents, VK_OBJECT_TYPE_BUFFER, "Hair Tangent Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, tangents.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Tangent Device Memory", id);

            vertices = vk::IndexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                hair_style.get_indices()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, vertices, VK_OBJECT_TYPE_BUFFER, "Hair Index Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, vertices.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Hair Index Device Memory", id);

            ++id;
        }

        void HairStyle::draw(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
            command_buffer.bind_descriptor_set(descriptor_set, pipeline);

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

            std::uint32_t light_count = vulkan_renderer.shadow_maps.size();

            struct Constants {
                std::uint32_t light_size;
            } constant_data {
                light_count
            };

            std::vector<VkSpecializationMapEntry> constants {
                { 0, 0, sizeof(std::uint32_t) } // light size
            };

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("strand.vert"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Strand Vertex Shader");
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("strand.frag"),
                                                constants, &constant_data, sizeof(constant_data));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[1],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Strand Fragment Shader");

            std::vector<vk::DescriptorSet::Binding> descriptor_bindings {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
            };

            for (std::uint32_t i { 0 }; i < light_count; ++i)
                descriptor_bindings.push_back({ 3 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device, descriptor_bindings
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.camera_vp[i]);
                pipeline.descriptor_sets[i].write(1, vulkan_renderer.light_buf[i]);
                pipeline.descriptor_sets[i].write(2, vulkan_renderer.sm_params[i]);
                for (std::uint32_t j { 0 }; j < light_count; ++j)
                    pipeline.descriptor_sets[i].write(3 + j, vulkan_renderer.shadow_maps[j].get_image_view(),
                                                             vulkan_renderer.shadow_maps[j].get_sampler());
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
                                         "Hair Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.color_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Hair Graphics Pipeline");
        }

        void HairStyle::depth_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3)});

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_LINE_LIST);

            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);

            pipeline.fixed_stages.set_culling_mode(VK_CULL_MODE_BACK_BIT);

            pipeline.fixed_stages.set_line_width(1.0);
            pipeline.fixed_stages.enable_depth_test();

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("depth_pass.vert"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Strand Depth Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Depth Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Depth Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // transforms.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Hair Depth Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.depth_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Hair Depth Graphics Pipeline");
        }

        void HairStyle::voxel_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("voxelize.comp"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0],
                                         VK_OBJECT_TYPE_SHADER_MODULE, "Strand Voxel Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                {
                    { 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER }
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout,
                                         VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Hair Voxel Descriptor Set Layout");
            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Hair Voxel Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout,
                                         VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         "Hair Voxel Pipeline Layout");

            pipeline.compute_pipeline = vk::ComputePipeline {
                vulkan_renderer.device,
                pipeline.shader_stages[0],
                pipeline.pipeline_layout
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.compute_pipeline,
                                         VK_OBJECT_TYPE_PIPELINE, "Hair Voxel Graphics Pipeline");
        }

        int HairStyle::id { 0 };
    }
}

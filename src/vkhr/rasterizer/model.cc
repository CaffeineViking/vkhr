#include <vkhr/rasterizer/model.hh>

#include <vkhr/rasterizer.hh>

#include <vkhr/scene_graph/camera.hh>
#include <vkhr/scene_graph/light_source.hh>

#include <vkpp/debug_marker.hh>

namespace vkhr {
    namespace vulkan {
        Model::Model(const vkhr::Model& wavefront_model,
                     vkhr::Rasterizer& vulkan_renderer) {
            load(wavefront_model, vulkan_renderer);
        }

        void Model::load(const vkhr::Model& wavefront_model,
                         vkhr::Rasterizer& vulkan_renderer) {
            vertices = vk::VertexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                wavefront_model.get_vertices()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, vertices, VK_OBJECT_TYPE_BUFFER, "Model Vertex Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, vertices.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Model Vertex Device Memory", id);

            elements = vk::IndexBuffer {
                vulkan_renderer.device,
                vulkan_renderer.command_pool,
                wavefront_model.get_elements()
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, elements, VK_OBJECT_TYPE_BUFFER, "Model Index Buffer", id);
            vk::DebugMarker::object_name(vulkan_renderer.device, elements.get_device_memory(), VK_OBJECT_TYPE_DEVICE_MEMORY,
                                         "Model Index Device Memory", id);

            ++id;
        }

        void Model::draw(Pipeline& pipeline, vk::DescriptorSet& descriptor_set, vk::CommandBuffer& command_buffer) {
            command_buffer.bind_descriptor_set(descriptor_set, pipeline);
            command_buffer.bind_vertex_buffer(0, vertices);
            command_buffer.bind_index_buffer(elements, 0);
            command_buffer.draw_indexed(elements.count());
        }

        void Model::build_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, sizeof(vkhr::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX });

            pipeline.fixed_stages.add_vertex_attribute({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });
            pipeline.fixed_stages.add_vertex_attribute({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) });
            pipeline.fixed_stages.add_vertex_attribute({ 2, 0, VK_FORMAT_R32G32_SFLOAT,    sizeof(glm::vec3) * 2 });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

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

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("models/model.vert"));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0], VK_OBJECT_TYPE_SHADER_MODULE, "Model Vertex Shader");
            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("models/model.frag"), constants, &constant_data, sizeof(constant_data));
            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[1], VK_OBJECT_TYPE_SHADER_MODULE, "Model Fragment Shader");

            std::vector<vk::DescriptorSet::Binding> descriptor_bindings {
                { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER },
                { 4, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER }
            };

            for (std::uint32_t i { 0 }; i < light_count; ++i)
                descriptor_bindings.push_back({ 5 + i, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER });

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device,
                descriptor_bindings
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Model Descriptor Set Layout");

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Model Descriptor Set");

            for (std::size_t i { 0 }; i < pipeline.descriptor_sets.size(); ++i) {
                pipeline.descriptor_sets[i].write(0, vulkan_renderer.camera[i]);
                pipeline.descriptor_sets[i].write(1, vulkan_renderer.lights[i]);
                pipeline.descriptor_sets[i].write(4, vulkan_renderer.params[i]);
                for (std::uint32_t j { 0 }; j < light_count; ++j)
                    pipeline.descriptor_sets[i].write(5 + j, vulkan_renderer.shadow_maps[j].get_image_view(),
                                                             vulkan_renderer.shadow_maps[j].get_sampler());
            }

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // model.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Model Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.color_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Model Graphics Pipeline");
        }

        void Model::depth_pipeline(Pipeline& pipeline, Rasterizer& vulkan_renderer) {
            pipeline = Pipeline { /* In the case we are re-creating the pipeline. */ };

            pipeline.fixed_stages.add_vertex_binding({ 0, sizeof(vkhr::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX });

            pipeline.fixed_stages.add_vertex_attribute({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 });

            pipeline.fixed_stages.set_scissor({ 0, 0, vulkan_renderer.swap_chain.get_extent() });
            pipeline.fixed_stages.set_viewport({ 0.0, 0.0,
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_width()),
                                                 static_cast<float>(vulkan_renderer.swap_chain.get_height()),
                                                 0.0, 1.0 });

            pipeline.fixed_stages.set_topology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_VIEWPORT);
            pipeline.fixed_stages.add_dynamic_state(VK_DYNAMIC_STATE_SCISSOR);

            pipeline.fixed_stages.set_culling_mode(VK_CULL_MODE_BACK_BIT);

            pipeline.fixed_stages.enable_depth_test();

            pipeline.shader_stages.emplace_back(vulkan_renderer.device, SHADER("self-shadowing/depth_map.vert"));

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.shader_stages[0], VK_OBJECT_TYPE_SHADER_MODULE, "Model Depth Shader");

            pipeline.descriptor_set_layout = vk::DescriptorSet::Layout {
                vulkan_renderer.device
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.descriptor_set_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "Model Depth Descriptor Set Layout");

            pipeline.descriptor_sets = vulkan_renderer.descriptor_pool.allocate(vulkan_renderer.swap_chain.size(),
                                                                                pipeline.descriptor_set_layout,
                                                                                "Model Depth Descriptor Set");

            pipeline.pipeline_layout = vk::Pipeline::Layout {
                vulkan_renderer.device,
                pipeline.descriptor_set_layout,
                {
                    { VK_SHADER_STAGE_ALL, 0, sizeof(glm::mat4) } // transforms.
                }
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline_layout, VK_OBJECT_TYPE_PIPELINE_LAYOUT, "Model Depth Pipeline Layout");

            pipeline.pipeline = vk::GraphicsPipeline {
                vulkan_renderer.device,
                pipeline.shader_stages,
                pipeline.fixed_stages,
                pipeline.pipeline_layout,
                vulkan_renderer.depth_pass
            };

            vk::DebugMarker::object_name(vulkan_renderer.device, pipeline.pipeline, VK_OBJECT_TYPE_PIPELINE, "Model Depth Graphics Pipeline");
        }

        int Model::id { 0 };
    }
}